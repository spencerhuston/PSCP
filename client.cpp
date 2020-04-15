#include "client.hpp"

Client::
Client(std::string & file_name, int & thread_num) :
file_name(file_name), thread_num(thread_num) {
	assign_threads(authenticate());

	make_header();
	if (request_copy()) 
		spawn_threads();
	else {
		std::cout << "Bad copy request\n";
		exit(1);
	}
}

// gives error and exits if bad login or file does not exist
// else returns list of file info
std::vector<std::string> Client::
authenticate() {
	std::string username, password;

	std::cout << "Username: ";
	std::cin >> username;

	std::cout << "Password: ";
	std::cin >> password;

	std::string res = "USER ";
	res += username;
	res += " PASS ";
	res += password;

	send_str(sock, res);

	res = recv_str(sock);
	if (res != "VALID") {
		std::cout << "Bad login\n";
		exit(0);
	}

	res = "FILENAME ";
	res += file_name;

	std::cout << res << '\n';

	send_str(sock, res);
	res = recv_str(sock);

	if (res == "NOFILE") {
		std::cout << "File does not exist or you do not have access to it\n";
		exit(0);
	}

	std::istringstream res_ss(res);
	std::vector<std::string> file_info((std::istream_iterator<std::string>(res_ss)),
			std::istream_iterator<std::string>());

	return file_info;
}

void Client::
assign_threads(const std::vector<std::string> & file_info) {
	if (file_info.at(1) == "FALSE") { // regular file
		int file_size = atoi(file_info.at(2).c_str());
		if (file_size < thread_num)
			thread_num = 5; // play around with #'s to find optimal assignment
		
		int split = file_size / thread_num;
		for (int byte = 0; byte < thread_num * split; byte += split) {
			struct thread_info info;
			info.file_name = file_name;
			info.start_byte = byte;
			info.chunk_size = split;
			std::vector<thread_info> single_file_assignment;
			single_file_assignment.push_back(info);
			thread_assignments.push_back(single_file_assignment);
		}
	
		// if remainder exists, add to last thread (always a small #)	
		thread_assignments.back().front().chunk_size += file_size % thread_num;
	} else { // directory

	}	
}

bool Client::
request_copy() {
	std::string req = "REQ ";
	req += header;
	req += " ";
	req += std::to_string(thread_num);

	send_str(sock, req);
	req = recv_str(sock);

	if (req.substr(0, 2) == "OK") {
		serv_port = atoi(req.substr(3, req.length() - 3).c_str());	
		return true;
	}
	return false;
}

void Client::
make_header() {
	const std::string CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<> dist(0, CHARS.length());

	header = "";
	for (int i = 0; i < HEADER_SIZE; ++i)
		header += CHARS[dist(generator)];
}

void Client::
spawn_threads() {
	
}

void Client::
copy_file() {
	
}

void * 
get_in_addr(struct sockaddr * sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void 
bind_socket(std::string & host_name) {
	struct addrinfo hints, *serv_info, *p;
	int res;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((res = getaddrinfo(host_name.c_str(), std::to_string(PORT).c_str(), &hints, &serv_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		exit(1);
	}

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

	freeaddrinfo(serv_info);
	
	if (p == NULL || sock < 0) {
		std::cerr << "Failed to connect\n";
		exit(2);
	}
}

uint16_t
parse_key(const std::string & authreq) {
	std::istringstream req_ss(authreq);
	std::vector<std::string> req_toks((std::istream_iterator<std::string>(req_ss)),
			std::istream_iterator<std::string>());

	if (req_toks.at(0) != "AUTHREQ") {
		std::cout << "Bad request\n";
		exit(3);
	}

	return (uint16_t)atoi(req_toks.at(2).c_str());
}

std::string
recv(const int & sock) {
	char recv_buff[MAXDATA];
	int byte_num;
	if ((byte_num = recv(sock, recv_buff, MAXDATA - 1, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	recv_buff[byte_num] = '\0';

	return std::string(recv_buff);
}

std::string
recv_str(const int & sock) {
	std::string str = recv(sock);
	std::transform(str.begin(), str.end(), str.begin(),
			[](char c) { return c ^ key; });
	return str;
}

void
send_str(const int & sock, std::string & str) {
	std::transform(str.begin(), str.end(), str.begin(),
			[](char c) { return c ^ key; });
	send(sock, str.c_str(), str.length(), 0);
}

void 
print(const std::string & str) {
	mtx.lock();
	std::cout << str << '\n';
	mtx.unlock();
}

int 
main(int argc, char ** argv) {
	// check thread num is a number
	bool is_num = true;
	std::string num = std::string(argv[1]);
	for (auto i = num.begin(); i != num.end(); ++i)
		is_num &= isdigit(*i);

	if (argc < 3 || !is_num) {
		std::cout << "Usage: pscp thread_num host:file_name\n";
		exit(1);
	}	
	
	// parse out host name and file name
	std::string host_file = std::string(argv[2]);
	std::size_t colon = host_file.find(":");
	
	if (colon == std::string::npos) {
		std::cout << "Usage: pscp thread_num host:file_name\n";
		exit(1);
	}
	
	std::string host_name = host_file.substr(0, colon);
	std::string file_name = host_file.substr(colon + 1, host_file.length() - colon);

	bind_socket(host_name);

	std::cout << "Connection made\n";

	// receive authentication request and XOR key
	std::string recv_buff = recv(sock);	

	// construct client object to take over the rest of the process
	int thread_num = atoi(argv[1]);
	key = parse_key(std::string(recv_buff));
	
	std::unique_ptr<Client> client(new Client(
					file_name,
					thread_num));  

	close(sock);

	return 0;
}
