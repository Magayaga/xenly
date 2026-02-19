#include "lexer.h"
#include "unicode.h"
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
    char c = lexer_peek(l);
    char n = lexer_peek_next(l);
    
    // Single-line comment: //
    if (c == '/' && n == '/') {
        lexer_advance(l); // skip first /
        lexer_advance(l); // skip second /
        while (l->pos < l->length && lexer_peek(l) != '\n')
            lexer_advance(l);
        return;
    }
    
    // Multi-line comment: /* ... */
    if (c == '/' && n == '*') {
        lexer_advance(l); // skip /
        lexer_advance(l); // skip *
        while (l->pos < l->length) {
            if (lexer_peek(l) == '*' && lexer_peek_next(l) == '/') {
                lexer_advance(l); // skip *
                lexer_advance(l); // skip /
                return;
            }
            lexer_advance(l);
        }
        // Unterminated comment - just return
        return;
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
    { "const",  TOKEN_CONST },
    { "fn",     TOKEN_FN },
    { "return", TOKEN_RETURN },
    { "if",     TOKEN_IF },
    { "else",   TOKEN_ELSE },
    { "while",  TOKEN_WHILE },
    { "for",    TOKEN_FOR },
    { "do",     TOKEN_DO },
    { "break",  TOKEN_BREAK },
    { "continue", TOKEN_CONTINUE },
    { "switch", TOKEN_SWITCH },
    { "case",   TOKEN_CASE },
    { "default", TOKEN_DEFAULT },
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
    { "enum",    TOKEN_ENUM },
    { "match",   TOKEN_MATCH },
    { "namespace", TOKEN_NAMESPACE },
    { "type",    TOKEN_TYPE },
    { "requires", TOKEN_REQUIRES },
    { "ensures",  TOKEN_ENSURES },
    { "invariant", TOKEN_INVARIANT },
    { "assert",   TOKEN_ASSERT },
    { "in",         TOKEN_IN },
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

    // Skip comments (// or /* */)
    while (1) {
        char c = lexer_peek(l);
        char n = lexer_peek_next(l);
        if ((c == '/' && n == '/') || (c == '/' && n == '*')) {
            skip_comment(l);
            skip_whitespace_no_newline(l);
        } else {
            break;
        }
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

    // ── Raw String Literal (r"..." or r'...') ──────────────────────────────────
    if (c == 'r' && (lexer_peek_next(l) == '"' || lexer_peek_next(l) == '\'')) {
        lexer_advance(l); // consume 'r'
        char quote = lexer_advance(l); // consume opening quote
        char buf[4096];
        int len = 0;
        
        // Raw strings: no escape processing, everything literal
        while (l->pos < l->length && lexer_peek(l) != quote) {
            buf[len++] = lexer_advance(l);
            if (len >= 4095) break;
        }
        buf[len] = '\0';
        if (lexer_peek(l) == quote) lexer_advance(l); // consume closing quote
        return make_token(TOKEN_STRING, buf, startLine, startCol);
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
                    case 'r':  buf[len++] = '\r'; break;
                    case '\\': buf[len++] = '\\'; break;
                    case '"':  buf[len++] = '"';  break;
                    case '0':  buf[len++] = '\0'; break;
                    case 'u': {
                        // Unicode escape: \uXXXX or \u{XXXXXX}
                        if (lexer_peek(l) == '{') {
                            lexer_advance(l); // skip {
                            uint32_t cp = 0;
                            int digits = 0;
                            while (lexer_peek(l) != '}' && digits < 6) {
                                char h = lexer_peek(l);
                                if (h >= '0' && h <= '9') cp = cp * 16 + (h - '0');
                                else if (h >= 'a' && h <= 'f') cp = cp * 16 + (h - 'a' + 10);
                                else if (h >= 'A' && h <= 'F') cp = cp * 16 + (h - 'A' + 10);
                                else break;
                                lexer_advance(l);
                                digits++;
                            }
                            if (lexer_peek(l) == '}') lexer_advance(l);
                            // Encode to UTF-8
                            char utf8[4];
                            size_t bytes = utf8_encode(cp, utf8);
                            for (size_t i = 0; i < bytes && len < 4095; i++) {
                                buf[len++] = utf8[i];
                            }
                        } else {
                            // \uXXXX format (4 hex digits)
                            uint32_t cp = 0;
                            for (int i = 0; i < 4; i++) {
                                char h = lexer_peek(l);
                                if (h >= '0' && h <= '9') cp = cp * 16 + (h - '0');
                                else if (h >= 'a' && h <= 'f') cp = cp * 16 + (h - 'a' + 10);
                                else if (h >= 'A' && h <= 'F') cp = cp * 16 + (h - 'A' + 10);
                                else break;
                                lexer_advance(l);
                            }
                            char utf8[4];
                            size_t bytes = utf8_encode(cp, utf8);
                            for (size_t i = 0; i < bytes && len < 4095; i++) {
                                buf[len++] = utf8[i];
                            }
                        }
                        break;
                    }
                    default:   buf[len++] = esc;  break;
                }
            } else {
                // Copy UTF-8 byte sequences directly
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
    // Support Unicode identifiers
    if (isalpha(c) || c == '_' || ((unsigned char)c >= 0x80)) {
        // Could be Unicode identifier
        size_t bytes;
        uint32_t cp = utf8_decode(&l->source[l->pos], &bytes);
        
        if (unicode_is_id_start(cp)) {
            char buf[256];
            int len = 0;
            
            // Add first character
            for (size_t i = 0; i < bytes && len < 255; i++) {
                buf[len++] = lexer_advance(l);
            }
            
            // Continue with ID_Continue characters
            while (l->pos < l->length) {
                unsigned char next = (unsigned char)lexer_peek(l);
                if (isalnum(next) || next == '_') {
                    buf[len++] = lexer_advance(l);
                } else if (next >= 0x80) {
                    // Check if valid Unicode ID_Continue
                    uint32_t next_cp = utf8_decode(&l->source[l->pos], &bytes);
                    if (unicode_is_id_continue(next_cp)) {
                        for (size_t i = 0; i < bytes && len < 255; i++) {
                            buf[len++] = lexer_advance(l);
                        }
                    } else {
                        break;
                    }
                } else {
                    break;
                }
                if (len >= 255) break;
            }
            
            buf[len] = '\0';
            TokenType kw = lookup_keyword(buf);
            return make_token(kw, buf, startLine, startCol);
        }
    }

    // ── Two-char Operators ───────────────────────────────────────────────────
    lexer_advance(l);
    char n = lexer_peek(l);

    switch (c) {
        case '=':
            if (n == '=') { lexer_advance(l); return make_token(TOKEN_EQ,   "==", startLine, startCol); }
            if (n == '>') { lexer_advance(l); return make_token(TOKEN_ARROW, "=>", startLine, startCol); }
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
        case '?':
            if (n == '?') { lexer_advance(l); return make_token(TOKEN_NULLISH, "??", startLine, startCol); }
            if (n == '.') { lexer_advance(l); return make_token(TOKEN_OPTCHAIN, "?.", startLine, startCol); }
            return make_token(TOKEN_QUESTION, "?", startLine, startCol);
    }

    // ── Single-char Tokens ───────────────────────────────────────────────────
    switch (c) {
        case '(': return make_token(TOKEN_LPAREN,    "(", startLine, startCol);
        case ')': return make_token(TOKEN_RPAREN,    ")", startLine, startCol);
        case '{': return make_token(TOKEN_LBRACE,    "{", startLine, startCol);
        case '}': return make_token(TOKEN_RBRACE,    "}", startLine, startCol);
        case '[': return make_token(TOKEN_LBRACKET,  "[", startLine, startCol);
        case ']': return make_token(TOKEN_RBRACKET,  "]", startLine, startCol);
        case ',': return make_token(TOKEN_COMMA,     ",", startLine, startCol);
        case '.': return make_token(TOKEN_DOT,       ".", startLine, startCol);
        case ':': return make_token(TOKEN_COLON,     ":", startLine, startCol);
        case ';': return make_token(TOKEN_SEMICOLON,";", startLine, startCol);
        case '|': return make_token(TOKEN_PIPE,      "|", startLine, startCol);
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
        case TOKEN_FOR: return "FOR"; case TOKEN_DO: return "DO";
        case TOKEN_BREAK: return "BREAK"; case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_SWITCH: return "SWITCH"; case TOKEN_CASE: return "CASE";
        case TOKEN_DEFAULT: return "DEFAULT"; case TOKEN_QUESTION: return "?";
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
        case TOKEN_CONST: return "CONST"; case TOKEN_ENUM: return "ENUM";
        case TOKEN_MATCH: return "MATCH"; case TOKEN_PIPE: return "|";
        case TOKEN_IN: return "IN";
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
        case TOKEN_LBRACKET: return "["; case TOKEN_RBRACKET: return "]";
        case TOKEN_ARROW: return "=>"; case TOKEN_NULLISH: return "??";
        case TOKEN_OPTCHAIN: return "?.";
        case TOKEN_COMMA: return ","; case TOKEN_DOT: return ".";
        case TOKEN_NEWLINE: return "NL"; case TOKEN_EOF: return "EOF";
        default: return "???";
    }
}
