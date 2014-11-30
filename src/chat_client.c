#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
#include <readline/readline.h>
#include "easysocket.h"
#include "chat.h"
#include "chat_client.h"
#include "util.h"

int main(int argc, char *argv[]){
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <username> <host> (<port>)\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_sockfd;
    char *host = argv[2];
    char *username = argv[1];
    char * port = CHAT_PORT_S;
    chat_buffer *cbuf;
    if (argc == 4) {
        port = argv[3];
    }

    server_sockfd = create_inet_client_socket(host, port, SOCKET_IPv4, 0);

    chat_packet p;
    p.username = username;
    p.opcode = OP_HELLO;
    size_t len;
    char *data = prepare_chat_packet(&p, &len);
    write(server_sockfd, data, len);

    cbuf = init_chat_buffer();
    fd_set read_fds;

    while(1) {
        struct timeval tv = set_timeval_seconds(0, 0);
        int sr = select_read_fds(server_sockfd, &read_fds, &tv);
        if (sr == -1)
            err_exit("select");
        
        if (FD_ISSET(server_sockfd, &read_fds)) {
            char buff[500];
            chat_packet * recv_packet;
            size_t received = recv(server_sockfd, buff,500, 0);
	        if (received == 0) {
	            puts("Connection terminated by server.");
	            exit(EXIT_SUCCESS);
	        } else if(received == -1){
	            err_exit("");
	        }
	        
	        recv_packet = unpack_chat_packet(buff, received);
	        switch (recv_packet->opcode) {
	            case OP_RMSG:
	                printf("\x1b[36m%s: %s\n", 
	                        recv_packet->username, 
	                        recv_packet->body.regular_msg.message);
	                break;
	            case OP_PMSG:
	                printf("\x1b[32mPM %s: %s\n",
	                        recv_packet->username,
	                        recv_packet->body.private_msg.message);
	                break;
	            case OP_ERROR:
	                printf("Error: %s: %s",
	                        recv_packet->username,
	                        recv_packet->body.error_msg.message);
	                break;
	        }
	        puts("\x1b[0m");
        }
        
        if (FD_ISSET(0, &read_fds)){
            char buf[100];
            fgets(buf, 100, stdin);
            parse_command(server_sockfd, username, buf);
        }
    }
}

chat_buffer *init_chat_buffer() {
    chat_buffer *cbuf;
    cbuf->buf_size = CBUF_SIZE_DEFAULT;
    cbuf->msg_buf = malloc(sizeof(char *) * cbuf->buf_size);
    cbuf->msg_count = 0;
    pthread_mutex_init(&cbuf->lock, NULL);

    return cbuf;
}

void destroy_chat_buffer(chat_buffer *cbuf) {
    int i;
    pthread_mutex_lock(&cbuf->lock);
    for (i = 0; i < cbuf->msg_count; i++) {
        free(cbuf->msg_buf[i]);
    }
    free(cbuf->msg_buf);
    pthread_mutex_unlock(&cbuf->lock);
    pthread_mutex_destroy(&cbuf->lock);
}

void add_chat_buffer(chat_buffer *cbuf, const char *msg){
    pthread_mutex_lock(&cbuf->lock);
    pthread_mutex_unlock(&cbuf->lock);
}

void parse_command(int sock, char * username, char * input ){
	if (input[0] == '/' && input[1] == 'p') {
		strtok(input, " ");
		char *rcp = strtok(NULL, " ");
		char *msg = strtok(NULL, "");
		puts("Sending PM");
		puts(msg);
		chat_packet p;
		p.opcode = OP_PMSG;
		p.username = username;
		p.body.private_msg.recipient = rcp;
		p.body.private_msg.message = msg;
		size_t len;
		char * data = prepare_chat_packet(&p, &len);
		write(sock, data, len);
    } else {
		chat_packet p;
		p.opcode = OP_RMSG;
		p.username = username;
		p.body.regular_msg.message = input;
		size_t len;
		char *data = prepare_chat_packet(&p, &len);
		write(sock, data, len);
	}
}
