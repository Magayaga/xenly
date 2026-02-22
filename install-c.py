#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in Python programming language.
#
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

UNAME_S = platform.system()   # 'Linux', 'Darwin', 'Windows', ...
UNAME_M = platform.machine()  # 'x86_64', 'arm64', ...

TARGET      = "xenly"
XENLYC      = "xenlyc"
RT_LIB      = "libxly_rt.a"

INTERP_SRCS = [
    "src/main.c", "src/lexer.c", "src/ast.c", "src/parser.c",
    "src/interpreter.c", "src/modules.c", "src/typecheck.c",
    "src/unicode.c", "src/multiproc.c", "src/multiproc_builtins.c",
]

XENLYC_SRCS = [
    "src/xenlyc_main.c", "src/lexer.c", "src/ast.c",
    "src/parser.c", "src/codegen.c", "src/unicode.c",
]

RT_SRCS = [
    "src/xly_rt.c",
    "src/unicode.c",
]

CC      = os.environ.get("CC", "gcc")
NO_NATIVE = os.environ.get("NO_NATIVE", "0") == "1"
DEBUG   = os.environ.get("DEBUG", "")
SANITIZE = os.environ.get("SANITIZE", "")
PREFIX  = os.environ.get("PREFIX", "/usr/local")

BASE_CFLAGS  = ["-Wall", "-Wextra", "-O3", "-std=c11",
                "-D_POSIX_C_SOURCE=200809L", "-Isrc"]
BASE_LDFLAGS = ["-lm"]

def configure_platform(cflags, ldflags):
    if UNAME_S == "Linux":
        CC_local = CC
        cflags  += ["-DPLATFORM_LINUX"]
        ldflags += ["-lpthread", "-ldl"]
        if not NO_NATIVE:
            cflags += ["-march=native", "-mtune=native", "-ffast-math"]
        if shutil.which("ld.gold"):
            ldflags += ["-fuse-ld=gold"]

    elif UNAME_S == "Darwin":
        CC_local = CC
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
        CC_local = "clang"
        cflags  += ["-DPLATFORM_FREEBSD"]
        ldflags += ["-lpthread"]
        if not NO_NATIVE:
            cflags += ["-march=native", "-mtune=native"]
        if shutil.which("ld.lld"):
            ldflags += ["-fuse-ld=lld"]

    elif UNAME_S == "OpenBSD":
        CC_local = "clang"
        cflags  = [f for f in cflags if f != "-O3"]
        cflags  += ["-DPLATFORM_OPENBSD"]
        ldflags += ["-lpthread"]

    elif UNAME_S == "NetBSD":
        CC_local = "gcc"
        cflags  += ["-DPLATFORM_NETBSD"]
        ldflags += ["-lpthread"]

    else:
        CC_local = CC
        print(f"Warning: unrecognised platform '{UNAME_S}', using defaults.")

    return CC_local, cflags, ldflags

def apply_debug_sanitize(cflags, ldflags):
    if DEBUG:
        cflags = [f for f in cflags if f not in ("-O3", "-O2")]
        cflags += ["-O0", "-g", "-DDEBUG"]
    if SANITIZE:
        cflags  += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        ldflags += ["-fsanitize=address,undefined"]
    return cflags, ldflags

def run(cmd, check=True):
    """Run a command list, printing it first."""
    print("  " + " ".join(cmd))
    result = subprocess.run(cmd)
    if check and result.returncode != 0:
        print(f"✗ Command failed (exit {result.returncode})")
        sys.exit(result.returncode)
    return result.returncode

def compile_objs(srcs, cc, cflags, extra_defines=None):
    """Compile a list of .c files to .o files; return list of object paths."""
    objs = []
    for src in srcs:
        obj = src.replace(".c", ".o")
        extra = extra_defines or []
        run([cc] + cflags + extra + ["-c", "-o", obj, src])
        objs.append(obj)
    return objs

def compile_modules_rt(cc, cflags):
    """Compile src/modules.c without multiprocessing for the runtime library."""
    obj = "src/modules_rt.o"
    print("  Compiling src/modules.c (runtime, no multiprocessing)...")
    run([cc] + cflags + ["-DXENLY_NO_MULTIPROC", "-c", "-o", obj, "src/modules.c"])
    return obj

def print_banner():
    print(f"Building Xenly for {UNAME_S} {UNAME_M}")
    print(f"Compiler: {CC}")
    print()

def build_interpreter(cc, cflags, ldflags):
    print(f"==> Building interpreter: {TARGET}")
    objs = compile_objs(INTERP_SRCS, cc, cflags)
    print("Linking interpreter...")
    run([cc] + cflags + ["-o", TARGET] + objs + ldflags)
    print(f"  Interpreter: {TARGET}")

def build_compiler(cc, cflags, ldflags):
    print(f"==> Building compiler driver: {XENLYC}")
    objs = compile_objs(XENLYC_SRCS, cc, cflags)
    print("Linking compiler...")
    run([cc] + cflags + ["-o", XENLYC] + objs + ldflags)
    print(f"  Compiler: {XENLYC}")

def build_runtime(cc, cflags):
    print(f"==> Building runtime library: {RT_LIB}")
    rt_objs    = compile_objs(RT_SRCS, cc, cflags)
    modules_rt = compile_modules_rt(cc, cflags)
    print("Creating runtime library...")
    run(["ar", "rcs", RT_LIB] + rt_objs + [modules_rt])
    print(f"  Runtime: {RT_LIB}")

def build_all(cc, cflags, ldflags):
    build_interpreter(cc, cflags, ldflags)
    build_compiler(cc, cflags, ldflags)
    build_runtime(cc, cflags)
    print()
    print("✓ Build complete!")
    print(f"  Interpreter: {TARGET}")
    print(f"  Compiler:    {XENLYC}")
    print(f"  Runtime:     {RT_LIB}")
    print()

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
    for pattern in ["**/*~", "**/*.swp", "**/.DS_Store"]:
        for f in Path(".").rglob(pattern.lstrip("**/")):
            f.unlink(missing_ok=True)

def cmd_install(cc, cflags, ldflags):
    build_all(cc, cflags, ldflags)
    bindir  = Path(PREFIX) / "bin"
    libdir  = Path(PREFIX) / "lib"
    incdir  = Path(PREFIX) / "include" / "xenly"
    docdir  = Path(PREFIX) / "share" / "doc" / "xenly"
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
    print("==> Building macOS universal binary...")
    cmd_clean()
    uni_cflags = [f for f in cflags if f not in ("-arch", "arm64", "x86_64")]
    uni_cflags += ["-arch", "x86_64", "-arch", "arm64"]
    build_all("clang", uni_cflags, ldflags)
    run(["file", TARGET, XENLYC])

def cmd_test(cc, cflags, ldflags):
    build_all(cc, cflags, ldflags)
    print("Running Xenly Test Suite")
    if run([f"./{TARGET}", "examples/hello.xe"], check=False) != 0:
        print("✗ Interpreter test failed"); sys.exit(1)
    print("✓ Interpreter works")
    if run([f"./{XENLYC}", "examples/hello.xe", "-o", "test_compiled"], check=False) != 0:
        print("✗ Compilation failed"); sys.exit(1)
    if run(["./test_compiled"], check=False) != 0:
        print("✗ Compiled binary failed"); sys.exit(1)
    Path("test_compiled").unlink(missing_ok=True)
    print("✓ All tests passed!")

def cmd_help():
    print("Xenly Build System")
    print(f"Usage: python {sys.argv[0]} [command]")
    print()
    print("Commands: all  run  compile  clean  distclean  install  uninstall  universal  test  help")
    print()
    print("Environment variables:")
    print("  CC=<compiler>    Compiler to use (default: gcc)")
    print("  PREFIX=<path>    Install prefix (default: /usr/local)")
    print("  DEBUG=1          Debug build (no optimisation, -g)")
    print("  SANITIZE=1       Enable address & UB sanitizers")
    print("  NO_NATIVE=1      Disable -march=native / CPU-specific tuning")

def main():
    command = sys.argv[1] if len(sys.argv) > 1 else "all"

    # Commands that don't need compiler setup
    if command == "clean":
        cmd_clean(); return
    if command == "distclean":
        cmd_distclean(); return
    if command == "uninstall":
        cmd_uninstall(); return
    if command == "help":
        cmd_help(); return

    print_banner()

    cc, cflags, ldflags = configure_platform(list(BASE_CFLAGS), list(BASE_LDFLAGS))
    cflags, ldflags     = apply_debug_sanitize(cflags, ldflags)

    dispatch = {
        "all":       lambda: cmd_all(cc, cflags, ldflags),
        "run":       lambda: cmd_run(cc, cflags, ldflags),
        "compile":   lambda: cmd_compile(cc, cflags, ldflags),
        "install":   lambda: cmd_install(cc, cflags, ldflags),
        "universal": lambda: cmd_universal(cflags, ldflags),
        "test":      lambda: cmd_test(cc, cflags, ldflags),
    }

    if command not in dispatch:
        print(f"Unknown command: '{command}'")
        cmd_help()
        sys.exit(1)

    dispatch[command]()

if __name__ == "__main__":
    main()
