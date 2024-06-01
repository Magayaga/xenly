# Compiler
CC = gcc

# Source files
SRCS = src/xenly.c src/xenly_math.c

# Executable name
EXEC = xenly

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Linker flags
LDFLAGS = -lm

# Targets and rules
.PHONY: all clean

all: $(EXEC)

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC) $(LDFLAGS)

clean:
	rm -f $(EXEC)
