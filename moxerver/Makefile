# target names
TARGET_BINARY = moxerver

# ==============================================================================

# directory for build results
BUILDDIR = build.dir
# installation root
INSTALLDIR = install.dir
# prefix for /bin directory
BIN_PREFIX = /usr

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

# build everything in a dedicated directory $(BUILDDIR)

# objects are .o files created from all .c files in the directory (same name)
OBJECTS = $(patsubst %.c, $(BUILDDIR)/%.o, $(wildcard *.c))
# headers are all .h files in the directory
HEADERS = $(wildcard *.h)

# all objects are built from their .c files in the directory
$(BUILDDIR)/%.o: %.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# target is built from all object files
$(BUILDDIR)/$(TARGET_BINARY): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $@

# ==============================================================================

# supported make options (clean, install...)
.PHONY: all default install clean

# all calls all other options
all: default install

# default builds target
default: $(BUILDDIR)/$(TARGET_BINARY)

# install target
install: default
	mkdir -p $(INSTALLDIR)/$(BIN_PREFIX)/bin
	cp $(BUILDDIR)/$(TARGET_BINARY) $(INSTALLDIR)/$(BIN_PREFIX)/bin

# clean removes object files and target (ignore errors with "-" before commands)
clean:
	-rm -rf $(BUILDDIR)
	-rm -rf $(INSTALLDIR)
