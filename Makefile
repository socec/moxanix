# target name
TARGET = moxerver

# special include directiories
INCDIRS = -I.
# special library directories
LIBDIRS = -L
# used libraries
#LIBS = -lm
LIBS = -lpthread

# compiler and flags
CC = gcc
CFLAGS = -Wall $(INCDIRS) $(LIBDIRS)

# objects are .o files created from all .c files in the directory (same name)
OBJECTS = $(patsubst %.c,%.o,$(wildcard *.c))
# headers are all .h files in the directory
HEADERS = $(wildcard *.h)

# all objects are built from their .c files and all headers in the directory
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@

# target is built from all object files
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) $(OBJECTS) -o $@


# support for default, clean and all options
.PHONY: default all clean

# all calls all other options
all: default

# default builds target
default: $(TARGET)

# clean removec object files and target
clean:
	-rm -f *.o
	-rm -f $(TARGET)

