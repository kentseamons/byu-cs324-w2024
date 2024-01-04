/*
 * echoserveri.c - An iterative echo server
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXLINE 8192

void echo(int connfd);

int main(int argc, char *argv[]) {

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

	while (1) {
		/* addrlen needs to be initialized before the call to
		 * recvfrom().  See the man page for recvfrom(). */
		addr_len = sizeof(struct sockaddr_storage);
		int connfd = accept(sfd, remote_addr, &addr_len);

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

		echo(connfd);
		close(connfd);
	}
	exit(0);
}
