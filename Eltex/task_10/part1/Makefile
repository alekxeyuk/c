# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wnull-dereference -fstack-protector-strong -D_FORTIFY_SOURCE=3 -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -g

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable
TARGET = solution

# Include directories
INCLUDES = -I.

# Linker flags
LDFLAGS =

LDLIBS = 
# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# Compile the source files into object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up the object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean distclean
