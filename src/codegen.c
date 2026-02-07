/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
/*
 * codegen.c  —  Xenly AST  →  x86-64 AT&T assembly (System V AMD64 ABI)
 *
 * Design contract:
 *   • Every call to emit_expr() leaves exactly one XlyVal* in %rax.
 *   • Every call to emit_stmt() leaves the stack pointer unchanged from
 *     where it was on entry (push/pop discipline).
 *   • Before ANY `call` instruction %rsp must be 16-byte aligned.  Our
 *     prologue does `push %rbp; sub $N, %rsp` where N is rounded to 16,
 *     so %rsp is 16-aligned inside the frame.  A single pushq misaligns;
 *     we therefore never leave a lone push dangling at a call site — we
 *     always pair pushes with pops, or use sub/add.
 *   • The red zone (128 bytes below %rsp) is NOT used; we never write
 *     below %rsp.
 *
 * Supported AST nodes:
 *   Literals    NUMBER STRING BOOL NULL
 *   Vars        VAR_DECL ASSIGN COMPOUND_ASSIGN INCREMENT DECREMENT IDENTIFIER
 *   Exprs       BINARY UNARY FN_CALL METHOD_CALL TYPEOF
 *   Stmts       PRINT IF WHILE FOR FOR_IN BREAK CONTINUE RETURN BLOCK
 *   Decls       FN_DECL IMPORT (import is a no-op; modules are linked)
 */

#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

/* ── codegen state ──────────────────────────────────────────────────────── */
typedef struct {
    FILE   *out;
    int     label_seq;          /* monotonic label counter */

    /* variable table: name + rbp-offset + scope-depth */
    struct { char *name; int offset; int depth; } *vars;
    int     var_count, var_cap;

    int     scope_depth;
    int     frame_offset;       /* next free slot (negative, grows down) */

    /* interned string literals → .rodata labels */
    struct { char *text; char *label; } *strings;
    int     str_count, str_cap;

    /* break / continue label stacks */
    char   **brk_labels;  int brk_top;
    char   **cnt_labels;  int cnt_top;

    /* collected fn declarations, emitted after main */
    struct { ASTNode *node; } *funcs;
    int     func_count, func_cap;

    int     had_error;
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

/* ── forward declarations ──────────────────────────────────────────────── */
static void emit_expr(CG *cg, ASTNode *node);
static void emit_stmt(CG *cg, ASTNode *node);

/* ── pre-pass: count VAR_DECLs recursively ─────────────────────────────── */
static int count_locals(ASTNode *node) {
    if (!node) return 0;
    int n = (node->type == NODE_VAR_DECL) ? 1 : 0;
    /* for-in declares 4 hidden slots per loop; count conservatively */
    if (node->type == NODE_FOR_IN) n += 4;
    for (size_t i = 0; i < node->child_count; i++)
        n += count_locals(node->children[i]);
    return n;
}

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
    if (!node) { emit(cg, "    call    xly_null"); return; }

    switch (node->type) {

    /* ── literals ────────────────────────────────────────────────────── */
    case NODE_NUMBER:
        emit_load_double(cg, node->num_value);
        emit(cg, "    call    xly_num");
        break;

    case NODE_STRING: {
        const char *lbl = intern_string(cg, node->str_value ? node->str_value : "");
        emit(cg, "    leaq    %s(%%rip), %%rdi", lbl);
        emit(cg, "    call    xly_str");
        break;
    }

    case NODE_BOOL:
        emit(cg, "    movl    $%d, %%edi", node->bool_value ? 1 : 0);
        emit(cg, "    call    xly_bool");
        break;

    case NODE_NULL:
        emit(cg, "    call    xly_null");
        break;

    /* ── identifier ──────────────────────────────────────────────────── */
    case NODE_IDENTIFIER: {
        int off = var_offset(cg, node->str_value);
        if (off != 0)
            emit(cg, "    movq    %d(%%rbp), %%rax", off);
        else
            emit(cg, "    call    xly_null");   /* undefined → null */
        break;
    }

    /* ── binary ──────────────────────────────────────────────────────── */
    case NODE_BINARY: {
        const char *op = node->str_value;

        /* ── short-circuit: and ──
         * eval left → if falsy jump to end (result = left)
         * eval right → result = right                          */
        if (strcmp(op, "and") == 0) {
            char lbl_end[64];
            fresh_label(cg, lbl_end, sizeof(lbl_end));

            emit_expr(cg, node->children[0]);          /* left → %rax */
            /* spill left to stack; test truthiness; branch */
            emit(cg, "    pushq   %%rax");             /* [left] on stack */
            emit(cg, "    movq    %%rax, %%rdi");
            emit(cg, "    call    xly_truthy");        /* eax = 0|1 */
            emit(cg, "    testl   %%eax, %%eax");
            emit(cg, "    jz      %s", lbl_end);       /* falsy → keep left */
            /* truthy: discard left, eval right */
            emit(cg, "    addq    $8, %%rsp");         /* pop left (discard) */
            emit_expr(cg, node->children[1]);          /* right → %rax */
            emit(cg, "    jmp     .Lxly_and_done_%d", cg->label_seq);
            emit(cg, "%s:", lbl_end);                  /* falsy exit: left still on stack */
            emit(cg, "    popq    %%rax");             /* restore left into %rax */
            emit(cg, ".Lxly_and_done_%d:", cg->label_seq++);
            break;
        }

        /* ── short-circuit: or ──
         * eval left → if truthy jump to end (result = left)
         * eval right → result = right                          */
        if (strcmp(op, "or") == 0) {
            char lbl_end[64];
            fresh_label(cg, lbl_end, sizeof(lbl_end));

            emit_expr(cg, node->children[0]);
            emit(cg, "    pushq   %%rax");
            emit(cg, "    movq    %%rax, %%rdi");
            emit(cg, "    call    xly_truthy");
            emit(cg, "    testl   %%eax, %%eax");
            emit(cg, "    jnz     %s", lbl_end);       /* truthy → keep left */
            emit(cg, "    addq    $8, %%rsp");
            emit_expr(cg, node->children[1]);
            emit(cg, "    jmp     .Lxly_or_done_%d", cg->label_seq);
            emit(cg, "%s:", lbl_end);
            emit(cg, "    popq    %%rax");
            emit(cg, ".Lxly_or_done_%d:", cg->label_seq++);
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
                emit(cg, "    call    xly_num");
            } else if (is_plus) {
                /* For +, check types at runtime and fall back to xly_add if needed */
                char lbl_slow[64], lbl_end[64];
                fresh_label(cg, lbl_slow, sizeof(lbl_slow));
                fresh_label(cg, lbl_end, sizeof(lbl_end));
                
                /* Eval both operands */
                emit_expr(cg, node->children[0]);
                emit(cg, "    pushq   %%rax");  /* save left */
                emit_expr(cg, node->children[1]);
                emit(cg, "    movq    %%rax, %%rsi");  /* rsi = right */
                emit(cg, "    popq    %%rdi");          /* rdi = left */
                
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
                emit(cg, "    call    xly_num");
                emit(cg, "    jmp     %s", lbl_end);
                
                /* Slow path: call xly_add for string concat or mixed types */
                emit(cg, "%s:", lbl_slow);
                emit(cg, "    call    xly_add");
                
                emit(cg, "%s:", lbl_end);
            } else {
                /* For -, *, /: always use unboxed path (no string operations) */
                
                /* Eval left operand → %rax (XlyVal*) */
                emit_expr(cg, node->children[0]);
                emit(cg, "    pushq   %%rax");  /* save left boxed value */
                
                /* Eval right operand → %rax (XlyVal*) */
                emit_expr(cg, node->children[1]);
                emit(cg, "    movq    %%rax, %%rsi");  /* rsi = right boxed */
                emit(cg, "    popq    %%rdi");          /* rdi = left boxed */
                
                /* Unbox both */
                emit(cg, "    movsd   8(%%rdi), %%xmm0");
                emit(cg, "    movsd   8(%%rsi), %%xmm1");
                
                /* Perform arithmetic */
                if      (strcmp(op,"-") == 0) emit(cg, "    subsd   %%xmm1, %%xmm0");
                else if (strcmp(op,"*") == 0) emit(cg, "    mulsd   %%xmm1, %%xmm0");
                else if (strcmp(op,"/") == 0) emit(cg, "    divsd   %%xmm1, %%xmm0");
                
                /* Box result */
                emit(cg, "    call    xly_num");
            }
        } else {
            /* Slow path for other ops: eval, push, call runtime function */
            emit_expr(cg, node->children[0]);          /* left  → %rax */
            emit(cg, "    pushq   %%rax");             /* spill left */
            emit_expr(cg, node->children[1]);          /* right → %rax */
            emit(cg, "    movq    %%rax, %%rsi");      /* rsi = right */
            emit(cg, "    popq    %%rdi");             /* rdi = left  */

            const char *fn = NULL;
            if      (strcmp(op,"%")  == 0) fn = "xly_mod";
            else if (strcmp(op,"==") == 0) fn = "xly_eq";
            else if (strcmp(op,"!=") == 0) fn = "xly_neq";
            else if (strcmp(op,"<")  == 0) fn = "xly_lt";
            else if (strcmp(op,">")  == 0) fn = "xly_gt";
            else if (strcmp(op,"<=") == 0) fn = "xly_lte";
            else if (strcmp(op,">=") == 0) fn = "xly_gte";

            if (fn) emit(cg, "    call    %s", fn);
        }
        break;
    }

    /* ── unary ─────────────────────────────────────────────────────── */
    case NODE_UNARY:
        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %%rdi");
        emit(cg, "    call    %s",
             strcmp(node->str_value, "-") == 0 ? "xly_neg" : "xly_not");
        break;

    /* ── user function call ──────────────────────────────────────────
     * Eval args left-to-right, push each.  Then pop into SysV regs
     * in the right order.  Up to 6 args supported.                    */
    case NODE_FN_CALL: {
        int nargs = (int)node->child_count;
        for (int i = 0; i < nargs; i++) {
            emit_expr(cg, node->children[i]);
            emit(cg, "    pushq   %%rax");
        }
        /* stack (top→bottom): argN-1 … arg1 arg0
         * pop in order: first pop = argN-1 → sysv_regs[N-1], …, last pop = arg0 → rdi */
        static const char *regs[] = {"rdi","rsi","rdx","rcx","r8","r9"};
        for (int i = nargs - 1; i >= 0; i--)
            emit(cg, "    popq    %%%s", regs[i]);
        emit(cg, "    call    .Lxly_fn_%s", node->str_value);
        break;
    }

    /* ── module method call  (mod.fn(args…)) ─────────────────────────
     * Signature: xly_call_module(const char *mod, const char *fn,
     *                            XlyVal **args, size_t argc)
     *            rdi             rsi             rdx              rcx
     *
     * We build the arg array on the stack (contiguous, first-arg at
     * lowest address).  Eval args left-to-right pushing each; then
     * compute the base pointer.                                        */
    case NODE_METHOD_CALL: {
        const char *mod_name = node->children[0]->str_value;
        const char *fn_name  = node->str_value;
        int argc = (int)node->child_count - 1;   /* children[1..] are args */

        /* Eval args and push in REVERSE order so arg[0] ends up at lowest address.
         * After all pushes, rsp points to arg[0], rsp+8 to arg[1], etc. */
        for (int i = argc; i >= 1; i--) {
            emit_expr(cg, node->children[i]);
            emit(cg, "    pushq   %%rax");
        }
        /* After argc pushes, rsp points to arg[0] */
        int pad = (argc & 1) ? 8 : 0;
        if (pad) emit(cg, "    subq    $8, %%rsp");

        /* rdx = &args[0] = rsp + pad */
        if (argc > 0) {
            emit(cg, "    movq    %%rsp, %%rdx");
            if (pad) emit(cg, "    addq    $%d, %%rdx", pad);
        } else {
            emit(cg, "    xorq    %%rdx, %%rdx");  /* NULL */
        }

        const char *ml = intern_string(cg, mod_name);
        const char *fl = intern_string(cg, fn_name);
        emit(cg, "    leaq    %s(%%rip), %%rdi", ml);
        emit(cg, "    leaq    %s(%%rip), %%rsi", fl);
        emit(cg, "    movl    $%d, %%ecx", argc);
        emit(cg, "    call    xly_call_module");

        /* clean up: argc*8 + pad */
        int cleanup = argc * 8 + pad;
        if (cleanup) emit(cg, "    addq    $%d, %%rsp", cleanup);
        break;
    }

    /* ── typeof ────────────────────────────────────────────────────── */
    case NODE_TYPEOF:
        emit_expr(cg, node->children[0]);
        emit(cg, "    movq    %%rax, %%rdi");
        emit(cg, "    call    xly_typeof");
        break;

    /* ── increment / decrement as expression ───────────────────────── */
    case NODE_INCREMENT:
    case NODE_DECREMENT: {
        int off = var_offset(cg, node->str_value);
        /* load current value */
        emit(cg, "    movq    %d(%%rbp), %%rdi", off);
        /* push it (spill across the xly_num call) */
        emit(cg, "    pushq   %%rdi");
        /* build xly_num(1.0) */
        emit_load_double(cg, 1.0);
        emit(cg, "    call    xly_num");          /* rax = num(1) */
        emit(cg, "    movq    %%rax, %%rsi");     /* rsi = num(1) */
        emit(cg, "    popq    %%rdi");            /* rdi = original value */
        emit(cg, "    call    %s",
             node->type == NODE_INCREMENT ? "xly_add" : "xly_sub");
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        break;
    }

    /* ── fallback ──────────────────────────────────────────────────── */
    default:
        emit(cg, "    call    xly_null");
        break;
    }
}

/* ── statement compiler ─────────────────────────────────────────────────
 * Post-condition: %rsp unchanged from entry.                             */
static void emit_stmt(CG *cg, ASTNode *node) {
    if (!node) return;

    switch (node->type) {

    /* ── var decl ──────────────────────────────────────────────────── */
    case NODE_VAR_DECL: {
        int off = var_declare(cg, node->str_value);
        if (node->child_count > 0)
            emit_expr(cg, node->children[0]);
        else
            emit(cg, "    call    xly_null");
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
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

        /* load current value, spill across rhs eval */
        emit(cg, "    movq    %d(%%rbp), %%rax", off);
        emit(cg, "    pushq   %%rax");             /* [current] */
        emit_expr(cg, node->children[1]);          /* rhs → %rax */
        emit(cg, "    movq    %%rax, %%rsi");      /* rsi = rhs */
        emit(cg, "    popq    %%rdi");             /* rdi = current */

        const char *op  = node->str_value;         /* "+=", "-=", "*=", "/=" */
        const char *fn  = "xly_add";
        if      (strcmp(op, "+=") == 0) fn = "xly_add";
        else if (strcmp(op, "-=") == 0) fn = "xly_sub";
        else if (strcmp(op, "*=") == 0) fn = "xly_mul";
        else if (strcmp(op, "/=") == 0) fn = "xly_div";

        emit(cg, "    call    %s", fn);
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        break;
    }

    /* ── increment / decrement as statement ────────────────────────── */
    case NODE_INCREMENT:
    case NODE_DECREMENT: {
        int off = var_offset(cg, node->str_value);
        emit(cg, "    movq    %d(%%rbp), %%rdi", off);
        emit(cg, "    pushq   %%rdi");
        emit_load_double(cg, 1.0);
        emit(cg, "    call    xly_num");
        emit(cg, "    movq    %%rax, %%rsi");
        emit(cg, "    popq    %%rdi");
        emit(cg, "    call    %s",
             node->type == NODE_INCREMENT ? "xly_add" : "xly_sub");
        emit(cg, "    movq    %%rax, %d(%%rbp)", off);
        break;
    }

    /* ── print ────────────────────────────────────────────────────── */
    case NODE_PRINT: {
        /*
         * xly_print(XlyVal **vals, size_t n)   — rdi, rsi
         *
         * Push args in REVERSE order so arg[0] lands at rsp (lowest
         * address).  Layout after all pushes:
         *   rsp+0:       arg[0]   ← rdi
         *   rsp+8:       arg[1]
         *   …
         *   rsp+(n-1)*8: arg[n-1]
         *
         * Alignment: n pushes + optional pad.  If n is odd we add a
         * dummy 8-byte sub first so that rsp is 16-aligned at call.
         */
        int n = (int)node->child_count;
        int pad = (n & 1) ? 1 : 0;
        if (pad) emit(cg, "    subq    $8, %%rsp");

        /* Eval and push in reverse order so arg[0] ends up at rsp */
        for (int i = n - 1; i >= 0; i--) {
            emit_expr(cg, node->children[i]);
            emit(cg, "    pushq   %%rax");
        }
        /* rsp now points directly to arg[0] */
        emit(cg, "    movq    %%rsp, %%rdi");
        emit(cg, "    movl    $%d, %%esi", n);
        emit(cg, "    call    xly_print");
        /* pop args + pad */
        int cleanup = n * 8 + (pad ? 8 : 0);
        if (cleanup) emit(cg, "    addq    $%d, %%rsp", cleanup);
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
        emit(cg, "    call    xly_truthy");
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
        emit(cg, "    call    xly_truthy");
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
            emit(cg, "    call    xly_truthy");
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
        emit(cg, "    call    xly_array_len");
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
        emit(cg, "    call    xly_array_get");
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
    case NODE_FN_DECL:
        if (cg->func_count >= cg->func_cap) {
            cg->func_cap = cg->func_cap ? cg->func_cap * 2 : 16;
            cg->funcs    = realloc(cg->funcs, sizeof(cg->funcs[0]) * (size_t)cg->func_cap);
        }
        cg->funcs[cg->func_count++].node = node;
        break;

    /* ── return ────────────────────────────────────────────────────── */
    case NODE_RETURN:
        if (node->child_count > 0)
            emit_expr(cg, node->children[0]);
        else
            emit(cg, "    call    xly_null");
        emit(cg, "    movq    %%rbp, %%rsp");
        emit(cg, "    popq    %%rbp");
        emit(cg, "    ret");
        break;

    /* ── import — no-op (stdlib linked statically) ────────────────── */
    case NODE_IMPORT:
        break;

    /* ── everything else — try as expression ─────────────────────── */
    default:
        emit_expr(cg, node);
        break;
    }
}

/* ── emit a user-defined function ──────────────────────────────────────── */
static void emit_function(CG *cg, ASTNode *fn) {
    const char *name    = fn->str_value;
    size_t      nparams = fn->param_count;
    ASTNode    *body    = fn->children[0];   /* NODE_BLOCK */

    /* save codegen frame state */
    int sv_vc = cg->var_count, sv_fo = cg->frame_offset, sv_sd = cg->scope_depth;
    cg->scope_depth  = 0;
    cg->frame_offset = 0;

    /* frame size: params + locals + generous padding, 16-aligned */
    int n = (int)nparams + count_locals(body) + 8;
    int frame = (n * 8 + 15) & ~15;

    emit(cg, "");
    emit(cg, ".Lxly_fn_%s:", name);
    emit(cg, "    pushq   %%rbp");
    emit(cg, "    movq    %%rsp, %%rbp");
    emit(cg, "    subq    $%d, %%rsp", frame);

    /* spill incoming SysV register args into stack slots */
    static const char *pregs[] = {"rdi","rsi","rdx","rcx","r8","r9"};
    for (size_t i = 0; i < nparams && i < 6; i++) {
        int off = var_declare(cg, fn->params[i].name);
        emit(cg, "    movq    %%%s, %d(%%rbp)", pregs[i], off);
    }

    /* body */
    scope_enter(cg);
    for (size_t i = 0; i < body->child_count; i++)
        emit_stmt(cg, body->children[i]);
    scope_leave(cg);

    /* implicit return null */
    emit(cg, "    call    xly_null");
    emit(cg, "    movq    %%rbp, %%rsp");
    emit(cg, "    popq    %%rbp");
    emit(cg, "    ret");

    /* restore codegen state */
    while (cg->var_count > sv_vc) { cg->var_count--; free(cg->vars[cg->var_count].name); }
    cg->frame_offset = sv_fo;
    cg->scope_depth  = sv_sd;
}

/* ── public entry ───────────────────────────────────────────────────────── */
int codegen(ASTNode *program, const char *outpath) {
    CG cg;
    memset(&cg, 0, sizeof(cg));
    cg.out = fopen(outpath, "w");
    if (!cg.out) { perror("codegen: fopen"); return 1; }

    /* main frame: generous */
    int n_top = count_locals(program) + 16;
    int mframe = (n_top * 8 + 15) & ~15;

    emit(&cg, ".section .text");
    emit(&cg, ".globl  main");
    emit(&cg, "main:");
    emit(&cg, "    pushq   %%rbp");
    emit(&cg, "    movq    %%rsp, %%rbp");
    emit(&cg, "    subq    $%d, %%rsp", mframe);

    for (size_t i = 0; i < program->child_count; i++)
        emit_stmt(&cg, program->children[i]);

    emit(&cg, "    movl    $0, %%edi");
    emit(&cg, "    call    xly_exit");
    emit(&cg, "    movq    %%rbp, %%rsp");
    emit(&cg, "    popq    %%rbp");
    emit(&cg, "    ret");

    /* user functions */
    for (int i = 0; i < cg.func_count; i++)
        emit_function(&cg, cg.funcs[i].node);

    /* .rodata */
    emit(&cg, "");
    emit(&cg, ".section .rodata");
    for (int i = 0; i < cg.str_count; i++) {
        emit(&cg, "%s:", cg.strings[i].label);
        fprintf(cg.out, "    .asciz  \"");
        for (const char *s = cg.strings[i].text; *s; s++) {
            switch (*s) {
                case '\n': fputs("\\n",  cg.out); break;
                case '\t': fputs("\\t",  cg.out); break;
                case '\\': fputs("\\\\", cg.out); break;
                case '"':  fputs("\\\"", cg.out); break;
                default:   fputc(*s,     cg.out); break;
            }
        }
        fputs("\"\n", cg.out);
    }

    /* .note.GNU-stack — tell the linker the stack is NOT executable */
    emit(&cg, "");
    emit(&cg, ".section .note.GNU-stack,\"\",@progbits");

    fclose(cg.out);

    /* cleanup */
    for (int i = 0; i < cg.var_count; i++)    free(cg.vars[i].name);
    free(cg.vars);
    for (int i = 0; i < cg.str_count; i++)   { free(cg.strings[i].text); free(cg.strings[i].label); }
    free(cg.strings);
    free(cg.funcs);
    for (int i = 0; i < cg.brk_top; i++) free(cg.brk_labels[i]);
    free(cg.brk_labels);
    for (int i = 0; i < cg.cnt_top; i++) free(cg.cnt_labels[i]);
    free(cg.cnt_labels);

    return cg.had_error;
}