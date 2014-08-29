/*
 * *****************************************************************************
 * file name   : messages.h
 * author      : Hung Q. Ngo (hungngo@cse.buffalo.edu)
 * description : read/write TCP/UDP messages
 * *****************************************************************************
 */

#ifndef _MESSAGES_H
#define _MESSAGES_H

#include <netinet/in.h>     /* sockaddr_in{} and other Internet defns */
#include "common_headers.h"

/*
 * ----------------------------------------------------------------------------
 * types
 * ----------------------------------------------------------------------------
 */
typedef struct {
  uint32_t pid;           /* peer's process ID */
  char     text[MAXLINE]; /* the text message  */
} tcp_message_t;

/*
 * ----------------------------------------------------------------------------
 * functions
 * ----------------------------------------------------------------------------
 */
int read_tcp_msg(int, tcp_message_t*);
void print_tcp_msg(int cid, tcp_message_t *msg);
int sendMessage(int cid, char *text, int len);

#endif /* _MESSAGES_H */


