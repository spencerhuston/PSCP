#include "server.hpp"

/*========== BEGIN SERVICER =======================*/
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
	decrypt(res);

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
encrypt(std::string & str) {
	for (auto & c : str)
		c = c ^ this->key;
}

void Servicer::
decrypt(std::string & str) {
	for (auto & c : str)
		c = c ^ this->key;
}
/*========== END SERVICER =========================*/



/*========== BEGIN DISPATCHER =====================*/
Dispatcher::
Dispatcher(const std::string & file_name, const std::string & header,
	   int & sock, int & chunk_size, int & key, char & start_byte) : 
file_name(file_name), header(header), sock(sock), 
chunk_size(chunk_size), key(key), start_byte(start_byte) {
	std::string constructor_str = "Processing " + file_name + " for session " + header;
	print(constructor_str);

	this->send_file_data();
}

// copy files over connection
void Dispatcher::
send_file_data() {

}

void Dispatcher::
encrypt(std::string & str) {

}

void Dispatcher::
decrypt(std::string & str) {

}
/*========== END DISPATCHER =======================*/



/*========== BEGIN MAIN ===========================*/
// Generate key for XOR
uint16_t
make_rand() {
	mtx.lock();
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> dist(10000, 65535);
	uint16_t key = dist(eng);
	mtx.unlock();
	return key;
}

// thread safe print out
void 
print(const std::string & str) {
	mtx.lock();
	std::cout << str << '\n';
	mtx.unlock();
}

// get host info to print out
std::pair<std::string, std::string>
get_host_info() {
	char host_buff[256];
	char *ip_buff;
	struct hostent * host_ent;
	int host_name;

	host_name = gethostname(host_buff, 256);
	host_ent = gethostbyname(host_buff);
	ip_buff = inet_ntoa(*((struct in_addr *)host_ent->h_addr_list[0]));
	
	return std::make_pair<std::string, std::string>(std::string(host_buff), 
							std::string(ip_buff));
}

// handle CTRL-C
void handler(int s) {
	std::cout << "Shutting server down\n";
	close(server_sock);
	exit(0);
}

void bind_socket() {
	struct addrinfo hints, *serv_info, *p;
	char s[INET6_ADDRSTRLEN];
	int res, yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	if ((res = getaddrinfo(NULL, std::to_string(PORT).c_str(), &hints, &serv_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		exit(1);
	}

	for (p = serv_info; p != NULL; p = p->ai_next) {
		if ((server_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Socket init");
			continue;
		}	

		if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			continue;
		}

		if (bind(server_sock, p->ai_addr, p->ai_addrlen) == -1) {
			close(server_sock);
			perror("socket bind");
			continue;
		}

		break;
	}

	freeaddrinfo(serv_info);

	if (p == NULL) {
		std::cerr << "Failed to bind socket\n";
		exit(1);
	}
}

int 
main(int argc, char ** argv) {
	int client_sock;
	socklen_t sin_size;
	struct sockaddr_storage client_addr;
	
	bind_socket();

	if (listen(server_sock, MAX_CLIENTS)) {
		perror("Listen");
		exit(1);
	}

	std::cout << "Listening for connections\n";
	auto host_info = get_host_info();
	std::cout << "Server name: " << host_info.first << '\n';
	std::cout << "Server IP: " << host_info.second << '\n';

	struct sigaction kill_handler;
	kill_handler.sa_handler = handler;
	sigemptyset(&kill_handler.sa_mask);
	kill_handler.sa_flags = 0;
	sigaction(SIGINT, &kill_handler, NULL);

	while (true) {
		sin_size = sizeof(client_addr);
		client_sock = accept(server_sock, 
				(struct sockaddr *)&client_addr, &sin_size);

		if (client_sock == -1) {
			perror("Client accept");
			continue;
		}	
	
		std::thread service([](int & sock){
				std::unique_ptr<Servicer> servicer(new Servicer(sock));
			}, std::ref(client_sock));
		service.detach();	
	};

	return 0;
}
