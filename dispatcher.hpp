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
#include <filesystem>

	// other
#include <vector>
#include <iostream>
#include <sstream>
#include <iterator>
#include <cstdint>
#include <sys/stat.h>
#include <algorithm>

#define MAXDATA 1024

extern std::mutex mtx;
extern std::atomic<int> servicer_num;

std::string recv(const int & sock);
std::string recv_str(const int & sock, const uint16_t & key);
void send_str(const int & sock, std::string & str, const uint16_t & key);

void crypt_pscp(std::string & str, const uint16_t & key);

void print(const std::string & str);

class Dispatcher {
	private:
		const std::string file_name, header;
		const int sock, chunk_size, port;
		const uint16_t key;
		const char start_byte;
		
		void send_file_data();

		std::string d_recv();
		void d_send(std::string & str);

	public:
		Dispatcher(const std::string & file_name,
			   const std::string & header,
			   int & sock,
			   int & chunk_size, 
			   int & port, 
			   uint16_t & key, 
			   char & start_byte);
};
