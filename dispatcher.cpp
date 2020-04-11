#include "dispatcher.hpp"

std::string recv_str(const int & sock) {
	char recv_buff[MAXDATA];
	int byte_num;
	if ((byte_num = recv(sock, recv_buff, MAXDATA - 1, 0)) == -1) {
		perror("recv");
		return "";
	}
	recv_buff[MAXDATA] = '\0';

	return std::string(recv_buff);
}

Dispatcher::
Dispatcher(const std::string & file_name, const std::string & header,
	   int & sock, int & port, int & chunk_size, int & key, char & start_byte) : 
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

void Dispatcher::
crypt(std::string & str) {
	for (auto & c : str)
		c = c ^ this->key;
}
