# â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
# Xenly Language Makefile
# â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

# â”€â”€â”€ Platform Detection â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# â”€â”€â”€ Compiler Selection â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
CC = gcc

# â”€â”€â”€ Base Compiler Flags â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
# FIX: Added -MMD -MP for automatic header dependency generation.
#      Without these, the -include *.d lines were silently no-ops, meaning
#      every `make` recompiled everything even when nothing changed.
CFLAGS  = -Wall -Wextra -O3 -std=c11 -D_POSIX_C_SOURCE=200809L -Isrc -MMD -MP
LDFLAGS = -lm

# â”€â”€â”€ Platform-Specific Configuration â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

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

# â”€â”€â”€ Debug Build Support â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
ifdef DEBUG
    CFLAGS := $(filter-out -O3 -O2,$(CFLAGS))
    CFLAGS += -O0 -g -DDEBUG
endif

# â”€â”€â”€ Sanitizer Support â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
ifdef SANITIZE
    CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
    LDFLAGS += -fsanitize=address,undefined
endif

# â”€â”€â”€ Build Banner â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
$(info â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—)
$(info â•‘  Building Xenly for $(UNAME_S) $(UNAME_M))
$(info â•‘  Compiler: $(CC))
$(info â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•)
$(info )

# â”€â”€â”€ Source Files â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

# --- Stale Object Guard -----------------------------------------------
# FIX: If the source tree is copied/zipped/rsynced between machines (or a
#      previous build used a different OS/arch/compiler), leftover .o files
#      can be newer than their .c files, so make treats them as "up to
#      date" and skips recompiling them. Linking those stale objects
#      against freshly compiled ones fails with a cryptic linker error like
#      "unknown file type in 'src/lexer.o'" instead of a clear message.
#      This stamp records the config used for the last build; if it
#      doesn't match the current one (different CC/OS/arch), stale .o/.d
#      files are purged before anything is compiled.
BUILD_STAMP  := .build_config
BUILD_CONFIG := $(CC)|$(UNAME_S)|$(UNAME_M)|$(ARCH_FLAG)

# NOTE: this must not become the makefile's default goal (make uses the
# first target defined in the file). Force the real default explicitly.
.DEFAULT_GOAL := all

.PHONY: check-stale-objects
check-stale-objects:
	@if [ -f $(BUILD_STAMP) ]; then \
	    if [ "$$(cat $(BUILD_STAMP))" != "$(BUILD_CONFIG)" ]; then \
	        echo "Build config changed since last build ($$(cat $(BUILD_STAMP)) -> $(BUILD_CONFIG))"; \
	        echo "Removing stale object files to avoid linker errors..."; \
	        rm -f src/*.o src/*.d; \
	    fi; \
	elif ls src/*.o >/dev/null 2>&1; then \
	    echo "Object files exist with no build-config stamp from this makefile"; \
	    echo "(likely committed to git or copied from another checkout/machine)."; \
	    echo "A fresh git checkout gives .o files the same timestamp as their .c"; \
	    echo "source, so make would treat them as already up to date and skip"; \
	    echo "recompiling them, which can link stale or incompatible objects."; \
	    echo "Removing them to force a clean rebuild..."; \
	    rm -f src/*.o src/*.d; \
	fi
	@echo "$(BUILD_CONFIG)" > $(BUILD_STAMP)

TARGET = xenly
INTERP_SRCS = src/main.c src/lexer.c src/ast.c src/parser.c \
	      src/interpreter.c src/modules.c src/typecheck.c \
	      src/unicode.c src/multiproc.c src/multiproc_builtins.c \
	      src/xly_http.c
INTERP_OBJS = $(INTERP_SRCS:.c=.o)

XENLYC = xenlyc
# xenly_linker.c provides the in-process xlnk linker (20x faster than
# spawning gcc/clang). Must be in XENLYC_SRCS so xenlyc links correctly.
XENLYC_SRCS = src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c \
	      src/codegen.c src/unicode.c src/sema.c \
	      src/xenly_linker.c
XENLYC_OBJS = $(XENLYC_SRCS:.c=.o)

RT_LIB = libxly_rt.a
# FIX: Added multiproc_rt.o and multiproc_builtins_rt.o to the runtime library.
#      They were compiled into the interpreter but missing from libxly_rt.a,
#      causing linker errors for anyone who links against the static runtime.
# xly_http.o and xenly_linker.o are included so libxly_rt.a exposes the
# HTTP/1.1 server and in-process linker to anyone linking against the runtime.
RT_OBJS = src/xly_rt.o src/modules_rt.o src/unicode.o \
	  src/multiproc_rt.o src/multiproc_builtins_rt.o \
	  src/xly_http.o src/xenly_linker.o

# libxly_rtc.a â€” compiler runtime with real sys module support.
# xenlyc links compiled .xe programs against this instead of libxly_rt.a.
# It must live next to the xenlyc binary so xenlyc can find it at link time.
#
# FIX: previously used xly_rt_compiler_stub.o's modules_get(), which always
#      returned "not found" ("compiled programs do not support dynamic
#      module import"). In reality only sys.* CONSTANTS were ever ported to
#      the compiler's codegen fast path (src/codegen.c) â€” every sys.*
#      FUNCTION (sys.getpid, sys.open, sys.read, sys.mmap, sockets, clocks,
#      etc.) fell through to that stub, silently returning null and printing
#      "[xenly] unknown module" at runtime for any compiled program that
#      used them. xly_rt_compiler_stub.c now includes src/sys_module.inc,
#      the same sys implementation src/modules.c uses for the interpreter
#      (see that file's header comment for why sharing it is ABI-safe),
#      giving xenlyc-compiled binaries a real sys module with no per-
#      function porting required. Linking the OTHER modules (reflect, http)
#      the same way isn't possible: they need interpreter-only glue
#      (Environment, HTTP server plumbing) that compiled binaries don't have.
RTC_LIB  = libxly_rtc.a
RTC_OBJS = src/xly_rt.o src/unicode.o src/xly_rt_compiler_stub.o

# â”€â”€â”€ Build Targets â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

.PHONY: all clean distclean install uninstall test test-sys test-http \
	linker-version run compile format help

all: check-stale-objects $(TARGET) $(XENLYC) $(RT_LIB) $(RTC_LIB)
	@echo ""
	@echo "âœ“ Build complete!"
	@echo "  Interpreter:     $(TARGET)"
	@echo "  Compiler:        $(XENLYC)"
	@echo "  Runtime (interp):$(RT_LIB)"
	@echo "  Runtime (xenlyc):$(RTC_LIB)"
	@echo ""
	@echo "  Usage:"
	@echo "    ./$(XENLYC) main.xe -o main   # compile a Xenly source file"
	@echo "    ./main                         # run the compiled executable"
	@echo ""

# FIX: Link step now uses only $(LDFLAGS), not $(CFLAGS).
#      Passing compile flags (-march=native, -ffast-math etc.) to the linker
#      is harmless but incorrect â€” the linker ignores them, creating noise.
$(TARGET): $(INTERP_OBJS)
	@echo "Linking interpreter..."
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "  Interpreter: $(TARGET)"

$(XENLYC): $(XENLYC_OBJS)
	@echo "Linking compiler (with built-in xlnk linker)..."
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "  Compiler: $(XENLYC)"

$(RT_LIB): $(RT_OBJS)
	@echo "Creating runtime library..."
	$(AR) rcs $@ $^
	@echo "  Runtime: $(RT_LIB)"

$(RTC_LIB): $(RTC_OBJS)
	@echo "Creating compiler runtime library..."
	$(AR) rcs $@ $^
	@echo "  Compiler runtime: $(RTC_LIB)"

src/xly_rt_compiler_stub.o: src/xly_rt_compiler_stub.c | check-stale-objects
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Runtime-specific object rules (XENLY_NO_MULTIPROC disables threading)
src/modules_rt.o: src/modules.c | check-stale-objects
	@echo "Compiling $< (runtime, no multiprocessing)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

src/multiproc_rt.o: src/multiproc.c | check-stale-objects
	@echo "Compiling $< (runtime stub)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

src/multiproc_builtins_rt.o: src/multiproc_builtins.c \
	      src/xly_http.c | check-stale-objects
	@echo "Compiling $< (runtime stub)..."
	$(CC) $(CFLAGS) -DXENLY_NO_MULTIPROC -c -o $@ $<

# Generic rule â€” -MMD -MP in CFLAGS now makes this emit src/*.d files,
# so header changes trigger the right recompiles automatically.
src/%.o: src/%.c | check-stale-objects
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Include auto-generated dependency files (now actually populated by -MMD -MP)
-include $(INTERP_OBJS:.o=.d)
-include $(XENLYC_OBJS:.o=.d)
-include $(RT_OBJS:.o=.d)

# â”€â”€â”€ Convenience Targets â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

run: $(TARGET)
	./$(TARGET) examples/hello.xe

compile: $(XENLYC) $(RT_LIB)
	./$(XENLYC) examples/hello.xe -o hello_compiled
	./hello_compiled
	@rm -f hello_compiled

# â”€â”€â”€ Test Targets â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

test: all
	@echo "â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—"
	@echo "â•‘         Running Xenly Test Suite                       â•‘"
	@echo "â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•"
	@./$(TARGET) examples/hello.xe || (echo "âœ— Interpreter test failed" && exit 1)
	@echo "âœ“ Interpreter works"
	@./$(XENLYC) examples/hello.xe -o test_compiled || (echo "âœ— Compilation failed" && exit 1)
	@./test_compiled || (echo "âœ— Compiled binary failed" && exit 1)
	@rm -f test_compiled
	@echo "âœ“ All tests passed!"

test-sys: $(TARGET)
	@echo "â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—"
	@echo "â•‘         Running sys Module Test                        â•‘"
	@echo "â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•"
	@./$(TARGET) examples/sys_demo.xe || (echo "âœ— sys module test failed" && exit 1)
	@echo "âœ“ sys module test passed!"

# FIX: Added test-http target to mirror install-c.py's cmd_test_http.
#      Starts the interpreter running examples/http_test.xe in the
#      background, curls /ping, then tears the server down.
test-http: all
	@echo "â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—"
	@echo "â•‘         Running HTTP Server Smoke Test                 â•‘"
	@echo "â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•"
	@if [ ! -f examples/http_test.xe ]; then \
	    echo "âš   examples/http_test.xe not found â€” skipping HTTP test"; \
	else \
	    ./$(TARGET) examples/http_test.xe & \
	    SRV_PID=$$!; \
	    sleep 0.5; \
	    curl -sf http://localhost:8080/ping; RC=$$?; \
	    kill $$SRV_PID 2>/dev/null; wait $$SRV_PID 2>/dev/null; \
	    if [ $$RC -ne 0 ]; then \
	        echo "âœ— HTTP server smoke test failed"; exit 1; \
	    fi; \
	    echo "âœ“ HTTP server smoke test passed!"; \
	fi

# FIX: Added linker-version target to mirror install-c.py's
#      cmd_linker_version â€” prints the built-in xlnk linker version.
linker-version: $(XENLYC)
	./$(XENLYC) --linker-version

# â”€â”€â”€ Code Formatting â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
format:
	@if command -v clang-format >/dev/null 2>&1; then \
	    echo "Formatting source files..."; \
	    clang-format -i src/*.c src/*.h; \
	    echo "âœ“ Formatting complete"; \
	else \
	    echo "âš   clang-format not found â€” install it or run manually"; \
	fi

# â”€â”€â”€ Cleaning â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

clean:
	@echo "Cleaning build artifacts..."
	rm -f src/*.o src/*.d
	rm -f $(TARGET) $(XENLYC) $(RT_LIB) $(RTC_LIB)
	rm -f *.s *.o *.d a.out hello_compiled test_compiled
	rm -f $(BUILD_STAMP)
	@echo "âœ“ Clean complete"

distclean: clean
	find . -name '*~' -delete
	find . -name '*.swp' -delete
	find . -name '.DS_Store' -delete

# â”€â”€â”€ Installation â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

PREFIX  ?= /usr/local
BINDIR  = $(PREFIX)/bin
LIBDIR  = $(PREFIX)/lib
INCDIR  = $(PREFIX)/include
DATADIR = $(PREFIX)/share

install: all
	$(INSTALL) -d $(BINDIR) $(LIBDIR)/xenly $(INCDIR)/xenly $(DATADIR)/doc/xenly
	$(INSTALL) -m 755 $(TARGET) $(XENLYC) $(BINDIR)/
	$(INSTALL) -m 644 $(RT_LIB) $(LIBDIR)/
	# libxly_rtc.a must sit beside xenlyc so it can be found at compile time
	$(INSTALL) -m 644 $(RTC_LIB) $(BINDIR)/
	$(INSTALL) -m 644 $(RTC_LIB) $(LIBDIR)/xenly/
	$(INSTALL) -m 644 src/*.h $(INCDIR)/xenly/
	$(INSTALL) -m 644 *.md $(DATADIR)/doc/xenly/ 2>/dev/null || true
	@echo "âœ“ Installed to $(PREFIX)"
	@echo "  xenlyc:       $(BINDIR)/$(XENLYC)"
	@echo "  libxly_rtc.a: $(BINDIR)/$(RTC_LIB)"
	@echo ""
	@echo "  Try it:"
	@echo "    xenlyc main.xe -o main && ./main"

uninstall:
	rm -f $(BINDIR)/$(TARGET) $(BINDIR)/$(XENLYC)
	rm -f $(BINDIR)/$(RTC_LIB)
	rm -f $(LIBDIR)/$(RT_LIB)
	rm -rf $(LIBDIR)/xenly $(INCDIR)/xenly $(DATADIR)/doc/xenly
	@echo "âœ“ Uninstalled"

# â”€â”€â”€ macOS Universal Binary â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
ifeq ($(UNAME_S),Darwin)
.PHONY: universal clean-objs
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
	@echo "âœ“ Universal binaries created"

clean-objs:
	rm -f src/*.o src/*.d
endif

# â”€â”€â”€ Help â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

help:
	@echo ""
	@echo "Xenly Build System"
	@echo "â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•"
	@echo ""
	@echo "  Quick start (Linux):"
	@echo "    make NO_NATIVE=1           # build everything"
	@echo "    ./xenlyc main.xe -o main   # compile a .xe file"
	@echo "    ./main                     # run the executable"
	@echo ""
	@echo "  Compiler pipeline:"
	@echo "    .xe â†’ lexer â†’ parser â†’ AST â†’ sema â†’ codegen â†’ .s"
	@echo "        â†’ gcc -nostartfiles â€¦ libxly_rtc.a â†’ ELF binary"
	@echo ""
	@echo "  Targets:"
	@echo "    all             Build interpreter, compiler, and both runtimes (default)"
	@echo "    run             Build and run examples/hello.xe (interpreter)"
	@echo "    compile         Build xenlyc + libxly_rtc.a and test-compile hello.xe"
	@echo "    test            Run the core test suite"
	@echo "    test-sys        Run the sys module demo (examples/sys_demo.xe)"
	@echo "    test-http       Run the HTTP server smoke test"
	@echo "    linker-version  Print the built-in xlnk linker version"
	@echo "    format          Auto-format all C source with clang-format"
	@echo "    clean           Remove all build artifacts"
	@echo "    distclean       Clean + remove editor temp files"
	@echo "    install         Install to PREFIX (default: /usr/local)"
	@echo "    uninstall       Remove installed files"
	@echo "    universal       [macOS only] Build x86_64 + arm64 fat binary"
	@echo "    help            Show this message"
	@echo ""
	@echo "  Variables:"
	@echo "    PREFIX=<path>  Install prefix       (default: /usr/local)"
	@echo "    DEBUG=1        Debug build (-O0 -g)"
	@echo "    SANITIZE=1     Enable ASan + UBSan"
	@echo "    NO_NATIVE=1    Disable -march=native (portable build)"
	@echo "    CC=<compiler>  Override compiler     (default: gcc)"
	@echo ""
	@echo "  Examples:"
	@echo "    make NO_NATIVE=1                    # portable build (recommended)"
	@echo "    make                                # native-optimised build"
	@echo "    make test                           # build + run tests"
	@echo "    make install PREFIX=~/.local        # install for current user"
	@echo "    make DEBUG=1                        # debug build"
	@echo "    make SANITIZE=1                     # build with sanitizers"
	@echo ""
