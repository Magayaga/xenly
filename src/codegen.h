/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
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
int  codegen(ASTNode *program, const char *outpath);

/*
 * codegen_set_opts  —  configure codegen before calling codegen()
 *
 *   opt_level   0=debug  1=basic  2=sys-optimized (default)  3=aggressive
 *   verbose_asm 1=emit source-level comments + stats in .s output
 */
void codegen_set_opts(int opt_level, int verbose_asm);

#endif /* CODEGEN_H */
