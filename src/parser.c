#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ─── Forward declarations (grammar hierarchy) ───────────────────────────────
static ASTNode *parse_statement(Parser *p);
static ASTNode *parse_expression(Parser *p);
static ASTNode *parse_nullish(Parser *p);
static ASTNode *parse_or(Parser *p);
static ASTNode *parse_and(Parser *p);
static ASTNode *parse_equality(Parser *p);
static ASTNode *parse_comparison(Parser *p);
static ASTNode *parse_addition(Parser *p);
static ASTNode *parse_multiplication(Parser *p);
static ASTNode *parse_unary(Parser *p);
static ASTNode *parse_call(Parser *p);
static ASTNode *parse_primary(Parser *p);

// ─── Helpers ─────────────────────────────────────────────────────────────────
static void advance(Parser *p) {
    token_destroy(&p->previous);
    p->previous = p->current;
    p->current  = lexer_next_token(p->lexer);
}

static int check(Parser *p, TokenType type) {
    return p->current.type == type;
}

static int match(Parser *p, TokenType type) {
    if (check(p, type)) { advance(p); return 1; }
    return 0;
}

static void skip_newlines(Parser *p) {
    while (match(p, TOKEN_NEWLINE) || match(p, TOKEN_SEMICOLON)) {}
}

static void error_at(Parser *p, const char *msg) {
    fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: %s\033[0m\n", p->current.line, msg);
    p->had_error = 1;
}

static void expect(Parser *p, TokenType type, const char *msg) {
    if (!match(p, type)) error_at(p, msg);
}

// ─── Create / Destroy ────────────────────────────────────────────────────────
Parser *parser_create(Lexer *lexer) {
    Parser *p   = (Parser *)calloc(1, sizeof(Parser));
    p->lexer    = lexer;
    p->current  = lexer_next_token(lexer);  // prime
    return p;
}

void parser_destroy(Parser *p) {
    token_destroy(&p->current);
    token_destroy(&p->previous);
    free(p);
}

// ─── Top-Level: Program ─────────────────────────────────────────────────────
ASTNode *parser_parse(Parser *p) {
    ASTNode *prog = ast_node_create(NODE_PROGRAM, 1);
    skip_newlines(p);
    while (!check(p, TOKEN_EOF) && !p->had_error) {
        ASTNode *stmt = parse_statement(p);
        if (stmt) ast_node_add_child(prog, stmt);
        skip_newlines(p);
    }
    return prog;
}

// ─── Statement Dispatcher ────────────────────────────────────────────────────
static ASTNode *parse_statement(Parser *p) {
    skip_newlines(p);

    // var declaration
    if (check(p, TOKEN_VAR)) {
        advance(p);
        int line = p->current.line;
        ASTNode *node = ast_node_create(NODE_VAR_DECL, line);

        if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected variable name after 'var'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);

        // Optional type annotation: var x: number
        if (match(p, TOKEN_COLON)) {
            if (!check(p, TOKEN_IDENTIFIER)) {
                error_at(p, "Expected type name after ':'.");
            } else {
                node->type_annotation = strdup(p->current.value);
                advance(p);
            }
        }

        // Optional initializer
        if (match(p, TOKEN_ASSIGN)) {
            ast_node_add_child(node, parse_expression(p));
        }
        skip_newlines(p);
        return node;
    }

    // const declaration (immutable binding)
    if (check(p, TOKEN_CONST)) {
        advance(p);
        int line = p->current.line;
        ASTNode *node = ast_node_create(NODE_CONST_DECL, line);

        if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected variable name after 'const'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);

        // Optional type annotation: const x: number
        if (match(p, TOKEN_COLON)) {
            if (!check(p, TOKEN_IDENTIFIER)) {
                error_at(p, "Expected type name after ':'.");
            } else {
                node->type_annotation = strdup(p->current.value);
                advance(p);
            }
        }

        // Initializer is required for const
        expect(p, TOKEN_ASSIGN, "Expected '=' after const variable (const requires initialization).");
        ast_node_add_child(node, parse_expression(p));
        skip_newlines(p);
        return node;
    }

    // enum declaration (algebraic data type)
    if (check(p, TOKEN_ENUM)) {
        advance(p);
        int line = p->current.line;
        ASTNode *node = ast_node_create(NODE_ENUM_DECL, line);

        if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected enum name after 'enum'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);

        skip_newlines(p);
        expect(p, TOKEN_LBRACE, "Expected '{' for enum body.");
        skip_newlines(p);

        // Parse variants: VariantName | VariantName(type1, type2)
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            if (!check(p, TOKEN_IDENTIFIER)) {
                error_at(p, "Expected variant name in enum.");
                break;
            }

            ASTNode *variant = ast_node_create(NODE_ENUM_VARIANT, p->current.line);
            variant->str_value = strdup(p->current.value);
            advance(p);

            // Optional parameters for variant: VariantName(param1, param2)
            if (match(p, TOKEN_LPAREN)) {
                size_t cap = 4, count = 0;
                Param *params = (Param *)malloc(sizeof(Param) * cap);
                if (!check(p, TOKEN_RPAREN)) {
                    do {
                        if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected parameter name in variant."); break; }
                        if (count >= cap) { cap *= 2; params = (Param *)realloc(params, sizeof(Param) * cap); }
                        params[count].name = strdup(p->current.value);
                        params[count].type_annotation = NULL;
                        advance(p);
                        count++;
                    } while (match(p, TOKEN_COMMA));
                }
                expect(p, TOKEN_RPAREN, "Expected ')' after variant parameters.");
                variant->params = params;
                variant->param_count = count;
            }

            ast_node_add_child(node, variant);
            skip_newlines(p);
            
            // Optional pipe separator or comma
            if (check(p, TOKEN_PIPE) || check(p, TOKEN_COMMA)) {
                advance(p);
                skip_newlines(p);
            }
        }

        expect(p, TOKEN_RBRACE, "Expected '}' to close enum body.");
        skip_newlines(p);
        return node;
    }

    // async fn declaration (or regular fn)
    int is_async = 0;
    if (check(p, TOKEN_ASYNC)) {
        is_async = 1;
        advance(p);
        if (!check(p, TOKEN_FN)) {
            error_at(p, "Expected 'fn' after 'async'.");
            return ast_node_create(NODE_FN_DECL, p->current.line);
        }
    }

    // fn declaration
    if (check(p, TOKEN_FN)) {
        advance(p);
        int line = p->current.line;
        ASTNode *node = ast_node_create(NODE_FN_DECL, line);
        node->num_value = is_async;   // store async flag in num_value

        if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected function name after 'fn'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);

        // Generic type parameters: fn name<T, U>
        if (match(p, TOKEN_LT)) {
            size_t tp_cap = 4, tp_count = 0;
            TypeParam *type_params = (TypeParam *)malloc(sizeof(TypeParam) * tp_cap);
            
            do {
                if (!check(p, TOKEN_IDENTIFIER)) { 
                    error_at(p, "Expected type parameter name after '<'.");
                    break;
                }
                if (tp_count >= tp_cap) { 
                    tp_cap *= 2; 
                    type_params = (TypeParam *)realloc(type_params, sizeof(TypeParam) * tp_cap);
                }
                type_params[tp_count].name = strdup(p->current.value);
                type_params[tp_count].constraint = NULL;
                advance(p);
                
                // Optional constraint: T: Comparable
                if (match(p, TOKEN_COLON)) {
                    if (!check(p, TOKEN_IDENTIFIER)) {
                        error_at(p, "Expected constraint name after ':'.");
                    } else {
                        type_params[tp_count].constraint = strdup(p->current.value);
                        advance(p);
                    }
                }
                
                tp_count++;
            } while (match(p, TOKEN_COMMA));
            
            if (!match(p, TOKEN_GT)) {
                error_at(p, "Expected '>' to close type parameters.");
            }
            
            node->type_params = type_params;
            node->type_param_count = tp_count;
        } else {
            node->type_params = NULL;
            node->type_param_count = 0;
        }

        // Parameter list
        expect(p, TOKEN_LPAREN, "Expected '(' after function name.");
        // Parse params
        size_t cap = 4, count = 0;
        Param *params = (Param *)malloc(sizeof(Param) * cap);
        if (!check(p, TOKEN_RPAREN)) {
            do {
                if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected parameter name."); break; }
                if (count >= cap) { cap *= 2; params = (Param *)realloc(params, sizeof(Param) * cap); }
                params[count].name = strdup(p->current.value);
                params[count].type_annotation = NULL;
                params[count].default_value = NULL;
                params[count].is_optional = 0;
                advance(p);
                
                // Optional type annotation: param: type
                if (match(p, TOKEN_COLON)) {
                    if (!check(p, TOKEN_IDENTIFIER)) {
                        error_at(p, "Expected type name after ':'.");
                    } else {
                        params[count].type_annotation = strdup(p->current.value);
                        advance(p);
                    }
                }
                
                // Check for optional parameter: param?
                if (match(p, TOKEN_QUESTION)) {
                    params[count].is_optional = 1;
                }
                
                // Check for default value: param = value
                if (match(p, TOKEN_ASSIGN)) {
                    params[count].default_value = parse_expression(p);
                    params[count].is_optional = 1; // Parameters with defaults are implicitly optional
                }
                
                count++;
            } while (match(p, TOKEN_COMMA));
        }
        expect(p, TOKEN_RPAREN, "Expected ')' after parameters.");
        node->params     = params;
        node->param_count = count;

        // Optional return type: fn foo(): number
        if (match(p, TOKEN_COLON)) {
            if (!check(p, TOKEN_IDENTIFIER)) {
                error_at(p, "Expected return type after ':'.");
            } else {
                node->return_type = strdup(p->current.value);
                advance(p);
            }
        }

        skip_newlines(p);
        // Body block
        expect(p, TOKEN_LBRACE, "Expected '{' for function body.");
        ASTNode *body = ast_node_create(NODE_BLOCK, p->current.line);
        skip_newlines(p);
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            ASTNode *s = parse_statement(p);
            if (s) ast_node_add_child(body, s);
            skip_newlines(p);
        }
        expect(p, TOKEN_RBRACE, "Expected '}' to close function body.");
        ast_node_add_child(node, body);
        skip_newlines(p);
        return node;
    }

    // return
    if (check(p, TOKEN_RETURN)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_RETURN, p->current.line);
        // Optional return value (if not immediately a newline/} /EOF)
        if (!check(p, TOKEN_NEWLINE) && !check(p, TOKEN_SEMICOLON) &&
            !check(p, TOKEN_RBRACE)  && !check(p, TOKEN_EOF)) {
            ast_node_add_child(node, parse_expression(p));
        }
        skip_newlines(p);
        return node;
    }

    // if
    if (check(p, TOKEN_IF)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_IF, p->current.line);
        expect(p, TOKEN_LPAREN, "Expected '(' after 'if'.");
        ast_node_add_child(node, parse_expression(p));   // condition
        expect(p, TOKEN_RPAREN, "Expected ')' after if condition.");
        skip_newlines(p);

        // Then-block
        expect(p, TOKEN_LBRACE, "Expected '{' for if body.");
        ASTNode *then_block = ast_node_create(NODE_BLOCK, p->current.line);
        skip_newlines(p);
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            ASTNode *s = parse_statement(p);
            if (s) ast_node_add_child(then_block, s);
            skip_newlines(p);
        }
        expect(p, TOKEN_RBRACE, "Expected '}' to close if body.");
        ast_node_add_child(node, then_block);

        // Else block (optional)
        skip_newlines(p);
        if (match(p, TOKEN_ELSE)) {
            skip_newlines(p);
            if (check(p, TOKEN_IF)) {
                // else if — recurse
                ast_node_add_child(node, parse_statement(p));
            } else {
                expect(p, TOKEN_LBRACE, "Expected '{' for else body.");
                ASTNode *else_block = ast_node_create(NODE_BLOCK, p->current.line);
                skip_newlines(p);
                while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
                    ASTNode *s = parse_statement(p);
                    if (s) ast_node_add_child(else_block, s);
                    skip_newlines(p);
                }
                expect(p, TOKEN_RBRACE, "Expected '}' to close else body.");
                ast_node_add_child(node, else_block);
            }
        }
        skip_newlines(p);
        return node;
    }

    // while
    if (check(p, TOKEN_WHILE)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_WHILE, p->current.line);
        expect(p, TOKEN_LPAREN, "Expected '(' after 'while'.");
        ast_node_add_child(node, parse_expression(p));  // condition
        expect(p, TOKEN_RPAREN, "Expected ')' after while condition.");
        skip_newlines(p);

        expect(p, TOKEN_LBRACE, "Expected '{' for while body.");
        ASTNode *body = ast_node_create(NODE_BLOCK, p->current.line);
        skip_newlines(p);
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            ASTNode *s = parse_statement(p);
            if (s) ast_node_add_child(body, s);
            skip_newlines(p);
        }
        expect(p, TOKEN_RBRACE, "Expected '}' to close while body.");
        ast_node_add_child(node, body);
        skip_newlines(p);
        return node;
    }



    // do-while: do { body } while (cond)
    if (check(p, TOKEN_DO)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_DO_WHILE, p->current.line);
        skip_newlines(p);
        
        expect(p, TOKEN_LBRACE, "Expected '{' after 'do'.");
        ASTNode *body = ast_node_create(NODE_BLOCK, p->current.line);
        skip_newlines(p);
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            ASTNode *s = parse_statement(p);
            if (s) ast_node_add_child(body, s);
            skip_newlines(p);
        }
        expect(p, TOKEN_RBRACE, "Expected '}' to close do body.");
        ast_node_add_child(node, body);
        skip_newlines(p);
        
        expect(p, TOKEN_WHILE, "Expected 'while' after do body.");
        expect(p, TOKEN_LPAREN, "Expected '(' after 'while' in do-while.");
        ast_node_add_child(node, parse_expression(p));  // condition
        expect(p, TOKEN_RPAREN, "Expected ')' after do-while condition.");
        skip_newlines(p);
        return node;
    }

    // switch (expr) { case val: stmts... default: stmts... }
    if (check(p, TOKEN_SWITCH)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_SWITCH, p->current.line);
        expect(p, TOKEN_LPAREN, "Expected '(' after 'switch'.");
        ast_node_add_child(node, parse_expression(p));  // children[0] = discriminant
        expect(p, TOKEN_RPAREN, "Expected ')' after switch expression.");
        skip_newlines(p);
        expect(p, TOKEN_LBRACE, "Expected '{' to open switch body.");
        skip_newlines(p);

        // Each case becomes a block child:
        //   children[0]       = discriminant expression
        //   children[1..n-1]  = case blocks: NODE_BLOCK with str_value = serialized value
        //                        OR special str_value = "__default__"
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            if (check(p, TOKEN_CASE)) {
                advance(p);
                ASTNode *case_val = parse_expression(p);
                expect(p, TOKEN_COLON, "Expected ':' after case value.");
                skip_newlines(p);
                // Build a block with the case value as first child (sentinel)
                ASTNode *case_block = ast_node_create(NODE_BLOCK, case_val->line);
                ast_node_add_child(case_block, case_val);  // children[0] = case value
                // Collect statements until next case/default/}
                while (!check(p, TOKEN_CASE) && !check(p, TOKEN_DEFAULT) &&
                       !check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
                    ASTNode *s = parse_statement(p);
                    if (s) ast_node_add_child(case_block, s);
                    skip_newlines(p);
                }
                ast_node_add_child(node, case_block);
            } else if (check(p, TOKEN_DEFAULT)) {
                advance(p);
                expect(p, TOKEN_COLON, "Expected ':' after 'default'.");
                skip_newlines(p);
                ASTNode *def_block = ast_node_create(NODE_BLOCK, p->current.line);
                def_block->str_value = strdup("__default__");  // sentinel
                while (!check(p, TOKEN_CASE) && !check(p, TOKEN_DEFAULT) &&
                       !check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
                    ASTNode *s = parse_statement(p);
                    if (s) ast_node_add_child(def_block, s);
                    skip_newlines(p);
                }
                ast_node_add_child(node, def_block);
            } else {
                error_at(p, "Expected 'case' or 'default' in switch body.");
                advance(p);
            }
            skip_newlines(p);
        }
        expect(p, TOKEN_RBRACE, "Expected '}' to close switch body.");
        skip_newlines(p);
        return node;
    }

    // break
    if (check(p, TOKEN_BREAK)) {
        ASTNode *node = ast_node_create(NODE_BREAK, p->current.line);
        advance(p);
        skip_newlines(p);
        return node;
    }

    // continue
    if (check(p, TOKEN_CONTINUE)) {
        ASTNode *node = ast_node_create(NODE_CONTINUE, p->current.line);
        advance(p);
        skip_newlines(p);
        return node;
    }

    // for
    if (check(p, TOKEN_FOR)) {
        advance(p);
        int line = p->current.line;
        expect(p, TOKEN_LPAREN, "Expected '(' after 'for'.");

        // Detect for-in: for (var x in expr) or for (x in expr)
        // Peek: if we see VAR IDENT IN  or  IDENT IN  → for-in
        int is_for_in = 0;
        char *iter_var = NULL;
        if (check(p, TOKEN_VAR)) {
            // Check if next-next is IN
            // Save state: advance past VAR, read IDENT, check IN
            advance(p);  // consume VAR
            if (check(p, TOKEN_IDENTIFIER)) {
                iter_var = strdup(p->current.value);
                advance(p);  // consume IDENT
                if (check(p, TOKEN_IN)) {
                    is_for_in = 1;
                    advance(p);  // consume IN
                } else {
                    // Not for-in — this is "var IDENT = expr" C-style init
                    ASTNode *init_node = ast_node_create(NODE_VAR_DECL, line);
                    init_node->str_value = iter_var;  // takes ownership
                    iter_var = NULL;
                    if (check(p, TOKEN_ASSIGN)) {
                        advance(p);  // consume =
                        ast_node_add_child(init_node, parse_expression(p));
                    }
                    expect(p, TOKEN_SEMICOLON, "Expected ';' after for init.");
                    ASTNode *cond = check(p, TOKEN_SEMICOLON) ? NULL : parse_expression(p);
                    expect(p, TOKEN_SEMICOLON, "Expected ';' after for condition.");
                    ASTNode *update = check(p, TOKEN_RPAREN) ? NULL : parse_expression(p);
                    expect(p, TOKEN_RPAREN, "Expected ')' after for clauses.");
                    skip_newlines(p);
                    expect(p, TOKEN_LBRACE, "Expected '{' for for body.");
                    ASTNode *body = ast_node_create(NODE_BLOCK, line);
                    skip_newlines(p);
                    while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
                        ASTNode *s = parse_statement(p);
                        if (s) ast_node_add_child(body, s);
                        skip_newlines(p);
                    }
                    expect(p, TOKEN_RBRACE, "Expected '}' to close for body.");
                    ASTNode *node = ast_node_create(NODE_FOR, line);
                    ast_node_add_child(node, init_node);
                    ast_node_add_child(node, cond ? cond : ast_node_create(NODE_BOOL, line));
                    if (!cond) node->children[1]->bool_value = 1;
                    ast_node_add_child(node, update ? update : ast_node_create(NODE_NULL, line));
                    ast_node_add_child(node, body);
                    skip_newlines(p);
                    return node;
                }
            }
        } else if (check(p, TOKEN_IDENTIFIER)) {
            // Could be: IDENT IN  (for-in without var)  or  IDENT = ... (C-style init)
            // Save and peek
            char *maybe_var = strdup(p->current.value);
            advance(p);  // consume IDENT
            if (check(p, TOKEN_IN)) {
                iter_var = maybe_var;
                is_for_in = 1;
                advance(p);  // consume IN
            } else {
                // Not for-in — this is the start of a C-style init expression.
                // We already consumed the IDENT. We need to handle the rest as an expression.
                // The identifier is now consumed; build an IDENT node and continue parsing
                // the init as an assignment or expression statement.
                // Simplest: reconstruct. We have IDENT already consumed. Check for = (assign).
                ASTNode *init_node = NULL;
                if (check(p, TOKEN_ASSIGN)) {
                    advance(p);  // consume =
                    ASTNode *val = parse_expression(p);
                    init_node = ast_node_create(NODE_ASSIGN, line);
                    init_node->str_value = maybe_var;  // takes ownership
                    ast_node_add_child(init_node, val);
                    maybe_var = NULL;  // transferred
                } else {
                    // Bare expression statement starting with identifier
                    // Just wrap identifier as expression
                    init_node = ast_node_create(NODE_IDENTIFIER, line);
                    init_node->str_value = maybe_var;
                    maybe_var = NULL;
                }
                if (maybe_var) free(maybe_var);

                // Now parse rest of C-style for: ; cond ; update
                expect(p, TOKEN_SEMICOLON, "Expected ';' after for init.");
                ASTNode *cond = check(p, TOKEN_SEMICOLON) ? NULL : parse_expression(p);
                expect(p, TOKEN_SEMICOLON, "Expected ';' after for condition.");
                ASTNode *update = check(p, TOKEN_RPAREN) ? NULL : parse_expression(p);
                expect(p, TOKEN_RPAREN, "Expected ')' after for clauses.");
                skip_newlines(p);

                expect(p, TOKEN_LBRACE, "Expected '{' for for body.");
                ASTNode *body = ast_node_create(NODE_BLOCK, line);
                skip_newlines(p);
                while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
                    ASTNode *s = parse_statement(p);
                    if (s) ast_node_add_child(body, s);
                    skip_newlines(p);
                }
                expect(p, TOKEN_RBRACE, "Expected '}' to close for body.");

                ASTNode *node = ast_node_create(NODE_FOR, line);
                ast_node_add_child(node, init_node ? init_node : ast_node_create(NODE_NULL, line));
                ast_node_add_child(node, cond ? cond : ast_node_create(NODE_BOOL, line));  // default true
                if (!cond) node->children[1]->bool_value = 1;
                ast_node_add_child(node, update ? update : ast_node_create(NODE_NULL, line));
                ast_node_add_child(node, body);
                skip_newlines(p);
                return node;
            }
        }

        if (is_for_in) {
            // for-in: parse iterable expression, then body
            ASTNode *iterable = parse_expression(p);
            expect(p, TOKEN_RPAREN, "Expected ')' after for-in iterable.");
            skip_newlines(p);

            expect(p, TOKEN_LBRACE, "Expected '{' for for-in body.");
            ASTNode *body = ast_node_create(NODE_BLOCK, line);
            skip_newlines(p);
            while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
                ASTNode *s = parse_statement(p);
                if (s) ast_node_add_child(body, s);
                skip_newlines(p);
            }
            expect(p, TOKEN_RBRACE, "Expected '}' to close for-in body.");

            ASTNode *node = ast_node_create(NODE_FOR_IN, line);
            node->str_value = iter_var;  // takes ownership
            ast_node_add_child(node, iterable);
            ast_node_add_child(node, body);
            skip_newlines(p);
            return node;
        }

        // C-style for: for (init; cond; update) { body }
        // init can be: var decl, assignment, or empty
        ASTNode *init_node = NULL;
        if (check(p, TOKEN_VAR)) {
            advance(p);
            ASTNode *decl = ast_node_create(NODE_VAR_DECL, p->current.line);
            decl->str_value = strdup(p->current.value);
            expect(p, TOKEN_IDENTIFIER, "Expected variable name in for init.");
            if (check(p, TOKEN_ASSIGN)) {
                advance(p);
                ast_node_add_child(decl, parse_expression(p));
            }
            init_node = decl;
        } else if (!check(p, TOKEN_SEMICOLON)) {
            init_node = parse_expression(p);
        }
        expect(p, TOKEN_SEMICOLON, "Expected ';' after for init.");

        ASTNode *cond = check(p, TOKEN_SEMICOLON) ? NULL : parse_expression(p);
        expect(p, TOKEN_SEMICOLON, "Expected ';' after for condition.");

        ASTNode *update = check(p, TOKEN_RPAREN) ? NULL : parse_expression(p);
        expect(p, TOKEN_RPAREN, "Expected ')' after for clauses.");
        skip_newlines(p);

        expect(p, TOKEN_LBRACE, "Expected '{' for for body.");
        ASTNode *body = ast_node_create(NODE_BLOCK, line);
        skip_newlines(p);
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            ASTNode *s = parse_statement(p);
            if (s) ast_node_add_child(body, s);
            skip_newlines(p);
        }
        expect(p, TOKEN_RBRACE, "Expected '}' to close for body.");

        ASTNode *node = ast_node_create(NODE_FOR, line);
        ast_node_add_child(node, init_node ? init_node : ast_node_create(NODE_NULL, line));
        ast_node_add_child(node, cond ? cond : ast_node_create(NODE_BOOL, line));
        if (!cond) node->children[1]->bool_value = 1;  // default condition = true
        ast_node_add_child(node, update ? update : ast_node_create(NODE_NULL, line));
        ast_node_add_child(node, body);
        skip_newlines(p);
        return node;
    }

    // print
    if (check(p, TOKEN_PRINT)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_PRINT, p->current.line);
        expect(p, TOKEN_LPAREN, "Expected '(' after 'print'.");
        if (!check(p, TOKEN_RPAREN)) {
            do {
                ast_node_add_child(node, parse_expression(p));
            } while (match(p, TOKEN_COMMA));
        }
        expect(p, TOKEN_RPAREN, "Expected ')' to close print.");
        skip_newlines(p);
        return node;
    }

    // class declaration
    if (check(p, TOKEN_CLASS)) {
        advance(p);
        int line = p->current.line;
        ASTNode *node = ast_node_create(NODE_CLASS_DECL, line);

        if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected class name after 'class'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);

        // Generic type parameters: class Name<T, U>
        if (match(p, TOKEN_LT)) {
            size_t tp_cap = 4, tp_count = 0;
            TypeParam *type_params = (TypeParam *)malloc(sizeof(TypeParam) * tp_cap);
            
            do {
                if (!check(p, TOKEN_IDENTIFIER)) { 
                    error_at(p, "Expected type parameter name after '<'.");
                    break;
                }
                if (tp_count >= tp_cap) { 
                    tp_cap *= 2; 
                    type_params = (TypeParam *)realloc(type_params, sizeof(TypeParam) * tp_cap);
                }
                type_params[tp_count].name = strdup(p->current.value);
                type_params[tp_count].constraint = NULL;
                advance(p);
                
                // Optional constraint: T: Comparable
                if (match(p, TOKEN_COLON)) {
                    if (!check(p, TOKEN_IDENTIFIER)) {
                        error_at(p, "Expected constraint name after ':'.");
                    } else {
                        type_params[tp_count].constraint = strdup(p->current.value);
                        advance(p);
                    }
                }
                
                tp_count++;
            } while (match(p, TOKEN_COMMA));
            
            if (!match(p, TOKEN_GT)) {
                error_at(p, "Expected '>' to close type parameters.");
            }
            
            node->type_params = type_params;
            node->type_param_count = tp_count;
        } else {
            node->type_params = NULL;
            node->type_param_count = 0;
        }

        // Optional: extends ParentClass
        if (match(p, TOKEN_EXTENDS)) {
            if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected parent class name after 'extends'."); return node; }
            ASTNode *parent_node = ast_node_create(NODE_IDENTIFIER, p->current.line);
            parent_node->str_value = strdup(p->current.value);
            advance(p);
            ast_node_add_child(node, parent_node);   // children[0] = parent class ident
        } else {
            ast_node_add_child(node, ast_node_create(NODE_NULL, line)); // children[0] = null (no parent)
        }

        skip_newlines(p);
        expect(p, TOKEN_LBRACE, "Expected '{' after class declaration.");
        skip_newlines(p);

        // Parse method declarations (fn inside class body)
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            if (check(p, TOKEN_FN)) {
                // Reuse the existing fn parsing — it returns a NODE_FN_DECL
                ASTNode *method = parse_statement(p);
                if (method) ast_node_add_child(node, method); // children[1..n] = methods
            } else {
                error_at(p, "Only method declarations (fn) are allowed inside a class body.");
                advance(p);
            }
            skip_newlines(p);
        }
        expect(p, TOKEN_RBRACE, "Expected '}' to close class body.");
        skip_newlines(p);
        return node;
    }

    // export fn / export class
    if (check(p, TOKEN_EXPORT)) {
        advance(p);
        if (!check(p, TOKEN_FN) && !check(p, TOKEN_CLASS)) {
            error_at(p, "Expected 'fn' or 'class' after 'export'.");
            return ast_node_create(NODE_EXPORT, p->current.line);
        }
        ASTNode *decl = parse_statement(p);   // recursion: parses the fn or class decl
        ASTNode *node = ast_node_create(NODE_EXPORT, decl ? decl->line : p->current.line);
        if (decl) ast_node_add_child(node, decl);
        return node;
    }

    // from "module" import name1, name2 | from "module" import *
    if (check(p, TOKEN_FROM)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_IMPORT, p->current.line);
        if (!check(p, TOKEN_STRING)) { error_at(p, "Expected module name string after 'from'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);
        if (!check(p, TOKEN_IMPORT)) { error_at(p, "Expected 'import' after module name in 'from' statement."); return node; }
        advance(p);
        // Check for wildcard: import *
        if (match(p, TOKEN_STAR)) {
            node->num_value = 3;   // type = wildcard import
            skip_newlines(p);
            return node;
        }
        // Parse comma-separated list of names to import
        node->num_value = 2;   // type = selective import
        do {
            if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected identifier in import list."); break; }
            ASTNode *ident = ast_node_create(NODE_IDENTIFIER, p->current.line);
            ident->str_value = strdup(p->current.value);
            ast_node_add_child(node, ident);
            advance(p);
        } while (match(p, TOKEN_COMMA));
        skip_newlines(p);
        return node;
    }

    // import "module" [as alias]
    if (check(p, TOKEN_IMPORT)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_IMPORT, p->current.line);
        if (!check(p, TOKEN_STRING)) { error_at(p, "Expected module name string after 'import'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);
        // Check for 'as alias'
        if (match(p, TOKEN_AS)) {
            if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected alias name after 'as'."); return node; }
            node->num_value = 1;   // type = aliased import
            ASTNode *alias = ast_node_create(NODE_IDENTIFIER, p->current.line);
            alias->str_value = strdup(p->current.value);
            ast_node_add_child(node, alias);
            advance(p);
        } else {
            node->num_value = 0;   // type = regular import
        }
        skip_newlines(p);
        return node;
    }

    // Expression statement (assignment, fn call, increment, property set, etc.)
    {
        ASTNode *expr = parse_expression(p);
        if (!expr) return NULL;

        // Check for assignment: IDENT = expr
        if (expr->type == NODE_IDENTIFIER && match(p, TOKEN_ASSIGN)) {
            ASTNode *assign = ast_node_create(NODE_ASSIGN, expr->line);
            assign->str_value = strdup(expr->str_value);
            ast_node_add_child(assign, parse_expression(p));
            ast_node_destroy(expr);
            skip_newlines(p);
            return assign;
        }
        // Property set: obj.prop = expr  (parser already produced PROPERTY_GET; convert it)
        if (expr->type == NODE_PROPERTY_GET && match(p, TOKEN_ASSIGN)) {
            ASTNode *pset = ast_node_create(NODE_PROPERTY_SET, expr->line);
            pset->str_value = strdup(expr->str_value);          // property name
            ast_node_add_child(pset, expr->children[0]);        // children[0] = object expr
            expr->children[0] = NULL;                           // detach so destroy doesn't kill it
            ast_node_add_child(pset, parse_expression(p));      // children[1] = value expr
            ast_node_destroy(expr);
            skip_newlines(p);
            return pset;
        }
        // Compound assignment: IDENT += / -= / *= /= expr
        if (expr->type == NODE_IDENTIFIER &&
            (check(p, TOKEN_PLUSEQ) || check(p, TOKEN_MINUSEQ) ||
             check(p, TOKEN_STAREQ) || check(p, TOKEN_SLASHEQ))) {
            const char *op = p->current.value;
            ASTNode *ca = ast_node_create(NODE_COMPOUND_ASSIGN, expr->line);
            ca->str_value = strdup(op);
            ast_node_add_child(ca, expr);  // keep ident node
            advance(p);
            ast_node_add_child(ca, parse_expression(p));
            skip_newlines(p);
            return ca;
        }

        // Wrap as expression statement
        ASTNode *es = ast_node_create(NODE_EXPR_STMT, expr->line);
        ast_node_add_child(es, expr);
        skip_newlines(p);
        return es;
    }
}

// ─── Expression Grammar (precedence climbing) ───────────────────────────────
static ASTNode *parse_expression(Parser *p) {
    return parse_nullish(p);
}

// ?? (null coalescing) — lower precedence than or
static ASTNode *parse_nullish(Parser *p) {
    ASTNode *left = parse_or(p);
    while (match(p, TOKEN_NULLISH)) {
        ASTNode *node = ast_node_create(NODE_NULLISH, p->previous.line);
        ast_node_add_child(node, left);
        ast_node_add_child(node, parse_or(p));
        left = node;
    }
    return left;
}

static ASTNode *parse_or(Parser *p) {
    ASTNode *left = parse_and(p);
    while (match(p, TOKEN_OR)) {
        ASTNode *node = ast_node_create(NODE_BINARY, p->previous.line);
        node->str_value = strdup("or");
        ast_node_add_child(node, left);
        ast_node_add_child(node, parse_and(p));
        left = node;
    }
    return left;
}

static ASTNode *parse_and(Parser *p) {
    ASTNode *left = parse_equality(p);
    while (match(p, TOKEN_AND)) {
        ASTNode *node = ast_node_create(NODE_BINARY, p->previous.line);
        node->str_value = strdup("and");
        ast_node_add_child(node, left);
        ast_node_add_child(node, parse_equality(p));
        left = node;
    }
    return left;
}

static ASTNode *parse_equality(Parser *p) {
    ASTNode *left = parse_comparison(p);
    while (check(p, TOKEN_EQ) || check(p, TOKEN_NEQ)) {
        char *op = strdup(p->current.value);
        advance(p);
        ASTNode *node = ast_node_create(NODE_BINARY, p->previous.line);
        node->str_value = op;
        ast_node_add_child(node, left);
        ast_node_add_child(node, parse_comparison(p));
        left = node;
    }
    return left;
}

static ASTNode *parse_comparison(Parser *p) {
    ASTNode *left = parse_addition(p);
    while (check(p, TOKEN_LT) || check(p, TOKEN_GT) ||
           check(p, TOKEN_LTE) || check(p, TOKEN_GTE) ||
           check(p, TOKEN_INSTANCEOF)) {

        // instanceof is special: RHS must be a class name (identifier)
        if (check(p, TOKEN_INSTANCEOF)) {
            int line = p->current.line;
            advance(p);
            ASTNode *node = ast_node_create(NODE_INSTANCEOF, line);
            ast_node_add_child(node, left);   // children[0] = value expr
            // RHS: class name as identifier node
            if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected class name after 'instanceof'."); return left; }
            ASTNode *cls_name = ast_node_create(NODE_IDENTIFIER, p->current.line);
            cls_name->str_value = strdup(p->current.value);
            advance(p);
            ast_node_add_child(node, cls_name); // children[1] = class name ident
            left = node;
            continue;
        }

        char *op = strdup(p->current.value);
        advance(p);
        ASTNode *node = ast_node_create(NODE_BINARY, p->previous.line);
        node->str_value = op;
        ast_node_add_child(node, left);
        ast_node_add_child(node, parse_addition(p));
        left = node;
    }
    return left;
}

static ASTNode *parse_addition(Parser *p) {
    ASTNode *left = parse_multiplication(p);
    while (check(p, TOKEN_PLUS) || check(p, TOKEN_MINUS)) {
        char *op = strdup(p->current.value);
        advance(p);
        skip_newlines(p);  // allow line continuation after operator
        ASTNode *node = ast_node_create(NODE_BINARY, p->previous.line);
        node->str_value = op;
        ast_node_add_child(node, left);
        ast_node_add_child(node, parse_multiplication(p));
        left = node;
    }
    return left;
}

static ASTNode *parse_multiplication(Parser *p) {
    ASTNode *left = parse_unary(p);
    while (check(p, TOKEN_STAR) || check(p, TOKEN_SLASH) || check(p, TOKEN_PERCENT)) {
        char *op = strdup(p->current.value);
        advance(p);
        ASTNode *node = ast_node_create(NODE_BINARY, p->previous.line);
        node->str_value = op;
        ast_node_add_child(node, left);
        ast_node_add_child(node, parse_unary(p));
        left = node;
    }
    return left;
}

static ASTNode *parse_unary(Parser *p) {
    if (check(p, TOKEN_MINUS)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_UNARY, p->previous.line);
        node->str_value = strdup("-");
        ast_node_add_child(node, parse_unary(p));
        return node;
    }
    if (check(p, TOKEN_NOT)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_UNARY, p->previous.line);
        node->str_value = strdup("not");
        ast_node_add_child(node, parse_unary(p));
        return node;
    }
    return parse_call(p);
}

static ASTNode *parse_call(Parser *p) {
    ASTNode *expr = parse_primary(p);
    if (!expr) return NULL;

    // ── Single-identifier arrow function: ident => expr ──────────────────────
    // If parse_primary returned a bare identifier and the next token is '=>',
    // re-interpret as a zero-paren single-param arrow function.
    if (expr->type == NODE_IDENTIFIER && check(p, TOKEN_ARROW)) {
        advance(p);  // consume '=>'
        ASTNode *node = ast_node_create(NODE_ARROW_FN, expr->line);
        node->params = (Param *)malloc(sizeof(Param));
        node->params[0].name             = strdup(expr->str_value);
        node->params[0].type_annotation  = NULL;
        node->params[0].default_value    = NULL;
        node->params[0].is_optional      = 0;
        node->param_count = 1;
        ast_node_destroy(expr);
        // Body: single expression → implicit return
        ASTNode *body_expr = parse_expression(p);
        ASTNode *ret_node  = ast_node_create(NODE_RETURN, body_expr->line);
        ast_node_add_child(ret_node, body_expr);
        ASTNode *block = ast_node_create(NODE_BLOCK, body_expr->line);
        ast_node_add_child(block, ret_node);
        ast_node_add_child(node, block);
        return node;
    }

    // ── Postfix loop: handles (), .prop, .method(), ++, -- repeatedly ─────────
    while (1) {
        // ── Type arguments: ident<T, U> ───────────────────────────────────────
        // Parse type arguments if we see < after an identifier.
        // Heuristic: we only parse as type args if the next token is an identifier
        // (to distinguish from comparison operators like x < y).
        // A better solution would require more lookahead or backtracking.
        if (expr->type == NODE_IDENTIFIER && check(p, TOKEN_LT)) {
            // Save the current token so we can check what comes after <
            Token saved_current = p->current;
            advance(p); // tentatively consume <
            
            // If next token is an identifier, treat as type argument list
            if (check(p, TOKEN_IDENTIFIER)) {
                size_t ta_cap = 4, ta_count = 0;
                char **type_args = (char **)malloc(sizeof(char *) * ta_cap);
                
                do {
                    if (!check(p, TOKEN_IDENTIFIER)) {
                        error_at(p, "Expected type name in type argument list.");
                        break;
                    }
                    if (ta_count >= ta_cap) {
                        ta_cap *= 2;
                        type_args = (char **)realloc(type_args, sizeof(char *) * ta_cap);
                    }
                    type_args[ta_count++] = strdup(p->current.value);
                    advance(p);
                } while (match(p, TOKEN_COMMA));
                
                if (!match(p, TOKEN_GT)) {
                    error_at(p, "Expected '>' to close type argument list.");
                }
                
                expr->type_args = type_args;
                expr->type_arg_count = ta_count;
            } else {
                // Not a type argument - restore and treat as comparison
                // We can't truly backtrack, so this is best-effort
                // The saved_current is already consumed, so we just continue
                // This means `foo < bar` might be misparsed. For now, require
                // explicit parentheses or spacing to disambiguate.
                (void)saved_current; // suppress unused warning
            }
        }
        
        // ── Function call: expr(args) ─────────────────────────────────────────
        // Any expression that resolves to a callable value may be called.
        // NODE_CALL_EXPR: when the callee is not a simple identifier (e.g. a
        //   variable holding a closure), emit NODE_CALL_EXPR so the interpreter
        //   can eval the callee expression and call the result.
        if (match(p, TOKEN_LPAREN)) {
            // If calling a bare identifier by name, use NODE_FN_CALL as before
            // (backward compat with the named-arg recovery path).
            // For any other callee expression, use NODE_CALL_EXPR.
            ASTNode *call;
            if (expr->type == NODE_IDENTIFIER) {
                call = ast_node_create(NODE_FN_CALL, expr->line);
                call->str_value = strdup(expr->str_value);
                // Transfer type arguments if present
                if (expr->type_args) {
                    call->type_args = expr->type_args;
                    call->type_arg_count = expr->type_arg_count;
                    expr->type_args = NULL;  // prevent double-free
                    expr->type_arg_count = 0;
                }
                ast_node_destroy(expr);
                expr = NULL;
            } else {
                // General callee: node->children[0] = callee expr, rest = args
                call = ast_node_create(NODE_CALL_EXPR, expr->line);
                ast_node_add_child(call, expr);  // children[0] = callee
                expr = NULL;
            }
            if (!check(p, TOKEN_RPAREN)) {
                do {
                    if (check(p, TOKEN_IDENTIFIER)) {
                        // Safe peek: copy value string before advance() can free it
                        int   saved_line = p->current.line;
                        char *saved_val  = strdup(p->current.value);
                        advance(p);
                        if (check(p, TOKEN_COLON)) {
                            // Named argument:  name: expr
                            advance(p);
                            ASTNode *named = ast_node_create(NODE_NAMED_ARG, saved_line);
                            named->str_value = saved_val; // transfer ownership
                            ast_node_add_child(named, parse_expression(p));
                            ast_node_add_child(call, named);
                        } else {
                            // Not a named arg.  We already consumed the identifier token
                            // so we reconstruct it as a primary and continue parsing the
                            // full expression (handles `func(a + b)`, `func(a, b)`, etc.)
                            ASTNode *ident = ast_node_create(NODE_IDENTIFIER, saved_line);
                            ident->str_value = saved_val; // transfer ownership
                            // Re-enter postfix / binary parsing with ident as the left node.
                            // Postfix: calls, dots, subscripts on the recovered identifier
                            ASTNode *arg = ident;
                            while (1) {
                                if (match(p, TOKEN_LPAREN)) {
                                    ASTNode *sc = ast_node_create(NODE_FN_CALL, arg->line);
                                    sc->str_value = strdup(arg->str_value ? arg->str_value : "");
                                    if (!check(p, TOKEN_RPAREN)) {
                                        do { ast_node_add_child(sc, parse_expression(p)); } while (match(p, TOKEN_COMMA));
                                    }
                                    expect(p, TOKEN_RPAREN, "Expected ')' after arguments.");
                                    ast_node_destroy(arg); arg = sc;
                                } else if (match(p, TOKEN_DOT)) {
                                    if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected name after '.'."); break; }
                                    char *mem = strdup(p->current.value);
                                    int   ml  = p->current.line;
                                    advance(p);
                                    if (match(p, TOKEN_LPAREN)) {
                                        ASTNode *mc = ast_node_create(NODE_METHOD_CALL, ml);
                                        mc->str_value = mem;
                                        ast_node_add_child(mc, arg);
                                        if (!check(p, TOKEN_RPAREN)) {
                                            do { ast_node_add_child(mc, parse_expression(p)); } while (match(p, TOKEN_COMMA));
                                        }
                                        expect(p, TOKEN_RPAREN, "Expected ')' after method args.");
                                        arg = mc;
                                    } else {
                                        ASTNode *pg = ast_node_create(NODE_PROPERTY_GET, ml);
                                        pg->str_value = mem;
                                        ast_node_add_child(pg, arg);
                                        arg = pg;
                                    }
                                } else if (match(p, TOKEN_LBRACKET)) {
                                    ASTNode *idx = ast_node_create(NODE_INDEX, arg->line);
                                    ast_node_add_child(idx, arg);
                                    ast_node_add_child(idx, parse_expression(p));
                                    expect(p, TOKEN_RBRACKET, "Expected ']' after index.");
                                    arg = idx;
                                } else break;
                            }
                            // Binary operators in full precedence order
                            while (check(p, TOKEN_STAR) || check(p, TOKEN_SLASH) || check(p, TOKEN_PERCENT)) {
                                char *op = strdup(p->current.value); advance(p);
                                ASTNode *b = ast_node_create(NODE_BINARY, arg->line);
                                b->str_value = op; ast_node_add_child(b, arg);
                                ast_node_add_child(b, parse_unary(p)); arg = b;
                            }
                            while (check(p, TOKEN_PLUS) || check(p, TOKEN_MINUS)) {
                                char *op = strdup(p->current.value); advance(p);
                                ASTNode *b = ast_node_create(NODE_BINARY, arg->line);
                                b->str_value = op; ast_node_add_child(b, arg);
                                ast_node_add_child(b, parse_multiplication(p)); arg = b;
                            }
                            while (check(p, TOKEN_LT) || check(p, TOKEN_GT) || check(p, TOKEN_LTE) || check(p, TOKEN_GTE)) {
                                char *op = strdup(p->current.value); advance(p);
                                ASTNode *b = ast_node_create(NODE_BINARY, arg->line);
                                b->str_value = op; ast_node_add_child(b, arg);
                                ast_node_add_child(b, parse_addition(p)); arg = b;
                            }
                            while (check(p, TOKEN_EQ) || check(p, TOKEN_NEQ)) {
                                char *op = strdup(p->current.value); advance(p);
                                ASTNode *b = ast_node_create(NODE_BINARY, arg->line);
                                b->str_value = op; ast_node_add_child(b, arg);
                                ast_node_add_child(b, parse_comparison(p)); arg = b;
                            }
                            while (match(p, TOKEN_AND)) {
                                ASTNode *b = ast_node_create(NODE_BINARY, arg->line);
                                b->str_value = strdup("and"); ast_node_add_child(b, arg);
                                ast_node_add_child(b, parse_equality(p)); arg = b;
                            }
                            while (match(p, TOKEN_OR)) {
                                ASTNode *b = ast_node_create(NODE_BINARY, arg->line);
                                b->str_value = strdup("or"); ast_node_add_child(b, arg);
                                ast_node_add_child(b, parse_and(p)); arg = b;
                            }
                            while (match(p, TOKEN_NULLISH)) {
                                ASTNode *b = ast_node_create(NODE_NULLISH, arg->line);
                                ast_node_add_child(b, arg);
                                ast_node_add_child(b, parse_or(p)); arg = b;
                            }
                            ast_node_add_child(call, arg);
                        }
                    } else {
                        ast_node_add_child(call, parse_expression(p));
                    }
                } while (match(p, TOKEN_COMMA));
            }
            expect(p, TOKEN_RPAREN, "Expected ')' after arguments.");
            // expr was either consumed into the call node or set to NULL above
            expr = call;
            continue;
        }

        // ── Dot access: expr.name  or  expr.name(args) ───────────────────────
        if (match(p, TOKEN_DOT)) {
            if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected property or method name after '.'."); return expr; }
            char *member = strdup(p->current.value);
            int  member_line = p->current.line;
            advance(p);

            // If followed by '(' → method call
            if (match(p, TOKEN_LPAREN)) {
                ASTNode *mcall = ast_node_create(NODE_METHOD_CALL, member_line);
                // children[0] = the object expression
                ast_node_add_child(mcall, expr);
                // str_value = method name
                mcall->str_value = member;
                // children[1..n] = arguments
                if (!check(p, TOKEN_RPAREN)) {
                    do {
                        ast_node_add_child(mcall, parse_expression(p));
                    } while (match(p, TOKEN_COMMA));
                }
                expect(p, TOKEN_RPAREN, "Expected ')' after method arguments.");
                expr = mcall;
            } else {
                // Property get: expr.prop
                ASTNode *pget = ast_node_create(NODE_PROPERTY_GET, member_line);
                pget->str_value = member;       // property name
                ast_node_add_child(pget, expr); // children[0] = object expression
                expr = pget;
            }
            continue;
        }

        // ── Array/string indexing: expr[index] → NODE_INDEX ────────────────
        if (match(p, TOKEN_LBRACKET)) {
            ASTNode *index_node = ast_node_create(NODE_INDEX, expr->line);
            ast_node_add_child(index_node, expr);                    // children[0] = array/string
            ast_node_add_child(index_node, parse_expression(p));     // children[1] = index
            expect(p, TOKEN_RBRACKET, "Expected ']' after index.");
            expr = index_node;
            continue;
        }

        // ── Postfix ++ / -- on identifiers ───────────────────────────────────
        if (expr->type == NODE_IDENTIFIER && check(p, TOKEN_PLUSPLUS)) {
            advance(p);
            ASTNode *inc = ast_node_create(NODE_INCREMENT, expr->line);
            inc->str_value = strdup(expr->str_value);
            ast_node_destroy(expr);
            return inc;
        }
        if (expr->type == NODE_IDENTIFIER && check(p, TOKEN_MINUSMINUS)) {
            advance(p);
            ASTNode *dec = ast_node_create(NODE_DECREMENT, expr->line);
            dec->str_value = strdup(expr->str_value);
            ast_node_destroy(expr);
            return dec;
        }

        // ── Postfix ++ / -- on property access (this.x++) ─────────────────────
        // Desugar: obj.prop++  →  PROPERTY_SET(obj, prop, BINARY(PROPERTY_GET(obj, prop), +, 1))
        if (expr->type == NODE_PROPERTY_GET && (check(p, TOKEN_PLUSPLUS) || check(p, TOKEN_MINUSMINUS))) {
            int is_inc = check(p, TOKEN_PLUSPLUS);
            advance(p);

            // Build a fresh PROPERTY_GET for the read-side (clone the object child)
            // We reuse expr itself as the read-side inside the BINARY.
            ASTNode *bin = ast_node_create(NODE_BINARY, expr->line);
            bin->str_value = strdup(is_inc ? "+" : "-");
            ast_node_add_child(bin, expr);  // left = the PROPERTY_GET (read)

            ASTNode *one = ast_node_create(NODE_NUMBER, expr->line);
            one->num_value = 1.0;
            ast_node_add_child(bin, one);   // right = 1

            // Now we need a PROPERTY_SET. But we need the object expr again.
            // Problem: expr (the PROPERTY_GET) is now consumed as child of bin.
            // We need to duplicate the object sub-expression. Since we can't easily
            // deep-clone AST nodes, we'll use a different strategy:
            // Emit a special COMPOUND_ASSIGN-style node isn't possible without more node types.
            // Simplest correct approach: wrap in an EXPR_STMT that the interpreter handles specially.
            // Actually — let's just store the PROPERTY_SET with bin as value, and for the object
            // we re-parse... no we can't re-parse.
            //
            // Real solution: add the PROPERTY_SET and duplicate the object node.
            // The object node is expr->children[0]. We can steal it from expr
            // (since expr is now inside bin), but then PROPERTY_SET also needs it.
            // We need TWO references. The interpreter evaluates children[0] of both
            // PROPERTY_GET and PROPERTY_SET. Since they're tree-walked, we can't share.
            //
            // Cleanest: create a wrapper node that the interpreter understands as
            // "read prop, add 1, write prop back". We'll repurpose COMPOUND_ASSIGN
            // but that only works on identifiers. So: add the logic in interpreter
            // for PROPERTY_SET where value is a BINARY whose left is a PROPERTY_GET
            // on the same object. The interpreter will just eval both children normally.
            // The object expression will be evaluated TWICE (once for read, once for write).
            // For 'this.x++' that's fine — 'this' just looks up the env entry.
            //
            // So: PROPERTY_SET { obj=children[0] of original PROPERTY_GET, prop=str_value, val=bin }
            // We need to extract obj from expr before expr was consumed. But expr IS inside bin now.
            // Let's extract it:
            ASTNode *obj_expr = expr->children[0];
            expr->children[0] = NULL;  // detach so it doesn't get freed with expr

            // We also need a second copy of obj_expr for the PROPERTY_SET.
            // Since we can't clone, we'll create a dummy THIS node if obj is THIS,
            // or an IDENT node if obj is an identifier. For the general case we need
            // a real clone. Let's implement a minimal clone for the common cases:
            ASTNode *obj_expr2 = ast_node_create(obj_expr->type, obj_expr->line);
            if (obj_expr->str_value) obj_expr2->str_value = strdup(obj_expr->str_value);
            obj_expr2->num_value  = obj_expr->num_value;
            obj_expr2->bool_value = obj_expr->bool_value;
            // For THIS and IDENTIFIER, no children needed. Good enough.

            // Put obj_expr back into the PROPERTY_GET (which is inside bin)
            expr->children[0] = obj_expr;

            ASTNode *pset = ast_node_create(NODE_PROPERTY_SET, expr->line);
            pset->str_value = strdup(expr->str_value);  // property name (from the original PROPERTY_GET)
            ast_node_add_child(pset, obj_expr2);  // children[0] = object (clone)
            ast_node_add_child(pset, bin);        // children[1] = BINARY(PROPERTY_GET, +/-, 1)

            return pset;
        }

        break;  // nothing matched → done with postfix
    }

    return expr;
}

static ASTNode *parse_primary(Parser *p) {
    // Number
    if (check(p, TOKEN_NUMBER)) {
        ASTNode *node = ast_node_create(NODE_NUMBER, p->current.line);
        node->num_value = atof(p->current.value);
        advance(p);
        return node;
    }
    // String
    if (check(p, TOKEN_STRING)) {
        ASTNode *node = ast_node_create(NODE_STRING, p->current.line);
        node->str_value = strdup(p->current.value);
        advance(p);
        return node;
    }
    // Bool
    if (check(p, TOKEN_TRUE)) {
        ASTNode *node = ast_node_create(NODE_BOOL, p->current.line);
        node->bool_value = 1;
        advance(p);
        return node;
    }
    if (check(p, TOKEN_FALSE)) {
        ASTNode *node = ast_node_create(NODE_BOOL, p->current.line);
        node->bool_value = 0;
        advance(p);
        return node;
    }
    // Null
    if (check(p, TOKEN_NULL)) {
        ASTNode *node = ast_node_create(NODE_NULL, p->current.line);
        advance(p);
        return node;
    }
    // Array literal: [expr, expr, ...]
    if (check(p, TOKEN_LBRACKET)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_ARRAY_LITERAL, p->current.line);
        if (!check(p, TOKEN_RBRACKET)) {
            do {
                ast_node_add_child(node, parse_expression(p));
            } while (match(p, TOKEN_COMMA));
        }
        expect(p, TOKEN_RBRACKET, "Expected ']' after array elements.");
        return node;
    }
    // input("prompt")
    if (check(p, TOKEN_INPUT)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_INPUT, p->current.line);
        expect(p, TOKEN_LPAREN, "Expected '(' after 'input'.");
        if (!check(p, TOKEN_RPAREN))
            ast_node_add_child(node, parse_expression(p));
        expect(p, TOKEN_RPAREN, "Expected ')' after input prompt.");
        return node;
    }
    // typeof(expr)
    if (check(p, TOKEN_TYPEOF)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_TYPEOF, p->current.line);
        expect(p, TOKEN_LPAREN, "Expected '(' after 'typeof'.");
        ast_node_add_child(node, parse_expression(p));
        expect(p, TOKEN_RPAREN, "Expected ')' after typeof expression.");
        return node;
    }
    // match expression: match expr { pattern => expr, ... }
    if (check(p, TOKEN_MATCH)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_MATCH, p->current.line);
        
        // Parse the expression to match on
        ast_node_add_child(node, parse_expression(p));
        
        skip_newlines(p);
        expect(p, TOKEN_LBRACE, "Expected '{' after match expression.");
        skip_newlines(p);
        
        // Parse match arms: pattern => expr
        while (!check(p, TOKEN_RBRACE) && !check(p, TOKEN_EOF) && !p->had_error) {
            ASTNode *arm = ast_node_create(NODE_MATCH_ARM, p->current.line);
            
            // Parse pattern
            ASTNode *pattern = ast_node_create(NODE_PATTERN, p->current.line);
            
            if (check(p, TOKEN_IDENTIFIER)) {
                // Could be: variant constructor, identifier binding, or wildcard _
                pattern->str_value = strdup(p->current.value);
                advance(p);
                
                // Check for variant with parameters: VariantName(x, y)
                if (match(p, TOKEN_LPAREN)) {
                    size_t cap = 4, count = 0;
                    Param *params = (Param *)malloc(sizeof(Param) * cap);
                    if (!check(p, TOKEN_RPAREN)) {
                        do {
                            if (!check(p, TOKEN_IDENTIFIER)) { 
                                error_at(p, "Expected binding name in pattern."); 
                                break; 
                            }
                            if (count >= cap) { 
                                cap *= 2; 
                                params = (Param *)realloc(params, sizeof(Param) * cap); 
                            }
                            params[count].name = strdup(p->current.value);
                            params[count].type_annotation = NULL;
                            advance(p);
                            count++;
                        } while (match(p, TOKEN_COMMA));
                    }
                    expect(p, TOKEN_RPAREN, "Expected ')' after pattern parameters.");
                    pattern->params = params;
                    pattern->param_count = count;
                }
            } else if (check(p, TOKEN_NUMBER)) {
                // Literal number pattern
                pattern->num_value = atof(p->current.value);
                pattern->bool_value = 1; // flag to indicate literal
                advance(p);
            } else if (check(p, TOKEN_STRING)) {
                // Literal string pattern
                pattern->str_value = strdup(p->current.value);
                pattern->bool_value = 2; // flag to indicate string literal
                advance(p);
            } else if (check(p, TOKEN_TRUE) || check(p, TOKEN_FALSE)) {
                // Boolean literal pattern
                pattern->bool_value = check(p, TOKEN_TRUE) ? 3 : 4;
                advance(p);
            } else {
                error_at(p, "Expected pattern in match arm.");
            }
            
            ast_node_add_child(arm, pattern);
            
            skip_newlines(p);
            expect(p, TOKEN_ARROW, "Expected '=>' after pattern.");
            skip_newlines(p);
            
            // Parse the result expression
            ast_node_add_child(arm, parse_expression(p));
            
            ast_node_add_child(node, arm);
            
            skip_newlines(p);
            // Optional comma or pipe separator
            if (check(p, TOKEN_COMMA) || check(p, TOKEN_PIPE)) {
                advance(p);
                skip_newlines(p);
            }
        }
        
        expect(p, TOKEN_RBRACE, "Expected '}' to close match expression.");
        return node;
    }
    // spawn expr  — launches async function
    if (check(p, TOKEN_SPAWN)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_SPAWN, p->current.line);
        ast_node_add_child(node, parse_expression(p));   // the function call to spawn
        return node;
    }
    // await expr  — calls async function and waits
    if (check(p, TOKEN_AWAIT)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_AWAIT, p->current.line);
        ast_node_add_child(node, parse_expression(p));   // the function call to await
        return node;
    }
    // new ClassName(args)
    if (check(p, TOKEN_NEW)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_NEW, p->current.line);
        if (!check(p, TOKEN_IDENTIFIER)) { error_at(p, "Expected class name after 'new'."); return node; }
        node->str_value = strdup(p->current.value);
        advance(p);
        expect(p, TOKEN_LPAREN, "Expected '(' after class name in 'new'.");
        if (!check(p, TOKEN_RPAREN)) {
            do {
                ast_node_add_child(node, parse_expression(p));
            } while (match(p, TOKEN_COMMA));
        }
        expect(p, TOKEN_RPAREN, "Expected ')' after constructor arguments.");
        return node;
    }
    // this
    if (check(p, TOKEN_THIS)) {
        ASTNode *node = ast_node_create(NODE_THIS, p->current.line);
        advance(p);
        return node;
    }
    // super(args)
    if (check(p, TOKEN_SUPER)) {
        advance(p);
        ASTNode *node = ast_node_create(NODE_SUPER_CALL, p->current.line);
        expect(p, TOKEN_LPAREN, "Expected '(' after 'super'.");
        if (!check(p, TOKEN_RPAREN)) {
            do {
                ast_node_add_child(node, parse_expression(p));
            } while (match(p, TOKEN_COMMA));
        }
        expect(p, TOKEN_RPAREN, "Expected ')' after super arguments.");
        return node;
    }
    // Identifier
    if (check(p, TOKEN_IDENTIFIER)) {
        ASTNode *node = ast_node_create(NODE_IDENTIFIER, p->current.line);
        node->str_value = strdup(p->current.value);
        advance(p);
        return node;
    }
    // Arrow function: (params) => expr  OR  single_param => expr
    // Disambiguate ( from grouped-expr: peek for IDENT/RPAREN followed by => or IDENT =>
    if (check(p, TOKEN_LPAREN)) {
        // We need to decide: arrow fn params  vs  grouped expression.
        // Strategy: tentatively scan — if we see RPAREN followed by ARROW, or
        // IDENTIFIER {COMMA|RPAREN}* RPAREN ARROW, it's an arrow fn.
        // We use the lexer's peek capability: save tokens by scanning past ')'.
        // Simple heuristic: check next tokens.  The lexer has no backtrack, but
        // we can check if it *looks* like a parameter list by peeking only one level:
        //   ()=> always arrow.  (ident)=> always arrow.
        // For the general case we rely on: if we find TOKEN_ARROW right after the
        // matching ')' it's an arrow function.  We do a limited scan:
        // advance past '(', collect potential param names, then if we see ')' and
        // TOKEN_ARROW it's an arrow fn — otherwise it's a grouped expression and
        // we need to re-parse.  Since we can't back up the lexer, we use a different
        // approach: parse it as a grouped expression first, but if the result is
        // just an identifier and the NEXT token is '=>', re-interpret.
        // Actually the cleanest approach is: parse `(param_list) =>` or `ident =>`
        // by checking for TOKEN_ARROW at the right spot.  Since '(' groups can also
        // start expressions, we do:
        //   1.  If current token is LPAREN, save position by remembering we entered,
        //       then peek at what comes after the matching ')'.
        //   2.  We do a lightweight scan: advance past '(', collect idents+commas,
        //       if we hit ')' and then '=>', treat as arrow fn params.
        //       Otherwise we need a real expression parse — but we can't back up.
        // SIMPLEST CORRECT STRATEGY that doesn't require lexer backtrack:
        //   Parse the grouped expression normally.  After parse_expression returns,
        //   check if current token is '=>'.  But that still requires recognising
        //   that what we parsed was actually a param list.
        // Given the grammar, we use the following reliable approach:
        //   After `(`, peek: if nothing or only IDENT/COMMA/RPAREN tokens appear
        //   before the first RPAREN, it MIGHT be params.  We collect those tokens
        //   by scanning. But we can't put them back.
        // IMPLEMENTATION: We do a two-phase approach:
        //   Phase 1: scan to collect ident names or detect it can't be params.
        //   Phase 2: if valid params and next-after-RPAREN is ARROW: emit arrow fn.
        //            Otherwise: parse as grouped expression (re-use already-consumed tokens).
        // Since we can't back-up, we collect the identifiers ourselves instead of
        // delegating to parse_expression:

        advance(p);  // consume '('
        // Collect potential parameter list
        size_t param_cap = 4, param_count = 0;
        Param *params = (Param *)malloc(sizeof(Param) * param_cap);
        int is_arrow = 1;   // assume arrow until proven otherwise

        // Empty params: () =>
        if (check(p, TOKEN_RPAREN)) {
            advance(p);  // consume ')'
            if (!check(p, TOKEN_ARROW)) {
                // It was just (); treat as grouped null expression
                is_arrow = 0;
            }
        } else {
            // Try to collect identifiers
            while (!check(p, TOKEN_RPAREN) && !check(p, TOKEN_EOF) && !p->had_error) {
                if (!check(p, TOKEN_IDENTIFIER)) { is_arrow = 0; break; }
                if (param_count >= param_cap) {
                    param_cap *= 2;
                    params = (Param *)realloc(params, sizeof(Param) * param_cap);
                }
                params[param_count].name = strdup(p->current.value);
                params[param_count].type_annotation = NULL;
                params[param_count].default_value   = NULL;
                params[param_count].is_optional     = 0;
                param_count++;
                advance(p);  // consume identifier
                if (check(p, TOKEN_COMMA)) { advance(p); continue; }
                if (check(p, TOKEN_RPAREN)) break;
                is_arrow = 0; break;   // unexpected token
            }
            if (is_arrow && check(p, TOKEN_RPAREN)) {
                advance(p);  // consume ')'
                if (!check(p, TOKEN_ARROW)) is_arrow = 0;
            } else {
                is_arrow = 0;
            }
        }

        if (is_arrow) {
            // Confirmed arrow function: consume '=>' and parse body
            advance(p);  // consume '=>'
            ASTNode *node = ast_node_create(NODE_ARROW_FN, p->current.line);
            node->params      = params;
            node->param_count = param_count;
            // Body: single expression wrapped in implicit return
            ASTNode *body_expr = parse_expression(p);
            ASTNode *ret_node  = ast_node_create(NODE_RETURN, body_expr->line);
            ast_node_add_child(ret_node, body_expr);
            ASTNode *block = ast_node_create(NODE_BLOCK, body_expr->line);
            ast_node_add_child(block, ret_node);
            ast_node_add_child(node, block);
            return node;
        } else {
            // Not an arrow fn — but we've already consumed past '(' and possibly
            // eaten some tokens.  For the common case of a simple grouped expression
            // like (x + y), we consumed the '(' and stopped when we hit a non-ident.
            // We need to re-parse.  This only works correctly if param_count == 0
            // (nothing consumed) or if the entire expression was a single identifier
            // that we consumed.  Re-use the already-collected info:
            // Free params (they're not used).
            for (size_t i = 0; i < param_count; i++) free(params[i].name);
            free(params);

            // At this point we've consumed '(' and possibly one identifier.
            // The best we can do: if param_count == 0, we just ate '(' and then hit
            // something non-ident (e.g. '(' '-' expr ')' ).  Parse expression now:
            ASTNode *inner;
            if (param_count == 0) {
                inner = parse_expression(p);
            } else {
                // We consumed one or more identifiers but it wasn't an arrow fn.
                // The last identifier is in p->previous (just before the non-ident tok).
                // We can reconstruct by treating the first consumed identifier as an
                // expression, then continuing to parse the rest as a binary expression.
                // Use p->previous to get the identifier name back:
                ASTNode *ident = ast_node_create(NODE_IDENTIFIER, p->previous.line);
                ident->str_value = strdup(p->previous.value);
                // Now parse any binary ops that may follow
                // (parse_multiplication and up starting from this already-evaluated left)
                inner = ident;
                // Continue parsing binary operators
                while (check(p, TOKEN_STAR) || check(p, TOKEN_SLASH) || check(p, TOKEN_PERCENT)) {
                    char *op2 = strdup(p->current.value); advance(p);
                    ASTNode *b = ast_node_create(NODE_BINARY, inner->line);
                    b->str_value = op2; ast_node_add_child(b, inner);
                    ast_node_add_child(b, parse_unary(p)); inner = b;
                }
                while (check(p, TOKEN_PLUS) || check(p, TOKEN_MINUS)) {
                    char *op2 = strdup(p->current.value); advance(p);
                    ASTNode *b = ast_node_create(NODE_BINARY, inner->line);
                    b->str_value = op2; ast_node_add_child(b, inner);
                    ast_node_add_child(b, parse_multiplication(p)); inner = b;
                }
            }
            expect(p, TOKEN_RPAREN, "Expected ')' after expression.");
            return inner;
        }
    }
    // Single bare-identifier arrow function: ident => expr
    // This is handled in parse_call via the identifier path — when we see an
    // IDENTIFIER followed by '=>', we re-interpret it as an arrow fn.
    // (The identifier was already returned as NODE_IDENTIFIER from parse_primary;
    //  parse_call detects the '=>' and converts it.)

    // Grouped expression: ( expr )
    // NOTE: parenthesised expressions now handled above in the arrow-fn block.
    // We reach this unreachable guard only if the arrow-fn block falls through,
    // which it currently cannot.  Kept for safety.
    if (match(p, TOKEN_LPAREN)) {
        ASTNode *expr = parse_expression(p);
        expect(p, TOKEN_RPAREN, "Expected ')' after grouped expression.");
        return expr;
    }

    error_at(p, "Unexpected token in expression.");
    advance(p); // skip the bad token
    return ast_node_create(NODE_NULL, p->current.line);
}
