/*
 * xdmml_dynload.c - Dynamic Library Loading
 */

#include "xdmml_extended.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    #include <dlfcn.h>
    #define XDMML_HAS_DLOPEN 1
#elif defined(XDMML_PLATFORM_MACOS)
    #include <dlfcn.h>
    #define XDMML_HAS_DLOPEN 1
#endif

// ─── Shared Object Structure ─────────────────────────────────────────────────

struct XDMML_SharedObject {
#ifdef XDMML_HAS_DLOPEN
    void* handle;
#endif
    char* path;
};

// ─── Dynamic Loading Functions ───────────────────────────────────────────────

XDMML_SharedObject* xdmml_load_library(const char* path) {
    if (!path) return NULL;
    
    XDMML_SharedObject* lib = (XDMML_SharedObject*)calloc(1, sizeof(XDMML_SharedObject));
    if (!lib) return NULL;
    
    lib->path = strdup(path);
    
#ifdef XDMML_HAS_DLOPEN
    lib->handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    if (!lib->handle) {
        free(lib->path);
        free(lib);
        return NULL;
    }
#endif
    
    return lib;
}

void xdmml_unload_library(XDMML_SharedObject* library) {
    if (!library) return;
    
#ifdef XDMML_HAS_DLOPEN
    if (library->handle) {
        dlclose(library->handle);
    }
#endif
    
    free(library->path);
    free(library);
}

void* xdmml_get_symbol(XDMML_SharedObject* library, const char* name) {
    if (!library || !name) return NULL;
    
#ifdef XDMML_HAS_DLOPEN
    return dlsym(library->handle, name);
#else
    return NULL;
#endif
}

const char* xdmml_library_error(void) {
#ifdef XDMML_HAS_DLOPEN
    return dlerror();
#else
    return "Dynamic loading not supported";
#endif
}

// ─── File System Functions ───────────────────────────────────────────────────

#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <pwd.h>
#endif

const char* xdmml_get_home_dir(void) {
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    const char* home = getenv("HOME");
    if (home) return home;
    
    struct passwd* pw = getpwuid(getuid());
    return pw ? pw->pw_dir : NULL;
#else
    return NULL;
#endif
}

const char* xdmml_get_pref_dir(const char* org, const char* app) {
    static char pref_dir[512];
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    const char* home = xdmml_get_home_dir();
    if (!home) return NULL;
    
    snprintf(pref_dir, sizeof(pref_dir), "%s/.local/share/%s/%s", 
             home, org ? org : "xenly", app ? app : "app");
#elif defined(XDMML_PLATFORM_MACOS)
    const char* home = xdmml_get_home_dir();
    if (!home) return NULL;
    
    snprintf(pref_dir, sizeof(pref_dir), "%s/Library/Application Support/%s/%s",
             home, org ? org : "Xenly", app ? app : "App");
#else
    return NULL;
#endif
    
    // Create directory if it doesn't exist
    xdmml_mkdir(pref_dir);
    
    return pref_dir;
}

const char* xdmml_get_base_path(void) {
    static char base_path[512];
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_FREEBSD)
    ssize_t len = readlink("/proc/self/exe", base_path, sizeof(base_path) - 1);
    if (len != -1) {
        base_path[len] = '\0';
        char* last_slash = strrchr(base_path, '/');
        if (last_slash) {
            *(last_slash + 1) = '\0';
        }
        return base_path;
    }
#elif defined(XDMML_PLATFORM_MACOS)
    // macOS implementation
    getcwd(base_path, sizeof(base_path));
    return base_path;
#endif
    
    return NULL;
}

bool xdmml_path_exists(const char* path) {
    if (!path) return false;
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    return access(path, F_OK) == 0;
#else
    return false;
#endif
}

bool xdmml_is_directory(const char* path) {
    if (!path) return false;
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
#endif
    
    return false;
}

bool xdmml_mkdir(const char* path) {
    if (!path) return false;
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    // Create parent directories recursively
    char temp[512];
    strncpy(temp, path, sizeof(temp) - 1);
    
    for (char* p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(temp, 0755);
            *p = '/';
        }
    }
    
    return mkdir(temp, 0755) == 0 || errno == EEXIST;
#else
    return false;
#endif
}

char** xdmml_list_directory(const char* path, int* count) {
    if (!path || !count) return NULL;
    
    *count = 0;
    
#if defined(XDMML_PLATFORM_LINUX) || defined(XDMML_PLATFORM_MACOS) || defined(XDMML_PLATFORM_FREEBSD)
    DIR* dir = opendir(path);
    if (!dir) return NULL;
    
    // Count entries
    int num_entries = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            num_entries++;
        }
    }
    
    if (num_entries == 0) {
        closedir(dir);
        return NULL;
    }
    
    // Allocate array
    char** entries = (char**)malloc(sizeof(char*) * num_entries);
    
    // Read entries
    rewinddir(dir);
    int i = 0;
    while ((entry = readdir(dir)) != NULL && i < num_entries) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            entries[i++] = strdup(entry->d_name);
        }
    }
    
    closedir(dir);
    *count = num_entries;
    return entries;
#else
    return NULL;
#endif
}

void xdmml_free_directory_list(char** list, int count) {
    if (!list) return;
    
    for (int i = 0; i < count; i++) {
        free(list[i]);
    }
    free(list);
}
