/*
 * UBtorrent: Project 2 (MNC Fall 2010)
 *
 * Modified by: Shreyas Nagaraj
 *              Pramod Nayak
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>
#define MAXLEN 4096
#define INFOHASH_LEN 20
#define MAXHOSTNAME 1025
#define GOOGLE_SRV_IP "8.8.8.8"
#define GOOGLE_SRV_PORT 53
#define ID_LENGTH 20
#define BUFFER_LENGTH 4096
#define ONE_BLOCK_SIZE 2048 
//#define max(a,b) ((a)>(b)?(a):(b))

int left,downloaded;
uint16_t tcpPort;
int uploaded;

// Linked list maintained to store the details of the various connections.

int peers_total_number;

struct peerList {
  char ipaddr[INET_ADDRSTRLEN];     
  uint16_t port;
  struct peerList * next;
};

typedef struct peerList* NODE;
// NODE temp_peerList;
// NODE first_peerList;

// Tracker Info
struct trackerInfo{

	long int peers;
	long int complete;
	long int downloaded;
	long int incomplete;
	long int interval;
	long int minInterval;
}tracker_data;


// Peer data - extended set of data 
struct peer_data{

 char *handshakeMsg;
 unsigned char *bitfield;
 int size_bitfield;

}peerData;

// Meta info stucture 
struct metaInfo{

	char ipAddr[INET_ADDRSTRLEN];
	char id[INFOHASH_LEN];
	char torrent_name[MAXLEN];
	char file_name[MAXLEN];
	char metainfo_file[MAXLEN];
	long file_length;
	int numPieces;
	char bitfield_snapshot[256];
	int size_bitfield;
	int size_bitfield_bytes;
	int length_bitfield;	
	char pieces[BUFFER_LENGTH];
	unsigned char info[MAXLEN];
	unsigned char  info_hash[INFOHASH_LEN];
	int pieceLength;
	long int fileSize;
	char announceUrl[MAXLEN];
	char announceIP[INET_ADDRSTRLEN];
	int announcePort;
	unsigned char peer_id[ID_LENGTH];
	int seed_leach_flag;
	// int dwn;
}meta_data;

void metainfo();

// Seeder and leacher flag

int seed_leach_flag;

// Peer list functions

int insertLL(char [],size_t);
int disconnectALL();
int displayLL(NODE);


int itoa(char *temp,int len);
void hexdump_f(unsigned char *buf, int dlen);
unsigned long fnSearchStr(char *str, FILE *fp);
int getMyIp();
void fnHash(uint8_t *msg,uint8_t *hash,int msg_len);
void fnCreateId(uint8_t *cid);
void hexdump_fmt(unsigned char *buf,int dlen);
void hexdump(unsigned char *buf,int dlen);
//int hex2str(unsigned char *buf,int dlen,char hexVal[]);
void fnParseMetaFile(char *torrent_file);
void metainfo();
unsigned char *url_encode(unsigned char *str, int length);
void fnAnnounce(char *eventStr);
void fnTrackerInfo();
void fnStatus();
void fnShow();
void fnSendTracker(char *eventStr);

// EOF
