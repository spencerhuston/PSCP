#pragma once

#define _DEFAULT_SOURCE

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
#include <cstring>
#include <signal.h>

	// key generation
#include <time.h>

	// threads
#include <mutex>
#include <atomic>
#include <thread>

	// memory mgmt
#include <memory>

	// file I/O
#include <string>
#include <fstream>
#include <cstdint>
#include <iostream>

#define MAX_CLIENTS 30
#define PORT 8000

std::atomic<int> servicer_num(0);
std::mutex mtx;

int make_rand();
void print(const std::string & str);

class Servicer {
	private:
		std::string dispatch_header;
		const int sock, key, serv_num;
		
		void service();
		bool authenticate_user();
		bool check_file_dir(const std::string & fp);
		void send_file_info(const std::string & fp);
		void get_header();
		void start_thread_dispatch();
	
		void encrypt(std::string & str);
		void decrypt(std::string & str);

	public:
		Servicer(int & sock);
};

class Dispatcher {
	private:
		const std::string file_name, header;
		const int sock, chunk_size, key;
		const char start_byte;
		
		void send_file_data();

		void encrypt(std::string & str);
		void decrypt(std::string & str);

	public:
		Dispatcher(const std::string & file_name,
			   const std::string & header,
			   int & sock, 
			   int & chunk_size, 
			   int & key, 
			   char & start_byte);
};
