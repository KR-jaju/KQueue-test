
#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <unistd.h>

int main(void) {
	int server = ::socket(PF_INET, SOCK_STREAM, 0);
	::fcntl(server, F_SETFL, O_NONBLOCK);
	sockaddr_in info;
	info.sin_family = AF_INET;
	info.sin_port = htons(6667);
	info.sin_addr.s_addr = ::inet_addr("127.0.0.1");
	int a = 1;
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &a, sizeof(a));
	::bind(server, (sockaddr *)(&info), sizeof(info));
	::listen(server, 5);
	int kq = ::kqueue();
	std::vector<struct kevent>	changes;
	struct kevent serv;

	EV_SET(&serv, server, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	changes.push_back(serv);
	struct kevent	out[16];
	int client;
	while (1) {
		usleep(4000000);
		std::cout << "Ready" << std::endl;
		int n = ::kevent(kq, &changes[0], changes.size(), out, 16, NULL);
		changes.clear();
		std::cout << n << std::endl;
		for (int i = 0; i < n; i++) {
			switch (out[i].filter)
			{
			case EVFILT_READ:{
				if (out[i].ident == server) {
					sockaddr_in	info;
					socklen_t	size = sizeof(info);
					client = ::accept(server, (sockaddr *)(&info), &size);
					std::cout << "client in" << std::endl;
					EV_SET(&serv, client, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					changes.push_back(serv);
				} else if (out[i].flags & EV_EOF) {
					std::cout << "client out" << std::endl;
					close(client);
				} else {
					char buf[1024];
					int readb = ::read(client, buf, 1024);
					buf[readb] = '\0';
					std::cout << "recevied : " << buf << std::endl;
				}
			}
				break;
			
			default:
				break;
			}
		}
	}
}