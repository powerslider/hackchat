#ifndef _SERVER_H_
#define _SERVER_H_

typedef struct {
    int socket;
    char *username;
} chat_user;

typedef struct {
    int num_clients;
    int size;
    chat_user * clients;
} client_store;

void init_client_store(client_store *cstore);
void send_all(client_store *cs, const chat_packet *packet);
void clear_fds(client_store *cs, fd_set *fds);
int add_client(client_store * cs, int sock, const char *username);
chat_user * get_user_by_name(client_store *cs, const char *username);
void delete_client(client_store *cs, int sock);
void free_client_store(client_store *cs);

#endif
