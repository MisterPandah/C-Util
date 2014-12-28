#include "Proxy.h"

#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 2048
#define BUFFER_LENGTH 2040

void closeConnections(int newsockfd, int remotesockfd) {
	close(newsockfd);
	close(remotesockfd);
}

void handle_client_remote_request(int hostsockfd, int newsockfd, char *request, char *indexFile) {

	int n = send(hostsockfd,request,strlen(request),0);
	char buffer[BUFFER_LENGTH];

	if(n > 0) {

		long cntntlen_size = 0; // Store the Content-Length of the current HTTP Response
		long content_recv = 0; // Store the current total of received bytes from HTTP server		
	
		int firstRead_flag = 0; // flag to determine a response has been read

		FILE *f_ptr;

		do {
			// Set buffer to zero
			bzero((char *)buffer,BUFFER_LENGTH);
			
			// 
			if((n = recv(hostsockfd,buffer,BUFFER_LENGTH,0)) == -1) {
				perror("Unable to retrieve data from the remote server");
				fprintf(stdout, "\n");
				closeConnections(newsockfd, hostsockfd);				

				break;
			}

			// First read of the buffer which should contain the HTTP response header and some 
			if(firstRead_flag == 0) {
		
				// Expected Content-Length size obtain from the HTTP Response header
				cntntlen_size = get_content_length(buffer);

				// Return the true byte-size of the content from the first read
				// This excludes the byte-size of the HTTP response header
				content_recv = n - get_http_response_header_size(buffer, n);

				read_http_response(buffer);
				firstRead_flag = 1;

			} else {
				content_recv = content_recv + n;
			}

			if(n > 0) {
				send(newsockfd,buffer,n,0);

				// Cache the result for future use
				f_ptr = fopen(indexFile, "a");
			
				if(f_ptr != NULL) {
					fprintf(f_ptr, "%s", buffer);
					fclose(f_ptr);
				}
			}			

			// Close the connection to remote server and client once all data is sent
			if(content_recv == cntntlen_size) {
				closeConnections(newsockfd, hostsockfd);
				break;
			}

		} while(n > 0);

	} else {
		perror("Unable to send HTTP request to remote server");
		fprintf(stdout, "\n");
	}
}
