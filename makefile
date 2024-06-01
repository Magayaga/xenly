CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
LIB_DIR = lib

MAIN_SRC = $(SRC_DIR)/xenly.c
LIB_SRC = $(SRC_DIR)/xenly_math.c

MAIN_OBJ = $(OBJ_DIR)/xenly.o
LIB_OBJ = $(OBJ_DIR)/xenly_math.o

MAIN_BIN = $(BIN_DIR)/xenly
LIB_SO = $(LIB_DIR)/math.so

all: $(MAIN_BIN) $(LIB_SO)

$(MAIN_BIN): $(MAIN_OBJ) $(LIB_SO)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) -ldl

$(LIB_SO): $(LIB_OBJ)
	@mkdir -p $(LIB_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LIB_DIR)

.PHONY: all clean

