CXX       = g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -O2 -pthread
TARGET    = oopdassign4
SOURCES   = main.cpp
HEADERS   = student.hpp database.hpp
OBJECTS   = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
