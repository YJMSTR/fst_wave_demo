CXX = g++
CXXFLAGS = -std=c++11 -I. -I./include -I../include -Wall -O2
LDFLAGS = -lfl -ldl -rdynamic

# Source files
SRCS = parse_gcd.cpp PNode.cpp plib.cpp lex.yy.cpp debug.cpp syntax.cpp
OBJS = $(SRCS:.cpp=.o)

# Target file
TARGET = parse_gcd

# Bison and Flex generated files
BISON_SRC = syntax.y
BISON_OUT = syntax.cpp
BISON_HEADER = syntax.hpp

FLEX_SRC = lexical.l
FLEX_OUT = lex.yy.cpp

# All generated files
GENERATED = $(BISON_OUT) $(BISON_HEADER) $(FLEX_OUT)

# Test directory
TEST_DIR = ../firrtl_fst_demo/test_run_dir

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Bison rules
$(BISON_OUT) $(BISON_HEADER): $(BISON_SRC)
	bison -d -o $(BISON_OUT) $(BISON_SRC)

# Flex rules
$(FLEX_OUT): $(FLEX_SRC) $(BISON_HEADER)
	flex -o $(FLEX_OUT) $(FLEX_SRC)

# Ensure necessary files are generated before compilation
parse_gcd.o: $(GENERATED)

# Test rules
test: $(TARGET)
	@echo "Parsing FIR files in $(TEST_DIR)..."
	@for file in $(TEST_DIR)/*.fir; do \
		echo "\nParsing $$file..."; \
		./$(TARGET) $$file; \
	done

clean:
	rm -f $(OBJS) $(TARGET) $(GENERATED)
	find . -name "*.o" -type f -delete 