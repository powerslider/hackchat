#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read()/write()
#include <stdint.h>
#include <netdb.h> // getaddrinfo()
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h> // e.g. struct sockaddr_in on OpenBSD
#include "easysocket.h"

#define SOCKET_BACKLOG  128 
#define SOCKET_SERVER 1
#define SOCKET_CLIENT 2

struct addrinfo set_address_info(struct addrinfo hints, char prot_osi3, char prot_osi4) {
    int domain, type;

    type = set_connection_type(prot_osi4);
    domain = set_ip_domain(prot_osi3);

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_socktype = type;
    hints.ai_family = domain;
    hints.ai_flags = AI_PASSIVE; // Use system IP address

    return hints;
}

int set_connection_type(int prot_osi4) {
     switch (prot_osi4) {
        case SOCKET_TCP:
            return SOCK_STREAM;
        case SOCKET_UDP:
            return SOCK_DGRAM;
        default:
            return -1;
    }
}

int set_ip_domain(int prot_osi3) {
   switch (prot_osi3) {
        case SOCKET_IPv4:
            return AF_INET;
        case SOCKET_IPv6:
            return AF_INET6;
        case SOCKET_IP_ALL:
            return AF_UNSPEC;
        default:
            return -1;
    }
}


int find_live_connection(struct addrinfo *service_info, struct addrinfo hints, int retval, int flags, int socket_type) {
    struct addrinfo *p;
    int socketfd;

    // go through the linked list of struct addrinfo elements
    for (p = service_info; p != NULL; p = p->ai_next) { 
        socketfd = socket(p->ai_family, p->ai_socktype | flags, p->ai_protocol);
        if (socketfd < 0) // Error at socket()!!!
            continue;

        if (socket_type == SOCKET_SERVER) {
            retval = bind(socketfd, p->ai_addr, (socklen_t)p->ai_addrlen);
            if (retval != 0) { // Error at bind()!!!
                close(socketfd);
                continue;
            }

            if (hints.ai_socktype == SOCKET_TCP)
               retval = listen(socketfd, SOCKET_BACKLOG);
        }

        if (socket_type == SOCKET_CLIENT) {
            retval = connect(socketfd, p->ai_addr, p->ai_addrlen);
        }

        if (retval == 0) // Safe to cancel the loop here, we have a connected socket.
            break;
        else
            close(socketfd);
    }

    p = service_info;

    return socketfd;
}


int create_inet_server_socket(const char *bind_addr, const char *bind_port, char prot_osi4, char prot_osi3, int flags) {
    int socketfd, retval;
    struct addrinfo hints, *service_info;

    if (bind_addr == NULL || bind_port == NULL)
        return -1;

    hints =  set_address_info(hints, prot_osi3, prot_osi4);
    if ((retval = getaddrinfo(bind_addr, bind_port, &hints, &service_info)) != 0) {
        fprintf(stderr, "Server socket -> getaddrinfo: %s\n%d", gai_strerror(retval), service_info->ai_flags);
        exit(EXIT_FAILURE);
    }

    socketfd = find_live_connection(service_info, hints, retval, flags, SOCKET_SERVER);
    if (service_info == NULL) {
        // Looped off the end of the list with no connection
        fprintf(stderr, "Failed to connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(service_info);

    return socketfd;
}

int create_inet_client_socket(const char* host, const char* service, char prot_osi3, int flags) {
    int socketfd, retval, conn_type;
    struct addrinfo hints, *service_info;

    if (host == NULL || service == NULL)
        return -1;

    hints = set_address_info(hints, prot_osi3, SOCK_STREAM);
    if ((retval = getaddrinfo(host, service, &hints, &service_info)) != 0) {
        fprintf(stderr, "Client socket -> getaddrinfo: %s\n", gai_strerror(retval));
        exit(EXIT_FAILURE);
    }

    socketfd = find_live_connection(service_info, hints, retval, flags, SOCKET_CLIENT);
    if (service_info == NULL) {
        // Looped off the end of the list with no connection
        fprintf(stderr, "Failed to connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(service_info);

    return socketfd;
}

int accept_inet_client_socket(int sockfd) {
    struct sockaddr_in client_info;
    socklen_t addrlen = sizeof(struct sockaddr_storage);

    int client_sockfd = accept(sockfd, (struct sockaddr *)&client_info, &addrlen);
    if (client_sockfd == -1) {
        fprintf(stderr, "Cannot accept\n");
        exit(EXIT_FAILURE);
    }

    return client_sockfd;
}

int close_inet_socket(int sockfd) {
    if (sockfd < 0)
        return -1;

    if (close(sockfd) == -1)
        return -1;

    return 0;
}

