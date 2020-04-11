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
#include <mutex>
#include <atomic>
#include <thread>

	// memory mgmt
#include <memory>

	// other
#include <string>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <iterator>
#include <signal.h>

#define MAX_CLIENTS 30
#define PORT 8000
#define MAXDATA 1024

std::atomic<int> servicer_num(0);
std::mutex mtx;

int server_sock;

uint16_t make_rand();
void print(const std::string & str);
std::pair<std::string, std::string> get_host_info();
void handler(int s);
void bind_socket();
