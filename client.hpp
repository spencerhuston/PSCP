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

#include <random>
#include <memory>
#include <vector>
#include <sstream>
#include <iterator>
#include <algorithm>

	// file I/O
#include <iostream>
#include <fstream>

#define PORT 8000
#define MAXDATA 1024
#define HEADER_SIZE 32

int sock;
std::mutex mtx;
uint16_t key;

void print(const std::string & str);

void * get_in_addr(struct sockaddr * sa);
void bind_socket(std::string & host_name);

uint16_t parse_key(const std::string & authreq);

std::string recv(const int & sock);
std::string recv_str(const int & sock);
void send_str(const int & sock, std::string & str);
void crypt_pscp(std::string & str);

class Client {
	private:
		const std::string file_name;
		int thread_num;
		int serv_port;
		std::string header;
		bool is_dir;

		struct thread_info {
			std::string file_name;
			int start_byte, chunk_size;
		};
		std::vector<std::vector<thread_info>> thread_assignments;

		std::vector<std::string> authenticate();
		void assign_threads(const std::vector<std::string> & file_info);
		bool request_copy();
		void make_header();
		void spawn_threads();
		void copy_file();		

	public:
		Client(std::string & file_name, int & thread_num);
};
