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

void * get_in_addr(struct sockaddr * sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
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

	char s[INET6_ADDRSTRLEN];
	struct addrinfo hints, *serv_info, *p;
	int res;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((res = getaddrinfo(argv[2], std::to_string(PORT).c_str(), &hints, &serv_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		exit(1);
	}

	int sock;
	for (p = serv_info; p != NULL; p = p->ai_next) {
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			perror("Socket init error");
			continue;
		}

		if (connect(sock, p->ai_addr, (socklen_t)p->ai_addrlen) < 0) {
			close(sock);
			perror("Socket connect error");
			sock = -1;
			continue;
		}

		break;
	}

	if (p == NULL || sock < 0) {
		std::cerr << "Failed to connect\n";
		exit(2);
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));

	freeaddrinfo(serv_info);
	std::cout << "Connection made\n";

	while (1) {}

	close(sock);

	return 0;
}
