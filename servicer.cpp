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

	service();
	close(sock);
}

// launch servicing process
void Servicer::
service() {
	if (!authenticate_user() || !check_file_dir())
		return;

	get_header();
	std::thread dispatch_thread(&Servicer::start_thread_dispatch, *this);
		
	dispatch_thread.join();
}

// request client for username and password and check credentials
// ***XOR key sent with request***
bool Servicer::
authenticate_user() {
	std::string authreq_key = "AUTHREQ KEY ";
	authreq_key += std::to_string(key);

	send(sock, authreq_key.c_str(), authreq_key.length(), 0);

	std::string res = s_recv();

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
	s_send(res);

	res = s_recv();
	res = res.substr(res.find(" ") + 1, res.length() - res.find(" "));

	// replace tilde with user's home directory
	if (res.at(0) == '~') {
		struct passwd * pw = getpwuid(getuid());
		res = std::string(pw->pw_dir) + res.substr(1, res.length() - 1);
	}

	std::cout << res << '\n';
	if (!std::filesystem::exists(res)) {
		std::string err = "NOFILE";
		s_send(err);
		return false;
	}

	if (std::filesystem::is_directory(res)) {
		// directory stuff here
	} else {
		std::string file_info = "IS_DIR FALSE ";
		file_info += std::to_string(std::filesystem::file_size(res));
		s_send(file_info);	
	}

	return true;
}

// accept session header info from client and give OK to send threads for copy
void Servicer::
get_header() {
	std::string req = s_recv();
	
	std::istringstream req_ss(req);
	std::vector<std::string> req_toks((std::istream_iterator<std::string>(req_ss)),
			std::istream_iterator<std::string>());

	if (req_toks.at(0) == "REQ") {
		dispatch_header = req_toks.at(1);
		thread_num = atoi(req_toks.at(2).c_str());
	}
}

// begin thread dispatch loop to accept connections
void Servicer::
start_thread_dispatch() {
	int dispatch_sock, client_sock;
	socklen_t sin_size;
	struct sockaddr_storage client_addr;

	bind_socket(dispatch_sock, -1);

	if (listen(dispatch_sock, thread_num)) {
		perror("Servicer listen");
		return;
	};	
	
	std::string ok = "OK";

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	int port = -1;
	if (getsockname(dispatch_sock, (struct sockaddr *)&sin, &len) == -1) {
		perror("Port value, service()");
		return;
	} else
		port = ntohs(sin.sin_port);

	ok += " ";
	ok += std::to_string(port);
	s_send(ok);

	while (true) {
		sin_size = sizeof(client_addr);
		client_sock = accept(dispatch_sock,
				(struct sockaddr *)&client_addr, &sin_size);

		if (client_sock == -1) {
			perror("Dispatch thread accept");
			continue;
		}

		std::string res = recv_str(client_sock, key);
		if (res != dispatch_header) {
			res = "Bad header, thread spawn";
			print(res);
			res = "NO";
			send_str(client_sock, res, key);
			close(client_sock);
			continue;
		}
		res = "OK";
		send_str(client_sock, res, key);
		res = recv_str(client_sock, key);

		std::istringstream res_ss(res);
		std::vector<std::string> res_toks((std::istream_iterator<std::string>(res_ss)),
						std::istream_iterator<std::string>());

		if (res_toks.at(0) == "DONE")
			break;

		std::string file_name = res_toks.at(0);
		int chunk_size = atoi(res_toks.at(1).c_str());
		int start_byte = atoi(res_toks.at(2).c_str());
		uint16_t d_key = key;

		auto dis = new Dispatcher(file_name, client_sock, chunk_size, d_key, start_byte);

		std::thread copy_thread([]
				(std::string file_name, int chunk_size, 
				 int c_sock, uint16_t key, int start_byte) {
					std::unique_ptr<Dispatcher> dis(new Dispatcher(file_name, 
										        chunk_size, 
										      	c_sock,
											key, 
											start_byte));
				}, file_name, chunk_size, client_sock, key, start_byte);
		
		copy_thread.detach();
	}		
}

std::string Servicer::
s_recv() {
	return recv_str(sock, key);
}

void Servicer::
s_send(std::string & str) {
	send_str(sock, str, key);
}
