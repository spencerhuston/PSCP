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
		file_size = atoi(file_info.at(2).c_str());
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
		file_size = atoi(file_info.at(2).c_str());	//byte_num for receiving string
		std::cout << "file_info = ";
		for (int i = 0; i < file_info.size(); i++) {
			std::cout << file_info.at(i) << ' ';
		}
		std::cout << "\n";

		char recv_buff[file_size + 1];		// will contain all files and file sizes
		int byte_num;
		if ((byte_num = recv(sock, recv_buff, file_size, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		recv_buff[byte_num] = '\0';
		std::string recv_buff_string = recv_buff;	//so that we can use std::transform

		std::transform(recv_buff_string.begin(), recv_buff_string.end(), recv_buff_string.begin(),
			[](char c) { return c ^ key; });

		std::cout << "recv_buff = " << recv_buff_string;
		std::cout << "\n";

		//segment recv buffer into a vector with all tokens
		std::stringstream res_ss(recv_buff_string);
		std::vector<std::string> file_tokens((std::istream_iterator<std::string>(res_ss)),
				 std::istream_iterator<std::string>());
		
		std::cout << "file_tokens = ";
		for (int i = 0; i < file_tokens.size(); i++) {
			std::cout << file_tokens.at(i) << ' ';
		}
		std::cout << "\n";
		
		for (int i = 0; i < file_tokens.size(); i += 2){
			//file size if i + 1;
			struct thread_info info;
			info.file_name = file_info.at(i);
			info.start_byte = 0;
			info.chunk_size = stoi(file_info.at(i + 1));	//chunk size is full file size
			std::vector<thread_info> full_file_assignment;
			full_file_assignment.push_back(info);
			thread_assignments.push_back(full_file_assignment);
		}
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
	std::uniform_int_distribution<> dist(0, CHARS.length() - 1);

	header = "";
	for (int i = 0; i < HEADER_SIZE; ++i)
		header += CHARS[dist(generator)];
}

void Client::
spawn_threads() {
	std::string file;
	if (!is_dir)
		file.reserve(file_size);

	std::vector<std::thread> copy_threads;
	for (auto & v : thread_assignments) {
		std::thread copy_thread(&Client::copy_file, *this, v, std::ref(file));
		copy_threads.push_back(std::move(copy_thread));
	}	

	for (auto & t : copy_threads)
		t.join();

	if (!is_dir) {
		// write string to file
	}
}

void Client::
copy_file(std::vector<struct thread_info> assignment, std::string & file) {
	int client_sock;
	bind_socket(host_name, client_sock, serv_port);

	for (auto & v : assignment) {
		send_str(client_sock, header);	
		std::string res = recv_str(client_sock);	
		
		if (res != "OK") {
			print("Bad header, copy file");
			return;
		}

		std::string req = v.file_name;
		req += " ";
		req += std::to_string(v.chunk_size);
		req += " ";
		req += std::to_string(v.start_byte);

		send_str(client_sock, req);
	}
	
	std::string done = "DONE";
	send_str(client_sock, done);	
}

void * 
get_in_addr(struct sockaddr * sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in *)sa)->sin_addr);
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void 
bind_socket(std::string & host_name, int & client_sock, int port) {
	struct addrinfo hints, *serv_info, *p;
	int res;
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if ((res = getaddrinfo(host_name.c_str(), std::to_string(port).c_str(), &hints, &serv_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		exit(1);
	}

	for (p = serv_info; p != NULL; p = p->ai_next) {
		if ((client_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
			perror("Socket init error");
			continue;
		}

		if (connect(client_sock, p->ai_addr, (socklen_t)p->ai_addrlen) < 0) {
			close(client_sock);
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
	
	host_name = host_file.substr(0, colon);
	std::string file_name = host_file.substr(colon + 1, host_file.length() - colon);

	bind_socket(host_name, sock, PORT);

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
