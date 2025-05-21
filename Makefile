INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj

CXX = g++
CXXFLAGS = -g -msse2 -msse4.1 -mavx2 -mavx -msse4.2 -I$(INCLUDE_DIR) -O3 -no-pie
SRCS     = $(wildcard $(SRC_DIR)/*.cpp)
SRCS_ASM = $(wildcard $(SRC_DIR)/*.s)
OBJS     = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_ASM = $(patsubst $(SRC_DIR)/%.s,   $(OBJ_DIR)/%.o, $(SRCS))

TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	nasm -f elf64 -g -O3 ./src/hash_funcktion.s -o ./obj/hash_funcktion.o
	$(CXX) $(CXXFLAGS) -o $@  $^ ./obj/hash_funcktion.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
