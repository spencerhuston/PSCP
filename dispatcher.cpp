#include "dispatcher.hpp"

std::string 
recv(const int & sock) {
	char recv_buff[MAXDATA];
	int byte_num;
	if ((byte_num = recv(sock, recv_buff, MAXDATA - 1, 0)) == -1) {
		perror("recv");
		return "";
	}
	recv_buff[byte_num] = '\0';

	return std::string(recv_buff);
}

std::string
recv_str(const int & sock, const uint16_t & key) {
	std::string res = recv(sock);
	std::transform(res.begin(), res.end(), res.begin(), 
			[&key](char c) { return c ^ key; });  
	return res;
}

void
send_str(const int & sock, std::string & str, const uint16_t & key) {
	std::transform(str.begin(), str.end(), str.begin(), 
			[&key](char c) { return c ^ key; });  
	send(sock, str.c_str(), str.length(), 0);
}

Dispatcher::
Dispatcher(int & sock, uint16_t & key) : sock(sock), key(key) {
	while (true) {
		std::string req = d_recv();
		std::istringstream req_ss(req);
		std::vector<std::string> req_toks((std::istream_iterator<std::string>(req_ss)),
							std::istream_iterator<std::string>());

		if (req_toks.empty() || req_toks.at(0) == "DONE") 
			break;

		std::string file_name = req_toks.at(0);
		if (file_name.at(0) == '~') {
			struct passwd * pw = getpwuid(getuid());
			file_name = std::string(pw->pw_dir) + file_name.substr(1, file_name.length() - 1);
		}
		int chunk_size = atoi(req_toks.at(1).c_str());
		int start_byte = atoi(req_toks.at(2).c_str());
	
		send_file_data(file_name, start_byte, chunk_size);
	}
	close(sock);
}

// copy files over connection
void Dispatcher::
send_file_data(const std::string & file_name, const int & start_byte, const int & chunk_size) {	
	unsigned char * copy_buffer;
	copy_buffer = (unsigned char *)malloc(chunk_size);
	memset(copy_buffer, 0, chunk_size);

	//std::ifstream file(file_name, std::ifstream::binary);
	FILE * file = NULL;
	file = fopen(file_name.c_str(), "rb");

	if (file) {
		/*
		file.seekg(start_byte);
		file.read((char *)&copy_buffer[0], chunk_size);
		file.close();*/

		fseek(file, start_byte, SEEK_SET);
		size_t res = fread(copy_buffer, 1, chunk_size, file);
		fclose(file);
	} else {
		std::string err = "Could not open file\n";
		print(err);
		return;
	}
	
	//printf("%s", copy_buffer);

	for (int i = 0; i < chunk_size; ++i)
		copy_buffer[i] = copy_buffer[i] ^ key;

	send(sock, copy_buffer, chunk_size, 0);
	free(copy_buffer);
}

std::string Dispatcher::
d_recv() {
	return recv_str(sock, key);
}

void Dispatcher::
d_send(std::string & str) {
	send_str(sock, str, key);
}
