/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 *
 */
#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "ast.h"
#include <stddef.h>

/* ---------------------------------------------------------------------------
 * Type-check modes
 * --------------------------------------------------------------------------- */
typedef enum {
    TYPECHECK_OFF      = 0,  /* disabled – no checks at all                   */
    TYPECHECK_WARN     = 1,  /* emit warnings, continue                        */
    TYPECHECK_GRADUAL  = 1,  /* alias for WARN (gradual typing)                */
    TYPECHECK_ERROR    = 2,  /* emit errors, non-zero exit on violations        */
    TYPECHECK_STRICT   = 2,  /* alias for ERROR                                */
    TYPECHECK_PEDANTIC = 3,  /* strict + flag all implicit 'any' annotations   */
} TypeCheckMode;

/* ---------------------------------------------------------------------------
 * Diagnostic severity
 * --------------------------------------------------------------------------- */
typedef enum {
    DIAG_NOTE    = 0,
    DIAG_WARNING = 1,
    DIAG_ERROR   = 2,
} DiagSeverity;

/* ---------------------------------------------------------------------------
 * A single type-check diagnostic
 * --------------------------------------------------------------------------- */
typedef struct TypeDiag {
    DiagSeverity  severity;
    int           line;
    char         *message;   /* heap-allocated; freed by typecheck_result_free */
    struct TypeDiag *next;
} TypeDiag;

/* ---------------------------------------------------------------------------
 * Result returned by typecheck_program_ex
 * --------------------------------------------------------------------------- */
typedef struct {
    int       warning_count;
    int       error_count;
    TypeDiag *diags;          /* linked list, oldest first */
} TypeCheckResult;

/* ---------------------------------------------------------------------------
 * Opaque type environment – exposed so the interpreter / REPL can reuse it
 * --------------------------------------------------------------------------- */
typedef struct TypeEnv TypeEnv;

TypeEnv *typeenv_create(TypeEnv *parent);
void     typeenv_destroy(TypeEnv *env);
void     typeenv_set(TypeEnv *env, const char *name, const char *type);
/* Returns borrowed pointer into env; do NOT free. Returns NULL if not found. */
const char *typeenv_get(TypeEnv *env, const char *name);

/* ---------------------------------------------------------------------------
 * Primary API
 * --------------------------------------------------------------------------- */

/*
 * typecheck_program – convenience wrapper.
 * Returns 0 if OK (or mode == OFF), >0 = number of hard errors.
 */
int typecheck_program(ASTNode *program, TypeCheckMode mode);

/*
 * typecheck_program_ex – full result with diagnostics list.
 * Caller must call typecheck_result_free() when done.
 */
TypeCheckResult typecheck_program_ex(ASTNode *program, TypeCheckMode mode);
void            typecheck_result_free(TypeCheckResult *result);

/* ---------------------------------------------------------------------------
 * Type utilities
 * --------------------------------------------------------------------------- */

/* Returns 1 if 'type' satisfies 'constraint' (e.g. "Comparable", "Numeric"). */
int satisfies_constraint(const char *type, const char *constraint);

/*
 * types_assignable – returns 1 when a value of type 'actual' can be assigned
 * to a slot of type 'expected'.  Handles union types ("|"), nullable ("?"),
 * and the "any" wildcard.
 */
int types_assignable(const char *expected, const char *actual);

/*
 * infer_generic_types – populate type-parameter→concrete-type mappings by
 * matching the declared parameter list of fn_decl against the supplied args.
 *
 * Outputs *type_map_names / *type_map_types / *map_size are heap-allocated;
 * caller must free each string and then the arrays themselves.
 */
void infer_generic_types(ASTNode   *fn_decl,
                         ASTNode  **args,
                         size_t     argc,
                         char    ***type_map_names,
                         char    ***type_map_types,
                         size_t    *map_size);

/*
 * resolve_type_param – substitute a concrete type for a type-parameter name.
 * e.g. resolve_type_param("T", names, types, n) → "number".
 * Returns a freshly strdup'd string; caller must free.
 */
char *resolve_type_param(const char  *raw_type,
                         char       **type_map_names,
                         char       **type_map_types,
                         size_t       map_size);

#endif /* TYPECHECK_H */
