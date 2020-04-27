#ifndef SERVICER_H
#define SERVICER_H

#include "dispatcher.hpp"

uint16_t make_rand();

class Servicer {
	private:
		std::string dispatch_header;
		int thread_num;
		const int sock, serv_num;
		const uint16_t key;
		
		void service();
		bool authenticate_user();
		bool check_file_dir();
		void get_header();
		void start_thread_dispatch();

		std::string s_recv();
		void s_send(std::string & str);

	public:
		Servicer(int & sock);
};

#endif
