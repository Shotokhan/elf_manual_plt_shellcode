CC := gcc
CFLAGS :=
LDFLAGS :=

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Get all source files in the directory (excluding the top-level file)
SRC_FILES := $(filter-out $(SRC_DIR)/$(TOP_LEVEL), $(wildcard $(SRC_DIR)/*.c))

# Create object file names for all source files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Specify the top-level file (default is "main.c")
TOP_LEVEL ?= main.c

# Specify the final executable name
TARGET := $(BIN_DIR)/$(basename $(notdir $(TOP_LEVEL)))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
