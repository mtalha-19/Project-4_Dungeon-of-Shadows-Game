CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

.PHONY: all terminal server clean run-gui

all: terminal server

terminal: adventure

server: adventure-server

adventure: main.cpp game.hpp
	$(CXX) $(CXXFLAGS) -o adventure main.cpp

adventure-server: server.cpp game.hpp
	$(CXX) $(CXXFLAGS) -o adventure-server server.cpp

run-gui: server
	./adventure-server

clean:
	rm -f adventure adventure-server
