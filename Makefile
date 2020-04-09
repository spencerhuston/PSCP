all: pscp_server pscp

pscp_server: server.hpp
	g++ -g -std=c++11 -pthread server.cpp -o pscp_server

pscp: client.hpp
	g++ -g -std=c++11 -pthread client.cpp -o pscp

clean: 
	rm pscp_server pscp
