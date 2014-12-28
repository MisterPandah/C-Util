#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#define BUFFER_SIZE 2048
#define BUFFER_LENGTH 2040

pthread_mutex_t createlock;
pthread_mutex_t updatelock;

struct table {
	char table[100][100];
	int size;
};

void filltable(FILE *list_file ,struct table *table) {

    char line[100];

    int i = 0;
    while(fscanf(list_file, "%s", line) != EOF) {
	
        if(i < 100) {
		strcpy(table->table[i],line);
		i++;
	}
	else {
		break;
	}
    }

    table->size = i;
}

/*
 * If the tr
 */
int contains_entry(struct table *table, char *str) {
	
	int i = 0;
	int match = 0;

	char tableEntry[100];
	char strMatch[100];

	while(i < table->size) {
		bzero((char*)tableEntry,100);
		bzero((char*)strMatch,100);

		strcpy(tableEntry, table->table[i]);
		strcpy(strMatch, str);

		touppercase(tableEntry);
		touppercase(strMatch);

		if(contains_string(strMatch, tableEntry)) {
		//if(contains_string(str, table->table[i])) {
			match = 1;
			break;
		}

		i++;
	}

	return match;
}


const char* const nomethod = 
	"HTTP/1.1 405 Method Now Allowed\r\n"
	"Connection: close\r\n\r\n";

const char* const badrequest = 
	"HTTP/1.1 400 Bad Request\r\n"
	"Connection: close\r\n\r\n";

const char* const blockrequest = 
	"HTTP/1.1 403 Request Blocked\r\n"
	"Connection: close\r\n\r\n";

struct thread_data {
	int sockfd;
	struct table dataTable;
};

void* test_thread(void* thread_args) {

	struct thread_data *args = (struct thread_data *)thread_args;
	fprintf(stdout, "In Thread: %d\n", args->sockfd);
}

unsigned long hashURL(char *url) {
	
	long hash = 5381;
	int c;

	while(c = *url++) {
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	}

	return hash;

	//return 1;
}

//void connection_handler(int socket_desc, struct table filtertable) {
void* connection_handler(void* thread_args) {
	int newsockfd, remotesockfd;
	struct sockaddr_in client_addr;

	struct thread_data *args = (struct thread_data *)thread_args;

	while(1) {
		// TODO: Going to have to include everything in this while loops for threading
		//if((newsockfd = accept(socket_desc, (struct sockaddr *)&client_addr, (socklen_t*) &client_addr)) == -1) {
		if((newsockfd = accept(args->sockfd, (struct sockaddr *)&client_addr, (socklen_t*) &client_addr)) == -1) {
			perror("Unable to accept connection from a client");
			fprintf(stdout, "\n");
			//exit(1);
		}

		char buffer[BUFFER_SIZE], method_header[64], url[2000], httpver[10], host_header[100], hostname[256];
		bzero((char*)buffer,BUFFER_LENGTH);

		if(recv(newsockfd,buffer,BUFFER_LENGTH,0) == -1) {
			perror("Unable to retrieve the HTTP request from client");
			fprintf(stdout, "\n");
			//exit(1);
		} else {
			
			sscanf(buffer, "%s %s %s\n%s %s", method_header, url, httpver, host_header, hostname);

			// TODO: BUG. Server will crash when there is an improper request sent. Figure out later on how to fix
			// Temporary hack is to check if the request retuns a NULL or not.

			char *hostline = get_http_header_line(buffer, "Host");

			// TODO: Implement cacheing based on URL
			
			// Create a directory called "cache" if it does not exist
			mkdir("cache", 0777);

			char hostDir[256];
			bzero((char *)hostDir, 256);

			strcpy(hostDir, "cache/");
			strcat(hostDir, hostname);			

			mkdir(hostDir, 0777);

			// Computing the hash URL
			unsigned long hashedURL = hashURL(url);

			// Convert the int into a string
			char hashedStringURL[15];
			bzero((char *)hashedStringURL, 15);

			sprintf(hashedStringURL, "%lu", hashedURL);

			// The hashname for the cache file
			char hashname[256];
			bzero((char *)hashname, 256);

			strcpy(hashname, hostDir);
			strcat(hashname, "/");
			strcat(hashname, hashedStringURL);

			char collisionFile[256];

			// TODO: Create the hash and implement hash collision strategy
			if(mkdir(hashname, 0777) == 0) {

				//This function will be moved to the "hashed" file
				FILE *newFile; // This file will be used to create a new file
				
				bzero((char *)collisionFile, 256);
				
				strcpy(collisionFile, hashname);
				strcat(collisionFile, "/collision");
			
				newFile = fopen(collisionFile, "w+");

				if(newFile) {
					//fprintf(stdout, "Closing file with value %d\n", newFile);
					fclose(newFile);
				}

			}

			// Attempt to create next index file

			unsigned long fileindex = 0;

			char indexFile[256];
			char indexNum[15];

			// Convert the index into a string
			bzero((char *)indexNum, 15);
			sprintf(indexNum, "%lu", fileindex);

			bzero((char *)indexFile, 256);
		
			strcpy(indexFile, hashname);
			strcat(indexFile, "/");
			strcat(indexFile, indexNum);

			pthread_mutex_lock(&createlock);	

			// Choose this index if it does not exist
			while(fopen(indexFile,"r")) {
				fileindex++;

				// Convert the index into a string
				bzero((char *)indexNum, 15);
				sprintf(indexNum, "%lu", fileindex);

				bzero((char *)indexFile, 256);
	
				strcpy(indexFile, hashname);
				strcat(indexFile, "/");
				strcat(indexFile, indexNum);
			}

			// TODO: create file before it is unlocked
			FILE *newFile;

			newFile = fopen(indexFile,"w+");

			if(newFile) {
				fclose(newFile);
			}

			pthread_mutex_unlock(&createlock);

			// Check if the request is bad. If the buffer cant be parsed, then the request is invalid
			if(hostline == NULL) {
				// Inform the client that the HTTP request is invalid and will not be sent
				send(newsockfd,badrequest,strlen(badrequest),0);

				// Close the socket connection to the client
				close(newsockfd);
			}
			//else if(contains_entry(&filtertable, hostname)) {
			else if(contains_entry(&(args->dataTable), hostname)) {

				// Inform the client that the request is blacklisted and will not be forwarded
				send(newsockfd,blockrequest,strlen(badrequest),0); 

				// Close the socket connection to the client
				close(newsockfd);
			}

			
			else if(strncmp(buffer, "GET", 3) != 0) {
				// Inform the client that the request method is not supported
				send(newsockfd,nomethod,strlen(nomethod),0);

				// Close the socket connection to the client
				close(newsockfd);
			}
			else {	
				read_http_request(buffer);			 // Read the HTTP Request from the client
				remotesockfd = create_remote_sock(hostname, 80); // Create a connection to the remote server

				if(remotesockfd != -1) {
					// Pass the request to the remote server and read the remote response back to the client
					handle_client_remote_request(remotesockfd, newsockfd, buffer, indexFile);

					// This will create a line to determine the mapping between the index file with the url
					char indexmap[2200];
					bzero((char *)indexmap, 2200);

					strcpy(indexmap, indexNum);
					strcat(indexmap, ",");
					strcat(indexmap, url);
					strcat(indexmap, "\n");

					FILE *f_ptr;

					// Update collision file
					pthread_mutex_lock(&updatelock);

					f_ptr = fopen(collisionFile, "a");
			
					if(f_ptr != NULL) {
						fprintf(f_ptr, "%s", indexmap);
						fclose(f_ptr);
					}

					pthread_mutex_unlock(&updatelock);
				}
			}
		}
	}

}

int main(int argc, char **argv) {

	// Global Variables
	FILE *list_file;
	int sockfd;	// Socket connection for host, client and remote server respectively

	struct table filtertable;
	struct thread_data t_data;

	// Exit the program with value of 1 (error) if it is not equal to 3
	if(argc != 3) {
		fprintf(stdout, "usage: %s port_no filter_textfile\n", argv[0]);
		exit(1);
	}

	// Exit the program with a value of 1 (error)
	if(!(list_file = fopen(argv[2],"r"))) {
		perror("Unable to open file");
		fprintf(stdout, "\n");
		exit(1);
	}

	// Fill the table with a list of string to act as a blacklist
	filltable(list_file, &filtertable);

	// Close the file
	fclose(list_file);

	if (pthread_mutex_init(&createlock, NULL) != 0)
	{
		fprintf(stdout, "Create mutex init failed\n");
		exit(1);
	}

	if (pthread_mutex_init(&updatelock, NULL) != 0)
	{
		fprintf(stdout, "Update mutex init failed\n");
		exit(1);
	}
	sockfd = create_server_sock(atoi(argv[1]));

	// TODO: TESTING THREADING
	
	t_data.sockfd = sockfd;
	t_data.dataTable = filtertable;
	
	//Create 4 worker threads
	pthread_t tid[4];
	int err;
	int i = 0;
	while (i < 4) {
		err = pthread_create(&tid[i], NULL, &connection_handler, (void*) &t_data);
		i++;
	}
	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	pthread_join(tid[2], NULL);
	pthread_join(tid[3], NULL);

	//connection_handler(sockfd, filtertable);

	fprintf(stdout, "This text should not appear");
	return 0; // Should never reach this point
}
