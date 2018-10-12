#ifndef _BDCP_H_
#define _BDCP_H_

#define BDCP_MAGIC              0xabcdef

//#define BDCP_NIC                "wlan0"
#define BDCP_MAC_LEN            6
#define BDCP_SERVER_PORT        1908
#define BDCP_CLIENT_PORT        1909
#define BDCP_DISCOVER_REQUEST   0x0001
#define BDCP_DISCOVER_RESPONSE  0x1001
#define BDCP_CONFIGURE_REQUEST  0x0002
#define BDCP_CONFIGURE_RESPONSE 0x1002

#define BDCP_MULTI_ADDR		"224.1.1.6"

#define BDCP_NIC                "wlan0"

#define MAX_DEV_COUNT 20

enum {
	UN_SET = 0,
	SERVER = 100,
	CLIENT = 101
};

typedef struct bdcp
{
	unsigned int  magic;
	unsigned int  code;
	unsigned int  ip;
	unsigned int mask;
	unsigned char mac[8];
	unsigned int  tag;
	unsigned char group_id[64];
	unsigned char version[20];
} bdcp_t;

#ifdef __cplusplus 
extern "C" {
#endif

unsigned int get_bdcp_ip();
void cre_bdcp_discover_pkt(bdcp_t * pkt);
int sendto_bdcp_server(int send_fd, bdcp_t * pkt);

void *task_bdcpd(void *arg);
void* handle_bdcp_client(void *arg);

unsigned int query_current_mode();
void set_current_mode(int tag);


#ifdef __cplusplus 
}
#endif

#endif /* __BDCP_H__ */
