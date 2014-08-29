/*
 * *****************************************************************************
 * file name   : messages.c
 * author      : Hung Q. Ngo (hungngo@cse.buffalo.edu)
 * description : read and write TCP/UDP messages
 * *****************************************************************************
 */

/*
 * -----------------------------------------------------------------------------
 * headers related to networking functions
 * -----------------------------------------------------------------------------
 */
#include <string.h>      /* strtok, mem*, and stuff                */
#include <netdb.h>       /* name and address conversion            */
#include <arpa/inet.h>   /* inet_pton/ntop                         */
#include <unistd.h>      /* read, write, close, exec, etc.         */
#include <string.h>      /* strtok, mem*, and stuff                */
#include <stdio.h>       /* fgets, etc., perror,                   */
#include <stdlib.h>      /* exit, etc.                             */

#include "include/robust_io.h"
#include "include/error_handlers.h"
#include "include/net_utils.h"
#include "include/messages.h"

#define ONE_MESSAGE_SIZE   4096

uint32_t my_pid;

/*
 * ----------------------------------------------------------------------------
 * read_tcp_msg():
 *   read tcp message from the given fd, copy it to the given struct
 *   return: 0 if connection is closed
 *           -1 on error
 *           #bytes read otherwise
 * ----------------------------------------------------------------------------
 */
int read_tcp_msg(int fd, tcp_message_t* msg)
{
    int bytes_read;
    uint32_t pl;
    uint32_t pid;

    // read the payload first
    bytes_read = readn(fd, &pl, 4);
    if (bytes_read < 0) sys_error("readn()");
    if (bytes_read == 0) return 0;

    pl = ntohl(pl);
    if ( (bytes_read !=4) || (pl < 4) ) return -1; /* badly formatted message */

    // now read the pid
    bytes_read = readn(fd, &pid, 4);
    if (bytes_read < 0) sys_error("readn()");
    if (bytes_read == 0) return 0;
    if (bytes_read != 4) return -1;
    msg->pid = ntohl(pid);

    // finally read the message
    bytes_read = readn(fd, msg->text, pl-4);
    if (bytes_read < 0) sys_error("readn()");
    if (bytes_read == 0) return 0;
    if (bytes_read != pl-4) return -1;

    // NULL-terminate the text
    msg->text[pl-4] = '\0';

    return pl+4;
}


/*
 * ----------------------------------------------------------------------------
 * print_tcp_msg(int cid, tcp_message_t* msg)
 *   print the message received from a tcp connection
 * ----------------------------------------------------------------------------
 */
void print_tcp_msg(int cid, tcp_message_t* msg)
{
    if (msg->text[strlen(msg->text)-1] == '\n') /* remove trailing '\n' */
        msg->text[strlen(msg->text)-1] = '\0';

    note(" Received mesg:     \"%s\"", msg->text);
    note(" Protocol:          TCP");
    note(" Connection number: %d", cid);
    note(" Sender's PID:      %d", msg->pid);
}




/*
 * ----------------------------------------------------------------------------
 * send a tcp message to the sock_fd
 * ----------------------------------------------------------------------------
 */
int sendMessage(int cid, char* text, int len)
{
    int sock_fd = cid;
    if (sock_fd < 0) {
        report_error("Connection %d is not active", cid);
        return 1;
    }

    /* uint32_t pl = strlen(text);
    // my_pid = getpid();	
    // uint32_t pid = htonl(my_pid);

    // pl = htonl(pl);

    // write the payload length 
    if (writen(sock_fd, &pl, 4) != 4) {
        report_error("Error writing to connection number %d", cid);
        return 1;
    }

    // write the PID
    if (writen(sock_fd, &pid, 4) != 4) {
        report_error("Error writing to connection number %d", cid);
        return 1;
    } */

    // and now write the actual message
    if (writen(sock_fd, text, len) !=  len) {
        report_error("Error writing to connection number %d", cid);
        return 1;
    }
    // printf("\n *** SENDING MESSAGE TO CONNECTIO - ID: %d *** \n",cid);
    // hexdump(text, 30);
    return 0;	
}

// EOF
