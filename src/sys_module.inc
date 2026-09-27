/*
 * sys_module.inc
 *
 * Shared implementation of Xenly's sys module (process control, file
 * descriptor I/O, filesystem, networking, clocks, system info, bit ops,
 * syslog/IPC, and constants).
 *
 * This is a raw C fragment, not a standalone translation unit â€” it is
 * #include'd from two different contexts:
 *   1. src/modules.c        â€” compiled against the interpreter's Value type.
 *   2. src/xly_rt_compiler_stub.c â€” compiled against the compiler runtime's
 *      XlyVal type, via a local 'typedef XlyVal Value;' plus NativeFn/
 *      NativeFunc/Module typedefs, before this file is included.
 *
 * The only external symbols this fragment calls are value_number,
 * value_string, value_bool, value_null, value_array and value_to_string.
 * XlyVal and Value are byte-for-byte identical structs with an identical
 * ValType/ValueType enum (see the XlyVal struct definition in xly_rt.c),
 * so the exact same object code is ABI-correct linked against either
 * set of constructors â€” xly_rt.c already provides value_number/_string/
 * _bool/_null/_array/_to_string as thin wrappers over its XlyVal
 * constructors for exactly this purpose. This keeps ONE implementation
 * of sys.* shared by the interpreter and the native compiler instead of
 * two to maintain, and avoids pulling modules.c's other modules (reflect,
 * http, etc., which need interpreter-only Environment/HTTP glue) into
 * compiled Xenly binaries.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <syslog.h>
#include <time.h>

/* ── macOS portability shims ─────────────────────────────────────────────────
 * Problems under -D_POSIX_C_SOURCE=200809L on macOS (Xcode clang):
 *
 *  1. MAP_ANON / MAP_ANONYMOUS:
 *     <sys/mman.h> only exposes MAP_ANON under _DARWIN_C_SOURCE, which our
 *     strict POSIX flag suppresses.  Hard-code the well-known value 0x1000.
 *
 *  2. sys/sysctl.h conflict:
 *     Including <sys/sysctl.h> under strict _POSIX_C_SOURCE pulls in
 *     <sys/ucred.h> and <sys/proc.h>, which use BSD-only typedefs (u_int,
 *     u_char, u_short) that are also stripped by the strict mode → cascade
 *     of "unknown type name" errors.  Solution: do NOT include <sys/sysctl.h>;
 *     instead forward-declare only the one function we need (sysctlbyname)
 *     with plain C types — this matches its actual ABI on all macOS versions.
 *
 *  3. _SC_NPROCESSORS_ONLN / _CONF, _SC_PHYS_PAGES, _SC_AVPHYS_PAGES:
 *     These GNU/Linux sysconf constants don't exist on macOS.  We replace
 *     them with sysctlbyname("hw.logicalcpu") for CPU counts, and
 *     sysctlbyname("hw.memsize") + sysconf(_SC_PAGE_SIZE) for RAM pages.
 *     Free pages come from /usr/lib/libSystem (host_statistics64 via the
 *     forward-declared Mach call — no <mach/mach.h> needed).
 * ─────────────────────────────────────────────────────────────────────────── */
#if defined(PLATFORM_MACOS)

   /* ── MAP_ANONYMOUS ─────────────────────────────────────────────────────── */
   /* Hard-code 0x1000 — MAP_ANON has had this value on every macOS / Darwin
    * release since 10.0.  We avoid referencing MAP_ANON by name because it
    * isn't visible under strict _POSIX_C_SOURCE. */
#  ifndef MAP_ANONYMOUS
#    define MAP_ANONYMOUS 0x1000
#  endif

   /* ── F_FULLFSYNC ────────────────────────────────────────────────────────
    * fcntl command that flushes data all the way to physical storage on macOS
    * (Apple extension, value 51).  Not exposed under strict _POSIX_C_SOURCE,
    * so we hard-code the value — stable since macOS 10.3 / Darwin 7.0.      */
#  ifndef F_FULLFSYNC
#    define F_FULLFSYNC 51
#  endif

   /* ── sysctlbyname forward declaration ──────────────────────────────────── */
   /* We declare it ourselves instead of including <sys/sysctl.h> to dodge the
    * BSD-typedef cascade.  The prototype matches the real libSystem symbol. */
   extern int sysctlbyname(const char *name,
                           void *oldp, size_t *oldlenp,
                           void *newp, size_t newlen);

   /* ── host_statistics64 forward declaration ─────────────────────────────── */
   /* Sufficient to call it without pulling in all of <mach/mach.h>. */
   typedef unsigned int  xly_mach_port_t;
   typedef int           xly_kern_return_t;
   typedef unsigned int  xly_mach_msg_type_number_t;
   typedef int *         xly_host_info64_t;
#  define XLY_HOST_VM_INFO64       ((int)4)
#  define XLY_HOST_VM_INFO64_COUNT ((xly_mach_msg_type_number_t)(sizeof(struct xly_vm_statistics64) / sizeof(int)))
#  define XLY_KERN_SUCCESS         0
   struct xly_vm_statistics64 {
       unsigned int free_count;
       unsigned int active_count;
       unsigned int inactive_count;
       unsigned int wire_count;
       /* remaining fields unused — we only read free/inactive */
       unsigned int zero_fill_count;
       unsigned int reactivations;
       unsigned int pageins;
       unsigned int pageouts;
       unsigned int faults;
       unsigned int cow_faults;
       unsigned int lookups;
       unsigned int hits;
       unsigned int purges;
       unsigned int purgeable_count;
       unsigned int speculative_count;
       unsigned int decompressions;
       unsigned int compressions;
       unsigned int swapins;
       unsigned int swapouts;
       unsigned int compressor_page_count;
       unsigned int throttled_count;
       unsigned int external_page_count;
       unsigned int internal_page_count;
       unsigned long long total_uncompressed_pages_in_compressor;
   };
   extern xly_kern_return_t host_statistics64(xly_mach_port_t host,
                                              int flavor,
                                              xly_host_info64_t info,
                                              xly_mach_msg_type_number_t *count);
   extern xly_mach_port_t   mach_host_self(void);

   /* ── helper implementations ────────────────────────────────────────────── */
   static inline long xly_nprocessors_onln(void) {
       int n = 0; size_t sz = sizeof(n);
       return (sysctlbyname("hw.logicalcpu", &n, &sz, NULL, 0) == 0 && n > 0)
              ? (long)n : 1L;
   }
   static inline long xly_nprocessors_conf(void) {
       int n = 0; size_t sz = sizeof(n);
       return (sysctlbyname("hw.logicalcpu_max", &n, &sz, NULL, 0) == 0 && n > 0)
              ? (long)n : 1L;
   }
#  ifndef _SC_PAGE_SIZE
#    define _SC_PAGE_SIZE 29
#  endif
   static inline long xly_phys_pages(void) {
       unsigned long long mem = 0; size_t sz = sizeof(mem);
       if (sysctlbyname("hw.memsize", &mem, &sz, NULL, 0) != 0) return 0;
       long ps = sysconf(_SC_PAGE_SIZE);
       if (ps <= 0) ps = 4096;
       return (long)(mem / (unsigned long long)ps);
   }
   static inline long xly_avphys_pages(void) {
       xly_mach_port_t host = mach_host_self();
       struct xly_vm_statistics64 vms;
       xly_mach_msg_type_number_t cnt = XLY_HOST_VM_INFO64_COUNT;
       if (host_statistics64(host, XLY_HOST_VM_INFO64,
                             (xly_host_info64_t)&vms, &cnt) != XLY_KERN_SUCCESS)
           return 0;
       return (long)(vms.free_count + vms.inactive_count);
   }

#else  /* Linux / BSD */

   static inline long xly_nprocessors_onln(void) {
       long n = sysconf(_SC_NPROCESSORS_ONLN); return n > 0 ? n : 1;
   }
   static inline long xly_nprocessors_conf(void) {
       long n = sysconf(_SC_NPROCESSORS_CONF); return n > 0 ? n : 1;
   }
   static inline long xly_phys_pages(void) {
       long p = sysconf(_SC_PHYS_PAGES); return p > 0 ? p : 0;
   }
   static inline long xly_avphys_pages(void) {
       long p = sysconf(_SC_AVPHYS_PAGES); return p > 0 ? p : 0;
   }

#endif /* PLATFORM_MACOS */

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 1 — PROCESS CONTROL
// ═════════════════════════════════════════════════════════════════════════════

// sys.exit(code) — terminate process with exit code (C: exit())
static Value *sys_exit(Value **args, size_t argc) {
    int code = (argc >= 1 && args[0]->type == VAL_NUMBER) ? (int)args[0]->num : 0;
    exit(code);
    return value_null();
}

// sys.abort() — abnormal termination, generates SIGABRT / core dump (C: abort())
static Value *sys_abort(Value **args, size_t argc) {
    (void)args; (void)argc;
    abort();
    return value_null();
}

// sys.getpid() — current process ID (C: getpid())
static Value *sys_getpid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getpid());
}

// sys.getppid() — parent process ID (C: getppid())
static Value *sys_getppid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getppid());
}

// sys.fork() — fork process; returns child PID in parent, 0 in child (C: fork())
static Value *sys_fork(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)fork());
}

// sys.exec(path, args_array) — replace process image (C: execv())
static Value *sys_exec(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    const char *path = args[0]->str;
    char **argv_list;
    size_t argv_count;
    if (argc >= 2 && args[1]->type == VAL_ARRAY) {
        argv_count = args[1]->array_len + 2;
        argv_list  = (char **)malloc(sizeof(char *) * argv_count);
        argv_list[0] = (char *)path;
        for (size_t i = 0; i < args[1]->array_len; i++)
            argv_list[i + 1] = (args[1]->array[i]->type == VAL_STRING) ? args[1]->array[i]->str : "";
        argv_list[argv_count - 1] = NULL;
    } else {
        argv_list = (char **)malloc(sizeof(char *) * 2);
        argv_list[0] = (char *)path;
        argv_list[1] = NULL;
    }
    execv(path, argv_list);
    int err = errno;
    free(argv_list);
    return value_number((double)(-err));
}

// sys.wait() — wait for any child, returns its PID (C: wait())
static Value *sys_wait(Value **args, size_t argc) {
    (void)args; (void)argc;
    int status = 0;
    return value_number((double)wait(&status));
}

// sys.waitpid(pid) — wait for specific child PID (C: waitpid())
static Value *sys_waitpid(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    int status = 0;
    return value_number((double)waitpid((pid_t)args[0]->num, &status, 0));
}

// sys.kill(pid, sig) — send signal to process (C: kill())
// Common: SIGHUP=1, SIGINT=2, SIGKILL=9, SIGTERM=15, SIGUSR1=10, SIGUSR2=12
static Value *sys_kill(Value **args, size_t argc) {
    if (argc < 2) return value_number(-1);
    return value_number((double)kill((pid_t)args[0]->num, (int)args[1]->num));
}

// sys.raise(sig) — send signal to current process (C: raise())
static Value *sys_raise(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)raise((int)args[0]->num));
}

// sys.getrlimit(resource) — get resource limit (C: getrlimit())
// resource: 0=CPU, 1=FSIZE, 2=DATA, 3=STACK, 4=CORE, 5=RSS, 7=NOFILE, 9=AS
// Returns [soft_limit, hard_limit] (-1 = unlimited)
static Value *sys_getrlimit(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    struct rlimit rl;
    if (getrlimit((int)args[0]->num, &rl) < 0) return value_null();
    Value **elems = (Value **)malloc(sizeof(Value *) * 2);
    elems[0] = value_number(rl.rlim_cur == RLIM_INFINITY ? -1.0 : (double)rl.rlim_cur);
    elems[1] = value_number(rl.rlim_max == RLIM_INFINITY ? -1.0 : (double)rl.rlim_max);
    return value_array(elems, 2);
}

// sys.setrlimit(resource, soft, hard) — set resource limit (C: setrlimit())
// Pass -1 for unlimited. Returns 0 on success, -1 on error.
static Value *sys_setrlimit(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_number(-1);
    struct rlimit rl;
    double soft = args[1]->num, hard = args[2]->num;
    rl.rlim_cur = (soft < 0) ? RLIM_INFINITY : (rlim_t)soft;
    rl.rlim_max = (hard < 0) ? RLIM_INFINITY : (rlim_t)hard;
    return value_number((double)setrlimit((int)args[0]->num, &rl));
}

// sys.getuid() — current user ID (C: getuid())
static Value *sys_getuid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getuid());
}

// sys.getgid() — current group ID (C: getgid())
static Value *sys_getgid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getgid());
}

// sys.geteuid() — effective user ID (C: geteuid())
static Value *sys_geteuid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)geteuid());
}

// sys.getegid() — effective group ID (C: getegid())
static Value *sys_getegid(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)getegid());
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 2 — FILE DESCRIPTOR I/O & MEMORY-MAPPED FILES
// ═════════════════════════════════════════════════════════════════════════════

// sys.open(path, flags, mode) — open file descriptor (C: open())
// flags: use sys.O_RDONLY(), sys.O_WRONLY(), sys.O_RDWR(), sys.O_CREAT(), sys.O_TRUNC()
static Value *sys_open(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    int    flags = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : O_RDONLY;
    mode_t mode  = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (mode_t)args[2]->num : 0644;
    return value_number((double)open(args[0]->str, flags, mode));
}

// sys.close(fd) — close file descriptor (C: close())
static Value *sys_close(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)close((int)args[0]->num));
}

// sys.read(fd, count) — read up to count bytes (C: read()), returns string or null
static Value *sys_read(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_NUMBER) return value_null();
    size_t count = (size_t)args[1]->num;
    if (count == 0 || count > 67108864) return value_null(); // cap 64 MB
    char *buf = (char *)malloc(count + 1);
    if (!buf) return value_null();
    ssize_t n = read((int)args[0]->num, buf, count);
    if (n < 0) { free(buf); return value_null(); }
    buf[n] = '\0';
    Value *v = value_string(buf);
    free(buf);
    return v;
}

// sys.write(fd, data) — write string to fd (C: write()), returns bytes written
static Value *sys_write(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    int fd = (int)args[0]->num;
    if (args[1]->type == VAL_STRING) {
        const char *s = args[1]->str;
        return value_number((double)write(fd, s, strlen(s)));
    }
    char *s = value_to_string(args[1]);
    ssize_t n = write(fd, s, strlen(s));
    free(s);
    return value_number((double)n);
}

// sys.seek(fd, offset, whence) — reposition file offset (C: lseek())
// whence: 0=SEEK_SET, 1=SEEK_CUR, 2=SEEK_END
static Value *sys_seek(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)lseek((int)args[0]->num, (off_t)args[1]->num, (int)args[2]->num));
}

// sys.dup(fd) — duplicate fd (C: dup())
static Value *sys_dup(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)dup((int)args[0]->num));
}

// sys.dup2(old, new) — duplicate old fd as new fd (C: dup2())
static Value *sys_dup2(Value **args, size_t argc) {
    if (argc < 2) return value_number(-1);
    return value_number((double)dup2((int)args[0]->num, (int)args[1]->num));
}

// sys.pipe() — create pipe, returns [read_fd, write_fd] (C: pipe())
static Value *sys_pipe(Value **args, size_t argc) {
    (void)args; (void)argc;
    int fds[2];
    if (pipe(fds) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)fds[0]);
    e[1] = value_number((double)fds[1]);
    return value_array(e, 2);
}

// sys.fsync(fd) — flush fd's kernel buffer to hardware (C: fsync())
// Essential for databases and transactional writes
static Value *sys_fsync(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)fsync((int)args[0]->num));
}

// sys.fdatasync(fd) — flush data only, no metadata
// On macOS fdatasync() exists but F_FULLFSYNC is the reliable data-durability call.
#if defined(PLATFORM_MACOS)
   /* F_FULLFSYNC is a Darwin extension declared in <sys/fcntl.h>, but
    * -D_POSIX_C_SOURCE=200809L suppresses non-POSIX symbols, making it
    * invisible.  The numeric value 51 has been stable on every macOS /
    * Darwin release since 10.3 and is safe to hard-code here.            */
#  ifndef F_FULLFSYNC
#    define F_FULLFSYNC 51
#  endif
#endif
static Value *sys_fdatasync(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
#if defined(PLATFORM_MACOS)
    /* macOS: fcntl(F_FULLFSYNC) guarantees data reaches physical storage
     * on HFS+/APFS; plain fdatasync() only flushes the kernel buffer.   */
    return value_number((double)fcntl((int)args[0]->num, F_FULLFSYNC));
#else
    return value_number((double)fdatasync((int)args[0]->num));
#endif
}

// sys.truncate(path, length) — truncate file to length bytes (C: truncate())
static Value *sys_truncate(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)truncate(args[0]->str, (off_t)args[1]->num));
}

// sys.ftruncate(fd, length) — truncate open file to length bytes (C: ftruncate())
static Value *sys_ftruncate(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)ftruncate((int)args[0]->num, (off_t)args[1]->num));
}

// sys.mmap(fd, size, prot, flags, offset) — memory-map a file or anonymous region
// Returns an opaque handle (pointer as number) or -1 on error.
// prot:  1=PROT_READ, 2=PROT_WRITE, 3=PROT_READ|PROT_WRITE
// flags: 1=MAP_SHARED, 2=MAP_PRIVATE, 34=MAP_PRIVATE|MAP_ANONYMOUS (fd=-1 for anon)
// Essential for shared memory, zero-copy I/O, fast file access.
static Value *sys_mmap(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    size_t size  = (size_t)args[0]->num;
    int    prot  = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : PROT_READ | PROT_WRITE;
    int    flags = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : MAP_PRIVATE | MAP_ANONYMOUS;  /* MAP_ANONYMOUS shim covers macOS MAP_ANON */
    int    fd    = (argc >= 4 && args[3]->type == VAL_NUMBER) ? (int)args[3]->num : -1;
    off_t  off   = (argc >= 5 && args[4]->type == VAL_NUMBER) ? (off_t)args[4]->num : 0;
    void  *ptr   = mmap(NULL, size, prot, flags, fd, off);
    if (ptr == MAP_FAILED) return value_number(-1);
    return value_number((double)(uintptr_t)ptr);
}

// sys.munmap(ptr, size) — unmap a memory-mapped region (C: munmap())
static Value *sys_munmap(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    void  *ptr  = (void *)(uintptr_t)(long long)args[0]->num;
    size_t size = (size_t)args[1]->num;
    return value_number((double)munmap(ptr, size));
}

// sys.mmap_read(ptr, offset, count) — read bytes from mmap region into string
static Value *sys_mmap_read(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_null();
    uint8_t *base   = (uint8_t *)(uintptr_t)(long long)args[0]->num;
    size_t   offset = (size_t)args[1]->num;
    size_t   count  = (size_t)args[2]->num;
    if (count == 0 || count > 67108864) return value_null();
    char *buf = (char *)malloc(count + 1);
    if (!buf) return value_null();
    memcpy(buf, base + offset, count);
    buf[count] = '\0';
    Value *v = value_string(buf);
    free(buf);
    return v;
}

// sys.mmap_write(ptr, offset, data) — write string into mmap region
static Value *sys_mmap_write(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER || args[2]->type != VAL_STRING) return value_number(-1);
    uint8_t    *base   = (uint8_t *)(uintptr_t)(long long)args[0]->num;
    size_t      offset = (size_t)args[1]->num;
    const char *data   = args[2]->str;
    size_t      len    = strlen(data);
    memcpy(base + offset, data, len);
    return value_number((double)len);
}

// sys.poll(fds_array, timeout_ms) — wait for I/O events (C: poll())
// fds_array: array of [fd, events] pairs. events: 1=POLLIN, 4=POLLOUT
// Returns number of ready fds, or -1 on error. Timeout -1 = block forever.
static Value *sys_poll(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_ARRAY) return value_number(-1);
    int timeout = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : -1;
    size_t nfds = args[0]->array_len;
    struct pollfd *pfds = (struct pollfd *)calloc(nfds, sizeof(struct pollfd));
    if (!pfds) return value_number(-1);
    for (size_t i = 0; i < nfds; i++) {
        Value *pair = args[0]->array[i];
        if (pair->type == VAL_ARRAY && pair->array_len >= 2) {
            pfds[i].fd     = (int)pair->array[0]->num;
            pfds[i].events = (short)pair->array[1]->num;
        }
    }
    int ret = poll(pfds, (nfds_t)nfds, timeout);
    free(pfds);
    return value_number((double)ret);
}

// sys.fcntl_lock(fd, type) — advisory file lock (C: fcntl() F_SETLK)
// type: 0=F_RDLCK (shared), 1=F_WRLCK (exclusive), 2=F_UNLCK (release)
// Used by databases (SQLite, PostgreSQL) to coordinate concurrent access.
static Value *sys_fcntl_lock(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    struct flock fl;
    memset(&fl, 0, sizeof(fl));
    int t = (int)args[1]->num;
    fl.l_type   = (t == 0) ? F_RDLCK : (t == 1) ? F_WRLCK : F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start  = 0;
    fl.l_len    = 0; // entire file
    return value_number((double)fcntl((int)args[0]->num, F_SETLK, &fl));
}

// sys.fcntl_setfl(fd, flags) — set file status flags (C: fcntl() F_SETFL)
// e.g. set O_NONBLOCK: sys.fcntl_setfl(fd, sys.O_NONBLOCK())
static Value *sys_fcntl_setfl(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)fcntl((int)args[0]->num, F_SETFL, (int)args[1]->num));
}

// sys.fcntl_getfl(fd) — get file status flags (C: fcntl() F_GETFL)
static Value *sys_fcntl_getfl(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)fcntl((int)args[0]->num, F_GETFL, 0));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 3 — FILESYSTEM
// ═════════════════════════════════════════════════════════════════════════════

// sys.stat(path) — file metadata (C: stat())
// Returns [size, mode, uid, gid, mtime, atime, nlinks] or null
static Value *sys_stat(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    struct stat st;
    if (stat(args[0]->str, &st) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 7);
    e[0] = value_number((double)st.st_size);
    e[1] = value_number((double)st.st_mode);
    e[2] = value_number((double)st.st_uid);
    e[3] = value_number((double)st.st_gid);
    e[4] = value_number((double)st.st_mtime);
    e[5] = value_number((double)st.st_atime);
    e[6] = value_number((double)st.st_nlink);
    return value_array(e, 7);
}

// sys.lstat(path) — like stat() but for symlinks themselves (C: lstat())
static Value *sys_lstat(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    struct stat st;
    if (lstat(args[0]->str, &st) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 7);
    e[0] = value_number((double)st.st_size);
    e[1] = value_number((double)st.st_mode);
    e[2] = value_number((double)st.st_uid);
    e[3] = value_number((double)st.st_gid);
    e[4] = value_number((double)st.st_mtime);
    e[5] = value_number((double)st.st_atime);
    e[6] = value_number((double)st.st_nlink);
    return value_array(e, 7);
}

// sys.isfile(path) — true if path is a regular file
static Value *sys_isfile(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    return value_bool(stat(args[0]->str, &st) == 0 && S_ISREG(st.st_mode));
}

// sys.isdir(path) — true if path is a directory
static Value *sys_isdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    return value_bool(stat(args[0]->str, &st) == 0 && S_ISDIR(st.st_mode));
}

// sys.islnk(path) — true if path is a symbolic link
static Value *sys_islnk(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_bool(0);
    struct stat st;
    return value_bool(lstat(args[0]->str, &st) == 0 && S_ISLNK(st.st_mode));
}

// sys.access(path, mode) — check file accessibility (C: access())
// mode: 0=F_OK(exists), 1=X_OK(exec), 2=W_OK(write), 4=R_OK(read)
static Value *sys_access(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)access(args[0]->str, (int)args[1]->num));
}

// sys.mkdir(path, mode) — create directory (C: mkdir())
static Value *sys_mkdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    mode_t m = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (mode_t)args[1]->num : 0755;
    return value_number((double)mkdir(args[0]->str, m));
}

// sys.rmdir(path) — remove empty directory (C: rmdir())
static Value *sys_rmdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)rmdir(args[0]->str));
}

// sys.unlink(path) — delete a file (C: unlink())
static Value *sys_unlink(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)unlink(args[0]->str));
}

// sys.rename(old, new) — rename/move file (C: rename())
static Value *sys_rename(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)rename(args[0]->str, args[1]->str));
}

// sys.link(old, new) — create hard link (C: link())
// Both names refer to the same inode — used by version control, atomic replace.
static Value *sys_link(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)link(args[0]->str, args[1]->str));
}

// sys.symlink(target, linkname) — create symbolic link (C: symlink())
static Value *sys_symlink(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)symlink(args[0]->str, args[1]->str));
}

// sys.readlink(path) — read symbolic link target (C: readlink())
static Value *sys_readlink(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    char buf[4096];
    ssize_t n = readlink(args[0]->str, buf, sizeof(buf) - 1);
    if (n < 0) return value_null();
    buf[n] = '\0';
    return value_string(buf);
}

// sys.chmod(path, mode) — change file permissions (C: chmod())
// mode is octal e.g. 0644=420, 0755=493
static Value *sys_chmod(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)chmod(args[0]->str, (mode_t)args[1]->num));
}

// sys.chown(path, uid, gid) — change file owner/group (C: chown())
// Pass -1 for uid or gid to leave unchanged.
static Value *sys_chown(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_STRING) return value_number(-1);
    uid_t uid = (uid_t)args[1]->num;
    gid_t gid = (gid_t)args[2]->num;
    return value_number((double)chown(args[0]->str, uid, gid));
}

// sys.getcwd() — current working directory (C: getcwd())
static Value *sys_getcwd(Value **args, size_t argc) {
    (void)args; (void)argc;
    char buf[4096];
    return getcwd(buf, sizeof(buf)) ? value_string(buf) : value_null();
}

// sys.chdir(path) — change working directory (C: chdir())
static Value *sys_chdir(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)chdir(args[0]->str));
}

// sys.mkfifo(path, mode) — create named pipe / FIFO (C: mkfifo())
// Named pipes allow unrelated processes to communicate via the filesystem.
static Value *sys_mkfifo(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    mode_t m = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (mode_t)args[1]->num : 0644;
    return value_number((double)mkfifo(args[0]->str, m));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 4 — NETWORKING (POSIX SOCKETS)
// ═════════════════════════════════════════════════════════════════════════════

// sys.socket(domain, type, proto) — create socket (C: socket())
// domain: 2=AF_INET (IPv4), 10=AF_INET6 (IPv6), 1=AF_UNIX
// type:   1=SOCK_STREAM (TCP), 2=SOCK_DGRAM (UDP)
// proto:  0=default
static Value *sys_socket(Value **args, size_t argc) {
    int domain = (argc >= 1 && args[0]->type == VAL_NUMBER) ? (int)args[0]->num : AF_INET;
    int type   = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : SOCK_STREAM;
    int proto  = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : 0;
    return value_number((double)socket(domain, type, proto));
}

// sys.connect(fd, ip, port) — connect TCP socket to ip:port (C: connect())
static Value *sys_connect(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_STRING) return value_number(-1);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((uint16_t)args[2]->num);
    if (inet_pton(AF_INET, args[1]->str, &addr.sin_addr) <= 0) return value_number(-1);
    return value_number((double)connect((int)args[0]->num, (struct sockaddr *)&addr, sizeof(addr)));
}

// sys.bind(fd, ip, port) — bind socket to address/port (C: bind())
// Use ip="" or "0.0.0.0" to bind to all interfaces.
static Value *sys_bind(Value **args, size_t argc) {
    if (argc < 3 || args[0]->type != VAL_NUMBER) return value_number(-1);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons((uint16_t)args[2]->num);
    addr.sin_addr.s_addr = (argc >= 2 && args[1]->type == VAL_STRING && strlen(args[1]->str) > 0)
                         ? inet_addr(args[1]->str) : INADDR_ANY;
    int optval = 1;
    setsockopt((int)args[0]->num, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    return value_number((double)bind((int)args[0]->num, (struct sockaddr *)&addr, sizeof(addr)));
}

// sys.listen(fd, backlog) — mark socket as passive (server) (C: listen())
static Value *sys_listen(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)listen((int)args[0]->num, (int)args[1]->num));
}

// sys.accept(fd) — accept incoming connection, returns [client_fd, client_ip] (C: accept())
static Value *sys_accept(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    struct sockaddr_in caddr;
    socklen_t clen = sizeof(caddr);
    int cfd = accept((int)args[0]->num, (struct sockaddr *)&caddr, &clen);
    if (cfd < 0) return value_null();
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &caddr.sin_addr, ip, sizeof(ip));
    Value **e = (Value **)malloc(sizeof(Value *) * 3);
    e[0] = value_number((double)cfd);
    e[1] = value_string(ip);
    e[2] = value_number((double)ntohs(caddr.sin_port));
    return value_array(e, 3);
}

// sys.send(fd, data, flags) — send data on socket (C: send())
// flags: 0=none, 1=MSG_OOB, 2=MSG_DONTROUTE
static Value *sys_send(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_STRING) return value_number(-1);
    int   flags = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : 0;
    const char *s = args[1]->str;
    return value_number((double)send((int)args[0]->num, s, strlen(s), flags));
}

// sys.recv(fd, count, flags) — receive data from socket (C: recv())
static Value *sys_recv(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER || args[1]->type != VAL_NUMBER) return value_null();
    size_t count = (size_t)args[1]->num;
    if (count == 0 || count > 67108864) return value_null();
    int flags = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : 0;
    char *buf = (char *)malloc(count + 1);
    if (!buf) return value_null();
    ssize_t n = recv((int)args[0]->num, buf, count, flags);
    if (n < 0) { free(buf); return value_null(); }
    buf[n] = '\0';
    Value *v = value_string(buf);
    free(buf);
    return v;
}

// sys.setsockopt(fd, level, optname, value) — set socket option (C: setsockopt())
// level: 1=SOL_SOCKET, 6=IPPROTO_TCP
// optname (SOL_SOCKET): 2=SO_REUSEADDR, 7=SO_KEEPALIVE, 8=SO_DONTROUTE
static Value *sys_setsockopt(Value **args, size_t argc) {
    if (argc < 4 || args[0]->type != VAL_NUMBER) return value_number(-1);
    int optval = (int)args[3]->num;
    return value_number((double)setsockopt((int)args[0]->num, (int)args[1]->num,
                                           (int)args[2]->num, &optval, sizeof(optval)));
}

// sys.inet_aton(ip) — convert IPv4 string to 32-bit integer (C: inet_addr())
static Value *sys_inet_aton(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)ntohl(inet_addr(args[0]->str)));
}

// sys.inet_ntoa(n) — convert 32-bit integer to IPv4 string (C: inet_ntoa())
static Value *sys_inet_ntoa(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_null();
    struct in_addr addr;
    addr.s_addr = htonl((uint32_t)args[0]->num);
    return value_string(inet_ntoa(addr));
}

// sys.htons(n) — host-to-network short (byte-swap if needed) (C: htons())
static Value *sys_htons(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)htons((uint16_t)args[0]->num));
}

// sys.ntohs(n) — network-to-host short (C: ntohs())
static Value *sys_ntohs(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)ntohs((uint16_t)args[0]->num));
}

// sys.htonl(n) — host-to-network long (C: htonl())
static Value *sys_htonl(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)htonl((uint32_t)args[0]->num));
}

// sys.ntohl(n) — network-to-host long (C: ntohl())
static Value *sys_ntohl(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)ntohl((uint32_t)args[0]->num));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 5 — HIGH-RESOLUTION TIME & CLOCKS
// ═════════════════════════════════════════════════════════════════════════════

// sys.clock_realtime() — wall clock time as [seconds, nanoseconds] (C: clock_gettime(CLOCK_REALTIME))
// Affected by NTP adjustments; use for timestamps.
static Value *sys_clock_realtime(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)ts.tv_sec);
    e[1] = value_number((double)ts.tv_nsec);
    return value_array(e, 2);
}

// sys.clock_monotonic() — monotonic clock [seconds, nanoseconds] (C: clock_gettime(CLOCK_MONOTONIC))
// Never goes backwards; use for measuring elapsed time / benchmarking.
static Value *sys_clock_monotonic(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)ts.tv_sec);
    e[1] = value_number((double)ts.tv_nsec);
    return value_array(e, 2);
}

// sys.clock_process() — CPU time used by this process [sec, nsec] (C: CLOCK_PROCESS_CPUTIME_ID)
static Value *sys_clock_process(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)ts.tv_sec);
    e[1] = value_number((double)ts.tv_nsec);
    return value_array(e, 2);
}

// sys.nanosleep(sec, nsec) — sleep with nanosecond precision (C: nanosleep())
// Far more precise than usleep(). Essential for real-time / embedded code.
static Value *sys_nanosleep(Value **args, size_t argc) {
    if (argc < 2) return value_number(-1);
    struct timespec req;
    req.tv_sec  = (time_t)args[0]->num;
    req.tv_nsec = (long)args[1]->num;
    return value_number((double)nanosleep(&req, NULL));
}

// sys.time() — Unix timestamp as integer seconds (C: time())
static Value *sys_time(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)time(NULL));
}

// sys.clock() — CPU time consumed (C: clock()), returns seconds as float
static Value *sys_clock_fn(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)clock() / (double)CLOCKS_PER_SEC);
}

// sys.gettimeofday() — microsecond-precision wall time [sec, usec] (C: gettimeofday())
static Value *sys_gettimeofday(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    Value **e = (Value **)malloc(sizeof(Value *) * 2);
    e[0] = value_number((double)tv.tv_sec);
    e[1] = value_number((double)tv.tv_usec);
    return value_array(e, 2);
}

// sys.sleep(sec) — sleep N whole seconds (C: sleep())
static Value *sys_sleep_sec(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(0);
    return value_number((double)sleep((unsigned int)args[0]->num));
}

// sys.usleep(usec) — sleep N microseconds (C: usleep())
static Value *sys_usleep(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_NUMBER) return value_number(-1);
    return value_number((double)usleep((useconds_t)args[0]->num));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 6 — SYSTEM INFORMATION & ENVIRONMENT
// ═════════════════════════════════════════════════════════════════════════════

// sys.uname() — OS/kernel identity [sysname, nodename, release, version, machine] (C: uname())
static Value *sys_uname(Value **args, size_t argc) {
    (void)args; (void)argc;
    struct utsname u;
    if (uname(&u) < 0) return value_null();
    Value **e = (Value **)malloc(sizeof(Value *) * 5);
    e[0] = value_string(u.sysname);
    e[1] = value_string(u.nodename);
    e[2] = value_string(u.release);
    e[3] = value_string(u.version);
    e[4] = value_string(u.machine);
    return value_array(e, 5);
}

// sys.hostname() — machine hostname (C: gethostname())
static Value *sys_hostname(Value **args, size_t argc) {
    (void)args; (void)argc;
    char buf[256];
    return (gethostname(buf, sizeof(buf)) == 0) ? value_string(buf) : value_null();
}

// sys.nproc() — online CPU count (portable: sysctlbyname on macOS, sysconf on Linux)
static Value *sys_nproc(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_nprocessors_onln());
}

// sys.nproc_conf() — configured CPU count (may be higher than online)
static Value *sys_nproc_conf(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_nprocessors_conf());
}

// sys.phys_pages() — total physical RAM pages (portable)
static Value *sys_phys_pages(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_phys_pages());
}

// sys.avphys_pages() — available (free) RAM pages (portable)
static Value *sys_avphys_pages(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)xly_avphys_pages());
}

// sys.PAGE_SIZE() — OS memory page size in bytes (typically 4096)
static Value *sys_PAGE_SIZE(Value **args, size_t argc) {
    (void)args; (void)argc;
    long ps = sysconf(_SC_PAGE_SIZE);
    return value_number((double)(ps > 0 ? ps : 4096));
}

// sys.endian() — byte order of this CPU: "little" or "big"
static Value *sys_endian(Value **args, size_t argc) {
    (void)args; (void)argc;
    uint16_t t = 0x0001;
    return value_string((*(uint8_t *)&t) ? "little" : "big");
}

// sys.sizeof_ptr() — pointer width: 4 (32-bit) or 8 (64-bit)
static Value *sys_sizeof_ptr(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)sizeof(void *));
}

// sys.getenv(name) — read environment variable (C: getenv())
static Value *sys_getenv(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    const char *v = getenv(args[0]->str);
    return v ? value_string(v) : value_null();
}

// sys.setenv(name, value) — set environment variable (C: setenv())
static Value *sys_setenv(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return value_number(-1);
    return value_number((double)setenv(args[0]->str, args[1]->str, 1));
}

// sys.unsetenv(name) — remove environment variable (C: unsetenv())
static Value *sys_unsetenv(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)unsetenv(args[0]->str));
}

// sys.errno() — current errno value (C: errno)
static Value *sys_errno_fn(Value **args, size_t argc) {
    (void)args; (void)argc;
    return value_number((double)errno);
}

// sys.strerror(code) — human-readable errno description (C: strerror())
static Value *sys_strerror(Value **args, size_t argc) {
    int code = (argc >= 1 && args[0]->type == VAL_NUMBER) ? (int)args[0]->num : errno;
    return value_string(strerror(code));
}

// sys.system(cmd) — run shell command, returns exit status (C: system())
static Value *sys_system(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_number(-1);
    return value_number((double)system(args[0]->str));
}

// sys.proc_status() — read /proc/self/status into a string (Linux)
// Returns kernel view of the current process: VmRSS, threads, state, etc.
static Value *sys_proc_status(Value **args, size_t argc) {
    (void)args; (void)argc;
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0) return value_null();
    char buf[8192];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 0) return value_null();
    buf[n] = '\0';
    return value_string(buf);
}

// sys.proc_maps() — read /proc/self/maps (virtual memory layout of process)
// Shows every mapped region: code, stack, heap, shared libs, mmap regions.
static Value *sys_proc_maps(Value **args, size_t argc) {
    (void)args; (void)argc;
    int fd = open("/proc/self/maps", O_RDONLY);
    if (fd < 0) return value_null();
    char buf[65536];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 0) return value_null();
    buf[n] = '\0';
    return value_string(buf);
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 7 — BIT & MEMORY OPERATIONS
// ═════════════════════════════════════════════════════════════════════════════

static Value *sys_band(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num, b = (uint64_t)(long long)args[1]->num;
    return value_number((double)(long long)(a & b));
}
static Value *sys_bor(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num, b = (uint64_t)(long long)args[1]->num;
    return value_number((double)(long long)(a | b));
}
static Value *sys_bxor(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num, b = (uint64_t)(long long)args[1]->num;
    return value_number((double)(long long)(a ^ b));
}
static Value *sys_bnot(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)(long long)(~a));
}
static Value *sys_shl(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(n >= 0 ? a << n : a >> (-n)));
}
static Value *sys_shr(Value **args, size_t argc) {
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(n >= 0 ? a >> n : a << (-n)));
}
static Value *sys_sar(Value **args, size_t argc) {
    // Arithmetic (signed) right shift — preserves sign bit
    if (argc < 2) return value_number(0);
    long long a = (long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(a >> n));
}
static Value *sys_rol(Value **args, size_t argc) {
    // Rotate left (32-bit)
    if (argc < 2) return value_number(0);
    uint32_t a = (uint32_t)(long long)args[0]->num; int n = ((int)args[1]->num) & 31;
    return value_number((double)(long long)((a << n) | (a >> (32 - n))));
}
static Value *sys_ror(Value **args, size_t argc) {
    // Rotate right (32-bit) — used in SHA, AES, CRC algorithms
    if (argc < 2) return value_number(0);
    uint32_t a = (uint32_t)(long long)args[0]->num; int n = ((int)args[1]->num) & 31;
    return value_number((double)(long long)((a >> n) | (a << (32 - n))));
}
static Value *sys_popcnt(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)__builtin_popcountll(a));
}
static Value *sys_clz(Value **args, size_t argc) {
    if (argc < 1) return value_number(64);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number(a == 0 ? 64.0 : (double)__builtin_clzll(a));
}
static Value *sys_ctz(Value **args, size_t argc) {
    if (argc < 1) return value_number(64);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number(a == 0 ? 64.0 : (double)__builtin_ctzll(a));
}
static Value *sys_bswap32(Value **args, size_t argc) {
    // Byte-swap 32-bit — used for endian conversion in network/file code
    if (argc < 1) return value_number(0);
    uint32_t a = (uint32_t)(long long)args[0]->num;
    return value_number((double)(long long)__builtin_bswap32(a));
}
static Value *sys_bswap64(Value **args, size_t argc) {
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)(long long)__builtin_bswap64(a));
}
static Value *sys_parity(Value **args, size_t argc) {
    // Returns 1 if number of set bits is odd, 0 if even (parity check)
    if (argc < 1) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num;
    return value_number((double)(__builtin_parityll(a)));
}
static Value *sys_bit_get(Value **args, size_t argc) {
    // Get bit at position n: sys.bit_get(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)((a >> n) & 1));
}
static Value *sys_bit_set(Value **args, size_t argc) {
    // Set bit at position n: sys.bit_set(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(a | ((uint64_t)1 << n)));
}
static Value *sys_bit_clear(Value **args, size_t argc) {
    // Clear bit at position n: sys.bit_clear(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(a & ~((uint64_t)1 << n)));
}
static Value *sys_bit_toggle(Value **args, size_t argc) {
    // Toggle bit at position n: sys.bit_toggle(value, n)
    if (argc < 2) return value_number(0);
    uint64_t a = (uint64_t)(long long)args[0]->num; int n = (int)args[1]->num;
    return value_number((double)(long long)(a ^ ((uint64_t)1 << n)));
}
static Value *sys_align_up(Value **args, size_t argc) {
    // Round n up to nearest multiple of align (must be power of 2)
    // Fundamental in memory allocators and struct layout
    if (argc < 2) return value_number(0);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    uint64_t a = (uint64_t)(long long)args[1]->num;
    if (a == 0) return value_number((double)(long long)n);
    return value_number((double)(long long)((n + a - 1) & ~(a - 1)));
}
static Value *sys_align_down(Value **args, size_t argc) {
    // Round n down to nearest multiple of align (must be power of 2)
    if (argc < 2) return value_number(0);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    uint64_t a = (uint64_t)(long long)args[1]->num;
    if (a == 0) return value_number((double)(long long)n);
    return value_number((double)(long long)(n & ~(a - 1)));
}
static Value *sys_is_pow2(Value **args, size_t argc) {
    // Returns true if n is a power of 2 (n > 0 && (n & (n-1)) == 0)
    if (argc < 1) return value_bool(0);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    return value_bool(n > 0 && (n & (n - 1)) == 0);
}
static Value *sys_next_pow2(Value **args, size_t argc) {
    // Next power of 2 >= n — used in buffer sizing, hash tables
    if (argc < 1) return value_number(1);
    uint64_t n = (uint64_t)(long long)args[0]->num;
    if (n == 0) return value_number(1);
    n--;
    n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8;
    n |= n >> 16; n |= n >> 32;
    return value_number((double)(long long)(n + 1));
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 8 — IPC & SYSTEM LOGGING
// ═════════════════════════════════════════════════════════════════════════════

// sys.openlog(ident, option, facility) — open syslog connection (C: openlog())
// Used by system daemons to write structured logs to the OS log daemon.
// option: 0=default, 1=LOG_PID, 2=LOG_CONS
// facility: 0=LOG_KERN, 1=LOG_USER, 3=LOG_DAEMON, 4=LOG_AUTH
static Value *sys_openlog(Value **args, size_t argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return value_null();
    int option   = (argc >= 2 && args[1]->type == VAL_NUMBER) ? (int)args[1]->num : LOG_PID;
    int facility = (argc >= 3 && args[2]->type == VAL_NUMBER) ? (int)args[2]->num : LOG_USER;
    openlog(args[0]->str, option, facility);
    return value_null();
}

// sys.syslog(priority, message) — write to system log (C: syslog())
// priority: 0=EMERG,1=ALERT,2=CRIT,3=ERR,4=WARNING,5=NOTICE,6=INFO,7=DEBUG
static Value *sys_syslog(Value **args, size_t argc) {
    if (argc < 2 || args[0]->type != VAL_NUMBER) return value_null();
    int priority = (int)args[0]->num;
    char *msg = (args[1]->type == VAL_STRING) ? args[1]->str : "";
    syslog(priority, "%s", msg);
    return value_null();
}

// sys.closelog() — close syslog (C: closelog())
static Value *sys_closelog(Value **args, size_t argc) {
    (void)args; (void)argc;
    closelog();
    return value_null();
}

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 9 — FILE OPEN FLAGS & SIGNAL CONSTANTS
// ═════════════════════════════════════════════════════════════════════════════

// Open flags — use these with sys.open()
static Value *sys_O_RDONLY(Value **a, size_t c)   { (void)a;(void)c; return value_number(O_RDONLY); }
static Value *sys_O_WRONLY(Value **a, size_t c)   { (void)a;(void)c; return value_number(O_WRONLY); }
static Value *sys_O_RDWR(Value **a, size_t c)     { (void)a;(void)c; return value_number(O_RDWR);   }
static Value *sys_O_CREAT(Value **a, size_t c)    { (void)a;(void)c; return value_number(O_CREAT);  }
static Value *sys_O_TRUNC(Value **a, size_t c)    { (void)a;(void)c; return value_number(O_TRUNC);  }
static Value *sys_O_APPEND(Value **a, size_t c)   { (void)a;(void)c; return value_number(O_APPEND); }
static Value *sys_O_NONBLOCK(Value **a, size_t c) { (void)a;(void)c; return value_number(O_NONBLOCK); }
static Value *sys_O_SYNC(Value **a, size_t c)     { (void)a;(void)c; return value_number(O_SYNC);   }
static Value *sys_O_EXCL(Value **a, size_t c)     { (void)a;(void)c; return value_number(O_EXCL);   }

// Signal numbers — use with sys.kill() and sys.raise()
static Value *sys_SIGHUP(Value **a, size_t c)     { (void)a;(void)c; return value_number(SIGHUP);   }
static Value *sys_SIGINT(Value **a, size_t c)     { (void)a;(void)c; return value_number(SIGINT);   }
static Value *sys_SIGQUIT(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGQUIT);  }
static Value *sys_SIGKILL(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGKILL);  }
static Value *sys_SIGTERM(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGTERM);  }
static Value *sys_SIGUSR1(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGUSR1);  }
static Value *sys_SIGUSR2(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGUSR2);  }
static Value *sys_SIGCHLD(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGCHLD);  }
static Value *sys_SIGPIPE(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGPIPE);  }
static Value *sys_SIGALRM(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGALRM);  }
static Value *sys_SIGSEGV(Value **a, size_t c)    { (void)a;(void)c; return value_number(SIGSEGV);  }

// Syslog priority constants
static Value *sys_LOG_EMERG(Value **a, size_t c)   { (void)a;(void)c; return value_number(LOG_EMERG);   }
static Value *sys_LOG_ALERT(Value **a, size_t c)   { (void)a;(void)c; return value_number(LOG_ALERT);   }
static Value *sys_LOG_CRIT(Value **a, size_t c)    { (void)a;(void)c; return value_number(LOG_CRIT);    }
static Value *sys_LOG_ERR(Value **a, size_t c)     { (void)a;(void)c; return value_number(LOG_ERR);     }
static Value *sys_LOG_WARNING(Value **a, size_t c) { (void)a;(void)c; return value_number(LOG_WARNING); }
static Value *sys_LOG_NOTICE(Value **a, size_t c)  { (void)a;(void)c; return value_number(LOG_NOTICE);  }
static Value *sys_LOG_INFO(Value **a, size_t c)    { (void)a;(void)c; return value_number(LOG_INFO);    }
static Value *sys_LOG_DEBUG(Value **a, size_t c)   { (void)a;(void)c; return value_number(LOG_DEBUG);   }

// Poll event flags
static Value *sys_POLLIN(Value **a, size_t c)    { (void)a;(void)c; return value_number(POLLIN);    }
static Value *sys_POLLOUT(Value **a, size_t c)   { (void)a;(void)c; return value_number(POLLOUT);   }
static Value *sys_POLLERR(Value **a, size_t c)   { (void)a;(void)c; return value_number(POLLERR);   }
static Value *sys_POLLHUP(Value **a, size_t c)   { (void)a;(void)c; return value_number(POLLHUP);   }

// mmap prot flags
static Value *sys_PROT_READ(Value **a, size_t c)  { (void)a;(void)c; return value_number(PROT_READ);  }
static Value *sys_PROT_WRITE(Value **a, size_t c) { (void)a;(void)c; return value_number(PROT_WRITE); }
static Value *sys_PROT_EXEC(Value **a, size_t c)  { (void)a;(void)c; return value_number(PROT_EXEC);  }
static Value *sys_PROT_NONE(Value **a, size_t c)  { (void)a;(void)c; return value_number(PROT_NONE);  }
static Value *sys_MAP_SHARED(Value **a, size_t c)    { (void)a;(void)c; return value_number(MAP_SHARED);    }
static Value *sys_MAP_PRIVATE(Value **a, size_t c)   { (void)a;(void)c; return value_number(MAP_PRIVATE);   }
static Value *sys_MAP_ANONYMOUS(Value **a, size_t c) { (void)a;(void)c; return value_number(MAP_ANONYMOUS); }

// Socket domain/type constants
static Value *sys_AF_INET(Value **a, size_t c)    { (void)a;(void)c; return value_number(AF_INET);    }
static Value *sys_AF_INET6(Value **a, size_t c)   { (void)a;(void)c; return value_number(AF_INET6);   }
static Value *sys_AF_UNIX(Value **a, size_t c)    { (void)a;(void)c; return value_number(AF_UNIX);    }
static Value *sys_SOCK_STREAM(Value **a, size_t c){ (void)a;(void)c; return value_number(SOCK_STREAM); }
static Value *sys_SOCK_DGRAM(Value **a, size_t c) { (void)a;(void)c; return value_number(SOCK_DGRAM);  }

// Resource limit resource IDs
static Value *sys_RLIMIT_CPU(Value **a, size_t c)    { (void)a;(void)c; return value_number(RLIMIT_CPU);    }
static Value *sys_RLIMIT_FSIZE(Value **a, size_t c)  { (void)a;(void)c; return value_number(RLIMIT_FSIZE);  }
static Value *sys_RLIMIT_STACK(Value **a, size_t c)  { (void)a;(void)c; return value_number(RLIMIT_STACK);  }
static Value *sys_RLIMIT_NOFILE(Value **a, size_t c) { (void)a;(void)c; return value_number(RLIMIT_NOFILE); }
static Value *sys_RLIMIT_AS(Value **a, size_t c)     { (void)a;(void)c; return value_number(RLIMIT_AS);     }

// ═════════════════════════════════════════════════════════════════════════════
// LEVEL 10 — NUMERIC CONSTANTS (from C limits.h / stdint.h)
// ═════════════════════════════════════════════════════════════════════════════
static Value *sys_STDIN(Value **a, size_t c)    { (void)a;(void)c; return value_number(0); }
static Value *sys_STDOUT(Value **a, size_t c)   { (void)a;(void)c; return value_number(1); }
static Value *sys_STDERR(Value **a, size_t c)   { (void)a;(void)c; return value_number(2); }
static Value *sys_INT_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT_MAX);  }
static Value *sys_INT_MIN_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT_MIN);  }
static Value *sys_UINT_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)UINT_MAX); }
static Value *sys_LONG_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)LONG_MAX); }
static Value *sys_INT8_MAX_fn(Value **a, size_t c)   { (void)a;(void)c; return value_number((double)INT8_MAX);   }
static Value *sys_INT8_MIN_fn(Value **a, size_t c)   { (void)a;(void)c; return value_number((double)INT8_MIN);   }
static Value *sys_INT16_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT16_MAX);  }
static Value *sys_INT16_MIN_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT16_MIN);  }
static Value *sys_INT32_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT32_MAX);  }
static Value *sys_INT32_MIN_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)INT32_MIN);  }
static Value *sys_UINT8_MAX_fn(Value **a, size_t c)  { (void)a;(void)c; return value_number((double)UINT8_MAX);  }
static Value *sys_UINT16_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)UINT16_MAX); }
static Value *sys_UINT32_MAX_fn(Value **a, size_t c) { (void)a;(void)c; return value_number((double)UINT32_MAX); }

// ═════════════════════════════════════════════════════════════════════════════
// MODULE FUNCTION TABLE
// ═════════════════════════════════════════════════════════════════════════════
static NativeFunc sys_fns[] = {
    // ── LEVEL 1: Process Control ────────────────────────────────────────────
    { "exit",           sys_exit            },
    { "abort",          sys_abort           },
    { "getpid",         sys_getpid          },
    { "getppid",        sys_getppid         },
    { "fork",           sys_fork            },
    { "exec",           sys_exec            },
    { "wait",           sys_wait            },
    { "waitpid",        sys_waitpid         },
    { "kill",           sys_kill            },
    { "raise",          sys_raise           },
    { "getuid",         sys_getuid          },
    { "getgid",         sys_getgid          },
    { "geteuid",        sys_geteuid         },
    { "getegid",        sys_getegid         },
    { "getrlimit",      sys_getrlimit       },
    { "setrlimit",      sys_setrlimit       },
    // ── LEVEL 2: File Descriptor I/O & mmap ────────────────────────────────
    { "open",           sys_open            },
    { "close",          sys_close           },
    { "read",           sys_read            },
    { "write",          sys_write           },
    { "seek",           sys_seek            },
    { "dup",            sys_dup             },
    { "dup2",           sys_dup2            },
    { "pipe",           sys_pipe            },
    { "fsync",          sys_fsync           },
    { "fdatasync",      sys_fdatasync       },
    { "truncate",       sys_truncate        },
    { "ftruncate",      sys_ftruncate       },
    { "mmap",           sys_mmap            },
    { "munmap",         sys_munmap          },
    { "mmap_read",      sys_mmap_read       },
    { "mmap_write",     sys_mmap_write      },
    { "poll",           sys_poll            },
    { "fcntl_lock",     sys_fcntl_lock      },
    { "fcntl_setfl",    sys_fcntl_setfl     },
    { "fcntl_getfl",    sys_fcntl_getfl     },
    // ── LEVEL 3: Filesystem ─────────────────────────────────────────────────
    { "stat",           sys_stat            },
    { "lstat",          sys_lstat           },
    { "isfile",         sys_isfile          },
    { "isdir",          sys_isdir           },
    { "islnk",          sys_islnk           },
    { "access",         sys_access          },
    { "mkdir",          sys_mkdir           },
    { "rmdir",          sys_rmdir           },
    { "unlink",         sys_unlink          },
    { "rename",         sys_rename          },
    { "link",           sys_link            },
    { "symlink",        sys_symlink         },
    { "readlink",       sys_readlink        },
    { "chmod",          sys_chmod           },
    { "chown",          sys_chown           },
    { "getcwd",         sys_getcwd          },
    { "chdir",          sys_chdir           },
    { "mkfifo",         sys_mkfifo          },
    // ── LEVEL 4: Networking ─────────────────────────────────────────────────
    { "socket",         sys_socket          },
    { "connect",        sys_connect         },
    { "bind",           sys_bind            },
    { "listen",         sys_listen          },
    { "accept",         sys_accept          },
    { "send",           sys_send            },
    { "recv",           sys_recv            },
    { "setsockopt",     sys_setsockopt      },
    { "inet_aton",      sys_inet_aton       },
    { "inet_ntoa",      sys_inet_ntoa       },
    { "htons",          sys_htons           },
    { "ntohs",          sys_ntohs           },
    { "htonl",          sys_htonl           },
    { "ntohl",          sys_ntohl           },
    // ── LEVEL 5: Time & Clocks ──────────────────────────────────────────────
    { "clock_realtime",  sys_clock_realtime  },
    { "clock_monotonic", sys_clock_monotonic },
    { "clock_process",   sys_clock_process   },
    { "nanosleep",       sys_nanosleep       },
    { "gettimeofday",    sys_gettimeofday    },
    { "sleep",           sys_sleep_sec       },
    { "usleep",          sys_usleep          },
    { "time",            sys_time            },
    { "clock",           sys_clock_fn        },
    // ── LEVEL 6: System Info & Environment ─────────────────────────────────
    { "uname",          sys_uname           },
    { "hostname",       sys_hostname        },
    { "nproc",          sys_nproc           },
    { "nproc_conf",     sys_nproc_conf      },
    { "phys_pages",     sys_phys_pages      },
    { "avphys_pages",   sys_avphys_pages    },
    { "PAGE_SIZE",      sys_PAGE_SIZE       },
    { "endian",         sys_endian          },
    { "sizeof_ptr",     sys_sizeof_ptr      },
    { "getenv",         sys_getenv          },
    { "setenv",         sys_setenv          },
    { "unsetenv",       sys_unsetenv        },
    { "errno",          sys_errno_fn        },
    { "strerror",       sys_strerror        },
    { "system",         sys_system          },
    { "proc_status",    sys_proc_status     },
    { "proc_maps",      sys_proc_maps       },
    // ── LEVEL 7: Bit & Memory Operations ───────────────────────────────────
    { "band",           sys_band            },
    { "bor",            sys_bor             },
    { "bxor",           sys_bxor            },
    { "bnot",           sys_bnot            },
    { "shl",            sys_shl             },
    { "shr",            sys_shr             },
    { "sar",            sys_sar             },
    { "rol",            sys_rol             },
    { "ror",            sys_ror             },
    { "popcnt",         sys_popcnt          },
    { "clz",            sys_clz             },
    { "ctz",            sys_ctz             },
    { "bswap32",        sys_bswap32         },
    { "bswap64",        sys_bswap64         },
    { "parity",         sys_parity          },
    { "bit_get",        sys_bit_get         },
    { "bit_set",        sys_bit_set         },
    { "bit_clear",      sys_bit_clear       },
    { "bit_toggle",     sys_bit_toggle      },
    { "align_up",       sys_align_up        },
    { "align_down",     sys_align_down      },
    { "is_pow2",        sys_is_pow2         },
    { "next_pow2",      sys_next_pow2       },
    // ── LEVEL 8: IPC & Syslog ───────────────────────────────────────────────
    { "openlog",        sys_openlog         },
    { "syslog",         sys_syslog          },
    { "closelog",       sys_closelog        },
    // ── LEVEL 9: Open Flags & Signal Constants ──────────────────────────────
    { "O_RDONLY",       sys_O_RDONLY        },
    { "O_WRONLY",       sys_O_WRONLY        },
    { "O_RDWR",         sys_O_RDWR          },
    { "O_CREAT",        sys_O_CREAT         },
    { "O_TRUNC",        sys_O_TRUNC         },
    { "O_APPEND",       sys_O_APPEND        },
    { "O_NONBLOCK",     sys_O_NONBLOCK      },
    { "O_SYNC",         sys_O_SYNC          },
    { "O_EXCL",         sys_O_EXCL          },
    { "SIGHUP",         sys_SIGHUP          },
    { "SIGINT",         sys_SIGINT          },
    { "SIGQUIT",        sys_SIGQUIT         },
    { "SIGKILL",        sys_SIGKILL         },
    { "SIGTERM",        sys_SIGTERM         },
    { "SIGUSR1",        sys_SIGUSR1         },
    { "SIGUSR2",        sys_SIGUSR2         },
    { "SIGCHLD",        sys_SIGCHLD         },
    { "SIGPIPE",        sys_SIGPIPE         },
    { "SIGALRM",        sys_SIGALRM         },
    { "SIGSEGV",        sys_SIGSEGV         },
    { "LOG_EMERG",      sys_LOG_EMERG       },
    { "LOG_ALERT",      sys_LOG_ALERT       },
    { "LOG_CRIT",       sys_LOG_CRIT        },
    { "LOG_ERR",        sys_LOG_ERR         },
    { "LOG_WARNING",    sys_LOG_WARNING     },
    { "LOG_NOTICE",     sys_LOG_NOTICE      },
    { "LOG_INFO",       sys_LOG_INFO        },
    { "LOG_DEBUG",      sys_LOG_DEBUG       },
    { "POLLIN",         sys_POLLIN          },
    { "POLLOUT",        sys_POLLOUT         },
    { "POLLERR",        sys_POLLERR         },
    { "POLLHUP",        sys_POLLHUP         },
    { "PROT_READ",      sys_PROT_READ       },
    { "PROT_WRITE",     sys_PROT_WRITE      },
    { "PROT_EXEC",      sys_PROT_EXEC       },
    { "PROT_NONE",      sys_PROT_NONE       },
    { "MAP_SHARED",     sys_MAP_SHARED      },
    { "MAP_PRIVATE",    sys_MAP_PRIVATE     },
    { "MAP_ANONYMOUS",  sys_MAP_ANONYMOUS   },
    { "AF_INET",        sys_AF_INET         },
    { "AF_INET6",       sys_AF_INET6        },
    { "AF_UNIX",        sys_AF_UNIX         },
    { "SOCK_STREAM",    sys_SOCK_STREAM     },
    { "SOCK_DGRAM",     sys_SOCK_DGRAM      },
    { "RLIMIT_CPU",     sys_RLIMIT_CPU      },
    { "RLIMIT_FSIZE",   sys_RLIMIT_FSIZE    },
    { "RLIMIT_STACK",   sys_RLIMIT_STACK    },
    { "RLIMIT_NOFILE",  sys_RLIMIT_NOFILE   },
    { "RLIMIT_AS",      sys_RLIMIT_AS       },
    // ── LEVEL 10: Integer Constants ─────────────────────────────────────────
    { "STDIN",          sys_STDIN           },
    { "STDOUT",         sys_STDOUT          },
    { "STDERR",         sys_STDERR          },
    { "INT_MAX",        sys_INT_MAX_fn      },
    { "INT_MIN",        sys_INT_MIN_fn      },
    { "UINT_MAX",       sys_UINT_MAX_fn     },
    { "LONG_MAX",       sys_LONG_MAX_fn     },
    { "INT8_MAX",       sys_INT8_MAX_fn     },
    { "INT8_MIN",       sys_INT8_MIN_fn     },
    { "INT16_MAX",      sys_INT16_MAX_fn    },
    { "INT16_MIN",      sys_INT16_MIN_fn    },
    { "INT32_MAX",      sys_INT32_MAX_fn    },
    { "INT32_MIN",      sys_INT32_MIN_fn    },
    { "UINT8_MAX",      sys_UINT8_MAX_fn    },
    { "UINT16_MAX",     sys_UINT16_MAX_fn   },
    { "UINT32_MAX",     sys_UINT32_MAX_fn   },
    { NULL, NULL }
};

Module module_sys(void) {
    Module m;
    m.name      = NULL;
    m.functions = sys_fns;
    m.fn_count  = sizeof(sys_fns)/sizeof(sys_fns[0]) - 1;
    return m;
}
