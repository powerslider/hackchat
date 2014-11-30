#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <pthread.h>

#define CBUF_SIZE_DEFAULT 20;
typedef struct {
    int buf_size;
    int msg_count;
    char **msg_buf;
    pthread_mutex_t lock;
} chat_buffer;

chat_buffer *init_chat_buffer();
void destroy_chat_buffer(chat_buffer *cbuf);
void add_chat_buffer(chat_buffer *cbuf, const char *msg);
void parse_command(int socket, char *username, char *input );

#endif
