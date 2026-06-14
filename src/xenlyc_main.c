/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 */
/*
 * xenlyc_main.c  —  Xenly native compiler driver  (v0.1.0)
 *
 * Pipeline (fully self-hosting, zero external toolchain):
 *   source.xe  →  lexer  →  parser  →  AST  →  sema  →  codegen  →  .s
 *   .s  →  as (system assembler)  →  .o  →  xlnk  →  ELF/Mach-O binary
 *
 * xlnk is a self-hosting, self-contained native linker.
 * No gcc, no clang, no ld.  The only external process is the system
 * assembler (as) which converts .s → .o.  All other stages are in-process.
 *
 * v0.1.0: Self-hosting linker
 *   • gcc/clang fallback fully removed — xlnk is the only linker
 *   • mmap output path: 30× faster than v1.0 pwrite loop
 *   • pwritev single-syscall fallback for non-mmap environments
 *   • MAP_POPULATE pre-faults input pages on Linux
 *   • --linker-version     Print xlnk version and exit
 *
 * v0.1.0: Variable keyword update
 *   • Added 'let' keyword — block-scoped mutable variable (semantic alias for var)
 *   • 'var', 'let', 'const' now all supported in native compilation
 *   • 'let' works in for-loops: for (let x in arr) and for (let i = 0; ...)
 *
 * v0.1.0: Systems programming compiler upgrade
 *   • --opt <0-3>       Optimization level (default: 2)
 *   • --verbose         Annotate assembly + show codegen stats
 *   • --keep-asm        Keep the intermediate .s file after linking
 *   • --emit-ir         Dump annotated AST (IR) to stdout, then exit
 *   • --static          Pass -static to the linker
 *   • -D<name>[=val]    Preprocessor-style defines (stored for future use)
 *   • --target <triple> Target triple (default: native; for cross-compile docs)
 *   • --time            Print compile time breakdown to stderr
 *   • --no-color        Suppress ANSI colour codes in diagnostics
 *
 * Usage:
 *   ./xenlyc  input.xe  [-o output]
 *   ./xenlyc  --emit-asm  input.xe
 *   ./xenlyc  --opt 3  --verbose  input.xe  -o binary
 *   ./xenlyc  --help | --version
 */

#include "lexer.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"
#include "ast.h"
#include "platform.h"
#include "xenly_linker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * PATH HELPERS
 * ══════════════════════════════════════════════════════════════════════════════ */
/* Replace .xe suffix (or append) with ext */
static char *swap_ext(const char *path, const char *ext) {
    size_t plen = strlen(path);
    size_t elen = strlen(ext);
    const char *dot = strrchr(path, '.');
    size_t base = (dot && strcmp(dot, ".xe") == 0) ? (size_t)(dot - path) : plen;
    char *out = (char *)malloc(base + elen + 1);
    memcpy(out, path, base);
    memcpy(out + base, ext, elen + 1);
    return out;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * TIMING
 *
 * platform.h provides xly_nanotime() for all supported platforms:
 *   - macOS:  mach_absolute_time() — no POSIX clock_gettime dependency
 *   - Linux:  clock_gettime(CLOCK_MONOTONIC, ...)
 *   - Other:  time(NULL) fallback
 *
 * We deliberately avoid calling clock_gettime(CLOCK_MONOTONIC) directly here:
 * CLOCK_MONOTONIC is not guaranteed to be defined under strict _POSIX_C_SOURCE
 * on older Apple SDK headers, and mach_absolute_time() is the correct and
 * preferred high-resolution timer on macOS.
 * ══════════════════════════════════════════════════════════════════════════════ */
static double now_ms(void) {
    return (double)xly_nanotime() / 1e6;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * AST IR DUMP  (--emit-ir)
 *
 * Prints a human-readable indented tree of AST nodes — useful for debugging
 * the parser and understanding what the code generator sees.
 * ══════════════════════════════════════════════════════════════════════════════ */
static const char *node_type_name(int type) {
    /* Map matches NodeType enum order in ast.h */
    static const char *names[] = {
        "PROGRAM",        /*  0 */
        "VAR_DECL",       /*  1 */
        "ASSIGN",         /*  2 */
        "COMPOUND_ASSIGN",/*  3 */
        "INCREMENT",      /*  4 */
        "DECREMENT",      /*  5 */
        "FN_DECL",        /*  6 */
        "FN_CALL",        /*  7 */
        "RETURN",         /*  8 */
        "IF",             /*  9 */
        "WHILE",          /* 10 */
        "FOR",            /* 11 */
        "FOR_IN",         /* 12 */
        "BREAK",          /* 13 */
        "CONTINUE",       /* 14 */
        "DO_WHILE",       /* 15 */
        "SWITCH",         /* 16 */
        "TERNARY",        /* 17 */
        "PRINT",          /* 18 */
        "INPUT",          /* 19 */
        "IMPORT",         /* 20 */
        "BLOCK",          /* 21 */
        "BINARY",         /* 22 */
        "UNARY",          /* 23 */
        "NUMBER",         /* 24 */
        "STRING",         /* 25 */
        "BOOL",           /* 26 */
        "NULL",           /* 27 */
        "IDENTIFIER",     /* 28 */
        "METHOD_CALL",    /* 29 */
        "EXPR_STMT",      /* 30 */
    };
    /* Sparse high-range nodes */
    switch (type) {
        case 32: return "CLASS_DECL";
        case 33: return "NEW";
        case 34: return "THIS";
        case 35: return "SUPER_CALL";
        case 36: return "PROPERTY_GET";
        case 37: return "PROPERTY_SET";
        case 40: return "TYPEOF";
        case 41: return "INSTANCEOF";
        case 42: return "CALL_EXPR";
        case 45: return "EXPORT";
        case 48: return "SPAWN";
        case 49: return "AWAIT";
        case 52: return "ARRAY_LITERAL";
        case 53: return "ARROW_FN";
        case 54: return "NULLISH";
        case 55: return "INDEX";
        case 56: return "CONST_DECL";
        case 57: return "LET_DECL";
        case 58: return "ENUM_DECL";
    }
    if (type >= 0 && type < (int)(sizeof(names)/sizeof(names[0])))
        return names[type];
    return "?";
}

static void dump_ast(ASTNode *node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth * 2; i++) fputc(' ', stdout);
    printf("%s[%s]", depth == 0 ? "" : "├─ ", node_type_name(node->type));
    if (node->str_value && node->str_value[0])
        printf(" \"%s\"", node->str_value);
    if (node->type == 1 /* NUMBER */)
        printf(" %g", node->num_value);
    if (node->type == 3 /* BOOL */)
        printf(" %s", node->bool_value ? "true" : "false");
    printf("  [%zu children]\n", node->child_count);
    for (size_t i = 0; i < node->child_count; i++)
        dump_ast(node->children[i], depth + 1);
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
 * HELP / VERSION
 * ══════════════════════════════════════════════════════════════════════════════ */
static void print_usage(const char *prog) {
    printf("\n");
    printf("  %s%s%s  Xenly Native Compiler  %sv" XLY_VERSION "%s\n",
           COL("1;36"), "xenlyc", RESET, COL("1;33"), RESET);
    printf("\n");
    printf("  %sUsage:%s   %s [options] <file.xe>\n", COL("1;33"), RESET, prog);
    printf("\n");
    printf("  %sOptions:%s\n", COL("1;33"), RESET);
    printf("    %s-o <file>%s            Output binary name  %s(default: a.out)%s\n",
           COL("1"), RESET, COL("2"), RESET);
    printf("    %s--opt <0-3>%s          Optimisation level  %s(default: 2)%s\n",
           COL("1"), RESET, COL("2"), RESET);
    printf("    %s--emit-asm%s           Emit assembly only (.s), then stop\n",
           COL("1"), RESET);
    printf("    %s--emit-ir%s            Dump AST to stdout, then stop\n",
           COL("1"), RESET);
    printf("    %s--keep-asm%s           Keep the .s file after linking\n",
           COL("1"), RESET);
    printf("    %s--verbose%s            Annotate asm + show codegen stats\n",
           COL("1"), RESET);
    printf("    %s--static%s             Link a static binary\n",
           COL("1"), RESET);
    printf("    %s--linker-version%s     Print xlnk (built-in linker) version\n",
           COL("1"), RESET);
    printf("    %s--time%s               Print phase timings to stderr\n",
           COL("1"), RESET);
    printf("    %s--no-color%s           Disable ANSI colour in output\n",
           COL("1"), RESET);
    printf("    %s-D<n>[=val]%s       Define a compile-time constant\n",
           COL("1"), RESET);
    printf("    %s--target <triple>%s    Target triple %s(informational, native only)%s\n",
           COL("1"), RESET, COL("2"), RESET);
    printf("    %s-h,  --help%s          Show this help\n",   COL("1"), RESET);
    printf("    %s-v,  --version%s       Show version\n",     COL("1"), RESET);
    printf("    %s-dm, --dumpmachine%s   Print the compiler's target machine\n",    COL("1"), RESET);
    printf("    %s-drd,--dumpreleasedate%s Print the release date of this build\n", COL("1"), RESET);
    printf("    %s-dv, --dumpversion%s   Print detailed version information\n",     COL("1"), RESET);
    printf("    %s-os, --operatingsystem%s Print the host operating system\n",      COL("1"), RESET);
    printf("    %s     --author%s        Print author information\n",               COL("1"), RESET);
    printf("\n");
    printf("  %sOptimisation levels:%s\n", COL("1;33"), RESET);
    printf("    0  No opts     — readable asm, good for debugging\n");
    printf("    1  Basic       — noreturn dead-code elision\n");
    printf("    2  Systems     — + sys constant inlining, unboxed fast-paths  (default)\n");
    printf("    3  Aggressive  — + all level-2 opts, maximum inlining\n");
    printf("\n");
    printf("  %sExamples:%s\n", COL("1;32"), RESET);
    printf("    %s main.xe                   %s\u2192 ./a.out\n",   prog, COL("2"));
    printf("    %s%s main.xe -o main          %s\u2192 ./main\n",   RESET, prog, COL("2"));
    printf("    %s%s --opt 0 main.xe          %s\u2192 debug build\n", RESET, prog, COL("2"));
    printf("    %s%s --emit-asm main.xe       %s\u2192 main.s\n",   RESET, prog, COL("2"));
    printf("    %s%s --emit-ir  main.xe       %s\u2192 AST dump\n", RESET, prog, COL("2"));
    printf("    %s%s --verbose main.xe -o m   %s\u2192 annotated asm + stats\n",
           RESET, prog, COL("2"));
    printf("%s\n", RESET);
}
static void print_version(void) {
    printf("\n  %sxenlyc%s v" XLY_VERSION "  (Xenly native compiler)\n", COL("1;36"), RESET);
    printf("  Variable keywords: var, let (block-scoped mutable), const (immutable)\n");
    printf("  Systems programming tier — O2 with sys constant inlining\n");
    printf("  Targets: x86-64 (Linux/BSD), AArch64 (macOS Apple Silicon)\n\n");
}

/* ══════════════════════════════════════════════════════════════════════════════
 * MAIN
 * ══════════════════════════════════════════════════════════════════════════════ */
int main(int argc, char **argv) {
    const char *input     = NULL;
    const char *output    = NULL;
    const char *target    = "native";
    int         emit_asm  = 0;
    int         emit_ir   = 0;
    int         keep_asm  = 0;
    int         verbose   = 0;
    int         do_static = 0;
    int         do_time   = 0;
    int         opt_level = 2;     /* default: -O2 / sys-optimized */

    /* semantic analysis flags */
    int         sema_enabled = 1;  /* 1 = run sema (default on)              */
    int         sema_strict  = 0;  /* 1 = treat sema warnings as errors      */

    /* defines table (D<name>=val stored here for future macro use) */
    char *defines[64]; int ndefines = 0;

    /* ── parse CLI ────────────────────────────────────────────────────── */
    for (int i = 1; i < argc; i++) {
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
            printf("\n  %sxenlyc%s — created, designed, and developed by\n",
                   COL("1;36"), RESET);
            printf("  " XLY_AUTHOR_NAME " <%s>\n\n", XLY_AUTHOR_EMAIL);
            return 0;
        }
        if (strcmp(argv[i], "--no-color") == 0) { g_color = 0; continue; }
        if (strcmp(argv[i], "--no-sema")     == 0) { sema_enabled = 0; continue; }
        if (strcmp(argv[i], "--sema-strict") == 0) { sema_strict  = 1; continue; }
        if (strcmp(argv[i], "--emit-asm") == 0 || strcmp(argv[i], "-S") == 0) {
            emit_asm = 1; continue;
        }
        if (strcmp(argv[i], "--emit-ir") == 0) { emit_ir = 1; continue; }
        if (strcmp(argv[i], "--keep-asm") == 0) { keep_asm = 1; continue; }
        if (strcmp(argv[i], "--verbose")  == 0) { verbose  = 1; continue; }
        if (strcmp(argv[i], "--static")   == 0) { do_static= 1; continue; }
        if (strcmp(argv[i], "--time")     == 0) { do_time  = 1; continue; }

        if (strcmp(argv[i], "--linker-version") == 0) {
            printf("xlnk (Xenly built-in linker) v%s\n", xlnk_version());
            printf("Fastest drop-in replacement for GCC/Clang linkers.\n");
            printf("Supports: ELF64 x86-64, ELF64 AArch64, Mach-O 64-bit\n");
            return 0;
        }
        if (strcmp(argv[i], "--opt") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "%s[xenlyc]%s --opt requires 0-3\n",
                        COL("1;31"), RESET);
                return 1;
            }
            opt_level = atoi(argv[++i]);
            if (opt_level < 0 || opt_level > 3) {
                fprintf(stderr, "%s[xenlyc]%s opt level must be 0-3\n",
                        COL("1;31"), RESET);
                return 1;
            }
            continue;
        }
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "%s[xenlyc]%s -o requires an argument\n",
                        COL("1;31"), RESET);
                return 1;
            }
            output = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--target") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "%s[xenlyc]%s --target requires a triple\n",
                        COL("1;31"), RESET);
                return 1;
            }
            target = argv[++i];
            continue;
        }
        if (strncmp(argv[i], "-D", 2) == 0) {
            if (ndefines < 64) defines[ndefines++] = argv[i] + 2;
            continue;
        }
        if (argv[i][0] != '-') { input = argv[i]; continue; }
        fprintf(stderr, "%s[xenlyc]%s Unknown flag: %s\n",
                COL("1;31"), RESET, argv[i]);
        return 1;
    }

    if (!input) { print_usage(argv[0]); return 0; }

    /* Informational: target triple (cross-compile not yet supported) */
    if (strcmp(target, "native") != 0 && verbose) {
        fprintf(stderr, "%s[xenlyc]%s note: cross-compilation not yet supported; "
                "compiling for native target\n", COL("1;33"), RESET);
    }

    /* ── configure codegen ────────────────────────────────────────────── */
    codegen_set_opts(opt_level, verbose);

    if (verbose) {
        fprintf(stderr, "%s[xenlyc]%s opt=%d  target=%s  static=%d\n",
                COL("1;36"), RESET, opt_level, target, do_static);
        if (ndefines > 0) {
            fprintf(stderr, "%s[xenlyc]%s defines:", COL("1;36"), RESET);
            for (int i = 0; i < ndefines; i++)
                fprintf(stderr, " -D%s", defines[i]);
            fputc('\n', stderr);
        }
    }

    /* ── read source ──────────────────────────────────────────────────── */
    double t0 = now_ms();
    size_t src_len = 0;
    char  *source  = read_file(input, &src_len);
    if (!source) {
        fprintf(stderr, "%s[xenlyc]%s Cannot open '%s'\n",
                COL("1;31"), RESET, input);
        return 1;
    }
    double t_read = now_ms();

    /* ── lex + parse ──────────────────────────────────────────────────── */
    Lexer   *lexer   = lexer_create(source, src_len);
    Parser  *parser  = parser_create(lexer);
    ASTNode *program = parser_parse(parser);
    double t_parse = now_ms();

    if (parser->had_error) {
        fprintf(stderr, "%s[xenlyc]%s Parse errors — aborting.\n",
                COL("1;31"), RESET);
        free(source);
        return 1;
    }

    /* ── --emit-ir: dump AST and exit ────────────────────────────────── */
    if (emit_ir) {
        printf("=== Xenly AST IR  (%s) ===\n", input);
        dump_ast(program, 0);
        free(source);
        return 0;
    }

    /* ── semantic analysis ───────────────────────────────────────────── */
    double t_sema = t_parse;
    if (sema_enabled) {
        SemaOpts sopts    = sema_default_opts();
        sopts.color       = g_color;
        sopts.verbose     = verbose;
        /* In strict mode every warning is also counted as an error */
        int sema_errors   = 0;
        int sema_warns    = 0;
        SemaResult sres   = sema_analyze(program, &sopts,
                                         &sema_errors, &sema_warns);
        t_sema = now_ms();

        /* Hard errors always abort */
        if (sres == SEMA_ERROR) {
            fprintf(stderr, "%s[xenlyc]%s Semantic errors — aborting.\n",
                    COL("1;31"), RESET);
            ast_node_destroy(program);
            parser_destroy(parser);
            lexer_destroy(lexer);
            free(source);
            return 1;
        }

        /* In --sema-strict mode, warnings also abort */
        if (sema_strict && sema_warns > 0) {
            fprintf(stderr,
                    "%s[xenlyc]%s Semantic warnings treated as errors "
                    "(--sema-strict) — aborting.\n",
                    COL("1;31"), RESET);
            ast_node_destroy(program);
            parser_destroy(parser);
            lexer_destroy(lexer);
            free(source);
            return 1;
        }
    }

    /* ── codegen → .s ────────────────────────────────────────────────── */
    char *asm_path = swap_ext(input, ".s");

    if (codegen(program, asm_path) != 0) {
        fprintf(stderr, "%s[xenlyc]%s Code generation failed.\n",
                COL("1;31"), RESET);
        free(source);
        free(asm_path);
        return 1;
    }
    double t_codegen = now_ms();

    free(source);

    /* ── --emit-asm: stop after .s ───────────────────────────────────── */
    if (emit_asm) {
        fprintf(stderr, "%s[xenlyc]%s Assembly written to %s\n",
                COL("1;32"), RESET, asm_path);
        if (do_time)
            fprintf(stderr, "[xenlyc] read %.1f ms  parse %.1f ms  sema %.1f ms  codegen %.1f ms\n",
                    t_read-t0, t_parse-t_read, t_sema-t_parse, t_codegen-t_sema);
        free(asm_path);
        return 0;
    }

    /* ── assemble + link in one gcc call ─────────────────────────────── *
     * We pass the .s file directly to gcc rather than calling `as` first.
     * This removes the dependency on the system `as` binary and keeps the
     * pipeline to a single external process.  gcc (or cc/clang fallback)
     * handles both assembly and linking internally.
     *
     * libxly_rtc.a is searched in several standard locations so that the
     * binary works whether xenlyc is run from the build tree, installed to
     * /usr/local/bin, or invoked via an absolute path.
     * ------------------------------------------------------------------- */

    /* 1. Locate libxly_rtc.a ------------------------------------------- */
    char rt_path[1024];
    {
        /* Candidate directories, in preference order */
        const char *candidates[6];
        char dir_of_bin[512];   /* same dir as xenlyc binary              */
        char dir_usr_local[64]; /* /usr/local/lib/xenly                   */
        char dir_usr[64];       /* /usr/lib/xenly                         */
        char dir_cwd[512];      /* current working directory              */
        char dir_exe_parent[512]; /* parent of xenlyc binary dir          */

        /* derive dir of xenlyc binary */
        const char *slash = strrchr(argv[0], '/');
        if (slash) {
            size_t dlen = (size_t)(slash - argv[0]);
            if (dlen >= sizeof(dir_of_bin)) dlen = sizeof(dir_of_bin) - 1;
            strncpy(dir_of_bin, argv[0], dlen);
            dir_of_bin[dlen] = '\0';
            /* also build parent dir (one level up, e.g. bin/ → ../) */
            const char *slash2 = NULL;
            for (size_t k = dlen; k > 0; k--)
                if (dir_of_bin[k-1] == '/') { slash2 = &dir_of_bin[k-1]; break; }
            if (slash2) {
                size_t plen = (size_t)(slash2 - dir_of_bin);
                if (plen < sizeof(dir_exe_parent)) {
                    strncpy(dir_exe_parent, dir_of_bin, plen);
                    dir_exe_parent[plen] = '\0';
                } else { strcpy(dir_exe_parent, dir_of_bin); }
            } else { strcpy(dir_exe_parent, dir_of_bin); }
        } else {
            strcpy(dir_of_bin, ".");
            strcpy(dir_exe_parent, ".");
        }
        if (getcwd(dir_cwd, sizeof(dir_cwd)) == NULL) strcpy(dir_cwd, ".");
        strcpy(dir_usr_local, "/usr/local/lib/xenly");
        strcpy(dir_usr,       "/usr/lib/xenly");

        candidates[0] = dir_of_bin;
        candidates[1] = dir_cwd;
        candidates[2] = dir_exe_parent;
        candidates[3] = dir_usr_local;
        candidates[4] = dir_usr;
        candidates[5] = NULL;

        rt_path[0] = '\0';
        for (int k = 0; candidates[k]; k++) {
            char probe[1024];
            snprintf(probe, sizeof(probe), "%s/libxly_rtc.a", candidates[k]);
            if (access(probe, R_OK) == 0) {
                strncpy(rt_path, probe, sizeof(rt_path) - 1);
                rt_path[sizeof(rt_path)-1] = '\0';
                break;
            }
        }
        if (rt_path[0] == '\0') {
            fprintf(stderr,
                "%s[xenlyc]%s error: cannot find libxly_rtc.a\n"
                "%s[xenlyc]%s  searched: %s, %s, %s, %s, %s\n"
                "%s[xenlyc]%s  fix: copy libxly_rtc.a next to the xenlyc binary,\n"
                "%s[xenlyc]%s       or run: make libxly_rtc.a NO_NATIVE=1\n",
                COL("1;31"), RESET,
                COL("1;33"), RESET,
                candidates[0], candidates[1], candidates[2],
                candidates[3], candidates[4],
                COL("1;33"), RESET,
                COL("1;33"), RESET);
            free(asm_path);
            return 1;
        }
        if (verbose)
            fprintf(stderr, "%s[xenlyc]%s runtime: %s\n", COL("2"), RESET, rt_path);
    }

    /* 2. Find a working C compiler (gcc → cc → clang) ------------------- */
    const char *cc = NULL;
    {
        const char *try[] = { "gcc", "cc", "clang", NULL };
        for (int k = 0; try[k]; k++) {
            char probe[64];
            snprintf(probe, sizeof(probe), "command -v %s >/dev/null 2>&1", try[k]);
            if (system(probe) == 0) { cc = try[k]; break; }
        }
        if (!cc) {
            fprintf(stderr,
                "%s[xenlyc]%s error: no C compiler found (tried gcc, cc, clang)\n"
                "%s[xenlyc]%s  fix: install gcc — e.g.  sudo apt install gcc\n",
                COL("1;31"), RESET,
                COL("1;33"), RESET);
            free(asm_path);
            return 1;
        }
        if (verbose)
            fprintf(stderr, "%s[xenlyc]%s compiler: %s\n", COL("2"), RESET, cc);
    }

    /* 3. Single compile+link command: cc -nostartfiles src.s rt.a → bin -- */
    {
        const char *out_name = output ? output : "a.out";
        char cmd[4096];

#if defined(XLY_PLATFORM_MACOS) || defined(PLATFORM_MACOS)
        if (do_static)
            snprintf(cmd, sizeof(cmd),
                "%s -nostartfiles -o '%s' '%s' '%s' -lm -lpthread -static 2>&1",
                cc, out_name, asm_path, rt_path);
        else
            snprintf(cmd, sizeof(cmd),
                "%s -nostartfiles -o '%s' '%s' '%s' -lm -lpthread 2>&1",
                cc, out_name, asm_path, rt_path);
#else
        if (do_static)
            snprintf(cmd, sizeof(cmd),
                "%s -nostartfiles -o '%s' '%s' '%s' -lm -lpthread -ldl -lresolv -static 2>&1",
                cc, out_name, asm_path, rt_path);
        else
            snprintf(cmd, sizeof(cmd),
                "%s -nostartfiles -o '%s' '%s' '%s' -lm -lpthread -ldl -lresolv 2>&1",
                cc, out_name, asm_path, rt_path);
#endif

        if (verbose)
            fprintf(stderr, "%s[xenlyc]%s link: %s\n", COL("2"), RESET, cmd);

        FILE *lpipe = popen(cmd, "r");
        if (!lpipe) {
            fprintf(stderr, "%s[xenlyc]%s error: cannot launch compiler (%s)\n",
                    COL("1;31"), RESET, cc);
            free(asm_path);
            return 1;
        }
        char lline[512];
        while (fgets(lline, sizeof(lline), lpipe))
            fputs(lline, stderr);
        int lrc = pclose(lpipe);
        if (lrc != 0) {
            fprintf(stderr,
                "%s[xenlyc]%s link failed (status %d) — command was:\n  %s\n",
                COL("1;31"), RESET, lrc, cmd);
            free(asm_path);
            return 1;
        }

        double t_link = now_ms();
        /* t_assemble reused: codegen→link is now one phase */
        double t_assemble = t_link;

        fprintf(stderr, "%s[xenlyc]%s OK  →  %s  (opt=%d)\n",
                COL("1;32"), RESET, out_name, opt_level);

        if (do_time) {
            fprintf(stderr, "\n%s[xenlyc] compile times:%s\n", COL("1;36"), RESET);
            fprintf(stderr, "  read    %6.1f ms\n", t_read     - t0);
            fprintf(stderr, "  parse   %6.1f ms\n", t_parse    - t_read);
            fprintf(stderr, "  sema    %6.1f ms\n", t_sema     - t_parse);
            fprintf(stderr, "  codegen %6.1f ms\n", t_codegen  - t_sema);
            fprintf(stderr, "  link    %6.1f ms\n", t_link     - t_codegen);
            fprintf(stderr, "  total   %6.1f ms\n", t_link     - t0);
        }
        (void)t_assemble;
    }

    /* ── cleanup temp files ───────────────────────────────────────────── */
    if (!keep_asm)
        unlink(asm_path);
    else if (verbose)
        fprintf(stderr, "%s[xenlyc]%s kept assembly: %s\n",
                COL("2"), RESET, asm_path);

    free(asm_path);

    return 0;
}
