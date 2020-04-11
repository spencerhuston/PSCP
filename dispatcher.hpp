#pragma once

	// authentication
#include <sys/types.h>
#include <pwd.h>

	// networking
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

	// key generation
#include <random>

	// threads
#include <mutex>
#include <thread>
#include <atomic>

	// file I/O
#include <string>
#include <fstream>

	// other
#include <vector>
#include <iostream>
#include <sstream>
#include <iterator>
#include <cstdint>

#define MAXDATA 1024

extern std::mutex mtx;
extern std::atomic<int> servicer_num;
void print(const std::string & str);

class Dispatcher {
	private:
		const std::string file_name, header;
		const int sock, chunk_size, key, port;
		const char start_byte;
		
		void send_file_data();

		void crypt(std::string & str);

	public:
		Dispatcher(const std::string & file_name,
			   const std::string & header,
			   int & sock,
			   int & port, 
			   int & chunk_size, 
			   int & key, 
			   char & start_byte);
};
