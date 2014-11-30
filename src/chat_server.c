#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include "easysocket.h"
#include "chat.h"
#include "chat_server.h"
#include "util.h"

int main(int argc, char **argv) {
    int server_sock, i;
    fd_set readfds;
    client_store cs;

    server_sock = create_inet_server_socket("127.0.0.1", "5000", SOCKET_TCP, SOCKET_IPv4, 0);
    init_client_store(&cs);
    int nfds = server_sock;
    while(1){
        FD_ZERO(&readfds);
        clear_fds(&cs, &readfds);
        FD_SET(server_sock, &readfds);

        struct timeval tv = set_timeval_seconds(5, 0);

        //Examine status of read file descriptor for open input/output channels
        select(nfds + 1, &readfds, NULL, NULL, &tv);

        if (FD_ISSET(server_sock, &readfds)){
            puts("Accepted");
            int connfd = accept_inet_client_socket(server_sock);
            if (connfd == -1)
                err_exit("accept");

            nfds = nfds < connfd ? connfd : nfds;
            char buf[500];
            size_t rxd = recv(connfd, buf, 500, 0);
            if (rxd == -1){
                perror("lrx");
                continue;
            }

            chat_packet *rc = unpack_chat_packet(buf, rxd);
            printf("%s Connected\n", rc->username);
            add_client(&cs, connfd, rc->username);

            chat_packet p;
            p.opcode = OP_RMSG;
            p.username = "SERVER";
            p.body.regular_msg.message = malloc(strlen(rc->username) + 25);
            sprintf(p.body.regular_msg.message, "%s has connected", rc->username);
            send_all(&cs, &p);
            free(rc);
        } else {
            for(i = 0; i < cs.num_clients; i++) {
                if (FD_ISSET(cs.clients[i].socket, &readfds)) {
                    puts("Rx");
                    char buf[500];
                    size_t rxd = recv(cs.clients[i].socket, buf, 500, 0);
                    if (rxd == 0 || rxd == -1){
                        puts("Deleting client...");
                        delete_client(&cs, rxd);
                        continue;
                    }

                    chat_packet *p = unpack_chat_packet(buf, rxd);
                    if (p->opcode == OP_RMSG){
                        send_all(&cs, p);
                    } else if (p->opcode == OP_PMSG){
                        chat_user *rcp = get_user_by_name(&cs, p->body.private_msg.recipient);
                        if (rcp != NULL) {
                            int wr = write(rcp->socket, buf, rxd);
                            if (wr >= 0) {
                                delete_client(&cs, rcp->socket);
                            }
                        }
                    }
                }
            }
        }
    }
}

void send_all(client_store *cs, const chat_packet *packet) {
    chat_user **dq = malloc(sizeof(chat_user *) * cs->num_clients);
    int i;
    int dq_n = 0;
    size_t len;
    char *data = prepare_chat_packet(packet, &len);

    for (i = 0; i < cs->num_clients; i++) {
        int res = write(cs->clients[i].socket, data, len);
        if (res == -1) {
            dq[dq_n++] = &cs->clients[i];
        }
    }

    if(dq_n > 0) {
        char **dq_names = malloc(sizeof(char *) * dq_n);
        for (i = 0; i < dq_n; i++) {
            dq_names[i] = strdup(dq[i]->username);
            delete_client(cs, dq[i]->socket);
        }

        for (i = 0; i < dq_n; i++) {
            chat_packet p;
            p.opcode = OP_RMSG;
            p.username = "SERVER";
            p.body.regular_msg.message = malloc(sizeof(char) * (strlen(dq_names[i]) + 25));
            sprintf(p.body.regular_msg.message, "%s has disconnected.", dq_names[i]);
            send_all(cs, &p);
            free(p.body.regular_msg.message);
            free(dq_names[i]);
        }
        free(dq_names);
    }
    free(dq);
}

void clear_fds(client_store *cs, fd_set *fds) {
    int i;
    for(i = 0; i < cs->num_clients; i++){
        int sock = cs->clients[i].socket;
        FD_SET(sock, fds);
    }
}

void init_client_store(client_store * cstore){
    cstore->size = 10;
    cstore->num_clients = 0;
    cstore->clients = calloc(10, sizeof(chat_user));
}

int add_client(client_store *cs, int sock, const char *username) {
    if (get_user_by_name(cs, username) != NULL)
        return -1;

    if (cs->size == cs->num_clients) {
        cs->size = (cs->size * 3 / 2) + 5;
        cs->clients = realloc(cs->clients, cs->size * sizeof(chat_user));
    }

    int id = cs->num_clients++;
    cs->clients[id].socket = sock;
    cs->clients[id].username = strdup(username);
    
    return id;
}

chat_user *get_user_by_name(client_store *cs, const char *username) {
    int i;
    for (i = 0; i < cs->num_clients; i++) {
        if (strcmp(username, cs->clients[i].username) == 0)
            return &cs->clients[i];
    }

    return NULL;
}

void delete_client(client_store *cs, int sock) {
    int id;
    for(id = 0; id < cs->num_clients; id++){
        if (sock == cs->clients[id].socket)
            break;
    }

    if (id == cs->num_clients) {
        cs->num_clients--;
    } else if(cs->num_clients > 1){
        cs->num_clients--;
        memmove(cs->clients + id, cs->clients + id + 1, (cs-> num_clients - id) * sizeof(chat_user));
    }
}

void free_client_store(client_store *cs) {
    int i;
    for (i = 0; i < cs->num_clients; i++) {
        free(cs->clients[i].username);
    }
    free(cs->clients);
    free(cs);
}
