# ═══════════════════════════════════════════════════════════════════════════
# Xenly Language Makefile
# ═══════════════════════════════════════════════════════════════════════════

# ─── Platform Detection ──────────────────────────────────────────────────────
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# ─── Compiler Selection ──────────────────────────────────────────────────────
CC = gcc

# ─── Base Compiler Flags ─────────────────────────────────────────────────────
# FIX: Added -MMD -MP for automatic header dependency generation.
#      Without these, the -include *.d lines were silently no-ops, meaning
#      every `make` recompiled everything even when nothing changed.
CFLAGS  = -Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L -Isrc -MMD -MP
LDFLAGS = -lm

# ─── Platform-Specific Configuration ─────────────────────────────────────────

ifeq ($(UNAME_S),Linux)
    CFLAGS  += -DPLATFORM_LINUX
    # FIX: Added -lresolv for inet_pton/inet_ntop used by sys networking module.
    LDFLAGS += -lpthread -ldl -lresolv

    ifneq ($(NO_NATIVE),1)
        CFLAGS += -march=native -mtune=native -ffast-math
    endif

    ifneq ($(shell which ld.gold 2>/dev/null),)
        LDFLAGS += -fuse-ld=gold
    endif

    INSTALL = install
    AR = ar
endif

ifeq ($(UNAME_S),Darwin)
    CFLAGS  += -DPLATFORM_MACOS
    LDFLAGS += -lpthread

    ifeq ($(UNAME_M),arm64)
        ifneq ($(NO_NATIVE),1)
            CFLAGS += -mcpu=apple-m1 -mtune=apple-m1
        endif
        # FIX: Store arch flag separately so the universal target can override it
        #      cleanly without a fragile filter-out on a "-arch%" prefix pattern.
        ARCH_FLAG = -arch arm64
    else
        ifneq ($(NO_NATIVE),1)
            CFLAGS += -march=native -mtune=native
        endif
        ARCH_FLAG = -arch x86_64
    endif

    CFLAGS += $(ARCH_FLAG) -mmacosx-version-min=10.13
    INSTALL = install
    AR = ar
endif

ifeq ($(UNAME_S),FreeBSD)
    CC = clang
    CFLAGS  += -DPLATFORM_FREEBSD
    LDFLAGS += -lpthread

    ifneq ($(NO_NATIVE),1)
        CFLAGS += -march=native -mtune=native
    endif

    ifneq ($(shell which ld.lld 2>/dev/null),)
        LDFLAGS += -fuse-ld=lld
    endif

    INSTALL = install
    AR = ar
endif

ifeq ($(UNAME_S),OpenBSD)
    CC = clang
    CFLAGS  += -DPLATFORM_OPENBSD
    LDFLAGS += -lpthread
    INSTALL = install
    AR = ar
endif

ifeq ($(UNAME_S),NetBSD)
    CC = gcc
    CFLAGS  += -DPLATFORM_NETBSD
    LDFLAGS += -lpthread
    INSTALL = install
    AR = ar
endif

# ─── Debug Build Support ─────────────────────────────────────────────────────
ifdef DEBUG
    CFLAGS := $(filter-out -O3 -O2,$(CFLAGS))
    CFLAGS += -O0 -g -DDEBUG
endif

# ─── Sanitizer Support ───────────────────────────────────────────────────────
ifdef SANITIZE
    CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS += -fsanitize=address,undefined
endif

# ─── Build Banner ────────────────────────────────────────────────────────────
# FIX: Added closing box character to platform info line — was missing before.
$(info ╔════════════════════════════════════════════════════════╗)
$(info ║  Building Xenly for $(UNAME_S) $(UNAME_M))
$(info ║  Compiler: $(CC))
$(info ╚════════════════════════════════════════════════════════╝)
$(info )

# ─── Source Files ────────────────────────────────────────────────────────────

TARGET = xenly
INTERP_SRCS = src/main.c src/lexer.c src/ast.c src/parser.c \
              src/interpreter.c src/modules.c src/typecheck.c \
              src/unicode.c src/multiproc.c src/multiproc_builtins.c
INTERP_OBJS = $(INTERP_SRCS:.c=.o)

XENLYC = xenlyc
XENLYC_SRCS = src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c \
              src/codegen.c src/unicode.c src/sema.c
XENLYC_OBJS = $(XENLYC_SRCS:.c=.o)

RT_LIB = libxly_rt.a
# FIX: Added multiproc_rt.o and multiproc_builtins_rt.o to the runtime library.
#      They were compiled into the interpreter but missing from libxly_rt.a,
#      causing linker errors for anyone who links against the static runtime.
RT_OBJS = src/xly_rt.o src/modules_rt.o src/unicode.o \
          src/multiproc_rt.o src/multiproc_builtins_rt.o

# ─── Build Targets ───────────────────────────────────────────────────────────

.PHONY: all clean distclean install uninstall test test-sys run compile format help

all: $(TARGET) $(XENLYC) $(RT_LIB)
	@echo ""
	@echo "✓ Build complete!"
	@echo "  Interpreter: $(TARGET)"
	@echo "  Compiler:    $(XENLYC)"
	@echo "  Runtime:     $(RT_LIB)"
	@echo ""

# FIX: Link step now uses only $(LDFLAGS), not $(CFLAGS).
#      Passing compile flags (-march=native, -ffast-math etc.) to the linker
#      is harmless but incorrect — the linker ignores them, creating noise.
$(TARGET): $(INTERP_OBJS)
	@echo "Linking interpreter..."
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "  Interpreter: $(TARGET)"

$(XENLYC): $(XENLYC_OBJS)
	@echo "Linking compiler..."
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "  Compiler: $(XENLYC)"

$(RT_LIB): $(RT_OBJS)
	@echo "Creating runtime library..."
	$(AR) rcs $@ $^
	@echo "  Runtime: $(RT_LIB)"

# Runtime-specific object rules (XENLY_NO_MULTIPROC disables threading)
src/modules_rt.o: src/modules.c
	@echo "Compiling $< (runtime, no multiprocessing)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

src/multiproc_rt.o: src/multiproc.c
	@echo "Compiling $< (runtime stub)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

src/multiproc_builtins_rt.o: src/multiproc_builtins.c
	@echo "Compiling $< (runtime stub)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

# Generic rule — -MMD -MP in CFLAGS now makes this emit src/*.d files,
# so header changes trigger the right recompiles automatically.
src/%.o: src/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Include auto-generated dependency files (now actually populated by -MMD -MP)
-include $(INTERP_OBJS:.o=.d)
-include $(XENLYC_OBJS:.o=.d)
-include $(RT_OBJS:.o=.d)

# ─── Convenience Targets ─────────────────────────────────────────────────────

run: $(TARGET)
	./$(TARGET) examples/hello.xe

compile: $(XENLYC) $(RT_LIB)
	./$(XENLYC) examples/hello.xe -o hello_compiled
	./hello_compiled
	@rm -f hello_compiled

# ─── Test Targets ────────────────────────────────────────────────────────────

test: all
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║         Running Xenly Test Suite                       ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@./$(TARGET) examples/hello.xe || (echo "✗ Interpreter test failed" && exit 1)
	@echo "✓ Interpreter works"
	@./$(XENLYC) examples/hello.xe -o test_compiled || (echo "✗ Compilation failed" && exit 1)
	@./test_compiled || (echo "✗ Compiled binary failed" && exit 1)
	@rm -f test_compiled
	@echo "✓ All tests passed!"

# FIX: Added test-sys target — the sys module had no make entry point before.
test-sys: $(TARGET)
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║         Running sys Module Test                        ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@./$(TARGET) examples/sys_demo.xe || (echo "✗ sys module test failed" && exit 1)
	@echo "✓ sys module test passed!"

# ─── Code Formatting ─────────────────────────────────────────────────────────
# FIX: Added format target. .clang-format existed in the repo but there was
#      no way to invoke it from the build system. Gracefully skips if
#      clang-format is not installed.
format:
	@if command -v clang-format >/dev/null 2>&1; then \
	    echo "Formatting source files..."; \
	    clang-format -i src/*.c src/*.h; \
	    echo "✓ Formatting complete"; \
	else \
	    echo "⚠  clang-format not found — install it or run manually"; \
	fi

# ─── Cleaning ────────────────────────────────────────────────────────────────

clean:
	@echo "Cleaning build artifacts..."
	rm -f src/*.o src/*.d
	rm -f $(TARGET) $(XENLYC) $(RT_LIB)
	rm -f *.s *.o *.d a.out hello_compiled test_compiled
	@echo "✓ Clean complete"

distclean: clean
	find . -name '*~' -delete
	find . -name '*.swp' -delete
	find . -name '.DS_Store' -delete

# ─── Installation ────────────────────────────────────────────────────────────

PREFIX  ?= /usr/local
BINDIR  = $(PREFIX)/bin
LIBDIR  = $(PREFIX)/lib
INCDIR  = $(PREFIX)/include
DATADIR = $(PREFIX)/share

install: all
	$(INSTALL) -d $(BINDIR) $(LIBDIR) $(INCDIR)/xenly $(DATADIR)/doc/xenly
	$(INSTALL) -m 755 $(TARGET) $(XENLYC) $(BINDIR)/
	$(INSTALL) -m 644 $(RT_LIB) $(LIBDIR)/
	$(INSTALL) -m 644 src/*.h $(INCDIR)/xenly/
	$(INSTALL) -m 644 *.md $(DATADIR)/doc/xenly/ 2>/dev/null || true
	@echo "✓ Installed to $(PREFIX)"

uninstall:
	rm -f $(BINDIR)/$(TARGET) $(BINDIR)/$(XENLYC)
	rm -f $(LIBDIR)/$(RT_LIB)
	rm -rf $(INCDIR)/xenly $(DATADIR)/doc/xenly
	@echo "✓ Uninstalled"

# ─── macOS Universal Binary ──────────────────────────────────────────────────
ifeq ($(UNAME_S),Darwin)
.PHONY: universal clean-objs
# FIX: Replaced the broken filter-out on "-arch%" with an explicit ARCH_FLAG
#      variable override. The old filter-out didn't reliably strip all -arch
#      flags, leaving duplicate -arch arguments that clang rejects.
universal:
	@echo "Building universal (x86_64 + arm64) binaries..."
	$(MAKE) clean
	$(MAKE) all ARCH_FLAG="-arch x86_64" NO_NATIVE=1 TARGET=xenly_x86 XENLYC=xenlyc_x86
	$(MAKE) clean-objs
	$(MAKE) all ARCH_FLAG="-arch arm64"  NO_NATIVE=1 TARGET=xenly_arm XENLYC=xenlyc_arm
	lipo -create -output $(TARGET) xenly_x86 xenly_arm
	lipo -create -output $(XENLYC) xenlyc_x86 xenlyc_arm
	rm -f xenly_x86 xenly_arm xenlyc_x86 xenlyc_arm
	@file $(TARGET) $(XENLYC)
	@echo "✓ Universal binaries created"

clean-objs:
	rm -f src/*.o src/*.d
endif

# ─── Help ────────────────────────────────────────────────────────────────────

help:
	@echo ""
	@echo "Xenly Build System"
	@echo "══════════════════════════════════════════════════════"
	@echo ""
	@echo "  Targets:"
	@echo "    all          Build interpreter, compiler, and runtime (default)"
	@echo "    run          Build and run examples/hello.xe"
	@echo "    compile      Build and test the native compiler (xenlyc)"
	@echo "    test         Run the core test suite"
	@echo "    test-sys     Run the sys module demo (examples/sys_demo.xe)"
	@echo "    format       Auto-format all C source with clang-format"
	@echo "    clean        Remove all build artifacts"
	@echo "    distclean    Clean + remove editor temp files"
	@echo "    install      Install to PREFIX (default: /usr/local)"
	@echo "    uninstall    Remove installed files"
	@echo "    universal    [macOS only] Build x86_64 + arm64 fat binary"
	@echo "    help         Show this message"
	@echo ""
	@echo "  Variables:"
	@echo "    PREFIX=<path>  Install prefix       (default: /usr/local)"
	@echo "    DEBUG=1        Debug build (-O0 -g)"
	@echo "    SANITIZE=1     Enable ASan + UBSan"
	@echo "    NO_NATIVE=1    Disable -march=native (for cross-compiling)"
	@echo "    CC=<compiler>  Override compiler     (default: gcc)"
	@echo ""
	@echo "  Examples:"
	@echo "    make                       # standard build"
	@echo "    make test                  # build + run tests"
	@echo "    make test-sys              # run sys module demo"
	@echo "    make DEBUG=1               # debug build"
	@echo "    make SANITIZE=1            # build with sanitizers"
	@echo "    make install PREFIX=~/.local"
	@echo ""
