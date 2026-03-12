/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 *
 */
#include "typecheck.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

/* ============================================================
 * Internal helpers
 * ============================================================ */

/* Safe strdup that never returns NULL (aborts on OOM). */
static char *xstrdup(const char *s) {
    if (!s) s = "any";
    char *p = strdup(s);
    if (!p) { fputs("typecheck: out of memory\n", stderr); abort(); }
    return p;
}

/* Formatted heap string – analogous to asprintf. */
static char *xsprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) return xstrdup("<fmt-error>");
    char *buf = (char *)malloc((size_t)n + 1);
    if (!buf) { fputs("typecheck: out of memory\n", stderr); abort(); }
    va_start(ap, fmt);
    vsnprintf(buf, (size_t)n + 1, fmt, ap);
    va_end(ap);
    return buf;
}

/* ============================================================
 * Type environment (public + internal)
 * ============================================================ */
struct TypeEnv {
    char        **names;
    char        **types;
    size_t        count;
    size_t        capacity;
    struct TypeEnv *parent;
};

TypeEnv *typeenv_create(TypeEnv *parent) {
    TypeEnv *env = (TypeEnv *)calloc(1, sizeof(TypeEnv));
    if (!env) { fputs("typecheck: out of memory\n", stderr); abort(); }
    env->capacity = 8;
    env->names    = (char **)malloc(sizeof(char *) * env->capacity);
    env->types    = (char **)malloc(sizeof(char *) * env->capacity);
    env->parent   = parent;
    return env;
}

void typeenv_destroy(TypeEnv *env) {
    if (!env) return;
    for (size_t i = 0; i < env->count; i++) {
        free(env->names[i]);
        free(env->types[i]);
    }
    free(env->names);
    free(env->types);
    free(env);
}

void typeenv_set(TypeEnv *env, const char *name, const char *type) {
    /* Update existing binding in this frame. */
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            free(env->types[i]);
            env->types[i] = xstrdup(type);
            return;
        }
    }
    /* Add new binding. */
    if (env->count >= env->capacity) {
        env->capacity *= 2;
        env->names = (char **)realloc(env->names, sizeof(char *) * env->capacity);
        env->types = (char **)realloc(env->types, sizeof(char *) * env->capacity);
    }
    env->names[env->count] = xstrdup(name);
    env->types[env->count] = xstrdup(type);
    env->count++;
}

const char *typeenv_get(TypeEnv *env, const char *name) {
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0)
            return env->types[i];
    }
    return env->parent ? typeenv_get(env->parent, name) : NULL;
}

/* Keep old internal aliases for backwards compat inside this file. */
static TypeEnv *type_env_create(TypeEnv *p) { return typeenv_create(p); }
static void     type_env_destroy(TypeEnv *e) { typeenv_destroy(e); }
static void     type_env_set(TypeEnv *e, const char *n, const char *t) { typeenv_set(e, n, t); }
static const char *type_env_get(TypeEnv *e, const char *n) { return typeenv_get(e, n); }

/* ============================================================
 * Diagnostic accumulator
 * ============================================================ */
typedef struct CheckCtx {
    TypeCheckMode  mode;
    TypeCheckResult *result;   /* NULL when using simple int-error path */
    int            *error_count_simple; /* used by simple path */
} CheckCtx;

static void emit_diag(CheckCtx *ctx, DiagSeverity sev, int line, const char *fmt, ...) {
    /* Build message string. */
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *msg = (char *)malloc((size_t)n + 1);
    va_start(ap, fmt);
    vsnprintf(msg, (size_t)n + 1, fmt, ap);
    va_end(ap);

    /* Choose ANSI color. */
    const char *color   = "\033[0m";
    const char *label   = "Note";
    if (sev == DIAG_WARNING) { color = "\033[1;33m"; label = "Type Warning"; }
    if (sev == DIAG_ERROR)   { color = "\033[1;31m"; label = "Type Error";   }

    fprintf(stderr, "%s[%s] Line %d: %s\033[0m\n", color, label, line, msg);

    /* Count / store. */
    if (ctx->result) {
        TypeDiag *d = (TypeDiag *)calloc(1, sizeof(TypeDiag));
        d->severity = sev;
        d->line     = line;
        d->message  = msg;
        /* Append to tail of linked list – walk to end. */
        TypeDiag **pp = &ctx->result->diags;
        while (*pp) pp = &(*pp)->next;
        *pp = d;

        if (sev == DIAG_WARNING) ctx->result->warning_count++;
        if (sev == DIAG_ERROR)   ctx->result->error_count++;
    } else {
        free(msg);
        if (sev == DIAG_ERROR && ctx->error_count_simple)
            (*ctx->error_count_simple)++;
    }
}

/* Decide severity based on mode; returns 0 if the diagnostic should be skipped. */
static int diag_severity(TypeCheckMode mode, int is_hard_error, DiagSeverity *out_sev) {
    if (mode == TYPECHECK_OFF) return 0;
    if (is_hard_error) {
        if (mode >= TYPECHECK_ERROR) { *out_sev = DIAG_ERROR;   return 1; }
        if (mode == TYPECHECK_WARN)  { *out_sev = DIAG_WARNING; return 1; }
        return 0;
    } else {
        /* soft warning */
        if (mode >= TYPECHECK_WARN)  { *out_sev = DIAG_WARNING; return 1; }
        return 0;
    }
}

/* ============================================================
 * Type compatibility / assignability
 * ============================================================ */

/*
 * Strip a trailing '?' to get the base type, e.g. "number?" → "number".
 * Returns a heap-allocated string; caller must free.
 */
static char *base_type(const char *t) {
    if (!t) return xstrdup("any");
    size_t len = strlen(t);
    if (len > 0 && t[len - 1] == '?') {
        char *b = (char *)malloc(len);
        memcpy(b, t, len - 1);
        b[len - 1] = '\0';
        return b;
    }
    return xstrdup(t);
}

static int is_nullable(const char *t) {
    if (!t) return 0;
    size_t len = strlen(t);
    return len > 0 && t[len - 1] == '?';
}

/*
 * Test membership in a union type string such as "number|string|bool".
 * Returns 1 if 'candidate' is one of the alternatives.
 */
static int union_contains(const char *union_type, const char *candidate) {
    if (!union_type || !candidate) return 0;
    char *copy = xstrdup(union_type);
    char *tok  = strtok(copy, "|");
    int found  = 0;
    while (tok) {
        /* Trim spaces. */
        while (*tok == ' ') tok++;
        char *end = tok + strlen(tok) - 1;
        while (end > tok && *end == ' ') { *end = '\0'; end--; }
        if (strcmp(tok, candidate) == 0) { found = 1; break; }
        tok = strtok(NULL, "|");
    }
    free(copy);
    return found;
}

int types_assignable(const char *expected, const char *actual) {
    if (!expected || !actual)                            return 1;
    if (strcmp(expected, "any") == 0)                    return 1;
    if (strcmp(actual,   "any") == 0)                    return 1;

    /* Nullable: "null" is assignable to "T?". */
    if (is_nullable(expected) && strcmp(actual, "null") == 0) return 1;

    /* Strip nullable wrapper before comparing bases. */
    char *exp_base = base_type(expected);
    char *act_base = base_type(actual);

    int result = 0;

    /* Union on expected side: actual must match one branch. */
    if (strchr(exp_base, '|')) {
        result = union_contains(exp_base, act_base);
    }
    /* Union on actual side: every branch must be assignable to expected.
       (conservative – acceptable for our purposes.) */
    else if (strchr(act_base, '|')) {
        result = union_contains(act_base, exp_base);
    }
    else {
        result = (strcmp(exp_base, act_base) == 0);
    }

    free(exp_base);
    free(act_base);
    return result;
}

/* Legacy alias used throughout older code paths. */
static int types_compatible(const char *expected, const char *actual) {
    return types_assignable(expected, actual);
}

/* ============================================================
 * Constraint satisfaction
 * ============================================================ */
int satisfies_constraint(const char *type, const char *constraint) {
    if (!constraint || strcmp(constraint, "any") == 0) return 1;
    if (!type)                                          return 1;

    /* Strip nullable wrapper for constraint checks. */
    char *t = base_type(type);
    int ok  = 0;

    if (strcmp(constraint, "Comparable") == 0) {
        ok = strcmp(t, "number") == 0 ||
             strcmp(t, "string") == 0 ||
             strcmp(t, "bool")   == 0;
    } else if (strcmp(constraint, "Numeric") == 0) {
        ok = strcmp(t, "number") == 0;
    } else if (strcmp(constraint, "Equatable") == 0) {
        ok = 1; /* All types support == / != */
    } else if (strcmp(constraint, "Hashable") == 0) {
        ok = strcmp(t, "number") == 0 ||
             strcmp(t, "string") == 0 ||
             strcmp(t, "bool")   == 0;
    } else if (strcmp(constraint, "Printable") == 0) {
        ok = 1; /* All types have a string representation */
    } else if (strcmp(constraint, "Iterable") == 0) {
        ok = (t[0] == '[');  /* Array types start with '[' */
    }
    /* Unknown constraint → warn but don't hard-fail here; handled at call site */

    free(t);
    return ok;
}

/* ============================================================
 * Generic type inference
 * ============================================================ */
void infer_generic_types(ASTNode   *fn_decl,
                         ASTNode  **args,
                         size_t     argc,
                         char    ***type_map_names,
                         char    ***type_map_types,
                         size_t    *map_size)
{
    if (!fn_decl || !fn_decl->type_params || fn_decl->type_param_count == 0) {
        *type_map_names = NULL;
        *type_map_types = NULL;
        *map_size       = 0;
        return;
    }

    size_t n = fn_decl->type_param_count;
    *type_map_names = (char **)malloc(sizeof(char *) * n);
    *type_map_types = (char **)malloc(sizeof(char *) * n);
    *map_size       = n;

    for (size_t i = 0; i < n; i++) {
        (*type_map_names)[i] = xstrdup(fn_decl->type_params[i].name);
        (*type_map_types)[i] = xstrdup("any");
    }

    /* Match declared parameter type annotations against argument node types. */
    for (size_t i = 0; i < argc && i < fn_decl->param_count; i++) {
        const char *param_ann = fn_decl->params[i].type_annotation;
        if (!param_ann) continue;

        /* Determine concrete type of this argument. */
        const char *arg_type = "any";
        if (args && args[i]) {
            switch (args[i]->type) {
                case NODE_NUMBER: arg_type = "number"; break;
                case NODE_STRING: arg_type = "string"; break;
                case NODE_BOOL:   arg_type = "bool";   break;
                case NODE_NULL:   arg_type = "null";   break;
                default:          arg_type = "any";    break;
            }
        }

        /* If the parameter annotation names a type parameter, bind it. */
        for (size_t j = 0; j < n; j++) {
            if (strcmp(param_ann, (*type_map_names)[j]) == 0) {
                /* Only update if still unresolved or same type. */
                if (strcmp((*type_map_types)[j], "any") == 0) {
                    free((*type_map_types)[j]);
                    (*type_map_types)[j] = xstrdup(arg_type);
                }
                break;
            }
        }
    }
}

char *resolve_type_param(const char  *raw_type,
                         char       **type_map_names,
                         char       **type_map_types,
                         size_t       map_size)
{
    if (!raw_type) return xstrdup("any");
    for (size_t i = 0; i < map_size; i++) {
        if (type_map_names[i] && strcmp(raw_type, type_map_names[i]) == 0) {
            return xstrdup(type_map_types[i] ? type_map_types[i] : "any");
        }
    }
    return xstrdup(raw_type);
}

/* ============================================================
 * Forward declaration for recursive infer_type
 * ============================================================ */
static char *infer_type(ASTNode *node, TypeEnv *env, CheckCtx *ctx);

/* ============================================================
 * Per-node type inference + checking
 * ============================================================ */

static char *infer_type(ASTNode *node, TypeEnv *env, CheckCtx *ctx) {
    if (!node) return xstrdup("any");

    switch (node->type) {

        /* ---- Literals ----------------------------------------- */
        case NODE_NUMBER: return xstrdup("number");
        case NODE_STRING: return xstrdup("string");
        case NODE_BOOL:   return xstrdup("bool");
        case NODE_NULL:   return xstrdup("null");

        /* ---- Identifier --------------------------------------- */
        case NODE_IDENTIFIER: {
            const char *t = type_env_get(env, node->str_value);
            if (!t) {
                /* In pedantic mode warn about undeclared / untyped identifiers. */
                if (ctx->mode >= TYPECHECK_PEDANTIC) {
                    emit_diag(ctx, DIAG_WARNING, node->line,
                              "Identifier '%s' has no known type (inferred 'any')",
                              node->str_value);
                }
                return xstrdup("any");
            }
            return xstrdup(t);
        }

        /* ---- Variable declarations ----------------------------- */
        case NODE_VAR_DECL:
        case NODE_LET_DECL: {
            const char *declared_type = node->type_annotation ? node->type_annotation : "any";
            char *inferred = NULL;

            if (node->child_count > 0) {
                inferred = infer_type(node->children[0], env, ctx);

                /* Pedantic: warn about implicit 'any' on declarations. */
                if (ctx->mode >= TYPECHECK_PEDANTIC &&
                        strcmp(declared_type, "any") == 0 &&
                        node->type_annotation == NULL) {
                    emit_diag(ctx, DIAG_WARNING, node->line,
                              "Variable '%s' has no type annotation (implicit 'any')",
                              node->str_value);
                }

                if (!types_compatible(declared_type, inferred)) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/1, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "Variable '%s' declared as '%s' but initialized with '%s'",
                                  node->str_value, declared_type, inferred);
                    }
                }
            } else {
                /* No initializer – use declared type or "any". */
                inferred = xstrdup(declared_type);
            }

            /* Register the *declared* type in the environment so later
               assignments are checked against the declaration, not the
               initializer. */
            type_env_set(env, node->str_value,
                         node->type_annotation ? node->type_annotation : inferred);

            return inferred; /* caller frees */
        }

        /* ---- Assignment --------------------------------------- */
        case NODE_ASSIGN: {
            if (node->child_count < 2) return xstrdup("any");

            /* Left-hand side type (from env). */
            const char *lhs_type = NULL;
            ASTNode    *lhs_node = node->children[0];
            if (lhs_node->type == NODE_IDENTIFIER) {
                lhs_type = type_env_get(env, lhs_node->str_value);
            }

            char *rhs_type = infer_type(node->children[1], env, ctx);

            if (lhs_type && !types_compatible(lhs_type, rhs_type)) {
                DiagSeverity sev;
                if (diag_severity(ctx->mode, /*hard=*/1, &sev)) {
                    emit_diag(ctx, sev, node->line,
                              "Cannot assign '%s' to variable of type '%s'",
                              rhs_type, lhs_type);
                }
            }

            return rhs_type;
        }

        /* ---- Binary expressions ------------------------------- */
        case NODE_BINARY: {
            char *left_type  = infer_type(node->children[0], env, ctx);
            char *right_type = infer_type(node->children[1], env, ctx);
            char *result     = NULL;
            const char *op   = node->str_value;

            /* Arithmetic */
            if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0 ||
                strcmp(op, "*") == 0 || strcmp(op, "/") == 0 ||
                strcmp(op, "%") == 0) {

                /* '+' is also valid for string concatenation */
                if (strcmp(op, "+") == 0 &&
                        types_compatible("string", left_type) &&
                        types_compatible("string", right_type)) {
                    result = xstrdup("string");
                } else {
                    if (!types_compatible("number", left_type) ||
                            !types_compatible("number", right_type)) {
                        DiagSeverity sev;
                        if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                            emit_diag(ctx, sev, node->line,
                                      "Operator '%s' expects numbers, got '%s' and '%s'",
                                      op, left_type, right_type);
                        }
                    }
                    result = xstrdup("number");
                }
            }
            /* Relational / equality / logical */
            else if (strcmp(op, "<")  == 0 || strcmp(op, ">")  == 0 ||
                     strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 ||
                     strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
                     strcmp(op, "and") == 0 || strcmp(op, "or") == 0) {
                /* For ordering ops, operands should be Comparable */
                if ((strcmp(op, "<")  == 0 || strcmp(op, ">")  == 0 ||
                     strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) &&
                    !types_compatible(left_type, right_type)) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "Operator '%s' applied to incompatible types '%s' and '%s'",
                                  op, left_type, right_type);
                    }
                }
                result = xstrdup("bool");
            }
            /* Bitwise */
            else if (strcmp(op, "&") == 0 || strcmp(op, "|") == 0 ||
                     strcmp(op, "^") == 0 || strcmp(op, "<<") == 0 ||
                     strcmp(op, ">>") == 0) {
                if (!types_compatible("number", left_type) ||
                        !types_compatible("number", right_type)) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "Bitwise operator '%s' expects numbers", op);
                    }
                }
                result = xstrdup("number");
            }
            else {
                result = xstrdup("any");
            }

            free(left_type);
            free(right_type);
            return result;
        }

        /* ---- Unary expressions --------------------------------- */
        case NODE_UNARY: {
            char *operand_type = infer_type(node->children[0], env, ctx);
            char *result       = NULL;
            const char *op     = node->str_value;

            if (strcmp(op, "not") == 0 || strcmp(op, "!") == 0) {
                result = xstrdup("bool");
            } else if (strcmp(op, "-") == 0 || strcmp(op, "+") == 0) {
                if (!types_compatible("number", operand_type)) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "Unary '%s' expects number, got '%s'", op, operand_type);
                    }
                }
                result = xstrdup("number");
            } else {
                result = xstrdup(operand_type);
            }

            free(operand_type);
            return result;
        }

        /* ---- Function declarations ----------------------------- */
        case NODE_FN_DECL: {
            TypeEnv *fn_env = type_env_create(env);

            /* Validate type parameter constraints. */
            if (node->type_params) {
                for (size_t i = 0; i < node->type_param_count; i++) {
                    TypeParam *tp = &node->type_params[i];
                    if (tp->constraint) {
                        int known = strcmp(tp->constraint, "Comparable") == 0 ||
                                    strcmp(tp->constraint, "Numeric")    == 0 ||
                                    strcmp(tp->constraint, "Equatable")  == 0 ||
                                    strcmp(tp->constraint, "Hashable")   == 0 ||
                                    strcmp(tp->constraint, "Printable")  == 0 ||
                                    strcmp(tp->constraint, "Iterable")   == 0;
                        if (!known) {
                            DiagSeverity sev;
                            if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                                emit_diag(ctx, sev, node->line,
                                          "Unknown constraint '%s' on type parameter '%s'",
                                          tp->constraint, tp->name);
                            }
                        }
                        /* Register type parameter name in env so it resolves to itself. */
                        type_env_set(fn_env, tp->name, tp->name);
                    }
                }
            }

            /* Register parameters. */
            for (size_t i = 0; i < node->param_count; i++) {
                const char *pt = node->params[i].type_annotation
                                     ? node->params[i].type_annotation
                                     : "any";
                if (ctx->mode >= TYPECHECK_PEDANTIC &&
                        !node->params[i].type_annotation) {
                    emit_diag(ctx, DIAG_WARNING, node->line,
                              "Parameter '%s' of function '%s' has no type annotation",
                              node->params[i].name,
                              node->str_value ? node->str_value : "<anonymous>");
                }
                type_env_set(fn_env, node->params[i].name, pt);
            }

            /* Register the function itself so recursive calls type-check. */
            if (node->str_value) {
                type_env_set(env, node->str_value, "function");
            }

            /* Check body. */
            char *body_type = xstrdup("null");
            if (node->child_count > 0) {
                free(body_type);
                body_type = infer_type(node->children[0], fn_env, ctx);
            }

            /* Validate declared return type. */
            if (node->return_type && strcmp(node->return_type, "void") != 0) {
                if (!types_compatible(node->return_type, body_type)) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/1, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "Function '%s' declared to return '%s' but body returns '%s'",
                                  node->str_value ? node->str_value : "<anonymous>",
                                  node->return_type, body_type);
                    }
                }
            }

            free(body_type);
            type_env_destroy(fn_env);
            return xstrdup("function");
        }

        /* ---- Function calls ------------------------------------ */
        case NODE_FN_CALL: {
            /*
             * Look up the function declaration in the environment.
             * We store function types as "function" – basic check.
             * For full argument checking we need the function's
             * parameter list; that requires storing the ASTNode* itself.
             * Here we perform what we can: check argument count and
             * type-check each argument expression.
             */
            const char *fn_type = type_env_get(env, node->str_value);
            if (fn_type && strcmp(fn_type, "function") != 0 &&
                           strcmp(fn_type, "any")      != 0) {
                DiagSeverity sev;
                if (diag_severity(ctx->mode, /*hard=*/1, &sev)) {
                    emit_diag(ctx, sev, node->line,
                              "'%s' is not a function (type is '%s')",
                              node->str_value, fn_type);
                }
            }

            /* Type-check all argument expressions. */
            for (size_t i = 0; i < node->child_count; i++) {
                char *arg_t = infer_type(node->children[i], env, ctx);
                free(arg_t);
            }

            return xstrdup("any"); /* return type inference requires fn registry */
        }

        /* ---- Return statement ---------------------------------- */
        case NODE_RETURN: {
            if (node->child_count > 0)
                return infer_type(node->children[0], env, ctx);
            return xstrdup("null");
        }

        /* ---- If expression / statement ----------------------- */
        case NODE_IF: {
            /* Check condition. */
            if (node->child_count > 0) {
                char *cond_type = infer_type(node->children[0], env, ctx);
                if (strcmp(cond_type, "bool") != 0 &&
                        strcmp(cond_type, "any")  != 0) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "If condition expects 'bool', got '%s'", cond_type);
                    }
                }
                free(cond_type);
            }

            char *then_type = xstrdup("null");
            char *else_type = xstrdup("null");

            if (node->child_count > 1) {
                free(then_type);
                then_type = infer_type(node->children[1], env, ctx);
            }
            if (node->child_count > 2) {
                free(else_type);
                else_type = infer_type(node->children[2], env, ctx);
            }

            /* Warn when branches diverge (soft warning). */
            if (!types_compatible(then_type, else_type)) {
                DiagSeverity sev;
                if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                    emit_diag(ctx, sev, node->line,
                              "If branches have incompatible types: '%s' and '%s'",
                              then_type, else_type);
                }
            }

            free(else_type);
            return then_type; /* then-branch type is canonical */
        }

        /* ---- While loop --------------------------------------- */
        case NODE_WHILE: {
            if (node->child_count > 0) {
                char *cond_type = infer_type(node->children[0], env, ctx);
                if (strcmp(cond_type, "bool") != 0 &&
                        strcmp(cond_type, "any")  != 0) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "While condition expects 'bool', got '%s'", cond_type);
                    }
                }
                free(cond_type);
            }
            if (node->child_count > 1) {
                char *body_type = infer_type(node->children[1], env, ctx);
                free(body_type);
            }
            return xstrdup("null");
        }

        /* ---- For loop ----------------------------------------- */
        case NODE_FOR: {
            TypeEnv *for_env = type_env_create(env);

            /*
             * Conventional layout: children[0] = init, children[1] = condition,
             * children[2] = update, children[3] = body.
             * For-in layout: children[0] = iterator var, children[1] = iterable,
             * children[2] = body.
             * We handle both gracefully.
             */
            for (size_t i = 0; i < node->child_count; i++) {
                char *child_type = infer_type(node->children[i], for_env, ctx);
                free(child_type);
            }

            type_env_destroy(for_env);
            return xstrdup("null");
        }

        /* ---- Block -------------------------------------------- */
        case NODE_BLOCK: {
            TypeEnv *block_env = type_env_create(env);
            char *last_type = xstrdup("null");
            for (size_t i = 0; i < node->child_count; i++) {
                free(last_type);
                last_type = infer_type(node->children[i], block_env, ctx);
            }
            type_env_destroy(block_env);
            return last_type;
        }

        /* ---- Array literal ------------------------------------- */
        case NODE_ARRAY_LITERAL: {
            if (node->child_count == 0) return xstrdup("[any]");

            char *elem_type = infer_type(node->children[0], env, ctx);

            for (size_t i = 1; i < node->child_count; i++) {
                char *other = infer_type(node->children[i], env, ctx);
                if (!types_compatible(elem_type, other)) {
                    DiagSeverity sev;
                    if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                        emit_diag(ctx, sev, node->line,
                                  "Array has mixed element types: '%s' and '%s'",
                                  elem_type, other);
                    }
                    /* Widen to "any" on mismatch. */
                    free(elem_type);
                    elem_type = xstrdup("any");
                }
                free(other);
            }

            char *array_type = xsprintf("[%s]", elem_type);
            free(elem_type);
            return array_type;
        }

        /* ---- Index expression ---------------------------------- */
        case NODE_INDEX: {
            if (node->child_count > 0) {
                char *obj_type = infer_type(node->children[0], env, ctx);
                /* If the object type is [T], the element type is T. */
                char *elem_type = xstrdup("any");
                if (obj_type[0] == '[') {
                    size_t len = strlen(obj_type);
                    if (obj_type[len - 1] == ']') {
                        elem_type = (char *)malloc(len - 1);
                        memcpy(elem_type, obj_type + 1, len - 2);
                        elem_type[len - 2] = '\0';
                    }
                }
                /* Check index is a number. */
                if (node->child_count > 1) {
                    char *idx_type = infer_type(node->children[1], env, ctx);
                    if (!types_compatible("number", idx_type) &&
                            strcmp(idx_type, "any") != 0) {
                        DiagSeverity sev;
                        if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                            emit_diag(ctx, sev, node->line,
                                      "Array index must be 'number', got '%s'", idx_type);
                        }
                    }
                    free(idx_type);
                }
                free(obj_type);
                return elem_type;
            }
            return xstrdup("any");
        }

        /* ---- Object / struct literal -------------------------- */
        case NODE_OBJECT_LITERAL: {
            /* Type-check all value expressions; return "object". */
            for (size_t i = 0; i < node->child_count; i++) {
                char *vtype = infer_type(node->children[i], env, ctx);
                free(vtype);
            }
            return xstrdup("object");
        }

        /* ---- Class declaration --------------------------------- */
        case NODE_CLASS_DECL: {
            /* Validate class-level type parameter constraints. */
            if (node->type_params) {
                for (size_t i = 0; i < node->type_param_count; i++) {
                    TypeParam *tp = &node->type_params[i];
                    if (tp->constraint) {
                        int known = strcmp(tp->constraint, "Comparable") == 0 ||
                                    strcmp(tp->constraint, "Numeric")    == 0 ||
                                    strcmp(tp->constraint, "Equatable")  == 0 ||
                                    strcmp(tp->constraint, "Hashable")   == 0 ||
                                    strcmp(tp->constraint, "Printable")  == 0 ||
                                    strcmp(tp->constraint, "Iterable")   == 0;
                        if (!known) {
                            DiagSeverity sev;
                            if (diag_severity(ctx->mode, /*hard=*/0, &sev)) {
                                emit_diag(ctx, sev, node->line,
                                          "Unknown constraint '%s' on class type parameter '%s'",
                                          tp->constraint, tp->name);
                            }
                        }
                    }
                }
            }

            TypeEnv *cls_env = type_env_create(env);

            /* Register type params so they resolve inside methods. */
            if (node->type_params) {
                for (size_t i = 0; i < node->type_param_count; i++) {
                    type_env_set(cls_env, node->type_params[i].name,
                                 node->type_params[i].name);
                }
            }

            /* Type-check all methods (skip child[0] which is the parent class). */
            size_t start = (node->child_count > 0 &&
                            node->children[0]->type == NODE_IDENTIFIER) ? 1 : 0;
            for (size_t i = start; i < node->child_count; i++) {
                if (node->children[i]->type == NODE_FN_DECL) {
                    char *mt = infer_type(node->children[i], cls_env, ctx);
                    free(mt);
                }
            }

            type_env_destroy(cls_env);
            return xstrdup("class");
        }

        /* ---- Member access ------------------------------------- */
        case NODE_MEMBER_ACCESS: {
            /* Propagate object type; member type is opaque for now. */
            if (node->child_count > 0) {
                char *obj_type = infer_type(node->children[0], env, ctx);
                free(obj_type);
            }
            return xstrdup("any");
        }

        /* ---- Default ------------------------------------------ */
        default:
            /* For unknown node types, recurse into children to catch
               nested errors, then return "any". */
            for (size_t i = 0; i < node->child_count; i++) {
                char *ct = infer_type(node->children[i], env, ctx);
                free(ct);
            }
            return xstrdup("any");
    }
}

/* ============================================================
 * Public entry points
 * ============================================================ */

TypeCheckResult typecheck_program_ex(ASTNode *program, TypeCheckMode mode) {
    TypeCheckResult result = { 0, 0, NULL };
    if (mode == TYPECHECK_OFF || !program) return result;

    CheckCtx ctx = { mode, &result, NULL };
    TypeEnv *env = type_env_create(NULL);

    if (program->type == NODE_PROGRAM || program->type == NODE_BLOCK) {
        for (size_t i = 0; i < program->child_count; i++) {
            char *t = infer_type(program->children[i], env, &ctx);
            free(t);
        }
    } else {
        char *t = infer_type(program, env, &ctx);
        free(t);
    }

    type_env_destroy(env);
    return result;
}

void typecheck_result_free(TypeCheckResult *result) {
    if (!result) return;
    TypeDiag *d = result->diags;
    while (d) {
        TypeDiag *next = d->next;
        free(d->message);
        free(d);
        d = next;
    }
    result->diags         = NULL;
    result->warning_count = 0;
    result->error_count   = 0;
}

int typecheck_program(ASTNode *program, TypeCheckMode mode) {
    if (mode == TYPECHECK_OFF || !program) return 0;

    int errors = 0;
    CheckCtx ctx = { mode, NULL, &errors };
    TypeEnv *env = type_env_create(NULL);

    if (program->type == NODE_PROGRAM || program->type == NODE_BLOCK) {
        for (size_t i = 0; i < program->child_count; i++) {
            char *t = infer_type(program->children[i], env, &ctx);
            free(t);
        }
    } else {
        char *t = infer_type(program, env, &ctx);
        free(t);
    }

    type_env_destroy(env);
    return errors;
}
