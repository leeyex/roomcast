#include <pthread.h>
#include "OssPlayer.h"
#include "TcpServer.h"
#include "TimeMessage.h"
#include "DataMessage.h"
#include "fifo_data.h"
#include "DataList.h"

const char *play_list[] = {
	"http://music.163.com/song/media/outer/url?id=436514312.mp3",
	"http://music.163.com/song/media/outer/url?id=574566207.mp3",
	"http://music.163.com/song/media/outer/url?id=138306.mp3",
	"http://music.163.com/song/media/outer/url?id=29723022.mp3",
	"http://music.163.com/song/media/outer/url?id=28815250.mp3",
	"http://music.163.com/song/media/outer/url?id=27090996.mp3",
	"http://music.163.com/song/media/outer/url?id=5264307.mp3",
	"http://music.163.com/song/media/outer/url?id=295245.mp3",
	"http://music.163.com/song/media/outer/url?id=28793140.mp3",
	"http://music.163.com/song/media/outer/url?id=1303019637.mp3",
	"http://music.163.com/song/media/outer/url?id=29004400.mp3",
	"http://music.163.com/song/media/outer/url?id=513791211.mp3",
	"http://mr1.doubanio.com/d4739ce4b6a3a3b9ba0db60b10220fec/0/fm/song/p2537992_128k.mp3",
	"http://mr1.doubanio.com/476322a6c49762eff43ce3fc5cd53e72/0/fm/song/p1488212_128k.mp3",
	//	"http://cdn.lizhi.fm/audio/2017/09/20/2625714442048966662_hd.mp3",
	"http://od.open.qingting.fm/m4a/59a6ba297cb8914779245b2e_7858040_64.m4a?u=758&channelId=177004&programId=7686110/?deviceid=1c88792002da",
	"http://fdfs.xmcdn.com/group3/M09/5B/3E/wKgDsVNYzxfDQrVWAGY8CUVCAwU357.mp3",
	"http://airsmart-photo1.oss-cn-shanghai.aliyuncs.com/m11.mp3",


	"end",
};


static pthread_mutex_t mutex;

Mp2_Codec TcpServer::mp2_handler;
FlacEncode TcpServer::flacEncode;

int server_delay = 0;

static int write_list_thd_running = 0;
static int play_thd_running = 0;
static int send_thd_running = 0;
static int accept_thd_running = 0;

int TcpServer::client[FD_SETSIZE];

TcpServer::TcpServer(char *ip, short port)
{
	memset(server_ip, 0, sizeof(server_ip));
	if (ip)
		memcpy(server_ip, ip, sizeof(server_ip) - 1);
	pthread_mutex_init(&mutex, NULL);
	server_port = port;
	server_socket = 0;
	//	TcpServer::mp2_handler.encode_init(44100, 256000, 2);
	TcpServer::mp2_handler.encode_init(44100, 256000, 2);
	TcpServer::flacEncode.encoderInit(44100, 16, 2);
	pcm_chan_em = UNKNOW;
	protocol_em = MP2_DATA;
}

TcpServer::~TcpServer()
{

}


int TcpServer::myBindAndListen(int listen_queue_size)
{
	struct sockaddr_in local;
	memset(&local, 0, sizeof(local));

	local.sin_family = AF_INET;
	local.sin_port = htons(server_port);
	if (strlen(server_ip) == 0)
		local.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		local.sin_addr.s_addr = inet_addr(server_ip);

	if (bind(server_socket, (struct sockaddr *)&local, sizeof(local)) == -1)
	{
		fprintf(stderr, "[%s:%d]%s::bind(): %s sockefd:%d\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno), server_socket);
		close(server_socket);
		return -1;
	}
	if (listen(server_socket, listen_queue_size) == -1)
	{
		fprintf(stderr, "[%s:%d]%s::listen(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
		close(server_socket);
		return -1;
	}
	return 0;

}

int TcpServer::getServerSk()
{
	return server_socket;
}

void TcpServer::setChannel(char *channel)
{
	if (channel == NULL)
		return;
	if (strcasecmp(channel, "left") == 0 || strcasecmp(channel, "l") == 0) {
		pcm_chan_em = LEFT;
		if (protocol_em == MP2_DATA)
			TcpServer::mp2_handler.encode_init(44100, 128000, 1);
	}
	else if (strcasecmp(channel, "right") == 0 || strcasecmp(channel, "r") == 0) {
		pcm_chan_em = RIGHT;
		if (protocol_em == MP2_DATA)
			TcpServer::mp2_handler.encode_init(44100, 128000, 1);
	}
}


void TcpServer::setProtocol(char *protocol)
{
	if (protocol == NULL)
		return;
	if (strcasecmp(protocol, "flac") == 0) {
		protocol_em = FLAC_DATA;
		//		TcpServer::mp2_handler.encode_init(44100, 128000, 1);
	}
	else if (strcasecmp(protocol, "mp2") == 0) {
		protocol_em = MP2_DATA;
	}
	else if (strcasecmp(protocol, "pcm") == 0) {
		protocol_em = PCM_DATA;
	}
}

int TcpServer::getProtocol()
{
	return protocol_em;
}

int TcpServer::getChannel()
{
	return pcm_chan_em;
}

int TcpServer::startServer(SERVER_MODE mode, int listen_queue_size)
{
	int rt = 0;
	pthread_t pid;
	server_socket = initTcpSocket();
	if (-1 == server_socket)
		return -1;
	rt = myBindAndListen(listen_queue_size);
	if (-1 == rt)
		return -1;

	switch (mode) {
	case SELECT:
		break;
	case EPOLL:
		break;
	case THREAD:
	default:
		accept_thd_running = 1;
		pthread_create(&pid, NULL, acceptThread, (void *)this);
	}
	return 0;
}

int TcpServer::stopServer()
{
	int client_sk;
	char buf[1024];
	int rt, i = 0;
	shutdown(server_socket, SHUT_RDWR);
	close(server_socket);
	write_list_thd_running = 0;
	play_thd_running = 0;
	send_thd_running = 0;
	accept_thd_running = 0;
	sleep(1);
	for (int i = 0; i < FD_SETSIZE; i++) {
		client_sk = client[i];
		if (client_sk > 0)
		{
			printf("block here\n");
			pthread_mutex_lock(&mutex);
			shutdown(client_sk, SHUT_RDWR);
			close(client_sk);
			client[i] = 0;
			pthread_mutex_unlock(&mutex);
		}
	}
	DataList<PcmChunk>::instance()->reInit();
	DataList<Mp2Chunk>::instance()->reInit();
	DataList<FlacChunk>::instance()->reInit();
}

int TcpServer::cleanSocketBuffer(int fd)
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
				printf("return from here\n");
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

void * TcpServer::writeListThread(void * tcp_server_)
{
	THREAD_DETACH;
	printf("write thread start\n");
	int rt = 0;
	int read_len = 0;
	int i, j, val = 0;
	int left_len, right_len = 0;
	int sleep_count = 1;
	unsigned char mp2[1024];
	unsigned char left[PCM_SIZE];
	unsigned char right[PCM_SIZE];
	char cmd[256];

	int mp2_len = 0;
	int read_target = 0;
	unsigned char *buf = NULL;
	buf = (unsigned char *)malloc(PCM_SIZE);
	FlacChunk *flacChunk = (FlacChunk *)malloc(sizeof(FlacChunk));
	memset(flacChunk, 0, sizeof(FlacChunk));
	TcpServer *tcp_server = (TcpServer *)tcp_server_;
	int protocol = tcp_server->getProtocol();
	int channel = tcp_server->getChannel();

	int dac = 0;
	static FILE *fp = NULL;
	if (fp == NULL)
	{
		fp = fopen("./test.ini", "r");
	}
	if (dac == 0) {
		char buf[8];
		memset(buf, 0, 8);
		if (fp) {
			fread(buf, 1, 8, fp);
			dac = atoi(buf);
			printf("dac == %d\n", dac);
			fclose(fp);
		}
		else {
			dac = 0;
		}
	}

	int64_t flac_duaration = (FLAC_PCM_SIZE / 176400.) * 1000000 + dac;
	int64_t mp2_duaration = (PCM_SIZE / 176400.) * 1000000 + dac;
	int read_zero_count = 0;
	int delay = 0;
	if (protocol == MP2_DATA) {
		read_target = PCM_SIZE;
	}
	else if (protocol == FLAC_DATA) {
		read_target = FLAC_PCM_SIZE;
	}
	tv now;
	timeval timeval_;
	timeval_.tv_sec = now.sec;
	timeval_.tv_usec = now.usec;
	while (write_list_thd_running) {
		rt = datafifo_read((char *)buf + read_len, read_target - read_len);
		if (rt < 0) {
			printf("read pipe error\n");
			continue;
		}
		else if (rt == 0) {
			printf("read pipe error rt == 0\n");
			read_zero_count++;
			if (read_zero_count == 20) {
				read_zero_count = 0;
				static int count = 0;
				const char *play_content = *(play_list + count);
				if (strcmp("end", play_content) != 0)
				{
					play_content = *(play_list + count);
					count++;
				}
				else {
					count = 0;
					play_content = *(play_list + count);
					count++;
				}

				snprintf(cmd, sizeof(cmd) - 1, "echo \"loadfile %s\" > /tmp/mpfifo", play_content);
				printf("cmd=%s\n", cmd);
				system(cmd);
				sleep(2);
				continue;
				//			sleep(10);
				//			exit(-1);
			}
			usleep(200 * MS);
			continue;
		}
		read_len += rt;
		if (protocol == FLAC_DATA && read_len == FLAC_PCM_SIZE)
		{
			TcpServer::flacEncode.Encode(buf, FLAC_PCM_SIZE, flacChunk->data_buf, (unsigned int *)&(flacChunk->data_size));	//spend about 1ms?	
	//			cout <<"add flac duaration="<<flac_duaration<<" sec="<<timeval_.tv_sec<<" usec"<<timeval_.tv_usec<< endl;
			tv current;
			if ((current - tv(timeval_)).getUs() > 2 * flac_duaration) {			// reset the time
				printf("reset time\n");
				timeval_.tv_sec = current.sec;
				timeval_.tv_usec = current.usec;
			}
			DataList<PcmChunk>::instance()->write(buf, FLAC_PCM_SIZE, tv(timeval_));
			flacChunk->time_stamp = tv(timeval_);
			tv::addUs(timeval_, flac_duaration + server_delay);
			if (flacChunk->data_size)														
				DataList<FlacChunk>::instance()->writeToTail(flacChunk);			//if List is full  clear all, put it to tail
			flacChunk->data_size = 0;
			read_len = 0;
			if (DataList<PcmChunk>::instance()->getSize() >= CACHE_MS / 13 * 2 - 50) {
				usleep(flac_duaration*sleep_count);
				sleep_count++;
			}
			else if (sleep_count > 1) {
				sleep_count--;
			}
//			usleep(flac_duaration - 1200);
			usleep(1000);
			//			usleep(15 * MS);

						//			usleep(24200);
			//			usleep(20000);

			/*			switch (channel) {
						case LEFT:
						case RIGHT:
							val = PCM_SIZE;
							i = j = 0;
							left_len = right_len = 0;
							while (val) {
								if (i < 2) {				 // 2 = bits per sample	/ 8
									left[left_len++] = buf[j++];
									i++;
									val--;
								}
								if (i >= 2) {
									right[right_len++] = buf[j++];
									i++;
									val--;
									if (i == 4)
										i = 0;
								}
							}
			//				write_to_file("/tmp/right.pcm", right, PCM_SIZE / 2);
							read_len = 0;
							usleep(24000);
							break;
						default:	*/
		}
		else if (protocol == MP2_DATA && read_len == PCM_SIZE) {
/*			TcpServer::mp2_handler.encode(buf, PCM_SIZE, mp2, &mp2_len);	//spend about 3.5ms
			tv now;
			DataList<PcmChunk>::instance()->write(buf, PCM_SIZE, now);
			DataList<Mp2Chunk>::instance()->write(mp2, mp2_len, now);
			read_len = 0;
			usleep(21200);	*/					// 	one mp2 packet takes 4608/(44100*4) sec, based on trying	
			TcpServer::mp2_handler.encode(buf, PCM_SIZE, mp2, &mp2_len);
			tv current;
			if ((current - tv(timeval_)).getUs() > 2 * mp2_duaration) {			//reset time
				printf("reset time\n");
				timeval_.tv_sec = current.sec;
				timeval_.tv_usec = current.usec;
			}
			DataList<PcmChunk>::instance()->write(buf, PCM_SIZE, tv(timeval_));
			DataList<Mp2Chunk>::instance()->write(mp2, mp2_len, tv(timeval_));
#ifdef  MY_TEST
			static FILE *fp = NULL;
			if (fp == NULL)
			{
				fp = fopen("./tmp.ini", "r");
			}
			if (delay == 0) {
				char buf[4];
				fread(buf, 1, 4, fp);
				delay = atoi(buf);
				printf("delay == %d\n", delay);
			}
#endif //  0
		//	delay = 1400;		 // based on trying
			delay = 0;
			tv::addUs(timeval_, mp2_duaration-delay + server_delay);		 
		
			read_len = 0;
//			usleep(21200);								   // based on trying

			if (DataList<PcmChunk>::instance()->getSize() >= CACHE_MS / 13 * 2 - 50) {
				usleep(mp2_duaration*sleep_count);
				sleep_count++;
			}
			else if (sleep_count > 1) {
				sleep_count--;
			}

			usleep(1000);		  

		}
	}
	write_list_thd_running = 0;
	free(buf);
	free(flacChunk);
	return NULL;
}

void *TcpServer::acceptThread(void *tcp_server)
{
	THREAD_DETACH;
	memset(client, 0, FD_SETSIZE);
	printf("start accept thread\n");
	int client_sk;
	pthread_t thd, thd_write, thd_send, thd_play;
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);

	while (accept_thd_running)
	{
		client_sk = accept(((TcpServer *)tcp_server)->getServerSk(), (struct sockaddr *)&client_addr, &addr_len);
		if (-1 == client_sk)
		{
			fprintf(stderr, "[%s:%d]%s::accept(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
			return NULL;
		}
		client[client_sk] = client_sk;
		pthread_create(&thd, NULL, processThread, (void *)client_sk);
		if (!write_list_thd_running) {
			write_list_thd_running = 1;
			pthread_create(&thd_write, NULL, writeListThread, tcp_server);

		}
		if (!send_thd_running) {
			send_thd_running = 1;
			pthread_create(&thd_send, NULL, sendThread, tcp_server);

		}
		if (!play_thd_running) {
			play_thd_running = 1;
			pthread_create(&thd_play, NULL, playThread, tcp_server);
		}
		printf("new client thread\n");
	}
	accept_thd_running = 0;
	return NULL;
}


void *TcpServer::processThread(void *c_sk)
{
	THREAD_DETACH;
	unsigned char buf[2048];
	int client_sk = *((int*)(&c_sk));;
	int expect_len = sizeof(Msg_Head);
	int recv_len = 0;
	int send_len = 0;
	int rt = 0;
	printf("before recv client sk = %d\n", client_sk);
	while (1) {
		rt = socket_recv(client_sk, (char *)buf, expect_len);
		if (rt <= 0 || errno == ECONNRESET || rt != expect_len) {
			cout << "recv base msg head error rt=" << rt << endl;
			goto SOCKET_ERR;
		}
		BaseMessage baseMsg;
		baseMsg.deSerialize(buf);
		tv t;
		baseMsg.head.received = t;
		if (baseMsg.head.magic != MAGIC)
			goto Error_Msg;
		switch (baseMsg.head.type)
		{
		case BASE:
			break;
		case TIME:
		{
			TimeMessage timeMsg(baseMsg);
			rt = socket_recv(client_sk, (char *)buf, baseMsg.head.size);
			if (rt <= 0 || errno == ECONNRESET) {
				cout << "recv time msg head error\n";
				goto SOCKET_ERR;
			}
			timeMsg.latency = timeMsg.head.received - timeMsg.head.sent;
			tv t1;
			timeMsg.head.sent = t;
			send_len = timeMsg.serialize(buf);
			if (timeMsg.head.magic != MAGIC) {
				printf("is error Magic\n");
				goto Error_Msg;
			}
			pthread_mutex_lock(&mutex);
			rt = socket_send(client_sk, (char *)buf, send_len);
			if (rt != send_len) {
				perror("send timemsg\n");
				pthread_mutex_unlock(&mutex);
				goto SOCKET_ERR;
			}
			pthread_mutex_unlock(&mutex);
			break;
		}
		case DATA:
			break;
		case CONTROL:
			break;
		default:
			goto Error_Msg;
			break;
		}
	}

Error_Msg:
	printf("error packet, close the connect\n");
	pthread_mutex_lock(&mutex);
	close(client_sk);
	client[client_sk] = 0;
	pthread_mutex_unlock(&mutex);
	return NULL;
SOCKET_ERR:
	printf("rt = %d, errno = %d\n", rt, errno);
	perror("recv");
	pthread_mutex_lock(&mutex);
	close(client_sk);
	client[client_sk] = 0;
	pthread_mutex_unlock(&mutex);
	return NULL;
}


void * TcpServer::sendThread(void *tcp_server_)
{
	THREAD_DETACH;
	DataMessage *dataMsg = new DataMessage;
	int client_sk;
	int sendlen = 0;
	int rt = 0;
	int tag = 0;
	int send_old_count = 0;
	unsigned char *buf = (unsigned char *)malloc(8192);
	if (buf == NULL) {
		perror("malloc");
		delete dataMsg;
		return NULL;
	}
	TcpServer *tcp_server = (TcpServer *)tcp_server_;
	int protocol = tcp_server->getProtocol();

	while (send_thd_running) {
		if (protocol == MP2_DATA) {
			rt = DataList<Mp2Chunk>::instance()->read(&dataMsg->mp2_chunk);
			if (rt == -1) {
				usleep(1 * MS);
				continue;
			}
			tv now;
			if ((now - dataMsg->mp2_chunk.time_stamp).getUs() / MS > CACHE_MS - send_old_count)
			{
				cout << " mp2_chunk is old time=" << (now - dataMsg->mp2_chunk.time_stamp).getUs() / MS << endl;
				if (send_old_count < CACHE_MS)
					send_old_count += 100;
				usleep(1 * MS);
				continue;
			}else {
				if(send_old_count > 0)
					send_old_count -= 100;
			}
			dataMsg->data_info.type = MP2_DATA;
			//			cout << "flac_chunk data_size=" << dataMsg->flac_chunk.data_size << endl;
			dataMsg->data_info.data_size = dataMsg->mp2_chunk.data_size;
			dataMsg->data_info.time_stamp = dataMsg->mp2_chunk.time_stamp;
			dataMsg->payload = (char *)dataMsg->mp2_chunk.data_buf;
			sendlen = dataMsg->serialize(buf);
		}
		else if (protocol == FLAC_DATA) {
			rt = DataList<FlacChunk>::instance()->read(&dataMsg->flac_chunk);
			if (rt == -1) {
				usleep(1 * MS);
				continue;
			}
			tv now;
			if ((now - dataMsg->flac_chunk.time_stamp).getUs() / MS > CACHE_MS - send_old_count)
			{
//				cout << " flac_chunk is old time=" << (now - dataMsg->flac_chunk.time_stamp).getUs() / MS << endl;
				if(send_old_count < CACHE_MS)
					send_old_count += 100;
				continue;
			}
			else {
				if(send_old_count > 0)
					send_old_count -= 100;
			}
			dataMsg->data_info.type = FLAC_DATA;
			//			cout << "flac_chunk data_size=" << dataMsg->flac_chunk.data_size << endl;
			dataMsg->data_info.data_size = dataMsg->flac_chunk.data_size;
			dataMsg->data_info.time_stamp = dataMsg->flac_chunk.time_stamp;
			dataMsg->payload = (char *)dataMsg->flac_chunk.data_buf;
			sendlen = dataMsg->serialize(buf);

		}

		if (tag++ % 500 == 0)
		{
			printf("sendlen = %d, sizeof(msghead)=%d\n", sendlen, sizeof(Msg_Head));
		}

		for (int i = 0; i < FD_SETSIZE; i++) {
			client_sk = client[i];
			if (client_sk > 0)
			{
				pthread_mutex_lock(&mutex);
				if (protocol == MP2_DATA) {
					rt = socket_send_timeout(client_sk, (char *)buf, sendlen);
				}
				else {
					rt = socket_send(client_sk, (char *)buf, sendlen);
				}
				if (rt != sendlen && rt != 0) {
					close(client_sk);
					client[i] = 0;
					printf("send data msg error, rt=%d sendlen=%d\n", rt, sendlen);
				}
				pthread_mutex_unlock(&mutex);
			}
		}
	}
	free(buf);
	delete dataMsg;
	send_thd_running = 0;
}

void * TcpServer::playThread(void *tcp_server_)
{
	THREAD_DETACH;
	PcmChunk *pcm_chunk = (PcmChunk *)malloc(sizeof(PcmChunk));
	if (pcm_chunk == NULL) {
		perror("malloc");
		return NULL;
	}
	TcpServer *tcp_server = (TcpServer *)tcp_server_;
	int protocol = tcp_server->getProtocol();
	OssPlayer *player;
	player = new OssPlayer(16, 2, 44100);
	player->init();
	int played_len = 0;
	int64_t delay = 0;		//unit us
	int64_t buff_dac_time = 0;
	if (protocol == MP2_DATA) {
		buff_dac_time = (PCM_SIZE / 176400.) * 1000000;
	}
	else if (protocol == FLAC_DATA) {
		buff_dac_time = (FLAC_PCM_SIZE / 176400.) * 1000000;
	}
	while (play_thd_running) {
		if (DataList<PcmChunk>::instance()->getSize() > 0)
		{
			int rt = DataList<PcmChunk>::instance()->read(pcm_chunk);
			//			write_to_file("/tmp/play.pcm", pcm_chunk->data_buf, pcm_chunk->data_size);
			while (play_thd_running) {
				if (protocol == MP2_DATA) {
					tv now;

					if (abs((now - pcm_chunk->time_stamp).getUs() - (CACHE_MS * 1000 + delay)) <=  buff_dac_time) {

						rt = player->playPcm(pcm_chunk->data_buf, pcm_chunk->data_size);
						played_len += rt;
						while (played_len != pcm_chunk->data_size) {
							printf("mp2 overload rt == %d\n", rt);
							rt = player->playPcm(pcm_chunk->data_buf + played_len, pcm_chunk->data_size - played_len);
							played_len += rt;
						}
						played_len = 0;			
						break;																						        
					}
					else if (((now - pcm_chunk->time_stamp).getUs() - (CACHE_MS * 1000 + delay)) >  buff_dac_time)		// too old
					{
						//					player->playPcm(pcm_chunk->data_buf, pcm_chunk->data_size);
						cout << "protocol mp2 server too old i drop it" <<
							((now - pcm_chunk->time_stamp).getUs()) - (CACHE_MS * 1000 + delay) << endl;
						break;
					}
					usleep(100);
					
				}
				else if (protocol == FLAC_DATA) {
					tv now;
					if (abs((now - pcm_chunk->time_stamp).getUs() - (CACHE_MS * 1000 + delay)) <= buff_dac_time) {

						rt = player->playPcm(pcm_chunk->data_buf, pcm_chunk->data_size);
						played_len += rt;
						while (played_len != pcm_chunk->data_size) {
							printf("flac overload rt == %d\n", rt);
							rt = player->playPcm(pcm_chunk->data_buf + played_len, pcm_chunk->data_size - played_len);
							played_len += rt;
						}
						played_len = 0;
						break;
					}
					else if (((now - pcm_chunk->time_stamp).getUs() - (CACHE_MS * 1000 + delay)) > buff_dac_time)		// too old
					{
						//					player->playPcm(pcm_chunk->data_buf, pcm_chunk->data_size);
						cout << "now.sec=" << now.sec << " now.usec=" << now.usec << " pcm_chunk.sec=" << pcm_chunk->time_stamp.sec
							<< " pcm_chunk.usec=" << pcm_chunk->time_stamp.usec << endl;
						//						cout << "protocol flac server too old  i drop it" <<
						//							(now - pcm_chunk->time_stamp).getUs() - (CACHE_MS * 1000 + delay) << endl;	
						break;
					}
					usleep(100);
				}
			}
		}
		usleep(100);
	}
	free(pcm_chunk);
	delete player;
	play_thd_running = 0;
}
