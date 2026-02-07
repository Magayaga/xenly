/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and macOS operating systems.
 *
 */
#include "typecheck.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Type environment: maps variable names to their inferred/annotated types
typedef struct TypeEnv {
    char **names;
    char **types;
    size_t count;
    size_t capacity;
    struct TypeEnv *parent;
} TypeEnv;

static TypeEnv *type_env_create(TypeEnv *parent) {
    TypeEnv *env = (TypeEnv *)calloc(1, sizeof(TypeEnv));
    env->capacity = 8;
    env->names = (char **)malloc(sizeof(char *) * env->capacity);
    env->types = (char **)malloc(sizeof(char *) * env->capacity);
    env->parent = parent;
    return env;
}

static void type_env_destroy(TypeEnv *env) {
    if (!env) return;
    for (size_t i = 0; i < env->count; i++) {
        free(env->names[i]);
        free(env->types[i]);
    }
    free(env->names);
    free(env->types);
    free(env);
}

static void type_env_set(TypeEnv *env, const char *name, const char *type) {
    // Check if exists - update
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0) {
            free(env->types[i]);
            env->types[i] = type ? strdup(type) : strdup("any");
            return;
        }
    }
    // Add new
    if (env->count >= env->capacity) {
        env->capacity *= 2;
        env->names = (char **)realloc(env->names, sizeof(char *) * env->capacity);
        env->types = (char **)realloc(env->types, sizeof(char *) * env->capacity);
    }
    env->names[env->count] = strdup(name);
    env->types[env->count] = type ? strdup(type) : strdup("any");
    env->count++;
}

static char *type_env_get(TypeEnv *env, const char *name) {
    for (size_t i = 0; i < env->count; i++) {
        if (strcmp(env->names[i], name) == 0)
            return env->types[i];
    }
    if (env->parent)
        return type_env_get(env->parent, name);
    return NULL;
}

static char *infer_type(ASTNode *node, TypeEnv *env, TypeCheckMode mode, int *errors);

// Check type compatibility
static int types_compatible(const char *expected, const char *actual) {
    if (!expected || !actual) return 1;  // NULL = unknown/any
    if (strcmp(expected, "any") == 0 || strcmp(actual, "any") == 0) return 1;
    return strcmp(expected, actual) == 0;
}

// Infer type from AST node
static char *infer_type(ASTNode *node, TypeEnv *env, TypeCheckMode mode, int *errors) {
    if (!node) return strdup("any");
    
    switch (node->type) {
        case NODE_NUMBER:
            return strdup("number");
        case NODE_STRING:
            return strdup("string");
        case NODE_BOOL:
            return strdup("bool");
        case NODE_NULL:
            return strdup("null");
            
        case NODE_IDENTIFIER: {
            char *type = type_env_get(env, node->str_value);
            return type ? strdup(type) : strdup("any");
        }
        
        case NODE_VAR_DECL: {
            char *declared_type = node->type_annotation ? node->type_annotation : "any";
            char *inferred_type = NULL;
            
            if (node->child_count > 0) {
                inferred_type = infer_type(node->children[0], env, mode, errors);
                
                if (!types_compatible(declared_type, inferred_type)) {
                    if (mode == TYPECHECK_ERROR) {
                        fprintf(stderr, "\033[1;31m[Type Error] Line %d: Variable '%s' declared as '%s' but initialized with '%s'\033[0m\n",
                                node->line, node->str_value, declared_type, inferred_type);
                        (*errors)++;
                    } else if (mode == TYPECHECK_WARN) {
                        fprintf(stderr, "\033[1;33m[Type Warning] Line %d: Variable '%s' declared as '%s' but initialized with '%s'\033[0m\n",
                                node->line, node->str_value, declared_type, inferred_type);
                    }
                }
            } else {
                inferred_type = strdup(declared_type);
            }
            
            type_env_set(env, node->str_value, inferred_type);
            char *result = strdup(inferred_type);
            free(inferred_type);
            return result;
        }
        
        case NODE_BINARY: {
            char *left_type = infer_type(node->children[0], env, mode, errors);
            char *right_type = infer_type(node->children[1], env, mode, errors);
            char *result = NULL;
            
            if (strcmp(node->str_value, "+") == 0 || strcmp(node->str_value, "-") == 0 ||
                strcmp(node->str_value, "*") == 0 || strcmp(node->str_value, "/") == 0) {
                if (!types_compatible("number", left_type) || !types_compatible("number", right_type)) {
                    if (mode == TYPECHECK_WARN || mode == TYPECHECK_ERROR) {
                        fprintf(stderr, "\033[1;33m[Type Warning] Line %d: Operator '%s' expects numbers, got '%s' and '%s'\033[0m\n",
                                node->line, node->str_value, left_type, right_type);
                        if (mode == TYPECHECK_ERROR) (*errors)++;
                    }
                }
                result = strdup("number");
            }
            else if (strcmp(node->str_value, "<") == 0 || strcmp(node->str_value, ">") == 0 ||
                     strcmp(node->str_value, "<=") == 0 || strcmp(node->str_value, ">=") == 0 ||
                     strcmp(node->str_value, "==") == 0 || strcmp(node->str_value, "!=") == 0 ||
                     strcmp(node->str_value, "and") == 0 || strcmp(node->str_value, "or") == 0) {
                result = strdup("bool");
            }
            else {
                result = strdup("any");
            }
            
            free(left_type);
            free(right_type);
            return result;
        }
        
        case NODE_FN_DECL: {
            TypeEnv *fn_env = type_env_create(env);
            
            for (size_t i = 0; i < node->param_count; i++) {
                char *param_type = node->params[i].type_annotation ? node->params[i].type_annotation : "any";
                type_env_set(fn_env, node->params[i].name, param_type);
            }
            
            if (node->child_count > 0) {
                char *body_type = infer_type(node->children[0], fn_env, mode, errors);
                free(body_type);
            }
            
            type_env_destroy(fn_env);
            return strdup("function");
        }
        
        case NODE_BLOCK: {
            char *last_type = strdup("null");
            for (size_t i = 0; i < node->child_count; i++) {
                free(last_type);
                last_type = infer_type(node->children[i], env, mode, errors);
            }
            return last_type;
        }
        
        default:
            return strdup("any");
    }
}

int typecheck_program(ASTNode *program, TypeCheckMode mode) {
    if (mode == TYPECHECK_OFF) return 0;
    
    TypeEnv *env = type_env_create(NULL);
    int errors = 0;
    
    if (program->type == NODE_BLOCK) {
        for (size_t i = 0; i < program->child_count; i++) {
            char *type = infer_type(program->children[i], env, mode, &errors);
            free(type);
        }
    }
    
    type_env_destroy(env);
    return errors;
}
