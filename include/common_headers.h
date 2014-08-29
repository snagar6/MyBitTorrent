/*
 * *****************************************************************************
 * file name   : common_headers.h
 * description : 
 *  - basic contants and marcos used by most modules
 * *****************************************************************************
 */

#ifndef _COMMON_HEADERS_H
#define _COMMON_HEADERS_H

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>

#include        <unistd.h>      /* basic Unix system programming          */
#include        <sys/types.h>   /* basic system data types                */
#include        <sys/time.h>    /* timeval{} for select()                 */
#include        <sys/wait.h>    /* 'wait' and 'waitpid'                   */
#include        <sys/select.h>  /* IO multiplexing                        */
#include        <errno.h>       /* error handling                         */
#include        <signal.h>      /* signal handling                        */

#define	MAXLINE	     4096     /* max text line length : 4K         */
#define	BUFFSIZE     8192     /* buffer size for reads and writes  */
#define TRUE         1
#define FALSE        0
#define STR_SIZE     4096
#define BUF_SIZE     8192 
#define PIECES_SIZE 262160

#define DEFAULT_TCP_PORT "12345"
#define DEFAULT_UDP_PORT "12345"

#ifndef NULL
  #define NULL (void *) 0     /* just a NULL pointer               */
#endif

typedef	void  Sigfunc(int);   /* for signal handlers               */

#define	min(a,b)     ((a) < (b) ? (a) : (b))
#define	max(a,b)     ((a) > (b) ? (a) : (b))

int get_cmd_index(char*, char*);
void syntax_report(const char* msg);
void connect_handler(char*, char*);
void show_handler(char*, char*);
void quit_handler(char*, char*);
void cmd_not_found(char*, char*);
int is_valid_conn_id(char*);

// New Additions over the Chatty functions written by the Prof.

void peer_message_handshake(int);
void peer_message_keepalive(int);
void peer_message_choke(int);
void peer_message_unchoke(int);
void peer_message_interested(int);
void peer_message_not_interested(int);
void peer_message_have(int);
void peer_message_bitfield(int);
void peer_message_piece(int,int,int,int);
void peer_message_cancel(int);
void peer_message_request(int conn_id, int pReqd);
void peer_message_handshake1(int sock_fd);

unsigned int  binary_to_decimal(char *);
void parseBitfields(int  bitfield,char *output);
int piece_required();

#endif /* _COMMON_HEADERS_H */
