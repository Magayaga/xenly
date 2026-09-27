/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 */
/*
 * sema.h  —  Xenly Semantic Analyzer
 *
 * Sits between the parser and the code generator in the xenlyc pipeline:
 *
 *   source.xe  →  lexer  →  parser  →  AST
 *                                         ↓
 *                                   sema_analyze()   ← HERE
 *                                         ↓
 *                                    codegen()
 *
 * Checks performed (two-pass):
 *
 *   Pass 1  — hoist all top-level fn / async fn / class / enum names so
 *             forward calls and mutual recursion resolve without error.
 *
 *   Pass 2  — full recursive walk:
 *
 *   Errors (abort compilation):
 *     • return outside function
 *     • break outside loop or switch
 *     • continue outside loop
 *     • const variable reassigned
 *     • duplicate parameter name in a function declaration
 *     • duplicate variable/function name in the same scope
 *
 *   Warnings (reported but compilation continues):
 *     • use of undeclared identifier  (warn only; modules can inject names)
 *     • wrong number of arguments to a known function
 *     • unreachable code after return / break / continue
 */

#ifndef SEMA_H
#define SEMA_H

#include "ast.h"

/* ── Result codes ────────────────────────────────────────────────────────── */
typedef enum {
    SEMA_OK    = 0,   /* no errors, no warnings                               */
    SEMA_WARN  = 1,   /* warnings only — compilation can proceed              */
    SEMA_ERROR = 2,   /* at least one hard error — caller should abort        */
} SemaResult;

/* ── Options ─────────────────────────────────────────────────────────────── */
typedef struct {
    int color;        /* 1 = emit ANSI colour codes in diagnostics (default)  */
    int verbose;      /* 1 = print summary line after analysis                */
    int warn_undecl;  /* 1 = warn on undeclared identifiers (default: 1)      */
    int warn_arity;   /* 1 = warn on wrong arg count for known fns (default:1)*/
    int warn_unreach; /* 1 = warn on unreachable code (default: 1)            */
} SemaOpts;

/* Default-initialised options (all warnings enabled, color on). */
SemaOpts sema_default_opts(void);

/* ── Entry point ─────────────────────────────────────────────────────────── */
/*
 * sema_analyze()  — walk the AST and emit diagnostics.
 *
 *   program     root NODE_PROGRAM returned by the parser
 *   opts        analysis options (pass NULL to use defaults)
 *   out_errors  set to the number of hard errors found  (may be NULL)
 *   out_warns   set to the number of warnings issued    (may be NULL)
 *
 * Returns SEMA_OK / SEMA_WARN / SEMA_ERROR.
 */
SemaResult sema_analyze(ASTNode *program,
                        const SemaOpts *opts,
                        int *out_errors,
                        int *out_warns);

#endif /* SEMA_H */
