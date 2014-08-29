/*
 * *****************************************************************************
 * file name   : net_utils.c
 * author      : Hung Q. Ngo (hungngo@cse.buffalo.edu)
 * description : implementations of basic networking functions
 *               modified from Stevens' code
 * *****************************************************************************
 */

/*
 * -----------------------------------------------------------------------------
 * headers related to networking functions
 * -----------------------------------------------------------------------------
 */
#include <sys/socket.h>  /* basic socket definitions               */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <netdb.h>       /* name and address conversion            */
#include <arpa/inet.h>   /* inet_pton/ntop                         */
#include <unistd.h>      /* read, write, close, exec, etc.         */
#include <errno.h>       /* error handlings                        */
#include <string.h>      /* strtok, mem*, and stuff                */
#include <stdio.h>       /* fgets, etc., perror,                   */
#include <signal.h>      /* signal handling                        */
#include <sys/wait.h>    /* wait, waitpid, etc.                    */

#include "include/common_headers.h"
#include "include/error_handlers.h"
#include "include/net_utils.h"

/*
 * ----------------------------------------------------------------------------
 * Getaddrinfo()
 *  wrapper function for system's getaddrinfo()
 * ----------------------------------------------------------------------------
 */
void Getaddrinfo (
        const char *hostname, 
        const char *servname,
        const struct addrinfo *hints, 
        struct addrinfo **res
        )
{
    int rv;
    rv = getaddrinfo(hostname, servname, hints, res);
    if (rv != 0) sys_error("getaddrinfo: %s\n", gai_strerror(rv));
}

/*
 * ----------------------------------------------------------------------------
 * Gethostname()
 *  wrapper function for system's gethostname()
 * ----------------------------------------------------------------------------
 */
void Gethostname(char* name, size_t len)
{
    if (gethostname(name, len) == -1) sys_error("gethostname()");
}


/*
 * ----------------------------------------------------------------------------
 * Inet_ntop()
 *  wrapper function for system's inet_ntop()
 * ----------------------------------------------------------------------------
 */
void Inet_ntop(int af, const void* src, char* dst, socklen_t size)
{
    if (inet_ntop(af, src, dst, size) == NULL) sys_error("inet_ntop()");
}

/*
 * ----------------------------------------------------------------------------
 * Accept()
 *  wrapper function for system's accept()
 * ----------------------------------------------------------------------------
 */
int Accept(int listen_fd, struct sockaddr *addr_ptr, socklen_t *len_ptr) {
    int fd;
    if ( (fd = accept(listen_fd, addr_ptr, len_ptr)) < 0 ) {
        if (errno == EINTR) {
            return -1;  /* restart accepting new connections */
        } else {     /* otherwise, some error has occured */
            sys_error("accept()");
        }
    }
    return fd;
}


/*
 * ----------------------------------------------------------------------------
 * get_sock_info()
 *  given a socket descriptor fd, return the peer's ip, host name, local port,
 *  and remote port, all in string format
 * return 0 on success, -1 on failure 
 * ----------------------------------------------------------------------------
 */
int get_sock_info(int sock_fd, char* ip, char* name, char* l_port, char* r_port)
{
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);

    // get the local port
    if (getsockname(sock_fd, (SA*) &sa, &sa_len) < 0) {
        sys_error("getsockname()");
    } else {
        snprintf(l_port, MAXLINE, "%u", ntohs(sa.sin_port));
    }

    // get peer's ip and port
    sa_len = sizeof(sa);
    if (getpeername(sock_fd, (SA *) &sa, &sa_len) < 0) {
        report_error("getpeername()");
        return -1;
    } else {
        if (inet_ntop(AF_INET, &sa.sin_addr, ip, MAXLINE) == NULL) {
            report_error("can't convert peer's IP from numeric to pres.");
            return -1;
        }

        if (getnameinfo((SA *) &sa, sa_len, name, MAXLINE, r_port, MAXLINE,
                    NI_NUMERICSERV) == -1) {
            report_error("can't convert peer's IP from numeric to pres.");
            return -1;
        }
    } /* and if (getpeername ...) */

    return 0;
}

/*
 * ----------------------------------------------------------------------------
 * create_tcp_passive_socket() : this works for both v4 and v6
 *  creates a listening socket at port
 * return: the socket descriptor on success
 *         -1 on error
 * ----------------------------------------------------------------------------
 */
int create_tcp_passive_socket(const char* port) {
    int listen_fd, n;
    const int on = 1;
    struct addrinfo hints, *res, *res_save;

    memset((void *) &hints, 0, sizeof(struct addrinfo));
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = PF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        report_error("getaddrinfo() error for port %s: %s",
                port, gai_strerror(n));
        return -1;
    }

    res_save = res;
    do {
        listen_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listen_fd < 0) continue; /* err, try next one */

        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, 
                    &on, sizeof(on)) < 0) {
            sys_error("setsockopt()");
        }

        if (bind(listen_fd, res->ai_addr, res->ai_addrlen) == 0) {
            break;                     /* success */
        }

        close(listen_fd);            /* bind error, close and try next one */
    } while ( (res = res->ai_next) != NULL);

    if (res == NULL) { /* errno from final socket() or bind() */
        report_error("tcp_listen(): cannot bind the socket to port %s", port);
        freeaddrinfo(res_save);
        return -1;
    }

    if (listen(listen_fd, LISTENQ) < 0) {
        freeaddrinfo(res_save);
        sys_error("listen()");
    }

    freeaddrinfo(res_save);

    return(listen_fd);
}

/*
 * ----------------------------------------------------------------------------
 * create_bound_udp_socket(): returns a bound udp socket, to port "port"
 *  works with both v4 and v6
 * return: the socket fd on success
 *         -1 on error
 * ----------------------------------------------------------------------------
 */
int create_bound_udp_socket(const char* port) {
    int udp_fd, n;
    struct addrinfo hints, *res, *res_save;

    memset((void *) &hints, 0, sizeof(struct addrinfo));
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = PF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        report_error("getaddrinfo() error for port %s: %s",
                port, gai_strerror(n));
        return -1;
    }

    res_save = res;
    do {
        udp_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (udp_fd < 0) continue; /* err, try next one */

        if (bind(udp_fd, res->ai_addr, res->ai_addrlen) == 0) {
            break;                     /* success */
        }

        close(udp_fd);            /* bind error, close and try next one */
    } while ( (res = res->ai_next) != NULL);

    if (res == NULL) { /* errno from final socket() or bind() */
        report_error("can't bind udp socket to %s", port);
        return -1;
    }

    freeaddrinfo(res_save);

    return(udp_fd);
}

/*
 * ----------------------------------------------------------------------------
 * tcp_connect()
 *  try to connect to "host" with "service" being either the port number or
 *   the actual service's name (like ftp, telnet, ...)
 *  "host" can either be a host name or a dotted decimal string
 *  "service" is either the service name or the port number in string
 * 
 * works with both v4 and v6
 *
 * Return: the socket descriptor on success,
 *         -1 on error
 * ----------------------------------------------------------------------------
 */
int tcp_connect(const char *host, const char *service)
{
    int sock_fd = -1, n;
    struct addrinfo hints, *res, *res_save;

    /*
     * fill out this structure to hint getaddrinfo what to search for
     * if SOCK_STREAM is not specified, for example, two or more
     * struct addrinfo could be returned since the service may be available
     * for both UDP and TCP.
     */
    memset((void*) &hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = PF_INET;     /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* but TCP only */

    if ((n = getaddrinfo(host, service, &hints, &res)) != 0) {
        report_error("getaddrinfo() error : %s", gai_strerror(n));
        return -1;
    }

    res_save = res;

    while (res != NULL) {
        sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock_fd < 0) {
            continue;       /* ignore this one, hope the next works */
        }


        if (connect(sock_fd, res->ai_addr, res->ai_addrlen) == 0) {
            break;          /* success */
        }

        report_error("connect()");

        close(sock_fd);   /* ignore this one */
        res = res->ai_next;
    }

    if (res == NULL) {  /* errno set from final connect() */
        report_error("tcp_connect error for %s, %s", host, service);
        return -1;
    }

    freeaddrinfo(res_save);

    return(sock_fd);
}

/*
 * ----------------------------------------------------------------------------
 *  print_c_n_times()
 *   print a given character 'n' times
 * ----------------------------------------------------------------------------
 */
void print_c_n_times(char c, int n) {
  int i;
  for (i=0; i<n; i++) { putchar(c); }
}
