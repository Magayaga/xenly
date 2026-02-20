#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "ast.h"

typedef enum {
    TYPECHECK_OFF     = 0,   /* disabled – no checks at all */
    TYPECHECK_WARN    = 1,   /* emit warnings, continue     */
    TYPECHECK_ERROR   = 2,   /* emit errors, abort          */
    TYPECHECK_STRICT  = 2,   /* alias for ERROR             */
    TYPECHECK_GRADUAL = 1,   /* alias for WARN              */
} TypeCheckMode;

/* Run type checking on a parsed program.  Returns 0 if OK, >0 on errors. */
int typecheck_program(ASTNode *program, TypeCheckMode mode);

/* Check if a type satisfies a constraint (e.g., "Comparable", "Numeric") */
int satisfies_constraint(const char *type, const char *constraint);

/* Infer generic type substitutions from arguments */
void infer_generic_types(ASTNode *fn_decl, ASTNode **args, size_t argc, 
                        char ***type_map_names, char ***type_map_types, size_t *map_size);

#endif /* TYPECHECK_H */
