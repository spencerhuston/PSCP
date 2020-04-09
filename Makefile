all: pscp_server

pscp_server: server.hpp
	g++ -g -std=c++11 -pthread server.cpp -o pscp_server

clean: 
	rm pscp_server
