#include "dispatcher.hpp"

std::string 
recv(const int & sock) {
	char recv_buff[MAXDATA];
	int byte_num;
	if ((byte_num = recv(sock, recv_buff, MAXDATA - 1, 0)) == -1) {
		perror("recv");
		return "";
	}
	recv_buff[byte_num] = '\0';

	return std::string(recv_buff);
}

std::string
recv_str(const int & sock, const uint16_t & key) {
	std::string res = recv(sock);
	std::transform(res.begin(), res.end(), res.begin(), 
			[&key](char c) { return c ^ key; });  
	return res;
}

void
send_str(const int & sock, std::string & str, const uint16_t & key) {
	std::transform(str.begin(), str.end(), str.begin(), 
			[&key](char c) { return c ^ key; });  
	send(sock, str.c_str(), str.length(), 0);
}

Dispatcher::
Dispatcher(const std::string & file_name, int & sock, 
	   int & chunk_size, uint16_t & key, int & start_byte) : 
file_name(file_name), sock(sock), chunk_size(chunk_size),
key(key), start_byte(start_byte) {
	send_file_data();

	close(sock);
}

// copy files over connection
void Dispatcher::
send_file_data() {
	
}

std::string Dispatcher::
d_recv() {
	return recv_str(sock, key);
}

void Dispatcher::
d_send(std::string & str) {
	send_str(sock, str, key);
}