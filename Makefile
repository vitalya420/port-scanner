# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -O2
DEBUG_FLAGS = -g -DDEBUG

# Source files
SOURCES = main.c ip.c tcp.c
HEADERS = ip.h tcp.h

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Binary name
TARGET = scanner

# Default target
all: $(TARGET)

# Debug build
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Install (requires root for setuid)
install: $(TARGET)
	install -m 4755 $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Check if running as root (for raw sockets)
check-root:
	@if [ "$$(id -u)" != "0" ]; then \
		echo "This program must be run as root for raw sockets" >&2; \
		exit 1; \
	fi

# Run the program (requires root)
run: $(TARGET) check-root
	./$(TARGET)

# Format source code using clang-format
format:
	clang-format -i $(SOURCES) $(HEADERS)

# Generate tags for code navigation
tags:
	ctags -R .

# Show help
help:
	@echo "Available targets:"
	@echo "  all        - Build the scanner (default)"
	@echo "  debug      - Build with debug symbols and DEBUG defined"
	@echo "  clean      - Remove build files"
	@echo "  install    - Install the scanner to /usr/local/bin"
	@echo "  uninstall  - Remove the scanner from /usr/local/bin"
	@echo "  run        - Build and run the scanner (requires root)"
	@echo "  format     - Format source code using clang-format"
	@echo "  tags       - Generate tags for code navigation"
	@echo "  help       - Show this help message"

.PHONY: all debug clean install uninstall check-root run format tags help