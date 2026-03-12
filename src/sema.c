/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 */
/*
 * sema.c  â€”  Xenly Semantic Analyzer
 *
 * See sema.h for the public API and a description of checks performed.
 */

#include "sema.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
 * DIAGNOSTICS
 * â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â• */

/* Diagnostic context â�� threaded through the entire walk */
typedef struct {
    const SemaOpts *opts;
    int errors;
    int warns;
} Diag;

static void sema_error(Diag *d, int line, const char *fmt, ...) {
    va_list ap;
    if (d->opts->color)
        fprintf(stderr, "\033[1;31m[xenlyc sema] error\033[0m"
                        " \033[2m(line %d)\033[0m: ", line);
    else
        fprintf(stderr, "[xenlyc sema] error (line %d): ", line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    d->errors++;
}

static void sema_warn(Diag *d, int line, const char *fmt, ...) {
    va_list ap;
    if (d->opts->color)
        fprintf(stderr, "\033[1;33m[xenlyc sema] warning\033[0m"
                        " \033[2m(line %d)\033[0m: ", line);
    else
        fprintf(stderr, "[xenlyc sema] warning (line %d): ", line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    d->warns++;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SYMBOL TABLE
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    SYM_VAR   = 0,   /* var / let — mutable                                  */
    SYM_CONST = 1,   /* const     — immutable                                */
    SYM_FN    = 2,   /* fn / async fn                                        */
    SYM_PARAM = 3,   /* function parameter                                   */
    SYM_CLASS = 4,   /* class declaration                                    */
    SYM_ENUM  = 5,   /* enum declaration                                     */
} SymKind;

typedef struct {
    char   *name;
    SymKind kind;
    int     line;      /* declaration line (for error messages)              */
    int     arity;     /* parameter count for SYM_FN; -1 for variadic/unknown*/
    int     min_arity; /* count of required params (those without defaults)  */
} Symbol;

typedef struct Scope {
    Symbol      *syms;
    int          count;
    int          cap;
    struct Scope *parent;
} Scope;

static Scope *scope_new(Scope *parent) {
    Scope *s = (Scope *)calloc(1, sizeof(Scope));
    s->cap    = 16;
    s->syms   = (Symbol *)malloc(sizeof(Symbol) * (size_t)s->cap);
    s->parent = parent;
    return s;
}

static void scope_free(Scope *s) {
    if (!s) return;
    for (int i = 0; i < s->count; i++)
        free(s->syms[i].name);
    free(s->syms);
    free(s);
}

/* Look up a name in this scope only (no parent walk). */
static Symbol *scope_find_local(Scope *s, const char *name) {
    for (int i = 0; i < s->count; i++)
        if (strcmp(s->syms[i].name, name) == 0)
            return &s->syms[i];
    return NULL;
}

/* Look up a name walking the entire scope chain. */
static Symbol *scope_lookup(Scope *s, const char *name) {
    for (Scope *c = s; c; c = c->parent) {
        Symbol *sym = scope_find_local(c, name);
        if (sym) return sym;
    }
    return NULL;
}

/* Declare a symbol in the current (innermost) scope.
 * Returns NULL on success, or a pointer to the conflicting symbol on
 * duplicate-in-same-scope.                                                   */
static Symbol *scope_declare(Scope *s, const char *name, SymKind kind,
                              int line, int arity, int min_arity) {
    Symbol *existing = scope_find_local(s, name);
    if (existing) return existing;   /* caller will report the error */

    if (s->count >= s->cap) {
        s->cap *= 2;
        s->syms = (Symbol *)realloc(s->syms, sizeof(Symbol) * (size_t)s->cap);
    }
    Symbol *sym = &s->syms[s->count++];
    sym->name      = strdup(name);
    sym->kind      = kind;
    sym->line      = line;
    sym->arity     = arity;
    sym->min_arity = min_arity;
    return NULL;   /* success */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WELL-KNOWN BUILTINS
 * These names are always in scope — checked separately from the scope chain
 * so that user declarations can freely shadow them without collision.
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char *k_builtins[] = {
    /* core */
    "print", "input", "len", "range", "typeof", "instanceof",
    /* type conversion */
    "number", "string", "bool", "int", "float",
    /* math */
    "abs", "floor", "ceil", "round", "sqrt", "pow", "min", "max",
    "sin", "cos", "tan", "log", "log2", "log10", "exp",
    /* string */
    "split", "join", "trim", "upper", "lower", "substr", "replace",
    "starts_with", "ends_with", "includes", "indexOf", "charAt",
    /* array / collection */
    "push", "pop", "shift", "unshift", "slice", "splice", "concat",
    "reverse", "sort", "filter", "map", "reduce", "forEach", "find",
    "findIndex", "every", "some", "flat", "flatMap",
    /* object */
    "keys", "values", "entries", "assign", "freeze",
    /* sys / async */
    "spawn", "await",
    /* common stdlib objects (used via dot notation) */
    "array", "sys", "math", "io", "json", "http", "net", "os",
    "fs", "path", "crypto", "re", "time", "datetime", "random",
    "process", "console",
    /* common globals */
    "parseInt", "parseFloat", "undefined", "NaN", "Infinity",
    NULL
};

static int is_builtin(const char *name) {
    for (int i = 0; k_builtins[i]; i++)
        if (strcmp(k_builtins[i], name) == 0) return 1;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * ANALYSIS CONTEXT
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    Diag        *diag;
    Scope       *scope;       /* current innermost scope                      */
    int          in_fn;       /* depth: 0 = top level, >0 = inside function   */
    int          in_loop;     /* depth: 0 = not in loop, >0 = inside loop     */
    int          in_switch;   /* depth: 0 = not in switch                     */
    int          has_return;  /* set when current block emits a return        */
} SemaCtx;

/* Forward declarations */
static void sema_stmt(SemaCtx *ctx, ASTNode *node);
static void sema_expr(SemaCtx *ctx, ASTNode *node);
static int  sema_block(SemaCtx *ctx, ASTNode *block);

/* ── scope helpers ────────────────────────────────────────────────────────── */

static void ctx_push_scope(SemaCtx *ctx) {
    ctx->scope = scope_new(ctx->scope);
}

static void ctx_pop_scope(SemaCtx *ctx) {
    Scope *old = ctx->scope;
    ctx->scope = old->parent;
    scope_free(old);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PASS 1 — HOIST TOP-LEVEL DECLARATIONS
 *
 * We collect fn / async fn / class / enum names before running the full
 * analysis pass so that forward calls and mutually recursive functions
 * resolve correctly without triggering "undeclared identifier" warnings.
 * ═══════════════════════════════════════════════════════════════════════════ */

static void hoist_toplevel(SemaCtx *ctx, ASTNode *program) {
    for (size_t i = 0; i < program->child_count; i++) {
        ASTNode *node = program->children[i];
        if (!node) continue;

        if (node->type == NODE_FN_DECL && node->str_value) {
            /* Count required params (those without default values) */
            int total  = (int)node->param_count;
            int req    = 0;
            for (int p = 0; p < total; p++)
                if (!node->params[p].default_value && !node->params[p].is_optional)
                    req++;
            scope_declare(ctx->scope, node->str_value,
                          SYM_FN, node->line, total, req);
        }
        else if (node->type == NODE_CLASS_DECL && node->str_value) {
            scope_declare(ctx->scope, node->str_value,
                          SYM_CLASS, node->line, -1, 0);
        }
        else if (node->type == NODE_ENUM_DECL && node->str_value) {
            scope_declare(ctx->scope, node->str_value,
                          SYM_ENUM, node->line, -1, 0);
            /* Also hoist variant names as zero-arg or any-arg constructors */
            for (size_t v = 0; v < node->child_count; v++) {
                ASTNode *vn = node->children[v];
                if (vn && vn->str_value)
                    scope_declare(ctx->scope, vn->str_value,
                                  SYM_FN, vn->line,
                                  (int)vn->param_count,
                                  (int)vn->param_count);
            }
        }
        /* Nested namespace / export — hoist inner fns shallowly */
        else if ((node->type == NODE_NAMESPACE ||
                  node->type == NODE_EXPORT) && node->str_value) {
            scope_declare(ctx->scope, node->str_value,
                          SYM_VAR, node->line, -1, 0);
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PASS 2 — FULL RECURSIVE ANALYSIS
 * ═══════════════════════════════════════════════════════════════════════════ */

/* ── expression analysis ──────────────────────────────────────────────────── */

static void sema_expr(SemaCtx *ctx, ASTNode *node) {
    if (!node) return;
    Diag *d = ctx->diag;

    switch (node->type) {

    /* Literals — nothing to check */
    case NODE_NUMBER:
    case NODE_STRING:
    case NODE_BOOL:
    case NODE_NULL:
        break;

    /* Identifier reference */
    case NODE_IDENTIFIER: {
        if (!node->str_value) break;
        if (d->opts->warn_undecl &&
            !scope_lookup(ctx->scope, node->str_value) &&
            !is_builtin(node->str_value)) {
            sema_warn(d, node->line,
                      "use of undeclared identifier '%s'", node->str_value);
        }
        break;
    }

    /* Binary / ternary / nullish */
    case NODE_BINARY:
    case NODE_TERNARY:
    case NODE_NULLISH:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    /* Unary */
    case NODE_UNARY:
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        break;

    /* typeof / instanceof */
    case NODE_TYPEOF:
    case NODE_INSTANCEOF:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    /* Array literal */
    case NODE_ARRAY_LITERAL:
    case NODE_TUPLE_LITERAL:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    /* Object literal */
    case NODE_OBJECT_LITERAL:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    /* Array / object index */
    case NODE_INDEX:
    case NODE_INDEX_ASSIGN:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    /* Property access */
    case NODE_PROPERTY_GET:
    case NODE_PROPERTY_SET:
        /* Eval the object; skip the field-name literal */
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        if (node->type == NODE_PROPERTY_SET && node->child_count > 1)
            sema_expr(ctx, node->children[1]);
        break;

    /* Named argument — just check the value expression */
    case NODE_NAMED_ARG:
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        break;

    /* Function call: name(args) */
    case NODE_FN_CALL: {
        if (!node->str_value) break;

        /* Resolve the callee */
        Symbol *sym = scope_lookup(ctx->scope, node->str_value);
        if (!sym && d->opts->warn_undecl && !is_builtin(node->str_value)) {
            sema_warn(d, node->line,
                      "call to undeclared function '%s'", node->str_value);
        }

        /* Arity check for known functions */
        int nargs = (int)node->child_count;
        /* Don't count named args toward positional arity */
        int positional = 0;
        for (size_t i = 0; i < node->child_count; i++)
            if (node->children[i] &&
                node->children[i]->type != NODE_NAMED_ARG)
                positional++;
        nargs = positional;

        if (sym && sym->kind == SYM_FN && sym->arity >= 0 &&
            d->opts->warn_arity) {
            if (nargs < sym->min_arity) {
                sema_warn(d, node->line,
                          "too few arguments to '%s': expected at least %d, got %d",
                          node->str_value, sym->min_arity, nargs);
            } else if (nargs > sym->arity) {
                sema_warn(d, node->line,
                          "too many arguments to '%s': expected at most %d, got %d",
                          node->str_value, sym->arity, nargs);
            }
        }

        /* Check argument expressions */
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;
    }

    /* Expression call: expr(args) */
    case NODE_CALL_EXPR: {
        /* children[0] = callee expression, children[1..n] = args */
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;
    }

    /* Method call: obj.method(args) */
    case NODE_METHOD_CALL: {
        /* children[0] = object, children[1..n] = args */
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;
    }

    /* new ClassName(args) */
    case NODE_NEW: {
        /* Check class name is known */
        if (node->str_value && d->opts->warn_undecl &&
            !is_builtin(node->str_value)) {
            Symbol *sym = scope_lookup(ctx->scope, node->str_value);
            if (!sym) {
                sema_warn(d, node->line,
                          "use of undeclared class '%s'", node->str_value);
            }
        }
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;
    }

    /* this / super */
    case NODE_THIS:
        if (ctx->in_fn == 0)
            sema_warn(d, node->line,
                      "'this' used outside a method or function");
        break;

    case NODE_SUPER_CALL:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    /* Arrow function: (params) => expr */
    case NODE_ARROW_FN: {
        ctx_push_scope(ctx);
        /* Declare parameters */
        for (size_t i = 0; i < node->param_count; i++)
            scope_declare(ctx->scope, node->params[i].name,
                          SYM_PARAM, node->line, -1, 0);
        ctx->in_fn++;
        if (node->child_count > 0) {
            ASTNode *body = node->children[0];
            if (body->type == NODE_BLOCK) {
                /* Block body: process as statements so declarations register */
                sema_block(ctx, body);
            } else {
                /* Expression body: (params) => expr */
                sema_expr(ctx, body);
            }
        }
        ctx->in_fn--;
        ctx_pop_scope(ctx);
        break;
    }

    /* await expr */
    case NODE_AWAIT:
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        break;

    /* spawn expr */
    case NODE_SPAWN:
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        break;

    /* Namespace access  Namespace.member */
    case NODE_NAMESPACE_ACCESS:
        /* children[0] may be the namespace expression */
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        break;

    /* match can appear in expression position (e.g. return match …)
     * Delegate to sema_stmt which has the full scope+binding logic.     */
    case NODE_MATCH:
    case NODE_MATCH_ARM:
        sema_stmt(ctx, node);
        break;

    /* fn declaration and block in expression position (object literal
     * method values, closures-as-values, etc.) — delegate to sema_stmt
     * so that const/var declarations inside the body are properly
     * registered in scope.                                               */
    case NODE_FN_DECL:
        sema_stmt(ctx, node);
        break;

    case NODE_BLOCK: {
        ctx_push_scope(ctx);
        sema_block(ctx, node);
        ctx_pop_scope(ctx);
        break;
    }

    default:
        /* Catch-all: recurse into any children */
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;
    }
}

/* ── block analysis (returns 1 if the block always terminates) ──────────── */

/* ── statement analysis ───────────────────────────────────────────────────── */

static void sema_stmt(SemaCtx *ctx, ASTNode *node) {
    if (!node) return;
    Diag *d = ctx->diag;

    switch (node->type) {

    /* ── variable / constant declaration ───────────────────────────────── */
    case NODE_VAR_DECL:
    case NODE_LET_DECL:
    case NODE_CONST_DECL: {
        if (!node->str_value) break;
        SymKind kind = (node->type == NODE_CONST_DECL) ? SYM_CONST : SYM_VAR;

        /* Analyse initialiser before declaring the name (no self-reference) */
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);

        Symbol *conflict = scope_declare(ctx->scope, node->str_value,
                                         kind, node->line, -1, 0);
        if (conflict) {
            /* const re-declaration in same scope is always an error;
             * var/let shadowing in same scope is a warning only         */
            if (kind == SYM_CONST && conflict->kind == SYM_CONST) {
                sema_error(d, node->line,
                           "duplicate const declaration of '%s' "
                           "(previously declared at line %d)",
                           node->str_value, conflict->line);
            } else {
                sema_warn(d, node->line,
                          "redeclaration of '%s' shadows earlier declaration "
                          "at line %d",
                          node->str_value, conflict->line);
            }
        }
        break;
    }

    /* ── assignment ─────────────────────────────────────────────────────── */
    case NODE_ASSIGN: {
        /* Check that we are not reassigning a const */
        if (node->str_value) {
            Symbol *sym = scope_lookup(ctx->scope, node->str_value);
            if (sym && sym->kind == SYM_CONST) {
                sema_error(d, node->line,
                           "cannot reassign const variable '%s' "
                           "(declared at line %d)",
                           node->str_value, sym->line);
            }
        }
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;
    }

    /* ── compound assign / increment / decrement ────────────────────────── */
    case NODE_COMPOUND_ASSIGN: {
        if (node->str_value) {
            Symbol *sym = scope_lookup(ctx->scope, node->str_value);
            if (sym && sym->kind == SYM_CONST) {
                sema_error(d, node->line,
                           "cannot modify const variable '%s'", node->str_value);
            }
        }
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;
    }

    case NODE_INCREMENT:
    case NODE_DECREMENT: {
        if (node->str_value) {
            Symbol *sym = scope_lookup(ctx->scope, node->str_value);
            if (sym && sym->kind == SYM_CONST) {
                sema_error(d, node->line,
                           "cannot %s const variable '%s'",
                           node->type == NODE_INCREMENT ? "increment" : "decrement",
                           node->str_value);
            }
        }
        break;
    }

    /* ── function declaration ───────────────────────────────────────────── */
    case NODE_FN_DECL: {
        if (!node->str_value) break;

        /* In nested scopes (not top level) we may not have hoisted yet */
        if (ctx->scope->parent != NULL) {
            /* Only declare if not already present (top-level hoist may have
             * already registered it; nested fn declarations may shadow)      */
            int total  = (int)node->param_count;
            int req    = 0;
            for (int p = 0; p < total; p++)
                if (!node->params[p].default_value && !node->params[p].is_optional)
                    req++;
            Symbol *conflict = scope_declare(ctx->scope, node->str_value,
                                              SYM_FN, node->line, total, req);
            if (conflict && conflict->line != node->line) {
                sema_error(d, node->line,
                           "duplicate function declaration '%s' "
                           "(previously declared at line %d)",
                           node->str_value, conflict->line);
            }
        }

        /* Open a new scope for the function body */
        ctx_push_scope(ctx);

        /* Check for duplicate parameter names */
        for (size_t i = 0; i < node->param_count; i++) {
            const char *pname = node->params[i].name;
            if (!pname) continue;
            Symbol *conflict = scope_declare(ctx->scope, pname,
                                              SYM_PARAM, node->line, -1, 0);
            if (conflict) {
                sema_error(d, node->line,
                           "duplicate parameter name '%s' in function '%s'",
                           pname, node->str_value ? node->str_value : "<anon>");
            }
            /* Check default value expressions in enclosing scope */
            if (node->params[i].default_value)
                sema_expr(ctx, node->params[i].default_value);
        }

        ctx->in_fn++;
        if (node->child_count > 0)
            sema_block(ctx, node->children[0]);
        ctx->in_fn--;

        ctx_pop_scope(ctx);
        break;
    }

    /* ── return ─────────────────────────────────────────────────────────── */
    case NODE_RETURN: {
        if (ctx->in_fn == 0) {
            sema_error(d, node->line,
                       "'return' outside of a function");
        }
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        ctx->has_return = 1;
        break;
    }

    /* ── break ──────────────────────────────────────────────────────────── */
    case NODE_BREAK: {
        if (ctx->in_loop == 0 && ctx->in_switch == 0) {
            sema_error(d, node->line,
                       "'break' outside of a loop or switch");
        }
        break;
    }

    /* ── continue ───────────────────────────────────────────────────────── */
    case NODE_CONTINUE: {
        if (ctx->in_loop == 0) {
            sema_error(d, node->line,
                       "'continue' outside of a loop");
        }
        break;
    }

    /* ── if ─────────────────────────────────────────────────────────────── */
    case NODE_IF: {
        /* children: [cond, then_block, else_block?] */
        if (node->child_count > 0) sema_expr(ctx, node->children[0]);
        if (node->child_count > 1) {
            ctx_push_scope(ctx);
            sema_block(ctx, node->children[1]);
            ctx_pop_scope(ctx);
        }
        if (node->child_count > 2) {
            ctx_push_scope(ctx);
            sema_block(ctx, node->children[2]);
            ctx_pop_scope(ctx);
        }
        break;
    }

    /* ── while ──────────────────────────────────────────────────────────── */
    case NODE_WHILE: {
        /* children: [cond, body] */
        if (node->child_count > 0) sema_expr(ctx, node->children[0]);
        if (node->child_count > 1) {
            ctx_push_scope(ctx);
            ctx->in_loop++;
            sema_block(ctx, node->children[1]);
            ctx->in_loop--;
            ctx_pop_scope(ctx);
        }
        break;
    }

    /* ── do-while ───────────────────────────────────────────────────────── */
    case NODE_DO_WHILE: {
        /* children: [body, cond] */
        if (node->child_count > 0) {
            ctx_push_scope(ctx);
            ctx->in_loop++;
            sema_block(ctx, node->children[0]);
            ctx->in_loop--;
            ctx_pop_scope(ctx);
        }
        if (node->child_count > 1) sema_expr(ctx, node->children[1]);
        break;
    }

    /* ── for (init; cond; update) ───────────────────────────────────────── */
    case NODE_FOR: {
        /* children: [init, cond, update, body] */
        ctx_push_scope(ctx);   /* loop has its own scope for init var */
        if (node->child_count > 0) sema_stmt(ctx, node->children[0]);
        if (node->child_count > 1) sema_expr(ctx, node->children[1]);
        if (node->child_count > 2) sema_stmt(ctx, node->children[2]);
        if (node->child_count > 3) {
            ctx->in_loop++;
            sema_block(ctx, node->children[3]);
            ctx->in_loop--;
        }
        ctx_pop_scope(ctx);
        break;
    }

    /* ── for-in ─────────────────────────────────────────────────────────── */
    case NODE_FOR_IN: {
        /* str_value = loop variable name; children: [iterable, body] */
        ctx_push_scope(ctx);
        if (node->str_value)
            scope_declare(ctx->scope, node->str_value,
                          SYM_VAR, node->line, -1, 0);
        if (node->child_count > 0) sema_expr(ctx, node->children[0]);
        if (node->child_count > 1) {
            ctx->in_loop++;
            sema_block(ctx, node->children[1]);
            ctx->in_loop--;
        }
        ctx_pop_scope(ctx);
        break;
    }

    /* ── switch ─────────────────────────────────────────────────────────── */
    case NODE_SWITCH: {
        /* children: [discriminant, case1, case2, ...] */
        if (node->child_count > 0) sema_expr(ctx, node->children[0]);
        ctx->in_switch++;
        for (size_t i = 1; i < node->child_count; i++) {
            ctx_push_scope(ctx);
            sema_block(ctx, node->children[i]);
            ctx_pop_scope(ctx);
        }
        ctx->in_switch--;
        break;
    }

    /* ── match ──────────────────────────────────────────────────────────── */
    case NODE_MATCH: {
        /* children: [discriminant, arm1, arm2, ...] */
        if (node->child_count > 0) sema_expr(ctx, node->children[0]);
        for (size_t i = 1; i < node->child_count; i++)
            sema_stmt(ctx, node->children[i]);
        break;
    }

    /* ── match arm ──────────────────────────────────────────────────────── */
    case NODE_MATCH_ARM: {
        /* children: [pattern, body] */
        ctx_push_scope(ctx);

        if (node->child_count > 0 && node->children[0] &&
            node->children[0]->type == NODE_PATTERN) {
            ASTNode *pat = node->children[0];

            /* Wildcard _ never binds */
            int is_wildcard = (pat->str_value &&
                               strcmp(pat->str_value, "_") == 0);

            if (!is_wildcard && pat->str_value) {
                /* ── Variant constructor pattern: Some(val), Ok(msg), … ──
                 * Binding variables are stored in pattern->params array.   */
                if (pat->param_count > 0) {
                    for (size_t p = 0; p < pat->param_count; p++) {
                        if (pat->params[p].name)
                            scope_declare(ctx->scope, pat->params[p].name,
                                          SYM_VAR, pat->line, -1, 0);
                    }
                } else {
                    /* ── Plain identifier pattern: binding variable or name ──
                     * Lowercase = binding variable.  Uppercase = enum variant
                     * used without parameters (e.g. None) — not a binding.  */
                    if (pat->str_value[0] >= 'a' && pat->str_value[0] <= 'z') {
                        Symbol *sym = scope_lookup(ctx->scope, pat->str_value);
                        if (!sym || sym->kind != SYM_FN)
                            scope_declare(ctx->scope, pat->str_value,
                                          SYM_VAR, pat->line, -1, 0);
                    }
                }
            }

            /* Children of a pattern node (tuple/struct destructure) */
            for (size_t i = 0; i < pat->child_count; i++) {
                ASTNode *c = pat->children[i];
                if (!c) continue;
                if (c->type == NODE_IDENTIFIER && c->str_value)
                    scope_declare(ctx->scope, c->str_value,
                                  SYM_VAR, c->line, -1, 0);
                else if (c->type == NODE_PATTERN && c->str_value &&
                         c->str_value[0] >= 'a' && c->str_value[0] <= 'z')
                    scope_declare(ctx->scope, c->str_value,
                                  SYM_VAR, c->line, -1, 0);
            }
        }

        /* Analyse body */
        if (node->child_count > 1)
            sema_stmt(ctx, node->children[1]);
        else if (node->child_count == 1 &&
                 node->children[0] &&
                 node->children[0]->type != NODE_PATTERN)
            sema_stmt(ctx, node->children[0]);

        ctx_pop_scope(ctx);
        break;
    }

    /* ── block ──────────────────────────────────────────────────────────── */
    case NODE_BLOCK: {
        ctx_push_scope(ctx);
        sema_block(ctx, node);
        ctx_pop_scope(ctx);
        break;
    }

    /* ── print ──────────────────────────────────────────────────────────── */
    case NODE_PRINT:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    /* ── import ─────────────────────────────────────────────────────────── */
    case NODE_IMPORT: {
        /* Declare the imported name(s) as any-typed variables */
        if (node->str_value)
            scope_declare(ctx->scope, node->str_value,
                          SYM_VAR, node->line, -1, 0);
        break;
    }

    /* ── export ─────────────────────────────────────────────────────────── */
    case NODE_EXPORT: {
        if (node->child_count > 0)
            sema_stmt(ctx, node->children[0]);
        break;
    }

    /* ── class ──────────────────────────────────────────────────────────── */
    case NODE_CLASS_DECL: {
        if (!node->str_value) break;
        /* Already hoisted at top level; only declare in nested scope */
        if (ctx->scope->parent != NULL) {
            Symbol *conflict = scope_declare(ctx->scope, node->str_value,
                                              SYM_CLASS, node->line, -1, 0);
            if (conflict) {
                sema_error(d, node->line,
                           "duplicate class declaration '%s' "
                           "(previously at line %d)",
                           node->str_value, conflict->line);
            }
        }
        /* Check methods — start from child 1 (child 0 = parent class ref) */
        for (size_t i = 1; i < node->child_count; i++)
            sema_stmt(ctx, node->children[i]);
        break;
    }

    /* ── enum ───────────────────────────────────────────────────────────── */
    case NODE_ENUM_DECL:
        /* Variants already hoisted; nothing more to check here */
        break;

    /* ── namespace ──────────────────────────────────────────────────────── */
    case NODE_NAMESPACE: {
        ctx_push_scope(ctx);
        for (size_t i = 0; i < node->child_count; i++)
            sema_stmt(ctx, node->children[i]);
        ctx_pop_scope(ctx);
        break;
    }

    /* ── expression statement ───────────────────────────────────────────── */
    case NODE_EXPR_STMT:
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        break;

    /* ── spawn / await as statements ────────────────────────────────────── */
    case NODE_SPAWN:
    case NODE_AWAIT:
        if (node->child_count > 0)
            sema_expr(ctx, node->children[0]);
        break;

    /* ── contracts ──────────────────────────────────────────────────────── */
    case NODE_REQUIRES:
    case NODE_ENSURES:
    case NODE_INVARIANT:
    case NODE_ASSERT:
        for (size_t i = 0; i < node->child_count; i++)
            sema_expr(ctx, node->children[i]);
        break;

    default:
        /* Generic: analyse any children we don't have a specific rule for */
        for (size_t i = 0; i < node->child_count; i++) {
            if (node->children[i])
                sema_stmt(ctx, node->children[i]);
        }
        break;
    }
}

/* ── block analysis ────────────────────────────────────────────────────────
 * Walks each statement in `block` (a NODE_BLOCK or NODE_PROGRAM).
 * Detects unreachable code after a terminating statement.
 * Returns 1 if the block definitely terminates (all paths exit).            */
static int sema_block(SemaCtx *ctx, ASTNode *block) {
    if (!block) return 0;
    Diag *d = ctx->diag;

    int terminated = 0;
    size_t start   = 0;
    size_t count   = block->child_count;

    /* For NODE_BLOCK the children are the statements directly.
     * For non-block nodes just treat the node itself as a single statement. */
    if (block->type != NODE_BLOCK && block->type != NODE_PROGRAM) {
        sema_stmt(ctx, block);
        return 0;
    }

    for (size_t i = start; i < count; i++) {
        ASTNode *stmt = block->children[i];
        if (!stmt) continue;

        if (terminated && d->opts->warn_unreach) {
            sema_warn(d, stmt->line,
                      "unreachable code after unconditional return/break/continue");
            /* Warn once per dead block — skip the rest */
            break;
        }

        /* Save and reset the has_return sentinel */
        int prev_hr = ctx->has_return;
        ctx->has_return = 0;

        sema_stmt(ctx, stmt);

        /* A statement terminates the block if it is one of:                 */
        if (stmt->type == NODE_RETURN   ||
            stmt->type == NODE_BREAK    ||
            stmt->type == NODE_CONTINUE) {
            terminated = 1;
        }

        ctx->has_return = prev_hr;
    }

    return terminated;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC API
 * ═══════════════════════════════════════════════════════════════════════════ */

SemaOpts sema_default_opts(void) {
    SemaOpts o;
    o.color        = 1;
    o.verbose      = 0;
    o.warn_undecl  = 1;
    o.warn_arity   = 1;
    o.warn_unreach = 1;
    return o;
}

SemaResult sema_analyze(ASTNode *program,
                        const SemaOpts *opts,
                        int *out_errors,
                        int *out_warns) {
    static const SemaOpts defaults_storage = {1,0,1,1,1};
    if (!opts) opts = &defaults_storage;

    Diag diag  = { opts, 0, 0 };
    SemaCtx ctx = { &diag, NULL, 0, 0, 0, 0 };

    /* ── Build global scope ──────────────────────────────────────────────── */
    ctx.scope = scope_new(NULL);

    /* NOTE: builtins are resolved via is_builtin() — not in the scope chain.
     * This allows user code to freely shadow/redeclare any builtin name.    */

    /* ── Pass 1: hoist top-level declarations ───────────────────────────── */
    if (program && (program->type == NODE_PROGRAM ||
                    program->type == NODE_BLOCK))
        hoist_toplevel(&ctx, program);

    /* ── Pass 2: full recursive analysis ───────────────────────────────── */
    if (program) {
        if (program->type == NODE_PROGRAM || program->type == NODE_BLOCK) {
            for (size_t i = 0; i < program->child_count; i++)
                sema_stmt(&ctx, program->children[i]);
        } else {
            sema_stmt(&ctx, program);
        }
    }

    /* ── Cleanup ─────────────────────────────────────────────────────────── */
    scope_free(ctx.scope);

    if (opts->verbose) {
        if (diag.errors == 0 && diag.warns == 0) {
            if (opts->color)
                fprintf(stderr, "\033[1;32m[xenlyc sema]\033[0m OK — "
                        "no issues found\n");
            else
                fprintf(stderr, "[xenlyc sema] OK — no issues found\n");
        } else {
            if (opts->color)
                fprintf(stderr,
                        "\033[1;36m[xenlyc sema]\033[0m "
                        "%d error(s), %d warning(s)\n",
                        diag.errors, diag.warns);
            else
                fprintf(stderr, "[xenlyc sema] %d error(s), %d warning(s)\n",
                        diag.errors, diag.warns);
        }
    }

    if (out_errors) *out_errors = diag.errors;
    if (out_warns)  *out_warns  = diag.warns;

    if (diag.errors > 0) return SEMA_ERROR;
    if (diag.warns  > 0) return SEMA_WARN;
    return SEMA_OK;
}
