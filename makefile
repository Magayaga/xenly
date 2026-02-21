# ─── Xenly Language Makefile ─────────────────────────────────────────────────
# Platform detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# ─── Compiler Selection ──────────────────────────────────────────────────────
CC = gcc

# ─── Base Compiler Flags ─────────────────────────────────────────────────────
CFLAGS  = -Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L -Isrc
LDFLAGS = -lm

# ─── XDMML Configuration ─────────────────────────────────────────────────────
# Set XDMML=0 to disable XDMML support (for systems without multimedia libs)
XDMML ?= 1

# ─── Platform-Specific Configuration ─────────────────────────────────────────

ifeq ($(UNAME_S),Linux)
    CFLAGS  += -DPLATFORM_LINUX
    LDFLAGS += -lpthread -ldl
    
    ifeq ($(XDMML),1)
        CFLAGS  += -DXDMML_ENABLED -DXDMML_PLATFORM_LINUX
        # Try to detect if multimedia libraries are available
        HAS_X11 := $(shell pkg-config --exists x11 2>/dev/null && echo 1 || echo 0)
        HAS_GL := $(shell pkg-config --exists gl 2>/dev/null && echo 1 || echo 0)
        HAS_ALSA := $(shell pkg-config --exists alsa 2>/dev/null && echo 1 || echo 0)
        
        ifeq ($(HAS_X11),1)
            CFLAGS += -DXDMML_HAS_X11
            LDFLAGS += $(shell pkg-config --libs x11)
        endif
        
        ifeq ($(HAS_GL),1)
            CFLAGS += -DXDMML_HAS_OPENGL
            LDFLAGS += $(shell pkg-config --libs gl)
        endif
        
        ifeq ($(HAS_ALSA),1)
            CFLAGS += -DXDMML_HAS_ALSA
            LDFLAGS += $(shell pkg-config --libs alsa)
        endif
    endif
    
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
    
    ifeq ($(XDMML),1)
        CFLAGS  += -DXDMML_ENABLED -DXDMML_PLATFORM_MACOS
        CFLAGS  += -DXDMML_HAS_COCOA -DXDMML_HAS_OPENGL -DXDMML_HAS_COREAUDIO
        LDFLAGS += -framework Cocoa -framework OpenGL -framework CoreAudio
    endif
    
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
    
    ifeq ($(XDMML),1)
        CFLAGS  += -DXDMML_ENABLED -DXDMML_PLATFORM_FREEBSD
        CFLAGS  += -DXDMML_HAS_X11
        LDFLAGS += -lX11 -lGL
    endif
    
    ifneq ($(NO_NATIVE),1)
        CFLAGS += -march=native -mtune=native
    endif
    
    ifneq ($(shell which ld.lld 2>/dev/null),)
        LDFLAGS += -fuse-ld=lld
    endif
    
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

# ─── Build Configuration ─────────────────────────────────────────────────────
$(info ╔════════════════════════════════════════════════════════╗)
$(info ║  Building Xenly for $(UNAME_S) $(UNAME_M))
$(info ║  Compiler: $(CC))
ifeq ($(XDMML),1)
$(info ║  XDMML: ENABLED)
else
$(info ║  XDMML: DISABLED)
endif
$(info ╚════════════════════════════════════════════════════════╝)
$(info )

# ─── Source Files ────────────────────────────────────────────────────────────

# XDMML Sources (only if enabled)
ifeq ($(XDMML),1)
    XDMML_SRCS = src/xdmml/xdmml_core.c \
                 src/xdmml/xdmml_audio.c \
                 src/xdmml/xdmml_threads.c \
                 src/xdmml/xdmml_net.c \
                 src/xdmml/xdmml_dynload.c
    XDMML_BINDINGS = src/xdmml_full_bindings.c
else
    XDMML_SRCS =
    XDMML_BINDINGS =
endif

# Interpreter
TARGET = xenly
INTERP_SRCS = src/main.c src/lexer.c src/ast.c src/parser.c \
              src/interpreter.c src/modules.c src/typecheck.c \
              src/unicode.c src/multiproc.c src/multiproc_builtins.c \
              $(XDMML_BINDINGS) $(XDMML_SRCS)
INTERP_OBJS = $(INTERP_SRCS:.c=.o)

# Native Compiler
XENLYC = xenlyc
XENLYC_SRCS = src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c \
              src/codegen.c src/unicode.c
XENLYC_OBJS = $(XENLYC_SRCS:.c=.o)

# Runtime Library
RT_LIB  = libxly_rt.a
RT_OBJS = src/xly_rt.o src/modules_rt.o src/unicode.o

# ─── Build Targets ───────────────────────────────────────────────────────────

.PHONY: all clean install uninstall test run compile help

all: $(TARGET) $(XENLYC) $(RT_LIB)
	@echo ""
	@echo "✓ Build complete!"
	@echo "  Interpreter: $(TARGET)"
ifeq ($(XDMML),1)
	@echo "               (with XDMML support)"
else
	@echo "               (without XDMML)"
endif
	@echo "  Compiler:    $(XENLYC)"
	@echo "  Runtime:     $(RT_LIB)"
	@echo ""
	@echo "To build without XDMML: make XDMML=0"

# ─── Interpreter ─────────────────────────────────────────────────────────────
$(TARGET): $(INTERP_OBJS)
	@echo "Linking interpreter..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ─── Compiler ────────────────────────────────────────────────────────────────
$(XENLYC): $(XENLYC_OBJS)
	@echo "Linking compiler..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ─── Runtime Library ─────────────────────────────────────────────────────────
$(RT_LIB): $(RT_OBJS)
	@echo "Creating runtime library..."
	$(AR) rcs $@ $^

# ─── Special Compilation Rules ───────────────────────────────────────────────

src/modules_rt.o: src/modules.c
	@echo "Compiling $< (no multiprocessing)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

src/%.o: src/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

src/xdmml/%.o: src/xdmml/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# ─── Dependency Tracking ─────────────────────────────────────────────────────
-include $(INTERP_OBJS:.o=.d)
-include $(XENLYC_OBJS:.o=.d)

# ─── Convenience Targets ─────────────────────────────────────────────────────

run: $(TARGET)
	@echo "Running hello world..."
	./$(TARGET) examples/hello.xe

compile: $(XENLYC) $(RT_LIB)
	@echo "Compiling hello world..."
	./$(XENLYC) examples/hello.xe -o hello_compiled
	@echo "Running compiled program..."
	./hello_compiled
	@rm -f hello_compiled

test: all
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║         Running Xenly Test Suite                       ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "→ Testing interpreter..."
	@./$(TARGET) examples/hello.xe || (echo "✗ Interpreter test failed" && exit 1)
	@echo "✓ Interpreter works"
	@echo ""
	@echo "→ Testing compiler..."
	@./$(XENLYC) examples/hello.xe -o test_compiled || (echo "✗ Compilation failed" && exit 1)
	@echo "✓ Compilation successful"
	@echo ""
	@echo "→ Running compiled binary..."
	@./test_compiled || (echo "✗ Compiled binary failed" && exit 1)
	@echo "✓ Compiled binary works"
	@rm -f test_compiled
	@echo ""
	@echo "✓ All tests passed!"

# ─── Cleaning ────────────────────────────────────────────────────────────────

clean:
	@echo "Cleaning build artifacts..."
	rm -f src/*.o src/*.d src/xdmml/*.o src/xdmml/*.d
	rm -f $(TARGET) $(XENLYC) $(RT_LIB)
	rm -f *.s *.o *.d a.out hello_compiled test_compiled
	@echo "✓ Clean complete"

distclean: clean
	@echo "Deep cleaning..."
	find . -name '*~' -delete
	find . -name '*.swp' -delete
	find . -name '.DS_Store' -delete
	@echo "✓ Deep clean complete"

# ─── Installation ────────────────────────────────────────────────────────────

PREFIX  ?= /usr/local
BINDIR  = $(PREFIX)/bin
LIBDIR  = $(PREFIX)/lib
INCDIR  = $(PREFIX)/include
DATADIR = $(PREFIX)/share

install: all
	@echo "Installing Xenly to $(PREFIX)..."
	$(INSTALL) -d $(BINDIR)
	$(INSTALL) -m 755 $(TARGET) $(BINDIR)/
	$(INSTALL) -m 755 $(XENLYC) $(BINDIR)/
	$(INSTALL) -d $(LIBDIR)
	$(INSTALL) -m 644 $(RT_LIB) $(LIBDIR)/
	$(INSTALL) -d $(INCDIR)/xenly
	$(INSTALL) -m 644 src/*.h $(INCDIR)/xenly/
ifeq ($(XDMML),1)
	$(INSTALL) -d $(INCDIR)/xdmml
	$(INSTALL) -m 644 src/xdmml/*.h $(INCDIR)/xdmml/
endif
	$(INSTALL) -d $(DATADIR)/doc/xenly
	$(INSTALL) -m 644 *.md $(DATADIR)/doc/xenly/ 2>/dev/null || true
	@echo "✓ Installation complete!"

uninstall:
	@echo "Uninstalling Xenly from $(PREFIX)..."
	rm -f $(BINDIR)/$(TARGET)
	rm -f $(BINDIR)/$(XENLYC)
	rm -f $(LIBDIR)/$(RT_LIB)
	rm -rf $(INCDIR)/xenly
	rm -rf $(INCDIR)/xdmml
	rm -rf $(DATADIR)/doc/xenly
	@echo "✓ Uninstall complete"

# ─── macOS Universal Binary ──────────────────────────────────────────────────

ifeq ($(UNAME_S),Darwin)
.PHONY: universal

universal:
	@echo "Building macOS universal binary (x86_64 + arm64)..."
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(filter-out -arch%,$(CFLAGS)) -arch x86_64 -arch arm64"
	@echo "✓ Universal binary created"
	@file $(TARGET)
	@file $(XENLYC)
endif

# ─── Help Target ─────────────────────────────────────────────────────────────

help:
	@echo "Xenly Build System"
	@echo "=================="
	@echo ""
	@echo "Targets:"
	@echo "  all         Build interpreter, compiler, and runtime (default)"
	@echo "  clean       Remove build artifacts"
	@echo "  distclean   Deep clean (includes editor files)"
	@echo "  test        Run test suite"
	@echo "  run         Run interpreter with hello.xe"
	@echo "  compile     Compile and run hello.xe"
	@echo "  install     Install to $(PREFIX)"
	@echo "  uninstall   Remove installation"
	@echo "  help        Show this help"
ifeq ($(UNAME_S),Darwin)
	@echo "  universal   Build macOS universal binary"
endif
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX      Installation prefix (default: /usr/local)"
	@echo "  DEBUG=1     Build with debug symbols"
	@echo "  SANITIZE=1  Build with address sanitizer"
	@echo "  NO_NATIVE=1 Disable native CPU optimizations"
	@echo "  XDMML=0     Disable XDMML multimedia support"
	@echo "  CC          C compiler (default: $(CC))"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build with XDMML (if libs available)"
	@echo "  make XDMML=0            # Build without XDMML"
	@echo "  make DEBUG=1            # Debug build"
	@echo "  make test               # Run tests"
	@echo "  make install PREFIX=~   # Install to home directory"

.PHONY: all clean distclean install uninstall test run compile help
