#ifndef _HTTPLIB_H
#define _HTTPLIB_H

char* get_http_header_line(char *header, char *match);
void read_http_request(char *request);
void read_http_response(char *response);
long get_content_length(char *buffer);
long get_http_response_header_size(char *response, int recv_size);

#endif
