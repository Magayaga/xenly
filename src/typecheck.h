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

#endif /* TYPECHECK_H */
