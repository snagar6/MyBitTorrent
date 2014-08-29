/*

 * Author : Shreyas Nagaraj
 * UB # : 36471719
 * Date : 10-04-2010

 * Author : Pramod Kundapur Nayak
 * UB # : 37646792
 * Date : 10-04-2010

*/


#include <string.h>
#include <stdlib.h>
#include "include/connections.h" 
#include "include/common_headers.h"
#include "include/ubtorrent.h"
#include "include/sha1.h"
#include "include/bencode.h"
#include "include/messages.h"
#include "include/robust_io.h"
#include <sys/stat.h>
#include <fcntl.h> 
#include <ctype.h>

#define BE_DEBUG 1
#define OPL 16

extern NODE first_peerList;
extern int _connect(char *destAddr,int destPort);
extern void print_c_n_times(char c, int n);


char this_hostname[MAXHOSTNAME] = "";
char current_ipaddress[INET_ADDRSTRLEN] = "";

void fnTrackerInfo();

void error(char *msg){

	printf("ERROR : %s\n",msg);
	exit(-1);
}

int itoa(char *temp,int len)
{

	int i;
	int val=0;
	int num;
	for(i=0;i<len-1;i++){
	 	if(isdigit(*(temp+i))){
			num=atoi(temp+i);	
			val*=10;
			val+=num;
		}		

	}	

return val;	

}

void hexdump_f(unsigned char *buf, int dlen)
{
	char	c[OPL+1];
	int	i, ct;

	if (dlen < 0) {
		printf("WARNING: computed dlen %d\n", dlen);
		dlen = 0;
	}
	for (i=0; i<dlen; ++i) {
		if (i == 0)
			printf("DATA: ");
		else if ((i % OPL) == 0) {
			c[OPL] = '\0';
			printf("\t\"%s\"\nDATA: ", c);
		}
		ct = buf[i] & 0xff;
		c[i % OPL] = (ct >= ' ' && ct <= '~') ? ct : '.';
		printf("%02x ", ct);
	}
	c[i%OPL] = '\0';
	for (; i % OPL; ++i)
		printf("   ");
	printf("\t\"%s\"\n", c);
}

unsigned long fnSearchStr(char *str, FILE *fp)
{

        unsigned long str_len,cur_pos=0;
        char *line;

        str_len=strlen(str);
        line=(char *)malloc(str_len);
        memset(line,'\0',str_len);

        fseek(fp,cur_pos,SEEK_SET);
        while(fread(line,1,str_len,fp)!=EOF){
                cur_pos++;
                fseek(fp,cur_pos,SEEK_SET);
                if(memcmp(line,str,str_len)==0)
                {
                        return (cur_pos-1);
                }
        }
	return (cur_pos);
}

int getMyIp(){

 	struct sockaddr_in si_other;
        struct sockaddr_in sa;
        unsigned int sa_len;
        int sample_udp_socket;
        struct hostent *he;
        struct in_addr ipv4addr;

        // get hostname
        if (gethostname(this_hostname, MAXHOSTNAME) < 0)
        {
                fprintf(stderr, "gethostname(): Failed to get the hostname!\n");
                return 1;
        }

        // create a udp socket and hit google dns to know host IP address
        if ((sample_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        {
                fprintf(stderr, "socket() :Sample UDP socket creation failed\n");
                return 1;
        }

        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(GOOGLE_SRV_PORT);

        if (inet_aton(GOOGLE_SRV_IP, &si_other.sin_addr)==0)
        {
                fprintf(stderr, "inet_aton() failed\n");
                return 1;
        }
        // connecting to google dns
        if (-1 == connect(sample_udp_socket, (struct sockaddr *)&si_other, sizeof(si_other)))
        {
                fprintf(stderr, "connect() :Sample UDP connect failed\n");
                close(sample_udp_socket);
                return 1;
	}	
 	sa_len = sizeof(sa);

        if (getsockname(sample_udp_socket, (struct sockaddr*)&sa, &sa_len) == -1)
        {
                fprintf(stderr, "getsockname(): failed\n");
                return 1;
        }

        memset(&current_ipaddress, '\0', sizeof(current_ipaddress));
        strncpy(current_ipaddress, inet_ntoa(sa.sin_addr), strlen(inet_ntoa(sa.sin_addr)));

        // collect data received from google dns response
        inet_pton(AF_INET, current_ipaddress, &ipv4addr);
        he = gethostbyaddr((void *)&ipv4addr, sizeof ipv4addr, AF_INET);
        strncpy(this_hostname, he->h_name, sizeof(he->h_name));

        // close the udp socket
        close(sample_udp_socket);
	return 0;
}


void fnHash(unsigned char *msg, unsigned char *hash, int msg_len)
{

	int         err;
  	SHA1Context sha;
  	unsigned char message_digest[20]; /* 160-bit SHA1 hash value */

	memset(hash,'\0',ID_LENGTH);
	err = SHA1Reset(&sha);
  	if (err) printf("SHA1Reset error %d\n", err);

  	/* 'seed' is the string we want to compute the hash of */
  	err = SHA1Input(&sha, (const unsigned char *)msg,msg_len);

  	if (err) printf("SHA1Input Error %d.\n", err );

  	err = SHA1Result(&sha, message_digest);

  	if (err) {
    		printf("SHA1Result Error %d, could not compute message digest.\n", err);
  	}	

	memcpy(hash,&message_digest,ID_LENGTH);

}


void fnCreateId(uint8_t *cid)
{


	int pid;
	char *temp_pid;	
	temp_pid=(char *)malloc(5*sizeof(char));
	memset(cid,'\0',ID_LENGTH);
	pid=getpid();

	uint8_t  temp_cid[ID_LENGTH];			
	memset(temp_cid,0,ID_LENGTH);
	memcpy(temp_cid,"aaaaaaaaaaaaaaa",15);
	memcpy(temp_cid,current_ipaddress,strlen(current_ipaddress));
	sprintf(temp_pid,"%d",pid);
	strncat((char *)temp_cid,temp_pid,strlen(temp_pid));
	fnHash((unsigned char*)temp_cid,(unsigned char *)cid,ID_LENGTH);	
}

// changes reqquird to be done and references - pramod
void hexdump_fmt(unsigned char *buf,int dlen)
{
	char	c[20+1];
	int	i, ct;
	int line=0;	

	if (dlen < 0) {
		printf("WARNING: computed dlen %d\n", dlen);
		dlen = 0;
	}
	for (i=0; i<dlen; ++i) {
		if (i == 0)
			printf(" ",c);
		if ((i % 20) == 0) {
			c[20] = '\0';
			//printf("\t\"%s\"\n%d: ",c,i);
			printf("\t\n  %d : ",line++);
		}
		ct = buf[i] & 0xff;
		c[i % 20] = (ct >= ' ' && ct <= '~') ? ct : '.';
		printf("%02x",ct); 	

	}
	c[i%20] = '\0';

	printf("\n\n");
}

void hexdump(unsigned char *buf,int dlen)
{
	char	c[16+1];
	int	i, ct;

	if (dlen < 0) {
		printf("WARNING: computed dlen %d\n", dlen);
		dlen = 0;
	}

 
	for (i=0; i<dlen; ++i) {
		if (i == 0)
			//printf(" ", c);
		if ((i % 16) == 0) {
			c[16] = '\0';
			//printf("\t\"%s\"\nDATA: ", c);
		}
		ct = buf[i] & 0xff;
		c[i % 16] = (ct >= ' ' && ct <= '~') ? ct : '.';
		printf("%02x",ct); 	

	}
	c[i%16] = '\0';
}


void fnParseMetaFile(char *torrent_file)
{
	unsigned long start,end;
	char file_buf[BUFFER_LENGTH];
	FILE *md=fopen(torrent_file,"rb");
	char *temp;
	char *temp1,*temp2;
	int temp_ret;
	int temp_fp;
	struct stat fileStat;

	if(md==NULL){
		error("Unable to open torrent file");	
	}

	// get my IP , in current_ipaddress variable
	getMyIp();
	//store torrent file name
	strcpy(meta_data.metainfo_file,torrent_file);

	//get announce URL 
	start=fnSearchStr("http://",md);
	end=fnSearchStr("/announce",md);	
	fseek(md,start,SEEK_SET);
	memset(meta_data.announceUrl,'\0',MAXLEN);
	fread(meta_data.announceUrl,1,((end+9)-(start)),md);
	
	//extract announce Port
		
	temp=strtok(meta_data.announceUrl,"/");
	temp=strtok(NULL,"");
	temp1=strtok(temp,"/");
	temp2=strtok(temp1,":");
	memset(meta_data.announceIP,'\0',INET_ADDRSTRLEN);
	strcpy(meta_data.announceIP,temp1);
	temp2=strtok(NULL,"");
	meta_data.announcePort=atoi(temp2);

	// get file length	
	fseek(md,0,SEEK_END);
	meta_data.file_length=ftell(md);

	// create client id
	fnCreateId(meta_data.peer_id);


	//ben decode all the data 
	fseek(md,0,SEEK_SET);
	fread(file_buf,1,BUFFER_LENGTH,md);
	be_node *node=be_decoden(file_buf,sizeof(file_buf));
	if(node){
		
		be_dump(node);
	}
	else{
		printf("\n Parsing failed !\n");
	}

	be_free(node);
	temp_fp = open(meta_data.file_name,O_RDONLY); 	
	temp_ret = fstat(temp_fp,&fileStat);
	if(temp_ret == 0) 
	{
//		printf("fileStat.st_size: %d and the meta_data.file_length: %ld \n",(int)fileStat.st_size,meta_data.file_length);
		if(fileStat.st_size == meta_data.file_length) 
		{
			seed_leach_flag = 1;
			printf("\n I am A Seeder !!\n");
			left=0;
                        downloaded=0;
		}
		else 
		{
			printf("\n Inconsistent File Deleted~!\n"); 
			unlink(meta_data.file_name);
		}
	}
	
	if (seed_leach_flag == 0) {
		 printf("\n I am a Leacher !!\n");
		 left=meta_data.file_length;
                downloaded=0;
        }
	close(temp_fp);

	memset(meta_data.info,'\0',MAXLEN);
	memset(meta_data.info_hash,'\0',INFOHASH_LEN);
	fseek(md,-1,SEEK_END);	
	end=ftell(md);
	fseek(md,0,SEEK_SET);
	char buf[8192];
	int bytes_read;
	int temp_fd = open(torrent_file,O_RDONLY);

	bytes_read = read(temp_fd,buf,8192);
	char *tok = strstr(buf,"4:infod");
	int temp_len = 0;
	tok+=6;
	temp_len = (int)((&buf[bytes_read-2]) - tok + 1); 
	memcpy(meta_data.info, tok, temp_len);	
	 // calculate info hash
	fnHash(meta_data.info, meta_data.info_hash,temp_len);

	close(temp_fd);
}


void metainfo()
{

	printf(" my IP/port   \t: %s/%d\n",current_ipaddress,tcpPort);
	printf(" my ID        \t: ");
	hexdump((void *)meta_data.peer_id,ID_LENGTH);
	printf("\n metainfo file\t: %s\n",meta_data.metainfo_file);
	printf(" info hash    \t: ");
	hexdump((void *)meta_data.info_hash,ID_LENGTH);
	printf("\n file name    \t: %s",meta_data.file_name);
	printf("\n piece length \t: %d\n",meta_data.pieceLength);
	printf(" file size    \t: %ld\n",meta_data.file_length);
	printf(" announce URL \t: %s\n",meta_data.announceUrl);
	printf(" pieces' hashes  :");
	hexdump_fmt((void *)meta_data.pieces,meta_data.numPieces);

}

/**************** CODE REFERED FROM http://www.geekhideout.com/urlcode.shtml ****************/
char from_hex(char ch) 
{
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char to_hex(char code) 
{
        static char hex[] = "0123456789abcdef";
        return hex[code & 15];
}

/* Returns a url-encoded version of str */
unsigned char *url_encode(unsigned char *str, int length) {
        unsigned char *pstr = str, *buf = malloc( length * 4 + 1), *pbuf = buf;
        while(length != 0) 
	{
                if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~')
                        *pbuf++ = *pstr;
                else  
                        *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
                pstr++;
                length--;
        }
        *pbuf = '\0';

        //*encoded_str_ln = (pbuf - buf + 1);
        return buf;
}


/********************************************************************************/

void fnAnnounce(char *eventStr)
{
	char requestMsg[MAXLEN];
	char receiveBuffer[MAXLEN];
	unsigned char *url_encoded_info_hash;
	unsigned char *url_encoded_peer_id;
	char port[8],uploaded_str[2],downloaded_str[2],left_str[2];
	int reqSock,num_bytes;
	struct sockaddr_in tracker_addr; 
	int num_bytes1;
	char *response;
	char *tracker_response = "\0";
	char dest_ip[INET_ADDRSTRLEN];
        uint32_t peers_ip;
        uint16_t peers_port;
        int j,plsize,plen;
        char *tt;
        char *peer_str;char *pport;
        int peer_desc;
        char peer_ext1[32],peer_ext2[10];
        char pStr[19]="BitTorrent protocol";
        uint8_t  pstrlen=19;
	 int i;
 
	memset(requestMsg,'\0',MAXLEN);
        memset(port,'\0',8);
        memset(uploaded_str,'\0',2);
        memset(downloaded_str,'\0',2);
        memset(left_str,'\0',2);
        strcpy(requestMsg,"GET /announce?info_hash=");

        url_encoded_info_hash = url_encode(meta_data.info_hash,20);
	
        strncat(requestMsg,(char *)url_encoded_info_hash,strlen((char *)url_encoded_info_hash));
        strncat(requestMsg,"&peer_id=",9);

	url_encoded_peer_id = url_encode(meta_data.peer_id,20);

        strncat(requestMsg,(char *)url_encoded_peer_id,strlen((char *)url_encoded_peer_id));
        strncat(requestMsg,"&port=",6);
        sprintf(port,"%d",tcpPort);
        strncat(requestMsg,port,strlen(port));
        sprintf(uploaded_str,"%d",uploaded);
        sprintf(downloaded_str,"%d",downloaded);
        sprintf(left_str,"%d",left);
        strncat(requestMsg,"&uploaded=",10);
        strncat(requestMsg,uploaded_str,strlen(uploaded_str));
        strncat(requestMsg,"&downloaded=",12);
        strncat(requestMsg,downloaded_str,strlen(downloaded_str));
        strncat(requestMsg,"&left=",6);
        strncat(requestMsg,left_str,strlen(left_str));
	strncat(requestMsg,"&compact=1&event=",17);
        strncat(requestMsg,eventStr,strlen(eventStr));
        strncat(requestMsg," HTTP/1.1\r\n\n\n",13);

	requestMsg[strlen(requestMsg)] = '\0';

	if((reqSock=socket(AF_INET,SOCK_STREAM,0))<0){
		error("Error creating request socket");
	}

	memset(&tracker_addr,'\0',sizeof(tracker_addr));
	tracker_addr.sin_family=AF_INET;
	tracker_addr.sin_addr.s_addr=inet_addr(meta_data.announceIP);
	tracker_addr.sin_port=htons(meta_data.announcePort);

	if(connect(reqSock,(struct sockaddr *)&tracker_addr,sizeof(tracker_addr))<0)
	{
		error("Unable to connect to tracker"); 
	}	

	if((num_bytes1=send(reqSock,requestMsg,MAXLEN,0))<0)
	{
                error("Unable to send request message");
        }

	while(1){
		num_bytes= recv(reqSock,receiveBuffer,sizeof(receiveBuffer),0);

		if(num_bytes>0)
		{	

			tracker_response=strstr(receiveBuffer,"d8");
			if(tracker_response==NULL)
			{
				printf("ANNOUNCE DINT GO THRU: Tracker did not respond properly! Please, TRY AGAIN ! \n\n");
				return;
			} 
			response=strtok(receiveBuffer,"\n");
			printf("++++ Tracker responded\n "); 
			// printf("%s\n",tracker_response);
		}

		if(num_bytes<0)
			error("Unable to receive tracker response");
		else if(num_bytes==0){
			printf("... Tracker has closed the conenction\n");
			break;
		}

		num_bytes=-1;

	}

	close(reqSock);

 
	//  GET THE Handshake Message ready
	peerData.handshakeMsg = (char *)malloc(68*sizeof(char));	


	pport=(char *)malloc(sizeof(char)*2);	
	tt=strstr(tracker_response,"peers");
	peer_str=strtok(tt,":");

	//memcpy(plsize,peer_str+5,sizeof(int));
	sscanf(peer_str+5,"%d",&plsize);
	tracker_data.peers=(plsize)/6;
	
	memset(peer_ext1,'\0',10);
	memset(peer_ext2,'\0',10);
	
	strncpy(peer_ext1,"peers",5);
	sprintf(peer_ext2,"%d",plsize);
	strcat(peer_ext1,peer_ext2);	
	plen = strlen(peer_ext1);	

	init_tcp_conns();

	peers_total_number = tracker_data.peers;

	disconnectALL();

	for(j=0;j<tracker_data.peers;j++)
	{
		memset(dest_ip,0,INET_ADDRSTRLEN);
                memcpy(&peers_ip,(tt+plen+1)+(6*j),4);
                memcpy(&peers_port,(tt+plen+1)+(6*j)+4,2);
                if(inet_ntop(AF_INET,&peers_ip,dest_ip,INET_ADDRSTRLEN)==NULL){
                        error("unable to convert ");
                }
		insertLL(dest_ip,ntohs(peers_port));	
		memset(peerData.handshakeMsg,'\0',68);
       		memcpy(peerData.handshakeMsg,&pstrlen,1);
        	memcpy(peerData.handshakeMsg+1,pStr,19);
        	memcpy(peerData.handshakeMsg+28,meta_data.info_hash,20);
        	memcpy(peerData.handshakeMsg+48,meta_data.peer_id,20);
 
		if( (strncmp(dest_ip,"127.0.0.1",strlen("127.0.0.1")) == 0) && (ntohs(peers_port) == tcpPort) )
		{
			continue;
		}
		peer_desc = _connect(dest_ip,ntohs(peers_port));

		 for (i=0; i<MAX_NO_CONNS; i++) {
                        if (conn_array[i].sock_fd == peer_desc) {
                                gettimeofday(&time1,NULL);
                                conn_array[i].start_time=(double)time1.tv_usec;
                                break;
                        }
                }

		printf("*******NEW CONN -FD : %d\n", peer_desc);

	if(peer_desc > 0) {
			add_conn(peer_desc);
			printf("\n  *************** FIRST PART OF HANDSHAKE ******************** \n");
			peer_message_handshake(peer_desc);
                	set_peer_attr(1,peer_desc,1);
		}
		else {
			close(peer_desc);
		}

	}

	print_tcp_conns();
	printf("**************\n");
    

	be_node *tracker_node=be_decoden(tracker_response,strlen(tracker_response));
	
	 if(tracker_node){
                tracker_dump(tracker_node);
        }
        else{
                error("Parsing of Tracker_node failed ");
        }

        free(tracker_node);

        fnTrackerInfo();

}


/*
 * trackerInfo()
 *
 *
 *
 */

void fnTrackerInfo()
{
	printf("\n");
	printf(" complete | downloaded | incomplete | interval | min interval |\n ");
	printf("--------------------------------------------------------------\n");
	printf("%10ld|%12ld|%12ld|%10ld|%14ld\n",tracker_data.complete,tracker_data.downloaded,tracker_data.incomplete,tracker_data.interval,tracker_data.minInterval);
	printf("--------------------------------------------------------------\n");
	printf("++++ Peer List (self included ):\n");
	printf("IP                | Port\n");
	displayLL(first_peerList);

}


/*
 * status()
 *
 *
 *
 */

void fnStatus()
{


        printf("\n");
   printf(" %-*s | %-*s | %-*s | %-*s \n",
            10, "Downloaded",
            10, "Uploaded",
            10, "Left",
            meta_data.length_bitfield, "My bit field");

        printf("---------------------------------------------------\n");
     printf(" %-10d | %-10d |  %-10d  | %-*s \n",downloaded,uploaded,left
             ,meta_data.length_bitfield, meta_data.bitfield_snapshot);
  fflush(stdout);

}

// SHOW COMMAND

 
void fnShow()
{
	double  t2,t3;

        if (number_of_conns() == 0) {
                printf("No active TCP connections to show\n");
                fflush(stdout);
                return;
        }

        printf("\n");
        int a=strlen("ID");/* ID field length */
        int b=strlen("IP address"); /* IP address field length          */
        int c=strlen("Status");  /* Status field length   */
        int d=strlen("Bitfield");      /* Bitfield length  */
        int e=strlen("Down/s");     /* Down field length */
        int f=strlen("Up/s");   /* Upload field length */
        int i;

    // find out how long each field needs to be
    for (i=0; i<MAX_NO_CONNS; i++) {
        if (conn_array[i].cs == CONNECTED) {
            b = max(b, strlen(conn_array[i].ip));
            d = max(d, strlen(conn_array[i].bitfield));
        }
    }
        e=10;
        f=10;

   printf(" %-*s | %-*s | %-*s | %-*s | %-*s | %-*s \n",
            2, "ID",
            b, "IP address",
            c, "Status",
            d, "Bitfield",
            e, "Down/s",
            f,"Up/s");
  print_c_n_times('-', a+b+c+d+e+f+16); putchar('\n');

 for (i=0; i<MAX_NO_CONNS; i++) {
        if (conn_array[i].cs == CONNECTED) {

	 gettimeofday(&time1,NULL);
        t2=(double)time1.tv_usec;

        t3=t2-(conn_array[i].start_time);

        conn_array[i].download_speed=(double)conn_array[i].downloaded/t3;

        conn_array[i].upload_speed=(double)conn_array[i].uploaded/t3;


        printf(" %2d | %-*s |  %d%d%d%d  | %-*s | %-10f | %-10f \n", i,
             b, conn_array[i].ip
             , conn_array[i].am_choking_flag
             , conn_array[i].am_interested_flag
             , conn_array[i].peer_choking_flag
             , conn_array[i].peer_interested_flag,
             d, conn_array[i].bitfield
             , conn_array[i].download_speed
             , conn_array[i].upload_speed);
    }
  }
  fflush(stdout);

}

void fnSendTracker(char *eventStr)
{
        char requestMsg[MAXLEN];
        char receiveBuffer[MAXLEN];
        unsigned char *url_encoded_info_hash;
        unsigned char *url_encoded_peer_id;
        char port[8],uploaded_str[2],downloaded_str[2],left_str[2];
        int reqSock,num_bytes;
        struct sockaddr_in tracker_addr;
        int num_bytes1;
        char *response;
        char *tracker_response = "\0";




        memset(requestMsg,'\0',MAXLEN);
        memset(port,'\0',8);
        memset(uploaded_str,'\0',2);
        memset(downloaded_str,'\0',2);
        memset(left_str,'\0',2);
        strcpy(requestMsg,"GET /announce?info_hash=");

        url_encoded_info_hash = url_encode(meta_data.info_hash,20);
        //printf("url_encoded_info_hash:%s and LEN:%d \n",url_encoded_info_hash,encoded_str_ln1);

        strncat(requestMsg,(char *)url_encoded_info_hash,strlen((char *)url_encoded_info_hash));
        strncat(requestMsg,"&peer_id=",9);

        url_encoded_peer_id = url_encode(meta_data.peer_id,20);
        //printf("url_encoded_peer_id:%s and LEN:%d \n",url_encoded_peer_id,encoded_str_ln2);

        strncat(requestMsg,(char *)url_encoded_peer_id,strlen((char *)url_encoded_peer_id));
        strncat(requestMsg,"&port=",6);
        sprintf(port,"%d",tcpPort);
        strncat(requestMsg,port,strlen(port));
        sprintf(uploaded_str,"%d",uploaded);
        sprintf(downloaded_str,"%d",downloaded);
        sprintf(left_str,"%d",left);
        strncat(requestMsg,"&uploaded=",10);
        strncat(requestMsg,uploaded_str,strlen(uploaded_str));
        strncat(requestMsg,"&downloaded=",12);
        strncat(requestMsg,downloaded_str,strlen(downloaded_str));
         strncat(requestMsg,"&left=",6);
        strncat(requestMsg,left_str,strlen(left_str));
        // strncat(requestMsg,"&compact=1&event=started HTTP/1.1\r\n\n\n",37);
        strncat(requestMsg,"&compact=1&event=",17);
        strncat(requestMsg,eventStr,strlen(eventStr));
        strncat(requestMsg," HTTP/1.1\r\n\n\n",13);

        requestMsg[strlen(requestMsg)] = '\0';

        if((reqSock=socket(AF_INET,SOCK_STREAM,0))<0){
                error("Error creating request socket");
        }

        memset(&tracker_addr,'\0',sizeof(tracker_addr));
        tracker_addr.sin_family=AF_INET;
        tracker_addr.sin_addr.s_addr=inet_addr(meta_data.announceIP);
        tracker_addr.sin_port=htons(meta_data.announcePort);

        if(connect(reqSock,(struct sockaddr *)&tracker_addr,sizeof(tracker_addr))<0)
        {
                error("Unable to connect to tracker");
        }

        if((num_bytes1=send(reqSock,requestMsg,MAXLEN,0))<0)
        {
                error("Unable to send request message");
        }

        while(1){
                num_bytes= recv(reqSock,receiveBuffer,sizeof(receiveBuffer),0);

                if(num_bytes>0)
                {

                        tracker_response=strstr(receiveBuffer,"d8");
                        if(tracker_response==NULL)
                        {
                                printf("ANNOUNCE DINT GO THRU: Tracker did not respond properly! Please, TRY AGAIN ! \n\n");
                                return;
                        }
                        response=strtok(receiveBuffer,"\n");
                        printf("++++ Tracker responded \n");
                        // printf("%s\n",tracker_response);
                }
		 if(num_bytes<0)
                        error("Unable to receive tracker response");
                else if(num_bytes==0){
                        printf("... Tracker has closed the conenction\n");
                        break;
                }

                num_bytes=-1;

        }

        close(reqSock);

        be_node *tracker_node=be_decoden(tracker_response,strlen(tracker_response));

        if(tracker_node){
                tracker_dump(tracker_node);
        }
        else{
                error("Parsing of Tracker_node failed ");
        }

        free(tracker_node);


}


// EOF
