#ifndef SERVICER_H
#define SERVICER_H

#include "dispatcher.hpp"

uint16_t make_rand();

class Servicer {
	private:
		std::string dispatch_header;
		const int sock, serv_num;
		const uint16_t key;
		
		void service();
		bool authenticate_user();
		bool check_file_dir();
		void send_file_info(const std::string & fp);
		void get_header();
		void start_thread_dispatch();
	
		void crypt(std::string & str);

	public:
		Servicer(int & sock);
};

#endif
