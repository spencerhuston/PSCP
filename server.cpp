#include "server.hpp"
#include "servicer.hpp"

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

	gethostname(host_buff, 256);

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

void bind_socket(int & sock, int port) {
	struct addrinfo hints, *serv_info, *p;
	int res, yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_TCP;

	const char * port_str = (port == -1) ? NULL : std::to_string(port).c_str();
	const char * host_str = (port == -1) ? get_host_info().first.c_str() : NULL;
	
	if ((res = getaddrinfo(host_str, port_str, &hints, &serv_info)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
		exit(1);
	}	

	for (p = serv_info; p != NULL; p = p->ai_next) {
		if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Socket init");
			continue;
		}	

		if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			continue;
		}

		if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
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
	
	bind_socket(server_sock, PORT);

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
