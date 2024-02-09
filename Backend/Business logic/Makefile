CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
EXECUTABLE = main

# List all the source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Generate the object file names
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Default target
all: directories $(BIN_DIR)/$(EXECUTABLE)

directories: $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

# Rule to compile the object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link the executable
$(BIN_DIR)/$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean the object files and the executable
# clean:
# 	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/$(EXECUTABLE)
