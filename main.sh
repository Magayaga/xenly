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
CFLAGS="-Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L"
LDFLAGS="-lm"

# ─── Platform-Specific Configuration ─────────────────────────────────────────
case "$UNAME_S" in
  Linux)
    CFLAGS="$CFLAGS -march=native -mtune=native -ffast-math"
    CFLAGS="$CFLAGS -DPLATFORM_LINUX"
    LDFLAGS="$LDFLAGS -lpthread -ldl"
    if command -v ld.gold &>/dev/null; then
      LDFLAGS="$LDFLAGS -fuse-ld=gold"
    fi
    ;;
  Darwin)
    CFLAGS="$CFLAGS -DPLATFORM_MACOS"
    LDFLAGS="$LDFLAGS -lpthread"
    if [ "$UNAME_M" = "arm64" ]; then
      CFLAGS="$CFLAGS -mcpu=apple-m1 -mtune=apple-m1 -arch arm64"
    else
      CFLAGS="$CFLAGS -march=native -mtune=native -arch x86_64"
    fi
    CFLAGS="$CFLAGS -mmacosx-version-min=10.13"
    ;;
  FreeBSD)
    CC="clang"
    CFLAGS="$CFLAGS -march=native -mtune=native"
    CFLAGS="$CFLAGS -DPLATFORM_FREEBSD"
    LDFLAGS="$LDFLAGS -lpthread -fuse-ld=lld"
    ;;
  OpenBSD)
    CC="cc"
    # Replace O3 with O2 for stability on OpenBSD
    CFLAGS="${CFLAGS/-O3/-O2}"
    CFLAGS="$CFLAGS -fstack-protector-strong -D_FORTIFY_SOURCE=2"
    CFLAGS="$CFLAGS -DPLATFORM_OPENBSD"
    LDFLAGS="$LDFLAGS -lpthread -Wl,-z,relro,-z,now"
    ;;
  NetBSD)
    CFLAGS="$CFLAGS -march=native -mtune=native"
    CFLAGS="$CFLAGS -DPLATFORM_NETBSD"
    LDFLAGS="$LDFLAGS -lpthread"
    ;;
  DragonFly)
    CFLAGS="$CFLAGS -march=native -mtune=native"
    CFLAGS="$CFLAGS -DPLATFORM_DRAGONFLY"
    LDFLAGS="$LDFLAGS -lpthread"
    ;;
esac

echo "Building for: $UNAME_S $UNAME_M"
echo "Compiler: $CC"
echo "Flags: $CFLAGS"

# ─── Source Files ─────────────────────────────────────────────────────────────
TARGET="xenly"
INTERP_SRCS="src/main.c src/lexer.c src/ast.c src/parser.c src/interpreter.c src/modules.c src/typecheck.c src/unicode.c"

XENLYC="xenlyc"
XENLYC_SRCS="src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c src/codegen.c src/unicode.c"

RT_SRCS="src/xly_rt.c src/modules.c src/unicode.c"
RT_LIB="libxly_rt.a"

# ─── Installation Paths ───────────────────────────────────────────────────────
PREFIX="${PREFIX:-/usr/local}"
BINDIR="$PREFIX/bin"
LIBDIR="$PREFIX/lib"
INCDIR="$PREFIX/include"

# ─── Helper: compile .c files to .o ──────────────────────────────────────────
compile_objs() {
  local srcs="$1"
  local objs=""
  for src in $srcs; do
    local obj="${src%.c}.o"
    echo "  CC $src -> $obj"
    $CC $CFLAGS -Isrc -c -o "$obj" "$src"
    objs="$objs $obj"
  done
  echo $objs
}

# ─── Build Targets ────────────────────────────────────────────────────────────
build_interpreter() {
  echo "==> Building interpreter: $TARGET"
  local objs
  objs=$(compile_objs "$INTERP_SRCS")
  $CC $CFLAGS -o "$TARGET" $objs $LDFLAGS
  echo "    Done: $TARGET"
}

build_compiler() {
  echo "==> Building compiler driver: $XENLYC"
  local objs
  objs=$(compile_objs "$XENLYC_SRCS")
  $CC $CFLAGS -o "$XENLYC" $objs $LDFLAGS
  echo "    Done: $XENLYC"
}

build_runtime() {
  echo "==> Building runtime library: $RT_LIB"
  local objs
  objs=$(compile_objs "$RT_SRCS")
  ar rcs "$RT_LIB" $objs
  echo "    Done: $RT_LIB"
}

build_all() {
  build_interpreter
  build_compiler
  build_runtime
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
}

cmd_clean() {
  echo "==> Cleaning..."
  rm -f src/*.o "$TARGET" "$XENLYC" "$RT_LIB"
  rm -f ./*.s ./*.o a.out hello_compiled
  echo "    Done."
}

cmd_install() {
  build_all
  echo "==> Installing to $PREFIX..."
  install -d "$BINDIR"
  install -m 755 "$TARGET" "$BINDIR/"
  install -m 755 "$XENLYC" "$BINDIR/"
  install -d "$LIBDIR"
  install -m 644 "$RT_LIB" "$LIBDIR/"
  install -d "$INCDIR/xenly"
  install -m 644 src/*.h "$INCDIR/xenly/"
  echo "Xenly installed to $PREFIX"
}

cmd_uninstall() {
  echo "==> Uninstalling from $PREFIX..."
  rm -f "$BINDIR/$TARGET"
  rm -f "$BINDIR/$XENLYC"
  rm -f "$LIBDIR/$RT_LIB"
  rm -rf "$INCDIR/xenly"
  echo "Xenly uninstalled from $PREFIX"
}

cmd_universal() {
  if [ "$UNAME_S" != "Darwin" ]; then
    echo "Error: universal target is macOS-only." >&2
    exit 1
  fi
  echo "==> Building macOS universal binary..."
  cmd_clean
  CFLAGS="$CFLAGS -arch x86_64 -arch arm64" build_all
  echo "Built universal binary for macOS"
}

cmd_test() {
  build_all
  echo "==> Running tests..."
  ./"$TARGET" examples/hello.xe
  ./"$XENLYC" examples/hello.xe -o test_compiled
  ./test_compiled
  rm -f test_compiled
  echo "Tests passed!"
}

# ─── Main Dispatcher ──────────────────────────────────────────────────────────
COMMAND="${1:-all}"

case "$COMMAND" in
  all)       cmd_all ;;
  run)       cmd_run ;;
  compile)   cmd_compile ;;
  clean)     cmd_clean ;;
  install)   cmd_install ;;
  uninstall) cmd_uninstall ;;
  universal) cmd_universal ;;
  test)      cmd_test ;;
  *)
    echo "Usage: $0 {all|run|compile|clean|install|uninstall|universal|test}"
    exit 1
    ;;
esac