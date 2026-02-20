#include "interpreter.h"
#include "modules.h"
#include "multiproc.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// File-scope pointer to the active interpreter — set during interpreter_run.
// Used by value_array to register arrays for shutdown cleanup.
static Interpreter *g_interp = NULL;

// ─── Value Constructors ──────────────────────────────────────────────────────
Value *value_number(double n) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = VAL_NUMBER; v->num = n;
    return v;
}

Value *value_string(const char *s) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = VAL_STRING; v->str = s ? strdup(s) : strdup("");
    return v;
}

Value *value_bool(int b) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = VAL_BOOL; v->boolean = b;
    return v;
}

Value *value_null(void) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = VAL_NULL;
    return v;
}

Value *value_break(void) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = VAL_BREAK;
    return v;
}

Value *value_continue(void) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = VAL_CONTINUE;
    return v;
}

Value *value_array(Value **items, size_t len) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type      = VAL_ARRAY;
    v->array     = items;         // takes ownership
    v->array_len = len;
    v->array_cap = items ? (len ? len : 4) : 0;
    // Register in global array list for shutdown cleanup
    if (g_interp) {
        if (g_interp->all_arrays_count >= g_interp->all_arrays_cap) {
            g_interp->all_arrays_cap = g_interp->all_arrays_cap ? g_interp->all_arrays_cap * 2 : 64;
            g_interp->all_arrays = (Value **)realloc(g_interp->all_arrays,
                sizeof(Value *) * g_interp->all_arrays_cap);
        }
        g_interp->all_arrays[g_interp->all_arrays_count++] = v;
    }
    return v;
}

Value *value_variant(const char *tag, Value **fields, size_t field_count) {
    Value *v = (Value *)calloc(1, sizeof(Value));
    v->type = VAL_ENUM_VARIANT;
    v->variant.tag = strdup(tag);
    v->variant.fields = fields;  // takes ownership
    v->variant.field_count = field_count;
    return v;
}

void value_destroy(Value *v) {
    if (!v) return;
    // Shared reference types are NOT freed here — their lifetime is managed by
    // the environment that owns them (global scope, class method table, etc.).
    // They are only freed during interpreter shutdown via interpreter_destroy.
    if (v->type == VAL_FUNCTION || v->type == VAL_CLASS || v->type == VAL_INSTANCE || 
        v->type == VAL_ARRAY || v->type == VAL_ENUM_VARIANT)
        return;
    if (v->str)   free(v->str);
    if (v->inner) value_destroy(v->inner);
    free(v);
}

// Truthiness: 0, null, false, "" → falsy; everything else → truthy
static int is_truthy(Value *v) {
    if (!v) return 0;
    switch (v->type) {
        case VAL_NULL:   return 0;
        case VAL_BOOL:   return v->boolean;
        case VAL_NUMBER: return v->num != 0.0;
        case VAL_STRING: return strlen(v->str) > 0;
        default:         return 1;
    }
}

char *value_to_string(Value *v) {
    if (!v) return strdup("null");
    char buf[256];
    switch (v->type) {
        case VAL_NUMBER: {
            // Print as integer if it has no fractional part
            if (v->num == (double)(long long)v->num && fabs(v->num) < 1e15)
                snprintf(buf, sizeof(buf), "%lld", (long long)v->num);
            else
                snprintf(buf, sizeof(buf), "%g", v->num);
            return strdup(buf);
        }
        case VAL_STRING:   return strdup(v->str);
        case VAL_BOOL:     return strdup(v->boolean ? "true" : "false");
        case VAL_NULL:     return strdup("null");
        case VAL_FUNCTION: {
            snprintf(buf, sizeof(buf), "<fn %s>", v->fn ? v->fn->name : "anonymous");
            return strdup(buf);
        }
        case VAL_CLASS: {
            snprintf(buf, sizeof(buf), "<class %s>", v->class_def ? v->class_def->name : "?");
            return strdup(buf);
        }
        case VAL_INSTANCE: {
            snprintf(buf, sizeof(buf), "<%s instance>",
                     v->instance && v->instance->class_def ? v->instance->class_def->name : "?");
            return strdup(buf);
        }
        case VAL_ARRAY: {
            // Build "[elem, elem, ...]"
            size_t cap = 64, pos = 0;
            char *out = (char *)malloc(cap);
            out[pos++] = '[';
            for (size_t i = 0; i < v->array_len; i++) {
                if (i > 0) {
                    if (pos + 2 >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
                    out[pos++] = ','; out[pos++] = ' ';
                }
                char *elem = value_to_string(v->array[i]);
                size_t elen = strlen(elem);
                // Wrap strings in quotes
                int is_str = (v->array[i] && v->array[i]->type == VAL_STRING);
                size_t need = elen + (is_str ? 2 : 0) + 1;
                while (pos + need >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
                if (is_str) out[pos++] = '"';
                memcpy(out + pos, elem, elen); pos += elen;
                if (is_str) out[pos++] = '"';
                free(elem);
            }
            if (pos + 2 >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
            out[pos++] = ']';
            out[pos]   = '\0';
            return out;
        }
        case VAL_ENUM_VARIANT: {
            // Build "VariantName" or "VariantName(field1, field2, ...)"
            size_t cap = 256, pos = 0;
            char *out = (char *)malloc(cap);
            size_t tag_len = strlen(v->variant.tag);
            if (pos + tag_len >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
            memcpy(out + pos, v->variant.tag, tag_len);
            pos += tag_len;
            
            if (v->variant.field_count > 0) {
                if (pos + 1 >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
                out[pos++] = '(';
                for (size_t i = 0; i < v->variant.field_count; i++) {
                    if (i > 0) {
                        if (pos + 2 >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
                        out[pos++] = ','; out[pos++] = ' ';
                    }
                    char *field_str = value_to_string(v->variant.fields[i]);
                    size_t flen = strlen(field_str);
                    while (pos + flen >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
                    memcpy(out + pos, field_str, flen);
                    pos += flen;
                    free(field_str);
                }
                if (pos + 1 >= cap) { cap *= 2; out = (char *)realloc(out, cap); }
                out[pos++] = ')';
            }
            out[pos] = '\0';
            return out;
        }
        default: return strdup("<unknown>");
    }
}

// ─── Environment ─────────────────────────────────────────────────────────────
Environment *env_create(Environment *parent) {
    Environment *e = (Environment *)calloc(1, sizeof(Environment));
    e->parent = parent;
    e->refcount = 1;  // starts with refcount 1
    return e;
}

void env_retain(Environment *env) {
    if (env) env->refcount++;
}

void env_destroy(Environment *env) {
    if (!env) return;
    env->refcount--;
    if (env->refcount > 0) return;  // still referenced elsewhere
    
    EnvEntry *cur = env->entries;
    while (cur) {
        EnvEntry *next = cur->next;
        free(cur->name);
        if (cur->value) {
            // VAL_FUNCTION: shared reference, never freed here (owned by declaring env)
            // VAL_BUILTIN_FN: statically allocated, never freed
            // VAL_INSTANCE: shared reference, freed by shutdown_destroy
            // VAL_CLASS: shared references (from env_get) are NOT freed here.
            //   Local wrappers (e.g. __super__, marked with local=1) ARE freed —
            //   their ClassDef is owned by the original class in global.
            if (cur->value->type == VAL_CLASS && cur->value->local) {
                free(cur->value);   // free local wrapper only; ClassDef owned by global
            } else if (cur->value->type != VAL_FUNCTION &&
                       cur->value->type != VAL_BUILTIN_FN &&
                       cur->value->type != VAL_CLASS &&
                       cur->value->type != VAL_INSTANCE) {
                value_destroy(cur->value);
            }
        }
        free(cur);
        cur = next;
    }
    free(env);
}

void env_set(Environment *env, const char *name, Value *val) {
    // Check if already exists in THIS scope → update
    for (EnvEntry *e = env->entries; e; e = e->next) {
        if (strcmp(e->name, name) == 0) {
            if (e->is_const) {
                fprintf(stderr, "\033[1;31m[Xenly Error] Cannot reassign const variable '%s'.\033[0m\n", name);
                return;
            }
            value_destroy(e->value);
            e->value = val;
            return;
        }
    }
    // New entry
    EnvEntry *entry = (EnvEntry *)malloc(sizeof(EnvEntry));
    entry->name  = strdup(name);
    entry->value = val;
    entry->is_const = 0;  // mutable by default
    entry->next  = env->entries;
    env->entries = entry;
}

void env_set_const(Environment *env, const char *name, Value *val) {
    // Set immutable binding
    EnvEntry *entry = (EnvEntry *)malloc(sizeof(EnvEntry));
    entry->name  = strdup(name);
    entry->value = val;
    entry->is_const = 1;  // immutable
    entry->next  = env->entries;
    env->entries = entry;
}

Value *env_get(Environment *env, const char *name) {
    for (Environment *e = env; e; e = e->parent) {
        for (EnvEntry *entry = e->entries; entry; entry = entry->next) {
            if (strcmp(entry->name, name) == 0)
                return entry->value;   // NOTE: borrowed reference
        }
    }
    return NULL;
}

// Update an existing variable anywhere in the scope chain
static int env_update(Environment *env, const char *name, Value *val) {
    for (Environment *e = env; e; e = e->parent) {
        for (EnvEntry *entry = e->entries; entry; entry = entry->next) {
            if (strcmp(entry->name, name) == 0) {
                if (entry->is_const) {
                    fprintf(stderr, "\033[1;31m[Xenly Error] Cannot reassign const variable '%s'.\033[0m\n", name);
                    return 0;
                }
                value_destroy(entry->value);
                entry->value = val;
                return 1;
            }
        }
    }
    return 0;  // not found
}

// Deep-destroy: frees shared types too. Used only at interpreter shutdown.
static void value_destroy_deep(Value *v) {
    if (!v) return;
    if (v->str) free(v->str);
    // Only free FnDef if we own it (not a shared reference)
    if (v->fn && !v->fn_shared) {
        // Release the closure environment (decrement refcount)
        if (v->fn->closure)
            env_destroy(v->fn->closure);
        free(v->fn->name);
        // Only free params for variant constructors (body == NULL)
        // Regular functions borrow params from AST
        if (v->fn->body == NULL && v->fn->params) {
            for (size_t i = 0; i < v->fn->param_count; i++) {
                free(v->fn->params[i].name);
                if (v->fn->params[i].type_annotation) {
                    free(v->fn->params[i].type_annotation);
                }
            }
            free(v->fn->params);
        }
        free(v->fn);
    }
    if (v->inner) value_destroy_deep(v->inner);
    if (v->type == VAL_CLASS && v->class_def) {
        free(v->class_def->name);
        // methods env will be destroyed via env_destroy_deep below
        // but we need to do it here since it's part of this class
        if (v->class_def->methods) {
            // destroy method env entries deeply
            EnvEntry *cur = v->class_def->methods->entries;
            while (cur) {
                EnvEntry *next = cur->next;
                free(cur->name);
                value_destroy_deep(cur->value);
                free(cur);
                cur = next;
            }
            free(v->class_def->methods);
        }
        free(v->class_def);
    }
    if (v->type == VAL_INSTANCE && v->instance) {
        // destroy instance fields deeply
        if (v->instance->fields) {
            EnvEntry *cur = v->instance->fields->entries;
            while (cur) {
                EnvEntry *next = cur->next;
                free(cur->name);
                value_destroy_deep(cur->value);
                free(cur);
                cur = next;
            }
            free(v->instance->fields);
        }
        free(v->instance);
    }
    if (v->type == VAL_ENUM_VARIANT) {
        if (v->variant.tag) free(v->variant.tag);
        if (v->variant.fields) {
            for (size_t i = 0; i < v->variant.field_count; i++) {
                value_destroy_deep(v->variant.fields[i]);
            }
            free(v->variant.fields);
        }
    }
    free(v);
}

static void env_destroy_deep(Environment *env) {
    if (!env) return;
    EnvEntry *cur = env->entries;
    while (cur) {
        EnvEntry *next = cur->next;
        free(cur->name);
        value_destroy_deep(cur->value);
        free(cur);
        cur = next;
    }
    free(env);
}
Interpreter *interpreter_create(void) {
    Interpreter *interp = (Interpreter *)calloc(1, sizeof(Interpreter));
    interp->global = env_create(NULL);
    interp->modules      = NULL;
    interp->module_count = 0;
    interp->user_modules      = NULL;
    interp->user_module_count = 0;
    interp->source_dir    = strdup(".");   // default: current working directory
    interp->loading_files = NULL;
    interp->loading_count = 0;
    interp->current_exports = NULL;
    interp->task_queue      = NULL;
    interp->task_queue_tail = NULL;
    
    // Register multiprocessing builtins
    register_multiproc_builtins(interp);
    
    return interp;
}

void interpreter_set_source_dir(Interpreter *interp, const char *dir) {
    if (!interp || !dir) return;
    free(interp->source_dir);
    interp->source_dir = strdup(dir);
}

void register_builtin(Interpreter *interp, const char *name, BuiltinFn fn) {
    Value *builtin = (Value *)calloc(1, sizeof(Value));
    builtin->type = VAL_BUILTIN_FN;
    builtin->builtin_fn = fn;
    env_set(interp->global, name, builtin);
}

void interpreter_destroy(Interpreter *interp) {
    if (!interp) return;
    // Use deep destroy at shutdown — this frees shared types (classes, instances,
    // functions) that value_destroy skips during normal execution.
    env_destroy_deep(interp->global);
    // Destroy loaded native modules
    for (size_t i = 0; i < interp->module_count; i++) {
        free(interp->modules[i].name);
        // functions array is statically allocated per module, don't free individual fns
    }
    free(interp->modules);
    // Destroy loaded user modules
    for (size_t i = 0; i < interp->user_module_count; i++) {
        free(interp->user_modules[i].name);
        free(interp->user_modules[i].filepath);
        // exports env was set as a shared flat env; destroy it deeply
        if (interp->user_modules[i].exports) {
            EnvEntry *cur = interp->user_modules[i].exports->entries;
            while (cur) {
                EnvEntry *next = cur->next;
                free(cur->name);
                // Values are shared with global; don't deep-destroy here
                free(cur);
                cur = next;
            }
            free(interp->user_modules[i].exports);
        }
        // ast is owned by the module; destroy it
        if (interp->user_modules[i].ast)
            ast_node_destroy(interp->user_modules[i].ast);
    }
    free(interp->user_modules);
    // Free the circular-import loading stack (should be empty, but defensive)
    for (size_t i = 0; i < interp->loading_count; i++)
        free(interp->loading_files[i]);
    free(interp->loading_files);
    // Free the array registry (the arrays themselves were freed by env_destroy_deep)
    free(interp->all_arrays);
    free(interp->source_dir);
    free(interp);
}

// ─── Forward: Evaluator ──────────────────────────────────────────────────────
Value *eval(Interpreter *interp, ASTNode *node, Environment *env);

// ─── Import Module ───────────────────────────────────────────────────────────
// ─── Forward declarations: user module helpers ──────────────────────────────
static char *resolve_module_path(Interpreter *interp, const char *modname);
static char *module_name_from_path(const char *path);
static int   load_user_module(Interpreter *interp, const char *filepath);
static int   is_user_module(Interpreter *interp, const char *name);
static Value *lookup_user_module(Interpreter *interp, const char *modname, const char *name);

static void do_import(Interpreter *interp, const char *modname, const char *alias) {
    const char *reg_name = alias ? alias : modname;

    // Check if already loaded under reg_name
    for (size_t i = 0; i < interp->module_count; i++)
        if (strcmp(interp->modules[i].name, reg_name) == 0) return;

    // Look up in the native module registry
    Module mod;
    if (modules_get(modname, &mod)) {
        interp->module_count++;
        interp->modules = (Module *)realloc(interp->modules, sizeof(Module) * interp->module_count);
        interp->modules[interp->module_count - 1] = mod;
        interp->modules[interp->module_count - 1].name = strdup(reg_name);
        env_set(interp->global, reg_name, value_string(reg_name));
    } else {
        // Fallback: try loading as a user .xe module
        char *path = resolve_module_path(interp, modname);
        if (path && load_user_module(interp, path)) {
            if (alias) {
                char *orig_name = module_name_from_path(path);
                for (size_t i = 0; i < interp->user_module_count; i++) {
                    if (strcmp(interp->user_modules[i].name, orig_name) == 0) {
                        interp->user_module_count++;
                        interp->user_modules = (UserModule *)realloc(interp->user_modules,
                            sizeof(UserModule) * interp->user_module_count);
                        UserModule *um = &interp->user_modules[interp->user_module_count - 1];
                        um->name     = strdup(alias);
                        um->filepath = strdup(interp->user_modules[i].filepath);
                        um->exports  = interp->user_modules[i].exports;
                        um->ast      = NULL;
                        env_set(interp->global, alias, value_string(alias));
                        break;
                    }
                }
                free(orig_name);
            }
        } else {
            fprintf(stderr, "\033[1;31m[Xenly Error] Module '%s' not found.\033[0m\n", modname);
            interp->had_error = 1;
        }
        free(path);
    }
}

// ─── Call a Module Function ──────────────────────────────────────────────────
static Value *call_module_fn(Interpreter *interp, const char *modname,
                             const char *fnname, Value **args, size_t argc) {
    for (size_t i = 0; i < interp->module_count; i++) {
        if (strcmp(interp->modules[i].name, modname) != 0) continue;
        Module *m = &interp->modules[i];
        for (size_t j = 0; j < m->fn_count; j++) {
            if (strcmp(m->functions[j].name, fnname) == 0)
                return m->functions[j].fn(args, argc);
        }
        fprintf(stderr, "\033[1;31m[Xenly Error] Function '%s' not found in module '%s'.\033[0m\n",
                fnname, modname);
        interp->had_error = 1;
        return value_null();
    }
    fprintf(stderr, "\033[1;31m[Xenly Error] Module '%s' is not loaded.\033[0m\n", modname);
    interp->had_error = 1;
    return value_null();
}

// ─── User Module: helpers ────────────────────────────────────────────────────

// Read entire file into heap string (same logic as main.c but internal)
static char *read_file_contents(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

// Resolve a module import path: "foo" → "<source_dir>/foo.xe"
// If the name already ends in .xe, use it as-is (still prepend source_dir).
static char *resolve_module_path(Interpreter *interp, const char *modname) {
    size_t dir_len  = strlen(interp->source_dir);
    size_t mod_len  = strlen(modname);
    int    needs_ext = (mod_len < 3 || strcmp(modname + mod_len - 3, ".xe") != 0);
    size_t total    = dir_len + 1 + mod_len + (needs_ext ? 3 : 0) + 1;
    char *path = (char *)malloc(total);
    snprintf(path, total, "%s/%s%s", interp->source_dir, modname,
             needs_ext ? ".xe" : "");
    return path;
}

// Extract module name from a file path: strip directory and .xe extension
static char *module_name_from_path(const char *path) {
    // Find last '/'
    const char *slash = strrchr(path, '/');
    const char *base  = slash ? slash + 1 : path;
    size_t len = strlen(base);
    // Strip .xe if present
    if (len > 3 && strcmp(base + len - 3, ".xe") == 0) len -= 3;
    char *name = (char *)malloc(len + 1);
    strncpy(name, base, len);
    name[len] = '\0';
    return name;
}

// Load and execute a user .xe module file, collecting its exports.
// Returns 1 on success, 0 on failure.
static int load_user_module(Interpreter *interp, const char *filepath) {
    // ── Circular import check ─────────────────────────────────────────────
    for (size_t i = 0; i < interp->loading_count; i++) {
        if (strcmp(interp->loading_files[i], filepath) == 0) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Circular import detected: '%s' is already being loaded.\033[0m\n",
                    filepath);
            interp->had_error = 1;
            return 0;
        }
    }

    // ── Already loaded? (dedup by filepath) ───────────────────────────────
    for (size_t i = 0; i < interp->user_module_count; i++) {
        if (strcmp(interp->user_modules[i].filepath, filepath) == 0)
            return 1;   // already loaded — no-op
    }

    // ── Push onto loading stack ───────────────────────────────────────────
    interp->loading_count++;
    interp->loading_files = (char **)realloc(interp->loading_files,
        sizeof(char *) * interp->loading_count);
    interp->loading_files[interp->loading_count - 1] = strdup(filepath);

    // ── Read file ─────────────────────────────────────────────────────────
    size_t src_len = 0;
    char  *source  = read_file_contents(filepath, &src_len);
    if (!source) {
        fprintf(stderr, "\033[1;31m[Xenly Error] Cannot open module file '%s'.\033[0m\n", filepath);
        interp->had_error = 1;
        // Pop loading stack
        interp->loading_count--;
        free(interp->loading_files[interp->loading_count]);
        return 0;
    }

    // ── Lex + Parse ───────────────────────────────────────────────────────
    Lexer   *lexer  = lexer_create(source, src_len);
    Parser  *parser = parser_create(lexer);
    ASTNode *program = parser_parse(parser);

    if (parser->had_error) {
        fprintf(stderr, "\033[1;31m[Xenly Error] Parse error in module '%s'.\033[0m\n", filepath);
        interp->had_error = 1;
        ast_node_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        interp->loading_count--;
        free(interp->loading_files[interp->loading_count]);
        return 0;
    }

    // ── Save and set source_dir to the module's directory ────────────────
    // This allows nested imports to resolve relative to the module's location.
    char *saved_source_dir = interp->source_dir;
    {
        const char *slash = strrchr(filepath, '/');
        if (slash) {
            size_t dlen = (size_t)(slash - filepath);
            interp->source_dir = (char *)malloc(dlen + 1);
            strncpy(interp->source_dir, filepath, dlen);
            interp->source_dir[dlen] = '\0';
        } else {
            interp->source_dir = strdup(".");
        }
    }

    // ── Set up exports collector ──────────────────────────────────────────
    Environment *saved_exports = interp->current_exports;
    Environment *mod_exports   = env_create(NULL);   // flat, no parent
    interp->current_exports    = mod_exports;

    // ── Evaluate the module in the GLOBAL env ─────────────────────────────
    // Module top-level declarations go into global so they can reference each
    // other and use imports. Only export-marked names are surfaced to the importer.
    Value *mod_result = eval(interp, program, interp->global);
    value_destroy(mod_result);

    // ── Restore interpreter state ─────────────────────────────────────────
    interp->current_exports = saved_exports;
    free(interp->source_dir);
    interp->source_dir = saved_source_dir;

    // ── Pop loading stack ─────────────────────────────────────────────────
    interp->loading_count--;
    free(interp->loading_files[interp->loading_count]);

    // ── Register user module ──────────────────────────────────────────────
    if (!interp->had_error) {
        char *modname = module_name_from_path(filepath);
        interp->user_module_count++;
        interp->user_modules = (UserModule *)realloc(interp->user_modules,
            sizeof(UserModule) * interp->user_module_count);
        UserModule *um = &interp->user_modules[interp->user_module_count - 1];
        um->name     = modname;
        um->filepath = strdup(filepath);
        um->exports  = mod_exports;
        um->ast      = program;     // AST ownership transferred — do NOT destroy here

        // Register module name in global so mod.fn() dispatch works
        env_set(interp->global, modname, value_string(modname));
    } else {
        // On error, discard exports and AST
        EnvEntry *cur = mod_exports->entries;
        while (cur) { EnvEntry *n = cur->next; free(cur->name); free(cur); cur = n; }
        free(mod_exports);
        ast_node_destroy(program);
    }

    // ── Cleanup ───────────────────────────────────────────────────────────
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);

    return interp->had_error ? 0 : 1;
}

// Look up a name in a user module's exports. Returns the Value* or NULL.
static Value *lookup_user_module(Interpreter *interp, const char *modname, const char *name) {
    for (size_t i = 0; i < interp->user_module_count; i++) {
        if (strcmp(interp->user_modules[i].name, modname) == 0) {
            // Search exports (flat env, no parent)
            for (EnvEntry *e = interp->user_modules[i].exports->entries; e; e = e->next) {
                if (strcmp(e->name, name) == 0)
                    return e->value;
            }
            return NULL;   // name not exported
        }
    }
    return NULL;   // module not found
}

// Check if a name is a loaded user module
static int is_user_module(Interpreter *interp, const char *name) {
    for (size_t i = 0; i < interp->user_module_count; i++)
        if (strcmp(interp->user_modules[i].name, name) == 0) return 1;
    return 0;
}
// ─── Main Eval Dispatch ──────────────────────────────────────────────────────
Value *eval(Interpreter *interp, ASTNode *node, Environment *env) {
    if (!node || interp->had_error) return value_null();

    switch (node->type) {

    // ── PROGRAM ────────────────────────────────────────────────────────────
    case NODE_PROGRAM: {
        Value *result = value_null();
        for (size_t i = 0; i < node->child_count; i++) {
            value_destroy(result);
            result = eval(interp, node->children[i], env);
            if (result && result->type == VAL_RETURN) {
                // Top-level return: just unwrap
                Value *inner = result->inner;
                result->inner = NULL;
                value_destroy(result);
                result = inner;
                break;
            }
        }
        return result;
    }

    // ── BLOCK ──────────────────────────────────────────────────────────────
    case NODE_BLOCK: {
        Environment *block_env = env_create(env);
        Value *result = value_null();
        for (size_t i = 0; i < node->child_count; i++) {
            value_destroy(result);
            result = eval(interp, node->children[i], block_env);
            if (result && (result->type == VAL_RETURN || result->type == VAL_BREAK || result->type == VAL_CONTINUE)) {
                env_destroy(block_env);
                return result;  // bubble up
            }
            if (interp->had_error) break;
        }
        env_destroy(block_env);
        return result;
    }

    // ── VAR DECL ───────────────────────────────────────────────────────────
    case NODE_VAR_DECL: {
        Value *val = value_null();
        if (node->child_count > 0) {
            val = eval(interp, node->children[0], env);
        }
        env_set(env, node->str_value, val);
        return value_null();
    }

    // ── CONST DECL ─────────────────────────────────────────────────────────
    case NODE_CONST_DECL: {
        Value *val = eval(interp, node->children[0], env);
        env_set_const(env, node->str_value, val);
        return value_null();
    }

    // ── ENUM DECL ──────────────────────────────────────────────────────────
    case NODE_ENUM_DECL: {
        // Enum creates variant constructor functions in the current scope
        // Each variant becomes a function that creates a VAL_ENUM_VARIANT
        // Variants with no params are created as values directly
        for (size_t i = 0; i < node->child_count; i++) {
            ASTNode *variant = node->children[i];
            const char *variant_name = variant->str_value;
            size_t param_count = variant->param_count;
            
            if (param_count == 0) {
                // Parameterless variant: create the value directly
                Value *variant_val = value_variant(variant_name, NULL, 0);
                env_set(env, variant_name, variant_val);
            } else {
                // Variant with params: create a constructor function
                FnDef *constructor = (FnDef *)malloc(sizeof(FnDef));
                constructor->name = strdup(variant_name);
                // Deep copy params
                constructor->param_count = param_count;
                constructor->params = (Param *)malloc(sizeof(Param) * param_count);
                for (size_t j = 0; j < param_count; j++) {
                    constructor->params[j].name = strdup(variant->params[j].name);
                    constructor->params[j].type_annotation = variant->params[j].type_annotation 
                        ? strdup(variant->params[j].type_annotation) : NULL;
                }
                constructor->body = NULL;  // NULL body means it's a variant constructor
                constructor->closure = env;  // capture environment
                constructor->is_async = 0;
                
                Value *constructor_val = (Value *)calloc(1, sizeof(Value));
                constructor_val->type = VAL_FUNCTION;
                constructor_val->fn = constructor;
                
                env_set(env, variant_name, constructor_val);
            }
        }
        return value_null();
    }

    // ── ASSIGN ─────────────────────────────────────────────────────────────
    case NODE_ASSIGN: {
        Value *val = eval(interp, node->children[0], env);
        if (!env_update(env, node->str_value, val)) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Undefined variable '%s'.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            value_destroy(val);
        }
        return value_null();
    }

    // ── COMPOUND ASSIGN (+=, -=, *=, /=) ───────────────────────────────────
    case NODE_COMPOUND_ASSIGN: {
        // children[0] = IDENT node, children[1] = rhs expr
        const char *name = node->children[0]->str_value;
        Value *cur = env_get(env, name);
        if (!cur) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Undefined variable '%s'.\033[0m\n",
                    node->line, name);
            interp->had_error = 1;
            return value_null();
        }
        Value *rhs = eval(interp, node->children[1], env);
        double result = 0;
        if (strcmp(node->str_value, "+=") == 0) {
            if (cur->type == VAL_STRING && rhs->type == VAL_STRING) {
                // String concatenation via +=
                char *newstr = (char *)malloc(strlen(cur->str) + strlen(rhs->str) + 1);
                strcpy(newstr, cur->str);
                strcat(newstr, rhs->str);
                env_update(env, name, value_string(newstr));
                free(newstr);
                value_destroy(rhs);
                return value_null();
            }
            result = cur->num + rhs->num;
        }
        else if (strcmp(node->str_value, "-=") == 0) result = cur->num - rhs->num;
        else if (strcmp(node->str_value, "*=") == 0) result = cur->num * rhs->num;
        else if (strcmp(node->str_value, "/=") == 0) {
            if (rhs->num == 0) {
                fprintf(stderr, "\033[1;31m[Xenly Error] Division by zero.\033[0m\n");
                interp->had_error = 1;
                value_destroy(rhs);
                return value_null();
            }
            result = cur->num / rhs->num;
        }
        value_destroy(rhs);
        env_update(env, name, value_number(result));
        return value_null();
    }

    // ── INCREMENT / DECREMENT ──────────────────────────────────────────────
    case NODE_INCREMENT: {
        Value *cur = env_get(env, node->str_value);
        if (!cur || cur->type != VAL_NUMBER) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Cannot increment '%s'.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            return value_null();
        }
        env_update(env, node->str_value, value_number(cur->num + 1));
        return value_null();
    }
    case NODE_DECREMENT: {
        Value *cur = env_get(env, node->str_value);
        if (!cur || cur->type != VAL_NUMBER) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Cannot decrement '%s'.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            return value_null();
        }
        env_update(env, node->str_value, value_number(cur->num - 1));
        return value_null();
    }

    // ── FN DECL ────────────────────────────────────────────────────────────
    case NODE_FN_DECL: {
        Value *fnval = (Value *)calloc(1, sizeof(Value));
        fnval->type      = VAL_FUNCTION;
        fnval->fn_shared = 0;  // this Value owns the FnDef
        fnval->fn   = (FnDef *)malloc(sizeof(FnDef));
        fnval->fn->name       = strdup(node->str_value);
        fnval->fn->params     = node->params;
        fnval->fn->param_count = node->param_count;
        fnval->fn->body       = node->children[0]; // the block
        fnval->fn->closure    = env;               // capture current env
        fnval->fn->is_async   = (int)node->num_value;  // async flag from parser
        env_retain(env);                           // increment refcount
        env_set(env, node->str_value, fnval);
        return value_null();
    }

    // ── FN CALL ────────────────────────────────────────────────────────────
    case NODE_FN_CALL: {
        Value *fnval = env_get(env, node->str_value);
        if (!fnval) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Undefined function '%s'.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            return value_null();
        }
        
        // Handle built-in functions
        if (fnval->type == VAL_BUILTIN_FN) {
            // Evaluate all arguments
            Value **args = (Value **)malloc(sizeof(Value *) * node->child_count);
            for (size_t i = 0; i < node->child_count; i++) {
                args[i] = eval(interp, node->children[i], env);
            }
            
            // Call the builtin
            Value *result = fnval->builtin_fn(args, node->child_count);
            free(args);
            return result;
        }
        
        if (fnval->type != VAL_FUNCTION) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: '%s' is not a function.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            return value_null();
        }
        FnDef *fn = fnval->fn;

        // Separate positional and named arguments
        size_t positional_count = 0;
        ASTNode **named_args = NULL;
        size_t named_count = 0;
        
        for (size_t i = 0; i < node->child_count; i++) {
            if (node->children[i]->type == NODE_NAMED_ARG) {
                named_count++;
                named_args = realloc(named_args, sizeof(ASTNode*) * named_count);
                named_args[named_count - 1] = node->children[i];
            } else {
                positional_count++;
            }
        }

        // Evaluate positional arguments
        Value **args = (Value **)malloc(sizeof(Value *) * positional_count);
        size_t arg_idx = 0;
        for (size_t i = 0; i < node->child_count; i++) {
            if (node->children[i]->type != NODE_NAMED_ARG) {
                args[arg_idx++] = eval(interp, node->children[i], env);
            }
        }

        // Execute body (NULL body means it's a variant constructor)
        if (fn->body == NULL) {
            // Variant constructor: create VAL_ENUM_VARIANT
            Value *result = value_variant(fn->name, args, positional_count);
            free(named_args);
            // Don't free args - they're owned by the variant now
            return result;
        }

        // Create new scope from closure
        Environment *fn_env = env_create(fn->closure);
        
        // Bind positional arguments to parameters
        size_t param_idx = 0;
        for (size_t i = 0; i < positional_count && param_idx < fn->param_count; i++) {
            env_set(fn_env, fn->params[param_idx].name, args[i]);
            param_idx++;
        }
        
        // Bind named arguments to parameters
        for (size_t i = 0; i < named_count; i++) {
            ASTNode *named = named_args[i];
            const char *param_name = named->str_value;
            Value *arg_value = eval(interp, named->children[0], env);
            
            // Find parameter by name
            int found = 0;
            for (size_t j = 0; j < fn->param_count; j++) {
                if (strcmp(fn->params[j].name, param_name) == 0) {
                    env_set(fn_env, param_name, arg_value);
                    found = 1;
                    break;
                }
            }
            
            if (!found) {
                fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Unknown parameter '%s' in function '%s'.\033[0m\n",
                        node->line, param_name, fn->name);
                interp->had_error = 1;
                value_destroy(arg_value);
            }
        }
        
        // Fill in defaults for unbound parameters
        for (size_t i = 0; i < fn->param_count; i++) {
            if (env_get(fn_env, fn->params[i].name) == NULL) {
                if (fn->params[i].default_value != NULL) {
                    Value *default_val = eval(interp, fn->params[i].default_value, fn_env);
                    env_set(fn_env, fn->params[i].name, default_val);
                } else if (fn->params[i].is_optional) {
                    env_set(fn_env, fn->params[i].name, value_null());
                } else {
                    fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Missing required parameter '%s' in function '%s'.\033[0m\n",
                            node->line, fn->params[i].name, fn->name);
                    interp->had_error = 1;
                    env_set(fn_env, fn->params[i].name, value_null());
                }
            }
        }

        Value *result = eval(interp, fn->body, fn_env);

        // Unwrap return sentinel
        if (result && result->type == VAL_RETURN) {
            Value *inner = result->inner;
            result->inner = NULL;
            value_destroy(result);
            result = inner;
        }

        // Cleanup
        Value **transient_fns = NULL;
        size_t  transient_count = 0;
        for (size_t i = 0; i < positional_count; i++) {
            if (args[i] && args[i]->type == VAL_FUNCTION) {
                int is_transient = 1;
                size_t pos_idx = 0;
                for (size_t j = 0; j < node->child_count && pos_idx <= i; j++) {
                    if (node->children[j]->type != NODE_NAMED_ARG) {
                        if (pos_idx == i) {
                            if (node->children[j]->type == NODE_IDENTIFIER || 
                                node->children[j]->type == NODE_THIS) {
                                is_transient = 0;
                            }
                            break;
                        }
                        pos_idx++;
                    }
                }
                if (is_transient) {
                    transient_count++;
                    transient_fns = (Value **)realloc(transient_fns,
                        sizeof(Value *) * transient_count);
                    transient_fns[transient_count - 1] = args[i];
                }
            }
        }
        free(args);
        free(named_args);
        env_destroy(fn_env);

        for (size_t i = 0; i < transient_count; i++) {
            Value *tfn = transient_fns[i];
            if (tfn->fn && tfn->fn->closure)
                env_destroy(tfn->fn->closure);
            free(tfn->fn->name);
            free(tfn->fn);
            free(tfn);
        }
        free(transient_fns);

        return result ? result : value_null();
    }

    // ── RETURN ─────────────────────────────────────────────────────────────
    case NODE_RETURN: {
        Value *val = value_null();
        if (node->child_count > 0)
            val = eval(interp, node->children[0], env);
        // Wrap in a RETURN sentinel
        Value *ret = (Value *)calloc(1, sizeof(Value));
        ret->type  = VAL_RETURN;
        ret->inner = val;
        return ret;
    }

    // ── IF ─────────────────────────────────────────────────────────────────
    case NODE_IF: {
        Value *cond = eval(interp, node->children[0], env);
        if (is_truthy(cond)) {
            value_destroy(cond);
            return eval(interp, node->children[1], env);  // then block
        }
        value_destroy(cond);
        if (node->child_count > 2)
            return eval(interp, node->children[2], env);  // else block / else-if
        return value_null();
    }

    // ── WHILE ──────────────────────────────────────────────────────────────
    case NODE_WHILE: {
        Value *result = value_null();
        while (1) {
            Value *cond = eval(interp, node->children[0], env);
            int truthy = is_truthy(cond);
            value_destroy(cond);
            if (!truthy) break;

            value_destroy(result);
            result = eval(interp, node->children[1], env);  // body
            if (result && result->type == VAL_RETURN) return result;  // bubble up
            if (result && result->type == VAL_BREAK) {
                value_destroy(result);
                return value_null();
            }
            if (result && result->type == VAL_CONTINUE) {
                value_destroy(result);
                result = value_null();
                continue;
            }
            if (interp->had_error) break;
        }
        return result;
    }

    // ── FOR LOOP ───────────────────────────────────────────────────────────
    case NODE_FOR: {
        // children[0] = init, children[1] = cond, children[2] = update, children[3] = body
        Value *init_result = eval(interp, node->children[0], env);
        value_destroy(init_result);
        
        Value *result = value_null();
        while (1) {
            Value *cond = eval(interp, node->children[1], env);
            int truthy = is_truthy(cond);
            value_destroy(cond);
            if (!truthy) break;
            
            value_destroy(result);
            result = eval(interp, node->children[3], env);  // body
            if (result && result->type == VAL_RETURN) return result;
            if (result && result->type == VAL_BREAK) {
                value_destroy(result);
                return value_null();
            }
            if (result && result->type == VAL_CONTINUE) {
                value_destroy(result);
                result = value_null();
            }
            if (interp->had_error) break;
            
            // Update
            Value *update_result = eval(interp, node->children[2], env);
            value_destroy(update_result);
        }
        return result;
    }

    // ── DO-WHILE LOOP ──────────────────────────────────────────────────────
    case NODE_DO_WHILE: {
        // children[0] = body, children[1] = condition
        Value *result = value_null();
        do {
            value_destroy(result);
            result = eval(interp, node->children[0], env);  // body
            if (result && result->type == VAL_RETURN) return result;
            if (result && result->type == VAL_BREAK) {
                value_destroy(result);
                return value_null();
            }
            if (result && result->type == VAL_CONTINUE) {
                value_destroy(result);
                result = value_null();
            }
            if (interp->had_error) break;
            
            Value *cond = eval(interp, node->children[1], env);
            int truthy = is_truthy(cond);
            value_destroy(cond);
            if (!truthy) break;
        } while (1);
        return result;
    }

    // ── SWITCH ────────────────────────────────────────────────────────────────
    case NODE_SWITCH: {
        // children[0] = discriminant expression
        // children[1..n] = case blocks (NODE_BLOCK):
        //   Regular case:  children[0] = case-value expr, children[1..] = statements
        //   Default case:  str_value == "__default__", children[0..] = statements
        Value *disc = eval(interp, node->children[0], env);

        ASTNode *default_block = NULL;
        Value *result = value_null();
        int matched = 0;

        for (size_t i = 1; i < node->child_count; i++) {
            ASTNode *case_block = node->children[i];

            // Is this the default clause?
            if (case_block->str_value && strcmp(case_block->str_value, "__default__") == 0) {
                default_block = case_block;
                continue;
            }

            // Evaluate the case value (first child of block) and compare
            ASTNode *case_val_node = case_block->children[0];
            Value *case_val = eval(interp, case_val_node, env);

            int eq = 0;
            if (disc->type == VAL_NUMBER && case_val->type == VAL_NUMBER)
                eq = disc->num == case_val->num;
            else if (disc->type == VAL_STRING && case_val->type == VAL_STRING)
                eq = strcmp(disc->str, case_val->str) == 0;
            else if (disc->type == VAL_BOOL && case_val->type == VAL_BOOL)
                eq = disc->boolean == case_val->boolean;
            else if (disc->type == VAL_NULL && case_val->type == VAL_NULL)
                eq = 1;
            value_destroy(case_val);

            if (eq) {
                matched = 1;
                // Execute statements (children[1..] of this block)
                for (size_t j = 1; j < case_block->child_count; j++) {
                    value_destroy(result);
                    result = eval(interp, case_block->children[j], env);
                    if (result && (result->type == VAL_RETURN ||
                                   result->type == VAL_BREAK  ||
                                   result->type == VAL_CONTINUE)) {
                        // Break exits the switch, return/continue bubble up
                        if (result->type == VAL_BREAK) {
                            value_destroy(result); result = value_null();
                        }
                        value_destroy(disc);
                        return result;
                    }
                    if (interp->had_error) break;
                }
                break;  // no fall-through
            }
        }

        // No case matched — run default if present
        if (!matched && default_block) {
            for (size_t j = 0; j < default_block->child_count; j++) {
                value_destroy(result);
                result = eval(interp, default_block->children[j], env);
                if (result && (result->type == VAL_RETURN  ||
                               result->type == VAL_BREAK   ||
                               result->type == VAL_CONTINUE)) {
                    if (result->type == VAL_BREAK) {
                        value_destroy(result); result = value_null();
                    }
                    value_destroy(disc);
                    return result;
                }
                if (interp->had_error) break;
            }
        }

        value_destroy(disc);
        return result;
    }

    // ── ARROW FUNCTION (NODE_ARROW_FN) ─────────────────────────────────────
    case NODE_ARROW_FN: {
        // Create a VAL_FUNCTION with the arrow fn's params and body,
        // capturing the current env as a closure (same as a regular fn decl).
        Value *fnval = (Value *)calloc(1, sizeof(Value));
        fnval->type      = VAL_FUNCTION;
        fnval->fn_shared = 0;  // this Value owns the FnDef
        fnval->fn   = (FnDef *)malloc(sizeof(FnDef));
        fnval->fn->name        = strdup("<arrow>");
        fnval->fn->params      = node->params;
        fnval->fn->param_count = node->param_count;
        fnval->fn->body        = node->children[0];  // the implicit-return block
        fnval->fn->closure     = env;
        env_retain(env);                             // increment refcount
        fnval->fn->is_async    = 0;
        return fnval;
    }

    // ── CALL_EXPR: call any expression that evaluates to a function ─────────
    case NODE_CALL_EXPR: {
        // children[0] = callee expression, children[1..n] = arguments
        Value *callee = eval(interp, node->children[0], env);
        if (!callee || callee->type != VAL_FUNCTION) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Expression is not callable.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            if (callee) value_destroy(callee);
            return value_null();
        }
        FnDef *fn = callee->fn;

        // Evaluate arguments (children[1..n])
        size_t argc = node->child_count - 1;
        Value **args = (Value **)malloc(sizeof(Value *) * (argc ? argc : 1));
        for (size_t i = 0; i < argc; i++)
            args[i] = eval(interp, node->children[i + 1], env);

        // Variant constructor case
        if (fn->body == NULL) {
            Value *r = value_variant(fn->name, args, argc);
            return r;
        }

        // Normal function call in closure scope
        Environment *fn_env = env_create(fn->closure);
        for (size_t i = 0; i < fn->param_count && i < argc; i++)
            env_set(fn_env, fn->params[i].name, args[i]);
        for (size_t i = argc; i < fn->param_count; i++) {
            if (fn->params[i].default_value)
                env_set(fn_env, fn->params[i].name, eval(interp, fn->params[i].default_value, fn_env));
            else
                env_set(fn_env, fn->params[i].name, value_null());
        }
        free(args);

        Value *result = eval(interp, fn->body, fn_env);
        if (result && result->type == VAL_RETURN) {
            Value *inner = result->inner;
            result->inner = NULL;
            value_destroy(result);
            result = inner;
        }
        env_destroy(fn_env);
        return result ? result : value_null();
    }

    // ── BREAK ──────────────────────────────────────────────────────────────
    case NODE_BREAK: {
        return value_break();
    }

    // ── CONTINUE ───────────────────────────────────────────────────────────
    case NODE_CONTINUE: {
        return value_continue();
    }

    // ── FOR-IN ────────────────────────────────────────────────────────────
    case NODE_FOR_IN: {
        Value *iterable = eval(interp, node->children[0], env);
        if (!iterable || iterable->type != VAL_ARRAY) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: for-in requires an array.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            value_destroy(iterable);
            return value_null();
        }
        Environment *loop_env = env_create(env);
        Value *result = value_null();
        for (size_t i = 0; i < iterable->array_len; i++) {
            Value *elem_copy;
            Value *elem = iterable->array[i];
            if (!elem)                         elem_copy = value_null();
            else if (elem->type == VAL_NUMBER) elem_copy = value_number(elem->num);
            else if (elem->type == VAL_STRING) elem_copy = value_string(elem->str);
            else if (elem->type == VAL_BOOL)   elem_copy = value_bool(elem->boolean);
            else if (elem->type == VAL_NULL)   elem_copy = value_null();
            else                               elem_copy = elem;
            env_set(loop_env, node->str_value, elem_copy);
            value_destroy(result);
            result = eval(interp, node->children[1], loop_env);
            if (result && result->type == VAL_RETURN) { env_destroy(loop_env); value_destroy(iterable); return result; }
            if (result && result->type == VAL_BREAK) { value_destroy(result); result = value_null(); break; }
            if (result && result->type == VAL_CONTINUE) { value_destroy(result); result = value_null(); continue; }
            if (interp->had_error) break;
        }
        env_destroy(loop_env);
        value_destroy(iterable);
        return result;
    }

    case NODE_PRINT: {
        for (size_t i = 0; i < node->child_count; i++) {
            Value *val = eval(interp, node->children[i], env);
            char *s = value_to_string(val);
            if (i > 0) printf(" ");
            printf("%s", s);
            free(s);
            value_destroy(val);
        }
        printf("\n");
        return value_null();
    }

    // ── INPUT ──────────────────────────────────────────────────────────────
    case NODE_INPUT: {
        if (node->child_count > 0) {
            Value *prompt = eval(interp, node->children[0], env);
            char *s = value_to_string(prompt);
            printf("%s", s);
            fflush(stdout);
            free(s);
            value_destroy(prompt);
        }
        char buf[4096];
        if (fgets(buf, sizeof(buf), stdin)) {
            // Strip trailing newline
            size_t len = strlen(buf);
            if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
            return value_string(buf);
        }
        return value_string("");
    }

    // ── IMPORT ─────────────────────────────────────────────────────────────
    case NODE_IMPORT: {
        const char *module_path = node->str_value;
        int import_type = (int)node->num_value;   // 0=regular, 1=aliased, 2=selective, 3=wildcard

        if (import_type == 0) {
            // Regular: import "module"
            do_import(interp, module_path, NULL);
        } else if (import_type == 1) {
            // Aliased: import "module" as alias
            const char *alias = node->children[0]->str_value;
            do_import(interp, module_path, alias);
        } else if (import_type == 2) {
            // Selective: from "module" import name1, name2
            do_import(interp, module_path, NULL);
            const char *module_name = module_name_from_path(module_path);
            for (size_t i = 0; i < node->child_count; i++) {
                const char *symbol_name = node->children[i]->str_value;
                Value *symbol = NULL;
                if (is_user_module(interp, module_name)) {
                    symbol = lookup_user_module(interp, module_name, symbol_name);
                } else {
                    fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Selective import only supported for user modules, not native module '%s'.\033[0m\n",
                            node->line, module_name);
                    interp->had_error = 1;
                    break;
                }
                if (!symbol) {
                    fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: '%s' is not exported from module '%s'.\033[0m\n",
                            node->line, symbol_name, module_name);
                    interp->had_error = 1;
                } else {
                    env_set(env, symbol_name, symbol);
                }
            }
            free((char *)module_name);
        } else if (import_type == 3) {
            // Wildcard: from "module" import *
            do_import(interp, module_path, NULL);
            const char *module_name = module_name_from_path(module_path);
            if (is_user_module(interp, module_name)) {
                // Import all exports into current scope
                for (size_t i = 0; i < interp->user_module_count; i++) {
                    if (strcmp(interp->user_modules[i].name, module_name) == 0) {
                        Environment *exports = interp->user_modules[i].exports;
                        for (EnvEntry *e = exports->entries; e; e = e->next) {
                            env_set(env, e->name, e->value);   // shared reference
                        }
                        break;
                    }
                }
            } else {
                fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Wildcard import only supported for user modules, not native module '%s'.\033[0m\n",
                        node->line, module_name);
                interp->had_error = 1;
            }
            free((char *)module_name);
        }
        return value_null();
    }

    // ── EXPORT ─────────────────────────────────────────────────────────────
    case NODE_EXPORT: {
        // children[0] = the fn or class declaration
        if (node->child_count == 0) return value_null();
        ASTNode *decl = node->children[0];

        // Evaluate the declaration (registers it in env as usual)
        Value *result = eval(interp, decl, env);

        // If we're inside a module load, copy the declared name into exports
        if (interp->current_exports && decl->str_value) {
            Value *exported = env_get(env, decl->str_value);
            if (exported) {
                // Store in exports env (shared reference — same Value*)
                env_set(interp->current_exports, decl->str_value, exported);
            }
        }
        return result;
    }

    // ── METHOD CALL (module.fn  OR  instance.method) ─────────────────────────
    case NODE_METHOD_CALL: {
        // New AST shape: children[0] = object expr, str_value = method name, children[1..n] = args
        Value *obj = eval(interp, node->children[0], env);
        const char *method_name = node->str_value;

        // Evaluate arguments (children[1..n])
        size_t argc = node->child_count - 1;
        Value **args = (Value **)malloc(sizeof(Value *) * (argc ? argc : 1));
        for (size_t i = 0; i < argc; i++)
            args[i] = eval(interp, node->children[i + 1], env);

        Value *result = value_null();
        int args_consumed = 0;   // set when args are handed to an env (instance call or user-module call)

        if (obj->type == VAL_INSTANCE) {
            // ── Instance method call ──────────────────────────────────────────
            InstanceData *inst = obj->instance;
            ClassDef     *cls  = inst->class_def;

            // Walk the class hierarchy to find the method
            Value *method_val = NULL;
            ClassDef *search = cls;
            while (search && !method_val) {
                method_val = env_get(search->methods, method_name);
                search = search->parent;
            }

            if (!method_val || method_val->type != VAL_FUNCTION) {
                fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Method '%s' not found on <%s>.\033[0m\n",
                        node->line, method_name, cls->name);
                interp->had_error = 1;
            } else {
                FnDef *fn = method_val->fn;
                // Create method scope from closure, bind 'this'
                Environment *method_env = env_create(fn->closure);
                env_set(method_env, "this", obj);  // shared ref — don't destroy

                // Bind parameters
                for (size_t i = 0; i < fn->param_count && i < argc; i++)
                    env_set(method_env, fn->params[i].name, args[i]);
                for (size_t i = argc; i < fn->param_count; i++)
                    env_set(method_env, fn->params[i].name, value_null());
                args_consumed = 1;   // env_set took ownership of args[i]
                if (cls->parent) {
                    Value *super_cls = (Value *)calloc(1, sizeof(Value));
                    super_cls->type = VAL_CLASS;
                    super_cls->class_def = cls->parent;
                    env_set(method_env, "__super__", super_cls);
                }

                value_destroy(result);
                result = eval(interp, fn->body, method_env);

                // Unwrap return sentinel
                if (result && result->type == VAL_RETURN) {
                    Value *inner = result->inner;
                    result->inner = NULL;
                    value_destroy(result);
                    result = inner;
                }
                env_destroy(method_env);
            }
        } else if (obj->type == VAL_STRING) {
            // ── Module method call (obj is a string holding the module name) ──
            // Try native module first
            int found_native = 0;
            for (size_t mi = 0; mi < interp->module_count; mi++) {
                if (strcmp(interp->modules[mi].name, obj->str) == 0) {
                    found_native = 1;
                    break;
                }
            }
            if (found_native) {
                value_destroy(result);
                result = call_module_fn(interp, obj->str, method_name, args, argc);
            } else if (is_user_module(interp, obj->str)) {
                // ── User module: look up exported symbol ──────────────────────
                Value *exported = lookup_user_module(interp, obj->str, method_name);
                if (!exported) {
                    fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: '%s' is not exported from module '%s'.\033[0m\n",
                            node->line, method_name, obj->str);
                    interp->had_error = 1;
                } else if (exported->type == VAL_FUNCTION) {
                    // Call the exported function
                    FnDef *fn = exported->fn;
                    Environment *fn_env = env_create(fn->closure);
                    for (size_t i = 0; i < fn->param_count && i < argc; i++)
                        env_set(fn_env, fn->params[i].name, args[i]);
                    for (size_t i = argc; i < fn->param_count; i++)
                        env_set(fn_env, fn->params[i].name, value_null());
                    args_consumed = 1;   // env_set took ownership of args[i]

                    Value *call_result = eval(interp, fn->body, fn_env);
                    if (call_result && call_result->type == VAL_RETURN) {
                        Value *inner = call_result->inner;
                        call_result->inner = NULL;
                        value_destroy(call_result);
                        call_result = inner;
                    }
                    env_destroy(fn_env);

                    value_destroy(result);
                    result = call_result ? call_result : value_null();
                } else if (exported->type == VAL_CLASS) {
                    // Accessing a class from a module: return it for use with 'new'
                    // This path is hit for mod.ClassName — return the class value
                    value_destroy(result);
                    result = exported;   // shared reference
                } else {
                    // Not callable and not a class
                    fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: '%s' in module '%s' is not callable.\033[0m\n",
                            node->line, method_name, obj->str);
                    interp->had_error = 1;
                }
            } else {
                fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Module '%s' not found.\033[0m\n",
                        node->line, obj->str);
                interp->had_error = 1;
            }
        } else {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Cannot call method '%s' on non-object.\033[0m\n",
                    node->line, method_name);
            interp->had_error = 1;
        }

        // Cleanup args: native module calls don't consume args, so free them.
        // Instance and user-module calls hand args off via env_set — don't double-free.
        if (!args_consumed) {
            for (size_t i = 0; i < argc; i++) value_destroy(args[i]);
        }
        free(args);
        // obj is shared (not owned here) for instances; for module strings we created it, so destroy.
        if (obj->type == VAL_STRING) value_destroy(obj);
        return result;
    }

    // ── CLASS DECL ─────────────────────────────────────────────────────────
    case NODE_CLASS_DECL: {
        // children[0] = parent class ident or NULL node
        // children[1..n] = NODE_FN_DECL method nodes
        ClassDef *cls = (ClassDef *)calloc(1, sizeof(ClassDef));
        cls->name    = strdup(node->str_value);
        cls->methods = env_create(NULL);  // flat method table, no parent chain
        cls->parent  = NULL;

        // Resolve parent class if specified
        if (node->children[0]->type == NODE_IDENTIFIER) {
            Value *parent_val = env_get(env, node->children[0]->str_value);
            if (!parent_val || parent_val->type != VAL_CLASS) {
                fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Parent class '%s' not found.\033[0m\n",
                        node->line, node->children[0]->str_value);
                interp->had_error = 1;
                env_destroy(cls->methods);
                free(cls->name);
                free(cls);
                return value_null();
            }
            cls->parent = parent_val->class_def;
        }

        // Register each method in the method table
        for (size_t i = 1; i < node->child_count; i++) {
            ASTNode *method_node = node->children[i];
            if (method_node->type != NODE_FN_DECL) continue;

            // Create a VAL_FUNCTION for this method, capturing current env as closure
            Value *fnval = (Value *)calloc(1, sizeof(Value));
            fnval->type = VAL_FUNCTION;
            fnval->fn   = (FnDef *)malloc(sizeof(FnDef));
            fnval->fn->name        = strdup(method_node->str_value);
            fnval->fn->params      = method_node->params;
            fnval->fn->param_count = method_node->param_count;
            fnval->fn->body        = method_node->children[0]; // the block
            fnval->fn->closure     = env;                      // capture defining env

            env_set(cls->methods, method_node->str_value, fnval);
        }

        // Create and register the VAL_CLASS value
        Value *class_val = (Value *)calloc(1, sizeof(Value));
        class_val->type      = VAL_CLASS;
        class_val->class_def = cls;
        env_set(env, node->str_value, class_val);
        return value_null();
    }

    // ── NEW ────────────────────────────────────────────────────────────────
    case NODE_NEW: {
        // Look up the class
        Value *class_val = env_get(env, node->str_value);
        if (!class_val || class_val->type != VAL_CLASS) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: '%s' is not a class.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            return value_null();
        }
        ClassDef *cls = class_val->class_def;

        // Create instance
        InstanceData *inst = (InstanceData *)calloc(1, sizeof(InstanceData));
        inst->class_def = cls;
        inst->fields    = env_create(NULL);  // flat field table

        Value *instance = (Value *)calloc(1, sizeof(Value));
        instance->type     = VAL_INSTANCE;
        instance->instance = inst;

        // Register instance in its own fields as 'this' isn't strictly needed here —
        // 'this' is bound when calling methods. But we store instance in a local var.

        // Evaluate constructor arguments
        size_t argc = node->child_count;
        Value **args = (Value **)malloc(sizeof(Value *) * (argc ? argc : 1));
        for (size_t i = 0; i < argc; i++)
            args[i] = eval(interp, node->children[i], env);

        // Find and call 'init' method if it exists (walk hierarchy)
        Value *init_method = NULL;
        ClassDef *search = cls;
        while (search && !init_method) {
            init_method = env_get(search->methods, "init");
            search = search->parent;
        }

        if (init_method && init_method->type == VAL_FUNCTION) {
            FnDef *fn = init_method->fn;
            Environment *init_env = env_create(fn->closure);

            // Bind 'this' to the new instance
            env_set(init_env, "this", instance);  // shared ref

            // Bind __super__ if class has a parent
            if (cls->parent) {
                Value *super_cls = (Value *)calloc(1, sizeof(Value));
                super_cls->type      = VAL_CLASS;
                super_cls->class_def = cls->parent;
                env_set(init_env, "__super__", super_cls);
            }

            // Bind parameters
            for (size_t i = 0; i < fn->param_count && i < argc; i++)
                env_set(init_env, fn->params[i].name, args[i]);
            for (size_t i = argc; i < fn->param_count; i++)
                env_set(init_env, fn->params[i].name, value_null());

            Value *ret = eval(interp, fn->body, init_env);
            // Unwrap return
            if (ret && ret->type == VAL_RETURN) {
                Value *inner = ret->inner;
                ret->inner = NULL;
                value_destroy(ret);
                ret = inner;
            }
            value_destroy(ret);
            env_destroy(init_env);
        } else {
            // No init — just free args
            for (size_t i = 0; i < argc; i++) value_destroy(args[i]);
        }
        free(args);

        return instance;
    }

    // ── THIS ───────────────────────────────────────────────────────────────
    case NODE_THIS: {
        Value *this_val = env_get(env, "this");
        if (!this_val) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: 'this' used outside of a method.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            return value_null();
        }
        return this_val;  // shared reference
    }

    // ── SUPER CALL ─────────────────────────────────────────────────────────
    case NODE_SUPER_CALL: {
        // 'super(args)' calls the parent class's init with the current 'this'
        Value *this_val = env_get(env, "this");
        Value *super_val = env_get(env, "__super__");
        if (!this_val || !super_val || super_val->type != VAL_CLASS) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: 'super' used outside of a class with a parent.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            return value_null();
        }
        ClassDef *parent_cls = super_val->class_def;

        // Find parent's init
        Value *parent_init = env_get(parent_cls->methods, "init");
        if (!parent_init || parent_init->type != VAL_FUNCTION) {
            // No init in parent — that's okay, just do nothing
            return value_null();
        }

        FnDef *fn = parent_init->fn;

        // Evaluate arguments
        size_t argc = node->child_count;
        Value **args = (Value **)malloc(sizeof(Value *) * (argc ? argc : 1));
        for (size_t i = 0; i < argc; i++)
            args[i] = eval(interp, node->children[i], env);

        // Create scope for parent init, bind same 'this'
        Environment *super_env = env_create(fn->closure);
        env_set(super_env, "this", this_val);  // same instance

        // Bind __super__ to grandparent if exists
        if (parent_cls->parent) {
            Value *gp = (Value *)calloc(1, sizeof(Value));
            gp->type      = VAL_CLASS;
            gp->class_def = parent_cls->parent;
            env_set(super_env, "__super__", gp);
        }

        // Bind parameters
        for (size_t i = 0; i < fn->param_count && i < argc; i++)
            env_set(super_env, fn->params[i].name, args[i]);
        for (size_t i = argc; i < fn->param_count; i++)
            env_set(super_env, fn->params[i].name, value_null());

        Value *ret = eval(interp, fn->body, super_env);
        if (ret && ret->type == VAL_RETURN) {
            Value *inner = ret->inner;
            ret->inner = NULL;
            value_destroy(ret);
            ret = inner;
        }
        value_destroy(ret);
        env_destroy(super_env);
        free(args);
        return value_null();
    }

    // ── PROPERTY GET ───────────────────────────────────────────────────────
    case NODE_PROPERTY_GET: {
        // children[0] = object expr, str_value = property name
        Value *obj = eval(interp, node->children[0], env);
        if (!obj || obj->type != VAL_INSTANCE) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Cannot access property '%s' on non-object.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            if (obj) value_destroy(obj);
            return value_null();
        }
        Value *prop = env_get(obj->instance->fields, node->str_value);
        if (!prop) {
            // Property not set yet — return null
            return value_null();
        }
        // Return a copy for primitives, shared for objects
        switch (prop->type) {
            case VAL_NUMBER:   return value_number(prop->num);
            case VAL_STRING:   return value_string(prop->str);
            case VAL_BOOL:     return value_bool(prop->boolean);
            case VAL_INSTANCE: return prop;   // shared
            case VAL_CLASS:    return prop;   // shared
            case VAL_FUNCTION: return prop;   // shared
            default:           return value_null();
        }
    }

    // ── PROPERTY SET ───────────────────────────────────────────────────────
    case NODE_PROPERTY_SET: {
        // children[0] = object expr, children[1] = value expr, str_value = property name
        Value *obj = eval(interp, node->children[0], env);
        if (!obj || obj->type != VAL_INSTANCE) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Cannot set property '%s' on non-object.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            if (obj) value_destroy(obj);
            return value_null();
        }
        Value *val = eval(interp, node->children[1], env);
        env_set(obj->instance->fields, node->str_value, val);
        return value_null();
    }
    // ── EXPR STMT ──────────────────────────────────────────────────────────
    case NODE_EXPR_STMT:
        return eval(interp, node->children[0], env);

    // ── LITERALS ───────────────────────────────────────────────────────────
    case NODE_NUMBER:
        return value_number(node->num_value);
    case NODE_STRING:
        return value_string(node->str_value);
    case NODE_BOOL:
        return value_bool(node->bool_value);
    case NODE_NULL:
        return value_null();

    // ── IDENTIFIER ─────────────────────────────────────────────────────────
    case NODE_IDENTIFIER: {
        Value *val = env_get(env, node->str_value);
        if (!val) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: Undefined variable '%s'.\033[0m\n",
                    node->line, node->str_value);
            interp->had_error = 1;
            return value_null();
        }
        // Return a copy for primitives; for complex types return a new wrapper
        // Value* that shares the underlying data. This prevents double-free when
        // the same function is stored in multiple variables (e.g. var d = f).
        switch (val->type) {
            case VAL_NUMBER:   return value_number(val->num);
            case VAL_STRING:   return value_string(val->str);
            case VAL_BOOL:     return value_bool(val->boolean);
            case VAL_NULL:     return value_null();
            case VAL_FUNCTION: {
                // Create a new Value wrapper pointing to the same FnDef.
                // The FnDef itself is owned by the original env entry and will be
                // freed when that entry is destroyed. Set fn_shared=1 to indicate
                // this wrapper doesn't own the FnDef and shouldn't free it.
                Value *fnval = (Value *)calloc(1, sizeof(Value));
                fnval->type      = VAL_FUNCTION;
                fnval->fn        = val->fn;  // shared reference to the FnDef
                fnval->fn_shared = 1;        // don't free FnDef on destroy
                return fnval;
            }
            default:           return val;  // class/instance/array – shared ref
        }
    }

    // ── BINARY ─────────────────────────────────────────────────────────────
    case NODE_BINARY: {
        Value *left  = eval(interp, node->children[0], env);
        Value *right = eval(interp, node->children[1], env);
        const char *op = node->str_value;
        Value *result  = value_null();

        // String concatenation: string + anything → string concat
        if (strcmp(op, "+") == 0 && (left->type == VAL_STRING || right->type == VAL_STRING)) {
            char *ls = value_to_string(left);
            char *rs = value_to_string(right);
            size_t len = strlen(ls) + strlen(rs) + 1;
            char *buf = (char *)malloc(len);
            strcpy(buf, ls); strcat(buf, rs);
            value_destroy(result);
            result = value_string(buf);
            free(ls); free(rs); free(buf);
            value_destroy(left); value_destroy(right);
            return result;
        }

        // Arithmetic (both must be numbers for these)
        if (strcmp(op, "+") == 0)  { value_destroy(result); result = value_number(left->num + right->num); }
        else if (strcmp(op, "-") == 0)  { value_destroy(result); result = value_number(left->num - right->num); }
        else if (strcmp(op, "*") == 0)  { value_destroy(result); result = value_number(left->num * right->num); }
        else if (strcmp(op, "/") == 0)  {
            if (right->num == 0) {
                fprintf(stderr, "\033[1;31m[Xenly Error] Division by zero.\033[0m\n");
                interp->had_error = 1;
            }
            value_destroy(result);
            result = value_number(right->num != 0 ? left->num / right->num : 0);
        }
        else if (strcmp(op, "%") == 0)  {
            value_destroy(result);
            result = value_number((double)((long long)left->num % (long long)right->num));
        }
        // Comparison
        else if (strcmp(op, "==") == 0) {
            int eq = 0;
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER)
                eq = left->num == right->num;
            else if (left->type == VAL_STRING && right->type == VAL_STRING)
                eq = strcmp(left->str, right->str) == 0;
            else if (left->type == VAL_BOOL && right->type == VAL_BOOL)
                eq = left->boolean == right->boolean;
            else if (left->type == VAL_NULL && right->type == VAL_NULL)
                eq = 1;
            value_destroy(result); result = value_bool(eq);
        }
        else if (strcmp(op, "!=") == 0) {
            int eq = 0;
            if (left->type == VAL_NUMBER && right->type == VAL_NUMBER)
                eq = left->num == right->num;
            else if (left->type == VAL_STRING && right->type == VAL_STRING)
                eq = strcmp(left->str, right->str) == 0;
            else if (left->type == VAL_BOOL && right->type == VAL_BOOL)
                eq = left->boolean == right->boolean;
            else if (left->type == VAL_NULL && right->type == VAL_NULL)
                eq = 1;
            value_destroy(result); result = value_bool(!eq);
        }
        else if (strcmp(op, "<") == 0)  { value_destroy(result); result = value_bool(left->num < right->num); }
        else if (strcmp(op, ">") == 0)  { value_destroy(result); result = value_bool(left->num > right->num); }
        else if (strcmp(op, "<=") == 0) { value_destroy(result); result = value_bool(left->num <= right->num); }
        else if (strcmp(op, ">=") == 0) { value_destroy(result); result = value_bool(left->num >= right->num); }
        // Logical — short-circuit: return the actual operand value, not a bool.
        // 'and' returns left if it's falsy, otherwise right  (like JS &&)
        // 'or'  returns left if it's truthy, otherwise right (like JS ||)
        else if (strcmp(op, "and") == 0) {
            value_destroy(result);
            if (!is_truthy(left)) { value_destroy(right); return left; }
            value_destroy(left); return right;
        }
        else if (strcmp(op, "or") == 0) {
            value_destroy(result);
            if (is_truthy(left)) { value_destroy(right); return left; }
            value_destroy(left); return right;
        }

        value_destroy(left);
        value_destroy(right);
        return result;
    }

    // ── UNARY ──────────────────────────────────────────────────────────────
    case NODE_UNARY: {
        Value *operand = eval(interp, node->children[0], env);
        if (strcmp(node->str_value, "-") == 0) {
            Value *r = value_number(-operand->num);
            value_destroy(operand);
            return r;
        }
        if (strcmp(node->str_value, "not") == 0) {
            Value *r = value_bool(!is_truthy(operand));
            value_destroy(operand);
            return r;
        }
        value_destroy(operand);
        return value_null();
    }

    // ── MATCH EXPRESSION ───────────────────────────────────────────────────
    case NODE_MATCH: {
        // children[0] = expression to match
        // children[1..n] = match arms
        Value *match_val = eval(interp, node->children[0], env);
        
        for (size_t i = 1; i < node->child_count; i++) {
            ASTNode *arm = node->children[i];  // NODE_MATCH_ARM
            ASTNode *pattern = arm->children[0];  // NODE_PATTERN
            ASTNode *result_expr = arm->children[1];
            
            // Try to match pattern
            int matched = 0;
            Environment *match_env = env_create(env);  // scope for bindings
            
            // Check pattern type
            if (strcmp(pattern->str_value, "_") == 0) {
                // Wildcard always matches
                matched = 1;
            } else if (pattern->bool_value == 1) {
                // Literal number pattern
                if (match_val->type == VAL_NUMBER && match_val->num == pattern->num_value) {
                    matched = 1;
                }
            } else if (pattern->bool_value == 2) {
                // Literal string pattern
                if (match_val->type == VAL_STRING && strcmp(match_val->str, pattern->str_value) == 0) {
                    matched = 1;
                }
            } else if (pattern->bool_value == 3) {
                // true literal
                if (match_val->type == VAL_BOOL && match_val->boolean == 1) {
                    matched = 1;
                }
            } else if (pattern->bool_value == 4) {
                // false literal
                if (match_val->type == VAL_BOOL && match_val->boolean == 0) {
                    matched = 1;
                }
            } else if (match_val->type == VAL_ENUM_VARIANT) {
                // Variant pattern: check if tags match
                if (strcmp(match_val->variant.tag, pattern->str_value) == 0) {
                    matched = 1;
                    // Bind variant fields to pattern parameters
                    for (size_t j = 0; j < pattern->param_count && j < match_val->variant.field_count; j++) {
                        env_set(match_env, pattern->params[j].name, match_val->variant.fields[j]);
                    }
                }
            } else if (pattern->param_count == 0) {
                // Simple binding pattern (just an identifier)
                matched = 1;
                env_set(match_env, pattern->str_value, match_val);
            }
            
            if (matched) {
                // Evaluate result expression with bindings
                Value *result = eval(interp, result_expr, match_env);
                env_destroy(match_env);
                value_destroy(match_val);
                return result;
            }
            
            env_destroy(match_env);
        }
        
        // No pattern matched - error
        fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: No pattern matched in match expression.\033[0m\n", node->line);
        interp->had_error = 1;
        value_destroy(match_val);
        return value_null();
    }

    // ── ARRAY LITERAL [e0, e1, …] ─────────────────────────────────────────
    case NODE_ARRAY_LITERAL: {
        size_t n = node->child_count;
        // Allocate at least 4 slots so push() after creation never immediately OOBs
        size_t cap = n > 0 ? n : 4;
        Value **items = (Value **)malloc(sizeof(Value *) * cap);
        for (size_t i = 0; i < n; i++)
            items[i] = eval(interp, node->children[i], env);
        // value_array takes ownership of items[]
        // Temporarily back-patch capacity so value_array records the right cap
        // (value_array sets cap = len ? len : 4, so we pass n directly)
        Value *arr = value_array(items, n);
        // If n==0, value_array already sets cap=4 internally; but items[] was cap=4 too.
        // For n>0, cap == n is fine.  Safe either way.
        return arr;
    }

    // ── INDEX  arr[index]  or  str[index] ────────────────────────────────
    case NODE_INDEX: {
        Value *collection = eval(interp, node->children[0], env);
        Value *index_val  = eval(interp, node->children[1], env);

        if (!collection || !index_val || index_val->type != VAL_NUMBER) {
            value_destroy(collection);
            value_destroy(index_val);
            return value_null();
        }

        int idx = (int)index_val->num;
        value_destroy(index_val);

        if (collection->type == VAL_ARRAY) {
            if (idx < 0 || (size_t)idx >= collection->array_len) {
                value_destroy(collection);
                return value_null();
            }
            Value *elem = collection->array[idx];
            // Return a copy for primitives; shared for objects
            Value *result;
            if (!elem)                          result = value_null();
            else if (elem->type == VAL_NUMBER)  result = value_number(elem->num);
            else if (elem->type == VAL_STRING)  result = value_string(elem->str);
            else if (elem->type == VAL_BOOL)    result = value_bool(elem->boolean);
            else if (elem->type == VAL_NULL)    result = value_null();
            else                                result = elem;  // shared
            // collection is shared (VAL_ARRAY is never freed by value_destroy)
            return result;
        }

        if (collection->type == VAL_STRING) {
            int len = (int)strlen(collection->str);
            if (idx < 0 || idx >= len) {
                value_destroy(collection);
                return value_null();
            }
            char buf[2] = { collection->str[idx], '\0' };
            value_destroy(collection);
            return value_string(buf);
        }

        value_destroy(collection);
        return value_null();
    }

    // ── NULLISH  expr ?? default ──────────────────────────────────────────
    case NODE_NULLISH: {
        Value *left = eval(interp, node->children[0], env);
        if (left && left->type != VAL_NULL)
            return left;
        value_destroy(left);
        return eval(interp, node->children[1], env);
    }

    // ── TYPEOF  typeof(expr) ──────────────────────────────────────────────
    case NODE_TYPEOF: {
        Value *operand = eval(interp, node->children[0], env);
        const char *type_name;
        switch (operand ? operand->type : (ValueType)-1) {
            case VAL_NUMBER:   type_name = "number";   break;
            case VAL_STRING:   type_name = "string";   break;
            case VAL_BOOL:     type_name = "bool";     break;
            case VAL_NULL:     type_name = "null";     break;
            case VAL_ARRAY:    type_name = "array";    break;
            case VAL_FUNCTION: type_name = "function"; break;
            case VAL_CLASS:    type_name = "class";    break;
            case VAL_INSTANCE: type_name = "object";   break;
            default:           type_name = "unknown";  break;
        }
        value_destroy(operand);
        return value_string(type_name);
    }

    // ── INSTANCEOF  expr instanceof ClassName ────────────────────────────
    case NODE_INSTANCEOF: {
        // children[0] = expr to test, children[1] = NODE_IDENTIFIER (class name)
        Value *obj = eval(interp, node->children[0], env);
        if (!obj || obj->type != VAL_INSTANCE) {
            value_destroy(obj);
            return value_bool(0);
        }
        // Class name is stored in children[1]->str_value
        const char *class_name = node->children[1]->str_value;
        Value *class_val = env_get(env, class_name);
        if (!class_val || class_val->type != VAL_CLASS) {
            value_destroy(obj);
            return value_bool(0);
        }
        // Walk class hierarchy: result is true if obj's class IS class_val or derives from it
        ClassDef *cls = obj->instance->class_def;
        int result = 0;
        while (cls) {
            if (cls == class_val->class_def) { result = 1; break; }
            cls = cls->parent;
        }
        value_destroy(obj);
        return value_bool(result);
    }

    // ── TERNARY  cond ? then : else ──────────────────────────────────────
    case NODE_TERNARY: {
        Value *cond = eval(interp, node->children[0], env);
        int truthy = is_truthy(cond);
        value_destroy(cond);
        return eval(interp, node->children[truthy ? 1 : 2], env);
    }

    // ── SPAWN (async task launch) ────────────────────────────────────────
    case NODE_SPAWN: {
        // spawn fn_call  — launches async function in background
        ASTNode *call = node->children[0];
        
        if (call->type != NODE_FN_CALL && call->type != NODE_CALL_EXPR) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: spawn expects a function call.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            return value_null();
        }
        
        // Get the function
        Value *fnval = NULL;
        if (call->type == NODE_FN_CALL) {
            fnval = env_get(env, call->str_value);
        } else {
            // NODE_CALL_EXPR: evaluate the callee
            fnval = eval(interp, call->children[0], env);
        }
        
        if (!fnval || fnval->type != VAL_FUNCTION) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: spawn requires a function.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            return value_null();
        }
        
        FnDef *fn = fnval->fn;
        if (!fn->is_async) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: spawn requires an async function.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            return value_null();
        }
        
        // Evaluate arguments
        size_t argc = (call->type == NODE_FN_CALL) ? call->child_count : 
                      (call->child_count > 1 ? call->child_count - 1 : 0);
        size_t arg_start = (call->type == NODE_CALL_EXPR) ? 1 : 0;
        
        Value **args = (Value **)malloc(sizeof(Value *) * argc);
        for (size_t i = 0; i < argc; i++) {
            args[i] = eval(interp, call->children[arg_start + i], env);
        }
        
        // Create task and add to queue
        Task *task = (Task *)calloc(1, sizeof(Task));
        task->fn = fn;
        task->args = args;
        task->argc = argc;
        task->call_env = env;
        env_retain(env);
        
        // Enqueue task
        if (interp->task_queue_tail) {
            interp->task_queue_tail->next = task;
            interp->task_queue_tail = task;
        } else {
            interp->task_queue = interp->task_queue_tail = task;
        }
        
        // Return a task handle (for now, just return null)
        // In a full implementation, return a Future/Promise value
        return value_null();
    }

    // ── AWAIT (wait for async result) ────────────────────────────────────
    case NODE_AWAIT: {
        // await fn_call  — calls async function and waits for result
        ASTNode *call = node->children[0];
        
        if (call->type != NODE_FN_CALL && call->type != NODE_CALL_EXPR) {
            fprintf(stderr, "\033[1;31m[Xenly Error] Line %d: await expects a function call.\033[0m\n",
                    node->line);
            interp->had_error = 1;
            return value_null();
        }
        
        // For now, just execute synchronously
        // A full implementation would check if it's a Future/Promise and wait
        Value *result = eval(interp, call, env);
        return result;
    }

    default:
        return value_null();
    }
}

// ─── Public Entry ────────────────────────────────────────────────────────────
Value *interpreter_run(Interpreter *interp, ASTNode *program) {
    g_interp = interp;
    return eval(interp, program, interp->global);
}
