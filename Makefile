CXX=g++
CPPFLAGS=-std=c++11 -pthread -Wall
OBJECTS := $(patsubst %.cc,%.o,$(wildcard *.cc))

sim: $(OBJECTS)
	$(CXX) $(CPPFLAGS) -o agc $(OBJECTS)

clean:
	rm -f $(OBJECTS)