# Compiler and flags
CC = gcc
AR = ar
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wformat=2 -Wundef -Wfloat-equal -Wwrite-strings -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -fstack-protector-strong -D_FORTIFY_SOURCE=3 -pedantic -Wunused-variable -Wunused-function -Wunused-parameter -Wold-style-definition -Wmissing-declarations -Wmissing-format-attribute -g
ARFLAGS = rcs

LIBNAME = libcalc.so

# Source files
LIBSRCS = add.c sub.c mul.c div.c
APPSRCS = main.c

# Object files
LIBOBJS = $(LIBSRCS:.c=.o)
APPOBJS = $(APPSRCS:.c=.o)

# Executable
TARGET = calculator

# Include directories
INCLUDES = -I.

# Linker flags
LDFLAGS = -L. -Wl,-rpath,'$$ORIGIN'

LDLIBS = -lcalc
# Default target
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(APPOBJS) $(LIBNAME)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(LIBNAME): $(LIBOBJS)
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $^

# Compile the source files into object files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean up the object files and executable
clean:
	rm -f $(LIBOBJS) $(APPOBJS) $(LIBNAME) $(TARGET)

rebuild: clean all

# Clean up everything, including the Makefile
distclean: clean
	rm -f Makefile

.PHONY: all clean rebuild distclean
