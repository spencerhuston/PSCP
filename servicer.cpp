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
	if (!authenticate_user() || !check_file_dir())
		return;

	// continue here after file info processed
}

// request client for username and password and check credentials
// ***XOR key sent with request***
bool Servicer::
authenticate_user() {
	std::string authreq_key = "AUTHREQ KEY ";
	authreq_key += std::to_string(this->key);

	send(this->sock, authreq_key.c_str(), authreq_key.length(), 0);

	std::string res = recv_str(this->sock);
	this->crypt(res);

	std::istringstream res_ss(res);
	std::vector<std::string> res_toks((std::istream_iterator<std::string>(res_ss)),
			std::istream_iterator<std::string>());

	// authenticate here
	print(res_toks.at(1));

	return true; 
}

// check if file or directory exists
bool Servicer::
check_file_dir() {
	std::string res = "VALID";
	crypt(res);
	send(this->sock, res.c_str(), res.length(), 0);

	res = recv_str(this->sock);
	this->crypt(res);

	mtx.lock();
	bool exists = (access(res.c_str(), F_OK) != -1);
	mtx.unlock();

	if (!exists) {
		std::string err = "NOFILE\n";
		crypt(err);
		send(this->sock, err.c_str(), err.length(), 0);
		return false;
	}

	struct stat path;
	stat(res.substr(res.find(" ") + 1, res.length() - res.find(" ")).c_str(), &path);
	bool is_not_dir = S_ISREG(path.st_mode);	

	// directory stuff here

	return true;
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
