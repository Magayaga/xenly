#
# XENLY - high-level and general-purpose programming language
# created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
# 
# It is initially written in C programming language.
#

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Source directory
SRC_DIR = src

# Source files for the main program and shared library
MAIN_SRC = $(SRC_DIR)/xenly.c $(SRC_DIR)/color.c $(SRC_DIR)/error.c $(SRC_DIR)/print_info.c $(SRC_DIR)/project.c
LIB_SRC = $(SRC_DIR)/xenly_math.c
LIB_BIN_SRC = $(SRC_DIR)/xenly_binary_math.c

# Object files corresponding to the source files
MAIN_OBJ = $(MAIN_SRC:.c=.o)
LIB_OBJ = $(LIB_SRC:.c=.o)

# Output binary and shared library names
MAIN_BIN = xenly
LIB_SO = math.so
LIB_BIN_SO = binary_math.so

# Default target: build the main binary and then clean object files
all: $(MAIN_BIN) clean_objs
	@echo "Adding current directory to LD_LIBRARY_PATH"
	@export LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:.

# Link the main binary from object files and shared library
$(MAIN_BIN): $(MAIN_OBJ) $(LIB_SO) $(LIB_BIN_SO)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) -ldl -lm

# Create the shared library from its object file
$(LIB_SO): $(LIB_OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^ -lm
	@chmod +rx $@

# Create the shared library from its object file
$(LIB_BIN_SO): $(LIB_OBJ)
	$(CC) $(CFLAGS) -shared -o $@ $^ -lm
	@chmod +rx $@

# Compile source files to object files in the same directory
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

# Remove object files after building the main binary and shared library
clean_objs:
	rm -f $(SRC_DIR)/*.o

# Clean all generated files: object files, binary, and shared library
clean:
	rm -f $(SRC_DIR)/*.o $(MAIN_BIN) $(LIB_SO)

# Mark these targets as not corresponding to actual files
.PHONY: all clean clean_objs