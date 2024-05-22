# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11

# Source files
SRCS = main.cpp optab.cpp parse.cpp types.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = asxe

# Default target
all: $(TARGET)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link object files into the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@
	rm -f *.o

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)
