/*
 * *****************************************************************************
 * file name   : include/net_utils.h
 * author      : Hung Q. Ngo (hungngo@cse.buffalo.edu)
 * description : for basic network programming needs
 * *****************************************************************************
 */

#ifndef _NETWORK_UTIL_H
#define _NETWORK_UTIL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

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

/* other constants */
#define MAXSOCKADDR  128      /* max socket address structure size */

/*
 * -----------------------------------------------------------------------------
 * prototypes for our utility functions
 * -----------------------------------------------------------------------------
 */
int Accept(int, struct sockaddr *, socklen_t *);
void Inet_ntop(int af, const void* src, char* dst, socklen_t size);
void Gethostname(char*, size_t);
void Getaddrinfo(const char*, const char *, const struct addrinfo *,
                  struct addrinfo **res);


int tcp_connect(const char*, const char*);
int create_tcp_passive_socket(const char*);
int create_bound_udp_socket(const char*);


int get_sock_info(int fd, char* ip, char* name, char* l_port, char* r_port);


#endif /* _NETWORK_UTIL_H */
