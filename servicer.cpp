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

	// continue here after file info processed
	get_header();
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
		/*namespace fs = std::filesystem;

		std::string dir_info = "IS_DIR TRUE ";
		std::string dir_info_files = "";
		std::vector<std::string> vec_dir_subFiles;
		std::vector<int> vec_dir_subFilesSize;

		for (auto& path: fs::recursive_directory_iterator(res)){
			vec_dir_subFiles.push_back(path.path());		//rename auto& path var
			vec_dir_subFilesSize.push_back(file_size(path));//make sure it shouldn't be path.path()
		}
		for (int i = 0; i < vec_dir_subFiles.size(); i++){
			dir_info_files += vec_dir_subFiles.at(i) + " ";
			dir_info_files += vec_dir_subFilesSize.at(i) + " ";
		}
		if (!dir_info_files.empty()){	//remove last space from dir_info_files
			dir_info_files.pop_back();
		}
		//add character count at the end of dir_info string
		int byte_num = dir_info_files.size();
		dir_info += std::to_string(byte_num) + " ";
		dir_info += dir_info_files;

		s_send(dir_info);*/
		
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

}

// begin thread dispatch loop to accept connections
void Servicer::
start_thread_dispatch() {

}

std::string Servicer::
s_recv() {
	return recv_str(sock, key);
}

void Servicer::
s_send(std::string & str) {
	send_str(sock, str, key);
}
