# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -fstack-protector-strong -D_FORTIFY_SOURCE=3 -fsanitize=address -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -g

# Source files
SRCS = main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable
TARGET = task_9_p1

# Include directories
INCLUDES = -I.

# Linker flags
LDFLAGS =

# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
        $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile the source files into object files
%.o: %.c
        $(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up the object files and executable
clean:
        rm -f $(OBJS) $(TARGET) output.txt

# Clean up everything, including the Makefile
distclean: clean
        rm -f Makefile

.PHONY: all clean distclean