#include "dispatcher.hpp"

std::string 
recv(const int & sock) {
	char recv_buff[MAXDATA];
	int byte_num;
	if ((byte_num = recv(sock, recv_buff, MAXDATA - 1, 0)) == -1) {
		perror("recv");
		return "";
	}
	recv_buff[MAXDATA] = '\0';

	return std::string(recv_buff);
}

std::string
recv_str(const int & sock, const uint16_t & key) {
	std::string res = recv(sock);
	crypt_pscp(res, key);
	return res;
}

void
send_str(const int & sock, std::string & str, const uint16_t & key) {
	crypt_pscp(str, key);
	send(sock, str.c_str(), str.length(), 0);
}

void 
crypt_pscp(std::string & str, const uint16_t & key) {
	for (auto & c : str)
		c = c ^ key;	
}

Dispatcher::
Dispatcher(const std::string & file_name, const std::string & header,
	   int & sock, int & port, int & chunk_size, uint16_t & key, char & start_byte) : 
file_name(file_name), header(header), sock(sock), port(port),
chunk_size(chunk_size), key(key), start_byte(start_byte) {
	std::string constructor_str = "Processing " + file_name + " for session " + header;
	print(constructor_str);

	this->send_file_data();
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
