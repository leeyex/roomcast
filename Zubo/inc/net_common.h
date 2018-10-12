#ifndef NET_COMMON_H
#define NET_COMMON_H
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

#define MAC_LEN  6

/*	get command . ifname :eg. like "eth0"
 * */
#ifdef __cplusplus 
extern "C" {
#endif

int get_ip(char * ifname,char * ip);
int get_local_mask_string(char * ifname, char * mask);
unsigned int get_local_mask_hex(const char * ifname);
int get_local_mac_string(char * ifname, char * mac);
int get_local_mac_hex(const char * ifname,char * mac);
int get_default_gateway();

/*	set command . ifname :eg. like "eth0"
 * */
int set_local_ip(const char * ifname, unsigned int ip);
int set_local_mask(const char * ifname, unsigned int mask);
int set_default_gateway(const char * ifname, unsigned int gw);


int set_interface_up(const char * ifname);
int set_broadcast_route(const char * ifname);

int init_udpsend_socket(int size);
int init_dd_udprecv_socket(const int size, const int port, char *mip);

#ifdef __cplusplus 
}
#endif

#endif
