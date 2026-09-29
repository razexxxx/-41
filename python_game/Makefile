# Makefile проекта "Питон" (C++17)
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Isrc
SRCDIR   := src
OBJDIR   := build
BIN      := python_game

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(OBJDIR) $(BIN)

.PHONY: all run clean
