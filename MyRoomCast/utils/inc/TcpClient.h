#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

#include "common.h"
#include "mp2_codec.h"
#include "FlacDecode.h"


/*
	TcpClient after connect has start under threads:

	processsThread:
		handle server msg, decode the mp2 data and put it in PcmList;
	sendThread: 
		just send the time msg to server;
	playThread: 
		get pcm data from PcmList, play it at the appropriate moment;

*/

class TcpClient
{
public:
	static Mp2_Codec mp2_handler;
	static FlacDecode flacDecode;
	char ip[16];
	short port;
	int client_socket;
	int protocol_em;
	pthread_mutex_t mutex;

	TcpClient();
	virtual ~TcpClient();
	int startClient(char *ip, short port);
	int stopClient();
	int getClientSk();
	void setClientSk(int tag);
	int mySend(unsigned char *buf, int len);
	void setChannel(char *channel);
	int getChannel();
private:
	PcmChannel pcm_chan_em;			// left \ right channel

	int myConnect(char *ip, short port);
	int cleanSocketBuffer(int fd);
protected:
	static void* sendThread(void *tcp_client);
	static void* processThread(void * tcp_client);
	static void* playThread(void *tcp_client);
	
};



#endif
