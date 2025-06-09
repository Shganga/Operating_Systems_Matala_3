CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

SRCS = main.cpp convex_hull.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = convex_hull.hpp

TARGET = main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)
