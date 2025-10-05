# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -fstack-protector-strong -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -g

# Linker flags
LDFLAGS = 
LDLIBS  = -lpthread

# Include directories
INCLUDES = -I.

# Directories
SRC_DIR = src
BIN_DIR = bin

# Source files
EPOLL_SRC = main.c utils.c driver.c

# Object files
EPOLL_OBJS    = $(addprefix $(SRC_DIR)/, $(EPOLL_SRC:.c=.o))

# Binary files
EPOLL_BIN    = $(BIN_DIR)/main

# Default target
all: $(SELECT_BIN) ${POLL_BIN} ${EPOLL_BIN}

$(SELECT_BIN): $(SELECT_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(POLL_BIN): $(POLL_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(EPOLL_BIN): $(EPOLL_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BIN_DIR):
	mkdir -p $@

# Clean up the object files and executable
clean:
	rm -rf $(BIN_DIR) $(EPOLL_OBJS)

rebuild: clean all

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean rebuild distclean
