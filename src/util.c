#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>

struct timeval set_timeval_seconds(time_t seconds, suseconds_t microseconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = microseconds;

    return tv;
}

int select_read_fds(int fd, fd_set read_fds, struct timeval *tv) {
    int nfds = fd + 1;
    FD_ZERO(&read_fds);
    FD_SET(0, &read_fds);
    FD_SET(fd, &read_fds);

    return select(nfds, &read_fds, NULL, NULL, tv);
}

void err_exit(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}


