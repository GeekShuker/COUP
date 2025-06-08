#meirshuker159@gmail.com
# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./include
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lstdc++fs

# Directories
SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
INCLUDE_DIR = include

# Create build directory
$(shell mkdir -p $(BUILD_DIR))

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Test files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executables
MAIN_EXEC = $(BUILD_DIR)/game
TEST_EXEC = $(BUILD_DIR)/tests

# Main target: build and run the GUI
Main: $(MAIN_EXEC)
	./$(MAIN_EXEC)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test files
$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link main executable
$(MAIN_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# Link test executable
$(TEST_EXEC): $(TEST_OBJS) $(filter-out $(BUILD_DIR)/main.o,$(OBJS))
	$(CXX) $(TEST_OBJS) $(filter-out $(BUILD_DIR)/main.o,$(OBJS)) -o $@ $(LDFLAGS)

# Test target: build and run tests
test: $(TEST_EXEC)
	./$(TEST_EXEC)

# Valgrind target: run GUI under valgrind
valgrind: $(MAIN_EXEC)
	valgrind --leak-check=full ./$(MAIN_EXEC)

# Clean target: only clean build directory
clean:
	rm -rf $(BUILD_DIR)/*

# Phony targets
.PHONY: Main test valgrind clean

# Help target
help:
	@echo "Available targets:"
	@echo "  Main      - Build and run the GUI"
	@echo "  test      - Build and run tests"
	@echo "  valgrind  - Run GUI under valgrind for memory leak check"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help message" 