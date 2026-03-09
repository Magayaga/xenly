/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 */
/*
 * main.c  —  Xenly interpreter driver  (v0.1.0)
 *
 * Pipeline:
 *   source.xe  →  lexer  →  parser  →  AST  →  interpreter  →  output
 *
 * Command-line options:
 *   -h,  --help               Show help and usage
 *   -v,  --version            Show version string
 *   -dm, --dumpmachine        Print target machine triple
 *   -drd,--dumpreleasedate    Print release date of this build
 *   -dv, --dumpversion        Print detailed version information
 *   -os, --operatingsystem    Print the host operating system
 *        --author             Print author information
 *        --tokens             Dump lexer token stream, then exit
 *        --ast                Dump parsed AST, then exit
 *        --typecheck          Enable type checking (warnings)
 *        --typecheck-strict   Enable strict type checking (errors)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "typecheck.h"

/* ══════════════════════════════════════════════════════════════════════════════
 * VERSION / BUILD METADATA  — change these in one place only
 * ══════════════════════════════════════════════════════════════════════════════ */
#define XLY_VERSION         "0.1.0"
#define XLY_RELEASE_DATE    "202X-XX-XX"
#define XLY_AUTHOR_NAME     "Cyril John Magayaga"
#define XLY_AUTHOR_EMAIL    "cjmagayaga957@gmail.com"

/* ══════════════════════════════════════════════════════════════════════════════
 * COLOUR HELPERS
 * ══════════════════════════════════════════════════════════════════════════════ */
static int g_color = 1;
#define COL(code) (g_color ? "\033[" code "m" : "")
#define RESET     (g_color ? "\033[0m"        : "")

/* ══════════════════════════════════════════════════════════════════════════════
 * FILE READER
 * ══════════════════════════════════════════════════════════════════════════════ */
static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t rd = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * PLATFORM / MACHINE STRINGS
 * ══════════════════════════════════════════════════════════════════════════════ */
static const char *xly_machine(void) {
#if defined(__aarch64__) || defined(__arm64__)
    return "aarch64";
#elif defined(__x86_64__) || defined(_M_X64)
    return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    return "i686";
#elif defined(__arm__)
    return "armv7";
#elif defined(__riscv) && __riscv_xlen == 64
    return "riscv64";
#else
    return "unknown";
#endif
}

static const char *xly_os(void) {
#if defined(__linux__)
    return "Linux";
#elif defined(__APPLE__) && defined(__MACH__)
    return "macOS";
#elif defined(__FreeBSD__)
    return "FreeBSD";
#elif defined(__OpenBSD__)
    return "OpenBSD";
#elif defined(__NetBSD__)
    return "NetBSD";
#elif defined(_WIN32) || defined(_WIN64)
    return "Windows";
#else
    return "Unknown";
#endif
}

static const char *xly_dumpmachine(void) {
#if defined(__aarch64__) || defined(__arm64__)
#  if defined(__APPLE__) && defined(__MACH__)
    return "aarch64-apple-darwin";
#  else
    return "aarch64-linux-gnu";
#  endif
#elif defined(__x86_64__) || defined(_M_X64)
#  if defined(__APPLE__) && defined(__MACH__)
    return "x86_64-apple-darwin";
#  elif defined(__linux__)
    return "x86_64-linux-gnu";
#  else
    return "x86_64-unknown";
#  endif
#elif defined(__i386__) || defined(_M_IX86)
    return "i686-linux-gnu";
#else
    return "unknown-unknown-unknown";
#endif
}

/* ══════════════════════════════════════════════════════════════════════════════
 * HELP / VERSION / AUTHOR
 * ══════════════════════════════════════════════════════════════════════════════ */
static void print_usage(const char *prog) {
    printf("\n");
    printf("  %sXenly%s  Interpreter  %sv" XLY_VERSION "%s\n",
           COL("1;36"), RESET, COL("1;33"), RESET);
    printf("\n");
    printf("  %sUsage:%s   %s [options] <file.xe>\n", COL("1;33"), RESET, prog);
    printf("\n");
    printf("  %sOptions:%s\n", COL("1;33"), RESET);
    printf("    %s-h,  --help%s               Show this help\n",                          COL("1"), RESET);
    printf("    %s-v,  --version%s            Show version\n",                            COL("1"), RESET);
    printf("    %s-dm, --dumpmachine%s        Print the interpreter's target machine\n",  COL("1"), RESET);
    printf("    %s-drd,--dumpreleasedate%s    Print the release date of this build\n",    COL("1"), RESET);
    printf("    %s-dv, --dumpversion%s        Print detailed version information\n",      COL("1"), RESET);
    printf("    %s-os, --operatingsystem%s    Print the host operating system\n",         COL("1"), RESET);
    printf("    %s     --author%s             Print author information\n",                COL("1"), RESET);
    printf("    %s     --tokens%s             Dump token stream, then exit\n",            COL("1"), RESET);
    printf("    %s     --ast%s                Dump AST tree, then exit\n",                COL("1"), RESET);
    printf("    %s     --typecheck%s          Enable type checking %s(warnings)%s\n",
           COL("1"), RESET, COL("2"), RESET);
    printf("    %s     --typecheck-strict%s   Enable strict type checking %s(errors)%s\n",
           COL("1"), RESET, COL("2"), RESET);
    printf("\n");
    printf("  %sExamples:%s\n", COL("1;32"), RESET);
    printf("    %s main.xe\n",               prog);
    printf("    %s --typecheck main.xe\n",    prog);
    printf("    %s --tokens main.xe\n",       prog);
    printf("    %s --ast main.xe\n",          prog);
    printf("%s\n", RESET);
}

static void print_version(void) {
    printf("\n  %sXenly%s v" XLY_VERSION "  (built with C)\n",
           COL("1;36"), RESET);
    printf("  Interpreter — tree-walk evaluator\n");
    printf("  Targets: %s (%s)\n", xly_os(), xly_machine());
    printf("\n");
}

static void print_author(void) {
    printf("\n  %sXenly%s — created, designed, and developed by\n",
           COL("1;36"), RESET);
    printf("  " XLY_AUTHOR_NAME " <%s>\n\n", XLY_AUTHOR_EMAIL);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * TOKEN DUMP  (--tokens)
 * ══════════════════════════════════════════════════════════════════════════════ */
static void dump_tokens(const char *source, size_t len) {
    Lexer *lexer = lexer_create(source, len);
    printf("\n  %s── Token Stream ──────────────────────────%s\n\n",
           COL("1;33"), RESET);
    int count = 0;
    while (1) {
        Token t = lexer_next_token(lexer);
        if (t.type == TOKEN_NEWLINE) { token_destroy(&t); continue; }
        printf("  %s%3d%s  %s%-12s%s",
               COL("0;90"), ++count, RESET,
               COL("1;36"), token_type_name(t.type), RESET);
        if (t.value && t.type != TOKEN_EOF)
            printf(" %s\"%s\"%s", COL("0;33"), t.value, RESET);
        printf("  %s(line %d)%s\n", COL("0;90"), t.line, RESET);
        if (t.type == TOKEN_EOF) { token_destroy(&t); break; }
        token_destroy(&t);
    }
    printf("\n");
    lexer_destroy(lexer);
}

/* ══════════════════════════════════════════════════════════════════════════════
 * MAIN
 * ══════════════════════════════════════════════════════════════════════════════ */
int main(int argc, char **argv) {
    const char    *filename       = NULL;
    int            dump_tok       = 0;
    int            dump_ast       = 0;
    TypeCheckMode  typecheck_mode = TYPECHECK_OFF;

    /* ── parse CLI args ────────────────────────────────────────────────── */
    for (int i = 1; i < argc; i++) {

        /* ── informational flags (print and exit) ── */
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]); return 0;
        }
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            print_version(); return 0;
        }
        if (strcmp(argv[i], "--dumpmachine") == 0 || strcmp(argv[i], "-dm") == 0) {
            printf("%s\n", xly_dumpmachine()); return 0;
        }
        if (strcmp(argv[i], "--dumpreleasedate") == 0 || strcmp(argv[i], "-drd") == 0) {
            printf("%s\n", XLY_RELEASE_DATE); return 0;
        }
        if (strcmp(argv[i], "--dumpversion") == 0 || strcmp(argv[i], "-dv") == 0) {
            printf("%s\n", XLY_VERSION); return 0;
        }
        if (strcmp(argv[i], "--operatingsystem") == 0 || strcmp(argv[i], "-os") == 0) {
            printf("%s (%s)\n", xly_os(), xly_machine()); return 0;
        }
        if (strcmp(argv[i], "--author") == 0) {
            print_author(); return 0;
        }

        /* ── mode flags ── */
        if (strcmp(argv[i], "--no-color") == 0) { g_color = 0; continue; }
        if (strcmp(argv[i], "--tokens")   == 0) { dump_tok = 1; continue; }
        if (strcmp(argv[i], "--ast")      == 0) { dump_ast = 1; continue; }
        if (strcmp(argv[i], "--typecheck") == 0) {
            typecheck_mode = TYPECHECK_WARN; continue;
        }
        if (strcmp(argv[i], "--typecheck-strict") == 0) {
            typecheck_mode = TYPECHECK_ERROR; continue;
        }

        /* ── positional: source file ── */
        if (argv[i][0] != '-') { filename = argv[i]; continue; }

        fprintf(stderr, "%s[Xenly]%s Unknown flag: %s\n",
                COL("1;31"), RESET, argv[i]);
        return 1;
    }

    if (!filename) {
        print_usage(argv[0]);
        return 0;
    }

    /* ── read source file ─────────────────────────────────────────────── */
    size_t  src_len = 0;
    char   *source  = read_file(filename, &src_len);
    if (!source) {
        fprintf(stderr, "%s[Xenly Error]%s Cannot open file '%s'.\n",
                COL("1;31"), RESET, filename);
        return 1;
    }

    /* ── token dump mode ──────────────────────────────────────────────── */
    if (dump_tok) {
        dump_tokens(source, src_len);
        free(source);
        return 0;
    }

    /* ── lex → parse ──────────────────────────────────────────────────── */
    Lexer   *lexer   = lexer_create(source, src_len);
    Parser  *parser  = parser_create(lexer);
    ASTNode *program = parser_parse(parser);

    if (parser->had_error) {
        ast_node_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        return 1;
    }

    /* ── AST dump mode ────────────────────────────────────────────────── */
    if (dump_ast) {
        printf("\n  %s── AST ───────────────────────────────────%s\n\n",
               COL("1;33"), RESET);
        ast_print(program, 2);
        printf("\n");
        ast_node_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        return 0;
    }

    /* ── type check ───────────────────────────────────────────────────── */
    if (typecheck_mode != TYPECHECK_OFF) {
        int type_errors = typecheck_program(program, typecheck_mode);
        if (type_errors > 0 && typecheck_mode == TYPECHECK_ERROR) {
            fprintf(stderr,
                    "\n%s[Xenly]%s Type checking failed with %d error(s).\n\n",
                    COL("1;31"), RESET, type_errors);
            ast_node_destroy(program);
            parser_destroy(parser);
            lexer_destroy(lexer);
            free(source);
            return 1;
        }
        if (type_errors > 0 && typecheck_mode == TYPECHECK_WARN) {
            fprintf(stderr,
                    "\n%s[Xenly]%s Type checking produced %d warning(s), continuing...\n\n",
                    COL("1;33"), RESET, type_errors);
        }
    }

    /* ── interpret ────────────────────────────────────────────────────── */
    Interpreter *interp = interpreter_create();

    /* Set source directory for relative module imports */
    {
        const char *slash = strrchr(filename, '/');
        if (slash) {
            size_t dlen = (size_t)(slash - filename);
            char  *dir  = (char *)malloc(dlen + 1);
            strncpy(dir, filename, dlen);
            dir[dlen] = '\0';
            interpreter_set_source_dir(interp, dir);
            free(dir);
        } else {
            interpreter_set_source_dir(interp, ".");
        }
    }

    Value *result = interpreter_run(interp, program);

    /* ── cleanup ──────────────────────────────────────────────────────── */
    int exit_code = interp->had_error ? 1 : 0;
    value_destroy(result);
    interpreter_destroy(interp);
    ast_node_destroy(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);

    return exit_code;
}
