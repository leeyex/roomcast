#include <pthread.h>
#include "TcpClient.h"
#include "BaseMessage.h"
#include "TimeMessage.h"
#include "DiffHandler.h"
#include "DataMessage.h"
#include "Stream.h"
#include "DataList.h"

#include "OssPlayer.h"
//#include "AlsaPlayer.h"

Mp2_Codec TcpClient::mp2_handler;
FlacDecode TcpClient::flacDecode;

static int play_thd_running = 0;
static int send_thd_running = 0;
static int pro_thd_running = 0;


TcpClient::TcpClient()
{
	client_socket = 0;
	pthread_mutex_init(&mutex, NULL);
	TcpClient::mp2_handler.decode_init();
	TcpClient::flacDecode.decodeInit(44100, 16, 2);
}

TcpClient::~TcpClient()
{

}

int TcpClient::getClientSk()
{
	return	client_socket;
}

void TcpClient::setClientSk(int tag)
{
	client_socket = tag;
}

int TcpClient::mySend(unsigned char *buf, int len)
{
	int i;
	if ((i = send(client_socket, buf, len, MSG_NOSIGNAL)) < 0)
	{
		perror("mySend");
		return -1;
	}
	return i;
}

void TcpClient::setChannel(char *channel)
{
	pcm_chan_em = UNKNOW;
	if (channel == NULL)
		return;
	if (strcasecmp(channel, "left") == 0 || strcasecmp(channel, "l") == 0) {
		pcm_chan_em = LEFT;
	}
	else if (strcasecmp(channel, "right") == 0 || strcasecmp(channel, "r") == 0) {
		pcm_chan_em = RIGHT;
	}
}

int TcpClient::getChannel()
{
	return pcm_chan_em;
}

int TcpClient::startClient(char *ip_, short port_)
{
	pthread_mutex_lock(&mutex);
	if (!ip_)
		goto ERROR;
	memcpy(ip, ip_, sizeof(ip));
	port = port_;
	int rt;
	pthread_t thd;
	if (client_socket > 0)
		goto ERROR;
	client_socket = initTcpSocket();
	if (-1 == client_socket)
		goto ERROR;
	while (1) {
		rt = myConnect(ip, port);
		if (-1 == rt)
			sleep(1);
		else
			break;
	}
	if (!pro_thd_running) {
		pro_thd_running = 1;
		pthread_create(&thd, NULL, processThread, (void *)this);
	}
	if (!send_thd_running) {
		send_thd_running = 1;
		pthread_create(&thd, NULL, sendThread, (void *)this);
	}
	if (!play_thd_running) {
		play_thd_running = 1;
		pthread_create(&thd, NULL, playThread, (void *)this);
	}
	pthread_mutex_unlock(&mutex);
	return rt;
ERROR:
	pthread_mutex_unlock(&mutex);
	return -1;
}

int TcpClient::stopClient()
{
	pthread_mutex_lock(&mutex);
	cout << "i stop the client\n";
	DiffHandler::instance()->clearDiff();
	shutdown(client_socket, SHUT_RDWR);
	close(client_socket);
	client_socket = -1;
	play_thd_running = 0;
	send_thd_running = 0;
	pro_thd_running = 0;
	sleep(2);
	DataList<PcmChunk>::instance()->reInit();
	DataList<Mp2Chunk>::instance()->reInit();
	DataList<FlacChunk>::instance()->reInit();
	pthread_mutex_unlock(&mutex);
}


int TcpClient::myConnect(char *ip, short port)
{
	//PRINTF("Connect ip %s , port %d\n",ip,port);
	printf("Connect ip %s , port %d\n", ip, port);
	int sockfd = client_socket;
	struct sockaddr_in 	remote_addr;
	socklen_t			addrlen = sizeof(struct sockaddr_in);
	unsigned int		remote_ip = inet_addr(ip);

	remote_addr.sin_family = AF_INET;

	if (remote_ip != INADDR_NONE)
		remote_addr.sin_addr.s_addr = remote_ip;
	else
	{
		fprintf(stderr, "ip addr not correct : %s \n", ip);
		close(sockfd);
		return -1;
	}
	remote_addr.sin_port = htons(port);
	int rt = 0;
	if ((rt = connect(sockfd, (SA *)&remote_addr, addrlen)) != 0)
	{
		//PRINTF("connect ERROR: %s \n", strerror(errno));  
		printf("connect ERROR: %s \n", strerror(errno));  
	//	close(sockfd);
		return -1;
	}
	else
	{
		return 0;
	}

}

int TcpClient::cleanSocketBuffer(int fd)
{
	char * buf = (char *)malloc(2048);
	memset(buf, 0, 2048);
	int maxn = 2048;
	fd_set ReadSet;
	FD_ZERO(&ReadSet);
	FD_SET(fd, &ReadSet);
	struct timeval time;
	time.tv_sec = 0;   //  
	time.tv_usec = 1000;
	select(fd + 1, &ReadSet, NULL, NULL, &time);
	if (FD_ISSET(fd, &ReadSet))
	{
		int n;
		int rc;
		char c;
		char * ptr;
		ptr = buf;
		for (n = 1; n < maxn; n++) {
		again:
			if ((rc = recv(fd, &c, 1, 0)) == 1) {
				if (c == '\n')
					break;
				*ptr++ = c;
			}
			else if (rc == 0) {
				*ptr = 0;
				free(buf);
				return n - 1;
			}
			else {
				if (errno == EINTR)
					goto again;

				free(buf);
				return -1;
			}
		}
		*ptr = 0;
	}
	printf("clean socket. recv:%s\n", buf);
	free(buf);
	return 0;

}


void* TcpClient::sendThread(void *tcp_client)	//only send timeMsg	  
{
	THREAD_DETACH;
	BaseMessage baseMsg;
	unsigned char buf[1024];
	int sendlen = 0;
	int rt = 0;
	int tag = 0;
	while (send_thd_running) {
		int client_sk = ((TcpClient *)tcp_client)->getClientSk();
		TimeMessage timeMsg;
		sendlen = timeMsg.serialize(buf);
		if (client_sk > 0) {
			rt = socket_send(client_sk, (char *)buf, sendlen);
			if (rt != sendlen ) {
				printf("send time msg error, send:%s\n", strerror(errno));
				break;
			}
			if (tag < 20) {				//first init the time
				tag++;
				usleep(50*MS);
			}
			else {
				sleep(1);
			}
			
		}
		else {
			break;
		}
	}
	send_thd_running = 0;
}

void *TcpClient::processThread(void *tcp_client_)
{
	THREAD_DETACH;
	int client_sk = ((TcpClient *)tcp_client_)->getClientSk();
	TcpClient *tcp_client = (TcpClient *)tcp_client_;
	BaseMessage baseMsg;
	unsigned char buf[5120];
	int expect_len = sizeof(Msg_Head);
	int recv_len = 0;
	int send_len = 0;
	int count = 0;
	int pcm_len;
	int rt = 0;
	tv now;
	memset(&now, 0, sizeof(tv));
	timeval timeval_;
	int64_t buff_dac_time = (PCM_SIZE / 176400.) * 1000000 ;

	printf("before client recv\n");
	while (pro_thd_running) {
		rt = socket_recv(client_sk, (char *)buf, expect_len);
		if (rt <= 0 || errno == ECONNRESET || rt != expect_len) {
			goto SOCKET_ERR;
		}
		baseMsg.deSerialize(buf);
		tv t;
		baseMsg.head.received = t;
		if (baseMsg.head.magic != MAGIC) {
			printf("is error Magic\n");
			goto Error_Msg;
		}
		switch (baseMsg.head.type)
		{
		case BASE:
			break;
		case TIME:
		{
			TimeMessage timeMsg(baseMsg);
			rt = socket_recv(client_sk, (char *)buf, baseMsg.head.size);
			if (rt <= 0 || errno == ECONNRESET) {
				goto SOCKET_ERR;
			}
			memcpy(&timeMsg.latency, &buf, rt);
			DiffHandler *diffHandler = DiffHandler::instance();
			// timeMsg.lantency =  Server received - client sent
			if ((timeMsg.head.received - timeMsg.head.sent + timeMsg.latency).getMs() < 20/*100*/) {
				diffHandler->setDiff(timeMsg.latency, timeMsg.head.received - timeMsg.head.sent);
			}
			break;
		}
		case DATA:
		{
			DataMessage dataMsg;
			rt = socket_recv(client_sk, (char *)buf, baseMsg.head.size);
			if (rt <= 0 || errno == ECONNRESET) {
				goto SOCKET_ERR;
			}
			dataMsg.deSerialize(baseMsg, buf);
			if (dataMsg.data_info.type == MP2_DATA) {
//				printf("is mp2???\n");
				tcp_client->protocol_em = MP2_DATA;
				mp2_handler.decode((unsigned char *)dataMsg.payload, dataMsg.data_info.data_size, dataMsg.pcm_chunk.data_buf, &pcm_len);
				//				printf("pcm len= %d\n", pcm_len);
				dataMsg.pcm_chunk.time_stamp = dataMsg.data_info.time_stamp;
				dataMsg.pcm_chunk.data_size = pcm_len;

			}
			else if (dataMsg.data_info.type == FLAC_DATA) {
//				printf("is flac\n");
			
				tcp_client->protocol_em = FLAC_DATA;
//				cout << "dataMsg.data_info.data_size=" << dataMsg.data_info.data_size << endl;	
//				write_to_file("/tmp/recv.flac", (unsigned char *)dataMsg.payload, dataMsg.data_info.data_size);
				flacDecode.Decode((unsigned char *)dataMsg.payload, dataMsg.data_info.data_size, dataMsg.pcm_chunk.data_buf, (unsigned int *)&pcm_len);
//				printf("pcm len= %d\n", pcm_len);
				if (pcm_len != 0) {	
					if (now.sec == 0) {
						cout << "init the time\n";
						now = dataMsg.data_info.time_stamp;
						timeval_.tv_sec = now.sec;
						timeval_.tv_usec = now.usec;
					}
					else if ((dataMsg.data_info.time_stamp - tv(timeval_)).getMs() > CACHE_MS) {
						cout << "reinit the time\n";
						timeval_.tv_sec = dataMsg.data_info.time_stamp.sec;
						timeval_.tv_usec = dataMsg.data_info.time_stamp.usec;
					}
					else {
						count++;
						if (count == 100) {
							timeval_.tv_sec = dataMsg.data_info.time_stamp.sec;
							timeval_.tv_usec = dataMsg.data_info.time_stamp.usec;
							count = 0;
						}	 
					}

/*					if (now.sec == 0) {
						cout << "init the time\n";
						now = dataMsg.data_info.time_stamp;
						timeval_.tv_sec = now.sec;
						timeval_.tv_usec = now.usec;
					}
					else if ((dataMsg.data_info.time_stamp - tv(timeval_)).getMs() > CACHE_MS) {
						timeval_.tv_sec = dataMsg.data_info.time_stamp.sec;
						timeval_.tv_usec = dataMsg.data_info.time_stamp.usec;
					} */
					dataMsg.pcm_chunk.time_stamp = tv(timeval_);
					dataMsg.pcm_chunk.data_size = pcm_len;
					tv::addUs(timeval_, buff_dac_time);
				}
				else {
					if (now.sec == 0) {
						cout << "init the time\n";
						now = dataMsg.data_info.time_stamp;
						timeval_.tv_sec = now.sec;
						timeval_.tv_usec = now.usec;
					}
					break;
				}
				
			}
			DataList<PcmChunk>::instance()->writeToTail(&dataMsg.pcm_chunk);
			break;
		}
		case CONTROL:
			break;
		default:
			printf("is error type\n");
			goto Error_Msg;
			break;
		}
		recv_len = 0;
		expect_len = sizeof(Msg_Head);
	}
	pthread_mutex_lock(&tcp_client->mutex);
	shutdown(client_sk, SHUT_RDWR);
	close(client_sk);
	tcp_client->setClientSk(-1);
	pthread_mutex_unlock(&tcp_client->mutex);
	pro_thd_running = 0;
	play_thd_running = 0;
	send_thd_running = 0;
	return NULL;
Error_Msg:
	printf("error packet, close the connect\n");
	pthread_mutex_lock(&tcp_client->mutex);
	shutdown(client_sk, SHUT_RDWR);
	close(client_sk);
	pthread_mutex_lock(&tcp_client->mutex);
	tcp_client->setClientSk(-1);
	pthread_mutex_unlock(&tcp_client->mutex);
	pro_thd_running = 0;
	play_thd_running = 0;
	send_thd_running = 0;
	return NULL;
SOCKET_ERR:
	printf("rt = %d, errno = %d\n", rt, errno);
	perror("recv");
	pthread_mutex_lock(&tcp_client->mutex);
	if (tcp_client->client_socket == -1) {
		pthread_mutex_unlock(&tcp_client->mutex);
	}
	else {
		pthread_mutex_unlock(&tcp_client->mutex);
		tcp_client->stopClient();
		tcp_client->startClient(tcp_client->ip, tcp_client->port);
		
	}
	return NULL;
}


void* TcpClient::playThread(void *tcp_client_)
{
	THREAD_DETACH;
	cout << "play thread start\n";
	int client_sk;
	TcpClient *tcp_client = (TcpClient *)tcp_client_;
	int channel = tcp_client->getChannel();
	int protocol_em;
	PcmChunk *pcm_chunk = (PcmChunk *)malloc(sizeof(PcmChunk));
	if (pcm_chunk == NULL) {
		perror("malloc");
		return NULL;
	}

/*	AlsaPlayer *player = new AlsaPlayer;
	player->initAlsa();
	int rt = 0;*/
	OssPlayer *player;
	player = new OssPlayer(16, 2, 44100);
	int rt = player->init();
	if (rt == -1) {
		play_thd_running = 0;
		free(pcm_chunk);
		delete player;
		return NULL;
	}		

	unsigned char  *buf = (unsigned char  *)malloc(5120);
	if (buf == NULL) {
		play_thd_running = 0;
		free(pcm_chunk);
		delete player;
		perror("malloc");
		return NULL;
	}
	Stream stream_;
	PcmChunk p;
	int64_t buff_dac_time = (PCM_SIZE / 176400.) * 1000000;
	int data_len;
	int played_len = 0;

	while (play_thd_running) {
		client_sk = ((TcpClient *)tcp_client)->getClientSk();
		if (client_sk <= 0) {
			free(buf);
			free(pcm_chunk);
			delete player;
			play_thd_running = 0;
			return NULL;
		}
		if (tcp_client->protocol_em == MP2_DATA) {
			if (stream_.getPlayChunk(buf, buff_dac_time, PCM_SIZE / 4, &data_len, &play_thd_running)) {
				rt = player->playPcm(buf, data_len);
				played_len += rt;
				while (played_len != data_len && rt != -1) {
					printf("mp2 overload rt == %d\n", rt);
					rt = player->playPcm(buf+played_len, data_len - played_len);
					played_len += rt;
				}
				played_len = 0;	   
			}
			else {
				while (DataList<PcmChunk>::instance()->getSize() == 0 && play_thd_running) {
					usleep(100);
				}
			}
		}
		else if (tcp_client->protocol_em == FLAC_DATA) {	
			while (DataList<PcmChunk>::instance()->read(pcm_chunk) != 0 && play_thd_running) {
				usleep(100);
			}
//			buff_dac_time = (pcm_chunk->data_size / 176400.) * 1000000;
			while (play_thd_running) {
				int64_t serverTime = DiffHandler::instance()->serverNow() * 1000;
				if (abs((serverTime - pcm_chunk->time_stamp.getUs()) - CACHE_MS * 1000) <= buff_dac_time) {
					rt = player->playPcm(pcm_chunk->data_buf, pcm_chunk->data_size);
					played_len += rt;
					while (played_len != pcm_chunk->data_size && rt != -1) {
						printf("flac overload rt == %d\n", rt);
						rt = player->playPcm(pcm_chunk->data_buf + played_len, pcm_chunk->data_size - played_len);
						played_len += rt;
					}
					played_len = 0;	   
					break;
				}
				else if (((serverTime - pcm_chunk->time_stamp.getUs()) - CACHE_MS * 1000) > buff_dac_time)		// too old
				{
//					player->playPcm(pcm_chunk->data_buf, pcm_chunk->data_size);
					cout << "protocl flac client too old discard------" << 
						(serverTime - pcm_chunk->time_stamp.getUs()) - CACHE_MS * 1000 << endl;
					break;
				}
				usleep(100);
			}		   
		}
	}
	play_thd_running = 0;
	free(buf);
	free(pcm_chunk);
	cout << "delete oss player\n";
	delete player;
	return NULL;
}

