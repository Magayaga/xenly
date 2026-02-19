/*
 * xenlyc_main.c  —  Xenly native compiler driver
 *
 * Pipeline:
 *   source.xe  ->  lexer  ->  parser  ->  AST  ->  codegen  ->  .s
 *   .s  ->  as  ->  .o  ->  gcc (link)  ->  ELF binary
 *
 * Usage:
 *   ./xenlyc  input.xe  [-o output]
 *   ./xenlyc  --help
 *   ./xenlyc  --version
 *   ./xenlyc  --emit-asm  input.xe          # stop after .s (debug)
 */

#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ── file reader ────────────────────────────────────────────────────────── */
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

/* ── path helpers ───────────────────────────────────────────────────────── */
/* replace .xe with .ext, or append .ext if no .xe suffix */
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

/* ── banner ─────────────────────────────────────────────────────────────── */
static void print_usage(const char *prog) {
    printf("\n");
    printf("  \033[1;36m╔══════════════════════════════════════════╗\033[0m\n");
    printf("  \033[1;36m║   ██╗  ██╗ ███╗   ██╗ ███████╗ ██╗      ║\033[0m\n");
    printf("  \033[1;36m║   ██║  ██║ ████╗  ██║ ██╔════╝ ██║      ║\033[0m\n");
    printf("  \033[1;36m║   ███████║ ██╔██╗ ██║ █████╗   ██║  ██╗ ║\033[0m\n");
    printf("  \033[1;36m║   ██╔══██║ ██║╚██╗██║ ██╔══╝   ██║  ██║ ║\033[0m\n");
    printf("  \033[1;36m║   ██║  ██║ ██║ ╚████║ ███████╗ ███████║ ║\033[0m\n");
    printf("  \033[1;36m║   ╚═╝  ╚═╝ ╚═╝  ╚═══╝ ╚══════╝ ╚═════╝  ║\033[0m\n");
    printf("  \033[1;36m║          native compiler                  ║\033[0m\n");
    printf("  \033[1;36m╚══════════════════════════════════════════╝\033[0m\n");
    printf("\n");
    printf("  \033[1;33mUsage:\033[0m   %s [options] <file.xe>\n", prog);
    printf("\n");
    printf("  \033[1;33mOptions:\033[0m\n");
    printf("          -o <file>      Output binary name (default: a.out)\n");
    printf("          --emit-asm     Emit assembly only (writes .s, no link)\n");
    printf("          --version      Show version\n");
    printf("          --help         Show this help\n");
    printf("\n");
    printf("  \033[1;32mExamples:\033[0m\n");
    printf("          %s main.xe                  # -> ./a.out\n", prog);
    printf("          %s main.xe -o main         # -> ./main\n", prog);
    printf("          %s --emit-asm main.xe      # -> main.s\n\n", prog);
}

/* ── main ───────────────────────────────────────────────────────────────── */
int main(int argc, char **argv) {
    const char *input    = NULL;
    const char *output   = NULL;   /* -o value; NULL = "a.out" */
    int         emit_asm = 0;

    /* ── parse CLI ────────────────────────────────────────────────────── */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]); return 0;
        }
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("\n  \033[1;36mxenlyc\033[0m v0.1.0  (native compiler)\n\n");
            return 0;
        }
        if (strcmp(argv[i], "--emit-asm") == 0) { emit_asm = 1; continue; }
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "\033[1;31m[xenlyc] -o requires an argument\033[0m\n");
                return 1;
            }
            output = argv[++i];
            continue;
        }
        if (argv[i][0] != '-') { input = argv[i]; continue; }
        fprintf(stderr, "\033[1;31m[xenlyc] Unknown flag: %s\033[0m\n", argv[i]);
        return 1;
    }

    if (!input) { print_usage(argv[0]); return 0; }

    /* ── read source ──────────────────────────────────────────────────── */
    size_t src_len = 0;
    char  *source  = read_file(input, &src_len);
    if (!source) {
        fprintf(stderr, "\033[1;31m[xenlyc] Cannot open '%s'\033[0m\n", input);
        return 1;
    }

    /* ── lex + parse ──────────────────────────────────────────────────── */
    Lexer   *lexer   = lexer_create(source, src_len);
    Parser  *parser  = parser_create(lexer);
    ASTNode *program = parser_parse(parser);

    if (parser->had_error) {
        fprintf(stderr, "\033[1;31m[xenlyc] Parse errors; aborting.\033[0m\n");
        /* ast_node_destroy(program); — skipped: shared nodes cause double-free */
        /* parser_destroy(parser); */
        /* lexer_destroy(lexer); */
        free(source);
        return 1;
    }

    /* ── codegen -> .s ────────────────────────────────────────────────── */
    char *asm_path = swap_ext(input, ".s");

    if (codegen(program, asm_path) != 0) {
        fprintf(stderr, "\033[1;31m[xenlyc] Code generation failed.\033[0m\n");
        /* ast_node_destroy(program); — skipped: shared nodes cause double-free */
        /* parser_destroy(parser); */
        /* lexer_destroy(lexer); */
        free(source);
        free(asm_path);
        return 1;
    }

    /* ast_node_destroy(program); — skipped: shared nodes cause double-free */
    /* parser_destroy(parser); */
    /* lexer_destroy(lexer); */
    free(source);


    /* ── if --emit-asm, stop here ─────────────────────────────────────── */
    if (emit_asm) {
        printf("\033[1;32m[xenlyc] Assembly written to %s\033[0m\n", asm_path);
        free(asm_path);
        return 0;
    }

    /* ── assemble: as input.s -o input.o ─────────────────────────────── */
    char *obj_path = swap_ext(input, ".o");
    {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd),
                 "as --64 -o %s %s 2>&1", obj_path, asm_path);
        FILE *pipe = popen(cmd, "r");
        char line[256];
        while (fgets(line, sizeof(line), pipe))
            fputs(line, stderr);
        int st = pclose(pipe);
        if (st != 0) {
            fprintf(stderr, "\033[1;31m[xenlyc] Assembly failed (status %d)\033[0m\n", st);
            free(asm_path); free(obj_path);
            return 1;
        }
    }

    /* ── link: gcc input.o -o output -L… libxly_rt.a -lm ────────────── */
    {
        const char *out_name = output ? output : "a.out";

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
        snprintf(cmd, sizeof(cmd),
                 "gcc -o %s %s -L%s -lxly_rt -lm 2>&1",
                 out_name, obj_path, rt_dir);
        FILE *pipe = popen(cmd, "r");
        char line[256];
        while (fgets(line, sizeof(line), pipe))
            fputs(line, stderr);
        int st = pclose(pipe);
        if (st != 0) {
            fprintf(stderr, "\033[1;31m[xenlyc] Link failed (status %d)\033[0m\n", st);
            free(asm_path); free(obj_path);
            return 1;
        }

        printf("\033[1;32m[xenlyc] OK  ->  %s\033[0m\n", out_name);
    }

    /* ── cleanup temp files ───────────────────────────────────────────── */
    unlink(asm_path);
    unlink(obj_path);
    free(asm_path);
    free(obj_path);

    return 0;
}