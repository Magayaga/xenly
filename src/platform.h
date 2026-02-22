/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 * 
 * It is available for the Linux and macOS operating systems.
 *
 */
#ifndef PLATFORM_H
#define PLATFORM_H

/*
 * platform.h — Cross-platform compatibility layer
 * 
 * Provides portable implementations of platform-specific functionality
 * for Linux, macOS, *BSD, and potentially Windows.
 */

#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// ─── Platform Detection ──────────────────────────────────────────────────────

#if defined(__linux__)
    #define XLY_PLATFORM_LINUX 1
    #define XLY_PLATFORM_NAME "linux"
#elif defined(__APPLE__) && defined(__MACH__)
    #define XLY_PLATFORM_MACOS 1
    #define XLY_PLATFORM_NAME "darwin"
    #include <TargetConditionals.h>
#elif defined(__FreeBSD__)
    #define XLY_PLATFORM_FREEBSD 1
    #define XLY_PLATFORM_NAME "freebsd"
#elif defined(__OpenBSD__)
    #define XLY_PLATFORM_OPENBSD 1
    #define XLY_PLATFORM_NAME "openbsd"
#elif defined(__NetBSD__)
    #define XLY_PLATFORM_NETBSD 1
    #define XLY_PLATFORM_NAME "netbsd"
#elif defined(__DragonFly__)
    #define XLY_PLATFORM_DRAGONFLY 1
    #define XLY_PLATFORM_NAME "dragonfly"
#elif defined(_WIN32) || defined(_WIN64)
    #define XLY_PLATFORM_WINDOWS 1
    #define XLY_PLATFORM_NAME "windows"
#else
    #define XLY_PLATFORM_UNKNOWN 1
    #define XLY_PLATFORM_NAME "unknown"
#endif

// ─── CPU Count Detection ─────────────────────────────────────────────────────

#if defined(XLY_PLATFORM_LINUX) || \
    defined(XLY_PLATFORM_FREEBSD) || \
    defined(XLY_PLATFORM_OPENBSD) || \
    defined(XLY_PLATFORM_NETBSD) || \
    defined(XLY_PLATFORM_DRAGONFLY)
    // POSIX sysconf available
    #define XLY_HAS_SYSCONF 1
#elif defined(XLY_PLATFORM_MACOS)
    // macOS has sysconf; use it instead of sysctl to avoid BSD-type compatibility issues
    #define XLY_HAS_SYSCONF 1
#elif defined(XLY_PLATFORM_WINDOWS)
    #include <windows.h>
#endif

static inline long xly_cpu_count(void) {
#if defined(XLY_PLATFORM_MACOS)
    // _SC_NPROCESSORS_ONLN is unavailable under strict _POSIX_C_SOURCE on macOS.
    // Use sysctlbyname which has a stable, minimal prototype in sys/types.h.
    int count = 0;
    size_t count_len = sizeof(count);
    // sysctlbyname is declared in sys/sysctl.h but we call through a forward decl
    // to avoid pulling in BSD types that conflict with _POSIX_C_SOURCE.
    extern int sysctlbyname(const char *, void *, size_t *, void *, size_t);
    if (sysctlbyname("hw.logicalcpu", &count, &count_len, NULL, 0) == 0 && count > 0)
        return (long)count;
    return 1;
#elif defined(XLY_HAS_SYSCONF)
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    return (nproc > 0) ? nproc : 1;
#elif defined(XLY_PLATFORM_WINDOWS)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return (long)sysinfo.dwNumberOfProcessors;
#else
    return 1; // Conservative fallback
#endif
}

// ─── Thread Support ──────────────────────────────────────────────────────────

#if defined(XLY_PLATFORM_LINUX) || \
    defined(XLY_PLATFORM_MACOS) || \
    defined(XLY_PLATFORM_FREEBSD) || \
    defined(XLY_PLATFORM_OPENBSD) || \
    defined(XLY_PLATFORM_NETBSD) || \
    defined(XLY_PLATFORM_DRAGONFLY)
    #define XLY_HAS_PTHREADS 1
    #include <pthread.h>
#elif defined(XLY_PLATFORM_WINDOWS)
    #define XLY_HAS_WIN32_THREADS 1
    #include <windows.h>
    #include <process.h>
#endif

// ─── Dynamic Loading ─────────────────────────────────────────────────────────

#if defined(XLY_PLATFORM_LINUX) || \
    defined(XLY_PLATFORM_FREEBSD) || \
    defined(XLY_PLATFORM_OPENBSD) || \
    defined(XLY_PLATFORM_NETBSD) || \
    defined(XLY_PLATFORM_DRAGONFLY)
    #define XLY_HAS_DLOPEN 1
    #include <dlfcn.h>
#elif defined(XLY_PLATFORM_MACOS)
    #define XLY_HAS_DLOPEN 1
    #include <dlfcn.h>
#elif defined(XLY_PLATFORM_WINDOWS)
    #define XLY_HAS_WIN32_DLL 1
    #include <windows.h>
#endif

// ─── File System ─────────────────────────────────────────────────────────────

#if defined(XLY_PLATFORM_LINUX) || \
    defined(XLY_PLATFORM_MACOS) || \
    defined(XLY_PLATFORM_FREEBSD) || \
    defined(XLY_PLATFORM_OPENBSD) || \
    defined(XLY_PLATFORM_NETBSD) || \
    defined(XLY_PLATFORM_DRAGONFLY)
    #define XLY_PATH_SEPARATOR '/'
    #define XLY_PATH_SEPARATOR_STR "/"
#elif defined(XLY_PLATFORM_WINDOWS)
    #define XLY_PATH_SEPARATOR '\\'
    #define XLY_PATH_SEPARATOR_STR "\\"
#endif

// ─── Compiler Attributes ─────────────────────────────────────────────────────

#if defined(__GNUC__) || defined(__clang__)
    #define XLY_UNUSED __attribute__((unused))
    #define XLY_NORETURN __attribute__((noreturn))
    #define XLY_PRINTF_LIKE(fmt, args) __attribute__((format(printf, fmt, args)))
    #define XLY_LIKELY(x) __builtin_expect(!!(x), 1)
    #define XLY_UNLIKELY(x) __builtin_expect(!!(x), 0)
#elif defined(_MSC_VER)
    #define XLY_UNUSED
    #define XLY_NORETURN __declspec(noreturn)
    #define XLY_PRINTF_LIKE(fmt, args)
    #define XLY_LIKELY(x) (x)
    #define XLY_UNLIKELY(x) (x)
#else
    #define XLY_UNUSED
    #define XLY_NORETURN
    #define XLY_PRINTF_LIKE(fmt, args)
    #define XLY_LIKELY(x) (x)
    #define XLY_UNLIKELY(x) (x)
#endif

// ─── Memory ──────────────────────────────────────────────────────────────────

#if defined(XLY_PLATFORM_LINUX) || \
      defined(XLY_PLATFORM_FREEBSD) || \
      defined(XLY_PLATFORM_OPENBSD)
    #include <malloc.h>
#endif

// ─── Time Functions ──────────────────────────────────────────────────────────

#include <time.h>

#if defined(XLY_PLATFORM_MACOS)
    // macOS uses different timer APIs
    #include <mach/mach_time.h>
    
    static inline uint64_t xly_nanotime(void) {
        static mach_timebase_info_data_t timebase;
        if (timebase.denom == 0) {
            mach_timebase_info(&timebase);
        }
        return (mach_absolute_time() * timebase.numer) / timebase.denom;
    }
#elif defined(XLY_PLATFORM_LINUX) || \
      defined(XLY_PLATFORM_FREEBSD) || \
      defined(XLY_PLATFORM_OPENBSD) || \
      defined(XLY_PLATFORM_NETBSD)
    static inline uint64_t xly_nanotime(void) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }
#elif defined(XLY_PLATFORM_WINDOWS)
    static inline uint64_t xly_nanotime(void) {
        LARGE_INTEGER frequency, counter;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        return (counter.QuadPart * 1000000000ULL) / frequency.QuadPart;
    }
#else
    static inline uint64_t xly_nanotime(void) {
        return (uint64_t)time(NULL) * 1000000000ULL;
    }
#endif

// ─── String Functions ────────────────────────────────────────────────────────

// strndup is not always available (notably missing on some macOS versions)
#if !defined(XLY_PLATFORM_LINUX) && !defined(XLY_PLATFORM_FREEBSD)
    static inline char *xly_strndup(const char *s, size_t n) {
        size_t len = strlen(s);
        if (len > n) len = n;
        char *result = (char *)malloc(len + 1);
        if (result) {
            memcpy(result, s, len);
            result[len] = '\0';
        }
        return result;
    }
    #define strndup xly_strndup
#endif

// ─── Debugging ───────────────────────────────────────────────────────────────

#ifdef NDEBUG
    #define XLY_DEBUG(...) ((void)0)
#else
    #define XLY_DEBUG(...) fprintf(stderr, __VA_ARGS__)
#endif

#endif // PLATFORM_H
