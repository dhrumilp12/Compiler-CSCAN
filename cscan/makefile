# Compilers
CC  := gcc
CXX := g++

CFLAGS   := -Wall -Wextra -Iinclude -g
CXXFLAGS := -Wall -Wextra -Iinclude -g
LDFLAGS  := 
LIBS     := -Lref_lib -lscan

SRC_DIR := src
OBJ_DIR := build

# Binaries
BIN1 := ref_scan
BIN2 := scan

# Sources
SRC1 := $(SRC_DIR)/scan_print.c
SRC2 := $(SRC_DIR)/scan.c $(SRC_DIR)/scan.cpp $(SRC_DIR)/scan_print.c

# Only keep ones that exist
SRC2 := $(wildcard $(SRC_DIR)/scan.c $(SRC_DIR)/scan.cpp) $(SRC_DIR)/scan_print.c

# Objects
OBJ1 := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC1))
OBJ2 := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(filter %.c,$(SRC2))) \
        $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter %.cpp,$(SRC2)))

.PHONY: all clean

all: $(BIN1) $(BIN2)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build ref_scan
$(BIN1): $(OBJ1)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Build scan
$(BIN2): $(OBJ2)
	$(CXX) $(LDFLAGS) -o $@ $^   # use C++ linker if any .cpp is present

# Compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN1) $(BIN2)

