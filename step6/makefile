CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

SERVER_SRCS = server_main.cpp server_reactor.cpp convex_hull.cpp reactor.cpp
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)
SERVER_DEPS = server_reactor.hpp convex_hull.hpp reactor.hpp
SERVER_TARGET = server_reactor

CLIENT_SRCS = client_main.cpp client.cpp
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
CLIENT_DEPS = client.hpp
CLIENT_TARGET = client

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(SERVER_DEPS) $(CLIENT_DEPS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS) $(SERVER_TARGET) $(CLIENT_TARGET)