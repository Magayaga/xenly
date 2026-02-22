#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
#
# It is initially written in Bash Script.
#
#!/usr/bin/env bash
set -euo pipefail

# ─── Platform Detection ───────────────────────────────────────────────────────
UNAME_S=$(uname -s)
UNAME_M=$(uname -m)

# Default compiler and flags
CC="gcc"
CFLAGS="-Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L -Isrc"
LDFLAGS="-lm"

# ─── Platform-Specific Configuration ─────────────────────────────────────────
case "$UNAME_S" in
  Linux)
    CFLAGS="$CFLAGS -DPLATFORM_LINUX"
    LDFLAGS="$LDFLAGS -lpthread -ldl"
    if [ "${NO_NATIVE:-0}" != "1" ]; then
      CFLAGS="$CFLAGS -march=native -mtune=native -ffast-math"
    fi
    if command -v ld.gold &>/dev/null; then
      LDFLAGS="$LDFLAGS -fuse-ld=gold"
    fi
    ;;
  Darwin)
    CFLAGS="$CFLAGS -DPLATFORM_MACOS"
    LDFLAGS="$LDFLAGS -lpthread"
    if [ "$UNAME_M" = "arm64" ]; then
      if [ "${NO_NATIVE:-0}" != "1" ]; then
        CFLAGS="$CFLAGS -mcpu=apple-m1 -mtune=apple-m1"
      fi
      CFLAGS="$CFLAGS -arch arm64"
    else
      if [ "${NO_NATIVE:-0}" != "1" ]; then
        CFLAGS="$CFLAGS -march=native -mtune=native"
      fi
      CFLAGS="$CFLAGS -arch x86_64"
    fi
    CFLAGS="$CFLAGS -mmacosx-version-min=10.13"
    ;;
  FreeBSD)
    CC="clang"
    CFLAGS="$CFLAGS -DPLATFORM_FREEBSD"
    LDFLAGS="$LDFLAGS -lpthread"
    if [ "${NO_NATIVE:-0}" != "1" ]; then
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

# ─── Debug Build Support ──────────────────────────────────────────────────────
if [ -n "${DEBUG:-}" ]; then
  CFLAGS="${CFLAGS/-O3/}"
  CFLAGS="${CFLAGS/-O2/}"
  CFLAGS="$CFLAGS -O0 -g -DDEBUG"
fi

# ─── Sanitizer Support ────────────────────────────────────────────────────────
if [ -n "${SANITIZE:-}" ]; then
  CFLAGS="$CFLAGS -fsanitize=address,undefined -fno-omit-frame-pointer"
  LDFLAGS="$LDFLAGS -fsanitize=address,undefined"
fi

# ─── Build Banner ─────────────────────────────────────────────────────────────
echo "╔════════════════════════════════════════════════════════╗"
echo "║  Building Xenly for $UNAME_S $UNAME_M"
echo "║  Compiler: $CC"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# ─── Source Files ─────────────────────────────────────────────────────────────
TARGET="xenly"
INTERP_SRCS="src/main.c src/lexer.c src/ast.c src/parser.c src/interpreter.c src/modules.c src/typecheck.c src/unicode.c src/multiproc.c src/multiproc_builtins.c"

XENLYC="xenlyc"
XENLYC_SRCS="src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c src/codegen.c src/unicode.c"

RT_LIB="libxly_rt.a"
RT_SRCS="src/xly_rt.c src/unicode.c"  # modules_rt compiled separately below

# ─── Installation Paths ───────────────────────────────────────────────────────
PREFIX="${PREFIX:-/usr/local}"
BINDIR="$PREFIX/bin"
LIBDIR="$PREFIX/lib"
INCDIR="$PREFIX/include"
DATADIR="$PREFIX/share"

# ─── Helper: compile .c files to .o ──────────────────────────────────────────
compile_objs() {
  local srcs="$1"
  local objs=""
  for src in $srcs; do
    local obj="${src%.c}.o"
    echo "  Compiling $src..."
    $CC $CFLAGS -c -o "$obj" "$src"
    objs="$objs $obj"
  done
  echo $objs
}

# ─── Special: compile modules.c without multiprocessing for the runtime lib ──
compile_modules_rt() {
  local obj="src/modules_rt.o"
  echo "  Compiling src/modules.c (runtime, no multiprocessing)..."
  $CC $CFLAGS -DXENLY_NO_MULTIPROC -c -o "$obj" "src/modules.c"
  echo "$obj"
}

# ─── Build Targets ────────────────────────────────────────────────────────────
build_interpreter() {
  echo "==> Building interpreter: $TARGET"
  local objs
  objs=$(compile_objs "$INTERP_SRCS")
  echo "Linking interpreter..."
  $CC $CFLAGS -o "$TARGET" $objs $LDFLAGS
  echo "  Interpreter: $TARGET"
}

build_compiler() {
  echo "==> Building compiler driver: $XENLYC"
  local objs
  objs=$(compile_objs "$XENLYC_SRCS")
  echo "Linking compiler..."
  $CC $CFLAGS -o "$XENLYC" $objs $LDFLAGS
  echo "  Compiler: $XENLYC"
}

build_runtime() {
  echo "==> Building runtime library: $RT_LIB"
  local rt_objs
  rt_objs=$(compile_objs "$RT_SRCS")
  local modules_rt_obj
  modules_rt_obj=$(compile_modules_rt)
  echo "Creating runtime library..."
  ar rcs "$RT_LIB" $rt_objs $modules_rt_obj
  echo "  Runtime: $RT_LIB"
}

build_all() {
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

# ─── Commands ─────────────────────────────────────────────────────────────────
cmd_all() {
  build_all
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
  build_all
  echo "==> Installing to $PREFIX..."
  install -d "$BINDIR" "$LIBDIR" "$INCDIR/xenly" "$DATADIR/doc/xenly"
  install -m 755 "$TARGET" "$XENLYC" "$BINDIR/"
  install -m 644 "$RT_LIB" "$LIBDIR/"
  install -m 644 src/*.h "$INCDIR/xenly/"
  install -m 644 ./*.md "$DATADIR/doc/xenly/" 2>/dev/null || true
  echo "✓ Installed to $PREFIX"
}

cmd_uninstall() {
  echo "==> Uninstalling from $PREFIX..."
  rm -f "$BINDIR/$TARGET" "$BINDIR/$XENLYC"
  rm -f "$LIBDIR/$RT_LIB"
  rm -rf "$INCDIR/xenly" "$DATADIR/doc/xenly"
  echo "✓ Uninstalled"
}

cmd_universal() {
  if [ "$UNAME_S" != "Darwin" ]; then
    echo "Error: universal target is macOS-only." >&2
    exit 1
  fi
  echo "==> Building macOS universal binary..."
  cmd_clean
  CFLAGS="$(echo "$CFLAGS" | sed 's/-arch [^ ]*//g') -arch x86_64 -arch arm64"
  build_all
  file "$TARGET" "$XENLYC"
}

cmd_test() {
  build_all
  echo "╔════════════════════════════════════════════════════════╗"
  echo "║         Running Xenly Test Suite                       ║"
  echo "╚════════════════════════════════════════════════════════╝"
  ./"$TARGET" examples/hello.xe || { echo "✗ Interpreter test failed"; exit 1; }
  echo "✓ Interpreter works"
  ./"$XENLYC" examples/hello.xe -o test_compiled || { echo "✗ Compilation failed"; exit 1; }
  ./test_compiled || { echo "✗ Compiled binary failed"; exit 1; }
  rm -f test_compiled
  echo "✓ All tests passed!"
}

cmd_help() {
  echo "Xenly Build System"
  echo "Usage: $0 {all|run|compile|clean|distclean|install|uninstall|universal|test|help}"
  echo ""
  echo "Variables: PREFIX DEBUG=1 SANITIZE=1 NO_NATIVE=1 CC"
}

# ─── Main Dispatcher ──────────────────────────────────────────────────────────
COMMAND="${1:-all}"

case "$COMMAND" in
  all)       cmd_all ;;
  run)       cmd_run ;;
  compile)   cmd_compile ;;
  clean)     cmd_clean ;;
  distclean) cmd_distclean ;;
  install)   cmd_install ;;
  uninstall) cmd_uninstall ;;
  universal) cmd_universal ;;
  test)      cmd_test ;;
  help)      cmd_help ;;
  *)
    echo "Usage: $0 {all|run|compile|clean|distclean|install|uninstall|universal|test|help}"
    exit 1
    ;;
esac