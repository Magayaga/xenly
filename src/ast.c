/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ─── Create ──────────────────────────────────────────────────────────────────
ASTNode *ast_node_create(NodeType type, int line) {
    ASTNode *n       = (ASTNode *)calloc(1, sizeof(ASTNode));
    n->type          = type;
    n->line          = line;
    n->child_capacity = 4;
    n->children      = (ASTNode **)malloc(sizeof(ASTNode *) * n->child_capacity);
    return n;
}

// ─── Add Child ───────────────────────────────────────────────────────────────
void ast_node_add_child(ASTNode *parent, ASTNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = (ASTNode **)realloc(
            parent->children,
            sizeof(ASTNode *) * parent->child_capacity
        );
    }
    parent->children[parent->child_count++] = child;
}

// ─── Destroy (recursive) ─────────────────────────────────────────────────────
void ast_node_destroy(ASTNode *node) {
    if (!node) return;
    // Recursively destroy children
    for (size_t i = 0; i < node->child_count; i++)
        ast_node_destroy(node->children[i]);
    free(node->children);
    // Free params
    if (node->params) {
        for (size_t i = 0; i < node->param_count; i++)
            free(node->params[i].name);
        free(node->params);
    }
    free(node->str_value);
    free(node);
}

// ─── Debug Print ─────────────────────────────────────────────────────────────
static const char *node_type_name(NodeType t) {
    switch(t) {
        case NODE_PROGRAM:        return "PROGRAM";
        case NODE_VAR_DECL:       return "VAR_DECL";
        case NODE_ASSIGN:         return "ASSIGN";
        case NODE_COMPOUND_ASSIGN:return "COMPOUND_ASSIGN";
        case NODE_INCREMENT:      return "INCREMENT";
        case NODE_DECREMENT:      return "DECREMENT";
        case NODE_FN_DECL:        return "FN_DECL";
        case NODE_FN_CALL:        return "FN_CALL";
        case NODE_RETURN:         return "RETURN";
        case NODE_IF:             return "IF";
        case NODE_WHILE:          return "WHILE";
        case NODE_PRINT:          return "PRINT";
        case NODE_INPUT:          return "INPUT";
        case NODE_IMPORT:         return "IMPORT";
        case NODE_BLOCK:          return "BLOCK";
        case NODE_BINARY:         return "BINARY";
        case NODE_UNARY:          return "UNARY";
        case NODE_NUMBER:         return "NUMBER";
        case NODE_STRING:         return "STRING";
        case NODE_BOOL:           return "BOOL";
        case NODE_NULL:           return "NULL";
        case NODE_IDENTIFIER:     return "IDENT";
        case NODE_METHOD_CALL:    return "METHOD_CALL";
        case NODE_EXPR_STMT:      return "EXPR_STMT";
        case NODE_CLASS_DECL:     return "CLASS_DECL";
        case NODE_NEW:            return "NEW";
        case NODE_THIS:           return "THIS";
        case NODE_SUPER_CALL:     return "SUPER_CALL";
        case NODE_PROPERTY_GET:   return "PROP_GET";
        case NODE_PROPERTY_SET:   return "PROP_SET";
        case NODE_TYPEOF:         return "TYPEOF";
        case NODE_INSTANCEOF:     return "INSTANCEOF";
        case NODE_CALL_EXPR:      return "CALL_EXPR";
        default:                  return "???";
    }
}

void ast_print(ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");

    printf("[%s]", node_type_name(node->type));

    if (node->str_value)  printf(" str=\"%s\"", node->str_value);
    if (node->type == NODE_NUMBER) printf(" num=%.6g", node->num_value);
    if (node->type == NODE_BOOL)   printf(" bool=%s", node->bool_value ? "true" : "false");
    if (node->type == NODE_FN_DECL && node->params) {
        printf(" params=(");
        for (size_t i = 0; i < node->param_count; i++) {
            if (i) printf(", ");
            printf("%s", node->params[i].name);
        }
        printf(")");
    }
    printf("\n");

    for (size_t i = 0; i < node->child_count; i++)
        ast_print(node->children[i], indent + 1);
}
