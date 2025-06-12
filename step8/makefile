CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -pthread

all: proactor_test

proactor_test: main.o reactor_proactor.o
	$(CXX) $(CXXFLAGS) -o proactor_test main.o reactor_proactor.o

main.o: main.cpp reactor_proactor.hpp
	$(CXX) $(CXXFLAGS) -c main.cpp

reactor_proactor.o: reactor_proactor.cpp reactor_proactor.hpp
	$(CXX) $(CXXFLAGS) -c reactor_proactor.cpp

clean:
	rm -f *.o proactor_test

.PHONY: all clean