#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/epoll.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>

#define MAXEVENTS 64
#define MAXLINE 2048


struct client_info {
	int fd;
	int total_length;
	char desc[1024];
};

int main(int argc, char **argv) 
{
	/* Check usage */
	if (!(argc == 2 || (argc == 3 &&
			(strcmp(argv[1], "-4") == 0 || strcmp(argv[1], "-6") == 0)))) {
		fprintf(stderr, "Usage: %s [ -4 | -6 ] port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int portindex;
	if (argc == 2) {
		portindex = 1;
	} else {
		portindex = 2;
	}

	/* Use IPv4 by default (or if -4 is specified);
	 * If -6 is specified, then use IPv6 instead. */
	int addr_fam;
	if (argc == 2 || strcmp(argv[1], "-4") == 0) {
		addr_fam = AF_INET;
	} else {
		addr_fam = AF_INET6;
	}

	unsigned short port = atoi(argv[portindex]);
	int sock_type = SOCK_STREAM;


	struct sockaddr_in ipv4addr;
	struct sockaddr_in6 ipv6addr;

	/* Variables associated with local address and port */
	struct sockaddr *local_addr;
	socklen_t addr_len;

	if (addr_fam == AF_INET) {
		/* We are using IPv4. */
		/* Populate ipv4addr with the appropriate family, address
		 * (listen on all addresses), and port */
		ipv4addr.sin_family = addr_fam;
		ipv4addr.sin_addr.s_addr = INADDR_ANY; // listen on any/all IPv4 addresses
		ipv4addr.sin_port = htons(port);       // specify port explicitly, in network byte order

		/* Point local_port to the structure associated with IPv4, and
		 * assign addr_len to the size of the ipv6addr structure.
		 * */
		local_addr = (struct sockaddr *)&ipv4addr;
		addr_len = sizeof(ipv4addr);
	} else { // addr_fam == AF_INET6
		/* We are using IPv6. */
		/* Populate ipv6addr with the appropriate family, address
		 * (listen on all addresses), and port */
		ipv6addr.sin6_family = addr_fam;
		ipv6addr.sin6_addr = in6addr_any;     // listen on any/all IPv6 addresses
		ipv6addr.sin6_port = htons(port);     // specify port explicitly, in network byte order

		/* Point local_port to the structure associated with IPv6, and
		 * assign addr_len to the size of the ipv6addr structure.
		 * */
		local_addr = (struct sockaddr *)&ipv6addr;
		addr_len = sizeof(ipv6addr);
	}

	int sfd;
	if ((sfd = socket(addr_fam, sock_type, 0)) < -1) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}
	if (bind(sfd, local_addr, addr_len) < 0) {
		perror("Could not bind");
		exit(EXIT_FAILURE);
	}
	if (listen(sfd, 100) < 0) {
		perror("Could not listen");
		exit(EXIT_FAILURE);
	}

	// set listening file descriptor nonblocking
	if (fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
		fprintf(stderr, "error setting socket option\n");
		exit(1);
	}

	/* Variables associated with remote address and port */
	struct sockaddr_in remote_addr_in;
	struct sockaddr_in6 remote_addr_in6;
	struct sockaddr *remote_addr;
	char remote_addr_str[INET6_ADDRSTRLEN];
	unsigned short remote_port;

	if (addr_fam == AF_INET) {
		remote_addr = (struct sockaddr *)&remote_addr_in;
	} else {
		remote_addr = (struct sockaddr *)&remote_addr_in6;
	}

	int efd;
	if ((efd = epoll_create1(0)) < 0) {
		perror("Error with epoll_create1");
		exit(EXIT_FAILURE);
	}

	// allocate memory for a new struct client_info, and populate it with
	// info for the listening socket
	struct client_info *listener =
		malloc(sizeof(struct client_info));
	listener->fd = sfd;
	sprintf(listener->desc, "Listen file descriptor (accepts new clients)");

	// register the listening file descriptor for incoming events using
	// edge-triggered monitoring
	struct epoll_event event;
	event.data.ptr = listener;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) < 0) {
		fprintf(stderr, "error adding event\n");
		exit(EXIT_FAILURE);
	}

	struct epoll_event events[MAXEVENTS];
	while (1) {
		// wait for event to happen (-1 == no timeout)
		int n = epoll_wait(efd, events, MAXEVENTS, -1);

		for (int i = 0; i < n; i++) {
			// grab the data structure from the event, and cast it
			// (appropriately) to a struct client_info *.
			struct client_info *active_client =
				(struct client_info *)(events[i].data.ptr);

			printf("New event for fd %d (%s)\n", active_client->fd, active_client->desc);

			if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(events[i].events & EPOLLRDHUP)) {
				/* An error has occured on this fd */
				fprintf(stderr, "epoll error on %s\n", active_client->desc);
				close(active_client->fd);
				free(active_client);
				continue;
			}

			if (sfd == active_client->fd) {
				// loop until all pending clients have been accepted
				while (1) {
					addr_len = sizeof(struct sockaddr_storage);
					int connfd = accept(active_client->fd, remote_addr, &addr_len);

					if (connfd < 0) {
						if (errno == EWOULDBLOCK ||
								errno == EAGAIN) {
							// no more clients ready to accept
							break;
						} else {
							perror("accept");
							exit(EXIT_FAILURE);
						}
					}

					if (addr_fam == AF_INET) {
						remote_addr_in = *(struct sockaddr_in *)remote_addr;
						/* Populate remote_addr_str (a string) with the
						 * presentation format of the IPv4 address.*/
						inet_ntop(addr_fam, &remote_addr_in.sin_addr,
								remote_addr_str, INET6_ADDRSTRLEN);
						/* Populate remote_port with the value of the port, in
						 * host byte order (as opposed to network byte order).
						 * */
						remote_port = ntohs(remote_addr_in.sin_port);
					} else {
						remote_addr_in6 = *(struct sockaddr_in6 *)remote_addr;
						/* Populate remote_addr_str (a string) with the
						 * presentation format of the IPv6 address.*/
						inet_ntop(addr_fam, &remote_addr_in6.sin6_addr,
								remote_addr_str, INET6_ADDRSTRLEN);
						/* Populate remote_port with the value of the port, in
						 * host byte order (as opposed to network byte order).
						 * */
						remote_port = ntohs(remote_addr_in6.sin6_port);
					}
					printf("Connection from %s:%d\n",
							remote_addr_str, remote_port);

					/* UNCOMMENT FOR NONBLOCKING
					// set client file descriptor nonblocking
					if (fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
						fprintf(stderr, "error setting socket option\n");
						exit(1);
					}
					*/

					// allocate memory for a new struct
					// client_info, and populate it with
					// info for the new client
					struct client_info *new_client =
						(struct client_info *)malloc(sizeof(struct client_info));
					new_client->fd = connfd;
					new_client->total_length = 0;
					sprintf(new_client->desc, "Client %s:%d (fd %d)",
							remote_addr_str, remote_port, connfd);

					// register the client file descriptor
					// for incoming events using
					// edge-triggered monitoring
					event.data.ptr = new_client;
					event.events = EPOLLIN;
					if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event) < 0) {
						fprintf(stderr, "error adding event\n");
						exit(1);
					}
				}
			} else {
				// read from socket until (1) the remote side
				// has closed the connection or (2) there is no
				// data left to be read.
				/* UNCOMMENT FOR NONBLOCKING
				while (1) {
				*/
					char buf[MAXLINE];
					int len = recv(active_client->fd, buf, MAXLINE, 0);
					if (len == 0) { // EOF received
						// closing the fd will automatically
						// unregister the fd from the efd
						close(active_client->fd);
						free(active_client);
						/* UNCOMMENT FOR NONBLOCKING
						break;
						*/
					} else if (len < 0) {
						/* UNCOMMENT FOR NONBLOCKING
						if (errno == EWOULDBLOCK ||
								errno == EAGAIN) {
							// no more data to be read
						} else {
						*/
							perror("client recv");
							close(active_client->fd);
							free(active_client);
						/* UNCOMMENT FOR NONBLOCKING
						}
						break;
						*/
					} else {
						active_client->total_length += len;
						printf("Received %d bytes (total: %d)\n", len, active_client->total_length);
						send(active_client->fd, buf, len, 0);
					}
				/* UNCOMMENT FOR NONBLOCKING
				}
				*/
			}
		}
	}
	free(listener);
}
