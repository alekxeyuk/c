# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -fstack-protector-strong -D_FORTIFY_SOURCE=3 -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -g

# Linker flags
LDFLAGS = 
LDLIBS  = 

# Include directories
INCLUDES = -I.

# Directories
SRC_DIR = src
BIN_DIR = bin

# Source files
CLIENT_SRC = client.c

# Object files
CLIENT_OBJS    = $(addprefix $(SRC_DIR)/, $(CLIENT_SRC:.c=.o))

# Binary files
CLIENT_BIN    = $(BIN_DIR)/client

# Default target
all: $(CLIENT_BIN)

$(CLIENT_BIN): $(CLIENT_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BIN_DIR):
	mkdir -p $@

# Clean up the object files and executable
clean:
	rm -rf $(BIN_DIR) $(SERVER_OBJS) $(CLIENT_OBJS)

rebuild: clean all

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean rebuild distclean
