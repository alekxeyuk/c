# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

# Source files
SRCS = abonent.c list.c utils.c main.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable
TARGET = task_6

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
	rm -f $(OBJS) $(TARGET)

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean distclean
