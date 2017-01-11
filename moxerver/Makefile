# target name
TARGET = moxerver

# directory for build results
BUILDDIR = build.dir

# ==============================================================================

# add include directories
INCDIRS = -I.
# add library directories
LIBDIRS = -L.
# list used libraries
#LIBS = -lm
LIBS = -lpthread

# ==============================================================================

# compiler and flags
CC = gcc
CFLAGS = -Wall $(INCDIRS) $(LIBDIRS) $(LIBS)

# ==============================================================================

# build everything in a dedicated directory $(OUTDIR)

# objects are .o files created from all .c files in the directory (same name)
OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))
# headers are all .h files in the directory
HEADERS = $(wildcard *.h)

# all objects are built from their .c files in the directory
$(BUILDDIR)/%.o: %.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# target is built from all object files
$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $@

# ==============================================================================

# supported make options (clean, install...)
.PHONY: default all clean

# all calls all other options
all: default

# default builds target
default: $(BUILDDIR)/$(TARGET)

# clean removes object files and target (ignore errors with "-" before commands)
clean:
	-rm -rf $(BUILDDIR)
