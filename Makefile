# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = -lSDL2

# Directories
SRCDIR = src
BINDIR = bin
OBJDIR = obj

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/chipper

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Build target
$(TARGET): $(OBJECTS) | $(BINDIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Rebuild everything
rebuild: clean all

# Install SDL2 dependencies (Arch Linux)
# Note: You can also just run: sudo pacman -S sdl2
install-deps:
	sudo pacman -S sdl2

# Check if SDL2 is installed
check-deps:
	@echo "Checking for SDL2..."
	@pkg-config --exists sdl2 && echo "SDL2 found" || echo "SDL2 not found - run 'sudo pacman -S sdl2'"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean rebuild install-deps check-deps run