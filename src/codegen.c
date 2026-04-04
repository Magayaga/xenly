/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
/*
 * codegen.c  —  Xenly AST → native assembly
 *
 * Two backends, selected at compile time:
 *
 *   x86-64  (Linux/BSD, macOS/Intel)
 *     • AT&T syntax, System V AMD64 ABI
 *     • Result XlyVal* in %rax after every emit_expr()
 *     • %rsp 16-byte aligned before every call; no red-zone use
 *
 *   AArch64 / ARM64  (macOS Apple Silicon)
 *     • Apple ARM64 ABI (AAPCS64 + Apple extensions)
 *     • Result XlyVal* in x0 after every emit_expr_a64()
 *     • sp must be 16-byte aligned at ALL times (not just at calls)
 *     • Frame: stp x29,x30,[sp,#-N]!  /  mov x29,sp
 *     • Variables at negative offsets from x29 (frame pointer)
 *     • Integer args: x0-x7   FP args: d0-d7   Scratch: x9-x15
 *
 * Supported AST nodes (both backends):
 *   Literals    NUMBER STRING BOOL NULL
 *   Vars        VAR_DECL ASSIGN COMPOUND_ASSIGN INCREMENT DECREMENT IDENTIFIER
 *   Exprs       BINARY UNARY FN_CALL METHOD_CALL TYPEOF
 *   Stmts       PRINT IF WHILE FOR FOR_IN BREAK CONTINUE RETURN BLOCK
 *   Decls       FN_DECL IMPORT (import is a no-op; modules are linked)
 */

#include "codegen.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

/*
 * macOS/Mach-O requires a leading underscore on C symbol names in assembly.
 * Linux/ELF does not.  XLY_SYM() wraps a literal symbol name appropriately.
 */
#if defined(XLY_PLATFORM_MACOS) || defined(PLATFORM_MACOS)
#  define XLY_SYM(s)  "_" s
#  define XLY_TEXT_SECTION   ".section __TEXT,__text,regular,pure_instructions"
#  define XLY_DATA_SECTION   ".section __TEXT,__cstring,cstring_literals"
#  define XLY_EMIT_GNU_STACK 0
#else
#  define XLY_SYM(s)  s
#  define XLY_TEXT_SECTION   ".section .text"
#  define XLY_DATA_SECTION   ".section .rodata"
#  define XLY_EMIT_GNU_STACK 1
#  define XLY_GNU_STACK_SECTION ".section .note.GNU-stack,\"\",@progbits"
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * ARCH SELECTION
 * ═══════════════════════════════════════════════════════════════════════════ */
#if defined(__arm64__) || defined(__aarch64__)
#  define XLY_ARCH_ARM64 1
#else
#  define XLY_ARCH_X86_64 1
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * SHARED: codegen state (same layout for both backends)
 * ═══════════════════════════════════════════════════════════════════════════ */
/* Named type for the rename table entries — used in CG, funcs[], and helpers */
typedef struct { char *src; char *lbl; } FnRename;

/* ── codegen state ──────────────────────────────────────────────────────── */
/* ── Optimization levels ────────────────────────────────────────────────────
 *   0 = -O0: no opts (debug output, most readable asm)
 *   1 = -O1: constant folding + noreturn elision
 *   2 = -O2: + unboxed numeric fast-paths, sys constant inlining  (default)
 *   3 = -O3: + aggressive inlining of single-arg sys calls
 * ─────────────────────────────────────────────────────────────────────────── */
typedef struct {
    FILE   *out;
    int     label_seq;          /* monotonic label counter                 */
    int     opt_level;          /* optimization level: 0-3                 */
    int     verbose;            /* emit source-level comments in asm       */
    int     noreturn_mode;      /* 1 = we are dead after sys.exit/abort    */

    /* variable table: name + rbp-offset + scope-depth */
    struct { char *name; int offset; int depth; } *vars;
    int     var_count, var_cap;

    /* global variable table: top-level vars accessible from any function.
     * These are stored in a fixed-size BSS block (__xly_globals) and
     * accessed via global pointer loads rather than rbp-relative.
     * global_offset[i] = byte offset into __xly_globals for the i-th global. */
    struct { char *name; int index; } *gvars;
    int     gvar_count, gvar_cap;
    int     in_main_scope;  /* 1 while emitting main() top-level statements */

    int     scope_depth;
    int     frame_offset;       /* next free slot (negative, grows down)   */

    /* interned string literals → .rodata labels */
    struct { char *text; char *label; } *strings;
    int     str_count, str_cap;

    /* break / continue label stacks */
    char   **brk_labels;  int brk_top;
    char   **cnt_labels;  int cnt_top;

    /* collected fn declarations, emitted after main */
    struct {
        ASTNode *node;
        char   **captures;   /* names of captured outer variables (NULL if none) */
        int      ncaptures;
        char    *asm_label;  /* unique asm label suffix (without .Lxly_fn_ prefix) */
        /* rename context snapshot: the fn_renames visible when this decl was encountered,
         * so that when the body is later emitted the same peer/sibling names resolve. */
        FnRename *rename_ctx;
        int rename_ctx_count;
    } *funcs;
    int     func_count, func_cap;

    /* set of declared function names (for call-site dispatch) */
    char  **fn_names;
    int     fn_name_count, fn_name_cap;

    /* set of asm labels already claimed for fn bodies (never reset) */
    struct { char *lbl; } *fn_labels_used;
    int fn_labels_used_count, fn_labels_used_cap;

    /* per-scope rename table: source-name → unique asm label
     * Saved/restored around each emit_function call.             */
    FnRename *fn_renames;
    int fn_rename_count, fn_rename_cap;

    /* enum variant name table: tracks variant names and their param counts
     * so NODE_FN_CALL and NODE_IDENTIFIER can emit xly_make_variant calls */
    struct { char *name; int nparams; } *variants;
    int     variant_count, variant_cap;

    int     had_error;

    /* ARM64-specific: frame-relative spill tracking (no sp movement) */
    int     a64_spill_depth;    /* current spill nesting level             */
    int     a64_frame_size;     /* total frame in bytes (set per function) */
    int     a64_sp_adj;         /* bytes sp has been moved below x29 by sub/add */

    /* stats (emitted with --verbose) */
    int     stat_const_fold;    /* number of compile-time constant folds   */
    int     stat_noreturn_elim; /* statements skipped after noreturn call  */
    int     stat_sys_inline;    /* sys.CONSTANT() calls inlined            */
    int     stat_unboxed_ops;   /* unboxed arithmetic/compare ops emitted  */
} CG;

/* ── emit helpers ───────────────────────────────────────────────────────── */
static void emit(CG *cg, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vfprintf(cg->out, fmt, ap);
    va_end(ap);
    fputc('\n', cg->out);
}

static int fresh_label(CG *cg, char *buf, size_t sz) {
    return snprintf(buf, sz, ".Lxly_%d", cg->label_seq++);
}

/* ── string intern ──────────────────────────────────────────────────────── */
static const char *intern_string(CG *cg, const char *text) {
    for (int i = 0; i < cg->str_count; i++)
        if (strcmp(cg->strings[i].text, text) == 0)
            return cg->strings[i].label;
    if (cg->str_count >= cg->str_cap) {
        cg->str_cap = cg->str_cap ? cg->str_cap * 2 : 32;
        cg->strings = realloc(cg->strings, sizeof(cg->strings[0]) * (size_t)cg->str_cap);
    }
    char lbl[64];
    snprintf(lbl, sizeof(lbl), ".Lxly_str_%d", cg->str_count);
    cg->strings[cg->str_count].text  = strdup(text);
    cg->strings[cg->str_count].label = strdup(lbl);
    cg->str_count++;
    return cg->strings[cg->str_count - 1].label;
}

/* ── variable table ─────────────────────────────────────────────────────── */
static int var_find(CG *cg, const char *name) {
    for (int i = cg->var_count - 1; i >= 0; i--)
        if (strcmp(cg->vars[i].name, name) == 0) return i;
    return -1;
}

/* ── global variable table ──────────────────────────────────────────────── */
/* Top-level const/var declarations are stored in global pointer slots so
 * user functions can read them.  Each gets an index; the BSS block is
 * __xly_globals (array of XlyVal*).
 *
 * gvar_find() returns the global index, or -1 if not global.
 * Global read:  movq __xly_globals+index*8(%rip), %rax  (x86-64)
 * Global write: movq %rax, __xly_globals+index*8(%rip)
 */
static int gvar_find(CG *cg, const char *name) {
    for (int i = 0; i < cg->gvar_count; i++)
        if (strcmp(cg->gvars[i].name, name) == 0) return cg->gvars[i].index;
    return -1;
}
static int gvar_declare(CG *cg, const char *name) {
    int existing = gvar_find(cg, name);
    if (existing >= 0) return existing;
    if (cg->gvar_count >= cg->gvar_cap) {
        cg->gvar_cap = cg->gvar_cap ? cg->gvar_cap * 2 : 16;
        cg->gvars = realloc(cg->gvars, sizeof(cg->gvars[0]) * (size_t)cg->gvar_cap);
    }
    int idx = cg->gvar_count;
    cg->gvars[cg->gvar_count].name  = strdup(name);
    cg->gvars[cg->gvar_count].index = idx;
    cg->gvar_count++;
    return idx;
}

/* var_offset: returns rbp-relative offset if found locally, 0 if not found.
 * For globals, callers check gvar_find() separately. */
static int var_offset(CG *cg, const char *name) {
    int i = var_find(cg, name);
    return i >= 0 ? cg->vars[i].offset : 0;
}
static int var_declare(CG *cg, const char *name) {
    /* idempotent at same depth */
    for (int i = cg->var_count - 1; i >= 0; i--) {
        if (cg->vars[i].depth != cg->scope_depth) break;
        if (strcmp(cg->vars[i].name, name) == 0) return cg->vars[i].offset;
    }
    if (cg->var_count >= cg->var_cap) {
        cg->var_cap = cg->var_cap ? cg->var_cap * 2 : 16;
        cg->vars    = realloc(cg->vars, sizeof(cg->vars[0]) * (size_t)cg->var_cap);
    }
    cg->frame_offset -= 8;
    cg->vars[cg->var_count].name   = strdup(name);
    cg->vars[cg->var_count].offset = cg->frame_offset;
    cg->vars[cg->var_count].depth  = cg->scope_depth;
    cg->var_count++;
    return cg->frame_offset;
}
static void scope_enter(CG *cg) { cg->scope_depth++; }
static void scope_leave(CG *cg) {
    while (cg->var_count > 0 && cg->vars[cg->var_count-1].depth == cg->scope_depth) {
        cg->var_count--;
        free(cg->vars[cg->var_count].name);
    }
    cg->scope_depth--;
}

/* ── break/continue stacks ─────────────────────────────────────────────── */
static void push_brk(CG *cg, const char *l) {
    cg->brk_labels = realloc(cg->brk_labels, sizeof(char*)*(size_t)(cg->brk_top+1));
    cg->brk_labels[cg->brk_top++] = strdup(l);
}
static void pop_brk(CG *cg) { free(cg->brk_labels[--cg->brk_top]); }
static void push_cnt(CG *cg, const char *l) {
    cg->cnt_labels = realloc(cg->cnt_labels, sizeof(char*)*(size_t)(cg->cnt_top+1));
    cg->cnt_labels[cg->cnt_top++] = strdup(l);
}
static void pop_cnt(CG *cg) { free(cg->cnt_labels[--cg->cnt_top]); }

/* ── forward declarations (x86-64 only) ────────────────────────────────── */
#ifdef XLY_ARCH_X86_64
static void emit_expr(CG *cg, ASTNode *node);
static void emit_stmt(CG *cg, ASTNode *node);
#endif

/* ── declared-function name registry ───────────────────────────────────── */
static void fn_name_register(CG *cg, const char *name) {
    for (int i = 0; i < cg->fn_name_count; i++)
        if (strcmp(cg->fn_names[i], name) == 0) return;
    if (cg->fn_name_count >= cg->fn_name_cap) {
        cg->fn_name_cap = cg->fn_name_cap ? cg->fn_name_cap * 2 : 16;
        cg->fn_names = realloc(cg->fn_names, sizeof(char*) * (size_t)cg->fn_name_cap);
    }
    cg->fn_names[cg->fn_name_count++] = strdup(name);
}
static int fn_name_is_declared(CG *cg, const char *name) {
    for (int i = 0; i < cg->fn_name_count; i++)
        if (strcmp(cg->fn_names[i], name) == 0) return 1;
    return 0;
}

/* ── fn label uniquification helpers ────────────────────────────────────
 * Guarantee every function body gets a globally-unique asm label even
 * when the same name is used in different scopes (nested functions,
 * closures, sibling namespaces, etc.).
 *
 * fn_labels_used  — global set of labels already claimed this compilation
 * fn_renames      — scope-local stack: source-name → unique asm label
 *                   saved/restored around each emit_function() call       */

static int fn_label_is_used(CG *cg, const char *lbl) {
    for (int i = 0; i < cg->fn_labels_used_count; i++)
        if (strcmp(cg->fn_labels_used[i].lbl, lbl) == 0) return 1;
    return 0;
}
static void fn_label_mark_used(CG *cg, const char *lbl) {
    if (fn_label_is_used(cg, lbl)) return;
    if (cg->fn_labels_used_count >= cg->fn_labels_used_cap) {
        cg->fn_labels_used_cap = cg->fn_labels_used_cap ? cg->fn_labels_used_cap * 2 : 16;
        cg->fn_labels_used = realloc(cg->fn_labels_used,
                             sizeof(cg->fn_labels_used[0]) * (size_t)cg->fn_labels_used_cap);
    }
    cg->fn_labels_used[cg->fn_labels_used_count++].lbl = strdup(lbl);
}
/* Pick a unique label for `name`; write into buf[bufsz]; mark as used. */
static void fn_unique_label(CG *cg, const char *name, char *buf, size_t bufsz) {
    if (!fn_label_is_used(cg, name)) {
        snprintf(buf, bufsz, "%s", name);
    } else {
        for (int n = 2; ; n++) {
            snprintf(buf, bufsz, "%s__%d", name, n);
            if (!fn_label_is_used(cg, buf)) break;
        }
    }
    fn_label_mark_used(cg, buf);
}
/* Push source→label rename for the current function scope. */
static void fn_rename_push(CG *cg, const char *src, const char *lbl) {
    if (cg->fn_rename_count >= cg->fn_rename_cap) {
        cg->fn_rename_cap = cg->fn_rename_cap ? cg->fn_rename_cap * 2 : 16;
        cg->fn_renames = realloc(cg->fn_renames,
                         sizeof(FnRename) * (size_t)cg->fn_rename_cap);
    }
    cg->fn_renames[cg->fn_rename_count].src = strdup(src);
    cg->fn_renames[cg->fn_rename_count].lbl = strdup(lbl);
    cg->fn_rename_count++;
}
/* Pop rename entries back to `target`, freeing strings. */
static void fn_renames_pop_to(CG *cg, int target) {
    while (cg->fn_rename_count > target) {
        cg->fn_rename_count--;
        free(cg->fn_renames[cg->fn_rename_count].src);
        free(cg->fn_renames[cg->fn_rename_count].lbl);
    }
}
/* Return the innermost asm label for `src`, or NULL if not renamed. */
static const char *fn_rename_lookup(CG *cg, const char *src) {
    for (int i = cg->fn_rename_count - 1; i >= 0; i--)
        if (strcmp(cg->fn_renames[i].src, src) == 0)
            return cg->fn_renames[i].lbl;
    return NULL;
}

/* Snapshot the current rename stack: returns a malloc'd copy array + count.
 * The caller owns the returned array; free with fn_rename_ctx_free().        */
static void fn_rename_ctx_snapshot(CG *cg, FnRename **out, int *out_count) {
    *out_count = cg->fn_rename_count;
    if (cg->fn_rename_count == 0) { *out = NULL; return; }
    *out = malloc(sizeof(FnRename) * (size_t)cg->fn_rename_count);
    for (int i = 0; i < cg->fn_rename_count; i++) {
        (*out)[i].src = strdup(cg->fn_renames[i].src);
        (*out)[i].lbl = strdup(cg->fn_renames[i].lbl);
    }
}
/* Restore (overlay) a snapshot onto the rename stack: push all entries from
 * snapshot that are not already present (inner scope entries win).           */
static void fn_rename_ctx_restore(CG *cg, FnRename *ctx, int count) {
    for (int i = 0; i < count; i++) {
        /* Only push if not already in stack (current scope wins) */
        if (!fn_rename_lookup(cg, ctx[i].src))
            fn_rename_push(cg, ctx[i].src, ctx[i].lbl);
    }
}
/* Free a snapshot. */
static void fn_rename_ctx_free(FnRename *ctx, int count) {
    for (int i = 0; i < count; i++) { free(ctx[i].src); free(ctx[i].lbl); }
    free(ctx);
}
/* Resolve identifier `src` to its asm label (rename wins over fn_names). */
static const char *fn_resolve_label(CG *cg, const char *src) {
    const char *r = fn_rename_lookup(cg, src);
    return r ? r : src;
}
/* True if `name` is a known callable function in the current scope.
 * Combines the global fn_names[] table (top-level) with the per-scope
 * rename table (nested / inner functions).                               */
static int fn_is_known(CG *cg, const char *name) {
    return fn_name_is_declared(cg, name) || fn_rename_lookup(cg, name) != NULL;
}

/* ── enum variant table helpers ──────────────────────────────────────────── */
static void variant_register(CG *cg, const char *name, int nparams) {
    for (int i = 0; i < cg->variant_count; i++)
        if (strcmp(cg->variants[i].name, name) == 0) return;
    if (cg->variant_count >= cg->variant_cap) {
        cg->variant_cap = cg->variant_cap ? cg->variant_cap * 2 : 8;
        cg->variants = realloc(cg->variants, sizeof(cg->variants[0]) * (size_t)cg->variant_cap);
    }
    cg->variants[cg->variant_count].name    = strdup(name);
    cg->variants[cg->variant_count].nparams = nparams;
    cg->variant_count++;
}
/* Returns nparams if name is a known variant, -1 otherwise. */
static int variant_find(CG *cg, const char *name) {
    for (int i = 0; i < cg->variant_count; i++)
        if (strcmp(cg->variants[i].name, name) == 0)
            return cg->variants[i].nparams;
    return -1;
}

/* ── free-variable analysis for closure capture ─────────────────────────
 * Collects names of identifiers in `node` that are NOT in `params[]`
 * and NOT globally declared functions — these are captured from outer scope. */
typedef struct { char **names; int count, cap; } NameSet;
static void nameset_add(NameSet *s, const char *n) {
    for (int i = 0; i < s->count; i++) if (strcmp(s->names[i], n) == 0) return;
    if (s->count >= s->cap) {
        s->cap = s->cap ? s->cap * 2 : 8;
        s->names = realloc(s->names, sizeof(char*) * (size_t)s->cap);
    }
    s->names[s->count++] = strdup(n);
}
static void nameset_free(NameSet *s) {
    for (int i = 0; i < s->count; i++) free(s->names[i]);
    free(s->names);
}
static int nameset_has(NameSet *s, const char *n) {
    for (int i = 0; i < s->count; i++) if (strcmp(s->names[i], n) == 0) return 1;
    return 0;
}
/* Collect all IDENTIFIER references in subtree, excluding param names */
static void collect_free_vars(CG *cg, ASTNode *node, NameSet *params, NameSet *out) {
    if (!node) return;
    if (node->type == NODE_IDENTIFIER && node->str_value) {
        const char *nm = node->str_value;
        if (!nameset_has(params, nm) && !fn_name_is_declared(cg, nm))
            nameset_add(out, nm);
    }
    /* FN_CALL: the callee name is in str_value, not a child IDENTIFIER */
    if (node->type == NODE_FN_CALL && node->str_value) {
        const char *nm = node->str_value;
        if (!nameset_has(params, nm) && !fn_name_is_declared(cg, nm))
            nameset_add(out, nm);
    }
    /* Don't recurse into nested fn decls/arrow fns — they have their own captures */
    if (node->type == NODE_FN_DECL || node->type == NODE_ARROW_FN) return;
    for (size_t i = 0; i < node->child_count; i++)
        collect_free_vars(cg, node->children[i], params, out);
}

/* ── pre-pass: collect top-level fn declaration names ────────────────── *
 * Only registers direct top-level function names so forward calls work.
 * Nested functions (inside other fn bodies) are uniquified at emit time
 * to prevent duplicate asm label errors when names are reused across    *
 * scopes.                                                                */
static void collect_fn_names(CG *cg, ASTNode *node) {
    if (!node) return;
    if (node->type == NODE_FN_DECL && node->str_value) {
        fn_name_register(cg, node->str_value);
        return; /* do NOT recurse into body — nested fn labels assigned at emit time */
    }
    for (size_t i = 0; i < node->child_count; i++)
        collect_fn_names(cg, node->children[i]);
    /* also recurse into params default values */
    for (size_t i = 0; i < node->param_count; i++)
        if (node->params && node->params[i].default_value)
            collect_fn_names(cg, node->params[i].default_value);
}

/* ── pre-pass: collect enum variant names ───────────────────────────────── */
static void collect_variant_names(CG *cg, ASTNode *node) {
    if (!node) return;
    if (node->type == NODE_ENUM_DECL) {
        for (size_t i = 0; i < node->child_count; i++) {
            ASTNode *v = node->children[i];
            if (v && v->str_value)
                variant_register(cg, v->str_value, (int)v->param_count);
        }
        return; /* don't recurse into children — they are variant nodes */
    }
    for (size_t i = 0; i < node->child_count; i++)
        collect_variant_names(cg, node->children[i]);
}

/* ── pre-pass: count VAR_DECLs recursively ─────────────────────────────── */
static int count_locals(ASTNode *node) {
    if (!node) return 0;
    int n = (node->type == NODE_VAR_DECL || node->type == NODE_CONST_DECL) ? 1 : 0;
    /* for-in declares 4 hidden slots per loop; count conservatively */
    if (node->type == NODE_FOR_IN) n += 4;
    for (size_t i = 0; i < node->child_count; i++)
        n += count_locals(node->children[i]);
    return n;
}

/* ── Global opt/verbose settings — shared by both backends ─────────────────
 * Set by codegen_set_opts() (called from xenlyc_main before codegen()).
 * Declared here (outside any arch #ifdef) so both x86-64 and ARM64 see them. */
static int g_opt_level   = 2;   /* default: -O2 */
static int g_verbose_asm = 0;   /* default: no annotation */

/* ═══════════════════════════════════════════════════════════════════════════
 * x86-64 BACKEND  (Linux/BSD + macOS Intel)
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifdef XLY_ARCH_X86_64

/* ── load a double constant into %xmm0 ─────────────────────────────────
 * We sub 8 from rsp, write the bits, movsd, add 8 back.  No red-zone, no
 * dangling push at a call boundary.                                        */
static void emit_load_double(CG *cg, double d) {
    union { double d; unsigned long long u; } uu; uu.d = d;
    emit(cg, "    subq    $8, %%rsp");
    emit(cg, "    movabsq $%llu, %%rax", (unsigned long long)uu.u);
    emit(cg, "    movq    %%rax, (%%rsp)");
    emit(cg, "    movsd   (%%rsp), %%xmm0");
    emit(cg, "    addq    $8, %%rsp");
}

/* ── expression compiler ────────────────────────────────────────────────
 * Post-condition: result XlyVal* is in %rax.  %rsp is unchanged.         */
static void emit_expr(CG *cg, ASTNode *node) {
    if (!node) { emit(cg, "    call    " XLY_SYM("xly_null")); return; }

    switch (node->type) {

    /* ── literals ────────────────────────────────────────────────────── */
    case NODE_NUMBER:
        emit_load_double(cg, node->num_value);
        emit(cg, "    call    " XLY_SYM("xly_num"));
        break;

    case NODE_STRING: {
        const char *lbl = intern_string(cg, node->str_value ? node->str_value : "");
        emit(cg, "    leaq    %s(%%rip), %%rdi", lbl);
        emit(cg, "    call    " XLY_SYM("xly_str"));
        break;
    }

    case NODE_BOOL:
        emit(cg, "    movl    $%d, %%edi", node->bool_value ? 1 : 0);
        emit(cg, "    call    " XLY_SYM("xly_bool"));
        break;

    case NODE_NULL:
        emit(cg, "    call    " XLY_SYM("xly_null"));
        break;

    /* ── anonymous function expression: fn(params) { body } ──────────
     * Synthesise a unique name, stash the node for later emission,
     * then wrap the raw label address as a VAL_FUNCTION XlyVal* via
     * xly_make_fn() so the value can be stored in a variable.         */
    case NODE_ARROW_FN: {
        char anon_name[64];
        snprintf(anon_name, sizeof(anon_name), "__anon_%d", cg->label_seq++);
        /* Give the node a synthesized name so emit_function can use it */
        if (node->str_value) free(node->str_value);
        node->str_value = strdup(anon_name);
        fn_name_register(cg, anon_name);

        /* ── free-variable analysis: find captured outer variables ── */
        NameSet fn_params = {0};
        for (size_t i = 0; i < node->param_count; i++)
            nameset_add(&fn_params, node->params[i].name);
        NameSet free_vars = {0};
        if (node->child_count > 0)
            collect_free_vars(cg, node->children[0], &fn_params, &free_vars);
        nameset_free(&fn_params);
        /* Filter to only vars that actually exist in current scope */
        NameSet captures = {0};
        for (int i = 0; i < free_vars.count; i++) {
            int off = var_offset(cg, free_vars.names[i]);
            if (off != 0)
                nameset_add(&captures, free_vars.names[i]);
        }
        nameset_free(&free_vars);

        /* Stash for emission after main */
        if (cg->func_count >= cg->func_cap) {
            cg->func_cap = cg->func_cap ? cg->func_cap * 2 : 16;
            cg->funcs    = realloc(cg->funcs, sizeof(cg->funcs[0]) * (size_t)cg->func_cap);
        }
        int fi = cg->func_count++;
        cg->funcs[fi].node       = node;
        cg->funcs[fi].ncaptures  = captures.count;
        cg->funcs[fi].captures   = captures.count > 0 ? captures.names : NULL;
        cg->funcs[fi].asm_label  = strdup(anon_name);
        fn_label_mark_used(cg, anon_name);
        fn_rename_ctx_snapshot(cg, &cg->funcs[fi].rename_ctx, &cg->funcs[fi].rename_ctx_count);
        /* (don't nameset_free: captures.names ownership transferred) */

        if (captures.count == 0) {
            /* No captures: simple function pointer wrap */
            emit(cg, "    leaq    .Lxly_fn_%s(%%rip), %%rdi", anon_name);
            emit(cg, "    call    " XLY_SYM("xly_make_fn"));
        } else {
            /* Build env array on stack: [cap0, cap1, ...] */
            int env_bytes = ((captures.count * 8) + 15) & ~15;
            emit(cg, "    subq    $%d, %%rsp", env_bytes);
            for (int i = 0; i < captures.count; i++) {
                int off = var_offset(cg, captures.names[i]);
                emit(cg, "    movq    %d(%%rbp), %%rax", off);
                emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
            }
            /* xly_make_closure(fp, env, env_size) */
            emit(cg, "    leaq    .Lxly_fn_%s(%%rip), %%rdi", anon_name);
            emit(cg, "    movq    %%rsp, %%rsi");
            emit(cg, "    movl    $%d, %%edx", captures.count);
            emit(cg, "    call    " XLY_SYM("xly_make_closure"));
            emit(cg, "    addq    $%d, %%rsp", env_bytes);
        }
        break;
    }

    /* ── identifier ──────────────────────────────────────────────────── */
    case NODE_IDENTIFIER: {
        int off = var_offset(cg, node->str_value);
        if (off != 0) {
            emit(cg, "    movq    %d(%%rbp), %%rax", off);
        } else {
            int gi = gvar_find(cg, node->str_value);
            if (gi >= 0) {
                /* Global variable: load from __xly_globals array */
                emit(cg, "    movq    " XLY_SYM("__xly_globals") "+%d(%%rip), %%rax", gi * 8);
            } else if (fn_is_known(cg, node->str_value)) {
                /* Declared function used as a value — wrap its address as XlyVal* */
                const char *lbl = fn_resolve_label(cg, node->str_value);
                emit(cg, "    leaq    .Lxly_fn_%s(%%rip), %%rdi", lbl);
                emit(cg, "    call    " XLY_SYM("xly_make_fn"));
            } else {
                emit(cg, "    call    " XLY_SYM("xly_null"));   /* undefined → null */
            }
        }
        break;
    }

    /* ── this ────────────────────────────────────────────────────────── */
    case NODE_THIS:
        emit(cg, "    call    " XLY_SYM("xly_this"));
        break;

    /* ── binary ──────────────────────────────────────────────────────── */
    case NODE_BINARY: {
        const char *op = node->str_value;

        /* ── short-circuit: and ──
         * eval left → if falsy jump to end (result = left)
         * eval right → result = right                          */
        if (strcmp(op, "and") == 0) {
            char lbl_end[64], lbl_done[64];
            int seq = cg->label_seq++;
            snprintf(lbl_end,  sizeof(lbl_end),  ".Lxly_%d_and_end",  seq);
            snprintf(lbl_done, sizeof(lbl_done), ".Lxly_%d_and_done", seq);

            emit_expr(cg, node->children[0]);          /* left → %rax */
            emit(cg, "    subq    $16, %%rsp");        /* aligned spill slot */
            emit(cg, "    movq    %%rax, (%%rsp)");    /* save left */
            emit(cg, "    movq    %%rax, %%rdi");
            emit(cg, "    call    " XLY_SYM("xly_truthy"));        /* eax = 0|1 */
            emit(cg, "    testl   %%eax, %%eax");
            emit(cg, "    jz      %s", lbl_end);       /* falsy → keep left */
            /* truthy: discard left, eval right */
            emit(cg, "    addq    $16, %%rsp");        /* free spill slot */
            emit_expr(cg, node->children[1]);          /* right → %rax */
            emit(cg, "    jmp     %s", lbl_done);
            emit(cg, "%s:", lbl_end);                  /* falsy exit: left in spill slot */
            emit(cg, "    movq    (%%rsp), %%rax");    /* restore left */
            emit(cg, "    addq    $16, %%rsp");
            emit(cg, "%s:", lbl_done);
            break;
        }

        /* ── short-circuit: or ──
         * eval left → if truthy jump to end (result = left)
         * eval right → result = right                          */
        if (strcmp(op, "or") == 0) {
            char lbl_end[64], lbl_done[64];
            int seq = cg->label_seq++;
            snprintf(lbl_end,  sizeof(lbl_end),  ".Lxly_%d_or_end",  seq);
            snprintf(lbl_done, sizeof(lbl_done), ".Lxly_%d_or_done", seq);

            emit_expr(cg, node->children[0]);
            emit(cg, "    subq    $16, %%rsp");        /* aligned spill slot */
            emit(cg, "    movq    %%rax, (%%rsp)");    /* save left */
            emit(cg, "    movq    %%rax, %%rdi");
            emit(cg, "    call    " XLY_SYM("xly_truthy"));
            emit(cg, "    testl   %%eax, %%eax");
            emit(cg, "    jnz     %s", lbl_end);       /* truthy → keep left */
            emit(cg, "    addq    $16, %%rsp");        /* free spill slot */
            emit_expr(cg, node->children[1]);
            emit(cg, "    jmp     %s", lbl_done);
            emit(cg, "%s:", lbl_end);
            emit(cg, "    movq    (%%rsp), %%rax");    /* restore left */
            emit(cg, "    addq    $16, %%rsp");
            emit(cg, "%s:", lbl_done);
            break;
        }

        /* ── general binary op ──
         * OPTIMIZATION: For arithmetic ops (+,-,*,/), emit unboxed floating-point
         * ops to avoid boxing overhead. BUT: + is also string concat, so we need
         * a runtime type check before unboxing. */
        int is_arith = (strcmp(op,"+") == 0 || strcmp(op,"-") == 0 || 
                        strcmp(op,"*") == 0 || strcmp(op,"/") == 0);

        if (is_arith) {
            /* For +, we need to handle string concatenation.
             * Strategy: check if left operand is a string at runtime.
             * If yes, fall back to xly_add. Otherwise, use unboxed path. */
            int is_plus = (strcmp(op,"+") == 0);
            
            /* Handle special case: both operands are number literals */
            if (node->children[0]->type == NODE_NUMBER && 
                node->children[1]->type == NODE_NUMBER) {
                /* Ultra-fast path: compile-time constant folding */
                double left_val = node->children[0]->num_value;
                double right_val = node->children[1]->num_value;
                double result;
                if      (strcmp(op,"+") == 0) result = left_val + right_val;
                else if (strcmp(op,"-") == 0) result = left_val - right_val;
                else if (strcmp(op,"*") == 0) result = left_val * right_val;
                else                          result = left_val / right_val;
                
                uint64_t bits;
                memcpy(&bits, &result, sizeof(double));
                emit(cg, "    subq    $8, %%rsp");
                emit(cg, "    movabsq $%lu, %%rax", bits);
                emit(cg, "    movq    %%rax, (%%rsp)");
                emit(cg, "    movsd   (%%rsp), %%xmm0");
                emit(cg, "    addq    $8, %%rsp");
                emit(cg, "    call    " XLY_SYM("xly_num"));
            } else if (is_plus) {
                /* For +, check types at runtime and fall back to xly_add if needed */
                char lbl_slow[64], lbl_end[64];
                fresh_label(cg, lbl_slow, sizeof(lbl_slow));
                fresh_label(cg, lbl_end, sizeof(lbl_end));
                
                /* Eval both operands */
                emit_expr(cg, node->children[0]);
                emit(cg, "    subq    $16, %%rsp");     /* aligned spill for left */
                emit(cg, "    movq    %%rax, (%%rsp)"); /* save left */
                emit_expr(cg, node->children[1]);
                emit(cg, "    movq    %%rax, %%rsi");   /* rsi = right */
                emit(cg, "    movq    (%%rsp), %%rdi"); /* rdi = left */
                emit(cg, "    addq    $16, %%rsp");     /* free spill slot */
                
                /* Check if left is VAL_STRING (type == 1) */
                emit(cg, "    cmpl    $1, (%%rdi)");    /* check left.type == VAL_STRING */
                emit(cg, "    je      %s", lbl_slow);   /* if string, use slow path */
                
                /* Check if right is VAL_STRING */
                emit(cg, "    cmpl    $1, (%%rsi)");    /* check right.type == VAL_STRING */
                emit(cg, "    je      %s", lbl_slow);   /* if string, use slow path */
                
                /* Fast path: both are numbers, unbox and add */
                emit(cg, "    movsd   8(%%rdi), %%xmm0");
                emit(cg, "    movsd   8(%%rsi), %%xmm1");
                emit(cg, "    addsd   %%xmm1, %%xmm0");
                emit(cg, "    call    " XLY_SYM("xly_num"));
                emit(cg, "    jmp     %s", lbl_end);
                
                /* Slow path: call xly_add for string concat or mixed types */
                emit(cg, "%s:", lbl_slow);
                emit(cg, "    call    " XLY_SYM("xly_add"));
                
                emit(cg, "%s:", lbl_end);
            } else {
                /* For -, *, /: always use unboxed path (no string operations) */
                
                /* Eval left operand → %rax (XlyVal*) */
                emit_expr(cg, node->children[0]);
                emit(cg, "    subq    $16, %%rsp");     /* aligned spill for left */
                emit(cg, "    movq    %%rax, (%%rsp)"); /* save left boxed */
                
                /* Eval right operand → %rax (XlyVal*) */
                emit_expr(cg, node->children[1]);
                emit(cg, "    movq    %%rax, %%rsi");   /* rsi = right boxed */
                emit(cg, "    movq    (%%rsp), %%rdi"); /* rdi = left boxed */
                emit(cg, "    addq    $16, %%rsp");     /* free spill slot */
                
                /* Unbox both */
                emit(cg, "    movsd   8(%%rdi), %%xmm0");
                emit(cg, "    movsd   8(%%rsi), %%xmm1");
                
                /* Perform arithmetic */
                if      (strcmp(op,"-") == 0) emit(cg, "    subsd   %%xmm1, %%xmm0");
                else if (strcmp(op,"*") == 0) emit(cg, "    mulsd   %%xmm1, %%xmm0");
                else if (strcmp(op,"/") == 0) emit(cg, "    divsd   %%xmm1, %%xmm0");
                
                /* Box result */
                emit(cg, "    call    " XLY_SYM("xly_num"));
            }
        } else {
            /* ── Comparisons and other ops ─────────────────────────────────── */
            int is_comparison = (strcmp(op,"<") == 0 || strcmp(op,">") == 0 || 
                                strcmp(op,"<=") == 0 || strcmp(op,">=") == 0 ||
                                strcmp(op,"==") == 0 || strcmp(op,"!=") == 0);
            
            if (is_comparison) {
                /* OPTIMIZATION: Unboxed comparisons for numbers */
                if (node->children[0]->type == NODE_NUMBER && 
                    node->children[1]->type == NODE_NUMBER) {
                    /* Compile-time constant folding */
                    double left_val = node->children[0]->num_value;
                    double right_val = node->children[1]->num_value;
                    int result;
                    if      (strcmp(op,"<") == 0)  result = left_val < right_val;
                    else if (strcmp(op,">") == 0)  result = left_val > right_val;
                    else if (strcmp(op,"<=") == 0) result = left_val <= right_val;
                    else if (strcmp(op,">=") == 0) result = left_val >= right_val;
                    else if (strcmp(op,"==") == 0) result = left_val == right_val;
                    else                           result = left_val != right_val;
                    
                    emit(cg, "    movl    $%d, %%edi", result);
                    emit(cg, "    call    " XLY_SYM("xly_bool"));
                } else {
                    /* Runtime unboxed comparison */
                    char lbl_slow[64], lbl_end[64];
                    fresh_label(cg, lbl_slow, sizeof(lbl_slow));
                    fresh_label(cg, lbl_end, sizeof(lbl_end));
                    
                    emit_expr(cg, node->children[0]);
                    emit(cg, "    subq    $16, %%rsp");     /* aligned spill */
                    emit(cg, "    movq    %%rax, (%%rsp)"); /* save left */
                    emit_expr(cg, node->children[1]);
                    emit(cg, "    movq    %%rax, %%rsi");
                    emit(cg, "    movq    (%%rsp), %%rdi"); /* restore left */
                    emit(cg, "    addq    $16, %%rsp");
                    
                    /* Check if both are numbers */
                    emit(cg, "    cmpl    $0, (%%rdi)");
                    emit(cg, "    jne     %s", lbl_slow);
                    emit(cg, "    cmpl    $0, (%%rsi)");
                    emit(cg, "    jne     %s", lbl_slow);
                    
                    /* Fast path: unbox and compare */
                    emit(cg, "    movsd   8(%%rdi), %%xmm0");
                    emit(cg, "    movsd   8(%%rsi), %%xmm1");
                    emit(cg, "    ucomisd %%xmm1, %%xmm0");
                    
                    if (strcmp(op,"<") == 0)       emit(cg, "    setb    %%al");
                    else if (strcmp(op,">") == 0)  emit(cg, "    seta    %%al");
                    else if (strcmp(op,"<=") == 0) emit(cg, "    setbe   %%al");
                    else if (strcmp(op,">=") == 0) emit(cg, "    setae   %%al");
                    else if (strcmp(op,"==") == 0) emit(cg, "    sete    %%al");
                    else                           emit(cg, "    setne   %%al");
                    
                    emit(cg, "    movzbl  %%al, %%edi");
                    emit(cg, "    call    " XLY_SYM("xly_bool"));
                    emit(cg, "    jmp     %s", lbl_end);
                    
                    /* Slow path */
                    emit(cg, "%s:", lbl_slow);
                    const char *fn = NULL;
                    if      (strcmp(op,"==") == 0) fn = XLY_SYM("xly_eq");
                    else if (strcmp(op,"!=") == 0) fn = XLY_SYM("xly_neq");
                    else if (strcmp(op,"<")  == 0) fn = XLY_SYM("xly_lt");
                    else if (strcmp(op,">")  == 0) fn = XLY_SYM("xly_gt");
                    else if (strcmp(op,"<=") == 0) fn = XLY_SYM("xly_lte");
                    else if (strcmp(op,">=") == 0) fn = XLY_SYM("xly_gte");
                    if (fn) emit(cg, "    call    %s", fn);
                    emit(cg, "%s:", lbl_end);
                }
            } else {
                /* Slow path for other ops */
                emit_expr(cg, node->children[0]);
                emit(cg, "    subq    $16, %%rsp");     /* aligned spill */
                emit(cg, "    movq    %%rax, (%%rsp)"); /* save left */
                emit_expr(cg, node->children[1]);
                emit(cg, "    movq    %%rax, %%rsi");
                emit(cg, "    movq    (%%rsp), %%rdi"); /* restore left */
                emit(cg, "    addq    $16, %%rsp");

                const char *fn = NULL;
                if (strcmp(op,"%") == 0) fn = XLY_SYM("xly_mod");
                if (fn) emit(cg, "    call    %s", fn);
            }
        }
        break;
    }

    /* ── unary ─────────────────────────────────────────────────────── */
    case NODE_UNARY:
        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %%rdi");
        emit(cg, "    call    %s",
             strcmp(node->str_value, "-") == 0 ? XLY_SYM("xly_neg") : XLY_SYM("xly_not"));
        break;

    /* ── user function call ──────────────────────────────────────────
     * If the callee name is a declared fn → direct call .Lxly_fn_<name>
     * If the callee name is a variable (holding a VAL_FUNCTION XlyVal*)
     *   → load from stack, extract .fn ptr at offset 32, indirect call.
     * Args are eval'd left-to-right, spilled, then loaded into SysV regs. */
    case NODE_FN_CALL: {
        int nargs = (int)node->child_count;
        static const char *regs[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        int nreg = nargs < 6 ? nargs : 6;  /* args 0-5 → registers       */
        int nstk = nargs - nreg;            /* args 6+  → stack (SysV ABI) */

        /* ── enum variant constructor ───────────────────────────────────────
         * If the callee name is a known variant, emit xly_make_variant call. */
        if (node->str_value && variant_find(cg, node->str_value) >= 0) {
            /* Build args array on stack, call xly_make_variant(tag, arr, n) */
            int arr_bytes = nargs > 0 ? (((nargs * 8) + 15) & ~15) : 0;
            if (arr_bytes > 0)
                emit(cg, "    subq    $%d, %%rsp", arr_bytes);
            for (int i = 0; i < nargs; i++) {
                emit_expr(cg, node->children[i]);
                emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
            }
            /* rdi = tag string (intern the variant name as a string literal) */
            const char *tag_lbl = intern_string(cg, node->str_value);
            emit(cg, "    leaq    %s(%%rip), %%rdi", tag_lbl);
            emit(cg, "    call    " XLY_SYM("xly_str"));
            emit(cg, "    movq    %%rax, %%rdi");         /* rdi = tag XlyVal* */
            if (nargs > 0)
                emit(cg, "    movq    %%rsp, %%rsi");     /* rsi = fields ptr  */
            else
                emit(cg, "    xorq    %%rsi, %%rsi");
            emit(cg, "    movl    $%d, %%edx", nargs);   /* edx = nfields     */
            emit(cg, "    call    " XLY_SYM("xly_make_variant"));
            if (arr_bytes > 0)
                emit(cg, "    addq    $%d, %%rsp", arr_bytes);
            break;
        }

        /* ── variable-fn path: use xly_call_fnval (all args in array) ─────
         * Handles any number of args cleanly via the args array.           */
        int var_off = var_offset(cg, node->str_value);
        int gi_fn   = (var_off == 0) ? gvar_find(cg, node->str_value) : -1;
        int is_var_fn = !fn_is_known(cg, node->str_value)
                        && (var_off != 0 || gi_fn >= 0);

        if (is_var_fn) {
            int arr_bytes = nargs > 0 ? (((nargs * 8) + 15) & ~15) : 0;
            if (arr_bytes > 0)
                emit(cg, "    subq    $%d, %%rsp", arr_bytes);
            for (int i = 0; i < nargs; i++) {
                emit_expr(cg, node->children[i]);
                emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
            }
            if (var_off != 0)
                emit(cg, "    movq    %d(%%rbp), %%rdi", var_off);
            else
                emit(cg, "    movq    " XLY_SYM("__xly_globals") "+%d(%%rip), %%rdi", gi_fn * 8);
            if (nargs > 0)
                emit(cg, "    movq    %%rsp, %%rsi");
            else
                emit(cg, "    xorq    %%rsi, %%rsi");
            emit(cg, "    movl    $%d, %%edx", nargs);
            emit(cg, "    call    " XLY_SYM("xly_call_fnval"));
            if (arr_bytes > 0)
                emit(cg, "    addq    $%d, %%rsp", arr_bytes);
            break;
        }

        if (!fn_is_known(cg, node->str_value)) {
            emit(cg, "    call    " XLY_SYM("xly_null"));
            break;
        }

        /* ── declared fn: SysV x86-64 ABI ──────────────────────────────────
         * Args 0-5 → rdi,rsi,rdx,rcx,r8,r9
         * Args 6+  → [rsp], [rsp+8], ...  (callee reads at [rbp+16], ...)
         *
         * We evaluate all args into a staging area first, then:
         *  1. Copy staging[nreg..nargs-1] to positions above staging
         *     (these become the stack args once staging is freed)
         *  2. Load staging[0..nreg-1] into the register args
         *  3. Free staging (stack args slide into place at rsp)
         *  4. call
         *  5. Caller cleans up stack args (SysV: caller-cleanup for stack args)
         */
        int stage_bytes = nargs > 0 ? (((nargs * 8) + 15) & ~15) : 0;
        int stk_bytes   = nstk > 0  ? (((nstk * 8)  + 15) & ~15) : 0;

        /* Evaluate all args into staging */
        if (stage_bytes > 0)
            emit(cg, "    subq    $%d, %%rsp", stage_bytes);
        for (int i = 0; i < nargs; i++) {
            emit_expr(cg, node->children[i]);
            emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
        }

        /* Pre-place stack args above the staging area (using r10 as scratch).
         * After addq $stage_bytes these will be at [rsp+0], [rsp+8], ... */
        for (int i = 0; i < nstk; i++) {
            emit(cg, "    movq    %d(%%rsp), %%r10", (nreg + i) * 8);
            emit(cg, "    movq    %%r10, %d(%%rsp)", stage_bytes + i * 8);
        }

        /* Load register args from staging */
        for (int i = 0; i < nreg; i++)
            emit(cg, "    movq    %d(%%rsp), %%%s", i * 8, regs[i]);

        /* Free staging — stack args now at [rsp..] */
        if (stage_bytes > 0)
            emit(cg, "    addq    $%d, %%rsp", stage_bytes);

        emit(cg, "    call    .Lxly_fn_%s", fn_resolve_label(cg, node->str_value));

        /* Caller cleans up stack args */
        if (stk_bytes > 0)
            emit(cg, "    addq    $%d, %%rsp", stk_bytes);

        break;
    }

    /* ── module method call  (mod.fn(args…)) ─────────────────────────
     * Signature: xly_call_module(const char *mod, const char *fn,
     *                            XlyVal **args, size_t argc)
     *            rdi             rsi             rdx              rcx
     *
     * SYSTEMS PROGRAMMING OPTIMIZATIONS (xenlyc v0.1.0):
     *   O2: sys.CONSTANT() zero-arg calls → inlined immediate (no call)
     *   O1: sys.exit/abort detected → noreturn_mode set for dead-code elim
     */
    case NODE_METHOD_CALL: {
        const char *mod_name = node->children[0]->str_value;
        const char *fn_name  = node->str_value;
        int argc = (int)node->child_count - 1;

        /* ── Detect local-variable object method call ───────────────────────
         * If children[0] is an identifier that resolves to a local variable
         * (not a module), treat this as obj.method(args) via xly_obj_call.
         * xly_obj_call(obj, method, args_array, argc)                         */
        int obj_var_off = (node->children[0]->type == NODE_IDENTIFIER)
                          ? var_offset(cg, mod_name) : 0;
        /* Also handle chained expressions: if children[0] is not a simple
         * identifier, always eval it and use obj_call. */
        int is_obj_call = (obj_var_off != 0) ||
                          (node->children[0]->type != NODE_IDENTIFIER);

        if (is_obj_call) {
            /* Build args array on stack */
            int arr_bytes = argc > 0 ? (((argc * 8) + 15) & ~15) : 0;
            if (arr_bytes > 0)
                emit(cg, "    subq    $%d, %%rsp", arr_bytes);
            for (int i = 0; i < argc; i++) {
                emit_expr(cg, node->children[i + 1]);
                emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
            }
            /* Eval the object expression */
            emit_expr(cg, node->children[0]);
            emit(cg, "    movq    %%rax, %%rdi");           /* obj */
            const char *mfl = intern_string(cg, fn_name);
            emit(cg, "    leaq    %s(%%rip), %%rsi", mfl); /* method name */
            if (argc > 0)
                emit(cg, "    movq    %%rsp, %%rdx");       /* args array */
            else
                emit(cg, "    xorq    %%rdx, %%rdx");
            emit(cg, "    movl    $%d, %%ecx", argc);
            emit(cg, "    call    " XLY_SYM("xly_obj_call"));
            if (arr_bytes > 0)
                emit(cg, "    addq    $%d, %%rsp", arr_bytes);
            break;
        }

        /* ── O2+: sys constant inlining ────────────────────────────────────
         * Zero-arg sys functions returning compile-time constants are inlined
         * as immediate loads — eliminates call overhead in tight sys loops.  */
        if (cg->opt_level >= 2 && strcmp(mod_name, "sys") == 0 && argc == 0) {
            static const struct { const char *name; long long val; } SC[] = {
                /* Standard file descriptors */
                {"STDIN",0},{"STDOUT",1},{"STDERR",2},
                /* Open flags (Linux x86-64) */
                {"O_RDONLY",0},{"O_WRONLY",1},{"O_RDWR",2},
                {"O_CREAT",64},{"O_TRUNC",512},{"O_APPEND",1024},
                {"O_NONBLOCK",2048},{"O_SYNC",1052672},{"O_EXCL",128},
                /* Signals */
                {"SIGHUP",1},{"SIGINT",2},{"SIGQUIT",3},{"SIGKILL",9},
                {"SIGTERM",15},{"SIGUSR1",10},{"SIGUSR2",12},
                {"SIGCHLD",17},{"SIGPIPE",13},{"SIGALRM",14},{"SIGSEGV",11},
                /* Syslog levels */
                {"LOG_EMERG",0},{"LOG_ALERT",1},{"LOG_CRIT",2},
                {"LOG_ERR",3},{"LOG_WARNING",4},{"LOG_NOTICE",5},
                {"LOG_INFO",6},{"LOG_DEBUG",7},
                /* mmap prot/map flags */
                {"PROT_READ",1},{"PROT_WRITE",2},{"PROT_EXEC",4},
                {"PROT_NONE",0},{"MAP_SHARED",1},{"MAP_PRIVATE",2},
                {"MAP_ANONYMOUS",32},
                /* Socket / address family */
                {"AF_INET",2},{"AF_INET6",10},{"AF_UNIX",1},
                {"SOCK_STREAM",1},{"SOCK_DGRAM",2},
                /* Poll events */
                {"POLLIN",1},{"POLLOUT",4},{"POLLERR",8},{"POLLHUP",16},
                /* Resource limits */
                {"RLIMIT_CPU",0},{"RLIMIT_FSIZE",1},
                {"RLIMIT_STACK",3},{"RLIMIT_NOFILE",7},{"RLIMIT_AS",9},
                /* Integer limits */
                {"INT8_MAX",127},{"INT8_MIN",-128},
                {"INT16_MAX",32767},{"INT16_MIN",-32768},
                {"INT32_MAX",2147483647LL},{"INT32_MIN",-2147483648LL},
                {"UINT8_MAX",255},{"UINT16_MAX",65535},
                {"UINT32_MAX",4294967295LL},
                {"INT_MAX",2147483647LL},{"INT_MIN",-2147483648LL},
                {"UINT_MAX",4294967295LL},
                {NULL,0}
            };
            for (int ci = 0; SC[ci].name; ci++) {
                if (strcmp(fn_name, SC[ci].name) == 0) {
                    if (cg->verbose)
                        emit(cg, "    # [xenlyc] inline sys.%s() = %lld",
                             fn_name, SC[ci].val);
                    emit_load_double(cg, (double)SC[ci].val);
                    emit(cg, "    call    " XLY_SYM("xly_num"));
                    cg->stat_sys_inline++;
                    goto x86_method_done;
                }
            }
        }

        /* ── O1+: noreturn detection ────────────────────────────────────────
         * sys.exit() and sys.abort() do not return — set noreturn_mode so
         * the enclosing statement compiler skips subsequent dead code.       */
        int is_noreturn = (cg->opt_level >= 1 &&
                           strcmp(mod_name, "sys") == 0 &&
                           (strcmp(fn_name, "exit") == 0 ||
                            strcmp(fn_name, "abort") == 0));

        /* ── general dispatch via xly_call_module ───────────────────────── */
        {
            int slot_bytes  = argc * 8;
            int alloc_bytes = argc > 0 ? ((slot_bytes + 15) & ~15) : 0;

            if (alloc_bytes > 0)
                emit(cg, "    subq    $%d, %%rsp", alloc_bytes);

            for (int i = 0; i < argc; i++) {
                emit_expr(cg, node->children[i + 1]);
                emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
            }

            if (argc > 0)
                emit(cg, "    movq    %%rsp, %%rdx");
            else
                emit(cg, "    xorq    %%rdx, %%rdx");

            const char *ml = intern_string(cg, mod_name);
            const char *fl = intern_string(cg, fn_name);
            emit(cg, "    leaq    %s(%%rip), %%rdi", ml);
            emit(cg, "    leaq    %s(%%rip), %%rsi", fl);
            emit(cg, "    movl    $%d, %%ecx", argc);
            emit(cg, "    call    " XLY_SYM("xly_call_module"));

            if (alloc_bytes > 0)
                emit(cg, "    addq    $%d, %%rsp", alloc_bytes);

            if (is_noreturn) {
                if (cg->verbose)
                    emit(cg, "    # [xenlyc] noreturn: dead code elision after this point");
                cg->noreturn_mode = 1;
                cg->stat_noreturn_elim++;
            }
        }

        x86_method_done:;
        break;
    }

    /* ── typeof ────────────────────────────────────────────────────── */
    case NODE_TYPEOF:
        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %%rdi");
        emit(cg, "    call    " XLY_SYM("xly_typeof"));
        break;

    /* ── increment / decrement as expression ───────────────────────── */
    case NODE_INCREMENT:
    case NODE_DECREMENT: {
        int off = var_offset(cg, node->str_value);
        /* load current value — aligned spill across xly_num call */
        emit(cg, "    movq    %d(%%rbp), %%rax", off);
        emit(cg, "    subq    $16, %%rsp");       /* aligned spill slot */
        emit(cg, "    movq    %%rax, (%%rsp)");   /* save current */
        emit_load_double(cg, 1.0);
        emit(cg, "    call    " XLY_SYM("xly_num"));          /* rax = num(1) */
        emit(cg, "    movq    %%rax, %%rsi");     /* rsi = num(1) */
        emit(cg, "    movq    (%%rsp), %%rdi");   /* rdi = current value */
        emit(cg, "    addq    $16, %%rsp");
        emit(cg, "    call    %s",
             node->type == NODE_INCREMENT ? XLY_SYM("xly_add") : XLY_SYM("xly_sub"));
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        break;
    }

    /* ── array literal [expr, expr, ...] ───────────────────────────── */
    case NODE_ARRAY_LITERAL: {
        int nelems = (int)node->child_count;

        /* Use sub+store (not push) so alignment is deterministic.
         * xly_array_create(XlyVal **elements, size_t count): rdi, rsi */
        int alloc_bytes = nelems > 0 ? (((nelems * 8) + 15) & ~15) : 0;

        if (alloc_bytes > 0)
            emit(cg, "    subq    $%d, %%rsp", alloc_bytes);

        /* Eval each element left-to-right, store at rsp[i*8] */
        for (int i = 0; i < nelems; i++) {
            emit_expr(cg, node->children[i]);
            emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
        }

        if (nelems > 0)
            emit(cg, "    movq    %%rsp, %%rdi");
        else
            emit(cg, "    xorq    %%rdi, %%rdi");  /* NULL for empty */
        emit(cg, "    movq    $%d, %%rsi", nelems);
        emit(cg, "    call    " XLY_SYM("xly_array_create"));

        if (alloc_bytes > 0)
            emit(cg, "    addq    $%d, %%rsp", alloc_bytes);
        break;
    }

    /* ── array/string indexing arr[index] ──────────────────────────── */
    case NODE_INDEX: {
        /* children[0] = array/string, children[1] = index */
        emit_expr(cg, node->children[0]);  /* array in rax */
        emit(cg, "    subq    $16, %%rsp");     /* aligned spill */
        emit(cg, "    movq    %%rax, (%%rsp)"); /* save array */
        emit_expr(cg, node->children[1]);  /* index in rax */
        emit(cg, "    movq    %%rax, %%rsi");   /* index to rsi */
        emit(cg, "    movq    (%%rsp), %%rdi"); /* array to rdi */
        emit(cg, "    addq    $16, %%rsp");
        emit(cg, "    call    " XLY_SYM("xly_index"));
        break;
    }

    /* ── object literal  { key: val, key: val, ... } ───────────────── */
    case NODE_OBJECT_LITERAL: {
        /* Parser produces: NODE_OBJECT_LITERAL
         *   children[i] = NODE_NAMED_ARG { str_value=key, children[0]=val_expr }
         * Create empty object, then set each field.                          */
        emit(cg, "    call    " XLY_SYM("xly_obj_new"));
        /* Spill obj ptr onto stack */
        emit(cg, "    subq    $16, %%rsp");
        emit(cg, "    movq    %%rax, (%%rsp)");
        int npairs = (int)node->child_count;
        for (int i = 0; i < npairs; i++) {
            ASTNode *pair = node->children[i];    /* NODE_NAMED_ARG */
            const char *key_str = pair->str_value;
            ASTNode *val_node   = pair->child_count > 0 ? pair->children[0] : NULL;
            const char *kl = intern_string(cg, key_str);
            /* Eval value; spill obj across the eval */
            if (val_node)
                emit_expr(cg, val_node);
            else
                emit(cg, "    call    " XLY_SYM("xly_null"));
            emit(cg, "    movq    %%rax, %%rdx");          /* rdx = value */
            emit(cg, "    movq    (%%rsp), %%rdi");        /* rdi = obj */
            emit(cg, "    leaq    %s(%%rip), %%rsi", kl); /* rsi = key */
            emit(cg, "    call    " XLY_SYM("xly_obj_set"));
        }
        emit(cg, "    movq    (%%rsp), %%rax");  /* return obj in rax */
        emit(cg, "    addq    $16, %%rsp");
        break;
    }

    /* ── property get  obj.field ──────────────────────────────────── */
    case NODE_PROPERTY_GET: {
        /* children[0] = object expression, str_value = field name */
        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %%rdi");
        const char *fl = intern_string(cg, node->str_value);
        emit(cg, "    leaq    %s(%%rip), %%rsi", fl);
        emit(cg, "    call    " XLY_SYM("xly_obj_get"));
        break;
    }

    /* ── property set  obj.field = val  (as expression) ─────────────
     * children[0] = object, children[1] = value; str_value = field   */
    case NODE_PROPERTY_SET: {
        emit_expr(cg, node->children[0]);
        emit(cg, "    subq    $16, %%rsp");
        emit(cg, "    movq    %%rax, (%%rsp)");   /* spill obj */
        emit_expr(cg, node->children[1]);
        emit(cg, "    movq    %%rax, %%rdx");     /* rdx = val */
        emit(cg, "    movq    (%%rsp), %%rdi");   /* rdi = obj */
        emit(cg, "    addq    $16, %%rsp");
        const char *fl2 = intern_string(cg, node->str_value);
        emit(cg, "    leaq    %s(%%rip), %%rsi", fl2);
        emit(cg, "    call    " XLY_SYM("xly_obj_set"));
        /* return the object (rdi) for chaining */
        emit(cg, "    movq    %%rdi, %%rax");
        break;
    }

    /* ── await expr ─────────────────────────────────────────────────────
     * await fn_call  — synchronous in current impl: just evaluate the call.
     * children[0] = the call expression (NODE_FN_CALL or NODE_CALL_EXPR).  */
    case NODE_AWAIT:
        emit_expr(cg, node->children[0]);
        break;

    /* ── spawn expr ─────────────────────────────────────────────────────
     * spawn fn_call  — evaluate the call for side effects, discard result.
     * In the compiled runtime there is no scheduler; spawn executes the
     * function synchronously (matching the interpreter "preview" semantics).
     * Result ends up in %rax but the expression produces null.             */
    case NODE_SPAWN:
        emit_expr(cg, node->children[0]);   /* execute fn call, result in %rax */
        emit(cg, "    call    " XLY_SYM("xly_null"));  /* spawn → null */
        break;

    /* ── fallback ──────────────────────────────────────────────────── */
    default:
        emit(cg, "    call    " XLY_SYM("xly_null"));
        break;
    }
}

/* ── statement compiler (x86-64) — Post-condition: %%rsp unchanged from entry.
 * If noreturn_mode is set (after sys.exit/abort), all subsequent statements
 * are elided — the code is unreachable and emitting it wastes binary space.  */
static void emit_stmt(CG *cg, ASTNode *node) {
    if (!node) return;
    /* Dead code elimination after noreturn calls (O1+) */
    if (cg->noreturn_mode) {
        cg->stat_noreturn_elim++;
        return;
    }

    switch (node->type) {

    /* ── var/const decl ────────────────────────────────────────────── */
    case NODE_VAR_DECL:
    case NODE_CONST_DECL: {
        if (node->child_count > 0)
            emit_expr(cg, node->children[0]);
        else
            emit(cg, "    call    " XLY_SYM("xly_null"));
        if (cg->in_main_scope && cg->scope_depth == 0) {
            /* Top-level decl: store in global slot so functions can read it */
            int gi = gvar_declare(cg, node->str_value);
            emit(cg, "    movq    %%rax, " XLY_SYM("__xly_globals") "+%d(%%rip)", gi * 8);
            /* Also declare a local alias for use within main() */
            int off = var_declare(cg, node->str_value);
            emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        } else {
            int off = var_declare(cg, node->str_value);
            emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        }
        break;
    }

    /* ── assign  (str_value = varname, children[0] = rhs) ─────────── */
    case NODE_ASSIGN: {
        emit_expr(cg, node->children[0]);
        int off = var_offset(cg, node->str_value);
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        break;
    }

    /* ── compound assign  (str_value = op like "+=",
     *                       children[0] = IDENT target,
     *                       children[1] = rhs expression)            ── */
    case NODE_COMPOUND_ASSIGN: {
        const char *varname = node->children[0]->str_value;
        int off = var_offset(cg, varname);

        /* load current value, spill across rhs eval — aligned slot */
        emit(cg, "    movq    %d(%%rbp), %%rax", off);
        emit(cg, "    subq    $16, %%rsp");        /* aligned spill */
        emit(cg, "    movq    %%rax, (%%rsp)");    /* save current */
        emit_expr(cg, node->children[1]);          /* rhs → %rax */
        emit(cg, "    movq    %%rax, %%rsi");      /* rsi = rhs */
        emit(cg, "    movq    (%%rsp), %%rdi");    /* rdi = current */
        emit(cg, "    addq    $16, %%rsp");

        const char *op  = node->str_value;         /* "+=", "-=", "*=", "/=" */
        const char *fn  = XLY_SYM("xly_add");
        if      (strcmp(op, "+=") == 0) fn = XLY_SYM("xly_add");
        else if (strcmp(op, "-=") == 0) fn = XLY_SYM("xly_sub");
        else if (strcmp(op, "*=") == 0) fn = XLY_SYM("xly_mul");
        else if (strcmp(op, "/=") == 0) fn = XLY_SYM("xly_div");

        emit(cg, "    call    %s", fn);
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        break;
    }

    /* ── increment / decrement as statement ────────────────────────── */
    case NODE_INCREMENT:
    case NODE_DECREMENT: {
        int off = var_offset(cg, node->str_value);
        emit(cg, "    movq    %d(%%rbp), %%rax", off);
        emit(cg, "    subq    $16, %%rsp");        /* aligned spill */
        emit(cg, "    movq    %%rax, (%%rsp)");    /* save current */
        emit_load_double(cg, 1.0);
        emit(cg, "    call    " XLY_SYM("xly_num"));
        emit(cg, "    movq    %%rax, %%rsi");      /* rsi = num(1) */
        emit(cg, "    movq    (%%rsp), %%rdi");    /* rdi = current */
        emit(cg, "    addq    $16, %%rsp");
        emit(cg, "    call    %s",
             node->type == NODE_INCREMENT ? XLY_SYM("xly_add") : XLY_SYM("xly_sub"));
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        break;
    }

    /* ── print ────────────────────────────────────────────────────── */
    case NODE_PRINT: {
        /*
         * xly_print(XlyVal **vals, size_t n)   — rdi, rsi
         *
         * Use sub+store (not push) so alignment is deterministic.
         * alloc_bytes = ((n*8) + 15) & ~15 keeps rsp 16-aligned.
         */
        int n = (int)node->child_count;
        int alloc_bytes = n > 0 ? (((n * 8) + 15) & ~15) : 0;

        if (alloc_bytes > 0)
            emit(cg, "    subq    $%d, %%rsp", alloc_bytes);

        /* Eval each arg left-to-right, store at rsp[i*8] */
        for (int i = 0; i < n; i++) {
            emit_expr(cg, node->children[i]);
            emit(cg, "    movq    %%rax, %d(%%rsp)", i * 8);
        }

        emit(cg, "    movq    %%rsp, %%rdi");
        emit(cg, "    movl    $%d, %%esi", n);
        emit(cg, "    call    " XLY_SYM("xly_print"));

        if (alloc_bytes > 0)
            emit(cg, "    addq    $%d, %%rsp", alloc_bytes);
        break;
    }

    /* ── expression statement ─────────────────────────────────────── */
    case NODE_EXPR_STMT:
        if (node->child_count > 0)
            emit_expr(cg, node->children[0]);
        break;

    /* ── if / else if / else ──────────────────────────────────────── */
    case NODE_IF: {
        char lbl_else[64], lbl_end[64];
        fresh_label(cg, lbl_else, sizeof(lbl_else));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %%rdi");
        emit(cg, "    call    " XLY_SYM("xly_truthy"));
        emit(cg, "    testl   %%eax, %%eax");
        emit(cg, "    jz      %s", lbl_else);

        emit_stmt(cg, node->children[1]);          /* then */
        emit(cg, "    jmp     %s", lbl_end);

        emit(cg, "%s:", lbl_else);
        if (node->child_count > 2)
            emit_stmt(cg, node->children[2]);      /* else / else-if */

        emit(cg, "%s:", lbl_end);
        break;
    }

    /* ── while ────────────────────────────────────────────────────── */
    case NODE_WHILE: {
        char lbl_cond[64], lbl_end[64];
        fresh_label(cg, lbl_cond, sizeof(lbl_cond));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        push_brk(cg, lbl_end);
        push_cnt(cg, lbl_cond);

        emit(cg, "%s:", lbl_cond);
        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %%rdi");
        emit(cg, "    call    " XLY_SYM("xly_truthy"));
        emit(cg, "    testl   %%eax, %%eax");
        emit(cg, "    jz      %s", lbl_end);

        emit_stmt(cg, node->children[1]);
        emit(cg, "    jmp     %s", lbl_cond);

        emit(cg, "%s:", lbl_end);
        pop_cnt(cg);
        pop_brk(cg);
        break;
    }

    /* ── for (C-style)  children: [0]=init [1]=cond [2]=update [3]=body ── */
    case NODE_FOR: {
        char lbl_cond[64], lbl_upd[64], lbl_end[64];
        fresh_label(cg, lbl_cond, sizeof(lbl_cond));
        fresh_label(cg, lbl_upd,  sizeof(lbl_upd));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        scope_enter(cg);
        push_brk(cg, lbl_end);
        push_cnt(cg, lbl_upd);

        emit_stmt(cg, node->children[0]);          /* init */

        emit(cg, "%s:", lbl_cond);
        /* if cond is literal true (parser default), skip the check */
        if (!(node->children[1]->type == NODE_BOOL && node->children[1]->bool_value)) {
            emit_expr(cg, node->children[1]);
            emit(cg, "    movq    %%rax, %%rdi");
            emit(cg, "    call    " XLY_SYM("xly_truthy"));
            emit(cg, "    testl   %%eax, %%eax");
            emit(cg, "    jz      %s", lbl_end);
        }

        emit_stmt(cg, node->children[3]);          /* body */

        emit(cg, "%s:", lbl_upd);
        if (node->children[2]->type != NODE_NULL)
            emit_stmt(cg, node->children[2]);      /* update */

        emit(cg, "    jmp     %s", lbl_cond);
        emit(cg, "%s:", lbl_end);

        pop_cnt(cg);
        pop_brk(cg);
        scope_leave(cg);
        break;
    }

    /* ── for-in  str_value=itervar, children[0]=iterable, [1]=body ── */
    case NODE_FOR_IN: {
        char lbl_cond[64], lbl_end[64];
        fresh_label(cg, lbl_cond, sizeof(lbl_cond));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        scope_enter(cg);

        /* declare: itervar + 3 hidden slots (arr, idx, len) */
        int off_iter = var_declare(cg, node->str_value);
        char tmp[80];
        snprintf(tmp, sizeof(tmp), "__fi_a_%d", cg->label_seq);
        int off_arr  = var_declare(cg, tmp);
        snprintf(tmp, sizeof(tmp), "__fi_i_%d", cg->label_seq);
        int off_idx  = var_declare(cg, tmp);
        snprintf(tmp, sizeof(tmp), "__fi_l_%d", cg->label_seq++);
        int off_len  = var_declare(cg, tmp);

        /* eval iterable */
        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %d(%%rbp)", off_arr);
        /* len = xly_array_len(arr) */
        emit(cg, "    movq    %%rax, %%rdi");
        emit(cg, "    call    " XLY_SYM("xly_array_len"));
        emit(cg, "    movq    %%rax, %d(%%rbp)", off_len);
        /* idx = 0 */
        emit(cg, "    movq    $0, %d(%%rbp)", off_idx);

        push_brk(cg, lbl_end);
        push_cnt(cg, lbl_cond);

        emit(cg, "%s:", lbl_cond);
        emit(cg, "    movq    %d(%%rbp), %%rax", off_idx);
        emit(cg, "    cmpq    %d(%%rbp), %%rax", off_len);
        emit(cg, "    jae     %s", lbl_end);

        /* itervar = array_get(arr, idx) */
        emit(cg, "    movq    %d(%%rbp), %%rdi", off_arr);
        emit(cg, "    movq    %d(%%rbp), %%rsi", off_idx);
        emit(cg, "    call    " XLY_SYM("xly_array_get"));
        emit(cg, "    movq    %%rax, %d(%%rbp)", off_iter);

        emit_stmt(cg, node->children[1]);          /* body */

        /* idx++ */
        emit(cg, "    movq    %d(%%rbp), %%rax", off_idx);
        emit(cg, "    addq    $1, %%rax");
        emit(cg, "    movq    %%rax, %d(%%rbp)", off_idx);
        emit(cg, "    jmp     %s", lbl_cond);
        emit(cg, "%s:", lbl_end);

        pop_cnt(cg);
        pop_brk(cg);
        scope_leave(cg);
        break;
    }

    /* ── break / continue ──────────────────────────────────────────── */
    case NODE_BREAK:
        if (cg->brk_top > 0)
            emit(cg, "    jmp     %s", cg->brk_labels[cg->brk_top-1]);
        break;
    case NODE_CONTINUE:
        if (cg->cnt_top > 0)
            emit(cg, "    jmp     %s", cg->cnt_labels[cg->cnt_top-1]);
        break;

    /* ── block ─────────────────────────────────────────────────────── */
    case NODE_BLOCK:
        scope_enter(cg);
        for (size_t i = 0; i < node->child_count; i++)
            emit_stmt(cg, node->children[i]);
        scope_leave(cg);
        break;

    /* ── fn decl — stash for emission after main ─────────────────── */
    case NODE_FN_DECL: {
        /* Allocate a globally-unique asm label for this function body.
         * If a function with this name was already emitted (e.g. same name
         * reused in a different inner scope), the label gets a numeric suffix
         * like "add__2" so the assembler never sees duplicate definitions.   */
        char ulbl[256];
        fn_unique_label(cg, node->str_value, ulbl, sizeof(ulbl));
        /* Push scope-local rename so call sites inside THIS body resolve
         * to the correct (possibly renamed) label.                           */
        fn_rename_push(cg, node->str_value, ulbl);
        if (cg->func_count >= cg->func_cap) {
            cg->func_cap = cg->func_cap ? cg->func_cap * 2 : 16;
            cg->funcs    = realloc(cg->funcs, sizeof(cg->funcs[0]) * (size_t)cg->func_cap);
        }
        { int fi = cg->func_count++;
          cg->funcs[fi].node      = node;
          cg->funcs[fi].captures  = NULL;
          cg->funcs[fi].ncaptures = 0;
          cg->funcs[fi].asm_label = strdup(ulbl);
          /* Snapshot the current rename table so that when this function's
           * body is emitted later (after the outer scope has been cleaned up),
           * peer functions declared in the same scope are still resolvable.   */
          fn_rename_ctx_snapshot(cg, &cg->funcs[fi].rename_ctx, &cg->funcs[fi].rename_ctx_count); }
        break;
    }

    /* ── return ────────────────────────────────────────────────────── */
    case NODE_RETURN:
        if (node->child_count > 0)
            emit_expr(cg, node->children[0]);
        else
            emit(cg, "    call    " XLY_SYM("xly_null"));
        emit(cg, "    movq    %%rbp, %%rsp");
        emit(cg, "    popq    %%rbp");
        emit(cg, "    ret");
        break;

    /* ── import — no-op (stdlib linked statically) ────────────────── */
    case NODE_IMPORT:
        break;

    /* ── enum declaration — register variants, create parameterless ones ── */
    case NODE_ENUM_DECL:
        /* Variant names already registered in variant table by pre-pass.
         * For parameterless variants, emit a global holding the singleton
         * VAL_ENUM_VARIANT value so NODE_IDENTIFIER can load it.
         * Parametric variants are constructed at call-site via xly_make_variant. */
        for (size_t vi = 0; vi < node->child_count; vi++) {
            ASTNode *vn = node->children[vi];
            if (!vn || !vn->str_value) continue;
            if (vn->param_count == 0) {
                /* Create the singleton and store in a global */
                const char *tag_label = intern_string(cg, vn->str_value);
                emit(cg, "    leaq    %s(%%rip), %%rdi", tag_label);
                emit(cg, "    call    " XLY_SYM("xly_str"));
                emit(cg, "    movq    %%rax, %%rdi");
                emit(cg, "    xorq    %%rsi, %%rsi");   /* fields = NULL */
                emit(cg, "    xorl    %%edx, %%edx");   /* nfields = 0   */
                emit(cg, "    call    " XLY_SYM("xly_make_variant"));
                /* Store in global slot so identifier lookups find it */
                int gi = gvar_declare(cg, vn->str_value);
                emit(cg, "    movq    %%rax, " XLY_SYM("__xly_globals") "+%d(%%rip)", gi * 8);
            }
            /* Parametric variants: nothing to emit here; call-sites use xly_make_variant */
        }
        break;

    /* ── everything else — try as expression ─────────────────────── */
    default:
        emit_expr(cg, node);
        break;
    }
}

/* ── emit a user-defined function ──────────────────────────────────────── */
static void emit_function(CG *cg, ASTNode *fn, const char *asm_label,
                          FnRename *rename_ctx, int rename_ctx_count,
                          char **captures, int ncaptures) {
    size_t      nparams = fn->param_count;
    ASTNode    *body    = fn->children[0];   /* NODE_BLOCK */

    /* save codegen frame state — clear var table so main() vars don't leak in */
    int sv_vc = cg->var_count, sv_fo = cg->frame_offset, sv_sd = cg->scope_depth;
    int sv_nm = cg->noreturn_mode;
    int sv_rename = cg->fn_rename_count;   /* save rename scope for nested fns */
    cg->scope_depth  = 0;
    cg->frame_offset = 0;
    cg->var_count    = 0;   /* hide main() vars: functions have their own scope */
    cg->noreturn_mode = 0;

    /* Restore the rename context that was visible when this fn was declared.
     * This makes sibling/peer inner function names resolvable inside the body. */
    fn_rename_ctx_restore(cg, rename_ctx, rename_ctx_count);

    /* frame size: params + captures + locals + generous padding, 16-aligned */
    int n = (int)nparams + ncaptures + count_locals(body) + 32;
    int frame = (n * 8 + 15) & ~15;

    emit(cg, "");
    emit(cg, ".Lxly_fn_%s:", asm_label);
    emit(cg, "    pushq   %%rbp");
    emit(cg, "    movq    %%rsp, %%rbp");
    emit(cg, "    subq    $%d, %%rsp", frame);

    /* For closures: hidden first arg is XlyVal** __env (in %rdi).
     * Explicit params are shifted: param[0] → %rsi, param[1] → %rdx, etc.
     * We load captured variables from __env[0..ncaptures-1] into stack slots. */
    static const char *pregs[] = {"rdi","rsi","rdx","rcx","r8","r9"};
    int param_reg_start = ncaptures > 0 ? 1 : 0; /* shift if closure */

    if (ncaptures > 0) {
        /* Spill __env ptr (rdi) into a temporary stack slot, then load captures */
        int env_slot = var_declare(cg, "__env");
        emit(cg, "    movq    %%rdi, %d(%%rbp)", env_slot);
        for (int i = 0; i < ncaptures; i++) {
            int cslot = var_declare(cg, captures[i]);
            emit(cg, "    movq    %d(%%rbp), %%rax", env_slot); /* __env ptr */
            emit(cg, "    movq    %d(%%rax), %%rax", i * 8);    /* env[i] */
            emit(cg, "    movq    %%rax, %d(%%rbp)", cslot);    /* store capture */
        }
    }

    /* spill incoming SysV register args into stack slots.
     * For closures param_reg_start=1 (rdi=env), so register params cap at
     * reg index 5, meaning up to (6 - param_reg_start) params in registers. */
    int nreg_params = 6 - param_reg_start;  /* how many params fit in regs */
    for (size_t i = 0; i < nparams; i++) {
        int off = var_declare(cg, fn->params[i].name);
        int reg_idx = (int)i + param_reg_start;
        if (reg_idx < 6) {
            /* Register-passed arg */
            emit(cg, "    movq    %%%s, %d(%%rbp)", pregs[reg_idx], off);
        } else {
            /* Stack-passed arg: [rbp+16] for param nreg_params,
             * [rbp+24] for param nreg_params+1, etc.
             * (rbp+0=saved_rbp, rbp+8=ret_addr, rbp+16=first stack arg) */
            int stk_slot = (int)(i - (size_t)nreg_params) * 8 + 16;
            emit(cg, "    movq    %d(%%rbp), %%rax", stk_slot);
            emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        }
        
        /* Handle optional/default parameters */
        if (fn->params[i].is_optional || fn->params[i].default_value) {
            char lbl_has_arg[64];
            fresh_label(cg, lbl_has_arg, sizeof(lbl_has_arg));
            
            emit(cg, "    movq    %d(%%rbp), %%rax", off);
            emit(cg, "    testq   %%rax, %%rax");
            emit(cg, "    jnz     %s", lbl_has_arg);
            
            if (fn->params[i].default_value) {
                emit_expr(cg, fn->params[i].default_value);
                emit(cg, "    movq    %%rax, %d(%%rbp)", off);
            }
            
            emit(cg, "%s:", lbl_has_arg);
        }
    }
    (void)nreg_params; /* suppress unused-variable warning if nparams==0 */

    /* body */
    scope_enter(cg);
    for (size_t i = 0; i < body->child_count; i++)
        emit_stmt(cg, body->children[i]);
    scope_leave(cg);

    /* implicit return null */
    emit(cg, "    call    " XLY_SYM("xly_null"));
    emit(cg, "    movq    %%rbp, %%rsp");
    emit(cg, "    popq    %%rbp");
    emit(cg, "    ret");

    /* restore codegen state */
    while (cg->var_count > sv_vc) { cg->var_count--; free(cg->vars[cg->var_count].name); }
    cg->frame_offset  = sv_fo;
    cg->scope_depth   = sv_sd;
    cg->noreturn_mode = sv_nm;
    cg->frame_offset = sv_fo;
    cg->scope_depth  = sv_sd;
    fn_renames_pop_to(cg, sv_rename);  /* pop renames pushed by nested fn decls */
}

/* ── x86-64 public entry (called by the dispatch codegen() below) ───────── */
/* g_opt_level / g_verbose_asm are declared above, outside all arch #ifdefs */
static int codegen_x86_64(ASTNode *program, const char *outpath) {
    CG cg;
    memset(&cg, 0, sizeof(cg));
    cg.out       = fopen(outpath, "w");
    if (!cg.out) { perror("codegen: fopen"); return 1; }
    cg.opt_level = g_opt_level;
    cg.verbose   = g_verbose_asm;

    if (cg.verbose)
        emit(&cg, "    # xenlyc v0.1.0  opt=%d  arch=x86-64  abi=sysv", cg.opt_level);

    /* pre-pass: collect all declared function names for call-site dispatch */
    collect_fn_names(&cg, program);
    collect_variant_names(&cg, program);

    /* main frame: generous */
    /* Frame = locals × 8 + 512 bytes of spill headroom.
     * Each aligned spill in emit_expr uses 16 bytes temporarily;
     * deeply nested expressions (sys calls inside arithmetic etc.)
     * can hold up to ~32 simultaneous spill slots = 512 bytes. */
    int n_top = count_locals(program) + 64;
    int mframe = (n_top * 8 + 15) & ~15;

    emit(&cg, XLY_TEXT_SECTION);
    emit(&cg, ".globl  " XLY_SYM("main"));
    emit(&cg, XLY_SYM("main") ":");
    emit(&cg, "    pushq   %%rbp");
    emit(&cg, "    movq    %%rsp, %%rbp");
    emit(&cg, "    subq    $%d, %%rsp", mframe);

    cg.in_main_scope = 1;
    for (size_t i = 0; i < program->child_count; i++)
        emit_stmt(&cg, program->children[i]);
    cg.in_main_scope = 0;

    emit(&cg, "    movl    $0, %%edi");
    emit(&cg, "    call    " XLY_SYM("xly_exit"));
    emit(&cg, "    movq    %%rbp, %%rsp");
    emit(&cg, "    popq    %%rbp");
    emit(&cg, "    ret");

    /* user functions */
    for (int i = 0; i < cg.func_count; i++)
        emit_function(&cg, cg.funcs[i].node, cg.funcs[i].asm_label,
                      cg.funcs[i].rename_ctx, cg.funcs[i].rename_ctx_count,
                      cg.funcs[i].captures, cg.funcs[i].ncaptures);

    /* .rodata */
    emit(&cg, "");
    emit(&cg, XLY_DATA_SECTION);
    for (int i = 0; i < cg.str_count; i++) {
        emit(&cg, "%s:", cg.strings[i].label);
        fprintf(cg.out, "    .asciz  \"");
        for (const char *s = cg.strings[i].text; *s; s++) {
            unsigned char ch = (unsigned char)*s;
            switch (ch) {
                case '\n': fputs("\\n",  cg.out); break;
                case '\t': fputs("\\t",  cg.out); break;
                case '\r': fputs("\\r",  cg.out); break;
                case '\\': fputs("\\\\", cg.out); break;
                case '"':  fputs("\\\"", cg.out); break;
                default:
                    // For UTF-8 multibyte sequences and high bytes, use octal escapes
                    // This ensures proper encoding in assembly
                    if (ch >= 0x80 || ch < 0x20) {
                        fprintf(cg.out, "\\%03o", ch);
                    } else {
                        fputc(ch, cg.out);
                    }
                    break;
            }
        }
        fputs("\"\n", cg.out);
    }

    /* .note.GNU-stack — tell the linker the stack is NOT executable */
    emit(&cg, "");
#if XLY_EMIT_GNU_STACK
    emit(&cg, XLY_GNU_STACK_SECTION);
#endif

    /* Global variable storage: __xly_globals[N] = array of XlyVal* pointers.
     * Initialised to zero (BSS); filled at runtime by main().              */
    if (cg.gvar_count > 0) {
        emit(&cg, "");
#if defined(__APPLE__)
        emit(&cg, ".section __DATA,__bss");
#else
        emit(&cg, ".section .bss");
#endif
        emit(&cg, ".globl  " XLY_SYM("__xly_globals"));
        emit(&cg, ".balign 8");
        emit(&cg, XLY_SYM("__xly_globals") ":");
        emit(&cg, "    .zero   %d", cg.gvar_count * 8);
    }

    /* Emit compilation stats as trailing comments when verbose */
    if (cg.verbose) {
        emit(&cg, "    # --- xenlyc stats ---");
        emit(&cg, "    # constant folds:   %d", cg.stat_const_fold);
        emit(&cg, "    # sys inlines:      %d", cg.stat_sys_inline);
        emit(&cg, "    # noreturn elim:    %d", cg.stat_noreturn_elim);
        emit(&cg, "    # unboxed ops:      %d", cg.stat_unboxed_ops);
    }

    fclose(cg.out);

    /* Print stats to stderr if verbose */
    if (g_verbose_asm) {
        fputs("\033[1;36m[xenlyc] codegen stats:\033[0m\n", stderr);
        fprintf(stderr, "  sys constants inlined:   %d\n", cg.stat_sys_inline);
        fprintf(stderr, "  noreturn elims:          %d\n", cg.stat_noreturn_elim);
        fprintf(stderr, "  constant folds:          %d\n", cg.stat_const_fold);
        fprintf(stderr, "  unboxed arithmetic ops:  %d\n", cg.stat_unboxed_ops);
    }

    /* cleanup */
    for (int i = 0; i < cg.var_count; i++)    free(cg.vars[i].name);
    free(cg.vars);
    for (int i = 0; i < cg.gvar_count; i++)   free(cg.gvars[i].name);
    free(cg.gvars);
    for (int i = 0; i < cg.str_count; i++)   { free(cg.strings[i].text); free(cg.strings[i].label); }
    free(cg.strings);
    for (int i = 0; i < cg.func_count; i++) {
        free(cg.funcs[i].asm_label);
        fn_rename_ctx_free(cg.funcs[i].rename_ctx, cg.funcs[i].rename_ctx_count);
    }
    free(cg.funcs);
    for (int i = 0; i < cg.fn_name_count; i++) free(cg.fn_names[i]);
    free(cg.fn_names);
    for (int i = 0; i < cg.fn_labels_used_count; i++) free(cg.fn_labels_used[i].lbl);
    free(cg.fn_labels_used);
    for (int i = 0; i < cg.fn_rename_count; i++) { free(cg.fn_renames[i].src); free(cg.fn_renames[i].lbl); }
    free(cg.fn_renames);
    for (int i = 0; i < cg.variant_count; i++) free(cg.variants[i].name);
    free(cg.variants);
    for (int i = 0; i < cg.brk_top; i++) free(cg.brk_labels[i]);
    free(cg.brk_labels);
    for (int i = 0; i < cg.cnt_top; i++) free(cg.cnt_labels[i]);
    free(cg.cnt_labels);

    return cg.had_error;
}

#endif /* XLY_ARCH_X86_64 */


/* ═══════════════════════════════════════════════════════════════════════════
 * AArch64 / ARM64 BACKEND  (macOS Apple Silicon)
 *
 * ABI (Apple ARM64 / AAPCS64):
 *   • Integer/pointer args:  x0–x7        Return: x0
 *   • FP args:               d0–d7        Return: d0
 *   • Caller-saved scratch:  x9–x15, d8–d15
 *   • Callee-saved:          x19–x28, x29 (fp), x30 (lr)
 *   • Frame pointer:         x29   Link register: x30
 *   • sp MUST be 16-byte aligned at ALL times (enforced by hardware)
 *   • No red zone on macOS/arm64
 *
 * Frame layout per function:
 *   sp after prologue:   [x29|x30] at highest addresses, locals below
 *   Variable slots:      negative offsets from x29 (e.g. [x29, #-8])
 *   Spill slots:         we use str/ldr pairs around calls instead of
 *                        push/pop (no push/pop on AArch64)
 *
 * Key emit conventions:
 *   emit_expr_a64(cg, node)  — result XlyVal* lands in x0; sp unchanged
 *   emit_stmt_a64(cg, node)  — sp unchanged on exit
 *   bl   SYM               — branch-with-link (call)
 *   adrp x9, SYM@PAGE      — load PC-relative address (2-instr sequence)
 *   add  x9, x9, SYM@PAGEOFF
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifdef XLY_ARCH_ARM64

/* ── runtime symbol name helper (ARM64 only) ─────────────────────────────
 * XLY_SYM() is a compile-time string concatenation macro, so it cannot
 * accept a runtime variable.  xly_sym_a64() returns a pointer to a static
 * buffer with the correct prefix for the current platform.               */
#ifdef XLY_ARCH_ARM64
static const char *xly_sym_a64(char *buf, size_t bufsz, const char *name) {
#if defined(XLY_PLATFORM_MACOS) || defined(PLATFORM_MACOS)
    snprintf(buf, bufsz, "_%s", name);
#else
    snprintf(buf, bufsz, "%s", name);
#endif
    return buf;
}
#define XLY_RSYM(buf, name)  xly_sym_a64((buf), sizeof(buf), (name))
#endif /* XLY_ARCH_ARM64 */
static void emit_expr_a64(CG *cg, ASTNode *node);
static void emit_stmt_a64(CG *cg, ASTNode *node);

/* ── load a 64-bit double constant into d0 ──────────────────────────────
 * Strategy: materialise the bit pattern in x9 via movz/movk, then
 * fmov d0, x9.  Works for any bit pattern without a .rodata entry.        */
static void emit_load_double_a64(CG *cg, double d) {
    union { double d; uint64_t u; } uu; uu.d = d;
    uint64_t u = uu.u;
    /* Build u in x9 with up to 4 movz/movk instructions */
    int first = 1;
    for (int shift = 0; shift <= 48; shift += 16) {
        uint64_t chunk = (u >> shift) & 0xFFFF;
        if (chunk == 0 && !first) continue;
        if (first) {
            emit(cg, "    movz    x9, #0x%llx, lsl #%d", (unsigned long long)chunk, shift);
            first = 0;
        } else {
            emit(cg, "    movk    x9, #0x%llx, lsl #%d", (unsigned long long)chunk, shift);
        }
    }
    if (first) emit(cg, "    movz    x9, #0");  /* d == 0.0 */
    emit(cg, "    fmov    d0, x9");
}

/* ── spill/reload helpers ───────────────────────────────────────────────
 * On Apple Silicon, sp must stay within the allocated frame at all times.
 * We use frame-relative spill slots at [x29, #offset] with NO sp movement.
 *
 * Spill slots are carved from the frame BELOW the current sp position:
 *   slot address = x29 + frame_offset - a64_sp_adj - (depth+1)*8
 *
 * a64_sp_adj tracks how many bytes sp has been moved down (by sub sp,#N
 * for print/method-call arg arrays).  This ensures spill slots always land
 * strictly below sp (i.e., in unused frame space), never in the arg arrays
 * that occupy [sp, #0 .. alloc_bytes-8].                                  */
/* ── safe_str_a64 / safe_ldr_a64 ────────────────────────────────────────
 * AArch64 unscaled load/store (STUR/LDUR) only encodes offsets in [-256,255].
 * For offsets outside that range, materialise the address in x9.
 * x9 is a caller-saved scratch register safe to clobber here.           */
static void safe_str_a64(CG *cg, const char *reg, int off) {
    if (off >= -256 && off <= 255) {
        emit(cg, "    str     %s, [x29, #%d]", reg, off);
    } else if (off < 0) {
        emit(cg, "    sub     x9, x29, #%d", -off);
        emit(cg, "    str     %s, [x9]", reg);
    } else {
        emit(cg, "    add     x9, x29, #%d", off);
        emit(cg, "    str     %s, [x9]", reg);
    }
}
static void safe_ldr_a64(CG *cg, const char *reg, int off) {
    if (off >= -256 && off <= 255) {
        emit(cg, "    ldr     %s, [x29, #%d]", reg, off);
    } else if (off < 0) {
        emit(cg, "    sub     x9, x29, #%d", -off);
        emit(cg, "    ldr     %s, [x9]", reg);
    } else {
        emit(cg, "    add     x9, x29, #%d", off);
        emit(cg, "    ldr     %s, [x9]", reg);
    }
}
static void spill_push_a64(CG *cg, const char *reg) {
    int slot = cg->frame_offset - cg->a64_sp_adj - (cg->a64_spill_depth + 1) * 8;
    safe_str_a64(cg, reg, slot);
    cg->a64_spill_depth++;
}
static void spill_pop_a64(CG *cg, const char *reg) {
    cg->a64_spill_depth--;
    int slot = cg->frame_offset - cg->a64_sp_adj - (cg->a64_spill_depth + 1) * 8;
    safe_ldr_a64(cg, reg, slot);
}
/* Emit sub/add sp and keep a64_sp_adj in sync */
static void sp_sub_a64(CG *cg, int bytes) {
    emit(cg, "    sub     sp, sp, #%d", bytes);
    cg->a64_sp_adj += bytes;
}
static void sp_add_a64(CG *cg, int bytes) {
    emit(cg, "    add     sp, sp, #%d", bytes);
    cg->a64_sp_adj -= bytes;
}

/* ── emit a PC-relative symbol address into a register ─────────────────
 * Two-instruction sequence required on AArch64:
 *   adrp  xN, sym@PAGE
 *   add   xN, xN, sym@PAGEOFF                                             */
static void emit_adrp_a64(CG *cg, const char *reg, const char *sym) {
    emit(cg, "    adrp    %s, %s@PAGE", reg, sym);
    emit(cg, "    add     %s, %s, %s@PAGEOFF", reg, reg, sym);
}

/* ── expression compiler (ARM64) — Post-condition: result XlyVal* in x0, sp unchanged. */
static void emit_expr_a64(CG *cg, ASTNode *node) {
    if (!node) { emit(cg, "    bl      " XLY_SYM("xly_null")); return; }

    switch (node->type) {

    /* ── literals ──────────────────────────────────────────────────── */
    case NODE_NUMBER:
        emit_load_double_a64(cg, node->num_value);
        emit(cg, "    bl      " XLY_SYM("xly_num"));
        break;

    case NODE_STRING: {
        const char *lbl = intern_string(cg, node->str_value ? node->str_value : "");
        emit_adrp_a64(cg, "x0", lbl);
        emit(cg, "    bl      " XLY_SYM("xly_str"));
        break;
    }

    case NODE_BOOL:
        emit(cg, "    mov     w0, #%d", node->bool_value ? 1 : 0);
        emit(cg, "    bl      " XLY_SYM("xly_bool"));
        break;

    case NODE_NULL:
        emit(cg, "    bl      " XLY_SYM("xly_null"));
        break;

    /* ── anonymous function expression: fn(params) { body } ──────────
     * Synthesise a unique name, stash for emission, wrap as XlyVal*. */
    case NODE_ARROW_FN: {
        char anon_name[64];
        snprintf(anon_name, sizeof(anon_name), "__anon_%d", cg->label_seq++);
        if (node->str_value) free(node->str_value);
        node->str_value = strdup(anon_name);
        fn_name_register(cg, anon_name);

        /* Free-variable analysis for closure capture */
        NameSet fn_params_a = {0};
        for (size_t i = 0; i < node->param_count; i++)
            nameset_add(&fn_params_a, node->params[i].name);
        NameSet free_vars_a = {0};
        if (node->child_count > 0)
            collect_free_vars(cg, node->children[0], &fn_params_a, &free_vars_a);
        nameset_free(&fn_params_a);
        NameSet captures_a = {0};
        for (int i = 0; i < free_vars_a.count; i++)
            if (var_offset(cg, free_vars_a.names[i]) != 0)
                nameset_add(&captures_a, free_vars_a.names[i]);
        nameset_free(&free_vars_a);

        if (cg->func_count >= cg->func_cap) {
            cg->func_cap = cg->func_cap ? cg->func_cap * 2 : 16;
            cg->funcs    = realloc(cg->funcs, sizeof(cg->funcs[0]) * (size_t)cg->func_cap);
        }
        { int fi = cg->func_count++;
          cg->funcs[fi].node = node;
          cg->funcs[fi].ncaptures = captures_a.count;
          cg->funcs[fi].captures  = captures_a.count > 0 ? captures_a.names : NULL;
          cg->funcs[fi].asm_label = strdup(anon_name);
          fn_label_mark_used(cg, anon_name);
          fn_rename_ctx_snapshot(cg, &cg->funcs[fi].rename_ctx, &cg->funcs[fi].rename_ctx_count); }

        /* Load address of the synthesized label, wrap via xly_make_fn/closure */
        emit(cg, "    adrp    x0, .Lxly_fn_%s@PAGE", anon_name);
        emit(cg, "    add     x0, x0, .Lxly_fn_%s@PAGEOFF", anon_name);
        if (captures_a.count == 0) {
            emit(cg, "    bl      " XLY_SYM("xly_make_fn"));
        } else {
            /* Build env on stack and call xly_make_closure */
            int env_bytes = ((captures_a.count * 8) + 15) & ~15;
            int alloc_a = ((env_bytes + 8) + 15) & ~15; /* +8 for fn ptr stash, 16-aligned */
            sp_sub_a64(cg, alloc_a); /* stash fn ptr at [sp], env[] at [sp+8] */
            emit(cg, "    str     x0, [sp]");  /* save fn ptr */
            for (int i = 0; i < captures_a.count; i++) {
                int off = var_offset(cg, captures_a.names[i]);
                safe_ldr_a64(cg, "x9", off);
                emit(cg, "    str     x9, [sp, #%d]", 8 + i * 8);
            }
            emit(cg, "    ldr     x0, [sp]");   /* fn ptr → x0 */
            emit(cg, "    add     x1, sp, #8"); /* env array → x1 */
            emit(cg, "    mov     w2, #%d", captures_a.count);
            emit(cg, "    bl      " XLY_SYM("xly_make_closure"));
            sp_add_a64(cg, alloc_a);
        }
        break;
    }

    /* ── identifier ─────────────────────────────────────────────────── */
    case NODE_IDENTIFIER: {
        int off = var_offset(cg, node->str_value);
        if (off != 0) {
            safe_ldr_a64(cg, "x0", off);
        } else {
            int gi = gvar_find(cg, node->str_value);
            if (gi >= 0) {
                /* Global: load from __xly_globals[gi] */
                emit(cg, "    adrp    x9, " XLY_SYM("__xly_globals") "@PAGE");
                emit(cg, "    add     x9, x9, " XLY_SYM("__xly_globals") "@PAGEOFF");
                emit(cg, "    ldr     x0, [x9, #%d]", gi * 8);
            } else if (fn_is_known(cg, node->str_value)) {
                const char *lbl_a = fn_resolve_label(cg, node->str_value);
                emit(cg, "    adrp    x0, .Lxly_fn_%s@PAGE", lbl_a);
                emit(cg, "    add     x0, x0, .Lxly_fn_%s@PAGEOFF", lbl_a);
                emit(cg, "    bl      " XLY_SYM("xly_make_fn"));
            } else {
                emit(cg, "    bl      " XLY_SYM("xly_null"));
            }
        }
        break;
    }

    /* ── this ────────────────────────────────────────────────────────── */
    case NODE_THIS:
        emit(cg, "    bl      " XLY_SYM("xly_this"));
        break;

    /* ── binary ─────────────────────────────────────────────────────── */
    case NODE_BINARY: {
        const char *op = node->str_value;

        /* short-circuit: and */
        if (strcmp(op, "and") == 0) {
            char lbl_end[64], lbl_done[64];
            int seq = cg->label_seq++;
            snprintf(lbl_end,  sizeof(lbl_end),  ".Lxly_%d_and_end",  seq);
            snprintf(lbl_done, sizeof(lbl_done), ".Lxly_%d_and_done", seq);

            emit_expr_a64(cg, node->children[0]);   /* x0 = left */
            spill_push_a64(cg, "x0");               /* save left */
            emit(cg, "    bl      " XLY_SYM("xly_truthy"));
            emit(cg, "    cbz     w0, %s", lbl_end);
            spill_pop_a64(cg, "x9");                /* discard left */
            emit_expr_a64(cg, node->children[1]);
            emit(cg, "    b       %s", lbl_done);
            emit(cg, "%s:", lbl_end);
            spill_pop_a64(cg, "x0");                /* left → x0 */
            emit(cg, "%s:", lbl_done);
            break;
        }

        /* short-circuit: or */
        if (strcmp(op, "or") == 0) {
            char lbl_end[64], lbl_done[64];
            int seq = cg->label_seq++;
            snprintf(lbl_end,  sizeof(lbl_end),  ".Lxly_%d_or_end",  seq);
            snprintf(lbl_done, sizeof(lbl_done), ".Lxly_%d_or_done", seq);

            emit_expr_a64(cg, node->children[0]);
            spill_push_a64(cg, "x0");
            emit(cg, "    bl      " XLY_SYM("xly_truthy"));
            emit(cg, "    cbnz    w0, %s", lbl_end);
            spill_pop_a64(cg, "x9");
            emit_expr_a64(cg, node->children[1]);
            emit(cg, "    b       %s", lbl_done);
            emit(cg, "%s:", lbl_end);
            spill_pop_a64(cg, "x0");
            emit(cg, "%s:", lbl_done);
            break;
        }

        /* arithmetic */
        int is_arith = (strcmp(op,"+") == 0 || strcmp(op,"-") == 0 ||
                        strcmp(op,"*") == 0 || strcmp(op,"/") == 0);
        if (is_arith) {
            int is_plus = (strcmp(op,"+") == 0);

            /* constant folding */
            if (node->children[0]->type == NODE_NUMBER &&
                node->children[1]->type == NODE_NUMBER) {
                double lv = node->children[0]->num_value;
                double rv = node->children[1]->num_value;
                double res = (strcmp(op,"+")==0) ? lv+rv :
                             (strcmp(op,"-")==0) ? lv-rv :
                             (strcmp(op,"*")==0) ? lv*rv : lv/rv;
                emit_load_double_a64(cg, res);
                emit(cg, "    bl      " XLY_SYM("xly_num"));
                break;
            }

            if (is_plus) {
                /* + needs type check: could be string concat */
                char lbl_slow[64], lbl_end2[64];
                fresh_label(cg, lbl_slow, sizeof(lbl_slow));
                fresh_label(cg, lbl_end2, sizeof(lbl_end2));

                emit_expr_a64(cg, node->children[0]);   /* x0 = left */
                spill_push_a64(cg, "x0");
                emit_expr_a64(cg, node->children[1]);   /* x0 = right */
                emit(cg, "    mov     x1, x0");         /* x1 = right */
                spill_pop_a64(cg, "x0");                /* x0 = left  */

                /* check left.type == VAL_STRING (1) */
                emit(cg, "    ldr     w9, [x0]");
                emit(cg, "    cmp     w9, #1");
                emit(cg, "    b.eq    %s", lbl_slow);
                /* check right.type */
                emit(cg, "    ldr     w9, [x1]");
                emit(cg, "    cmp     w9, #1");
                emit(cg, "    b.eq    %s", lbl_slow);

                /* fast numeric path */
                emit(cg, "    ldr     d0, [x0, #8]");
                emit(cg, "    ldr     d1, [x1, #8]");
                emit(cg, "    fadd    d0, d0, d1");
                emit(cg, "    bl      " XLY_SYM("xly_num"));
                emit(cg, "    b       %s", lbl_end2);

                emit(cg, "%s:", lbl_slow);
                emit(cg, "    bl      " XLY_SYM("xly_add"));
                emit(cg, "%s:", lbl_end2);
            } else {
                /* -, *, / — always numeric */
                emit_expr_a64(cg, node->children[0]);
                spill_push_a64(cg, "x0");
                emit_expr_a64(cg, node->children[1]);
                emit(cg, "    mov     x1, x0");
                spill_pop_a64(cg, "x0");

                emit(cg, "    ldr     d0, [x0, #8]");
                emit(cg, "    ldr     d1, [x1, #8]");
                if      (strcmp(op,"-") == 0) emit(cg, "    fsub    d0, d0, d1");
                else if (strcmp(op,"*") == 0) emit(cg, "    fmul    d0, d0, d1");
                else if (strcmp(op,"/") == 0) emit(cg, "    fdiv    d0, d0, d1");
                emit(cg, "    bl      " XLY_SYM("xly_num"));
            }
            break;
        }

        /* comparisons */
        int is_cmp = (strcmp(op,"<")  == 0 || strcmp(op,">")  == 0 ||
                      strcmp(op,"<=") == 0 || strcmp(op,">=") == 0 ||
                      strcmp(op,"==") == 0 || strcmp(op,"!=") == 0);
        if (is_cmp) {
            /* constant folding */
            if (node->children[0]->type == NODE_NUMBER &&
                node->children[1]->type == NODE_NUMBER) {
                double lv = node->children[0]->num_value;
                double rv = node->children[1]->num_value;
                int res = (strcmp(op,"<" )==0) ? lv <  rv :
                          (strcmp(op,">" )==0) ? lv >  rv :
                          (strcmp(op,"<=")==0) ? lv <= rv :
                          (strcmp(op,">=")==0) ? lv >= rv :
                          (strcmp(op,"==")==0) ? lv == rv : lv != rv;
                emit(cg, "    mov     w0, #%d", res);
                emit(cg, "    bl      " XLY_SYM("xly_bool"));
                break;
            }

            char lbl_slow[64], lbl_end2[64];
            fresh_label(cg, lbl_slow, sizeof(lbl_slow));
            fresh_label(cg, lbl_end2, sizeof(lbl_end2));

            emit_expr_a64(cg, node->children[0]);
            spill_push_a64(cg, "x0");
            emit_expr_a64(cg, node->children[1]);
            emit(cg, "    mov     x1, x0");
            spill_pop_a64(cg, "x0");

            /* check both are numbers (type == 0) */
            emit(cg, "    ldr     w9, [x0]");
            emit(cg, "    cbnz    w9, %s", lbl_slow);
            emit(cg, "    ldr     w9, [x1]");
            emit(cg, "    cbnz    w9, %s", lbl_slow);

            /* fast FP compare */
            emit(cg, "    ldr     d0, [x0, #8]");
            emit(cg, "    ldr     d1, [x1, #8]");
            emit(cg, "    fcmp    d0, d1");
            /* cset maps condition → 0/1 in w0 */
            if      (strcmp(op,"<" )==0) emit(cg, "    cset    w0, lt");
            else if (strcmp(op,">" )==0) emit(cg, "    cset    w0, gt");
            else if (strcmp(op,"<=")==0) emit(cg, "    cset    w0, le");
            else if (strcmp(op,">=")==0) emit(cg, "    cset    w0, ge");
            else if (strcmp(op,"==")==0) emit(cg, "    cset    w0, eq");
            else                         emit(cg, "    cset    w0, ne");
            emit(cg, "    bl      " XLY_SYM("xly_bool"));
            emit(cg, "    b       %s", lbl_end2);

            emit(cg, "%s:", lbl_slow);
            const char *fn = NULL;
            if      (strcmp(op,"==") == 0) fn = "xly_eq";
            else if (strcmp(op,"!=") == 0) fn = "xly_neq";
            else if (strcmp(op,"<")  == 0) fn = "xly_lt";
            else if (strcmp(op,">")  == 0) fn = "xly_gt";
            else if (strcmp(op,"<=") == 0) fn = "xly_lte";
            else if (strcmp(op,">=") == 0) fn = "xly_gte";
            if (fn) {
                char sym[64];
                emit(cg, "    bl      %s", XLY_RSYM(sym, fn));
            }
            emit(cg, "%s:", lbl_end2);
            break;
        }

        /* other binary ops (%) */
        emit_expr_a64(cg, node->children[0]);
        spill_push_a64(cg, "x0");
        emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x1, x0");
        spill_pop_a64(cg, "x0");
        if (strcmp(op, "%") == 0)
            emit(cg, "    bl      " XLY_SYM("xly_mod"));
        break;
    }

    /* ── unary ──────────────────────────────────────────────────────── */
    case NODE_UNARY: {
        emit_expr_a64(cg, node->children[0]);
        const char *unfn = strcmp(node->str_value, "-") == 0 ? "xly_neg" : "xly_not";
        char unsym[64];
        emit(cg, "    bl      %s", XLY_RSYM(unsym, unfn));
        break;
    }

    /* ── user function call ─────────────────────────────────────────── */
    case NODE_FN_CALL: {
        int nargs = (int)node->child_count;
        if (nargs > 8) {
            fprintf(stderr,
                "[xenlyc] warning: '%s' called with %d args; max 8 on ARM64 — extra ignored.\n",
                node->str_value, nargs);
            cg->had_error = 1;
            nargs = 8;
        }

        /* ── enum variant constructor (ARM64) ───────────────────────────── */
        if (node->str_value && variant_find(cg, node->str_value) >= 0) {
            int arr_bytes = nargs > 0 ? (((nargs * 8) + 15) & ~15) : 0;
            if (arr_bytes > 0)
                sp_sub_a64(cg, arr_bytes);
            for (int i = 0; i < nargs; i++) {
                emit_expr_a64(cg, node->children[i]);
                emit(cg, "    str     x0, [sp, #%d]", i * 8);
            }
            /* x0 = tag string */
            const char *tag_lbl = intern_string(cg, node->str_value);
            emit_adrp_a64(cg, "x0", tag_lbl);
            emit(cg, "    bl      " XLY_SYM("xly_str"));
            /* x1 = fields ptr (sp, or 0 if none) */
            if (nargs > 0)
                emit(cg, "    mov     x1, sp");
            else
                emit(cg, "    mov     x1, #0");
            emit(cg, "    mov     w2, #%d", nargs);
            emit(cg, "    bl      " XLY_SYM("xly_make_variant"));
            if (arr_bytes > 0)
                sp_add_a64(cg, arr_bytes);
            break;
        }

        /* AArch64: args go in x0–x7.  We eval left-to-right, spill each,
         * then reload in reverse order into the correct registers.         */
        static const char *aregs[] = {"x0","x1","x2","x3","x4","x5","x6","x7"};
        for (int i = 0; i < nargs; i++) {
            emit_expr_a64(cg, node->children[i]);
            spill_push_a64(cg, "x0");
        }
        /* Reload: top of spill stack is arg[nargs-1] → last reg */
        for (int i = nargs - 1; i >= 0; i--)
            spill_pop_a64(cg, (char*)aregs[i]);

        /* Dispatch: declared fn → bl direct; variable fn → xly_call_fnval */
        if (fn_is_known(cg, node->str_value)) {
            emit(cg, "    bl      .Lxly_fn_%s", fn_resolve_label(cg, node->str_value));
        } else {
            int var_off = var_offset(cg, node->str_value);
            int gi_fn_a = (var_off == 0) ? gvar_find(cg, node->str_value) : -1;
            if (var_off != 0 || gi_fn_a >= 0) {
                /* Use xly_call_fnval(fn_val, args_array, argc) which handles
                 * both plain fns and closures correctly via the builtin_fn
                 * field (offset 48) and inner env pointer (offset 56).
                 * xly_call_fnval(x0=fn, x1=args_ptr, x2=argc)             */
                int arr_bytes = nargs > 0 ? (((nargs * 8) + 15) & ~15) : 0;
                if (arr_bytes > 0)
                    sp_sub_a64(cg, arr_bytes);
                for (int i = 0; i < nargs; i++)
                    emit(cg, "    str     %s, [sp, #%d]", aregs[i], i * 8);
                /* Load fn_val XlyVal*: local variable or global */
                if (var_off != 0) {
                    if (var_off >= -255 && var_off <= 255)
                        emit(cg, "    ldr     x0, [x29, #%d]", var_off);
                    else {
                        emit(cg, "    mov     x9, #%d", var_off < 0 ? -var_off : var_off);
                        if (var_off < 0) emit(cg, "    neg     x9, x9");
                        emit(cg, "    ldr     x0, [x29, x9]");
                    }
                } else {
                    /* Global variable: load from __xly_globals[gi] */
                    emit(cg, "    adrp    x9, " XLY_SYM("__xly_globals") "@PAGE");
                    emit(cg, "    add     x9, x9, " XLY_SYM("__xly_globals") "@PAGEOFF");
                    emit(cg, "    ldr     x0, [x9, #%d]", gi_fn_a * 8);
                }
                if (nargs > 0)
                    emit(cg, "    mov     x1, sp");
                else
                    emit(cg, "    mov     x1, #0");
                emit(cg, "    mov     x2, #%d", nargs);
                emit(cg, "    bl      " XLY_SYM("xly_call_fnval"));
                if (arr_bytes > 0)
                    sp_add_a64(cg, arr_bytes);
            } else {
                emit(cg, "    bl      " XLY_SYM("xly_null"));
            }
        }
        break;
    }

    /* ── method call ─────────────────────────────────────────────────── */
    case NODE_METHOD_CALL: {
        const char *mod_name = node->children[0]->str_value;
        const char *fn_name  = node->str_value;
        int argc = (int)node->child_count - 1;

        /* Detect local variable object method call: if children[0] resolves
         * to a local variable (or is a complex expression), use xly_obj_call. */
        int obj_var_off_a = (node->children[0]->type == NODE_IDENTIFIER)
                            ? var_offset(cg, mod_name) : 0;
        int is_obj_call_a = (obj_var_off_a != 0) ||
                            (node->children[0]->type != NODE_IDENTIFIER);
        if (is_obj_call_a) {
            /* xly_obj_call(x0=obj, x1=method_str, x2=args_ptr, x3=argc) */
            int arr_bytes = argc > 0 ? (((argc * 8) + 15) & ~15) : 0;
            if (arr_bytes > 0)
                sp_sub_a64(cg, arr_bytes);
            for (int i = 0; i < argc; i++) {
                emit_expr_a64(cg, node->children[i + 1]);
                emit(cg, "    str     x0, [sp, #%d]", i * 8);
            }
            emit_expr_a64(cg, node->children[0]);    /* x0 = obj */
            const char *mfl = intern_string(cg, fn_name);
            emit_adrp_a64(cg, "x1", mfl);            /* x1 = method name */
            if (argc > 0)
                emit(cg, "    mov     x2, sp");
            else
                emit(cg, "    mov     x2, xzr");
            emit(cg, "    mov     x3, #%d", argc);
            emit(cg, "    bl      " XLY_SYM("xly_obj_call"));
            if (arr_bytes > 0)
                sp_add_a64(cg, arr_bytes);
            break;
        }

        /*
         * xly_call_module(mod, fn, args, argc)
         *   x0=mod  x1=fn  x2=args_ptr  x3=argc
         *
         * We reserve space for the args array on the stack (16-aligned),
         * eval each arg and store, then pass sp as the array pointer.
         */
        int slot_bytes  = argc * 8;
        int alloc_bytes = argc > 0 ? ((slot_bytes + 15) & ~15) : 0;

        if (alloc_bytes > 0)
            sp_sub_a64(cg, alloc_bytes);

        for (int i = 0; i < argc; i++) {
            emit_expr_a64(cg, node->children[i + 1]);
            emit(cg, "    str     x0, [sp, #%d]", i * 8);
        }

        /* x0 = mod string ptr */
        const char *ml = intern_string(cg, mod_name);
        emit_adrp_a64(cg, "x0", ml);
        /* x1 = fn string ptr */
        const char *fl = intern_string(cg, fn_name);
        emit_adrp_a64(cg, "x1", fl);
        /* x2 = args array pointer */
        if (argc > 0)
            emit(cg, "    mov     x2, sp");
        else
            emit(cg, "    mov     x2, xzr");
        /* x3 = argc */
        emit(cg, "    mov     x3, #%d", argc);
        emit(cg, "    bl      " XLY_SYM("xly_call_module"));

        if (alloc_bytes > 0)
            sp_add_a64(cg, alloc_bytes);
        break;
    }

    /* ── typeof ─────────────────────────────────────────────────────── */
    case NODE_TYPEOF:
        emit_expr_a64(cg, node->children[0]);
        /* x0 already holds the value pointer */
        emit(cg, "    bl      " XLY_SYM("xly_typeof"));
        break;

    /* ── increment / decrement as expression ────────────────────────── */
    case NODE_INCREMENT:
    case NODE_DECREMENT: {
        int off = var_offset(cg, node->str_value);
        safe_ldr_a64(cg, "x0", off);                   /* current value */
        spill_push_a64(cg, "x0");
        emit_load_double_a64(cg, 1.0);
        emit(cg, "    bl      " XLY_SYM("xly_num"));   /* x0 = num(1) */
        emit(cg, "    mov     x1, x0");
        spill_pop_a64(cg, "x0");
        { char isym[64]; emit(cg, "    bl      %s",
              XLY_RSYM(isym, node->type == NODE_INCREMENT ? "xly_add" : "xly_sub")); }
        safe_str_a64(cg, "x0", off);
        break;
    }

    /* ── array literal ─────────────────────────────────────────────── */
    case NODE_ARRAY_LITERAL: {
        int nelems = (int)node->child_count;
        int alloc_bytes = nelems > 0 ? (((nelems * 8) + 15) & ~15) & ~15 : 0;

        if (alloc_bytes > 0)
            sp_sub_a64(cg, alloc_bytes);

        for (int i = 0; i < nelems; i++) {
            emit_expr_a64(cg, node->children[i]);
            emit(cg, "    str     x0, [sp, #%d]", i * 8);
        }

        if (nelems > 0)
            emit(cg, "    mov     x0, sp");
        else
            emit(cg, "    mov     x0, xzr");
        emit(cg, "    mov     x1, #%d", nelems);
        emit(cg, "    bl      " XLY_SYM("xly_array_create"));

        if (alloc_bytes > 0)
            sp_add_a64(cg, alloc_bytes);
        break;
    }

    /* ── index ──────────────────────────────────────────────────────── */
    case NODE_INDEX:
        emit_expr_a64(cg, node->children[0]);
        spill_push_a64(cg, "x0");
        emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x1, x0");
        spill_pop_a64(cg, "x0");
        emit(cg, "    bl      " XLY_SYM("xly_index"));
        break;

    /* ── object literal  { key: val, ... } ─────────────────────────── */
    case NODE_OBJECT_LITERAL: {
        /* Parser produces NODE_NAMED_ARG children: str_value=key, children[0]=val */
        emit(cg, "    bl      " XLY_SYM("xly_obj_new"));
        /* Declare a hidden var slot to hold the obj ptr across iterations */
        char obj_slot_name[64];
        snprintf(obj_slot_name, sizeof(obj_slot_name), "__obj_%d", cg->label_seq++);
        int obj_slot = var_declare(cg, obj_slot_name);
        safe_str_a64(cg, "x0", obj_slot);
        int npairs = (int)node->child_count;
        for (int i = 0; i < npairs; i++) {
            ASTNode *pair     = node->children[i];  /* NODE_NAMED_ARG */
            const char *kl    = intern_string(cg, pair->str_value);
            ASTNode *val_node = pair->child_count > 0 ? pair->children[0] : NULL;
            if (val_node)
                emit_expr_a64(cg, val_node);
            else
                emit(cg, "    bl      " XLY_SYM("xly_null"));
            emit(cg, "    mov     x2, x0");            /* x2 = value */
            safe_ldr_a64(cg, "x0", obj_slot);         /* x0 = obj */
            emit_adrp_a64(cg, "x1", kl);              /* x1 = key */
            emit(cg, "    bl      " XLY_SYM("xly_obj_set"));
        }
        safe_ldr_a64(cg, "x0", obj_slot);   /* result = obj */
        break;
    }

    /* ── property get  obj.field ──────────────────────────────────── */
    case NODE_PROPERTY_GET: {
        emit_expr_a64(cg, node->children[0]);
        /* x0 = obj */
        const char *fl = intern_string(cg, node->str_value);
        emit_adrp_a64(cg, "x1", fl);
        emit(cg, "    bl      " XLY_SYM("xly_obj_get"));
        break;
    }

    /* ── property set  obj.field = val ──────────────────────────── */
    case NODE_PROPERTY_SET: {
        emit_expr_a64(cg, node->children[0]);
        spill_push_a64(cg, "x0");   /* spill obj */
        emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x2, x0");
        spill_pop_a64(cg, "x0");
        const char *fl2 = intern_string(cg, node->str_value);
        emit_adrp_a64(cg, "x1", fl2);
        emit(cg, "    bl      " XLY_SYM("xly_obj_set"));
        break;
    }

    /* ── await expr (ARM64) ─────────────────────────────────────────── */
    case NODE_AWAIT:
        emit_expr_a64(cg, node->children[0]);
        break;

    /* ── spawn expr (ARM64) ─────────────────────────────────────────── */
    case NODE_SPAWN:
        emit_expr_a64(cg, node->children[0]);   /* execute call, result in x0 */
        emit(cg, "    bl      " XLY_SYM("xly_null"));   /* spawn → null */
        break;

    default:
        emit(cg, "    bl      " XLY_SYM("xly_null"));
        break;
    }
}

/* ── statement compiler (ARM64) — Post-condition: sp unchanged from entry. */
static void emit_stmt_a64(CG *cg, ASTNode *node) {
    if (!node) return;

    switch (node->type) {

    case NODE_VAR_DECL:
    case NODE_CONST_DECL: {
        if (node->child_count > 0)
            emit_expr_a64(cg, node->children[0]);
        else
            emit(cg, "    bl      " XLY_SYM("xly_null"));
        if (cg->in_main_scope && cg->scope_depth == 0) {
            int gi = gvar_declare(cg, node->str_value);
            emit(cg, "    adrp    x9, " XLY_SYM("__xly_globals") "@PAGE");
            emit(cg, "    add     x9, x9, " XLY_SYM("__xly_globals") "@PAGEOFF");
            emit(cg, "    str     x0, [x9, #%d]", gi * 8);
            int off = var_declare(cg, node->str_value);
            safe_str_a64(cg, "x0", off);
        } else {
            int off = var_declare(cg, node->str_value);
            safe_str_a64(cg, "x0", off);
        }
        break;
    }

    case NODE_ASSIGN: {
        emit_expr_a64(cg, node->children[0]);
        int off = var_offset(cg, node->str_value);
        safe_str_a64(cg, "x0", off);
        break;
    }

    case NODE_COMPOUND_ASSIGN: {
        const char *varname = node->children[0]->str_value;
        int off = var_offset(cg, varname);
        safe_ldr_a64(cg, "x0", off);
        spill_push_a64(cg, "x0");
        emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x1, x0");
        spill_pop_a64(cg, "x0");
        const char *op = node->str_value;
        const char *fn = "xly_add";
        if      (strcmp(op,"+=") == 0) fn = "xly_add";
        else if (strcmp(op,"-=") == 0) fn = "xly_sub";
        else if (strcmp(op,"*=") == 0) fn = "xly_mul";
        else if (strcmp(op,"/=") == 0) fn = "xly_div";
        { char csym[64]; emit(cg, "    bl      %s", XLY_RSYM(csym, fn)); }
        safe_str_a64(cg, "x0", off);
        break;
    }

    case NODE_INCREMENT:
    case NODE_DECREMENT: {
        int off = var_offset(cg, node->str_value);
        safe_ldr_a64(cg, "x0", off);
        spill_push_a64(cg, "x0");
        emit_load_double_a64(cg, 1.0);
        emit(cg, "    bl      " XLY_SYM("xly_num"));
        emit(cg, "    mov     x1, x0");
        spill_pop_a64(cg, "x0");
        { char isym2[64]; emit(cg, "    bl      %s",
              XLY_RSYM(isym2, node->type == NODE_INCREMENT ? "xly_add" : "xly_sub")); }
        safe_str_a64(cg, "x0", off);
        break;
    }

    case NODE_PRINT: {
        /*
         * xly_print(XlyVal **vals, size_t n)   x0=vals, x1=n
         * Reserve stack space for the val pointer array.
         * Use sp_sub_a64/sp_add_a64 so a64_sp_adj tracks sp movement;
         * spill_push_a64 then places spills strictly below sp.
         */
        int n = (int)node->child_count;
        int alloc_bytes = n > 0 ? (((n * 8) + 15) & ~15) : 0;
        if (alloc_bytes > 0)
            sp_sub_a64(cg, alloc_bytes);

        for (int i = 0; i < n; i++) {
            emit_expr_a64(cg, node->children[i]);
            emit(cg, "    str     x0, [sp, #%d]", i * 8);
        }

        emit(cg, "    mov     x0, sp");
        emit(cg, "    mov     x1, #%d", n);
        emit(cg, "    bl      " XLY_SYM("xly_print"));

        if (alloc_bytes > 0)
            sp_add_a64(cg, alloc_bytes);
        break;
    }

    case NODE_EXPR_STMT:
        if (node->child_count > 0)
            emit_expr_a64(cg, node->children[0]);
        break;

    case NODE_IF: {
        char lbl_else[64], lbl_end[64];
        fresh_label(cg, lbl_else, sizeof(lbl_else));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        emit_expr_a64(cg, node->children[0]);
        emit(cg, "    bl      " XLY_SYM("xly_truthy"));
        emit(cg, "    cbz     w0, %s", lbl_else);

        emit_stmt_a64(cg, node->children[1]);
        emit(cg, "    b       %s", lbl_end);

        emit(cg, "%s:", lbl_else);
        if (node->child_count > 2)
            emit_stmt_a64(cg, node->children[2]);

        emit(cg, "%s:", lbl_end);
        break;
    }

    case NODE_WHILE: {
        char lbl_cond[64], lbl_end[64];
        fresh_label(cg, lbl_cond, sizeof(lbl_cond));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        push_brk(cg, lbl_end);
        push_cnt(cg, lbl_cond);

        emit(cg, "%s:", lbl_cond);
        emit_expr_a64(cg, node->children[0]);
        emit(cg, "    bl      " XLY_SYM("xly_truthy"));
        emit(cg, "    cbz     w0, %s", lbl_end);

        emit_stmt_a64(cg, node->children[1]);
        emit(cg, "    b       %s", lbl_cond);

        emit(cg, "%s:", lbl_end);
        pop_cnt(cg);
        pop_brk(cg);
        break;
    }

    case NODE_FOR: {
        char lbl_cond[64], lbl_upd[64], lbl_end[64];
        fresh_label(cg, lbl_cond, sizeof(lbl_cond));
        fresh_label(cg, lbl_upd,  sizeof(lbl_upd));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        scope_enter(cg);
        push_brk(cg, lbl_end);
        push_cnt(cg, lbl_upd);

        emit_stmt_a64(cg, node->children[0]);   /* init */

        emit(cg, "%s:", lbl_cond);
        if (!(node->children[1]->type == NODE_BOOL && node->children[1]->bool_value)) {
            emit_expr_a64(cg, node->children[1]);
            emit(cg, "    bl      " XLY_SYM("xly_truthy"));
            emit(cg, "    cbz     w0, %s", lbl_end);
        }

        emit_stmt_a64(cg, node->children[3]);   /* body */

        emit(cg, "%s:", lbl_upd);
        if (node->children[2]->type != NODE_NULL)
            emit_stmt_a64(cg, node->children[2]);

        emit(cg, "    b       %s", lbl_cond);
        emit(cg, "%s:", lbl_end);

        pop_cnt(cg);
        pop_brk(cg);
        scope_leave(cg);
        break;
    }

    case NODE_FOR_IN: {
        char lbl_cond[64], lbl_end[64];
        fresh_label(cg, lbl_cond, sizeof(lbl_cond));
        fresh_label(cg, lbl_end,  sizeof(lbl_end));

        scope_enter(cg);

        int off_iter = var_declare(cg, node->str_value);
        char tmp[80];
        snprintf(tmp, sizeof(tmp), "__fi_a_%d", cg->label_seq);
        int off_arr = var_declare(cg, tmp);
        snprintf(tmp, sizeof(tmp), "__fi_i_%d", cg->label_seq);
        int off_idx = var_declare(cg, tmp);
        snprintf(tmp, sizeof(tmp), "__fi_l_%d", cg->label_seq++);
        int off_len = var_declare(cg, tmp);

        emit_expr_a64(cg, node->children[0]);
        safe_str_a64(cg, "x0", off_arr);
        emit(cg, "    bl      " XLY_SYM("xly_array_len"));
        safe_str_a64(cg, "x0", off_len);
        /* idx = 0 (store integer 0 — raw, not a boxed XlyVal) */
        safe_str_a64(cg, "xzr", off_idx);

        push_brk(cg, lbl_end);
        push_cnt(cg, lbl_cond);

        emit(cg, "%s:", lbl_cond);
        safe_ldr_a64(cg, "x9", off_idx);
        safe_ldr_a64(cg, "x10", off_len);
        emit(cg, "    cmp     x9, x10");
        emit(cg, "    b.hs    %s", lbl_end);   /* idx >= len → done */

        safe_ldr_a64(cg, "x0", off_arr);
        safe_ldr_a64(cg, "x1", off_idx);
        emit(cg, "    bl      " XLY_SYM("xly_array_get"));
        safe_str_a64(cg, "x0", off_iter);

        emit_stmt_a64(cg, node->children[1]);

        safe_ldr_a64(cg, "x9", off_idx);
        emit(cg, "    add     x9, x9, #1");
        safe_str_a64(cg, "x9", off_idx);
        emit(cg, "    b       %s", lbl_cond);
        emit(cg, "%s:", lbl_end);

        pop_cnt(cg);
        pop_brk(cg);
        scope_leave(cg);
        break;
    }

    case NODE_BREAK:
        if (cg->brk_top > 0)
            emit(cg, "    b       %s", cg->brk_labels[cg->brk_top-1]);
        break;
    case NODE_CONTINUE:
        if (cg->cnt_top > 0)
            emit(cg, "    b       %s", cg->cnt_labels[cg->cnt_top-1]);
        break;

    case NODE_BLOCK:
        scope_enter(cg);
        for (size_t i = 0; i < node->child_count; i++)
            emit_stmt_a64(cg, node->children[i]);
        scope_leave(cg);
        break;

    case NODE_FN_DECL: {
        char ulbl_a[256];
        fn_unique_label(cg, node->str_value, ulbl_a, sizeof(ulbl_a));
        fn_rename_push(cg, node->str_value, ulbl_a);
        if (cg->func_count >= cg->func_cap) {
            cg->func_cap = cg->func_cap ? cg->func_cap * 2 : 16;
            cg->funcs    = realloc(cg->funcs, sizeof(cg->funcs[0]) * (size_t)cg->func_cap);
        }
        { int fi = cg->func_count++;
          cg->funcs[fi].node      = node;
          cg->funcs[fi].captures  = NULL;
          cg->funcs[fi].ncaptures = 0;
          cg->funcs[fi].asm_label = strdup(ulbl_a);
          fn_rename_ctx_snapshot(cg, &cg->funcs[fi].rename_ctx, &cg->funcs[fi].rename_ctx_count); }
        break;
    }

    case NODE_RETURN:
        if (node->child_count > 0)
            emit_expr_a64(cg, node->children[0]);
        else
            emit(cg, "    bl      " XLY_SYM("xly_null"));
        /* Epilogue: x29 = sp + fp_adj (= frame - 16).
         * Restore frame base into sp, reload saved pair from top, deallocate. */
        {
            int fs   = cg->a64_frame_size;
            int fpad = fs - 16;
            emit(cg, "    sub     sp, x29, #%d", fpad);  /* sp → frame base */
            if (fpad <= 504) {
                emit(cg, "    ldp     x29, x30, [sp, #%d]", fpad);
            } else {
                emit(cg, "    add     x9, sp, #%d", fpad);
                emit(cg, "    ldp     x29, x30, [x9]");
            }
            emit(cg, "    add     sp, sp, #%d", fs);     /* sp → old_sp     */
        }
        emit(cg, "    ret");
        break;

    case NODE_IMPORT:
        break;

    /* ── enum declaration (ARM64) ─────────────────────────────────────── */
    case NODE_ENUM_DECL:
        for (size_t vi = 0; vi < node->child_count; vi++) {
            ASTNode *vn = node->children[vi];
            if (!vn || !vn->str_value) continue;
            if (vn->param_count == 0) {
                /* Create singleton and store in global */
                const char *tag_lbl = intern_string(cg, vn->str_value);
                emit_adrp_a64(cg, "x0", tag_lbl);
                emit(cg, "    bl      " XLY_SYM("xly_str"));
                emit(cg, "    mov     x1, #0");   /* fields = NULL */
                emit(cg, "    mov     w2, #0");   /* nfields = 0   */
                emit(cg, "    bl      " XLY_SYM("xly_make_variant"));
                int gi = gvar_declare(cg, vn->str_value);
                emit(cg, "    adrp    x9, " XLY_SYM("__xly_globals") "@PAGE");
                emit(cg, "    add     x9, x9, " XLY_SYM("__xly_globals") "@PAGEOFF");
                emit(cg, "    str     x0, [x9, #%d]", gi * 8);
            }
        }
        break;

    /* ── Generator function declaration (ARM64) ─────────────────────────────
     * gen fn name(params) { body }
     *
     * Strategy: emit the generator body as a regular function but add
     * a state-machine wrapper.  The function returns an iterator object
     * allocated on the heap:  xly_call_module("iter","__make_gen__",…)
     *
     * For the native compiler the cleanest approach is to lower the
     * generator to a plain Xenly function whose first argument is a
     * heap-allocated state struct.  We call the runtime helper
     * xly_gen_create(fn_ptr, env_ptr) which returns a VAL_INSTANCE
     * iterator.  The generator body is emitted as a helper with a
     * switch-based PC dispatch.                                         */
    case NODE_GEN_DECL: {
        const char *gen_name = node->str_value ? node->str_value : "__anon_gen";
        char body_lbl[128]; snprintf(body_lbl, sizeof(body_lbl), "_xly_gen_%s", gen_name);

        /* Emit the state-machine body function */
        emit(cg, "// ── generator body: %s ─────────────────────────────", gen_name);
        emit(cg, "    adrp    x0, %s@PAGE", body_lbl);
        emit(cg, "    add     x0, x0, %s@PAGEOFF", body_lbl);
        /* Pass fn pointer + current env to runtime helper */
        emit(cg, "    mov     x1, sp");   /* env approximation */
        emit(cg, "    bl      " XLY_SYM("xly_gen_create"));
        /* x0 = iterator VAL_INSTANCE */
        /* Store in global if named */
        if (node->str_value) {
            int gi = gvar_declare(cg, node->str_value);
            emit(cg, "    adrp    x9, " XLY_SYM("__xly_globals") "@PAGE");
            emit(cg, "    add     x9, x9, " XLY_SYM("__xly_globals") "@PAGEOFF");
            emit(cg, "    str     x0, [x9, #%d]", gi * 8);
        }
        break;
    }

    /* ── yield expr (ARM64) ─────────────────────────────────────────────────
     * Within a generator body, yield suspends execution.
     * In native codegen this is lowered to:
     *   1. Evaluate the yield expression → x0
     *   2. Store x0 as the current yield value via xly_gen_yield(x0)
     *   3. Return from the body function (the runtime resumes on next .next())
     * The runtime tracks the PC via the state struct.                   */
    case NODE_YIELD:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        else emit(cg, "    bl      " XLY_SYM("xly_null"));
        emit(cg, "    bl      " XLY_SYM("xly_gen_yield"));  /* saves yield value */
        emit(cg, "    ret");                                  /* suspend */
        break;

    case NODE_YIELD_EMPTY:
        emit(cg, "    bl      " XLY_SYM("xly_null"));
        emit(cg, "    bl      " XLY_SYM("xly_gen_yield"));
        emit(cg, "    ret");
        break;

    /* ── for (x of iterable) — ARM64 ────────────────────────────────────────
     * children[0] = iterable, children[1] = body, str_value = loop var name
     * Lowers to: call xly_for_of_next(iter) in a loop.                */
    case NODE_FOR_OF: {
        int lbl = cg->label_count++;
        char loop_lbl[32], end_lbl[32];
        snprintf(loop_lbl, sizeof(loop_lbl), ".Lfor_of_%d", lbl);
        snprintf(end_lbl,  sizeof(end_lbl),  ".Lfor_of_end_%d", lbl);

        /* Evaluate iterable → x19 (callee-saved) */
        emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");   /* save iterator */

        emit(cg, "%s:", loop_lbl);
        /* Call .next() on iterator */
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_for_of_next"));
        /* x0 = {value, done} or NULL if done */
        emit(cg, "    cbz     x0, %s", end_lbl);
        /* Store value in loop variable */
        if (node->str_value) {
            const char *var_lbl = intern_string(cg, node->str_value);
            emit(cg, "    adrp    x9, %s@PAGE", var_lbl);
            emit(cg, "    add     x9, x9, %s@PAGEOFF", var_lbl);
            emit(cg, "    str     x0, [x9]");
        }
        /* Emit body */
        if (node->child_count > 1) emit_stmt_a64(cg, node->children[1]);
        emit(cg, "    b       %s", loop_lbl);
        emit(cg, "%s:", end_lbl);
        break;
    }

    /* ── Computed property access: obj.[expr] (ARM64) ───────────────────────
     * children[0] = object, children[1] = key expression               */
    case NODE_COMPUTED_PROP:
        emit_expr_a64(cg, node->children[0]);   /* x0 = obj */
        emit(cg, "    mov     x19, x0");
        emit_expr_a64(cg, node->children[1]);   /* x0 = key */
        emit(cg, "    mov     x1, x0");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_index"));   /* xly_index(obj, key) */
        break;

    case NODE_COMPUTED_PROP_SET:
        emit_expr_a64(cg, node->children[0]);   /* x0 = obj */
        emit(cg, "    mov     x19, x0");
        emit_expr_a64(cg, node->children[1]);   /* x0 = key */
        emit(cg, "    mov     x20, x0");
        emit_expr_a64(cg, node->children[2]);   /* x0 = val */
        emit(cg, "    mov     x2, x0");
        emit(cg, "    mov     x1, x20");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_index_set"));
        break;

    /* ── Reflect calls — lower to xly_rt reflect helpers (ARM64) ────────────
     * Each reflect.* operation maps to an xly_rt function.             */
    case NODE_REFLECT_KEYS:
    case NODE_REFLECT_OWN_KEYS:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    bl      " XLY_SYM("xly_reflect_keys"));
        break;

    case NODE_REFLECT_GET:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");
        if (node->child_count > 1) emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x1, x0");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_reflect_get"));
        break;

    case NODE_REFLECT_SET:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");
        if (node->child_count > 1) emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x20, x0");
        if (node->child_count > 2) emit_expr_a64(cg, node->children[2]);
        emit(cg, "    mov     x2, x0");
        emit(cg, "    mov     x1, x20");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_reflect_set"));
        break;

    case NODE_REFLECT_HAS:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");
        if (node->child_count > 1) emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x1, x0");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_reflect_has"));
        break;

    case NODE_REFLECT_DELETE:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");
        if (node->child_count > 1) emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x1, x0");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_reflect_delete"));
        break;

    case NODE_REFLECT_FREEZE:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    bl      " XLY_SYM("xly_reflect_freeze"));
        break;

    case NODE_REFLECT_IS_FROZEN:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    bl      " XLY_SYM("xly_reflect_is_frozen"));
        break;

    case NODE_REFLECT_APPLY:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");
        if (node->child_count > 2) emit_expr_a64(cg, node->children[2]);
        emit(cg, "    mov     x1, x0");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_reflect_apply"));
        break;

    case NODE_REFLECT_CONSTRUCT:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");
        if (node->child_count > 1) emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x1, x0");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_reflect_construct"));
        break;

    case NODE_REFLECT_DEFINE:
        if (node->child_count > 0) emit_expr_a64(cg, node->children[0]);
        emit(cg, "    mov     x19, x0");
        if (node->child_count > 1) emit_expr_a64(cg, node->children[1]);
        emit(cg, "    mov     x20, x0");
        if (node->child_count > 2) emit_expr_a64(cg, node->children[2]);
        emit(cg, "    mov     x2, x0");
        emit(cg, "    mov     x1, x20");
        emit(cg, "    mov     x0, x19");
        emit(cg, "    bl      " XLY_SYM("xly_reflect_define"));
        break;

    /* ── Block fn and trailing block call: desugar identical to ARROW_FN ─── */
    case NODE_BLOCK_FN:
        emit_expr_a64(cg, node);  /* fall through to expression emitter */
        break;

    default:
        emit_expr_a64(cg, node);
        break;
    }
}

/* ── emit a user function (ARM64) ────────────────────────────────────────── */
static void emit_function_a64(CG *cg, ASTNode *fn, const char *asm_label,
                              FnRename *rename_ctx, int rename_ctx_count,
                              char **captures, int ncaptures) {
    size_t      nparams = fn->param_count;
    ASTNode    *body    = fn->children[0];

    int sv_vc = cg->var_count, sv_fo = cg->frame_offset, sv_sd = cg->scope_depth;
    int sv_spill = cg->a64_spill_depth;
    int sv_nm_a = cg->noreturn_mode;
    int sv_rename_a = cg->fn_rename_count;   /* save rename scope */
    cg->scope_depth    = 0;
    cg->frame_offset   = 0;
    cg->var_count      = 0;   /* hide main() vars */
    cg->noreturn_mode  = 0;
    cg->a64_spill_depth = 0;
    cg->a64_sp_adj      = 0;

    /* Restore the rename context visible when this fn was declared. */
    fn_rename_ctx_restore(cg, rename_ctx, rename_ctx_count);

    /* Frame layout (macOS ARM64 ABI — NO red zone):
     *
     *   old_sp  ──► [ ... caller frame ... ]
     *               ┌─────────────────────────────────┐  ← old_sp - 0
     *               │  saved x30 (lr)                 │  [x29, #8]
     *               │  saved x29 (fp)  ◄── x29 ──     │  [x29, #0]
     *               ├─────────────────────────────────┤  ← old_sp - 16
     *               │  local var 0                    │  [x29, #-8]
     *               │  local var 1                    │  [x29, #-16]
     *               │  ...                            │
     *               │  spill slots                    │
     *               └─────────────────────────────────┘  ← sp = old_sp - frame
     *
     * x29 is set to (sp + frame - 16) so that:
     *   - [x29,  0] / [x29, #8]  hold the saved fp/lr pair
     *   - [x29, #-8], [x29, #-16], ... hold locals — all within [sp, old_sp)
     *
     * This satisfies the macOS AArch64 requirement that all memory accesses
     * stay within the allocated frame (no red zone below sp).               */
    int n = (int)nparams + count_locals(body) + 16;
    int locals_bytes = (n * 8 + 15) & ~15;
    int frame = locals_bytes + 16;  /* +16 for saved x29/x30 pair */
    frame = (frame + 15) & ~15;
    int fp_adj = frame - 16;       /* x29 = sp + fp_adj  (saved pair offset) */
    cg->a64_frame_size = frame;    /* stored for NODE_RETURN epilogue */

    emit(cg, "");
    emit(cg, ".Lxly_fn_%s:", asm_label);
    /* Prologue: allocate frame, save fp+lr at TOP, set x29 to saved-pair addr.
     * stp scaled-immediate range is [-512, 504] in steps of 8.
     * add/sub immediate range is [0, 4095].                                  */
    emit(cg, "    sub     sp, sp, #%d", frame);
    if (fp_adj <= 504) {
        emit(cg, "    stp     x29, x30, [sp, #%d]", fp_adj);
    } else {
        /* fp_adj > 504: use a scratch register to address the save slot */
        emit(cg, "    add     x9, sp, #%d", fp_adj);
        emit(cg, "    stp     x29, x30, [x9]");
    }
    emit(cg, "    add     x29, sp, #%d", fp_adj);
    /* Locals: var_declare assigns [x29,#-8], [x29,#-16], ...
     * All negative offsets from x29 are within [sp, sp+frame) — safe.
     * Spill slots go below locals, tracked by a64_spill_depth.              */

    /* For closures: hidden first arg x0 = XlyVal** __env.
     * Explicit params are shifted: param[0]->x1, param[1]->x2, etc.
     * Load captured variables from __env[0..ncaptures-1] into stack slots. */
    static const char *aregs[] = {"x0","x1","x2","x3","x4","x5","x6","x7"};
    int param_reg_start_a = (ncaptures > 0) ? 1 : 0;

    if (ncaptures > 0) {
        /* Spill __env ptr (x0) into a temp stack slot, then load each capture */
        int env_slot = var_declare(cg, "__env");
        safe_str_a64(cg, "x0", env_slot);
        for (int i = 0; i < ncaptures; i++) {
            int cslot = var_declare(cg, captures[i]);
            safe_ldr_a64(cg, "x9", env_slot);            /* x9 = __env ptr */
            emit(cg, "    ldr     x10, [x9, #%d]", i * 8); /* x10 = env[i] */
            safe_str_a64(cg, "x10", cslot);               /* store capture */
        }
    }

    /* Spill incoming args into their stack slots */
    for (size_t i = 0; i < nparams && (i + (size_t)param_reg_start_a) < 8; i++) {
        int off = var_declare(cg, fn->params[i].name);
        safe_str_a64(cg, aregs[i + param_reg_start_a], off);

        if (fn->params[i].is_optional || fn->params[i].default_value) {
            char lbl_has_arg[64];
            fresh_label(cg, lbl_has_arg, sizeof(lbl_has_arg));
            safe_ldr_a64(cg, "x9", off);
            emit(cg, "    cbnz    x9, %s", lbl_has_arg);
            if (fn->params[i].default_value) {
                emit_expr_a64(cg, fn->params[i].default_value);
                safe_str_a64(cg, "x0", off);
            }
            emit(cg, "%s:", lbl_has_arg);
        }
    }

    scope_enter(cg);
    for (size_t i = 0; i < body->child_count; i++)
        emit_stmt_a64(cg, body->children[i]);
    scope_leave(cg);

    /* Implicit return null + epilogue.
     * x29 = sp + fp_adj, so frame base = x29 - fp_adj.
     * Restore sp, reload saved pair from top of frame, deallocate.         */
    emit(cg, "    bl      " XLY_SYM("xly_null"));
    emit(cg, "    sub     sp, x29, #%d", fp_adj);   /* sp → frame base     */
    if (fp_adj <= 504) {
        emit(cg, "    ldp     x29, x30, [sp, #%d]", fp_adj);
    } else {
        emit(cg, "    add     x9, sp, #%d", fp_adj);
        emit(cg, "    ldp     x29, x30, [x9]");
    }
    emit(cg, "    add     sp, sp, #%d", frame);      /* sp → old_sp         */
    emit(cg, "    ret");

    while (cg->var_count > sv_vc) { cg->var_count--; free(cg->vars[cg->var_count].name); }
    cg->frame_offset    = sv_fo;
    cg->scope_depth     = sv_sd;
    cg->a64_spill_depth = sv_spill;
    cg->noreturn_mode   = sv_nm_a;
    cg->a64_sp_adj      = 0;  /* reset: caller's sp_adj is already 0 at call sites */
    fn_renames_pop_to(cg, sv_rename_a);  /* pop renames pushed by nested fn decls */
}

#endif /* XLY_ARCH_ARM64 */


/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC ENTRY POINT  —  dispatches to x86-64 or ARM64 backend
 * ═══════════════════════════════════════════════════════════════════════════ */
/* ── Public configuration — called by xenlyc_main before codegen() ───────── */
void codegen_set_opts(int opt_level, int verbose_asm) {
    g_opt_level   = opt_level;
    g_verbose_asm = verbose_asm;
}

int codegen(ASTNode *program, const char *outpath) {
#ifdef XLY_ARCH_ARM64
    /* ── ARM64 code generation ────────────────────────────────────────── */
    CG cg;
    memset(&cg, 0, sizeof(cg));
    cg.out       = fopen(outpath, "w");
    if (!cg.out) { perror("codegen: fopen"); return 1; }
    cg.opt_level = g_opt_level;
    cg.verbose   = g_verbose_asm;

    cg.a64_spill_depth = 0;
    cg.a64_sp_adj      = 0;

    /* pre-pass: collect all declared function names for call-site dispatch */
    collect_fn_names(&cg, program);
    collect_variant_names(&cg, program);
    int n_top  = count_locals(program) + 16;
    /* main frame: locals + spill headroom (16 slots) + x29/x30, 16-aligned.
     * The +16 gives enough room for simultaneous spills at any expression depth.
     * safe_str_a64/safe_ldr_a64 handle encoding for any offset within the frame. */
    int locals  = (n_top * 8 + 15) & ~15;
    int mframe  = locals + 16;
    mframe = (mframe + 15) & ~15;
    cg.a64_frame_size = mframe;  /* used by NODE_RETURN in top-level fns */
    int main_fp_adj = mframe - 16; /* x29 = sp + main_fp_adj */

    emit(&cg, XLY_TEXT_SECTION);
    emit(&cg, ".globl  " XLY_SYM("main"));
    emit(&cg, XLY_SYM("main") ":");
    /* Same frame layout as emit_function_a64: allocate first, save pair at
     * TOP of frame, set x29 to saved-pair address so that locals at
     * [x29, #-8], [x29, #-16], ... remain within [sp, old_sp).            */
    emit(&cg, "    sub     sp, sp, #%d", mframe);
    if (main_fp_adj <= 504) {
        emit(&cg, "    stp     x29, x30, [sp, #%d]", main_fp_adj);
    } else {
        emit(&cg, "    add     x9, sp, #%d", main_fp_adj);
        emit(&cg, "    stp     x29, x30, [x9]");
    }
    emit(&cg, "    add     x29, sp, #%d", main_fp_adj);

    cg.in_main_scope = 1;
    for (size_t i = 0; i < program->child_count; i++)
        emit_stmt_a64(&cg, program->children[i]);
    cg.in_main_scope = 0;

    /* call xly_exit(0) -- unreachable epilogue follows for completeness */
    emit(&cg, "    mov     w0, #0");
    emit(&cg, "    bl      " XLY_SYM("xly_exit"));
    emit(&cg, "    sub     sp, x29, #%d", main_fp_adj);
    if (main_fp_adj <= 504) {
        emit(&cg, "    ldp     x29, x30, [sp, #%d]", main_fp_adj);
    } else {
        emit(&cg, "    add     x9, sp, #%d", main_fp_adj);
        emit(&cg, "    ldp     x29, x30, [x9]");
    }
    emit(&cg, "    add     sp, sp, #%d", mframe);
    emit(&cg, "    ret");

    /* user-defined functions */
    for (int i = 0; i < cg.func_count; i++)
        emit_function_a64(&cg, cg.funcs[i].node, cg.funcs[i].asm_label,
                          cg.funcs[i].rename_ctx, cg.funcs[i].rename_ctx_count,
                          cg.funcs[i].captures, cg.funcs[i].ncaptures);

    /* string literals section */
    emit(&cg, "");
    emit(&cg, XLY_DATA_SECTION);
    for (int i = 0; i < cg.str_count; i++) {
        emit(&cg, "%s:", cg.strings[i].label);
        fprintf(cg.out, "    .asciz  \"");
        for (const char *s = cg.strings[i].text; *s; s++) {
            unsigned char ch = (unsigned char)*s;
            switch (ch) {
                case '\n': fputs("\\n",  cg.out); break;
                case '\t': fputs("\\t",  cg.out); break;
                case '\r': fputs("\\r",  cg.out); break;
                case '\\': fputs("\\\\", cg.out); break;
                case '"':  fputs("\\\"", cg.out); break;
                default:
                    if (ch >= 0x80 || ch < 0x20)
                        fprintf(cg.out, "\\%03o", ch);
                    else
                        fputc(ch, cg.out);
                    break;
            }
        }
        fputs("\"\n", cg.out);
    }

    /* Global variable storage for ARM64 */
    if (cg.gvar_count > 0) {
        emit(&cg, "");
        emit(&cg, ".section __DATA,__bss");
        emit(&cg, ".globl  " XLY_SYM("__xly_globals"));
        emit(&cg, ".balign 8");
        emit(&cg, XLY_SYM("__xly_globals") ":");
        emit(&cg, "    .zero   %d", cg.gvar_count * 8);
    }

    fclose(cg.out);

    for (int i = 0; i < cg.var_count; i++) free(cg.vars[i].name);
    free(cg.vars);
    for (int i = 0; i < cg.gvar_count; i++) free(cg.gvars[i].name);
    free(cg.gvars);
    for (int i = 0; i < cg.str_count; i++) { free(cg.strings[i].text); free(cg.strings[i].label); }
    free(cg.strings);
    for (int i = 0; i < cg.func_count; i++) {
        free(cg.funcs[i].asm_label);
        fn_rename_ctx_free(cg.funcs[i].rename_ctx, cg.funcs[i].rename_ctx_count);
    }
    free(cg.funcs);
    for (int i = 0; i < cg.fn_name_count; i++) free(cg.fn_names[i]);
    free(cg.fn_names);
    for (int i = 0; i < cg.fn_labels_used_count; i++) free(cg.fn_labels_used[i].lbl);
    free(cg.fn_labels_used);
    for (int i = 0; i < cg.fn_rename_count; i++) { free(cg.fn_renames[i].src); free(cg.fn_renames[i].lbl); }
    free(cg.fn_renames);
    for (int i = 0; i < cg.variant_count; i++) free(cg.variants[i].name);
    free(cg.variants);
    for (int i = 0; i < cg.brk_top; i++) free(cg.brk_labels[i]);
    free(cg.brk_labels);
    for (int i = 0; i < cg.cnt_top; i++) free(cg.cnt_labels[i]);
    free(cg.cnt_labels);

    return cg.had_error;

#else
    /* ── x86-64 code generation (original) ───────────────────────────── */
    return codegen_x86_64(program, outpath);
#endif
}
