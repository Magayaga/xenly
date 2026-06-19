#!/usr/bin/env python3
# ═══════════════════════════════════════════════════════════════════════════
# XVM — Xenly Virtual Machine Build System (Python Version)
#
# Created, designed, and developed by Cyril John Magayaga
# Available for Windows, macOS, and Linux
# ═══════════════════════════════════════════════════════════════════════════

import os
import sys
import subprocess
import platform
import shutil

BINDIR = "bin"
XENLYBYC = os.path.join(BINDIR, "xenlybyc")
XENLYRUN = os.path.join(BINDIR, "xenlyrun")
XENLYIMG = os.path.join(BINDIR, "xenlyimg")

if os.name == "nt":
    XENLYBYC += ".exe"
    XENLYRUN += ".exe"
    XENLYIMG += ".exe"

LDFLAGS = ["-ldflags", "-s -w"]

def run(cmd, env=None):
    print(" ".join(cmd))
    subprocess.check_call(cmd, env=env)

def ensure_bindir():
    os.makedirs(BINDIR, exist_ok=True)

# ── Build ─────────────────────────────────────────────

def build_xenlybyc():
    ensure_bindir()
    print("Building xenlybyc...")
    run(["go", "build", *LDFLAGS, "-o", XENLYBYC, "./xenlybyc/"])
    print("✓", XENLYBYC)

def build_xenlyrun():
    ensure_bindir()
    print("Building xenlyrun...")
    run(["go", "build", *LDFLAGS, "-o", XENLYRUN, "./xenlyrun/"])
    print("✓", XENLYRUN)

def build_xenlyimg():
    ensure_bindir()
    print("Building xenlyimg...")
    run(["go", "build", *LDFLAGS, "-o", XENLYIMG, "./xenlyimg/"])
    print("✓", XENLYIMG)

def build():
    build_xenlybyc()
    build_xenlyrun()
    build_xenlyimg()
    print("\n✓ XVM build complete")
    print("Bytecode compiler :", XENLYBYC)
    print("VM launcher       :", XENLYRUN)
    print("Native image AOT  :", XENLYIMG)

# ── Cross Compile ─────────────────────────────────────

def cross_compile(goos, goarch):
    ensure_bindir()

    suffix = f"{goos}-{goarch}"
    exe = ".exe" if goos == "windows" else ""

    env = os.environ.copy()
    env["GOOS"] = goos
    env["GOARCH"] = goarch

    run(["go", "build", *LDFLAGS, "-o", f"{BINDIR}/xenlybyc-{suffix}{exe}", "./xenlybyc/"], env=env)
    run(["go", "build", *LDFLAGS, "-o", f"{BINDIR}/xenlyrun-{suffix}{exe}", "./xenlyrun/"], env=env)
    run(["go", "build", *LDFLAGS, "-o", f"{BINDIR}/xenlyimg-{suffix}{exe}", "./xenlyimg/"], env=env)

def cross_all():
    targets = [
        ("linux","amd64"),
        ("linux","arm64"),
        ("darwin","amd64"),
        ("darwin","arm64"),
        ("windows","amd64"),
    ]

    for goos, goarch in targets:
        cross_compile(goos, goarch)

    print("✓ All cross-compile targets built")

# ── Test ──────────────────────────────────────────────

def test():
    build()

    testfile = "/tmp/xvm_test.xe"
    bytecode = "/tmp/xvm_test.xebc"

    print("Running XVM test suite")

    with open(testfile,"w") as f:
        f.write('print("Hello, World!")')

    run([XENLYBYC, testfile, "-o", bytecode])
    print("✓ xenlybyc compiled hello")

    run([XENLYRUN, bytecode])
    print("✓ xenlyrun executed hello")

    native = "/tmp/xvm_test_native" + (".exe" if os.name == "nt" else "")
    run([XENLYIMG, bytecode, "-o", native])
    run([native])
    print("✓ xenlyimg built and executed native hello")

    os.remove(testfile)
    os.remove(bytecode)
    if os.path.exists(native):
        os.remove(native)

    run(["go","test","./..."])
    print("✓ All tests passed")

# ── Code Quality ──────────────────────────────────────

def fmt():
    run(["go","fmt","./..."])

def vet():
    run(["go","vet","./..."])

# ── Clean ─────────────────────────────────────────────

def clean():
    print("Cleaning...")
    shutil.rmtree(BINDIR, ignore_errors=True)

    for f in os.listdir("."):
        if f.endswith(".xebc"):
            os.remove(f)

    print("✓ Clean complete")

# ── Help ──────────────────────────────────────────────

def help():
    print("""
XVM — Xenly Virtual Machine Build System

Commands:

  build        Build xenlybyc + xenlyrun
  xenlybyc     Build bytecode compiler only
  xenlyrun     Build VM launcher only
  xenlyimg     Build native image builder only
  test         Build and run test suite
  fmt          Format Go source
  vet          Run go vet
  cross-all    Cross compile for all platforms
  clean        Remove build artifacts
  help         Show this help

Examples:

  python build.py build
  python build.py test
  python build.py cross-all
""")

# ── CLI ───────────────────────────────────────────────

commands = {
    "build": build,
    "xenlybyc": build_xenlybyc,
    "xenlyrun": build_xenlyrun,
    "xenlyimg": build_xenlyimg,
    "cross-all": cross_all,
    "test": test,
    "fmt": fmt,
    "vet": vet,
    "clean": clean,
    "help": help
}

if __name__ == "__main__":
    if len(sys.argv) < 2:
        build()
    else:
        cmd = sys.argv[1]
        commands.get(cmd, help)()
