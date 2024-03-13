# Compiler and flags:
CC         = g++
CFLAGS     = -c -O3 -MMD -MP -g
LDFLAGS    = 
INCLUDES   = -Iinclude -Iinclude/rendering -Iinclude/mocap
EXECUTABLE = previz

# Source and object files:
SOURCES    = previz.cpp skeleton.cpp motion.cpp displaySkeleton.cpp
OBJECTS    = $(SOURCES:.cpp=.o)
DEPS       = $(OBJECTS:.o=.d)

# Main target:
all: $(SOURCES) $(EXECUTABLE)

# Linking the executable:
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Compiling the source files:
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

# Include the automatically generated dependency files:
-include $(DEPS)

# Cleaning up:
clean:
	rm -f *.o $(EXECUTABLE) $(DEPS)
