#include "servicer.hpp"

uint16_t make_rand() {
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> dist(10000, 65535);
	uint16_t key = dist(eng);
	return key;
}

// launch service
Servicer::
Servicer(int & sock) : 
sock(sock), serv_num(servicer_num++), key(make_rand()) {
	std::string constructor_str = "";
	constructor_str += "Launching servicer ";
	constructor_str += std::to_string(serv_num);
	constructor_str += " with key ";
	constructor_str += std::to_string(key);

	print(constructor_str);

	this->service();
	close(sock);
}

// launch servicing process
void Servicer::
service() {
	if (authenticate_user().first) {
		//valid
	} else {
		// send second
		return;
	}
}

// request client for username and password and check credentials
// ***XOR key sent with request***
std::pair<bool, std::string> Servicer::
authenticate_user() {
	std::string authreq_key = "AUTHREQ KEY ";
	authreq_key += std::to_string(this->key);

	send(this->sock, authreq_key.c_str(), authreq_key.length(), 0);

	char recv_buff[MAXDATA];
	int byte_num;
	if ((byte_num = recv(this->sock, recv_buff, MAXDATA - 1, 0)) == -1) {
		perror("recv, authenticate_user");
		return std::make_pair<bool, std::string>(false, std::string("Server error"));
	}
	recv_buff[byte_num] = '\0';

	std::string res = std::string(recv_buff);
	this->crypt(res);

	std::istringstream res_ss(res);
	std::vector<std::string> res_toks((std::istream_iterator<std::string>(res_ss)),
			std::istream_iterator<std::string>());

	// authenticate here

	return std::make_pair<bool, std::string>(true, nullptr);
}

// check if file or directory exists
bool Servicer::
check_file_dir(const std::string & fp) {
	
}

// send file info to client to process 
void Servicer:: 
send_file_info(const std::string & fp) {

}

// accept session header info from client and give OK to send threads for copy
void Servicer::
get_header() {

}

// begin thread dispatch loop to accept connections
void Servicer::
start_thread_dispatch() {

}

void Servicer::
crypt(std::string & str) {
	for (auto & c : str)
		c = c ^ this->key;
}
