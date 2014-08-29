/*
 * UBtorrent: Project 2 (MNC Fall 2010)
 *
 * Modified by: Shreyas Nagaraj
 *              Pramod Nayak
 *
 * Leveraged From  Hung Q. Ngo (Original Author) Project 1 Implmentation
 * *****************************************************************************
 *
 * file name   : connections.h
 * author      : Hung Q. Ngo (hungngo@cse.buffalo.edu)
 * description : manage TCP connections
 * *****************************************************************************
 */

#ifndef _CONNECTIONS_H
#define _CONNECTIONS_H

#include <sys/select.h> 
#include "common_headers.h"

#define MAX_NO_CONNS 10
#define MAXLINE 4096

/*
 * ----------------------------------------------------------------------------
 * data
 * ----------------------------------------------------------------------------
 */
typedef enum {
    CONNECTED,
    DISCONNECTED
} conn_status_t;


struct timeval time1;

typedef struct {
    conn_status_t cs;           /* connection status */
    int  sock_fd;               /* the socket  */
    char ip[MAXLINE];           /* IP address  */
    char name[MAXLINE];         /* host name   */
    char l_port[MAXLINE];       /* local port  */
    char r_port[MAXLINE];       /* remove port */
    int  am_choking_flag;       /* am_choking - flag */
    int  am_interested_flag;    /* am_interested - flag */
    int  peer_choking_flag;     /* peer choking - flag */
    int  peer_interested_flag;  /* peer_interested - flag */
    char bitfield[256];          /* Current Bitfiled */
    int  hs_flag_1;             /* Handshake Flag 1 - completed */
    int  hs_flag_2;             /* Handshake Flag 2 - sent */
    double download_speed;
    int downloaded;
    int uploaded;
    double start_time;
    double upload_speed;
} connection_t;

connection_t conn_array[MAX_NO_CONNS];

void init_tcp_conns();  /* initialize connection array            */
void print_tcp_conns(); /* prints active connections beautifully  */
int  number_of_conns(); /* returns number of active connections   */
int  add_conn(int fd);  /* add new connection whose sock fd is fd */

/* choking and interested SETs and GETs */

void peer_choking_flag_set(int conn_id, int flag);
void peer_interested_flag_set(int conn_id, int flag);

void am_choking_flag_set(int conn_id, int flag);
void am_interested_flag_set(int conn_id, int flag);

/* Bitfield Setting and getting */

void bitfield_setting(int conn_fd, unsigned char* bitfield, int size);
char* bitfield_recv(int conn_id);
int is_bitfield_set(char *temp_bitfield, int piece_index);

/* Connection operations prototypes */

int  remove_conn(int conn_id); /* remove connection conn_id                */
void fd_set_conns(fd_set*);    /* mark fd of all connections for select()  */
int  get_conn_fd(int conn_id); /* return socket fd of conn_id, -1 on error */
int  get_max_fd();          /* return max connnection sock_fd, -1 on error */

/* Handshake Flags SETs and GETs */

void set_peer_attr (int type, int conn_fd, int flag);
int get_peer_attr (int type, int conn_fd);

#endif /* _CONNECTIONS_H */

