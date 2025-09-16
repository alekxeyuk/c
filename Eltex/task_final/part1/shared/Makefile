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
PLUGIN_BIN_DIR = bin
INSTALL_DIR = ../client/bin

# Source files
PLUGINS_SRC = UDP.c

# Object files
PLUGINS_OBJS    = $(addprefix $(SRC_DIR)/, $(PLUGINS_SRC:.c=.o))

# Binary files
PLUGIN_LIBS    = $(addprefix $(PLUGIN_BIN_DIR)/, $(PLUGINS_SRC:%.c=lib%.so))
INSTALL_AS     = libclient.so

# Default target
all: $(PLUGIN_LIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/client_lib.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(PLUGIN_BIN_DIR)/lib%.so: $(SRC_DIR)/%.o | $(PLUGIN_BIN_DIR)
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $<

$(PLUGIN_BIN_DIR):
	mkdir -p $@

install:
	@if [ -z "$(PLUGIN_LIBS)" ]; then \
		echo "No libs found."; \
		exit 1; \
	fi; \
	\
	echo "Libs:"; \
	i=1; \
	for lib in $(PLUGIN_LIBS); do \
		echo "$${i}: $${lib}"; \
		i=$$((i+1)); \
	done; \
	\
	echo ""; \
	echo -n "Select lib (1-$(words $(PLUGIN_LIBS))): "; \
	read choice; \
	\
	i=1; \
	selected=""; \
	for lib in $(PLUGIN_LIBS); do \
		if [ "$$i" = "$$choice" ]; then \
			selected="$$lib"; \
			break; \
		fi; \
		i=$$((i+1)); \
	done; \
	\
	if [ -z "$$selected" ]; then \
		echo "Wrong selection."; \
		exit 1; \
	fi; \
	\
	echo "Selected: $$selected"; \
	cp -v "$$selected" "${INSTALL_DIR}/${INSTALL_AS}"; \
	\
	if [ $$? -eq 0 ]; then \
		echo "Installation successful!"; \
	else \
		echo "Failed to copy $$selected"; \
		exit 1; \
	fi

# Clean up the object files and executable
clean:
	rm -rf $(PLUGIN_BIN_DIR) $(PLUGINS_OBJS)

rebuild: clean all

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean rebuild distclean
