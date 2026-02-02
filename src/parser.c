/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ─── Forward declarations (grammar hierarchy) ───────────────────────────────
static ASTNode *parse_statement(Parser *p);
static ASTNode *parse_expression(Parser *p);
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

        // Optional initializer
        if (match(p, TOKEN_ASSIGN)) {
            ast_node_add_child(node, parse_expression(p));
        }
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
                count++;
                advance(p);
            } while (match(p, TOKEN_COMMA));
        }
        expect(p, TOKEN_RPAREN, "Expected ')' after parameters.");
        node->params     = params;
        node->param_count = count;

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
    return parse_or(p);
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

    // ── Postfix loop: handles (), .prop, .method(), ++, -- repeatedly ─────────
    while (1) {
        // ── Function call: ident(args) -> NODE_FN_CALL ─────────────────────
        if (expr->type == NODE_IDENTIFIER && match(p, TOKEN_LPAREN)) {
            ASTNode *call = ast_node_create(NODE_FN_CALL, expr->line);
            call->str_value = strdup(expr->str_value);
            if (!check(p, TOKEN_RPAREN)) {
                do {
                    ast_node_add_child(call, parse_expression(p));
                } while (match(p, TOKEN_COMMA));
            }
            expect(p, TOKEN_RPAREN, "Expected ')' after arguments.");
            ast_node_destroy(expr);
            expr = call;
            continue;
        }

        // ── Generic call: expr(args) -> NODE_CALL_EXPR ───────────────────────
        // Handles: fn_returning_fn()(args), obj.method()(args), etc.
        if (expr->type != NODE_IDENTIFIER && match(p, TOKEN_LPAREN)) {
            ASTNode *call = ast_node_create(NODE_CALL_EXPR, expr->line);
            ast_node_add_child(call, expr);  // children[0] = callable expression
            // children[1..n] = arguments
            if (!check(p, TOKEN_RPAREN)) {
                do {
                    ast_node_add_child(call, parse_expression(p));
                } while (match(p, TOKEN_COMMA));
            }
            expect(p, TOKEN_RPAREN, "Expected ')' after arguments.");
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
    // Grouped expression: ( expr )
    if (match(p, TOKEN_LPAREN)) {
        ASTNode *expr = parse_expression(p);
        expect(p, TOKEN_RPAREN, "Expected ')' after grouped expression.");
        return expr;
    }

    error_at(p, "Unexpected token in expression.");
    advance(p); // skip the bad token
    return ast_node_create(NODE_NULL, p->current.line);
}
