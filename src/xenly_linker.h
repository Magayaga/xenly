/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for the Linux and macOS operating systems.
 *
 */
/*
 * xenly_linker.h — Xenly Built-in Linker  (xlnk)
 *
 * Self-hosting, self-contained native linker for Linux and macOS.
 * Replaces GCC/Clang/ld/ld64 entirely.  Zero external process spawning.
 *
 * v4.0 platform-specific optimizations:
 *   Linux:  MAP_POPULATE, MAP_NORESERVE, MAP_HUGETLB, posix_fadvise,
 *           posix_fallocate, mmap(MAP_SHARED) output, pwritev fallback
 *   macOS:  F_NOCACHE, F_RDADVISE, mmap(MAP_SHARED) output, pwrite fallback
 *   Both:   Robin Hood hashing, wyhash, huge-page arenas, zero-copy input,
 *           madvise SEQUENTIAL + DONTNEED, LIKELY/UNLIKELY branch hints  Links one or more ELF-64 or Mach-O-64
 * object files and static archives (.a) into a native executable without
 * spawning any subprocess.
 *
 * Why it is faster than invoking gcc/clang:
 *   • No fork/exec overhead — linking runs in-process
 *   • Memory-mapped I/O for all input files
 *   • Single-pass symbol resolution using a hash table
 *   • Minimal output format: only required segments and sections are emitted
 *   • No LTO, no DWARF generation, no link-map files by default
 *
 * Supported input:
 *   ELF64  — x86-64 (Linux, FreeBSD) and AArch64 (Linux)
 *   Mach-O 64-bit — x86-64 and arm64 (macOS)
 *   AR archives  — System V / BSD format (.a static libraries)
 *
 * Supported output:
 *   ELF64 dynamically linked executable (PT_INTERP, PLT/GOT, .dynamic)
 *   ELF64 statically linked executable  (--static)
 *   Mach-O 64-bit executable (LC_MAIN, LC_LOAD_DYLIB)
 *
 * Pipeline position:
 *   source.xe → lexer → parser → AST → sema → codegen
 *     → .s → as → .o → xlnk → ELF/Mach-O binary
 *
 * Usage (C API):
 *   XlnkConfig cfg = xlnk_default_config();
 *   xlnk_add_object(&cfg, "main.o");
 *   xlnk_add_library(&cfg, "libxly_rt.a");
 *   xlnk_add_soname(&cfg, "libm.so.6");
 *   xlnk_add_soname(&cfg, "libpthread.so.0");
 *   cfg.output = "a.out";
 *   int rc = xlnk_link(&cfg);
 *
 * Usage (command-line, future xenlyld):
 *   xenlyld -o a.out main.o -Lpath -lxly_rt -lm -lpthread
 */

#ifndef XENLY_LINKER_H
#define XENLY_LINKER_H

#include <stddef.h>
#include <stdint.h>

/* ── Version ─────────────────────────────────────────────────────────────── */
#define XLNK_VERSION_MAJOR 4
#define XLNK_VERSION_MINOR 0
#define XLNK_VERSION_PATCH 0
#define XLNK_VERSION_STR   "4.0.0"

/* ── Limits ──────────────────────────────────────────────────────────────── */
#define XLNK_MAX_OBJECTS  256   /* input .o / archive-member files           */
#define XLNK_MAX_LIBS     64    /* static .a archives                        */
#define XLNK_MAX_SONAMES  32    /* shared libraries (.so / .dylib)           */
#define XLNK_MAX_LIBDIRS  32    /* -L search directories                     */
#define XLNK_MAX_SYMBOLS  65536 /* symbol table capacity                     */
#define XLNK_MAX_RELOCS   65536 /* relocation entries per input              */

/* ── Linker config ───────────────────────────────────────────────────────── */
typedef struct {
    /* Output */
    const char *output;         /* output file path (default "a.out")        */

    /* Input files — populated by xlnk_add_* helpers */
    const char *objects[XLNK_MAX_OBJECTS];
    int         n_objects;

    const char *libraries[XLNK_MAX_LIBS];  /* paths to .a archives          */
    int         n_libraries;

    const char *sonames[XLNK_MAX_SONAMES]; /* DT_NEEDED sonames              */
    int         n_sonames;

    const char *libdirs[XLNK_MAX_LIBDIRS]; /* -L search paths                */
    int         n_libdirs;

    /* Flags */
    int         is_static;      /* 1 = produce fully static binary           */
    int         verbose;        /* 1 = print progress to stderr              */
    int         strip;          /* 1 = omit symbol table from output         */
    int         pic;            /* 1 = position-independent output (PIE)     */

    /* Entry point (NULL = auto: "_start" on Linux, "_main" on macOS) */
    const char *entry;

    /* Dynamic linker path (NULL = platform default) */
    const char *interp;        /* "/lib64/ld-linux-x86-64.so.2", etc.       */

    /* Stack size in bytes (0 = default 8 MiB) */
    size_t      stack_size;

    /* Base address for static executables (0 = default 0x400000) */
    uint64_t    base_address;

    /* Diagnostics callback (NULL = use fprintf to stderr) */
    void (*diag)(int level, const char *msg, void *userdata);
    void *diag_userdata;
} XlnkConfig;

/* Diagnostic levels */
#define XLNK_DIAG_INFO    0
#define XLNK_DIAG_WARN    1
#define XLNK_DIAG_ERROR   2

/* ── Return codes ────────────────────────────────────────────────────────── */
#define XLNK_OK              0
#define XLNK_ERR_OPEN        1   /* cannot open input file              */
#define XLNK_ERR_FORMAT      2   /* unrecognised or corrupt file format */
#define XLNK_ERR_UNDEF       3   /* unresolved symbol reference         */
#define XLNK_ERR_RELOC       4   /* cannot apply relocation             */
#define XLNK_ERR_OUTPUT      5   /* cannot write output file            */
#define XLNK_ERR_NOENTRY     6   /* entry point symbol not found        */
#define XLNK_ERR_NOMEM       7   /* memory allocation failure           */
#define XLNK_ERR_UNSUPPORTED 8   /* unsupported feature / format        */

/* ── Public API ──────────────────────────────────────────────────────────── */

/*
 * xlnk_default_config — return a zeroed config with sensible defaults.
 */
XlnkConfig xlnk_default_config(void);

/* Helpers to populate config */
int xlnk_add_object (XlnkConfig *cfg, const char *path);
int xlnk_add_library(XlnkConfig *cfg, const char *path);   /* .a archive */
int xlnk_add_soname (XlnkConfig *cfg, const char *soname); /* shared lib  */
int xlnk_add_libdir (XlnkConfig *cfg, const char *dir);    /* -L path     */

/*
 * xlnk_link — perform the complete link step described by cfg.
 * Returns XLNK_OK on success, or one of the XLNK_ERR_* codes on failure.
 * On failure the output file is not written (or is removed if partially written).
 */
int xlnk_link(const XlnkConfig *cfg);

/*
 * xlnk_error_string — return a human-readable description of an error code.
 */
const char *xlnk_error_string(int code);

/*
 * xlnk_version — return the version string "4.0.0".
 */
const char *xlnk_version(void);

/*
 * xlnk_detect_format — sniff the format of an object file.
 * Returns one of:
 *   "elf64-x86-64"  "elf64-aarch64"
 *   "macho64-x86"   "macho64-arm64"
 *   "ar"
 *   "unknown"
 */
const char *xlnk_detect_format(const char *path);

#endif /* XENLY_LINKER_H */
