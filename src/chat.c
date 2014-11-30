#include "chat.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

char *prepare_chat_packet(const chat_packet *p, size_t *len){
    char *buf;

    switch(p->opcode){
        case OP_HELLO: 
            {
                size_t uname_len = strlen(p->username) + 1;
                *len = 2 + uname_len;
                buf = malloc(2 + uname_len);
                *(uint16_t*)buf = htons(p->opcode);
                strncpy(buf + 2, p->username, uname_len);
            }
            break;
        case OP_RMSG:
            {
                size_t uname_len = strlen(p->username) + 1;
                size_t msg_len = strlen(p->body.regular_msg.message) + 1;
                *len = 2 + uname_len + msg_len;
                buf = malloc(2 + uname_len + msg_len);
                *(uint16_t*)buf = htons(p->opcode);
                strncpy(buf + 2, p->username, uname_len);
                strncpy(buf + 2 + uname_len, p->body.regular_msg.message, msg_len);
            }
            break;
        case OP_PMSG:
            {
                size_t uname_len = strlen(p->username) + 1;
                size_t rcp_len = strlen(p->body.private_msg.recipient) + 1;
                size_t msg_len = strlen(p->body.private_msg.message) + 1;
                *len = 2 + uname_len + msg_len + rcp_len;
                buf = malloc(2 + uname_len + msg_len + rcp_len);
                *(uint16_t*)buf = htons(p->opcode);
                strncpy(buf + 2, p->username, uname_len);
                strncpy(buf + 2 + uname_len, p->body.private_msg.recipient, rcp_len);
                strncpy(buf + 2 + uname_len + rcp_len, p->body.private_msg.message, msg_len);
            }
            break;
        case OP_ERROR:
            {
                size_t uname_len = strlen(p->username) + 1;
                size_t emsg_len = strlen(p->body.error_msg.message) + 1;
                *len = 4 + uname_len + emsg_len;
                buf = malloc(4 + uname_len + emsg_len);
                *(uint16_t*)buf = htons(p->opcode);
                strncpy(buf + 2, p->username, uname_len);
                *(uint16_t*)(buf + 2 + uname_len) = htons(p->body.error_msg.errorcode);
                strncpy(buf + 4 + uname_len, p->body.error_msg.message, emsg_len);
            }
            break;
    }

    return buf;
}

chat_packet *unpack_chat_packet(const char *buf, size_t len){
    chat_packet *p = malloc(sizeof(chat_packet));
    p->opcode = ntohs(*(uint16_t *) buf);
    size_t uname_len = strlen(buf + 2) + 1;
    p->username = calloc(sizeof(char), uname_len);
    strcpy(p->username, buf + 2);

    switch(p->opcode){
        case OP_HELLO:
            {
            }
        case OP_RMSG:
            {
                size_t msg_len = strlen(buf + 2 + uname_len) + 1;
                p->body.regular_msg.message = calloc(msg_len, sizeof(char));
                strcpy(p->body.regular_msg.message, buf + 2 + uname_len);
            }
            break;
        case OP_PMSG:
            {
                size_t rcp_len = strlen(buf + 2 + uname_len) + 1;
                p->body.private_msg.recipient = calloc(rcp_len, sizeof(char));
                strcpy(p->body.private_msg.recipient, buf + 2 + uname_len);
                size_t msg_len = strlen(buf + 2 + uname_len + rcp_len) + 1;
                p->body.private_msg.message = calloc(msg_len, sizeof(char));
                strcpy(p->body.private_msg.message, buf + 2 + uname_len + rcp_len);
            }
            break;
        case OP_ERROR:
            {
                p->body.error_msg.errorcode = ntohs(*(uint16_t*)( buf + 2 + uname_len));
                size_t msg_len = strlen(buf + 4 + uname_len) + 1;
                p->body.error_msg.message = calloc(msg_len, 0);
                strcpy(p->body.error_msg.message, buf + 4 + uname_len);
            }
            break;
    }
    
    return p;
}
