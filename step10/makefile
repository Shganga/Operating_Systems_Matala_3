CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -pthread

all: server client

server: server_main.o server.o convex_hull.o reactor_proactor.o
	$(CXX) $(CXXFLAGS) -o server server_main.o server.o convex_hull.o reactor_proactor.o

client: client_main.o client.o
	$(CXX) $(CXXFLAGS) -o client client_main.o client.o

server_main.o: server_main.cpp server.hpp
	$(CXX) $(CXXFLAGS) -c server_main.cpp

server.o: server.cpp server.hpp convex_hull.hpp reactor_proactor.hpp
	$(CXX) $(CXXFLAGS) -c server.cpp

convex_hull.o: convex_hull.cpp convex_hull.hpp
	$(CXX) $(CXXFLAGS) -c convex_hull.cpp

reactor_proactor.o: reactor_proactor.cpp reactor_proactor.hpp
	$(CXX) $(CXXFLAGS) -c reactor_proactor.cpp

client_main.o: client_main.cpp client.hpp
	$(CXX) $(CXXFLAGS) -c client_main.cpp

client.o: client.cpp client.hpp
	$(CXX) $(CXXFLAGS) -c client.cpp

clean:
	rm -f *.o server client

.PHONY: all clean
