#include "client.hpp"

Client::
Client(std::string & file_name, int & thread_num, int & key) :
file_name(file_name), thread_num(thread_num), key(key) {

}

bool Client::
authenticate() {

}

void Client::
assign_threads() {

}

void Client::
request_copy() {

}

const std::string Client::
make_header() {

}

void Client::
spawn_threads() {

}

void Client::
copy_file() {
	
}

void Client::
encrypt(std::string & str) {

}

void Client::
decrypt(std::string & str) {

}

int 
main(int argc, char ** argv) {
	bool is_num = true;
	std::string num = std::string(argv[1]);
	for (auto i = num.begin(); i != num.end(); ++i)
		is_num &= isdigit(*i);

	if (argc < 3 || !is_num) {
		std::cout << "Usage: pscp thread_num host\n";
		exit(1);
	}	

	int sock;
	struct sockaddr_in server_addr;

	sock = socket(PF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(argv[2]);

	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

	if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
		std::cout << "Connection made\n";
	else {
		std::cout << "Connection failed\n";
		exit(1);
	}

	return 0;
}
