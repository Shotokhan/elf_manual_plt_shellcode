CC := gcc
CFLAGS := -nostdlib -ffunction-sections -Wl,--gc-sections
LDFLAGS := -nostartfiles -T linker_script.ld

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

# Get all source files in the directory (including both .c and .S files)
SRC_FILES := $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*.S)

# Create object file names for all source files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(filter %.c, $(SRC_FILES))) \
             $(patsubst $(SRC_DIR)/%.S, $(OBJ_DIR)/%.o, $(filter %.S, $(SRC_FILES)))

# Specify the top-level file (default is "main.c")
TOP_LEVEL ?= main.c

# Specify the final executable name
TARGET := $(BIN_DIR)/$(basename $(notdir $(TOP_LEVEL)))

# Specify the tester path
TEST := test/loader.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	$(CC) --version | head -n1 > $(BIN_DIR)/version.txt
	objcopy --dump-section .text=$(BIN_DIR)/shellcode $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

inspect: $(BIN_DIR)/shellcode
	objdump -D $(TARGET)
	objdump -M intel,x86-64 -b binary -D -mi386 $(BIN_DIR)/shellcode

loader: $(BIN_DIR)/shellcode
	$(CC) $(TEST) -o $(BIN_DIR)/loader
