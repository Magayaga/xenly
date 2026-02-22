# ═══════════════════════════════════════════════════════════════════════════
# Xenly Language Makefile
# ═══════════════════════════════════════════════════════════════════════════

# ─── Platform Detection ──────────────────────────────────────────────────────
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# ─── Compiler Selection ──────────────────────────────────────────────────────
CC = gcc

# ─── Base Compiler Flags ─────────────────────────────────────────────────────
CFLAGS  = -Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L -Isrc
LDFLAGS = -lm

# ─── Platform-Specific Configuration ─────────────────────────────────────────

ifeq ($(UNAME_S),Linux)
    CFLAGS  += -DPLATFORM_LINUX
    LDFLAGS += -lpthread -ldl

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
        CFLAGS += -arch arm64
    else
        ifneq ($(NO_NATIVE),1)
            CFLAGS += -march=native -mtune=native
        endif
        CFLAGS += -arch x86_64
    endif

    CFLAGS += -mmacosx-version-min=10.13
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
              src/codegen.c src/unicode.c
XENLYC_OBJS = $(XENLYC_SRCS:.c=.o)

RT_LIB  = libxly_rt.a
RT_OBJS = src/xly_rt.o src/modules_rt.o src/unicode.o

# ─── Build Targets ───────────────────────────────────────────────────────────

.PHONY: all clean distclean install uninstall test run compile help

all: $(TARGET) $(XENLYC) $(RT_LIB)
	@echo ""
	@echo "✓ Build complete!"
	@echo "  Interpreter: $(TARGET)"
	@echo "  Compiler:    $(XENLYC)"
	@echo "  Runtime:     $(RT_LIB)"
	@echo ""

$(TARGET): $(INTERP_OBJS)
	@echo "Linking interpreter..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "  Interpreter: $(TARGET)"

$(XENLYC): $(XENLYC_OBJS)
	@echo "Linking compiler..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "  Compiler: $(XENLYC)"

$(RT_LIB): $(RT_OBJS)
	@echo "Creating runtime library..."
	$(AR) rcs $@ $^
	@echo "  Runtime: $(RT_LIB)"

# modules_rt: compiled without multiprocessing for the static runtime library
src/modules_rt.o: src/modules.c
	@echo "Compiling $< (runtime, no multiprocessing)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

src/%.o: src/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(INTERP_OBJS:.o=.d)
-include $(XENLYC_OBJS:.o=.d)

# ─── Convenience Targets ─────────────────────────────────────────────────────

run: $(TARGET)
	./$(TARGET) examples/hello.xe

compile: $(XENLYC) $(RT_LIB)
	./$(XENLYC) examples/hello.xe -o hello_compiled
	./hello_compiled
	@rm -f hello_compiled

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
.PHONY: universal
universal:
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(filter-out -arch%,$(CFLAGS)) -arch x86_64 -arch arm64"
	@file $(TARGET) $(XENLYC)
endif

# ─── Help ────────────────────────────────────────────────────────────────────
help:
	@echo "Xenly Build System"
	@echo "Targets: all clean distclean test run compile install uninstall help"
	@echo "Variables: PREFIX DEBUG=1 SANITIZE=1 NO_NATIVE=1 CC"
