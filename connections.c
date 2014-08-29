/*
 * UBtorrent: Project 2 (MNC Fall 2010)
 *
 * Modified by: Shreyas Nagaraj
 *              Pramod Nayak
 *
 * Leveraged From  Hung Q. Ngo (Original Author) Project 1 Implmentation
 * *****************************************************************************
 * file name   : connections.c
 * author      : Hung Q. Ngo (hungngo@cse.buffalo.edu)
 * description : manage TCP connections
 * *****************************************************************************
 */

/*
 * -----------------------------------------------------------------------------
 * headers related to networking functions
 * -----------------------------------------------------------------------------
 */
#include <string.h> /* strtok, mem*, and stuff                */
#include <stdio.h>  /* fgets, etc., perror,                   */

#include "include/common_headers.h"
#include "include/error_handlers.h"
#include "include/connections.h"
#include "include/net_utils.h"

extern void print_c_n_times(char c, int n);

/*
 * ----------------------------------------------------------------------------
 * print_tcp_conns()
 *  print active tcp connections in a "pretty" format
 * ----------------------------------------------------------------------------
 */
void print_tcp_conns()
{
    if (number_of_conns() == 0) {
        printf("No active TCP connections to show\n");
        fflush(stdout);
        return;
    }

    int a=strlen("IP address"); /* IP field length          */
    int b=strlen("Host name");  /* host name field length   */
    int c=strlen("Local");      /* local port field length  */
    int d=strlen("Remote");     /* remote port field length */
    int status = strlen("Status"); /* Current Status */
    int bit_field = strlen("Bit Field"); /* Bit field Snapshot */
    int i;

    // find out how long each field needs to be
    for (i=0; i<MAX_NO_CONNS; i++) {
        if (conn_array[i].cs == CONNECTED) {
            a = max(a, strlen(conn_array[i].ip));
            b = max(b, strlen(conn_array[i].name));
            c = max(c, strlen(conn_array[i].l_port));
            d = max(d, strlen(conn_array[i].r_port));
	    bit_field = max(bit_field, strlen(conn_array[i].bitfield));
        }
    }

   printf(" %-*s | %-*s | %-*s | %-*s | %-*s | %-*s | %-*s\n", 
            3, "cID",
            a, "IP address",
            b, "Host name",
            c, "Local",
            d, "Remote",
	    status, "Status",
            bit_field, "Bit Field");
    print_c_n_times('-', a+b+c+d+status+bit_field+16); putchar('\n');

     for (i=0; i<MAX_NO_CONNS; i++) {
        if (conn_array[i].cs == CONNECTED) {
        printf(" %3d | %-*s | %-*s | %-*s | %-*s | %d%d%d%d | %s\n", i,
             a, conn_array[i].ip,
             b, conn_array[i].name,
             c, conn_array[i].l_port,
             d, conn_array[i].r_port,
             conn_array[i].am_choking_flag,
             conn_array[i].am_interested_flag,
             conn_array[i].peer_choking_flag,
	     conn_array[i].peer_interested_flag,
             conn_array[i].bitfield);
    }
  }
  fflush(stdout);
}

/*
 * ----------------------------------------------------------------------------
 * get_conn_fd(int cid)
 *   return connection fd of connection whose id is cid
 *   -1 if cid is not inactive
 * ----------------------------------------------------------------------------
 */
int get_conn_fd(int cid)
{
    int found=-1;
    if ( (cid >= 0) && (cid < MAX_NO_CONNS) ) {
        if (conn_array[cid].cs == CONNECTED) {
            found = conn_array[cid].sock_fd;
        }
    }
    return found;
}

/*
 * ----------------------------------------------------------------------------
 * get_max_fd()
 *    return maximum sock_fd of all active connections
 *    -1 if no connection is active
 * ----------------------------------------------------------------------------
 */
int get_max_fd()
{
    int i, maxfd=-1;
    for (i=0; i<MAX_NO_CONNS; i++) {
        if (conn_array[i].cs == CONNECTED) {
            maxfd = max(maxfd, conn_array[i].sock_fd);
        }
    }
    return maxfd;
}

/*
 * ----------------------------------------------------------------------------
 * fd_set_conns(read_set*)
 *   mark all sock_fd of active connections for select()
 * ----------------------------------------------------------------------------
 */
void fd_set_conns(fd_set* read_set_ptr)
{
    int i;
    for (i=0; i<MAX_NO_CONNS; i++) {
        if (conn_array[i].cs == CONNECTED) {
            FD_SET(conn_array[i].sock_fd, read_set_ptr);
        }
    }
}

/*
 * ----------------------------------------------------------------------------
 * remove_conn(int conn_id) 
 *   remove the connection whose ID is conn_id
 *   return -1 if the connection is not active
 *   return the connection's socket_fd if it is
 * ----------------------------------------------------------------------------
 */
int remove_conn(int conn_fd)
{
    int i, fd=-1;
    for (i=0; i<MAX_NO_CONNS; i++) {
        if ( (conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
            conn_array[i].cs = DISCONNECTED;
            fd = conn_array[i].sock_fd;
            break;
        }
    }
    return fd;
}
/*
 * ----------------------------------------------------------------------------
 * add_conn(int sock_fd) 
 *   add a new connection to the connection array
 *   return 0 on success, -1 on failure (array full)
 * ----------------------------------------------------------------------------
 */
int add_conn(int sock_fd)
{
  char ip[STR_SIZE], name[STR_SIZE], l_port[STR_SIZE], r_port[STR_SIZE];
  if ( get_sock_info(sock_fd, ip, name, l_port, r_port) < 0 ) {
    printf("Can't get info about socket \n");
    return -1;
  }

  int i, success=-1;
  for (i=0; i<MAX_NO_CONNS; i++) {
    if (conn_array[i].cs == DISCONNECTED) {
      conn_array[i].cs      = CONNECTED;
      conn_array[i].sock_fd = sock_fd;
      strncpy(conn_array[i].ip, ip, STR_SIZE-1);
      strncpy(conn_array[i].name, name, STR_SIZE-1);
      strncpy(conn_array[i].l_port, l_port, STR_SIZE-1);
      strncpy(conn_array[i].r_port, r_port, STR_SIZE-1);
      conn_array[i].am_interested_flag = 0;
      conn_array[i].peer_interested_flag = 0;
      conn_array[i].am_choking_flag = 1;
      conn_array[i].peer_choking_flag = 1;
      conn_array[i].hs_flag_1 = 0;
      conn_array[i].download_speed = 0;
      conn_array[i].upload_speed = 0;
      conn_array[i].uploaded=0;
      conn_array[i].downloaded=0;
      conn_array[i].start_time=0;
      success = 0;
      break;
    }
  }
  return success;
}

/*
 * ----------------------------------------------------------------------------
 * number_of_conns() 
 *   returns the number of active TCP connections
 * ----------------------------------------------------------------------------
 */
int number_of_conns()
{
    int i, count=0;
    for (i=0; i<MAX_NO_CONNS; i++) {
        if (conn_array[i].cs == CONNECTED) count++;
    }
    return count;
}

/*
 * ----------------------------------------------------------------------------
 * init_tcp_conns() : initialize the connection array
 * ----------------------------------------------------------------------------
 */
void init_tcp_conns()
{
  int i,temp;
  for (i=0; i<MAX_NO_CONNS; i++) {
    conn_array[i].cs      = DISCONNECTED;
    conn_array[i].sock_fd = -1;
    (conn_array[i].ip)[0]     = '\0';
    (conn_array[i].name)[0]   = '\0';
    (conn_array[i].l_port)[0] = '\0';
    (conn_array[i].r_port)[0] = '\0';
    for(temp = 0; temp <256; temp++) 	
    	conn_array[i].bitfield[temp] = '\0';
    conn_array[i].am_choking_flag = 1;
    conn_array[i].am_interested_flag = 0;
    conn_array[i].peer_choking_flag = 1;
    conn_array[i].peer_interested_flag = 0;
    conn_array[i].hs_flag_1 = 0;
    conn_array[i].hs_flag_2 = 0;
    conn_array[i].download_speed = 0;
    conn_array[i].upload_speed = 0;
    conn_array[i].uploaded=0;
    conn_array[i].downloaded=0;
    conn_array[i].start_time=0;

  }
}

/*
 * ------------------------------------------------------------------------------
 * Other Utility GET/SET routines needed to get and set the various fields/flags  
 * ------------------------------------------------------------------------------
 */

/* GET FUNCTIONS */ 

int get_peer_attr (int type, int conn_fd) 
{
  int i, val = 0;

  for (i=0; i<MAX_NO_CONNS; i++) {              // handshake flag 1 
  	if (type == 1) {
		if ( ( conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
      			val = conn_array[i].hs_flag_1;
      			break;
    		}
	}
	else if (type == 2) {     		// handshake flag 2 
                if ( ( conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        val = conn_array[i].hs_flag_2;
                        break;
                }
        }
	 else if (type == 3) {                   // am_chocking
                if ( ( conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        val = conn_array[i].am_choking_flag;
                        break;
                }
        }
	else if (type == 4) {                   // peer_chocking
                if ( ( conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        val = conn_array[i].peer_choking_flag;
                        break;
                }
        }
	else if (type == 5) {                   // am _ interested 
                if ( ( conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        val = conn_array[i].am_interested_flag;
                        break;
                }
        }
	else if (type == 6) {                   // peer_ peer interested
                if ( ( conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        val = conn_array[i].peer_interested_flag;
                        break;
                }
        }
  }
  return val;
}


/* SET FUNCTIONS */

void set_peer_attr (int type, int conn_fd, int flag)
{
  int i;

  for (i=0; i<MAX_NO_CONNS; i++) {              // handshake flag 1
        if (type == 1) {
		if (conn_array[i].sock_fd == conn_fd && conn_array[i].cs == CONNECTED) {
      			conn_array[i].hs_flag_1 = flag;
      			break;
    		}
	}
        else if (type == 2) {                   // handshake flag 2
		if (conn_array[i].sock_fd == conn_fd && conn_array[i].cs == CONNECTED) {
                        conn_array[i].hs_flag_2 = flag;
                        break;
                }
	}
	else if (type == 3) {                   // am_choking flag
		if ( (conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
      			conn_array[i].am_choking_flag = flag;
      			break;
    		}
        }
	else if (type == 4) {                   // peer_choking flag
		if ( (conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        conn_array[i].peer_choking_flag = flag;
                        break;
                }
        }
	else if (type == 5) {                   // am_interested flag
		if ( (conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        conn_array[i].am_interested_flag = flag;
                        break;
                }
        }
	 else if (type == 6) {                   // peer_interested flag
                if ( (conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
                        conn_array[i].peer_interested_flag = flag;
                        break;
                }
        }
  }

}

// BITFIELD OPERATIONS

char* bitfield_recv(int conn_fd)
{
  int i;
  char *val = "\0";

  for (i=0; i<MAX_NO_CONNS; i++) {
    if ( (conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
      val = conn_array[i].bitfield;
      break;
    }
  }
  return val;
}


void bitfield_setting(int conn_fd, unsigned char* bitfield, int size)
{
  int i;int j;int k;int cnt;

  for (i=0; i<MAX_NO_CONNS; i++) {
    if ( (conn_array[i].sock_fd == conn_fd) && (conn_array[i].cs == CONNECTED) ) {
	cnt = 0;

	for (j=0; j<size; j++) 
	{
		for(k=1; k<=8; k++) 
		{
			if(bitfield[j] & (0x01 << (8-k))) 
			{
				conn_array[i].bitfield[cnt++] = '1';			
			}
			else
			{
				conn_array[i].bitfield[cnt++] = '0';
			}
		}
	}
        break;
    }
  }
  // printf("\nDEBUG -  conn_array[i].bitfield: !%s! \n",conn_array[i].bitfield);

}

int is_bitfield_set(char *temp_bitfield,int piece_index)
{
	if(temp_bitfield[piece_index] == '1')	
	{
		return 1;
	}
	return 0;
}
	
// END of File !
