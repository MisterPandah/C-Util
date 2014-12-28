#include "Socket.h"
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

int create_server_sock(int port) {

    int sockfd;

    struct sockaddr_in server_addr;

    // Exit the program if a socket can not be recreated
    if((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
	perror("ERROR when opening socket");
        exit(1);
    }

	//reuse port if already binded
	int yes = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

    // Bind an address to the socket
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if( (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1) {
	perror("Unable to bind the address to server socket");
        //exit(1);
    }

    // Set the server to start listening 
    listen(sockfd, 50);

    return sockfd;
}

int create_remote_sock(char *hostname, int port) {

	int hostsockfd, hostnewsockfd;
	struct hostent* host;

	host = gethostbyname(hostname);		

	struct sockaddr_in host_addr;
	bzero((char*)&host_addr,sizeof(host_addr));
	host_addr.sin_family = AF_INET;
	host_addr.sin_port = htons(port);
	bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length);

	hostsockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	// Setting timeout
	struct timeval tv;
	tv.tv_sec = 10; /* Set timeout for 10 seconds */
	tv.tv_usec = 0;

	setsockopt(hostsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));


	if((hostnewsockfd = connect(hostsockfd,(struct sockaddr*)&host_addr,sizeof(struct sockaddr))) == -1) {
		perror("Unable to connect to the remote server");
		//exit(1); // potential error?
	}

	return hostsockfd;
}
