/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// ─── Helpers ─────────────────────────────────────────────────────────────────
static char lexer_peek(Lexer *l) {
    if (l->pos >= l->length) return '\0';
    return l->source[l->pos];
}

static char lexer_peek_next(Lexer *l) {
    if (l->pos + 1 >= l->length) return '\0';
    return l->source[l->pos + 1];
}

static char lexer_advance(Lexer *l) {
    char ch = l->source[l->pos++];
    if (ch == '\n') { l->line++; l->col = 1; }
    else            { l->col++; }
    return ch;
}

static void skip_whitespace_no_newline(Lexer *l) {
    while (l->pos < l->length) {
        char c = lexer_peek(l);
        if (c == ' ' || c == '\t' || c == '\r') lexer_advance(l);
        else break;
    }
}

static void skip_comment(Lexer *l) {
    // Single-line comment: #
    if (lexer_peek(l) == '#') {
        while (l->pos < l->length && lexer_peek(l) != '\n')
            lexer_advance(l);
    }
}

static Token make_token(TokenType type, const char *val, int line, int col) {
    Token t;
    t.type  = type;
    t.line  = line;
    t.col   = col;
    t.value = val ? strdup(val) : NULL;
    return t;
}

// ─── Keyword Table ───────────────────────────────────────────────────────────
typedef struct { const char *word; TokenType type; } Keyword;

static const Keyword keywords[] = {
    { "var",    TOKEN_VAR },
    { "fn",     TOKEN_FN },
    { "return", TOKEN_RETURN },
    { "if",     TOKEN_IF },
    { "else",   TOKEN_ELSE },
    { "while",  TOKEN_WHILE },
    { "import", TOKEN_IMPORT },
    { "as",     TOKEN_AS },
    { "from",   TOKEN_FROM },
    { "print",  TOKEN_PRINT },
    { "input",  TOKEN_INPUT },
    { "true",    TOKEN_TRUE },
    { "false",   TOKEN_FALSE },
    { "null",    TOKEN_NULL },
    { "class",   TOKEN_CLASS },
    { "new",     TOKEN_NEW },
    { "this",    TOKEN_THIS },
    { "super",   TOKEN_SUPER },
    { "extends",    TOKEN_EXTENDS },
    { "export",     TOKEN_EXPORT },
    { "async",      TOKEN_ASYNC },
    { "spawn",      TOKEN_SPAWN },
    { "await",      TOKEN_AWAIT },
    { "sleep",      TOKEN_SLEEP },
    { "typeof",  TOKEN_TYPEOF },
    { "instanceof", TOKEN_INSTANCEOF },
    { "and",     TOKEN_AND },
    { "or",     TOKEN_OR },
    { "not",    TOKEN_NOT },
    { NULL, 0 }
};

static TokenType lookup_keyword(const char *ident) {
    for (int i = 0; keywords[i].word; i++) {
        if (strcmp(ident, keywords[i].word) == 0)
            return keywords[i].type;
    }
    return TOKEN_IDENTIFIER;
}

// ─── Lexer Create / Destroy ──────────────────────────────────────────────────
Lexer *lexer_create(const char *source, size_t length) {
    Lexer *l = (Lexer *)malloc(sizeof(Lexer));
    l->source = source;
    l->length = length;
    l->pos    = 0;
    l->line   = 1;
    l->col    = 1;
    return l;
}

void lexer_destroy(Lexer *lexer) {
    free(lexer);
}

void token_destroy(Token *t) {
    if (t && t->value) { free(t->value); t->value = NULL; }
}

// ─── Main Tokenizer ──────────────────────────────────────────────────────────
Token lexer_next_token(Lexer *l) {
    skip_whitespace_no_newline(l);

    // Skip comments
    while (lexer_peek(l) == '#') {
        skip_comment(l);
        skip_whitespace_no_newline(l);
    }

    if (l->pos >= l->length)
        return make_token(TOKEN_EOF, "EOF", l->line, l->col);

    int  startLine = l->line;
    int  startCol  = l->col;
    char c         = lexer_peek(l);

    // ── Newline ──
    if (c == '\n') {
        lexer_advance(l);
        return make_token(TOKEN_NEWLINE, "\\n", startLine, startCol);
    }

    // ── String Literal ───────────────────────────────────────────────────────
    if (c == '"') {
        lexer_advance(l); // consume opening "
        char buf[4096];
        int  len = 0;
        while (l->pos < l->length && lexer_peek(l) != '"') {
            if (lexer_peek(l) == '\\') {
                lexer_advance(l);
                char esc = lexer_advance(l);
                switch (esc) {
                    case 'n':  buf[len++] = '\n'; break;
                    case 't':  buf[len++] = '\t'; break;
                    case '\\': buf[len++] = '\\'; break;
                    case '"':  buf[len++] = '"';  break;
                    case '0':  buf[len++] = '\0'; break;
                    default:   buf[len++] = esc;  break;
                }
            } else {
                buf[len++] = lexer_advance(l);
            }
            if (len >= 4095) break;
        }
        buf[len] = '\0';
        if (lexer_peek(l) == '"') lexer_advance(l); // consume closing "
        return make_token(TOKEN_STRING, buf, startLine, startCol);
    }

    // ── Number ───────────────────────────────────────────────────────────────
    if (isdigit(c) || (c == '.' && isdigit(lexer_peek_next(l)))) {
        char buf[64];
        int len = 0;
        while (l->pos < l->length && (isdigit(lexer_peek(l)) || lexer_peek(l) == '.'))
            buf[len++] = lexer_advance(l);
        buf[len] = '\0';
        return make_token(TOKEN_NUMBER, buf, startLine, startCol);
    }

    // ── Identifier / Keyword ─────────────────────────────────────────────────
    if (isalpha(c) || c == '_') {
        char buf[256];
        int len = 0;
        while (l->pos < l->length && (isalnum(lexer_peek(l)) || lexer_peek(l) == '_'))
            buf[len++] = lexer_advance(l);
        buf[len] = '\0';
        TokenType kw = lookup_keyword(buf);
        return make_token(kw, buf, startLine, startCol);
    }

    // ── Two-char Operators ───────────────────────────────────────────────────
    lexer_advance(l);
    char n = lexer_peek(l);

    switch (c) {
        case '=':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_EQ,   "==", startLine, startCol); }
            return make_token(TOKEN_ASSIGN, "=", startLine, startCol);
        case '!':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_NEQ,  "!=", startLine, startCol); }
            return make_token(TOKEN_ERROR, "!", startLine, startCol);
        case '<':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_LTE,  "<=", startLine, startCol); }
            return make_token(TOKEN_LT,  "<", startLine, startCol);
        case '>':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_GTE,  ">=", startLine, startCol); }
            return make_token(TOKEN_GT,  ">", startLine, startCol);
        case '+':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_PLUSEQ,    "+=", startLine, startCol); }
            if (n == '+') { lexer_advance(l); return make_token(TOKEN_PLUSPLUS,  "++", startLine, startCol); }
            return make_token(TOKEN_PLUS,  "+", startLine, startCol);
        case '-':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_MINUSEQ,      "-=", startLine, startCol); }
            if (n == '-') { lexer_advance(l); return make_token(TOKEN_MINUSMINUS,  "--", startLine, startCol); }
            return make_token(TOKEN_MINUS, "-", startLine, startCol);
        case '*':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_STAREQ,  "*=", startLine, startCol); }
            return make_token(TOKEN_STAR,  "*", startLine, startCol);
        case '/':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_SLASHEQ, "/=", startLine, startCol); }
            return make_token(TOKEN_SLASH, "/", startLine, startCol);
        case '%':
            return make_token(TOKEN_PERCENT, "%", startLine, startCol);
    }

    // ── Single-char Tokens ───────────────────────────────────────────────────
    switch (c) {
        case '(': return make_token(TOKEN_LPAREN,    "(", startLine, startCol);
        case ')': return make_token(TOKEN_RPAREN,    ")", startLine, startCol);
        case '{': return make_token(TOKEN_LBRACE,    "{", startLine, startCol);
        case '}': return make_token(TOKEN_RBRACE,    "}", startLine, startCol);
        case ',': return make_token(TOKEN_COMMA,     ",", startLine, startCol);
        case '.': return make_token(TOKEN_DOT,       ".", startLine, startCol);
        case ':': return make_token(TOKEN_COLON,     ":", startLine, startCol);
        case ';': return make_token(TOKEN_SEMICOLON,";", startLine, startCol);
    }

    // ── Unknown ──────────────────────────────────────────────────────────────
    char errbuf[32];
    snprintf(errbuf, sizeof(errbuf), "Unexpected character '%c'", c);
    return make_token(TOKEN_ERROR, errbuf, startLine, startCol);
}

// ─── Debug ───────────────────────────────────────────────────────────────────
const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_IDENTIFIER: return "IDENT";
        case TOKEN_VAR: return "VAR"; case TOKEN_FN: return "FN";
        case TOKEN_RETURN: return "RETURN"; case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE"; case TOKEN_WHILE: return "WHILE";
        case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_AS:     return "AS";
        case TOKEN_FROM:   return "FROM";
        case TOKEN_PRINT:  return "PRINT";
        case TOKEN_INPUT: return "INPUT";
        case TOKEN_TRUE: return "TRUE"; case TOKEN_FALSE: return "FALSE";
        case TOKEN_NULL: return "NULL";
        case TOKEN_CLASS: return "CLASS"; case TOKEN_NEW: return "NEW";
        case TOKEN_THIS: return "THIS"; case TOKEN_SUPER: return "SUPER";
        case TOKEN_EXTENDS: return "EXTENDS";
        case TOKEN_EXPORT:  return "EXPORT";
        case TOKEN_ASYNC:   return "ASYNC";
        case TOKEN_SPAWN:   return "SPAWN";
        case TOKEN_AWAIT:   return "AWAIT";
        case TOKEN_SLEEP:   return "SLEEP";
        case TOKEN_TYPEOF: return "TYPEOF"; case TOKEN_INSTANCEOF: return "INSTANCEOF";
        case TOKEN_PLUS: return "+"; case TOKEN_MINUS: return "-";
        case TOKEN_STAR: return "*"; case TOKEN_SLASH: return "/";
        case TOKEN_PERCENT: return "%"; case TOKEN_ASSIGN: return "=";
        case TOKEN_EQ: return "=="; case TOKEN_NEQ: return "!=";
        case TOKEN_LT: return "<"; case TOKEN_GT: return ">";
        case TOKEN_LTE: return "<="; case TOKEN_GTE: return ">=";
        case TOKEN_AND: return "AND"; case TOKEN_OR: return "OR";
        case TOKEN_NOT: return "NOT";
        case TOKEN_LPAREN: return "("; case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACE: return "{"; case TOKEN_RBRACE: return "}";
        case TOKEN_COMMA: return ","; case TOKEN_DOT: return ".";
        case TOKEN_NEWLINE: return "NL"; case TOKEN_EOF: return "EOF";
        default: return "???";
    }
}
