/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
/*
 * xenlyc_main.c  —  Xenly native compiler driver  (v0.1.0)
 *
 * Pipeline:
 *   source.xe  →  lexer  →  parser  →  AST  →  codegen  →  .s
 *   .s  →  as  →  .o  →  gcc/clang (link)  →  ELF/Mach-O binary
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
#include "codegen.h"
#include "ast.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

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
 * ══════════════════════════════════════════════════════════════════════════════ */
static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

/* ══════════════════════════════════════════════════════════════════════════════
 * AST IR DUMP  (--emit-ir)
 *
 * Prints a human-readable indented tree of AST nodes — useful for debugging
 * the parser and understanding what the code generator sees.
 * ══════════════════════════════════════════════════════════════════════════════ */
static const char *node_type_name(int type) {
    switch (type) {
        case 0:  return "PROGRAM";      case 1:  return "NUMBER";
        case 2:  return "STRING";       case 3:  return "BOOL";
        case 4:  return "NULL";         case 5:  return "IDENTIFIER";
        case 6:  return "BINARY";       case 7:  return "UNARY";
        case 8:  return "VAR_DECL";     case 9:  return "CONST_DECL";
        case 10: return "ASSIGN";       case 11: return "COMPOUND_ASSIGN";
        case 12: return "IF";           case 13: return "WHILE";
        case 14: return "FOR";          case 15: return "FOR_IN";
        case 16: return "BLOCK";        case 17: return "PRINT";
        case 18: return "FN_DECL";      case 19: return "FN_CALL";
        case 20: return "RETURN";       case 21: return "IMPORT";
        case 22: return "METHOD_CALL";  case 23: return "BREAK";
        case 24: return "CONTINUE";     case 25: return "INCREMENT";
        case 26: return "DECREMENT";    case 27: return "TYPEOF";
        case 28: return "ARRAY_LITERAL";case 29: return "INDEX";
        case 30: return "EXPR_STMT";
        default: return "?";
    }
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
 * HELP / VERSION
 * ══════════════════════════════════════════════════════════════════════════════ */
static void print_usage(const char *prog) {
    printf("\n");
    printf("  \033[1;33mUsage:\033[0m   %s [options] <file.xe>\n", prog);
    printf("\n");
    printf("  %sOptimisation levels:%s\n", COL("1;33"), RESET);
    printf("    0  No opts     — readable asm, good for debugging\n");
    printf("    1  Basic       — noreturn dead-code elision\n");
    printf("    2  Systems     — + sys constant inlining, unboxed fast-paths  (default)\n");
    printf("    3  Aggressive  — + all level-2 opts, maximum inlining\n");
    printf("\n");
    printf("  %sExamples:%s\n", COL("1;32"), RESET);
    printf("    %s main.xe                   %s→ ./a.out\n",   prog, COL("2"));
    printf("    %s%s main.xe -o main          %s→ ./main\n",   RESET, prog, COL("2"));
    printf("    %s%s --opt 0 main.xe          %s→ debug build\n", RESET, prog, COL("2"));
    printf("    %s%s --emit-asm main.xe       %s→ main.s\n",   RESET, prog, COL("2"));
    printf("    %s%s --emit-ir  main.xe       %s→ AST dump\n", RESET, prog, COL("2"));
    printf("    %s%s --verbose main.xe -o m   %s→ annotated asm + stats\n",
           RESET, prog, COL("2"));
    printf("%s\n", RESET);
}

/* ── main ───────────────────────────────────────────────────────────────── */
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

    /* defines table (D<name>=val stored here for future macro use) */
    char *defines[64]; int ndefines = 0;

    /* ── parse CLI ────────────────────────────────────────────────────── */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]); return 0;
        }
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("\n  \033[1;36mxenlyc\033[0m v0.1.0  (native compiler)\n\n");
            return 0;
        }
        if (strcmp(argv[i], "--no-color") == 0) { g_color = 0; continue; }
        if (strcmp(argv[i], "--emit-asm") == 0 || strcmp(argv[i], "-S") == 0) {
            emit_asm = 1; continue;
        }
        if (strcmp(argv[i], "--emit-ir") == 0) { emit_ir = 1; continue; }
        if (strcmp(argv[i], "--keep-asm") == 0) { keep_asm = 1; continue; }
        if (strcmp(argv[i], "--verbose")  == 0) { verbose  = 1; continue; }
        if (strcmp(argv[i], "--static")   == 0) { do_static= 1; continue; }
        if (strcmp(argv[i], "--time")     == 0) { do_time  = 1; continue; }
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
            fprintf(stderr, "[xenlyc] read %.1f ms  parse %.1f ms  codegen %.1f ms\n",
                    t_read-t0, t_parse-t_read, t_codegen-t_parse);
        free(asm_path);
        return 0;
    }

    /* ── assemble: as input.s -o input.o ─────────────────────────────── */
    char *obj_path = swap_ext(input, ".o");
    {
        char cmd[1024];
#if defined(XLY_PLATFORM_MACOS) || defined(PLATFORM_MACOS)
#  if defined(__arm64__) || defined(__aarch64__)
        snprintf(cmd, sizeof(cmd), "as -arch arm64 -o %s %s 2>&1",
                 obj_path, asm_path);
#  else
        snprintf(cmd, sizeof(cmd), "as -arch x86_64 -o %s %s 2>&1",
                 obj_path, asm_path);
#  endif
#else
        snprintf(cmd, sizeof(cmd), "as --64 -o %s %s 2>&1",
                 obj_path, asm_path);
#endif
        if (verbose) fprintf(stderr, "%s[xenlyc]%s assemble: %s\n",
                             COL("2"), RESET, cmd);
        FILE *pipe = popen(cmd, "r");
        char line[256];
        while (fgets(line, sizeof(line), pipe))
            fputs(line, stderr);
        int st = pclose(pipe);
        if (st != 0) {
            fprintf(stderr, "%s[xenlyc]%s Assembly failed (status %d)\n",
                    COL("1;31"), RESET, st);
            free(asm_path); free(obj_path);
            return 1;
        }
    }
    double t_assemble = now_ms();

    /* ── link ─────────────────────────────────────────────────────────── */
    {
        const char *out_name = output ? output : "a.out";

        /* Runtime library lives in same directory as xenlyc binary */
        char rt_dir[512];
        {
            const char *slash = strrchr(argv[0], '/');
            if (slash) {
                size_t dlen = (size_t)(slash - argv[0]);
                if (dlen >= sizeof(rt_dir)) dlen = sizeof(rt_dir) - 1;
                strncpy(rt_dir, argv[0], dlen);
                rt_dir[dlen] = '\0';
            } else {
                strcpy(rt_dir, ".");
            }
        }

        char cmd[2048];
        const char *static_flag = do_static ? "-static" : "";

#if defined(XLY_PLATFORM_MACOS) || defined(PLATFORM_MACOS)
#  if defined(__arm64__) || defined(__aarch64__)
        snprintf(cmd, sizeof(cmd),
                 "clang -arch arm64 %s -o %s %s -L%s -lxly_rt -lm 2>&1",
                 static_flag, out_name, obj_path, rt_dir);
#  else
        snprintf(cmd, sizeof(cmd),
                 "clang -arch x86_64 %s -o %s %s -L%s -lxly_rt -lm 2>&1",
                 static_flag, out_name, obj_path, rt_dir);
#  endif
#else
        snprintf(cmd, sizeof(cmd),
                 "gcc %s -o %s %s -L%s -lxly_rt -lm 2>&1",
                 static_flag, out_name, obj_path, rt_dir);
#endif
        if (verbose) fprintf(stderr, "%s[xenlyc]%s link: %s\n",
                             COL("2"), RESET, cmd);
        FILE *pipe = popen(cmd, "r");
        char line[256];
        while (fgets(line, sizeof(line), pipe))
            fputs(line, stderr);
        int st = pclose(pipe);
        if (st != 0) {
            fprintf(stderr, "%s[xenlyc]%s Link failed (status %d)\n",
                    COL("1;31"), RESET, st);
            free(asm_path); free(obj_path);
            return 1;
        }

        double t_link = now_ms();

        fprintf(stderr, "%s[xenlyc]%s OK  →  %s  (opt=%d)\n",
                COL("1;32"), RESET, out_name, opt_level);

        if (do_time) {
            fprintf(stderr, "\n%s[xenlyc] compile times:%s\n", COL("1;36"), RESET);
            fprintf(stderr, "  read    %6.1f ms\n", t_read     - t0);
            fprintf(stderr, "  parse   %6.1f ms\n", t_parse    - t_read);
            fprintf(stderr, "  codegen %6.1f ms\n", t_codegen  - t_parse);
            fprintf(stderr, "  assem   %6.1f ms\n", t_assemble - t_codegen);
            fprintf(stderr, "  link    %6.1f ms\n", t_link     - t_assemble);
            fprintf(stderr, "  total   %6.1f ms\n", t_link     - t0);
        }
    }

    /* ── cleanup temp files ───────────────────────────────────────────── */
    if (!keep_asm)
        unlink(asm_path);
    else if (verbose)
        fprintf(stderr, "%s[xenlyc]%s kept assembly: %s\n",
                COL("2"), RESET, asm_path);

    unlink(obj_path);
    free(asm_path);
    free(obj_path);

    return 0;
}
