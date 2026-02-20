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

// Check if a type satisfies a constraint
int satisfies_constraint(const char *type, const char *constraint) {
    if (!constraint || strcmp(constraint, "any") == 0) return 1;
    
    // Comparable: number, string, bool
    if (strcmp(constraint, "Comparable") == 0) {
        return strcmp(type, "number") == 0 || 
               strcmp(type, "string") == 0 || 
               strcmp(type, "bool") == 0;
    }
    
    // Numeric: number only
    if (strcmp(constraint, "Numeric") == 0) {
        return strcmp(type, "number") == 0;
    }
    
    // Equatable: all types
    if (strcmp(constraint, "Equatable") == 0) {
        return 1;  // All types can be compared for equality
    }
    
    // Hashable: number, string, bool
    if (strcmp(constraint, "Hashable") == 0) {
        return strcmp(type, "number") == 0 || 
               strcmp(type, "string") == 0 || 
               strcmp(type, "bool") == 0;
    }
    
    return 0;  // Unknown constraint
}

// Infer generic type substitutions from function call arguments
void infer_generic_types(ASTNode *fn_decl, ASTNode **args, size_t argc,
                        char ***type_map_names, char ***type_map_types, size_t *map_size) {
    (void)args;  // Currently unused - full type inference not yet implemented
    
    if (!fn_decl || !fn_decl->type_params) {
        *type_map_names = NULL;
        *type_map_types = NULL;
        *map_size = 0;
        return;
    }
    
    // Create mapping from type parameter names to actual types
    size_t num_type_params = fn_decl->type_param_count;
    *type_map_names = (char **)malloc(sizeof(char *) * num_type_params);
    *type_map_types = (char **)malloc(sizeof(char *) * num_type_params);
    *map_size = num_type_params;
    
    // Initialize with type parameter names
    for (size_t i = 0; i < num_type_params; i++) {
        (*type_map_names)[i] = strdup(fn_decl->type_params[i].name);
        (*type_map_types)[i] = strdup("any");  // default to any
    }
    
    // Infer from arguments (simple version - just use argument types)
    for (size_t i = 0; i < argc && i < fn_decl->param_count; i++) {
        char *param_annotation = fn_decl->params[i].type_annotation;
        if (!param_annotation) continue;
        
        // Check if parameter type is a type parameter (e.g., "T")
        for (size_t j = 0; j < num_type_params; j++) {
            if (strcmp(param_annotation, (*type_map_names)[j]) == 0) {
                // This parameter has type T, so infer T from argument
                // For now, just mark as "inferred" - full implementation would
                // recursively infer from argument structure
                free((*type_map_types)[j]);
                (*type_map_types)[j] = strdup("inferred");
                break;
            }
        }
    }
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
            
            // Check type parameter constraints
            if (node->type_params) {
                for (size_t i = 0; i < node->type_param_count; i++) {
                    TypeParam *tp = &node->type_params[i];
                    if (tp->constraint) {
                        // For now, just validate that the constraint name is known
                        if (strcmp(tp->constraint, "Comparable") != 0 &&
                            strcmp(tp->constraint, "Numeric") != 0 &&
                            strcmp(tp->constraint, "Equatable") != 0 &&
                            strcmp(tp->constraint, "Hashable") != 0) {
                            if (mode == TYPECHECK_WARN || mode == TYPECHECK_ERROR) {
                                fprintf(stderr, "\033[1;33m[Type Warning] Line %d: Unknown constraint '%s' on type parameter '%s'\033[0m\n",
                                        node->line, tp->constraint, tp->name);
                            }
                        }
                    }
                }
            }
            
            // Add parameters to function environment
            for (size_t i = 0; i < node->param_count; i++) {
                char *param_type = node->params[i].type_annotation ? node->params[i].type_annotation : "any";
                type_env_set(fn_env, node->params[i].name, param_type);
            }
            
            // Check function body
            if (node->child_count > 0) {
                char *body_type = infer_type(node->children[0], fn_env, mode, errors);
                
                // Check return type if specified
                if (node->return_type) {
                    if (!types_compatible(node->return_type, body_type)) {
                        if (mode == TYPECHECK_WARN || mode == TYPECHECK_ERROR) {
                            fprintf(stderr, "\033[1;33m[Type Warning] Line %d: Function '%s' declared to return '%s' but body returns '%s'\033[0m\n",
                                    node->line, node->str_value, node->return_type, body_type);
                            if (mode == TYPECHECK_ERROR) (*errors)++;
                        }
                    }
                }
                
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
        
        case NODE_ARRAY_LITERAL: {
            // Infer array element type from first element
            if (node->child_count > 0) {
                char *elem_type = infer_type(node->children[0], env, mode, errors);
                
                // Check all elements have compatible types
                for (size_t i = 1; i < node->child_count; i++) {
                    char *other_type = infer_type(node->children[i], env, mode, errors);
                    if (!types_compatible(elem_type, other_type)) {
                        if (mode == TYPECHECK_WARN || mode == TYPECHECK_ERROR) {
                            fprintf(stderr, "\033[1;33m[Type Warning] Line %d: Array has mixed types: '%s' and '%s'\033[0m\n",
                                    node->line, elem_type, other_type);
                        }
                    }
                    free(other_type);
                }
                
                char *array_type = (char *)malloc(strlen(elem_type) + 3);
                sprintf(array_type, "[%s]", elem_type);
                free(elem_type);
                return array_type;
            }
            return strdup("[any]");
        }
        
        case NODE_IF: {
            // Check condition is boolean-compatible
            if (node->child_count > 0) {
                char *cond_type = infer_type(node->children[0], env, mode, errors);
                if (strcmp(cond_type, "bool") != 0 && strcmp(cond_type, "any") != 0) {
                    if (mode == TYPECHECK_WARN || mode == TYPECHECK_ERROR) {
                        fprintf(stderr, "\033[1;33m[Type Warning] Line %d: If condition expects bool, got '%s'\033[0m\n",
                                node->line, cond_type);
                    }
                }
                free(cond_type);
            }
            
            // Check branches - both should return compatible types
            char *then_type = strdup("null");
            char *else_type = strdup("null");
            
            if (node->child_count > 1) {
                then_type = infer_type(node->children[1], env, mode, errors);
            }
            if (node->child_count > 2) {
                else_type = infer_type(node->children[2], env, mode, errors);
            }
            
            if (!types_compatible(then_type, else_type)) {
                if (mode == TYPECHECK_WARN) {
                    fprintf(stderr, "\033[1;33m[Type Warning] Line %d: If branches have incompatible types: '%s' and '%s'\033[0m\n",
                            node->line, then_type, else_type);
                }
            }
            
            // Return the then branch type (arbitrarily)
            free(else_type);
            return then_type;
        }
        
        case NODE_RETURN: {
            if (node->child_count > 0) {
                return infer_type(node->children[0], env, mode, errors);
            }
            return strdup("null");
        }
        
        case NODE_FN_CALL: {
            // TODO: Check argument types match parameter types
            // For now, assume functions return 'any'
            return strdup("any");
        }
        
        case NODE_CLASS_DECL: {
            // Check type parameters if present
            if (node->type_params) {
                for (size_t i = 0; i < node->type_param_count; i++) {
                    TypeParam *tp = &node->type_params[i];
                    if (tp->constraint) {
                        if (strcmp(tp->constraint, "Comparable") != 0 &&
                            strcmp(tp->constraint, "Numeric") != 0 &&
                            strcmp(tp->constraint, "Equatable") != 0 &&
                            strcmp(tp->constraint, "Hashable") != 0) {
                            if (mode == TYPECHECK_WARN || mode == TYPECHECK_ERROR) {
                                fprintf(stderr, "\033[1;33m[Type Warning] Line %d: Unknown constraint '%s' on class type parameter '%s'\033[0m\n",
                                        node->line, tp->constraint, tp->name);
                            }
                        }
                    }
                }
            }
            
            // Type-check methods
            for (size_t i = 1; i < node->child_count; i++) {  // Skip parent (child 0)
                if (node->children[i]->type == NODE_FN_DECL) {
                    char *method_type = infer_type(node->children[i], env, mode, errors);
                    free(method_type);
                }
            }
            return strdup("class");
        }
        
        default:
            return strdup("any");
    }
}

int typecheck_program(ASTNode *program, TypeCheckMode mode) {
    if (mode == TYPECHECK_OFF) return 0;
    
    TypeEnv *env = type_env_create(NULL);
    int errors = 0;
    
    if (program->type == NODE_PROGRAM || program->type == NODE_BLOCK) {
        for (size_t i = 0; i < program->child_count; i++) {
            char *type = infer_type(program->children[i], env, mode, &errors);
            free(type);
        }
    }
    
    type_env_destroy(env);
    return errors;
}
