#ifndef EASYSOCKET_H
#define EASY_SOCKET_H

#define SOCKET_TCP 1
#define SOCKET_UDP 2
#define SOCKET_IPv4 3
#define SOCKET_IPv6 4
#define SOCKET_IP_ALL 5

int create_inet_server_socket(const char *bind_addr, const char *bind_port, char prot_osi4, char prot_osi3, int flags);
int accept_inet_client_socket(int sockfd);
int create_inet_client_socket(const char* host, const char* service, char prot_osi3, int flags);
int close_inet_socket(int sockfd);

#endif
