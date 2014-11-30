#ifndef __CHAT_H__
#define __CHAT_H__

#include <stdint.h>
#include <stdlib.h>

#define CHAT_PORT 8899
#define CHAT_PORT_S "8899"
#define OP_HELLO 0 //Greet the server, so it can tell other people who you are
#define OP_RMSG 1 //Regular Message
#define OP_PMSG 2 //Private Message
#define OP_ERROR 3 //Error Message

typedef struct {
    char *message;
} chat_msg;

typedef struct {
    char *recipient;
    char *message;
} chat_private_msg;

typedef struct {
    uint16_t errorcode;
    char *message;
} chat_error_msg;

typedef struct {
    uint16_t opcode;
    char *username;
    union {
        chat_msg regular_msg;
        chat_private_msg private_msg;
        chat_error_msg error_msg;
    } body;
} chat_packet;

char *prepare_chat_packet(const chat_packet *p, size_t *len);
chat_packet *unpack_chat_packet(const char *buffer, size_t len);

#endif
