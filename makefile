# ─── Xenly Language Makefile ─────────────────────────────────────────────────
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11 -D_POSIX_C_SOURCE=200809L
LDFLAGS = -lm

# ─── Interpreter ─────────────────────────────────────────────────────────────
TARGET  = xenly
INTERP_SRCS = src/main.c src/lexer.c src/ast.c src/parser.c \
              src/interpreter.c src/modules.c src/typecheck.c
INTERP_OBJS = $(INTERP_SRCS:.c=.o)

# ─── Native Compiler ─────────────────────────────────────────────────────────
XENLYC  = xenlyc

# xenlyc driver + frontend (shares lexer / parser / ast with interpreter)
XENLYC_SRCS = src/xenlyc_main.c src/lexer.c src/ast.c src/parser.c src/codegen.c
XENLYC_OBJS = $(XENLYC_SRCS:.c=.o)

# Runtime library: everything a compiled binary needs at run-time.
# xly_rt.c  — value ops, print, module dispatch shim
# modules.c — the 100 stdlib native functions (shared with interpreter)
RT_SRCS  = src/xly_rt.c src/modules.c
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

.PHONY: all run compile clean