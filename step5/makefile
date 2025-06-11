CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

SRCS = main.cpp reactor.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = reactor.hpp

TARGET = main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
