/*
 * Program  : ubtorrent.c
 * Description : This program simulates bitTorrent 
 *
 * To execute : 
 * On command line type ./ubtorrent <torrent file name > <tcp_port>
 * 
 * Author : Shreyas Nagaraj 
 * UB # : 36471719 
 * Date : 10-04-2010
 
 * Author : Pramod Kundapur Nayak
 * UB # : 37646792
 * Date : 10-04-2010
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "include/ubtorrent.h"
#include "include/connections.h"
#include "include/messages.h"
#include "include/net_utils.h"
#include "include/robust_io.h"

#define MAX 4096
#define MAX_ARGS 25

void message_handler(uint8_t message_id, int conn_fd, char *message_buffer, int pay_len);
void other_message_handler (uint8_t message_id, int conn_fd, char *message_buffer,int pay_len);


int _connect(char *destAddr,int destPort)
{
        int cli_sock;
        struct sockaddr_in server_addr;
        char str[INET_ADDRSTRLEN];
      
        
        // create an active socket to connect to another ubtorrent instance
        if((cli_sock=socket(AF_INET,SOCK_STREAM,0))<0)
        {
                fprintf(stderr, "socket(): failed\n");
                return 1;
        }

        // fill the details of the destination chatty instance
        memset(&server_addr,'\0',sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(destPort);
	inet_pton(AF_INET,destAddr,&server_addr.sin_addr);
      
        // connect logic if ip address given
        if(connect(cli_sock,(struct sockaddr *)&server_addr,sizeof(server_addr))<0)
        {
                fprintf(stderr, "_connect : connect(): failed\n");
                return 1;
        }

        inet_ntop(AF_INET,&(server_addr.sin_addr),str,INET_ADDRSTRLEN);
          
	printf("\n// Connection to %s : %d established \n",str,destPort);

	return (cli_sock);

}


/*
 * Parameters :
 * argv[1] - torrent file name 
 * argv[2] - tcp_port
 *
 */

int main(int argc, char *argv[])
{
	/* declare variables */	
	size_t len=0;
	char *cmd[]={"metainfo","announce","trackerinfo","show","status","quit"};
	char input[MAX]="\0";
	int i=0;
	char ch;
	char *token=NULL;
	int num_of_args;
	int opt;
	char arg[MAX_ARGS][40];
	int tcp_sock;
	int num_fd;
	fd_set read_stream;	
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	int tcp_fd=0,on,curr_fd;
	int read_bytes;
	char *rec_line;
	char cli_infohash[INFOHASH_LEN];
	int conn_fd;
	// For messages
	uint8_t  payload_identifier;
	uint32_t  payload_length;

	printf("\n\n*********WELCOME TO UBTORRENT*********\n\n"); 

	if (argc != 3) {
		printf("Usage: ./ubtorrent <torrent-file> <port_num> \n\n\n"); 
		exit(0);
	}

	tcpPort=atoi(argv[2]);


	if (atoi(argv[2]) > 65535 || atoi(argv[2]) < 1024) {
                printf("Usage: Give a Suitable TCP Port between 1024 - 65000\n\n\n");
                exit(0);
        }


	// open a passive socket for tcp
        if((tcp_sock=socket(AF_INET,SOCK_STREAM,0))==-1){
                fprintf(stderr,"ERROR : opening socket");
                exit(1);
        }

        memset((char *)&server_addr,0,sizeof(server_addr));
        server_addr.sin_family=AF_INET;
        server_addr.sin_port = htons(tcpPort); 
        server_addr.sin_addr.s_addr=htonl(INADDR_ANY);

        // set socket options to reuse address
        if(setsockopt(tcp_sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0){
                fprintf(stderr,"ERROR in setsockopt(): ");
                exit(1);
        }
        // bind the scoket to a port
        if(bind(tcp_sock,(struct sockaddr *)&server_addr,sizeof(server_addr))){
                fprintf(stderr,"Error in Binding");
                exit(EXIT_FAILURE);
        }

	init_tcp_conns();
        // listen for tcp connections
        if(listen(tcp_sock,10)==-1){
                fprintf(stderr,"Error in Listening");
                exit(EXIT_FAILURE);
        }


	fnParseMetaFile(argv[1]);
	FD_ZERO(&read_stream);
        num_fd=max(tcp_sock,0)+1;

	/* infinite loop to keep returning to prompt */
	while(1)
	{


		printf("%s","ubTorrent>>");
		fflush(stdin);
                fflush(stdout);


                FD_ZERO(&read_stream);
                FD_SET(tcp_sock,&read_stream);
                //FD_SET(udp_sock,&read_stream);
                FD_SET(fileno(stdin),&read_stream);

		fd_set_conns(&read_stream);
                num_fd=max(max(0,tcp_sock),get_max_fd())+1;

                // use select to check which descriptor has inputs
                if((curr_fd=select(num_fd,&read_stream,NULL,NULL,NULL))<0)
                {
                        if(errno==EINTR)
                        {
                                continue;
                        }
                        else
                        {
                                perror("select()");
                                exit(1);
                        }
                }

		if(FD_ISSET(fileno(stdin),&read_stream))
                {
                        // if there is input at stdin
                        i=0;
                        while((ch=fgetc(stdin))!='\n')
                        {
                                input[i++]=ch;
                        }

                        input[i]='\0';
                        len=strlen(input);

			/* if input is "exit" then close program */
			if(strcmp(input,"quit")==0)
			{
				fnSendTracker("stopped");	
				exit(0);
			}


                        if(len>MAX || len<=1 || input[i]==' ')
                        {
                                printf("\nInvalid Input !\n");
                                len=0;
                        }
                        else
                        {
                                // break the input into tokens
				memset(arg[0],'\0',40);
                                i=0;
                                token=strtok(input," ");
                                while(token!=NULL)
                                {
                                        strncpy(arg[i],token,strlen(token));
                                        strcat(arg[i],"\0");
                                        token=strtok(NULL," ");
                                        i++;
                                }
                                num_of_args = i;
                                opt=10;
                                for(i=0;i<6;i++)
                                {
                                        if(strncmp(arg[0],cmd[i],strlen(cmd[i]))==0)
                                        {
                                                opt=i;
                                                break;
                                        }
                                }

                                switch(opt){
                                             
					case 0:
                                       		if(num_of_args == 1)
                                       		{
                                         		metainfo();
                                                }
                                                else
                                                {
                                                        printf("Usage: metainfo \n");
                                                }
                                                break;
                                        case 1:// announce command
                                                if(num_of_args == 1)
                                                {
                                                        fnAnnounce("started"); 
							rec_line = (char *)malloc(meta_data.pieceLength*sizeof(char)+20);
                                                }
                                                else
                                                {
                                                        printf("Usage: announce\n");
                                                }
                                                break;
                                        case 2: // TrackerInfo command
                                                if(num_of_args == 1)
                                                {
                                                        fnTrackerInfo();
                                                }
                                                else
                                                {
                                                        printf("Usage: trackerinfo \n");
                                                }
                                                break;
					case 3: // show command
                                                if(num_of_args == 1)
                                                {
							fnShow();
                                                }
                                                else
                                                {
                                                        printf("Usage: show\n");
                                                }
                                                break;
					case 4: // status command
                                                if(num_of_args == 1)
                                                {
							fnStatus();
                                                }
                                                else
                                                {
                                                        printf("Usage: status \n");
                                                }
                                                break;
					case 5: // Quit command
                                                if(num_of_args == 1)
                                                {
                                                }
                                                else
                                                {
                                                        printf("Usage: quit \n");
                                                }
                                                break;

					default: printf("Invalid command\n");
						
					break;

				}
			}
		}
		 else if(FD_ISSET(tcp_sock,&read_stream))
                 {
                                // to accept incomming tcp connectionis
				client_addr_len=sizeof(struct sockaddr);

                                if((tcp_fd=accept(tcp_sock,(struct sockaddr *)&client_addr,&client_addr_len)) > 0)
                                {
                                        // printf(" Adding new connection \n");
                                        add_conn(tcp_fd);
                                }

                }

		for(i=0; i<10; i++) {
	            conn_fd = get_conn_fd(i);	
                    if (conn_fd!= -1) 
		    {
                         if (FD_ISSET(conn_fd,&read_stream))
                         {

                            read_bytes=-1;
		
	//	printf(" (get_peer_attr(2,conn_fd) : %d \n\n\n",(get_peer_attr(2,conn_fd)));

			    if(get_peer_attr(2,conn_fd) == 0)
			    {
	         	    	memset(rec_line,'\0',meta_data.pieceLength*sizeof(char)+20);
		                read_bytes = read(conn_fd,rec_line,68);
			    }
			    else 
		            {
				memset(&payload_length,0,4);
				read_bytes = readn(conn_fd,rec_line,5);
				memcpy(&payload_length, rec_line, 4);
				payload_length = ntohl(payload_length);

				if(read_bytes<0)
					continue;

				if (read_bytes > 0 && payload_length == 0) 
				{
					printf(" \n ***** RECEIVED A KEEP ALIVE MESSAGE FROM PEER: %d \n",conn_fd);  
					continue;
				}
				else if ( read_bytes > 0 && payload_length > 0)
				{
					memset(&payload_identifier,0,1);
					memcpy(&payload_identifier,rec_line+4,1);
					if ((payload_length - 1) > 0)
					{	
						memset(rec_line,'\0',meta_data.pieceLength*sizeof(char)+20);
						read_bytes = readn(conn_fd,rec_line,(payload_length - 1));
					}
			//		printf(" \n ***** RECVD THE MESSAGE IDENTIFIER : %d ****\n",payload_identifier);				
				}
			    }		
//			    printf("\n *******ORIGNINALLY READ BYTES : %d ****\n",read_bytes);
	

				if(read_bytes==0)
                                {
				        // printf("\n\n// Connection number %d  was closed by peer \n\n",conn_fd);
                                        remove_conn(conn_fd);
					close(conn_fd);
                                }
                                
				else if (read_bytes > 0)
                                {
					if(get_peer_attr(1,conn_fd) == 0)
                                	{
                                                memset(cli_infohash,'\0',INFOHASH_LEN);
                                                memcpy(cli_infohash,rec_line+28,20);
						// memset(rec_line,'\0',meta_data.pieceLength*sizeof(char)+20);

                                                if(memcmp(cli_infohash,meta_data.info_hash,20)==0) 
						{
							printf("\n ***************  INFOHASH OF THE HANDSHAKE VERIFIED ********* \n");
							 printf("\n *************** HANDSHAKE SENT ******************** \n");
                                                        peer_message_handshake(conn_fd);
							printf("\n");
							set_peer_attr(1,conn_fd,1);
							// set_peer_attr(2,conn_fd,1);

							if((get_peer_attr(2,conn_fd) == 0)) 
		                                        {
								printf("\n ******** BITFIELD MESSAGE SENT *********** \n");
                	                                        peer_message_bitfield(conn_fd);
								set_peer_attr(2,conn_fd,1);
                         	                        } 
						}
						else 
						{
							printf("\n ***************  INFOHASH OF THE HANDSHAKE VERIFICATION FAILED  ********* \n");
							remove_conn(conn_fd);
		                                        close(conn_fd);
							printf("**************** CONNECTION REMOVED FROM THIS CLIENT ********* \n"); 
						}
					}
			
					else if (get_peer_attr(1,conn_fd) == 1 && read_bytes == 68 )
					{
						 printf("***** HANDSHAKE MESSAGE RECVD FROM CONN-ID: %d \n",conn_fd);
						if (read_bytes  == 68)
						{
							if(get_peer_attr(2,conn_fd) == 0)
                                                        {
                                                                printf("\n ******** THE BITFIELD MESSAGE SENT *********** \n");
                                                                peer_message_bitfield(conn_fd);
								set_peer_attr(2,conn_fd,1);
                                                        }
						}
					}
					
                                        else if (get_peer_attr(1,conn_fd) == 1 && read_bytes != 68  )
					{
                				if (payload_identifier == 5) // Bitfield Message ONly
                				{ 
							message_handler(payload_identifier, conn_fd, rec_line, payload_length);
							// memset(rec_line,'\0',meta_data.pieceLength*sizeof(char)+20);
						}
						else if (payload_identifier != 5)
						{
					//		printf(" \n ***** RECVD THE MESSAGE IDENTIFIER : %d ****\n",payload_identifier);
					//		printf("\n ***** PAYLOAD LENGTH OF RECVD MESSAGE: %d ****\n",payload_length);
							other_message_handler(payload_identifier,conn_fd,rec_line,payload_length);
							// memset(rec_line,'\0',meta_data.pieceLength*sizeof(char)+20);
						}
					}
				}
                         }
                     }
                 }
	}
	return 0;
}

// ONLY for MSGID:5 - BITFIELD 

void message_handler(uint8_t message_id, int conn_fd, char *message_buffer, int pay_len) 
{
    unsigned char * payload_bitfield;

    if (message_id == 5) // Bitfield Message ONly
    {
	printf("\n ******** RECEIVED A BIT FIELD MESSAGE ******* (MSG ID:5) ***************  \n");
//	printf("\n ***** PAYLOAD LENGTH OF RECVD BITFIELD MESSAGE: %d ****\n",pay_len);

	payload_bitfield = (unsigned char*)malloc(sizeof(char)*(pay_len-1));
	memset(payload_bitfield,'\0',sizeof(char)*(pay_len-1));
	memcpy(payload_bitfield, message_buffer, (pay_len)-1);


	if (meta_data.size_bitfield_bytes != (pay_len-1))	
	{
		remove_conn(conn_fd);
		close(conn_fd);
		printf("\n Connection Closed: Receive bitfields are not of the correct size \n");
	}
	else
	{
                    	bitfield_setting(conn_fd, payload_bitfield, (sizeof(char)*(pay_len-1)));
			// peer_message_unchoke(conn_fd);
                        set_peer_attr(3,conn_fd,0);
        }
    }

}


// Other Message Types 

void other_message_handler (uint8_t message_id, int conn_fd, char *message_buffer, int pay_len)
{
 
	FILE *fp; 
	int reqt_piece;
	char *temp_buffer;
	char *temp_bitfield;
	uint32_t pieceIndex;
	uint32_t pieceBegin;
	uint32_t length_request;
	int piece_pay_len = 0;
	int i;	

	fp = fopen(meta_data.file_name, "ab");

	switch(message_id)
	{

		case 0: // Message - CHOKE message is fixed-length and has no payload
			printf("\n\n ******** RECEIVED A  CHOKE MESSAGE ******* (MSG ID:0) ***************  \n\n");	
			set_peer_attr(4,conn_fd,1);
			break;


		case 1: // Message - UNCHOKE message is fixed-length and has no payloadi
			printf("\n\n ******** RECEIVED UNCHOKE MESSAGE FROM PEER *** (MSG ID:1) ******  \n\n");
			set_peer_attr(4,conn_fd,0);

			int cnt2 = 0; 
			if (seed_leach_flag == 0)
                        {
                                reqt_piece = piece_required();
				if (reqt_piece == -1)
                                {
                                        printf("** Am NOT INTERESTED IN REMOTE PEER ***\n");
                                        peer_message_not_interested(conn_fd);
                                        set_peer_attr(5,conn_fd,0);
                                }

				while(reqt_piece!= -1 && reqt_piece!=(meta_data.size_bitfield))
                                {
                                        temp_bitfield = bitfield_recv(conn_fd);
                                        if(is_bitfield_set(temp_bitfield,reqt_piece))
                                        {
					   if (cnt2 == 0) 
					     {		
        	                                printf("\n**** THIS CLIENT SENDING - INTERESTED MESSAGE TO A SEEDER *******\n");
	        	                        peer_message_interested(conn_fd);
                        	                set_peer_attr(5,conn_fd,1);
					     }
	
                                             if(!get_peer_attr(3,conn_fd))
                                             {
                                                    printf("\n **SENDING REQUEST MESSAGE (MESG ID:6) and THE REQUESTED PIECE: %d \n",reqt_piece);
                                                    peer_message_request(conn_fd,reqt_piece);
						    meta_data.bitfield_snapshot[reqt_piece] = '1';
                                             }
                                         }
					cnt2++;
					reqt_piece = piece_required();
                                  }
                         }
			 
			break;


		case 2: // Message - Interested message is fixed-length and has no payload
			printf("\n\n ******** RECEIVED A INTERESTED MESSAGE ******* (MSG ID:2) ***************  \n\n");
	//		printf("\n *** SENDING A UNCHOKE MESSAGE FOR A CLIENT - WHO IS INTERESTED IN A PIECE I HAVE ** \n"); 
			peer_message_unchoke(conn_fd);
			set_peer_attr(3,conn_fd,0);
			set_peer_attr(6,conn_fd,1);
                        break;
                

		case 3: // Message - Not interested message is fixed-length and no payload
			printf("\n\n ******** RECEIVED A NOT INTERESTED MESSAGE ******* (MSG ID:3) ***************  \n\n");
	//		printf("\n** PEER/CLIENT NOT INTERESTED - Setting AM_CHOKING FlAG and Re-setting PEER INTERESTED FLAG to 0 *** \n"); 
			// set_peer_attr(3, conn_fd,1);
			set_peer_attr(6, conn_fd,0);
			set_peer_attr(2, conn_fd,1);	
			// peer_message_choke(conn_fd);
			break;


		case 4: // Message - HAVE message is fixed length
			printf("\n\n ******** RECEIVED A HAVE MESSAGE ******* (MSG ID:4) ***************  \n\n");
			break;		

		
		case 6: // Message - REQUEST: <len=0013><id=6><index><begin><length> 
			printf("\n\n ******** RECEIVED A REQUEST MESSAGE ******* (MSG ID:6) ***************  \n\n");
			if ( (get_peer_attr(3, conn_fd) == 0) &&  (get_peer_attr(6, conn_fd) == 1) )  // IF THIS PEER IS NOT choking THE REMOTE PEER
			{
				pay_len = pay_len - 1;
				memset(&pieceIndex, 0, 4);
                        	memcpy(&pieceIndex, message_buffer, 4);
                       
				memset(&pieceBegin, 0, 4);
                        	memcpy(&pieceBegin, message_buffer+4, 4);

				memset(&length_request, 0, 4);
                                memcpy(&length_request, message_buffer+8, 4);

				uploaded+=ntohl(length_request);

				peer_message_piece(conn_fd, ntohl(pieceIndex), ntohl(pieceBegin), ntohl(length_request));

				for (i=0; i<MAX_NO_CONNS; i++) {
                                        if (conn_array[i].sock_fd == conn_fd) {
                                                conn_array[i].uploaded+=ntohl(length_request);
                                                break;
                                        }
                                }

			}	
			else {
				printf("\n ISSUE: I AM CHOKING THE REMOTE PEER: %d , Hence CAnnot Process ReQuest Message !\n",conn_fd);
			}

			break;

		
		case 7: // Message - PIECE:<len=0009+X><id=7><index><begin><block> 
			printf("\n\n ******** RECEIVED A PIECE MESSAGE ******* (MSG ID:7) ***************  \n\n");


			pay_len = pay_len - 1;
			piece_pay_len = pay_len - 8;
			memset(&pieceIndex, 0, 4);
			memcpy(&pieceIndex, message_buffer, 4);
			memset(&pieceBegin, 0, 4);
			memcpy(&pieceBegin, message_buffer+4, 4);

			left-=piece_pay_len;
                        downloaded+=piece_pay_len;
                        temp_buffer = message_buffer+8;


                         for (i=0; i<MAX_NO_CONNS; i++) {
                                if (conn_array[i].sock_fd == conn_fd) {
                                            conn_array[i].downloaded+=piece_pay_len;
                                            break;
                                }
                        }

			fseek(fp, (ntohl(pieceIndex) * piece_pay_len) + 1, SEEK_SET);

			fwrite(temp_buffer, piece_pay_len, 1, fp);

			if (ntohl(pieceIndex) == (meta_data.file_length/meta_data.pieceLength))
			{
				printf("\n\n CONGRATS: FILE DOWNLOADED - %s \n\n",meta_data.file_name);
				fnSendTracker("completed");	
			} 
		
			fclose(fp);
			break;


		case 8: // Message - CANCEL: <len=0013><id=8><index><begin><length> 
			printf("\n\n ******** RECEIVED A CANCEL MESSAGE ******* (MSG ID:8) ***************  \n\n");
			break;


		default:
			printf("\n\n ******** RECEIVED A INVALID MESSAGE !!!  ***************  \n\n");
			break;
	
	}

}	     
