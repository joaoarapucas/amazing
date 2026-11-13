# amazing - biblioteca de geração de labirintos
#
#   make          compila a biblioteca (build/libamazing.a) e o demo
#   make lib      apenas a biblioteca
#   make demo     executável de demonstração (build/maze)
#   make run      compila e executa o demo
#   make test     compila e executa os testes
#
# Projetos que usam a biblioteca podem repassar CXX, CXXFLAGS e AR:
#   $(MAKE) -C caminho/amazing lib CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)"

CXX      ?= g++
AR       ?= ar
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
CPPFLAGS += -Iinclude -MMD -MP

BUILD := build
LIB   := $(BUILD)/libamazing.a
DEMO  := $(BUILD)/maze
TEST  := $(BUILD)/amazing_test

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:src/%.cpp=$(BUILD)/%.o)
DEPS := $(OBJS:.o=.d) $(BUILD)/main.d $(BUILD)/amazing_test.d

.PHONY: all lib demo run test clean

all: lib demo

lib: $(LIB)

demo: $(DEMO)

$(LIB): $(OBJS)
	$(AR) rcs $@ $^

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(DEMO): app/main.cpp $(LIB) | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MF $(BUILD)/main.d $< $(LIB) -o $@

$(TEST): tests/amazing_test.cpp $(LIB) | $(BUILD)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MF $(BUILD)/amazing_test.d $< $(LIB) -o $@

$(BUILD):
	mkdir -p $@

run: $(DEMO)
	@./$(DEMO)

test: $(TEST)
	./$(TEST)

clean:
	rm -rf $(BUILD)

-include $(DEPS)
