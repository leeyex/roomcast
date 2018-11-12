#ifndef _COMMON_H
#define _COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <ctype.h>
#include <stdarg.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <net/route.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <termios.h>
#include <pthread.h>


//static int my_time = 1;
//system("hwclock --hctosys");
//struct tm * ltm = localtime(&tv.tv_sec);(unsigned int64_t)tv.tv_sec
//#define PRINTF(fmt, args...) {printf("[%s:%d][%s]: ", __FILE__,__LINE__,__FUNCTION__);printf(fmt, ##args);}
//#define PRINTF(fmt, args...) {}
#define LOG(module,event){char tmpbuf[1024];\
						  struct timeval tv;\
						  gettimeofday(&tv, NULL); \
						  struct tm * ltm = localtime(&tv.tv_sec);\
						  sprintf(tmpbuf,"echo [%s][%s][%d-%d-%d-%d-%d-%d] >> /media/log/ms.log\n",module,event,ltm->tm_year+1900,ltm->tm_mon+1,ltm->tm_mday,ltm->tm_hour,ltm->tm_min,ltm->tm_sec);\
						  system(tmpbuf);}

//#define PRINTF(fmt, args...) {}
#define SA struct sockaddr
#define MAC_LEN 6
#define MAX_CHANNEL  7

#define THREAD_DETACH {if(pthread_detach(pthread_self()) != 0) return NULL;}

#define MS	1000

typedef enum {
	UNKNOW = 0,
	LEFT = 1,
	RIGHT = 2,
}PcmChannel;

#ifdef __cplusplus 
extern "C" {
#endif

/***set time in "yy-mm-dd hh:mm:ss" format***/
void  set_sys_time(char*  tm); 

/***set /dev/ttySx speed .this is right version,more good then len's***/
void init_tty_bautrate(char * ttyname ,unsigned int speed);


int select_socket(int Sock, int IsRead, int Min, int Sec);

int select_socket_block(int Sock, int IsRead);

int initTcpSocket();


/*
	recv len from sk ;
	retunr -1; error;
	return  0; success;
*/

int socket_recv(int Sock, char *Buf, int Len);

int socket_recv_timeout(int Sock, char *Buf, int Len, int sec);

int socket_send(int Sock, char *Buf, int Len);

int socket_send_timeout(int Sock, char *Buf, int Len);

int write_to_file(char *filename, unsigned char *buf, int len);
#ifdef __cplusplus 
}
#endif

#endif
