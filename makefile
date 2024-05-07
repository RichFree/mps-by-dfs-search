SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

TARGET := $(BIN_DIR)/dpt_planarizer
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# compiler and flags
CXX := g++
CPPFLAGS := -Iinclude -MMD -MP
CXXFLAGS := -std=c++14 -Wall -g
LDFLAGS := -lOGDF

build: $(TARGET)

# build target executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS) 

# compile source to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# include dependency files
-include $(DEPS)

.PHONY: clean
clean:
	rm -r $(OBJ_DIR)
