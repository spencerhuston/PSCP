all: pscp_server pscp

pscp_server: server.hpp servicer.hpp dispatcher.hpp
	g++ -g -std=c++17 -pthread server.cpp servicer.cpp dispatcher.cpp -o pscp_server -lstdc++fs

pscp: client.hpp
	g++ -g -std=c++17 -pthread client.cpp -o pscp

clean: 
	rm pscp_server pscp
