#ifndef _SOCKET_H
#define _SOCKET_H

int create_server_sock(int port);
int create_remote_sock(char *hostname, int port);

#endif
