/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer   *lexer;
    Token    current;       // current token
    Token    previous;      // previous token (for error reporting)
    int      had_error;     // flag: did a parse error occur?
} Parser;

Parser  *parser_create(Lexer *lexer);
void     parser_destroy(Parser *parser);
ASTNode *parser_parse(Parser *parser);   // returns NODE_PROGRAM root

#endif // PARSER_H
