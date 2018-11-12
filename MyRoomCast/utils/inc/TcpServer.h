#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "common.h"
#include "mp2_codec.h"
#include "FlacEncode.h"

typedef enum{
	THREAD=0,
	SELECT,
	EPOLL,	
}SERVER_MODE;


/*
	TcpServer after init, start accept thread ;

	accept thread:
		after accept return, start three thread;

		processThread:
			handle the recv msg	from client;
		writeListThread:
			read pcm data from pipe;
			encode it to mp2 and write it to mp2List(used to send), also write it to pcmList at same time(used to play);
			the timestamp is marked when write to the list
		sendThread:
			read mp2 data from mp2List and send mp2 data to every connected client
		playThread:
			read pcm data from pcmList and play it at the appropriate moment;

*/

typedef struct socket_mutex {
	int socket;
	pthread_mutex_t mutex;
}Socket_Mutex;

class TcpServer
{
public:
	static Socket_Mutex client[FD_SETSIZE];
	static Mp2_Codec mp2_handler;
	static FlacEncode flacEncode;
private:
	char server_ip[16];
	short server_port;
	int server_socket;
	PcmChannel pcm_chan_em;			// left \ right channel
	int protocol_em;
public:
	TcpServer(char *ip, short port);		 
	virtual ~TcpServer();
	
	int startServer(SERVER_MODE mode, int listen_queue_size);	
	int stopServer();
	int getServerSk();
	void setChannel(char *channel);
	void setProtocol(char *protocol);
	int getProtocol();
	int getChannel();
protected: // thread about
	static	void * writeListThread(void * tcp_server) ;	
	static	void * acceptThread(void * tcp_server) ;
	static	void * processThread(void * client_sk) ;
//	static  void * sendThread(void *tcp_server);
	static  void * writeQueueThread(void *p);
	static  void * eachSendThread(void *client_sk);
	static  void * playThread(void *tcp_server);
private:
	int myBindAndListen(int listen_queue_size);
	int cleanSocketBuffer(int fd);
	
};




#endif
