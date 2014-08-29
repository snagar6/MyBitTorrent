/*
 * UBtorrent: Project 2 (MNC Fall 2010)
 *
 * Modified by: Shreyas Nagaraj
 *              Pramod Nayak
 *
 * Leveraged From  Hung Q. Ngo (Original Author) Project 1 Implmentation
 * *****************************************************************************
 * file name   : commands.c
 * author      : Hung Q. Ngo (hungngo@cse.buffalo.edu)
 * description : process commands typed in by users
 * *****************************************************************************
 */

#include <string.h>      /* strtok, mem*, and stuff                */
#include <stdio.h>       /* fgets, etc., perror,                   */
#include <stdlib.h>      /* exit                                   */
#include <unistd.h>      /* read, write, close, exec, etc.         */

#include <sys/socket.h>  /* basic socket definitions               */
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <netdb.h>       /* name and address conversion            */

#include "include/common_headers.h"
#include "include/error_handlers.h"
#include "include/net_utils.h"
#include "include/connections.h"
#include "include/color.h"
#include "include/ubtorrent.h"
#include "include/messages.h"
#include "include/robust_io.h"

/*
 * ----------------------------------------------------------------------------
 * some misc. constants, types
 * ----------------------------------------------------------------------------
 */
#define EMPTY_COMMAND    -1
#define ONE_MESSAGE_SIZE   4096 
#define ONE_PIECE_SIZE 262160


typedef struct {
    char* name;                     /* command name, like "info", "show", ... */
    void  (*handler)(char*, char*); /* function to handle this command        */
    char* doc;                      /* command documentation: syntax, ...     */
} command_t;

extern uint32_t my_pid;
extern char my_host_name[MAXLINE];
extern char my_ip[MAXLINE];

// For messages
uint8_t  payload_identifier;
uint32_t  payload_length;
unsigned char  *payload_bitfield;
char 	  *peer_message;
uint32_t  piece_index;

extern char udp_port[MAXLINE];
extern char tcp_port[MAXLINE];

/*
 * ----------------------------------------------------------------------------
 * function prototypesn
 * ----------------------------------------------------------------------------
 */
/* static int get_cmd_index(char*, char*);
static void syntax_report(const char* msg);
static void connect_handler(char*, char*);
static void show_handler(char*, char*);
static void quit_handler(char*, char*);
static void cmd_not_found(char*, char*);
static int is_valid_conn_id(char*);
*/

/*
 * ----------------------------------------------------------------------------
 * data
 * ----------------------------------------------------------------------------
 */

// command array
static command_t cmd_list[] = {
    {"show", show_handler, "\n\
        show\n\
            show the state information of all existing TCP connections\n"},
    {"connect", connect_handler, "\n\
        connect <ip-address> <tcp-port>\n\
            try to establish a tcp conn. to <ip-address> at port <tcp-port>.\n\
            For example: connect 192.168.0.3 99999\n"},
    {"quit", quit_handler, "\n\
        quit | bye | exit\n"},
    {"bye", quit_handler, "\n\
        quit | bye | exit\n"},
    {"exit", quit_handler, "\n\
        quit | bye | exit\n"},
    {0, cmd_not_found, "\n\
        invalid command. Use one of the following commands:\n\
            info, connect, show, send, sendto, disconnect, quit\n"}
};

/*
 * ----------------------------------------------------------------------------
 * execute_cmd()n
 *  parse the line, report error or call appropriate handler to handle the cmd
 * ----------------------------------------------------------------------------
 */
void execute_cmd(char* line) {
    int cmd_idx;             /* the index to the command array */
    char arguments[MAXLINE]; /* store the arg part of the cmd  */

    cmd_idx = get_cmd_index(line, arguments);

    if (cmd_idx != EMPTY_COMMAND) {
        (cmd_list[cmd_idx].handler)(arguments, cmd_list[cmd_idx].doc);
    }

    return;
}

/*
 * ----------------------------------------------------------------------------
 * cmd_not_found()
 * ----------------------------------------------------------------------------
 */
void cmd_not_found(char* arg, char* doc) {
    warning("Command not found");
}


// NEW MESSAGE FUNTIONS for UBTORRENT ONLY !

int generate_bitfield()
{

	//unsigned char *gen_bitfield;
	int piece_count=0;
	int i,j,k,p,sum;
	int bf_num_pieces=0;
	int bf_num_bytes=0;
	// int cnt=0;
	int cnt1=0 ;int temp_size;
	long int num_of_pieces;
	int temp_mask = 0; 

	sum = 0;
	temp_size = 0;
	num_of_pieces = meta_data.file_length; 
	piece_count=(int)num_of_pieces/ONE_PIECE_SIZE;

	if(num_of_pieces % ONE_PIECE_SIZE){
       		piece_count+=1;
	}	

	bf_num_pieces =  piece_count % 8;
	bf_num_bytes =   piece_count / 8;

	if(bf_num_pieces !=0){
        	bf_num_bytes+=1; 
	}

	peerData.size_bitfield = bf_num_bytes;

	peerData.bitfield = (unsigned char *)malloc(peerData.size_bitfield*1);

        memset(peerData.bitfield,'\0', sizeof(peerData.size_bitfield));

	for(i=0;i<bf_num_bytes;i++)
	{
        	if(i < (bf_num_bytes-1))
        	{
                       		peerData.bitfield[i]= (unsigned char)0xff;
        	}
        	else if (i == bf_num_bytes-1 && (bf_num_pieces != 0))
        	{
				peerData.bitfield[i]= (unsigned char)0xff;
				peerData.bitfield[i] = peerData.bitfield[i]<<(8-bf_num_pieces);	
        	}	
		else if ((i == bf_num_bytes-1 && (bf_num_pieces == 0)))
		{	
			peerData.bitfield[i]= (unsigned char)0xff;
		}
	}
	if (seed_leach_flag == 0)
        {
	    for(i=0;i<bf_num_bytes;i++)	
		peerData.bitfield[i] = (unsigned char)0x0;
        }

	temp_size = bf_num_bytes;
	
	// Setting the Local bitfield snapshot for having the Status bitfield set for the current ubtorrent client  
	cnt1 = 0;
	for (p=0; p<256; p++) 
		 meta_data.bitfield_snapshot[p] = '\0';	

	if (bf_num_pieces == 0)
		temp_mask = 0;
	else
		temp_mask = (8 - bf_num_pieces); 


	meta_data.size_bitfield_bytes = temp_size;
	meta_data.size_bitfield = (temp_size*8) - temp_mask;
	meta_data.length_bitfield = (temp_size*8);


//	printf("\n\n DEBUh - meta_data.size_bitfield_bytes:%d | meta_data.size_bitfield: %d | meta_data.length_bitfield:%d \n",meta_data.size_bitfield_bytes,meta_data.size_bitfield,meta_data.length_bitfield);
 
//	printf("\nDEBUG --  meta_data.size_bitfield: %d and meta_data.length_bitfield: %d \n",meta_data.size_bitfield,meta_data.length_bitfield);
/*
	for (j=0; j<temp_size; j++)
        {
                for(k=1; k<=8; k++)
                {
                        if(peerData.bitfield[j] & (0x01 << (8-k)))
                        {
                                meta_data.bitfield_snapshot[cnt++] = '1';
                        }
                        else
                        {
                                meta_data.bitfield_snapshot[cnt++] = '0';
                        }
                }
        }
*/
     if (seed_leach_flag == 1) {
	for (j=0; j<meta_data.size_bitfield; j++)
		 meta_data.bitfield_snapshot[j] = '1';

	for (k = meta_data.size_bitfield; k < meta_data.length_bitfield;k++)
		meta_data.bitfield_snapshot[k] = '0';
     }
     else {
	for (j=0; j<meta_data.length_bitfield; j++)
                 meta_data.bitfield_snapshot[j] = '0';
     }
//	printf("meta_data.bitfield_snapshot: !%s!\n",meta_data.bitfield_snapshot);
	return temp_size;	
}



void parseBitfields(int  bitfield,char *output)
{

	int rem,i=0,j=0;
	char buffer[60];

        while(bitfield>0)
        {
                rem=bitfield % 2;
                bitfield=bitfield / 2;
                buffer[i++]=rem+'0';
        }

        while(i>=0){
                output[j++]=buffer[--i];
        }

        output[j-1]=0;

}



int piece_required() {

	static int bit = 0; 

	while(bit!=meta_data.size_bitfield){
	if (meta_data.bitfield_snapshot[bit] =='0') {

		return bit;
	}
	else if (meta_data.bitfield_snapshot[bit] =='1')
	{

		bit++;

		continue;	
	}

	}

	return -1;


}


void peer_message_handshake(int sock_fd) {

	if (peerData.handshakeMsg == NULL) {
		printf("peer_message_handshake(): Failed - (peerData.handshakeMsg) issue! \n");
	}

        if (send(sock_fd, peerData.handshakeMsg, 68,0) != 68) {
		printf("peer_message_handshake(): Failed ! \n");
	}
	else {
                printf("\nPeer_message_handshake(): SUCCESS \n");
        }
}	


void peer_message_keepalive(int conn_id) {

	int ret; 

	peer_message = (char *)malloc(4096*sizeof(char));
	memset(peer_message,'\0',ONE_MESSAGE_SIZE);
	
	payload_length = 0;
	payload_length = htonl(payload_length);
	memcpy(peer_message,&payload_length,4);
 	ret = sendMessage(conn_id, peer_message, 0);
	if(ret == 1) {
		printf("peer_message_keepalive(): Failed ! \n");
	} 	
	free(peer_message);
}


void peer_message_choke(int conn_id) {

	int ret;

	peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',ONE_MESSAGE_SIZE);
	
	payload_identifier = 0;
        payload_length = 1;
        payload_length = htonl(payload_length);
        memcpy(peer_message,&payload_length,4);
	memcpy(peer_message+4,&payload_identifier,1);
        ret = send(conn_id, peer_message, 5,0);
	printf("CHOKE MESSAGE SENT to CONN: %d\n",conn_id);
        if(ret < 5) {
                printf("peer_message_choke(): Failed ! \n");
        }
	free(peer_message);	
}


void peer_message_unchoke(int conn_id) {

	int ret;

        peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',ONE_MESSAGE_SIZE);

	payload_identifier = 1;
        payload_length = 1;
        payload_length = htonl(payload_length);
        memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4,&payload_identifier,1);
	printf("UNCHOKE MESSAGE SENT to CONN: %d\n",conn_id);	
        ret = send(conn_id, peer_message, 5, 0);
        if(ret < 5) {
                printf("peer_message_unchoke(): Failed ! \n");
        }
	free(peer_message);
}


void peer_message_interested(int conn_id ) { 

	int ret=0;

	peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',ONE_MESSAGE_SIZE);

        payload_identifier = 2;
        payload_length = 1;
        payload_length = htonl(payload_length);
        memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4,&payload_identifier,1);
	
	printf("\nSENDING: INTERESTED MESSAGE \n");
        ret = send(conn_id,peer_message,5,0);
	if(ret == 1) {
                printf("peer_message_interested(): Failed ! \n");
        }
	free(peer_message);
}


void peer_message_not_interested(int conn_id) {

	int ret = 0;

	peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',ONE_MESSAGE_SIZE);

        payload_identifier = 3;
        payload_length = 1;
        payload_length = htonl(payload_length);
        memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4,&payload_identifier,1);

        printf(" \n*** SENDING: NOT INTERESTED MESSAGE \n");
        ret = send(conn_id,peer_message,5,0);
	if(ret == 1) {
                printf("peer_message_not_interested(): Failed ! \n");
        }
	free(peer_message);
}


void peer_message_have(int conn_id) {

	int ret;

	peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',ONE_MESSAGE_SIZE);

        payload_identifier = 4;
        payload_length = 5;
        payload_length = htonl(payload_length);
	piece_index = 1;

        memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4,&payload_identifier,1);
	memcpy(peer_message+5,&piece_index,4);
        ret = sendMessage(conn_id, peer_message, 0);
        if(ret == 1) {
                printf("peer_message_have (): Failed ! \n");
        }
	free(peer_message);
}


void peer_message_handshake1(int conn_id) {

        int ret;
        int temp_size;
	
	
	if (peerData.handshakeMsg == NULL) 
	{
                printf("peer_message_handshake(): Failed - (peerData.handshakeMsg) issue! \n");
        }

        peer_message = (char *)malloc(256*sizeof(char));
        memset(peer_message,'\0',256);
        memset(&payload_length,0,4);

        temp_size = generate_bitfield();
       
        payload_identifier = 5;
        payload_length = 1 + temp_size; 
        payload_length = htonl(payload_length);


	memcpy(peer_message, peerData.handshakeMsg, 68);
        memcpy(peer_message+68,&payload_length,4);
        memcpy(peer_message+68+4,&payload_identifier,1);
        memcpy(peer_message+68+5,peerData.bitfield, temp_size);
        ret = send(conn_id,peer_message,68+4+1+temp_size,0);
	printf("\n\n SHREYAS - :%d\n",ret);       
 
        if(ret == -1) {
                printf("peer_message_bitfield and HandShake : Failed ! \n");
        }
        free(peer_message);
}


void peer_message_bitfield(int conn_id) {

        int ret;
	int temp_size;

        peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',ONE_MESSAGE_SIZE);
	memset(&payload_length,0,4);

	temp_size = generate_bitfield();
//	printf("TEMP SIZE # 2 : %d  \n",temp_size);

        payload_identifier = 5;
        payload_length = 1 + temp_size; // payload_bitfield is a unsigned char *  type

	// printf("payload_length: %d \n", payload_length);
        payload_length = htonl(payload_length);

        memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4,&payload_identifier,1);
        memcpy(peer_message+5,peerData.bitfield, temp_size);
	ret = writen(conn_id,peer_message,4+1+temp_size);
	//ret = sendMessage(conn_id, peer_message, 4+1+temp_size);
	printf("SENDING: BITFIELD MESSAGE to CONN_ID: %d\n",conn_id);
        if(ret == -1) {
                printf("peer_message_bitfield(): Failed ! \n");
        }
        free(peer_message);
}


void peer_message_request(int conn_id, int pReqd) {

	int ret;
	int num_blks = 0;
	int  remainder = 0;
	unsigned int begin;
	unsigned int index;
	unsigned int length;

	if(pReqd == -1) {
		printf("\nInvalid Piece\n");
		return;	
	}

	peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',4096);

        payload_identifier = 6;
	index = htonl(pReqd);
        payload_length = htonl(13);
	length = htonl(meta_data.pieceLength);

	num_blks = meta_data.file_length/meta_data.pieceLength;
	remainder = (meta_data.file_length % meta_data.pieceLength);

	if (remainder>0)
		num_blks = num_blks + 1;

	if (pReqd == (num_blks-1))
	{
		length = htonl(remainder);		
	}

	printf("\n ---- Sending Block Request: [%d] ----\n",pReqd); 
	begin = htonl(0);

//	printf("\nDEBUG: FILE SIZE :%ld  Blocksize: %d begin:%d  and length:%d\n",meta_data.file_length,meta_data.pieceLength,ntohl(begin),ntohl(length));

	memset(peer_message,'\0',4096);
	memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4,&payload_identifier,1);
	memcpy(peer_message+5,&index,4);
        memcpy(peer_message+9,&begin,4);
        memcpy(peer_message+13,&length,4);

	printf("\nPIECE MESSAGE DETAILS: INDEX: %d  BEGIN:%d  SIZE:%d \n",pReqd,ntohl(begin),ntohl(length));
	printf("SENDING: REQUEST MESSAGE to CONN_ID: %d\n",conn_id);
        ret = send(conn_id,peer_message,17,0);
        if(ret == 1) {
               	printf(" peer_message_request(): Failed ! \n");
        }
	free(peer_message);
}



void peer_message_piece(int conn_id,  int piece_index, int piece_begin, int length_req) {

	int ret;
	FILE *temp_fd;

	peer_message = (char *)malloc(length_req*sizeof(char)+20);
        memset(peer_message,'\0',(length_req*sizeof(char)+20));

//	printf("\n DEBUG: peer_message_piece - piece_index: %d | piece_begin :%d | length_req: %d \n",piece_index,piece_begin, length_req);

        payload_identifier = 7;
        payload_length = 9 + length_req;
        payload_length = htonl(payload_length);

	piece_index = htonl(piece_index);
	piece_begin = htonl(piece_begin);

        memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4, &payload_identifier, 1);
	memcpy(peer_message+5, &piece_index, 4);
	memcpy(peer_message+9, &piece_begin, 4);

	temp_fd = fopen(meta_data.file_name , "rb");
        fseek(temp_fd,((ntohl(piece_index) * meta_data.pieceLength)+ ntohl(piece_begin)) ,SEEK_SET);
        fread(peer_message+13 ,1 , length_req, temp_fd);
	fclose(temp_fd);

	printf("\nPIECE MESSAGE DETAILS: INDEX: %d  BEGIN:%d  SIZE:%d \n",ntohl(piece_index),ntohl(piece_begin),length_req);
	printf("SENDING: PIECE MESSAGE to CONN_ID: %d\n",conn_id);
        ret = writen(conn_id, peer_message, 13 + length_req);

        if(ret == 1) {
                printf("peer_message_interested(): Failed ! \n");
        }
	free(peer_message);
}


// Currently CANCEL message option is not used

void peer_message_cancel(int conn_id) {

	int ret;
        unsigned int begin;
        unsigned int index;
        unsigned int length;
        unsigned int cancel_piece = 1; 

	peer_message = (char *)malloc(4096*sizeof(char));
        memset(peer_message,'\0',ONE_MESSAGE_SIZE);

        payload_identifier = 8;
        index = htonl(cancel_piece);
        payload_length = htonl(13);
        begin = htonl(0);
        length = htonl(meta_data.pieceLength);

        memcpy(peer_message,&payload_length,4);
        memcpy(peer_message+4,&payload_identifier,1);
        memcpy(peer_message+5,&index,4);
        memcpy(peer_message+9,&begin,4);
        memcpy(peer_message+13,&length,4);

        ret = sendMessage(conn_id, peer_message, 0);
        if(ret == 1) {
                printf(" peer_message_request(): Failed ! \n");
        }
	free(peer_message);
}


/*
 * ----------------------------------------------------------------------------
 * connect_handler()
 *  handles 'connect' command
 * ----------------------------------------------------------------------------
 */
void connect_handler(char* arg, char* doc) {
    char* token;
    char ip[MAXLINE] = ""; char port[MAXLINE] = "";

    // first argument
    token = strtok(arg, " \t\n");
    if (token == NULL) { syntax_report(doc); return; }
    strcpy(ip, token); ip[MAXLINE-1] = 0;

    // second argument
    token = strtok(NULL, " \t\n");
    if (token == NULL) { syntax_report(doc); return; }
    strcpy(port, token); port[MAXLINE-1] = 0;

    // anything left?
    token = strtok(NULL, " \t\n");
    if ( token != NULL ) { syntax_report(doc); return; }

    int sock_fd;
    if (number_of_conns() < MAX_NO_CONNS) {
        sock_fd = tcp_connect(ip, port);
        if (sock_fd != -1) add_conn(sock_fd);
    } else {
        report_error("Maximum number of connections reached");
    }
}

/*
 * ----------------------------------------------------------------------------
 * show_handler()
 *  handles 'show' command
 * ----------------------------------------------------------------------------
 */
void show_handler(char* arg, char* doc) {
    char* token;
    // parse the argument
    token = strtok(arg, " \t\n");
    if (token != NULL) {
        syntax_report(doc);
        return;
    } else {
        textcolor(BRIGHT, CYAN, BLACK);
        print_tcp_conns(); /* print info about the existing tcp connections */
        textnormal();
    }
}

/*
 * ----------------------------------------------------------------------------
 * quit_handler()
 *  handles 'quit', 'bye', etc.
 * ----------------------------------------------------------------------------
 */
void quit_handler(char* arg, char* doc) {
    char* token;
    // parse the argument
    token = strtok(arg, " \t\n");
    if (token != NULL) {
        syntax_report(doc);
        return;
    } else {
        exit(0); /* this will close all open file descriptors */
    }
}


/*
 * ----------------------------------------------------------------------------
 * get_cmd_index()
 *  returns the index in command_array which line represents, or
 *         EMPTY_COMMAND 
 *  also returns the argument of the command (the rest of line except command)
 *  side effect: changes both line and argument
 *  assumes argument points to a pre-allocated string of length MAXLINE
 * ----------------------------------------------------------------------------
 */ 
int get_cmd_index(char* line, char* argument) {
    int i=0;
    char* token;

    token = strtok(line, " \t\n");
    if (token == NULL) return EMPTY_COMMAND;  
    argument[0] = '\0'; 
    while (cmd_list[i].name != 0) {
        if (!strcasecmp(token, cmd_list[i].name)) {
            token = strtok(NULL, "");
            if (token != NULL) {
                strncpy(argument, token, MAXLINE);
                argument[MAXLINE] = '\0'; /* make sure it is NULL terminated */
            }
            break;
        }
        i++;
    }
    return i;
}

/*
 * ----------------------------------------------------------------------------
 *  syntax_report(..)
 *  just report the syntax of a command
 * ----------------------------------------------------------------------------
 */  
void syntax_report(const char* doc) {
    printf("SYNTAX: %s", doc); /* assuming doc always finish with a \n */
    fflush(stdout);  
}

/*
 * ----------------------------------------------------------------------------
 * is_valid_conn_id(char* conn_id)
 *   if the string conn_id represent a valid connection id, return the integer
 *   return -1 otherwise
 * ----------------------------------------------------------------------------
 */
int is_valid_conn_id(char* conn_id) {
    // check if conn_id is an integer first
    char** temp_ptr = (char**) malloc(4);
    int cid = strtol(conn_id, temp_ptr, 10);
    if ( ((*temp_ptr) == conn_id) || ((*temp_ptr)[0] != '\0') ) {
        free(temp_ptr);
        fflush(stdout);
        return -1;
    }
    free(temp_ptr);

    // check if it is in range
    if ( (cid >=0) && (cid < MAX_NO_CONNS) ) {
        return cid;
    } else {
        return -1;
    }
}



// BIN to DEC Function  - REFERENCE: http://www.daniweb.com/forums/thread297172.html


unsigned int binary_to_decimal(char *inputString) {

        unsigned int result = 0;

        char *p = inputString;

        while (*p == '0' || *p == '1') {
                result *= 2;
                result += (*p - '0');
                p++;
        }
        return result;
}


// END OF FILE ..
