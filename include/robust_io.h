/*
 * *****************************************************************************
 * file name   : include/robuts_io.h
 * author      : Randal E. Bryant and David R. O'Hallaron 
 * description : Hung Ngo copied from Bryant & O'Hallaron's book sample codes
 * *****************************************************************************
 */

#ifndef _RIO_H
#define _RIO_H

#include "common_headers.h"

/*
 * -----------------------------------------------------------------------------
 * misc. macros and constants
 * -----------------------------------------------------------------------------
 */

/*
 * The following could be derived from SOMAXCONN in <sys/socket.h>, but many
 * kernels still #define it as 5, while actually supporting many more
 */
#define LISTENQ      1024     /* 2nd argument to listen()          */

/* to shorten all the type casts of pointer arguments */
#define SA           struct sockaddr

#define MAXSOCKADDR  128      /* max socket address structure size */

/* Persistent state for the robust I/O (Rio) package */
/* $begin rio_t */
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* descriptor for this internal buf */
    int rio_cnt;               /* unread bytes in internal buf */
    char *rio_bufptr;          /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;
/* $end rio_t */


/*
 * -----------------------------------------------------------------------------
 * Robust I/O package. 
 * -----------------------------------------------------------------------------
 */
/* these remain the same as before */
ssize_t readn(int fd, void *usrbuf, size_t n);
ssize_t writen(int fd, const void *usrbuf, size_t n);

/* robust buffered version, thread-safe */
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

Sigfunc* simpler_sigaction(int, Sigfunc*);
void     sig_child_handler(int);

#endif /* _RIO_H */

