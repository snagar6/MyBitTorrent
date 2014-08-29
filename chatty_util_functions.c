/*
 * Program : chatty.c
 * Requires : chatty_util_functions.c ,network_util.c ,chatty.h
 * Description :
 * Utility functions for Chatty application , MNC project 1 
 *
 * To execute :
 * On command line type ./chatty <tcp_port> <udp_port>
 *
 * Author :
 * Shreyas Nagaraj
 * UB # : 36471719
 *
 * Pramod Kundapur Nayak
 * UB # : 37646792
 *
 * Date : 10-04-2010
*/

#include "include/ubtorrent.h"
#include "include/connections.h"

NODE first_peerList = NULL;

/*
 * Function : send_tcp_message
 *
 * Description : Sending the TCP Messages while using the "send" command
 * Parameters  : conn_id  -> connection id
 *  		 full_message_buffer -> contains message to be sent 
 * Output : sends the message using tcp conneciton 
 */

/*
int send_tcp_message(int conn_id, char full_message_buffer[])
{
	int message_length;
 	uint32_t total_length;	
	uint32_t sender_pid;
	struct tcp_packet *packet;
	ssize_t nbytes;
	NODE currNode;

	// do validations
	if ((full_message_buffer == NULL) || (strlen(full_message_buffer) == 0)) 
	{
		fprintf(stderr,"send_tcp_message(): Message empty\n");
		return 1;
	}
	message_length = strlen(full_message_buffer);
	sender_pid = (uint32_t)getpid();
	packet = (struct tcp_packet *)malloc(sizeof(struct tcp_packet)); 

	// create a packet	
	total_length = (uint32_t)(message_length + 4);
	packet->len = htonl(total_length); 
	packet->sender_pid = htonl(sender_pid); // network byte ordering - BIG-ENDIAN
	memset(packet->message,'\0',4096);
	full_message_buffer[message_length+1]='\n';	
	memcpy(packet->message,full_message_buffer,4096);
			
	currNode=first;
       
	// loop throught the linked list of connection ids 
	if (currNode == NULL)
	{
        	fprintf(stderr,"send_tcp_message(): No Connections to send \n");
		return 1;
	}
	while (currNode !=NULL)
	{
		if(currNode->conn_id==conn_id)
			break;
		else
        	{
        		currNode=currNode->link;
        	}
	}	
	if (currNode == NULL)
	{
		fprintf(stderr,"send_tcp_message(): Connections ID not found to send \n");
		return 1;
	}	
	// send the tcp packet
	nbytes = writen(currNode->socket_desc, packet, sizeof(struct tcp_packet));
	if (nbytes != sizeof(struct tcp_packet)) 
	{
		 fprintf(stderr,"send_tcp_message(): Message send failed! \n");
                 return 1;
        }
	return 0;
}
*/
	
/*
 * Function : getNode
 *
 * Description : get a new node in linked list
 * Parameters  : none 
 * Output : new node 
 */

NODE getNode()
{
	NODE temp=malloc(sizeof(struct peerList));
	temp->next = NULL;
	return temp;
}

/*
 * Function : insertLL 
 *
 * Description : insert connection data into linked list 
 */

int insertLL(char ipAddr[],size_t port)
{
	NODE cNode=NULL;	
	NODE temp=getNode();
	temp->next = NULL;
	memset(temp->ipaddr,'\0',INET_ADDRSTRLEN);
	strncpy(temp->ipaddr,ipAddr,INET_ADDRSTRLEN);
	temp->port = port;

	if(first_peerList == NULL)
	{
		first_peerList = temp;
	}
	else
	{
		cNode = first_peerList;
		while(cNode->next!=NULL){
			cNode=cNode->next;
		}	
		cNode->next=temp;
	}
	return 0;
}

/*
 * Function : disconnectLL
 *
 * Description : disconect all the connections 
 */

int disconnectALL()
{
	NODE temp;
	NODE prev;

	if (first_peerList == NULL) 
	{
		return 0;
	}

	temp = first_peerList;

	while (temp != NULL)
	{
		prev = temp;
		temp = temp->next;
		free(prev);
	}
	first_peerList = NULL;
	return 0;
}

/*
 * Function : displayLL
 *
 * Description : displays the linked list details 
 * Parameters  : first -> first node of lnked list
 * Output : none
 */

int displayLL(NODE first_peerList)
{

	NODE temp;
	temp = first_peerList;
	if (temp == NULL) 
	{
		fprintf(stderr,"\n\n// Show(): No Active TCP Connections to show\n\n");
		return 1;
	}
	printf("\n-----------------------------------------------------------------------------");
	while(temp!=NULL)
	{
	 	printf("\n%-15s|%-12d",temp->ipaddr,(int)temp->port);
		temp=temp->next;  
	}	
	printf("\n");
	return 0;
}


// END OF Linked LIst Function

/* END */
