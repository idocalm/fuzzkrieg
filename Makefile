CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I./include -I/usr/local/include -I/opt/homebrew/include
LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -limobiledevice -lplist -lusb-1.0

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(SRC_DIR)/core/*.c) \
       $(wildcard $(SRC_DIR)/device/*.c) \
       $(wildcard $(SRC_DIR)/mutators/*.c) \
       $(wildcard $(SRC_DIR)/testcase/*.c) \
       $(wildcard $(SRC_DIR)/analysis/*.c)

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Binary
BIN = $(BIN_DIR)/fuzzkrieg

# Create directories
$(shell mkdir -p $(OBJ_DIR)/core $(OBJ_DIR)/device $(OBJ_DIR)/mutators $(OBJ_DIR)/testcase $(OBJ_DIR)/analysis $(BIN_DIR))

# Default target
all: $(BIN)

# Link
$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean 