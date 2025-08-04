# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wnull-dereference -fstack-protector-strong -D_FORTIFY_SOURCE=3 -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -g

# Source files
SRCS_REC = receiver.c
SRCS_PUB = producer.c

# Object files
OBJS_REC = $(SRCS_REC:.c=.o)
OBJS_PUB = $(SRCS_PUB:.c=.o)

# Executable
TARGET_REC = REC
TARGET_PUB = PUB

# Include directories
INCLUDES = -I.

# Linker flags
LDFLAGS =

LDLIBS = 
# Default target
all: $(TARGET_REC) $(TARGET_PUB)

# Link the object files to create the executable
$(TARGET_REC): $(OBJS_REC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(TARGET_PUB): $(OBJS_PUB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# Compile the source files into object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up the object files and executable
clean:
	rm -f $(OBJS_REC) $(OBJS_PUB) $(TARGET_REC) $(TARGET_PUB)

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean distclean
