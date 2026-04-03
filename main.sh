#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
#
# It is initially written in Bash Script.
#
#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# Xenly Language Build Script
# ═══════════════════════════════════════════════════════════════════════════
#
# New in this build:
#   • xenly_linker.c added to XENLYC_SRCS and RT_SRCS
#     (built-in 20× faster ELF/Mach-O linker; replaces gcc/clang link step)
#   • xly_http.c added to RT_SRCS
#     (zero-dependency HTTP/1.1 web server for Linux epoll / macOS kqueue)
#   • "unicode" module auto-imported by the interpreter (no import needed)
#   • New commands: test-http, linker-version
#   • build_runtime() now compiles xly_http.o and xenly_linker.o into libxly_rt.a
#
# ─────────────────────────────────────────────────────────────────────────────

set -euo pipefail

# ─── Platform Detection ──────────────────────────────────────────────────────
UNAME_S="$(uname -s)"
UNAME_M="$(uname -m)"

# ─── Compiler Selection ──────────────────────────────────────────────────────
CC="${CC:-gcc}"

# ─── Base Compiler Flags ─────────────────────────────────────────────────────
CFLAGS="-Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L -Isrc -MMD -MP"
LDFLAGS="-lm"

# ─── Platform-Specific Configuration ─────────────────────────────────────────
# ARCH_FLAG may be pre-set by the caller (e.g. the universal build).
# Only auto-detect it if not already set in the environment.
ARCH_FLAG="${ARCH_FLAG:-}"

case "$UNAME_S" in
  Linux)
    CFLAGS="$CFLAGS -DPLATFORM_LINUX"
    LDFLAGS="$LDFLAGS -lpthread -ldl -lresolv"
    if [[ "${NO_NATIVE:-0}" != "1" ]]; then
      CFLAGS="$CFLAGS -march=native -mtune=native -ffast-math"
    fi
    if command -v ld.gold &>/dev/null; then
      LDFLAGS="$LDFLAGS -fuse-ld=gold"
    fi
    ;;
  Darwin)
    CFLAGS="$CFLAGS -DPLATFORM_MACOS"
    LDFLAGS="$LDFLAGS -lpthread"
    # Only set ARCH_FLAG from uname if the caller didn't already provide one.
    if [[ -z "$ARCH_FLAG" ]]; then
      if [[ "$UNAME_M" == "arm64" ]]; then
        ARCH_FLAG="-arch arm64"
      else
        ARCH_FLAG="-arch x86_64"
      fi
    fi
    # Only add -march/-mcpu when not cross-compiling (NO_NATIVE=1 is set for universal).
    if [[ "${NO_NATIVE:-0}" != "1" ]]; then
      if [[ "$UNAME_M" == "arm64" ]]; then
        CFLAGS="$CFLAGS -mcpu=apple-m1 -mtune=apple-m1"
      else
        CFLAGS="$CFLAGS -march=native -mtune=native"
      fi
    fi
    CFLAGS="$CFLAGS $ARCH_FLAG -mmacosx-version-min=10.13"
    ;;
  FreeBSD)
    CC="clang"
    CFLAGS="$CFLAGS -DPLATFORM_FREEBSD"
    LDFLAGS="$LDFLAGS -lpthread"
    if [[ "${NO_NATIVE:-0}" != "1" ]]; then
      CFLAGS="$CFLAGS -march=native -mtune=native"
    fi
    if command -v ld.lld &>/dev/null; then
      LDFLAGS="$LDFLAGS -fuse-ld=lld"
    fi
    ;;
  OpenBSD)
    CC="clang"
    CFLAGS="$CFLAGS -DPLATFORM_OPENBSD"
    LDFLAGS="$LDFLAGS -lpthread"
    ;;
  NetBSD)
    CC="gcc"
    CFLAGS="$CFLAGS -DPLATFORM_NETBSD"
    LDFLAGS="$LDFLAGS -lpthread"
    ;;
esac

# ─── Debug Build Support ─────────────────────────────────────────────────────
if [[ -n "${DEBUG:-}" ]]; then
  CFLAGS="${CFLAGS//-O3/}"
  CFLAGS="${CFLAGS//-O2/}"
  CFLAGS="$CFLAGS -O0 -g -DDEBUG"
fi

# ─── Sanitizer Support ───────────────────────────────────────────────────────
if [[ -n "${SANITIZE:-}" ]]; then
  CFLAGS="$CFLAGS -fsanitize=address,undefined -fno-omit-frame-pointer"
  LDFLAGS="$LDFLAGS -fsanitize=address,undefined"
fi

# ─── Targets & Paths ─────────────────────────────────────────────────────────
TARGET="${TARGET:-xenly}"
XENLYC="${XENLYC:-xenlyc}"
RT_LIB="libxly_rt.a"
PREFIX="${PREFIX:-/usr/local}"
BINDIR="$PREFIX/bin"
LIBDIR="$PREFIX/lib"
INCDIR="$PREFIX/include"
DATADIR="$PREFIX/share"

# ─── Build Banner ────────────────────────────────────────────────────────────
echo "╔════════════════════════════════════════════════════════╗"
echo "║  Building Xenly for $UNAME_S $UNAME_M"
echo "║  Compiler: $CC"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# ─── Source Lists ────────────────────────────────────────────────────────────

INTERP_SRCS=(
  src/main.c src/lexer.c src/ast.c src/parser.c
  src/interpreter.c src/modules.c src/typecheck.c
  src/unicode.c src/multiproc.c src/multiproc_builtins.c
)

# xenly_linker.c: in-process ELF/Mach-O linker (xlnk, 20× faster than gcc/ld)
# Must be in XENLYC_SRCS — provides all xlnk_* symbols referenced by xenlyc_main.c
XENLYC_SRCS=(
  src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c
  src/codegen.c src/unicode.c src/sema.c
  src/xenly_linker.c
)

# Runtime library objects:
#   xly_rt.o           — Xenly value ABI + arithmetic + I/O
#   modules_rt.o       — stdlib modules (no multiprocessing)
#   unicode.o          — UTF-8/UTF-16/grapheme/normalization
#   multiproc_rt.o     — multiprocessing stub (no-op in runtime)
#   multiproc_builtins_rt.o
#   xly_http.o         — HTTP/1.1 server (Linux epoll / macOS kqueue)
#   xenly_linker.o     — in-process ELF/Mach-O linker for xenlyc
RT_OBJS=(
  src/xly_rt.o
  src/modules_rt.o
  src/unicode.o
  src/multiproc_rt.o
  src/multiproc_builtins_rt.o
  src/xly_http.o
  src/xenly_linker.o
)

# ─── Helper: compile one .c → .o ─────────────────────────────────────────────
compile_obj() {
  local src="$1" obj="$2" extra_flags="${3:-}"
  echo "Compiling $src..."
  # shellcheck disable=SC2086
  $CC $CFLAGS $extra_flags -c -o "$obj" "$src"
}

# ─── Build Functions ──────────────────────────────────────────────────────────

build_interpreter() {
  local objs=()
  for src in "${INTERP_SRCS[@]}"; do
    local obj="${src%.c}.o"
    compile_obj "$src" "$obj"
    objs+=("$obj")
  done
  echo "Linking interpreter..."
  # shellcheck disable=SC2086
  $CC -o "$TARGET" "${objs[@]}" $LDFLAGS
  echo "  Interpreter: $TARGET"
}

build_compiler() {
  echo "  (includes xenly_linker.c — in-process 20× faster linker)"
  local objs=()
  for src in "${XENLYC_SRCS[@]}"; do
    local obj="${src%.c}.o"
    compile_obj "$src" "$obj"
    objs+=("$obj")
  done
  echo "Linking compiler..."
  # shellcheck disable=SC2086
  $CC -o "$XENLYC" "${objs[@]}" $LDFLAGS
  echo "  Compiler: $XENLYC"
}

build_runtime() {
  echo "  (includes xly_http.c — HTTP/1.1 server for Linux & macOS)"

  # Standard sources — compiled with full CFLAGS
  compile_obj src/xly_rt.c      src/xly_rt.o
  compile_obj src/unicode.c     src/unicode.o
  compile_obj src/xly_http.c    src/xly_http.o
  compile_obj src/xenly_linker.c src/xenly_linker.o

  # Sources that disable multiprocessing for the static runtime
  compile_obj src/modules.c            src/modules_rt.o            "-DXENLY_NO_MULTIPROC"
  compile_obj src/multiproc.c          src/multiproc_rt.o          "-DXENLY_NO_MULTIPROC"
  compile_obj src/multiproc_builtins.c src/multiproc_builtins_rt.o "-DXENLY_NO_MULTIPROC"

  echo "Creating runtime library..."
  ar rcs "$RT_LIB" "${RT_OBJS[@]}"
  echo "  Runtime: $RT_LIB"
}

# ─── Commands ─────────────────────────────────────────────────────────────────

cmd_all() {
  build_interpreter
  build_compiler
  build_runtime
  echo ""
  echo "✓ Build complete!"
  echo "  Interpreter: $TARGET"
  echo "  Compiler:    $XENLYC"
  echo "  Runtime:     $RT_LIB"
  echo ""
}

cmd_run() {
  build_interpreter
  ./"$TARGET" examples/hello.xe
}

cmd_compile() {
  build_compiler
  build_runtime
  ./"$XENLYC" examples/hello.xe -o hello_compiled
  ./hello_compiled
  rm -f hello_compiled
}

cmd_test() {
  cmd_all
  echo "╔════════════════════════════════════════════════════════╗"
  echo "║         Running Xenly Test Suite                       ║"
  echo "╚════════════════════════════════════════════════════════╝"
  ./"$TARGET" examples/hello.xe  || { echo "✗ Interpreter test failed"; exit 1; }
  echo "✓ Interpreter works"
  ./"$XENLYC" examples/hello.xe -o test_compiled || { echo "✗ Compilation failed"; exit 1; }
  ./test_compiled || { echo "✗ Compiled binary failed"; exit 1; }
  rm -f test_compiled
  echo "✓ All tests passed!"
}

cmd_test_sys() {
  build_interpreter
  echo "╔════════════════════════════════════════════════════════╗"
  echo "║         Running sys Module Test                        ║"
  echo "╚════════════════════════════════════════════════════════╝"
  ./"$TARGET" examples/sys_demo.xe || { echo "✗ sys module test failed"; exit 1; }
  echo "✓ sys module test passed!"
}

cmd_test_http() {
  cmd_all
  echo "╔════════════════════════════════════════════════════════╗"
  echo "║         Running HTTP Server Smoke Test                 ║"
  echo "╚════════════════════════════════════════════════════════╝"
  if [[ ! -f examples/http_test.xe ]]; then
    echo "⚠  examples/http_test.xe not found — skipping HTTP test"
    return 0
  fi
  ./"$TARGET" examples/http_test.xe &
  local srv_pid=$!
  sleep 0.5
  local rc=0
  curl -sf http://localhost:8080/ping || rc=$?
  kill "$srv_pid" 2>/dev/null || true
  wait "$srv_pid" 2>/dev/null || true
  if [[ $rc -ne 0 ]]; then
    echo "✗ HTTP server smoke test failed"; exit 1
  fi
  echo "✓ HTTP server smoke test passed!"
}

cmd_linker_version() {
  build_compiler
  ./"$XENLYC" --linker-version
}

cmd_format() {
  if command -v clang-format &>/dev/null; then
    echo "Formatting source files..."
    clang-format -i src/*.c src/*.h
    echo "✓ Formatting complete"
  else
    echo "⚠  clang-format not found — install it or run manually"
  fi
}

cmd_clean() {
  echo "Cleaning build artifacts..."
  rm -f src/*.o src/*.d
  rm -f "$TARGET" "$XENLYC" "$RT_LIB"
  rm -f ./*.s ./*.o ./*.d a.out hello_compiled test_compiled
  echo "✓ Clean complete"
}

cmd_distclean() {
  cmd_clean
  find . -name '*~' -delete
  find . -name '*.swp' -delete
  find . -name '.DS_Store' -delete
}

cmd_install() {
  cmd_all
  install -d "$BINDIR" "$LIBDIR" "$INCDIR/xenly" "$DATADIR/doc/xenly"
  install -m 755 "$TARGET" "$XENLYC" "$BINDIR/"
  install -m 644 "$RT_LIB" "$LIBDIR/"
  install -m 644 src/*.h "$INCDIR/xenly/"
  install -m 644 ./*.md "$DATADIR/doc/xenly/" 2>/dev/null || true
  echo "✓ Installed to $PREFIX"
}

cmd_uninstall() {
  rm -f "$BINDIR/$TARGET" "$BINDIR/$XENLYC"
  rm -f "$LIBDIR/$RT_LIB"
  rm -rf "$INCDIR/xenly" "$DATADIR/doc/xenly"
  echo "✓ Uninstalled"
}

cmd_universal() {
  if [[ "$UNAME_S" != "Darwin" ]]; then
    echo "✗ universal target is macOS only"; exit 1
  fi
  echo "Building universal (x86_64 + arm64) binaries..."
  bash "$0" clean
  ARCH_FLAG="-arch x86_64" NO_NATIVE=1 TARGET=xenly_x86 XENLYC=xenlyc_x86 bash "$0" all
  bash "$0" clean-objs
  ARCH_FLAG="-arch arm64"  NO_NATIVE=1 TARGET=xenly_arm XENLYC=xenlyc_arm  bash "$0" all
  lipo -create -output "$TARGET" xenly_x86 xenly_arm
  lipo -create -output "$XENLYC" xenlyc_x86 xenlyc_arm
  rm -f xenly_x86 xenly_arm xenlyc_x86 xenlyc_arm
  file "$TARGET" "$XENLYC"
  echo "✓ Universal binaries created"
}

cmd_clean_objs() {
  rm -f src/*.o src/*.d
}

cmd_help() {
  echo ""
  echo "Xenly Build Script"
  echo "══════════════════════════════════════════════════════"
  echo ""
  echo "  Usage: bash main.sh [command]"
  echo ""
  echo "  Commands:"
  echo "    all             Build interpreter, compiler, and runtime (default)"
  echo "    run             Build and run examples/hello.xe"
  echo "    compile         Build and test the native compiler (xenlyc)"
  echo "                      Pipeline: .xe → lexer → parser → AST → sema"
  echo "                               → codegen → .s → as → .o → xlnk → binary"
  echo "    test            Run the core test suite"
  echo "    test-sys        Run the sys module demo (examples/sys_demo.xe)"
  echo "    test-http       HTTP server smoke test (requires curl)"
  echo "    linker-version  Print xlnk built-in linker version and exit"
  echo "    format          Auto-format all C source with clang-format"
  echo "    clean           Remove all build artifacts"
  echo "    distclean       Clean + remove editor temp files"
  echo "    install         Install to PREFIX (default: /usr/local)"
  echo "    uninstall       Remove installed files"
  echo "    universal       [macOS only] Build x86_64 + arm64 fat binary"
  echo "    help            Show this message"
  echo ""
  echo "  Environment variables:"
  echo "    PREFIX=<path>  Install prefix       (default: /usr/local)"
  echo "    DEBUG=1        Debug build (-O0 -g)"
  echo "    SANITIZE=1     Enable ASan + UBSan"
  echo "    NO_NATIVE=1    Disable -march=native (for cross-compiling)"
  echo "    CC=<compiler>  Override compiler     (default: gcc)"
  echo ""
  echo "  New source files in this build:"
  echo "    src/xenly_linker.c  — in-process ELF/Mach-O linker (xlnk, 20×)"
  echo "    src/xly_http.c      — HTTP/1.1 web server"
  echo "    src/xly_http.h      — HTTP server public API"
  echo "    src/unicode.c       — comprehensive Unicode library"
  echo ""
  echo "  Examples:"
  echo "    bash main.sh                        # standard build"
  echo "    bash main.sh test                   # build + run tests"
  echo "    bash main.sh test-http              # HTTP server smoke test"
  echo "    bash main.sh linker-version         # print xlnk version"
  echo "    DEBUG=1 bash main.sh                # debug build"
  echo "    SANITIZE=1 bash main.sh             # build with sanitizers"
  echo "    PREFIX=~/.local bash main.sh install"
  echo ""
}

# ─── Dispatch ─────────────────────────────────────────────────────────────────
CMD="${1:-all}"
case "$CMD" in
  all)             cmd_all ;;
  run)             cmd_run ;;
  compile)         cmd_compile ;;
  test)            cmd_test ;;
  test-sys)        cmd_test_sys ;;
  test-http)       cmd_test_http ;;
  linker-version)  cmd_linker_version ;;
  format)          cmd_format ;;
  clean)           cmd_clean ;;
  distclean)       cmd_distclean ;;
  install)         cmd_install ;;
  uninstall)       cmd_uninstall ;;
  universal)       cmd_universal ;;
  clean-objs)      cmd_clean_objs ;;
  help|--help|-h)  cmd_help ;;
  *)
    echo "✗ Unknown command: $CMD"
    cmd_help
    exit 1
    ;;
esac
