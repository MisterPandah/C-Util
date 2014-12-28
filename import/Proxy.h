#ifndef _PROXY_H
#define _PROXY_H

// Helper Functions
void closeConnections(int newsockfd, int remotesockfd);

void handle_client_remote_request(int hostsockfd, int newsockfd, char *request, char *indexFile);

#endif
