#pragma once

	// networking
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

	// threads
#include <thread>
#include <atomic>
#include <mutex>

#include <memory>
#include <vector>
#include <sstream>
#include <iterator>

	// file I/O
#include <iostream>
#include <fstream>

#define PORT 8000
#define MAXDATA 1024

int sock;
std::mutex mtx;

void * get_in_addr(struct sockaddr * sa);
void bind_socket(std::string & host_name);
uint16_t parse_key(const std::string & authreq);

class Client {
	private:
		const std::string file_name;
		const int thread_num;
		const uint16_t key;
		int serv_port;
		std::string header;
		bool is_dir;

		// file_name, start_byte, chunk_size
		std::vector<std::pair<std::string, std::pair<int, int>>> thread_info;

		bool authenticate();
		void assign_threads();
		void request_copy();
		const std::string make_header();
		void spawn_threads();
		void copy_file();
		
		void crypt(std::string & str);
		void print(const std::string & str);

	public:
		Client(std::string & file_name, int & thread_num, uint16_t & key);
};
