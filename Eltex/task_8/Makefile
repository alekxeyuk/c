# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -fstack-protector-strong -D_FORTIFY_SOURCE=3 -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -g

# Linker flags
LDFLAGS = -L. -Wl,-rpath,'$$ORIGIN'
LDLIBS = -ldl

# Include directories
INCLUDES = -I.

# Directories
SRC_DIR        = src
BIN_DIR        = bin
PLUGIN_DIR     = plugins
PLUGIN_BIN_DIR = $(BIN_DIR)/$(PLUGIN_DIR)
PLUGIN_SRC_DIR = $(SRC_DIR)/$(PLUGIN_DIR)

# Source files
PLUGIN_SRC = add.c sub.c mul.c div.c
CALC_SRC   = main.c helper.c

# Object files
PLUGIN_OBJS = $(PLUGIN_SRC:.c=.o)
CALC_OBJ    = $(addprefix $(SRC_DIR)/, $(CALC_SRC:.c=.o))

# Binary files
PLUGIN_LIBS = $(PLUGIN_SRC:%.c=$(PLUGIN_BIN_DIR)/lib%.so)
CALC_BIN    = $(BIN_DIR)/calculator

# Default target
all: $(CALC_BIN) $(PLUGIN_LIBS)

$(CALC_BIN): $(CALC_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BIN_DIR):
	mkdir -p $@

$(PLUGIN_BIN_DIR)/lib%.so: $(PLUGIN_SRC_DIR)/%.c $(PLUGIN_SRC_DIR)/plugin.h | $(PLUGIN_BIN_DIR)
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $<

$(PLUGIN_BIN_DIR):
	mkdir -p $@

# Clean up the object files and executable
clean:
	rm -rf $(BIN_DIR) $(CALC_OBJ)

rebuild: clean all

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean rebuild distclean
