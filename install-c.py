#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Python programming language.
#
# install-c.py — Xenly build script (Python 3.6+)
#
# Mirrors the logic of makefile and main.sh; use whichever suits your
# workflow.  Supports all the same commands and environment variables.
#
# New in this build:
#   • xenly_linker.c added to XENLYC_SRCS and RT_SRCS
#     (built-in 20× faster ELF/Mach-O linker; replaces gcc/clang link step)
#   • xly_http.c / xly_http.h added to RT_SRCS
#     (zero-dependency HTTP/1.1 web server for Linux and macOS)
#   • "unicode" module auto-imported (no explicit import needed)
#   • New commands: test-http, linker-version
#   • build_runtime() now compiles xly_http.o and xenly_linker.o into libxly_rt.a
#
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

UNAME_S = platform.system()   # 'Linux', 'Darwin', 'Windows', ...
UNAME_M = platform.machine()  # 'x86_64', 'arm64', ...

TARGET  = "xenly"
XENLYC  = "xenlyc"
RT_LIB  = "libxly_rt.a"

# ── Source lists ──────────────────────────────────────────────────────────────

INTERP_SRCS = [
    "src/main.c", "src/lexer.c", "src/ast.c", "src/parser.c",
    "src/interpreter.c", "src/modules.c", "src/typecheck.c",
    "src/unicode.c", "src/multiproc.c", "src/multiproc_builtins.c",
]

# xenly_linker.c provides the in-process xlnk linker (20× faster than
# spawning gcc/clang).  Must be in XENLYC_SRCS so xenlyc links correctly.
XENLYC_SRCS = [
    "src/xenlyc_main.c", "src/lexer.c", "src/ast.c",
    "src/parser.c", "src/codegen.c", "src/unicode.c", "src/sema.c",
    "src/xenly_linker.c",
]

# Runtime library sources.
# xly_http.c   — HTTP/1.1 server (Linux epoll / macOS kqueue)
# xenly_linker.c — in-process ELF64/Mach-O linker
RT_SRCS = [
    "src/xly_rt.c",
    "src/unicode.c",
    "src/xly_http.c",
    "src/xenly_linker.c",
]

# Sources compiled with -DXENLY_NO_MULTIPROC for the runtime library
RT_NOMP_SRCS = [
    ("src/modules.c",            "src/modules_rt.o"),
    ("src/multiproc.c",          "src/multiproc_rt.o"),
    ("src/multiproc_builtins.c", "src/multiproc_builtins_rt.o"),
]

# ── Environment variables ─────────────────────────────────────────────────────

CC        = os.environ.get("CC", "gcc")
NO_NATIVE = os.environ.get("NO_NATIVE", "0") == "1"
DEBUG     = os.environ.get("DEBUG", "")
SANITIZE  = os.environ.get("SANITIZE", "")
PREFIX    = os.environ.get("PREFIX", "/usr/local")

BASE_CFLAGS  = ["-Wall", "-Wextra", "-O3", "-std=c11",
                "-D_POSIX_C_SOURCE=200809L", "-Isrc"]
BASE_LDFLAGS = ["-lm"]

# ── Platform configuration ────────────────────────────────────────────────────

def configure_platform(cflags, ldflags):
    cc = CC

    if UNAME_S == "Linux":
        cflags  += ["-DPLATFORM_LINUX"]
        ldflags += ["-lpthread", "-ldl", "-lresolv"]
        if not NO_NATIVE:
            cflags += ["-march=native", "-mtune=native", "-ffast-math"]
        if shutil.which("ld.gold"):
            ldflags += ["-fuse-ld=gold"]

    elif UNAME_S == "Darwin":
        cflags  += ["-DPLATFORM_MACOS"]
        ldflags += ["-lpthread"]
        if UNAME_M == "arm64":
            if not NO_NATIVE:
                cflags += ["-mcpu=apple-m1", "-mtune=apple-m1"]
            cflags += ["-arch", "arm64"]
        else:
            if not NO_NATIVE:
                cflags += ["-march=native", "-mtune=native"]
            cflags += ["-arch", "x86_64"]
        cflags += ["-mmacosx-version-min=10.13"]

    elif UNAME_S == "FreeBSD":
        cc = "clang"
        cflags  += ["-DPLATFORM_FREEBSD"]
        ldflags += ["-lpthread"]
        if not NO_NATIVE:
            cflags += ["-march=native", "-mtune=native"]
        if shutil.which("ld.lld"):
            ldflags += ["-fuse-ld=lld"]

    elif UNAME_S == "OpenBSD":
        cc = "clang"
        cflags  = [f for f in cflags if f != "-O3"]
        cflags  += ["-DPLATFORM_OPENBSD"]
        ldflags += ["-lpthread"]

    elif UNAME_S == "NetBSD":
        cc = "gcc"
        cflags  += ["-DPLATFORM_NETBSD"]
        ldflags += ["-lpthread"]

    else:
        print(f"Warning: unrecognised platform '{UNAME_S}', using defaults.")

    return cc, cflags, ldflags

def apply_debug_sanitize(cflags, ldflags):
    if DEBUG:
        cflags = [f for f in cflags if f not in ("-O3", "-O2")]
        cflags += ["-O0", "-g", "-DDEBUG"]
    if SANITIZE:
        cflags  += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        ldflags += ["-fsanitize=address,undefined"]
    return cflags, ldflags

# ── Build helpers ─────────────────────────────────────────────────────────────

def run(cmd, check=True):
    """Run a command list, printing it first."""
    print("  " + " ".join(cmd))
    result = subprocess.run(cmd)
    if check and result.returncode != 0:
        print(f"✗ Command failed (exit {result.returncode})")
        sys.exit(result.returncode)
    return result.returncode

def compile_obj(src, obj, cc, cflags, extra=None):
    """Compile one .c file to one .o file."""
    cmd = [cc] + cflags + (extra or []) + ["-c", "-o", obj, src]
    print(f"  Compiling {src}...")
    run(cmd)

def compile_objs(srcs, cc, cflags, extra=None):
    """Compile a list of .c files; return list of .o paths."""
    objs = []
    for src in srcs:
        obj = src.replace(".c", ".o")
        compile_obj(src, obj, cc, cflags, extra)
        objs.append(obj)
    return objs

def print_banner():
    print("╔════════════════════════════════════════════════════════╗")
    print(f"║  Building Xenly for {UNAME_S} {UNAME_M}")
    print(f"║  Compiler: {CC}")
    print("╚════════════════════════════════════════════════════════╝")
    print()

# ── Build targets ─────────────────────────────────────────────────────────────

def build_interpreter(cc, cflags, ldflags):
    print(f"==> Building interpreter: {TARGET}")
    objs = compile_objs(INTERP_SRCS, cc, cflags)
    print("Linking interpreter...")
    run([cc] + ["-o", TARGET] + objs + ldflags)
    print(f"  Interpreter: {TARGET}")

def build_compiler(cc, cflags, ldflags):
    print(f"==> Building compiler driver: {XENLYC}")
    print("    (includes xenly_linker.c — in-process 20× faster linker)")
    objs = compile_objs(XENLYC_SRCS, cc, cflags)
    print("Linking compiler (with built-in xlnk linker)...")
    run([cc] + ["-o", XENLYC] + objs + ldflags)
    print(f"  Compiler: {XENLYC}")

def build_runtime(cc, cflags):
    print(f"==> Building runtime library: {RT_LIB}")
    print("    (includes xly_http.c — HTTP/1.1 server for Linux & macOS)")
    # Standard runtime sources
    rt_objs = compile_objs(RT_SRCS, cc, cflags)
    # Sources that need -DXENLY_NO_MULTIPROC
    for src, obj in RT_NOMP_SRCS:
        compile_obj(src, obj, cc, cflags, extra=["-DXENLY_NO_MULTIPROC"])
        rt_objs.append(obj)
    print("Creating runtime library...")
    run(["ar", "rcs", RT_LIB] + rt_objs)
    print(f"  Runtime: {RT_LIB}")

def build_all(cc, cflags, ldflags):
    build_interpreter(cc, cflags, ldflags)
    build_compiler(cc, cflags, ldflags)
    build_runtime(cc, cflags)
    print()
    print("✓ Build complete!")
    print(f"  Interpreter: {TARGET}")
    print(f"  Compiler:    {XENLYC}  (xlnk built-in linker v1.0.0)")
    print(f"  Runtime:     {RT_LIB}  (includes xly_http HTTP server)")
    print()

# ── Commands ──────────────────────────────────────────────────────────────────

def cmd_all(cc, cflags, ldflags):
    build_all(cc, cflags, ldflags)

def cmd_run(cc, cflags, ldflags):
    build_interpreter(cc, cflags, ldflags)
    run([f"./{TARGET}", "examples/hello.xe"])

def cmd_compile(cc, cflags, ldflags):
    build_compiler(cc, cflags, ldflags)
    build_runtime(cc, cflags)
    run([f"./{XENLYC}", "examples/hello.xe", "-o", "hello_compiled"])
    run(["./hello_compiled"])
    Path("hello_compiled").unlink(missing_ok=True)

def cmd_test(cc, cflags, ldflags):
    build_all(cc, cflags, ldflags)
    print("╔════════════════════════════════════════════════════════╗")
    print("║         Running Xenly Test Suite                       ║")
    print("╚════════════════════════════════════════════════════════╝")
    if run([f"./{TARGET}", "examples/hello.xe"], check=False) != 0:
        print("✗ Interpreter test failed"); sys.exit(1)
    print("✓ Interpreter works")
    if run([f"./{XENLYC}", "examples/hello.xe", "-o", "test_compiled"], check=False) != 0:
        print("✗ Compilation failed"); sys.exit(1)
    if run(["./test_compiled"], check=False) != 0:
        print("✗ Compiled binary failed"); sys.exit(1)
    Path("test_compiled").unlink(missing_ok=True)
    print("✓ All tests passed!")

def cmd_test_sys(cc, cflags, ldflags):
    build_interpreter(cc, cflags, ldflags)
    print("╔════════════════════════════════════════════════════════╗")
    print("║         Running sys Module Test                        ║")
    print("╚════════════════════════════════════════════════════════╝")
    if run([f"./{TARGET}", "examples/sys_demo.xe"], check=False) != 0:
        print("✗ sys module test failed"); sys.exit(1)
    print("✓ sys module test passed!")

def cmd_test_http(cc, cflags, ldflags):
    """Quick HTTP server smoke-test: starts server, curls /, stops."""
    build_all(cc, cflags, ldflags)
    print("╔════════════════════════════════════════════════════════╗")
    print("║         Running HTTP Server Smoke Test                 ║")
    print("╚════════════════════════════════════════════════════════╝")
    # The test script starts the server in the background, curls it, checks.
    if not Path("examples/http_test.xe").exists():
        print("⚠  examples/http_test.xe not found — skipping HTTP test")
        return
    import threading, time
    srv = subprocess.Popen([f"./{TARGET}", "examples/http_test.xe"])
    time.sleep(0.5)
    rc = run(["curl", "-sf", "http://localhost:8080/ping"], check=False)
    srv.terminate(); srv.wait()
    if rc != 0:
        print("✗ HTTP server smoke test failed"); sys.exit(1)
    print("✓ HTTP server smoke test passed!")

def cmd_linker_version(cc, cflags, ldflags):
    """Print the built-in xlnk linker version."""
    build_compiler(cc, cflags, ldflags)
    run([f"./{XENLYC}", "--linker-version"])

def cmd_clean():
    print("Cleaning build artifacts...")
    for pattern in ["src/*.o", "src/*.d"]:
        for f in Path(".").glob(pattern):
            f.unlink()
    for name in [TARGET, XENLYC, RT_LIB, "a.out", "hello_compiled", "test_compiled"]:
        Path(name).unlink(missing_ok=True)
    for pattern in ["*.s", "*.o", "*.d"]:
        for f in Path(".").glob(pattern):
            f.unlink()
    print("✓ Clean complete")

def cmd_distclean():
    cmd_clean()
    for pattern in ["*~", "*.swp", ".DS_Store"]:
        for f in Path(".").rglob(pattern):
            f.unlink(missing_ok=True)

def cmd_install(cc, cflags, ldflags):
    build_all(cc, cflags, ldflags)
    bindir = Path(PREFIX) / "bin"
    libdir = Path(PREFIX) / "lib"
    incdir = Path(PREFIX) / "include" / "xenly"
    docdir = Path(PREFIX) / "share" / "doc" / "xenly"
    for d in (bindir, libdir, incdir, docdir):
        d.mkdir(parents=True, exist_ok=True)
    for exe in (TARGET, XENLYC):
        shutil.copy2(exe, bindir / exe)
        (bindir / exe).chmod(0o755)
    shutil.copy2(RT_LIB, libdir / RT_LIB)
    for h in Path("src").glob("*.h"):
        shutil.copy2(h, incdir / h.name)
    for md in Path(".").glob("*.md"):
        shutil.copy2(md, docdir / md.name)
    print(f"✓ Installed to {PREFIX}")

def cmd_uninstall():
    bindir = Path(PREFIX) / "bin"
    libdir = Path(PREFIX) / "lib"
    incdir = Path(PREFIX) / "include" / "xenly"
    docdir = Path(PREFIX) / "share" / "doc" / "xenly"
    for exe in (TARGET, XENLYC):
        (bindir / exe).unlink(missing_ok=True)
    (libdir / RT_LIB).unlink(missing_ok=True)
    shutil.rmtree(incdir, ignore_errors=True)
    shutil.rmtree(docdir, ignore_errors=True)
    print(f"✓ Uninstalled from {PREFIX}")

def cmd_universal(cflags, ldflags):
    if UNAME_S != "Darwin":
        print("Error: universal target is macOS-only.", file=sys.stderr)
        sys.exit(1)
    print("==> Building macOS universal binary (x86_64 + arm64)...")
    cmd_clean()
    env_x86 = {**os.environ, "ARCH_FLAG": "-arch x86_64", "NO_NATIVE": "1",
                "TARGET": "xenly_x86", "XENLYC": "xenlyc_x86"}
    subprocess.run([sys.executable, __file__, "all"], env=env_x86, check=True)
    cmd_clean()
    env_arm = {**os.environ, "ARCH_FLAG": "-arch arm64",  "NO_NATIVE": "1",
                "TARGET": "xenly_arm", "XENLYC": "xenlyc_arm"}
    subprocess.run([sys.executable, __file__, "all"], env=env_arm, check=True)
    run(["lipo", "-create", "-output", TARGET,  "xenly_x86",  "xenly_arm"])
    run(["lipo", "-create", "-output", XENLYC, "xenlyc_x86", "xenlyc_arm"])
    for f in ("xenly_x86", "xenly_arm", "xenlyc_x86", "xenlyc_arm"):
        Path(f).unlink(missing_ok=True)
    run(["file", TARGET, XENLYC])
    print("✓ Universal binaries created")

def cmd_format():
    if shutil.which("clang-format"):
        print("Formatting source files...")
        run(["clang-format", "-i"] + list(map(str, Path("src").glob("*.c")))
            + list(map(str, Path("src").glob("*.h"))))
        print("✓ Formatting complete")
    else:
        print("⚠  clang-format not found — install it or run manually")

def cmd_help():
    print()
    print("Xenly Build System  (install-c.py)")
    print(f"Usage: python {sys.argv[0]} [command]")
    print()
    print("  Commands:")
    print("    all             Build interpreter, compiler, and runtime (default)")
    print("    run             Build and run examples/hello.xe")
    print("    compile         Build and test the native compiler (xenlyc)")
    print("    test            Run the core test suite")
    print("    test-sys        Run the sys module demo")
    print("    test-http       HTTP server smoke test (requires curl)")
    print("    linker-version  Print xlnk built-in linker version")
    print("    format          Auto-format all C source with clang-format")
    print("    clean           Remove all build artifacts")
    print("    distclean       Clean + remove editor temp files")
    print("    install         Install to PREFIX (default: /usr/local)")
    print("    uninstall       Remove installed files")
    print("    universal       [macOS only] Build x86_64 + arm64 fat binary")
    print("    help            Show this message")
    print()
    print("  Environment variables:")
    print("    CC=<compiler>    Compiler to use (default: gcc)")
    print("    PREFIX=<path>    Install prefix (default: /usr/local)")
    print("    DEBUG=1          Debug build (-O0 -g)")
    print("    SANITIZE=1       Enable ASan + UBSan")
    print("    NO_NATIVE=1      Disable -march=native (for cross-compiling)")
    print()
    print("  New source files:")
    print("    src/xenly_linker.c  — in-process ELF/Mach-O linker (20× faster)")
    print("    src/xly_http.c      — HTTP/1.1 server (Linux & macOS)")
    print("    src/xly_http.h      — HTTP server public API")
    print("    src/unicode.c       — comprehensive Unicode support")
    print()

# ── Dispatch ──────────────────────────────────────────────────────────────────

def main():
    command = sys.argv[1] if len(sys.argv) > 1 else "all"

    # Commands that don't need compiler setup
    if command == "clean":
        cmd_clean(); return
    if command == "distclean":
        cmd_distclean(); return
    if command == "uninstall":
        cmd_uninstall(); return
    if command in ("help", "--help", "-h"):
        cmd_help(); return
    if command == "format":
        cmd_format(); return

    print_banner()
    cc, cflags, ldflags = configure_platform(list(BASE_CFLAGS), list(BASE_LDFLAGS))
    cflags, ldflags     = apply_debug_sanitize(cflags, ldflags)

    dispatch = {
        "all":            lambda: cmd_all(cc, cflags, ldflags),
        "run":            lambda: cmd_run(cc, cflags, ldflags),
        "compile":        lambda: cmd_compile(cc, cflags, ldflags),
        "test":           lambda: cmd_test(cc, cflags, ldflags),
        "test-sys":       lambda: cmd_test_sys(cc, cflags, ldflags),
        "test-http":      lambda: cmd_test_http(cc, cflags, ldflags),
        "linker-version": lambda: cmd_linker_version(cc, cflags, ldflags),
        "install":        lambda: cmd_install(cc, cflags, ldflags),
        "universal":      lambda: cmd_universal(cflags, ldflags),
    }

    if command not in dispatch:
        print(f"Unknown command: '{command}'")
        cmd_help()
        sys.exit(1)

    dispatch[command]()

if __name__ == "__main__":
    main()
