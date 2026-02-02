/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

// ─── Read entire file into a heap-allocated string ───────────────────────────
static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }

    size_t read = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[read] = '\0';
    if (out_len) *out_len = read;
    return buf;
}

// ─── Print usage info ────────────────────────────────────────────────────────
static void print_usage(const char *prog) {
    printf("\n");
    printf("  \033[1;36m╔══════════════════════════════════════╗\033[0m\n");
    printf("  \033[1;36m║   ██╗  ██╗ ███╗   ██╗ ███████╗ ██╗  ║\033[0m\n");
    printf("  \033[1;36m║   ██║  ██║ ████╗  ██║ ██╔════╝ ██║  ║\033[0m\n");
    printf("  \033[1;36m║   ███████║ ██╔██╗ ██║ █████╗   ██║  ║\033[0m\n");
    printf("  \033[1;36m║   ██╔══██║ ██║╚██╗██║ ██╔══╝   ██║  ║\033[0m\n");
    printf("  \033[1;36m║   ██║  ██║ ██║ ╚████║ ███████╗ ██║  ║\033[0m\n");
    printf("  \033[1;36m║   ╚═╝  ╚═╝ ╚═╝  ╚═══╝ ╚══════╝ ╚═╝  ║\033[0m\n");
    printf("  \033[1;36m╚══════════════════════════════════════╝\033[0m\n");
    printf("\n");
    printf("  \033[1;33mUsage:\033[0m  %s <file.xe>\n", prog);
    printf("  \033[1;33mFlags:\033[0m  --version    Show version\n");
    printf("          --help       Show this help\n");
    printf("          --tokens     Dump token stream\n");
    printf("          --ast        Dump AST tree\n\n");
    printf("  \033[1;32mExamples:\033[0m\n");
    printf("          %s main.xe\n", prog);
    printf("          %s --tokens main.xe\n", prog);
    printf("          %s --ast main.xe\n\n", prog);
}

// ─── Dump Token Stream ───────────────────────────────────────────────────────
static void dump_tokens(const char *source, size_t len) {
    Lexer *lexer = lexer_create(source, len);
    printf("\n  \033[1;33m── Token Stream ──────────────────────────\033[0m\n\n");
    int count = 0;
    while (1) {
        Token t = lexer_next_token(lexer);
        if (t.type == TOKEN_NEWLINE) { token_destroy(&t); continue; }
        printf("  \033[0;90m%3d\033[0m  \033[1;36m%-12s\033[0m",
               ++count, token_type_name(t.type));
        if (t.value && t.type != TOKEN_EOF)
            printf(" \033[0;33m\"%s\"\033[0m", t.value);
        printf("  \033[0;90m(line %d)\033[0m\n", t.line);
        if (t.type == TOKEN_EOF) { token_destroy(&t); break; }
        token_destroy(&t);
    }
    printf("\n");
    lexer_destroy(lexer);
}

// ─── Main ────────────────────────────────────────────────────────────────────
int main(int argc, char **argv) {
    const char *filename   = NULL;
    int        dump_tok    = 0;
    int        dump_ast    = 0;

    // ── Parse CLI args ─────────────────────────────────────────────────────
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            printf("\n  \033[1;36mXenly\033[0m v0.1.0  (\033[0;90mbuilt with C\033[0m)\n\n");
            return 0;
        }
        if (strcmp(argv[i], "--tokens") == 0) { dump_tok = 1; continue; }
        if (strcmp(argv[i], "--ast")    == 0) { dump_ast = 1; continue; }
        if (argv[i][0] != '-') { filename = argv[i]; continue; }

        fprintf(stderr, "\033[1;31m[Xenly] Unknown flag: %s\033[0m\n", argv[i]);
        return 1;
    }

    if (!filename) {
        print_usage(argv[0]);
        return 0;
    }

    // ── Read source file ───────────────────────────────────────────────────
    size_t src_len = 0;
    char  *source  = read_file(filename, &src_len);
    if (!source) {
        fprintf(stderr, "\033[1;31m[Xenly Error] Cannot open file '%s'.\033[0m\n", filename);
        return 1;
    }

    // ── Token dump mode ────────────────────────────────────────────────────
    if (dump_tok) {
        dump_tokens(source, src_len);
        free(source);
        return 0;
    }

    // ── Lex ────────────────────────────────────────────────────────────────
    Lexer *lexer = lexer_create(source, src_len);

    // ── Parse ──────────────────────────────────────────────────────────────
    Parser *parser = parser_create(lexer);
    ASTNode *program = parser_parse(parser);

    if (parser->had_error) {
        ast_node_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        return 1;
    }

    // ── AST dump mode ──────────────────────────────────────────────────────
    if (dump_ast) {
        printf("\n  \033[1;33m── AST ───────────────────────────────────\033[0m\n\n");
        ast_print(program, 2);
        printf("\n");
        ast_node_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        return 0;
    }

    // ── Interpret ──────────────────────────────────────────────────────────
    Interpreter *interp = interpreter_create();

    // Set source directory for relative module imports
    {
        const char *slash = strrchr(filename, '/');
        if (slash) {
            size_t dlen = (size_t)(slash - filename);
            char *dir = (char *)malloc(dlen + 1);
            strncpy(dir, filename, dlen);
            dir[dlen] = '\0';
            interpreter_set_source_dir(interp, dir);
            free(dir);
        } else {
            interpreter_set_source_dir(interp, ".");
        }
    }

    Value *result = interpreter_run(interp, program);

    // ── Cleanup ────────────────────────────────────────────────────────────
    int exit_code = interp->had_error ? 1 : 0;
    value_destroy(result);
    interpreter_destroy(interp);
    ast_node_destroy(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);

    return exit_code;
}
