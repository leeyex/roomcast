#include "bdcp.h"
#include "net_common.h"
#include "dotini_parser.h"
#include <pthread.h>
#include <stdio.h>

#define CONFIG_FILE "/usr/airsmart_data/config/system.ini"

static unsigned int current_mode = UN_SET;


void cre_bdcp_discover_pkt(bdcp_t * pkt)
{
	if (pkt == NULL)
		return;
	memset(pkt, 0, sizeof(bdcp_t));
	pkt->magic = ntohl(BDCP_MAGIC);
	pkt->code = htonl(BDCP_DISCOVER_REQUEST);
}

unsigned int query_current_mode()
{
	return current_mode;
}

void set_current_mode(int tag)
{
	current_mode = tag;
}


int sendto_bdcp_server(int send_fd, bdcp_t * pkt)
{
	if (send_fd == 0)
		return -1;
	struct sockaddr_in peer;
	memset(&peer, 0, sizeof(peer));
	peer.sin_family = AF_INET;
	peer.sin_port = htons(BDCP_SERVER_PORT);
	peer.sin_addr.s_addr = inet_addr(BDCP_MULTI_ADDR);

	int len = sendto(send_fd, pkt, sizeof(bdcp_t), 0, (const struct sockaddr *)&peer, sizeof(peer));
	if (len == -1)
		fprintf(stderr, "[%s:%d]%s::sendto(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));

}

static void mac_to_str(unsigned char mac[], char *str)
{
	memset(str, 0, 32);
	sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


static void convert_ip(unsigned int ip, char str[])
{
	unsigned int hip = ntohl(ip);
	memset(str, 0, sizeof(str));
	sprintf(str, "%d.%d.%d.%d", (hip & 0xFF000000) >> 24,
		(hip & 0xFF0000) >> 16, (hip & 0xFF00) >> 8, (hip & 0xFF));
}


unsigned int get_bdcp_ip()
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *psin = NULL;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return 0;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, BDCP_NIC, (sizeof(ifr.ifr_name) - 1));
	ifr.ifr_addr.sa_family = AF_INET;

	if (ioctl(fd, SIOCGIFADDR, &ifr) == -1)
	{
		close(fd);
		return 0;
	}

	close(fd);
	psin = (struct sockaddr_in *)(&(ifr.ifr_addr));
	return psin->sin_addr.s_addr;
}


void *task_bdcpd(void *arg)
{
	int fd;
	int tag = 0;
	int flag;
	int len;
	char ipstr[32];
	char str[32];
	char command[64];
	struct sockaddr_in host;
	struct sockaddr_in peer;
	socklen_t peerlen;
	bdcp_t pkt;
	int send_fd;
	unsigned int nip = 0;
	while ((nip = get_bdcp_ip()) == 0) {
		printf("nip == 0\n");
		sleep(1);
	}
	host.sin_addr.s_addr = nip;
	unsigned int nmask = get_local_mask_hex(BDCP_NIC);
	char mac[20];
	memset(mac, 0, 20);
	get_local_mac_hex(BDCP_NIC, mac);
	/* Detach itself */
	if (pthread_detach(pthread_self()) != 0)
		return NULL;

	set_interface_up(BDCP_NIC);
	char group_id[64];
	memset(group_id, 0, 64);
	int rt  = dotini_open(CONFIG_FILE);
	if (rt == -1) {
		printf("open %s error\n", CONFIG_FILE);
		return NULL;
	}
	snprintf(group_id, 63, "%s", dotini_get_string("upnp", "groupUuid"));
	printf("group_id=%s\n", group_id);

	fd = init_dd_udprecv_socket(1024 * 32, BDCP_SERVER_PORT, BDCP_MULTI_ADDR);
	send_fd = init_udpsend_socket(1024 * 32);
	/* Loop */
	for (; ;)
	{
		snprintf(group_id, 63, "%s", dotini_get_string("upnp", "groupUuid"));
		memset(&pkt, 0, sizeof(pkt));
		peerlen = sizeof(peer);
		len = recvfrom(fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&peer, &peerlen);
		if (len == -1)
		{
			fprintf(stderr, "[%s:%d]%s::recvfrom(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
			continue;
		}
		else if (len == 0)
		{
			fprintf(stderr, "[%s:%d]%s::recvfrom(): Got shutdown signal.\n", __FILE__, __LINE__, __FUNCTION__);
			break;
		}

		if (pkt.magic != ntohl(BDCP_MAGIC))
			continue;

		switch (ntohl(pkt.code))
		{
		case BDCP_DISCOVER_REQUEST:

			memcpy(pkt.mac, mac, BDCP_MAC_LEN);
			nip = get_bdcp_ip();
			pkt.ip = nip;
			pkt.mask = nmask;
			memcpy(pkt.group_id, group_id, sizeof(pkt.group_id) - 1);
			pkt.tag = current_mode;

			struct in_addr hip;
			hip.s_addr = pkt.ip;

			memcpy(ipstr, inet_ntoa(hip), 16);
			if (tag++ % 100 == 0) {
				printf("BDCP_DISCOVER_REQUEST:\nIP:%s current_mode=%d\n", ipstr, current_mode);
			}
			pkt.code = htonl(BDCP_DISCOVER_RESPONSE);
			break;
		case BDCP_DISCOVER_RESPONSE:
		default:
			continue;
		}

		memset(&peer, 0, sizeof(peer));
		peer.sin_family = AF_INET;
		peer.sin_port = htons(BDCP_CLIENT_PORT);
		peer.sin_addr.s_addr = inet_addr(BDCP_MULTI_ADDR);

		len = sendto(send_fd, &pkt, sizeof(pkt), 0, (const struct sockaddr *)&peer, sizeof(peer));
		if (len == -1)
			fprintf(stderr, "[%s:%d]%s::sendto(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
	}

	close(fd);
	close(send_fd);
	dotini_close();
	return NULL;
}


void* handle_bdcp_client(void *arg)
{
	if (pthread_detach(pthread_self()) != 0)
		return NULL;
	int fd;
	int len;
	bdcp_t server;
	bdcp_t pkt;
	fd = init_dd_udprecv_socket(1024 * 32, BDCP_CLIENT_PORT, BDCP_MULTI_ADDR);
	if (fd <= 0)
		exit(-1);
	int i;
	int tag = 0;
	char str[32];
	struct sockaddr_in peer;
	struct sockaddr_in host;
	unsigned int tmp1, tmp2;
	int ttt = sizeof(peer);
	int server_live_tag = 0;
	unsigned int local_ip = 0;
	while ((local_ip = get_bdcp_ip()) == 0) {
		printf("local_ip == 0\n");
		sleep(1);
	}
	host.sin_addr.s_addr = local_ip;
	unsigned int local_mask = get_local_mask_hex(BDCP_NIC);
	
	char group_id[64];
	memset(group_id, 0, 64);
	int rt = dotini_open(CONFIG_FILE);
	if (rt == -1) {
		printf("open %s error\n", CONFIG_FILE);
		return NULL;
	}
	snprintf(group_id, 63, "%s", dotini_get_string("upnp", "groupUuid"));
	printf("group_id=%s\n", group_id);
	while (1) {
		snprintf(group_id, 63, "%s", dotini_get_string("upnp", "groupUuid"));
		len = recvfrom(fd, &pkt, sizeof(pkt), 0, (struct sockaddr *) &peer, (socklen_t *)&ttt);
		if (len == -1) {
			fprintf(stderr, "[%s:%d]%s::recvfrom(): %s\n",
				__FILE__, __LINE__, __FUNCTION__,
				strerror(errno));
			continue;
		}
		else if (len == 0) {
			fprintf(stderr,
				"[%s:%d]%s::recvfrom(): Got shutdown signal.\n",
				__FILE__, __LINE__, __FUNCTION__);
			close(fd);
			return NULL;
		}
		else if (len == -2) {
			close(fd);
			return NULL;
		}
		if (pkt.magic != ntohl(BDCP_MAGIC))
			continue;
		switch (ntohl(pkt.code)) {
		case BDCP_DISCOVER_RESPONSE:
			//	convert_ip(pkt.mac, str);
			mac_to_str(pkt.mac, str);
			
			if (tag % 100 == 0) {
				printf("mac=%s\n", str);
			}
			convert_ip(pkt.ip, str);
			if (tag++ % 100 == 0) {
				printf("ip = %s\n", str);
				printf("pkg.tag=%d\n", pkt.tag);
			}
			
			if (pkt.tag == SERVER) {
				if (strcmp((const char*)pkt.group_id, group_id) != 0) 
					continue;
				if (server.ip == pkt.ip && server.mac == pkt.mac) {
					 continue;
				}
				else if (server.ip != pkt.ip && server.mac == pkt.mac) {
					if (current_mode != SERVER) {
						memcpy(&server, &pkt, sizeof(bdcp_t));
						current_mode = CLIENT;
						char cmd[64];
						memset(cmd, 0, 64);
						struct in_addr s;
						s.s_addr = server.ip;
						printf("killall roomcast\n");
						system("killall roomcast");
						snprintf(cmd, 63, "roomcast 1 %s &", inet_ntoa(s));
						system(cmd);
						printf("%s\n", cmd);
					}
				}
				else if (server.ip != pkt.ip && server.mac != pkt.mac) {
//					printf("--pkt.ip&pkt.mask=%u, local_ip&local_mask=%u, current_mode=%d\n", pkt.ip&pkt.mask, local_ip&local_mask, current_mode);
					if (current_mode != SERVER &&  ((tmp1=pkt.ip&pkt.mask) == (tmp2=local_ip&local_mask))) {
//						printf("pkt.ip&pkt.mask=%d, local_ip&local_mask=%d\n", pkt.ip&pkt.mask, local_ip&local_mask);
						memcpy(&server, &pkt, sizeof(bdcp_t));
						current_mode = CLIENT;
						char cmd[64];
						memset(cmd, 0, 64);
						struct in_addr s;
						s.s_addr = server.ip;
						printf("killall roomcast\n");
						system("killall roomcast");
						snprintf(cmd, 63, "roomcast 1 %s &", inet_ntoa(s));
						system(cmd);
						printf("%s\n", cmd);
					}
				}
			}
			break;
		case BDCP_CONFIGURE_RESPONSE:
			break;
		case BDCP_CONFIGURE_REQUEST:
		case BDCP_DISCOVER_REQUEST:
		default:
			break;
		}
	}

	close(fd);
	dotini_close();
	return NULL;


}