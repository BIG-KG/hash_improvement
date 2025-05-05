INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj

CXX = g++
CXXFLAGS = -g -msse2 -msse4.1 -mavx -msse4.2 -I$(INCLUDE_DIR) -O3

SRCS     = $(wildcard $(SRC_DIR)/*.cpp)
SRCS_ASM = $(wildcard $(SRC_DIR)/*.s)
OBJS     = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_ASM = $(patsubst $(SRC_DIR)/%.s,   $(OBJ_DIR)/%.o, $(SRCS))

TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	g++ -g -msse2 -msse4.1 -mavx -msse4.2 -O3 -c -I$(INCLUDE_DIR) ./src_O3/hash_cmp_strings.cpp -o ./obj/hash_cmp_strings.o
	nasm -f elf64 -g ./src/hash_funcktion.s -o ./obj/hash_funcktion.o
	$(CXX) $(CXXFLAGS) -o $@  $^ ./obj/hash_funcktion.o ./obj/hash_cmp_strings.o

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
