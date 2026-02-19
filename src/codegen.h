#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

/*
 * codegen  —  AST  →  x86-64 System-V assembly text (.s)
 *
 * compile_program()  walks the AST once and writes a complete, self-contained
 * .s file to *outpath*.  The caller then invokes `as` + `gcc` to produce
 * the final ELF binary.
 *
 * Returns 0 on success, non-zero on error.
 */
int codegen(ASTNode *program, const char *outpath);

#endif /* CODEGEN_H */
