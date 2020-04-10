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

	// file I/O
#include <iostream>
#include <fstream>

#define PORT 9000

class Client {
	private:
		const std::string file_name;
		const int thread_num, key;
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
		
		void encrypt(std::string & str);
		void decrypt(std::string & str);

	public:
		Client(std::string & file_name, int & thread_num, int & key);
};
