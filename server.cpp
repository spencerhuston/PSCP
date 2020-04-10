#include "server.hpp"

/*========== BEGIN SERVICER =======================*/
// launch service
Servicer::
Servicer(int & sock) : 
sock(sock), serv_num(servicer_num++), key(make_rand()) {
	std::string constructor_str = "";
	constructor_str += "Launching servicer " + serv_num;
	constructor_str += " with key " + key;

	print(constructor_str);

	this->service();
}

// launch servicing process
void Servicer::
service() {
	
}

// request client for username and password and check credentials
// ***XOR key sent with request***
bool Servicer::
authenticate_user() {

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
int 
make_rand() {
	mtx.lock();
	int key = rand();	
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

int 
main(int argc, char ** argv) {
	int server_sock, client_sock;
	struct sockaddr_in server_addr;
	struct sockaddr_storage server_storage;
	socklen_t addr_len;

	server_sock = socket(PF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));
	
	addr_len = sizeof(server_addr);
	bind(server_sock, (struct sockaddr *)&server_addr, addr_len);

	char hostbuff[256];
	char *ipbuff;
	struct hostent * host_ent;
	int hostname;

	hostname = gethostname(hostbuff, 256);
	host_ent = gethostbyname(hostbuff);
	ipbuff = inet_ntoa(*((struct in_addr *) host_ent->h_addr_list[0]));

	if (listen(server_sock, MAX_CLIENTS) == 0) {
		std::cout << "Server is listening\n";
		std::cout << "Host name: " << std::string(hostbuff) << '\n';
		std::cout << "Host IP: " << std::string(ipbuff) << '\n';
	}
	else
		std::cout << "Error listening, closing\n";

	srand(time(0));

	while (true) {
		addr_len = sizeof(server_storage);
		client_sock = accept(server_sock, 
				(struct sockaddr *)&server_storage, &addr_len);
		
		std::thread service([](int & sock){
				std::unique_ptr<Servicer> servicer(new Servicer(sock));
			}, std::ref(client_sock));
		service.detach();	
	};

	return 0;
}
