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
	constructor_str += ", socket ";
	constructor_str += std::to_string(sock);

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
	start_thread_dispatch();
}

// request client for username and password and check credentials
// ***XOR key sent with request***
bool Servicer::
authenticate_user() {
	std::string authreq_key = "AUTHREQ KEY ";
	authreq_key += std::to_string(key);

	std::string msg = "\n";
	print(msg);
	msg = "authenticate_user()";
	print(msg);
	msg = "To client: ";
	msg += authreq_key;
	print(msg);

	send(sock, authreq_key.c_str(), authreq_key.length(), 0);

	std::string res = s_recv();

	print(res);
	msg = "\n\n";
	print(msg);

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
	std::string msg = "\n\ncheck_file_dir()";
	print(msg);

	std::string res = "VALID";
	s_send(res);

	res = s_recv();
	print(res);
	res = res.substr(res.find(" ") + 1, res.length() - res.find(" "));

	// replace tilde with user's home directory
	std::string home_dir = "";
	if (res.at(0) == '~') {
		struct passwd * pw = getpwuid(getuid());
		home_dir = std::string(pw->pw_dir);
		res = home_dir + res.substr(1, res.length() - 1);
	}

	std::cout << res << '\n';
	if (!std::filesystem::exists(res)) {
		std::string err = "NOFILE";
		s_send(err);
		return false;
	}

	if (std::filesystem::is_directory(res)) {
		// directory stuff here
		namespace fs = std::filesystem;

		std::string dir_info = "IS_DIR TRUE ";
		
		std::string dir_info_files = iterate_directory(res);
		
		int byte_num = dir_info_files.size();
		dir_info += std::to_string(byte_num);
		std::cout << "dir_info = " + dir_info;
		s_send(dir_info);
		std::cout << std::endl;
		
		struct passwd * pw = getpwuid(getuid());
		home_dir = std::string(pw->pw_dir);
		
		dir_info = dir_info_files;
		std::cout << "dir_info = " + dir_info;
		std::cout << std::endl;
		s_send(dir_info);
	} else {
		std::string file_info = "IS_DIR FALSE ";
		file_info += std::to_string(std::filesystem::file_size(res));
		
		msg = "To client: ";
		msg += file_info;
		print(msg);
			
		s_send(file_info);
	}

	return true;
}

std::string Servicer::
iterate_directory(const std::filesystem::path& dir) {
	std::string returnString;
	namespace fs = std::filesystem;

	std::vector<std::string> vec_dir_subFiles;
	std::vector<std::string> vec_dir_subFilesSize;

	for (auto& path: fs::recursive_directory_iterator(dir)){
		if (fs::is_directory(path)){
			returnString += path.path();
			returnString += "/ FI ";
			iterate_directory(path);
		}else{
			vec_dir_subFiles.push_back(path.path());
			std::uintmax_t subFileSize = fs::file_size(path);
			vec_dir_subFilesSize.push_back(std::to_string(subFileSize));
		}
	}
	for (unsigned int i = 0; i < vec_dir_subFiles.size(); i++){
		returnString += vec_dir_subFiles.at(i) + " ";
		returnString += vec_dir_subFilesSize.at(i) + " ";
	}
	if (!returnString.empty()){	//remove last space from dir_info_files
		returnString.pop_back();
	}

	return returnString;
}

// accept session header info from client and give OK to send threads for copy
void Servicer::
get_header() {
	std::string req = s_recv();

	std::string msg = "get_header()";
	print(msg);	
	print(req);

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
	std::string msg = "\n\nstart_thread_dispatch()";
	print(msg);
	
	int dispatch_sock, client_sock;
	socklen_t sin_size;
	struct sockaddr_storage client_addr;

	bind_socket(dispatch_sock, -1);

	if (listen(dispatch_sock, thread_num)) {
		perror("Servicer listen");
		return;
	};	

	msg = "Dispatch sock: ";
	msg += std::to_string(dispatch_sock);
	print(msg);

	std::string ok = "OK";
	s_send(ok);

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	std::string host_port;
	if (getsockname(dispatch_sock, (struct sockaddr *)&sin, &len) == -1) {
		perror("Port value, service()");
		return;
	} else
		host_port = std::to_string(ntohs(sin.sin_port));
	
	host_port += " ";
	host_port += get_host_info().second;

	print(host_port);

	s_send(host_port);

	int num_accepted = 0;
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

		std::thread copy_thread([] (int c_sock, uint16_t key) {
				std::unique_ptr<Dispatcher> dis(new Dispatcher(c_sock, key));
			}, client_sock, key);
		
		copy_thread.detach();

		if (++num_accepted == thread_num)
			break;
	}
	msg = "servicer done";
	print(msg);
}

std::string Servicer::
s_recv() {
	return recv_str(sock, key);
}

void Servicer::
s_send(std::string & str) {
	send_str(sock, str, key);
}
