#ifndef __UTIL_H__
#define __UTIL_H__

#include <sys/select.h>
#include <sys/types.h>

struct timeval set_timeval_seconds(time_t seconds, suseconds_t microseconds);
int select_read_fds(int nfds, fd_set *read_fds, struct timeval *tv);
void *err_exit(char *msg);

#endif
