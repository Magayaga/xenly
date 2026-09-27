/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

// ─── Token Types ─────────────────────────────────────────────────────────────
typedef enum {
    // Literals
    TOKEN_NUMBER,       // 42, 3.14
    TOKEN_STRING,       // "hello"
    TOKEN_IDENTIFIER,   // myVar, foo

    // Keywords
    TOKEN_VAR,          // var
    TOKEN_FN,           // fn
    TOKEN_RETURN,       // return
    TOKEN_IF,           // if
    TOKEN_ELSE,         // else
    TOKEN_WHILE,        // while
    TOKEN_FOR,          // for
    TOKEN_DO,           // do
    TOKEN_BREAK,        // break
    TOKEN_CONTINUE,     // continue
    TOKEN_IN,           // in
    TOKEN_SWITCH,       // switch
    TOKEN_CASE,         // case
    TOKEN_DEFAULT,      // default
    TOKEN_IMPORT,       // import
    TOKEN_AS,           // as
    TOKEN_FROM,         // from
    TOKEN_PRINT,        // print
    TOKEN_INPUT,        // input
    TOKEN_TRUE,         // true
    TOKEN_FALSE,        // false
    TOKEN_NULL,         // null
    TOKEN_CLASS,        // class
    TOKEN_NEW,          // new
    TOKEN_THIS,         // this
    TOKEN_SUPER,        // super
    TOKEN_EXTENDS,      // extends
    TOKEN_EXPORT,       // export
    TOKEN_ASYNC,        // async
    TOKEN_SPAWN,        // spawn
    TOKEN_AWAIT,        // await
    TOKEN_SLEEP,        // sleep
    TOKEN_TYPEOF,       // typeof
    TOKEN_INSTANCEOF,   // instanceof
    TOKEN_CONST,        // const (immutable bindings)
    TOKEN_LET,          // let   (block-scoped mutable, alias for var)
    TOKEN_ENUM,         // enum (algebraic data types)
    TOKEN_MATCH,        // match (pattern matching)
    TOKEN_PIPE,         // | (for enum variants and match patterns)
    TOKEN_AMPERSAND,    // & — bitwise AND
    TOKEN_CARET,        // ^ — bitwise XOR
    TOKEN_TILDE,        // ~ — bitwise NOT
    TOKEN_NAMESPACE,    // namespace (for organizing code)
    TOKEN_TYPE,         // type (for type aliases)
    TOKEN_REQUIRES,     // requires (precondition)
    TOKEN_ENSURES,      // ensures (postcondition)
    TOKEN_INVARIANT,    // invariant (class invariant)
    TOKEN_ASSERT,       // assert (runtime assertion)

    // ── Imperative control flow ───────────────────────────────────────────
    TOKEN_UNLESS,       // unless (cond) { body }  — inverse if, no else branch
    TOKEN_REPEAT,       // repeat N { body }        — counted loop
    TOKEN_FOREVER,      // forever { body }         — infinite loop (break to exit)

    // ── Declarative / functional ──────────────────────────────────────────
    TOKEN_PIPE_FORWARD, // |>   — pipe-forward: expr |> fn
    TOKEN_WHERE,        // where — local binding clause: expr where x = val

    // Operators
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_STAR,         // *
    TOKEN_SLASH,        // /
    TOKEN_PERCENT,      // %
    TOKEN_ASSIGN,       // =
    TOKEN_EQ,           // ==
    TOKEN_NEQ,          // !=
    TOKEN_LT,           // <
    TOKEN_GT,           // >
    TOKEN_LTE,          // <=
    TOKEN_GTE,          // >=
    TOKEN_SHL,          // << left shift
    TOKEN_SHR,          // >> right shift
    TOKEN_AND,          // and
    TOKEN_OR,           // or
    TOKEN_NOT,          // not
    TOKEN_PLUSEQ,       // +=
    TOKEN_MINUSEQ,      // -=
    TOKEN_STAREQ,       // *=
    TOKEN_SLASHEQ,      // /=
    TOKEN_PLUSPLUS,     // ++
    TOKEN_MINUSMINUS,   // --
    TOKEN_ARROW,        // =>
    TOKEN_NULLISH,      // ??
    TOKEN_OPTCHAIN,     // ?.

    // Delimiters
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    TOKEN_COMMA,        // ,
    TOKEN_DOT,          // .
    TOKEN_COLON,        // :
    TOKEN_QUESTION,     // ? (for ternary)
    TOKEN_SEMICOLON,    // ; (optional, newline also works)
    TOKEN_NEWLINE,      // \n

    // ── Generators & Iterators ─────────────────────────────────────────
    TOKEN_YIELD,        // yield expr    — suspend generator, emit value
    TOKEN_GEN,          // gen fn name() — generator function keyword
    TOKEN_OF,           // for x of iter — for-of iterator loop

    // ── Reflection / Metaprogramming ──────────────────────────────────
    TOKEN_REFLECT,      // reflect       — built-in reflection namespace
    TOKEN_DOT_BRACKET,  // .[            — computed dynamic property access

    TOKEN_PIPE_PARAM,   // | in { |params| body } — block parameter delimiter

    // Special
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType   type;
    char       *value;      // Raw text of the token
    int         line;
    int         col;
} Token;

// ─── Lexer State ─────────────────────────────────────────────────────────────
typedef struct {
    const char *source;
    size_t      length;
    size_t      pos;
    int         line;
    int         col;
} Lexer;

// ─── API ─────────────────────────────────────────────────────────────────────
Lexer  *lexer_create(const char *source, size_t length);
void    lexer_destroy(Lexer *lexer);
Token   lexer_next_token(Lexer *lexer);
void    token_destroy(Token *token);
const char *token_type_name(TokenType type);

#endif // LEXER_H
