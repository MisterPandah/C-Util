#include "HTTPlib.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Read the buffer that contains the HTTP header and a string to match against. If there is a line in the buffer
 * that matches against the match argument, then return the first line that contains that match.
 */
char* get_http_header_line(char *header, char *match) {
	
	char copy[strlen(header)];
	strcpy(copy, header);

	char* token;

	token = strtok(copy, "\r\n");

	while(token != NULL)
	{
		if(strncmp(token, match, strlen(match)) == 0) {
			return strdup(token);
			break;
		}

		token = strtok(NULL, "\r\n");
	}

	return NULL;
}

void read_http_request(char *request) {

	//fprintf(stdout, "%s", request);

	char* getline = get_http_header_line(request, "GET");
	char* hostline = get_http_header_line(request, "Host");
	
	if((getline != NULL) && (hostline != NULL)) {
		fprintf(stdout,"%s\n%s\n\n", getline, hostline);

		free(getline);
		free(hostline);
	}
}

void read_http_response(char *response) {

	//fprintf(stdout, "%s", response);
	
	char* httpline = get_http_header_line(response, "HTTP");

	if(httpline != NULL) {
		fprintf(stdout, "%s\n\n", httpline);

		free(httpline);
	}
}

/*
 * Read the buffer that contains the HTTP response header and return the value of Content-Length field
 * If no Content-Length field is found in the response header, thenreturn the value for Content-Length is 0.
 */
long get_content_length(char *buffer) {
	long cntntlen_size = 0;
	char* cntntlen_line = get_http_header_line(buffer, "Content-Length");

	// Fix for content length to prevent segmentation fault. This is when it is null
	if(cntntlen_line != NULL) {
		char copy[100];
		bzero((char *)copy, 100);

		strcpy(copy, cntntlen_line);
		sscanf(copy, "Content-Length: %lu", &cntntlen_size);

		free(cntntlen_line);
	}

	return cntntlen_size;
}

/*
 * Read the buffer that contains the HTTP Response header and the number of bytes the buffer has read.
 * If the buffer contains the HTTP response header and some of the content, seperate the 
 */
long get_http_response_header_size(char *response, int recv_size) {
	
	long headerSize = 0;
	char *httpcontent = strstr(response, "\r\n\r\n");

	if(httpcontent != NULL) {

		// This returns the size of the content including \r\n\r\n
		// If the content is binary and not text, this will result in a very different answer
		long contentLength = strlen(httpcontent) - 4;

		// Calculate the header size from the "suppose" size of the HTTP response content
		// Since both uses strlen() to determine the header and content, the error will be the same and
		// accurately obtain the header size
		headerSize = strlen(response) - contentLength;
	}

	// A NULL might represent an error, as the header may not contain the \r\n\r\n
		
	return headerSize;			
}

