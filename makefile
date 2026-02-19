# ─── Xenly Language Makefile ─────────────────────────────────────────────────
# Platform detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Default compiler and flags
CC      = gcc
CFLAGS  = -Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lm

# ─── Platform-Specific Configuration ─────────────────────────────────────────
ifeq ($(UNAME_S),Linux)
    # Linux optimizations
    CFLAGS += -march=native -mtune=native -ffast-math
    CFLAGS += -DPLATFORM_LINUX
    LDFLAGS += -lpthread -ldl
    # Use gold linker if available for faster linking
    ifneq ($(shell which ld.gold 2>/dev/null),)
        LDFLAGS += -fuse-ld=gold
    endif
endif

ifeq ($(UNAME_S),Darwin)
    # macOS optimizations
    CFLAGS += -DPLATFORM_MACOS
    LDFLAGS += -lpthread
    
    # Apple Silicon (M1/M2/M3)
    ifeq ($(UNAME_M),arm64)
        CFLAGS += -mcpu=apple-m1 -mtune=apple-m1
        CFLAGS += -arch arm64
    else
        # Intel Mac
        CFLAGS += -march=native -mtune=native
        CFLAGS += -arch x86_64
    endif
    
    # macOS deployment target
    CFLAGS += -mmacosx-version-min=10.13
endif

ifeq ($(UNAME_S),FreeBSD)
    # FreeBSD optimizations
    CC = clang
    CFLAGS += -march=native -mtune=native
    CFLAGS += -DPLATFORM_FREEBSD
    LDFLAGS += -lpthread
    # Use ld.lld for faster linking
    LDFLAGS += -fuse-ld=lld
endif

ifeq ($(UNAME_S),OpenBSD)
    # OpenBSD (security-focused)
    CC = cc
    CFLAGS += -O2  # O3 can be unstable on OpenBSD
    CFLAGS += -fstack-protector-strong
    CFLAGS += -D_FORTIFY_SOURCE=2
    CFLAGS += -DPLATFORM_OPENBSD
    LDFLAGS += -lpthread
    # OpenBSD security hardening
    LDFLAGS += -Wl,-z,relro,-z,now
endif

ifeq ($(UNAME_S),NetBSD)
    # NetBSD optimizations
    CFLAGS += -march=native -mtune=native
    CFLAGS += -DPLATFORM_NETBSD
    LDFLAGS += -lpthread
endif

ifeq ($(UNAME_S),DragonFly)
    # DragonFly BSD
    CFLAGS += -march=native -mtune=native
    CFLAGS += -DPLATFORM_DRAGONFLY
    LDFLAGS += -lpthread
endif

# Print platform info
$(info Building for: $(UNAME_S) $(UNAME_M))
$(info Compiler: $(CC))
$(info Flags: $(CFLAGS))

# ─── Interpreter ─────────────────────────────────────────────────────────────
TARGET  = xenly
INTERP_SRCS = src/main.c src/lexer.c src/ast.c src/parser.c \
              src/interpreter.c src/modules.c src/typecheck.c src/unicode.c
INTERP_OBJS = $(INTERP_SRCS:.c=.o)

# ─── Native Compiler ─────────────────────────────────────────────────────────
XENLYC  = xenlyc

# xenlyc driver + frontend (shares lexer / parser / ast with interpreter)
XENLYC_SRCS = src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c src/codegen.c src/unicode.c
XENLYC_OBJS = $(XENLYC_SRCS:.c=.o)

# Runtime library: everything a compiled binary needs at run-time.
# xly_rt.c  — value ops, print, module dispatch shim
# modules.c — the 100 stdlib native functions (shared with interpreter)
# unicode.c — UTF-8 handling
RT_SRCS  = src/xly_rt.c src/modules.c src/unicode.c
RT_OBJS  = $(RT_SRCS:.c=.o)
RT_LIB   = libxly_rt.a

# ─── Default: build everything ───────────────────────────────────────────────
all: $(TARGET) $(XENLYC) $(RT_LIB)

# ─── Interpreter ─────────────────────────────────────────────────────────────
$(TARGET): $(INTERP_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ─── Compiler driver ─────────────────────────────────────────────────────────
$(XENLYC): $(XENLYC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ─── Runtime library ─────────────────────────────────────────────────────────
$(RT_LIB): $(RT_OBJS)
	ar rcs $@ $^

# ─── Generic .c → .o ────────────────────────────────────────────────────────
src/%.o: src/%.c
	$(CC) $(CFLAGS) -Isrc -c -o $@ $<

# ─── Convenience targets ─────────────────────────────────────────────────────
run: $(TARGET)
	./$(TARGET) examples/hello.xe

compile: $(XENLYC) $(RT_LIB)
	./$(XENLYC) examples/hello.xe -o hello_compiled
	./hello_compiled

# ─── Clean ───────────────────────────────────────────────────────────────────
clean:
	rm -f src/*.o $(TARGET) $(XENLYC) $(RT_LIB)
	rm -f *.s *.o a.out hello_compiled

# ─── Installation ────────────────────────────────────────────────────────────
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include

install: all
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/
	install -m 755 $(XENLYC) $(BINDIR)/
	install -d $(LIBDIR)
	install -m 644 $(RT_LIB) $(LIBDIR)/
	install -d $(INCDIR)/xenly
	install -m 644 src/*.h $(INCDIR)/xenly/
	@echo "Xenly installed to $(PREFIX)"

uninstall:
	rm -f $(BINDIR)/$(TARGET)
	rm -f $(BINDIR)/$(XENLYC)
	rm -f $(LIBDIR)/$(RT_LIB)
	rm -rf $(INCDIR)/xenly
	@echo "Xenly uninstalled from $(PREFIX)"

# ─── macOS Universal Binary ──────────────────────────────────────────────────
ifeq ($(UNAME_S),Darwin)
universal:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(CFLAGS) -arch x86_64 -arch arm64"
	@echo "Built universal binary for macOS"
endif

# ─── Testing ─────────────────────────────────────────────────────────────────
test: all
	@echo "Running tests..."
	./$(TARGET) examples/hello.xe
	./$(XENLYC) examples/hello.xe -o test_compiled
	./test_compiled
	rm -f test_compiled
	@echo "Tests passed!"

.PHONY: all run compile clean install uninstall universal test