
```

#------------------------------------------
# Makefile for "obj_to_md2".
#------------------------------------------

EXECUTABLE = obj_to_md2

#------------------------------------------
# Variables for my code.
#------------------------------------------

SOURCES = main.cpp

#------------------------------------------
# Create common variables.
#------------------------------------------

INCLUDE_DIRS =
LIBRARY_DIRS =
LIBRARIES =
DEFINES =
OPTIMIZATION = -g
#OPTIMIZATION = -O3

#------------------------------------------
# Compile based on common variables.
#------------------------------------------

OBJECTS = $(SOURCES:.cpp=.o)
CFLAGS = $(OPTIMIZATION) -Wall $(DEFINES) $(INCLUDE_DIRS)
LDFLAGS = $(OPTIMIZATION) -Wall $(LIBRARY_DIRS) $(LIBRARIES)
COMPILER = g++

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(COMPILER) $(LDFLAGS) $(OBJECTS) -o $@
.cpp.o:
	$(COMPILER) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS)
clobber: clean
	rm -f $(EXECUTABLE)

#------------------------------------------

```