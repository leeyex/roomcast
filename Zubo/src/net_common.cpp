#include "net_common.h"
#include "bdcp.h"

int get_ip(char * ifname,char * ip)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *psin = NULL;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	ifr.ifr_addr.sa_family = AF_INET;
	
	if (ioctl(fd, SIOCGIFADDR, &ifr) == -1)
	{
		close(fd);
		return -1;
	}
	
	close(fd);
	psin = (struct sockaddr_in *)(&(ifr.ifr_addr));
	char * tmpip =NULL;
	tmpip  = inet_ntoa(psin->sin_addr);
	if(tmpip != NULL){
		memcpy(ip,tmpip,strlen(tmpip));
		memset(ip+strlen(tmpip),0,1);
		return 0;
	}
	else
		return -1;
	
}

int get_default_gateway()
{
	static char buf[4096];
	static char			global_default_route_interface[32];
	char *p = buf;
	char *q = buf;
	char *s = buf;
	static char ifname[256];
	static char dstnet[32];
	static char dstgw[32];
	int tab = 0;

	memset( buf, 0 ,sizeof(buf) );
	memset( ifname, 0 ,sizeof(ifname) );
	memset( dstnet, 0, sizeof(dstnet) );
	memset( dstgw, 0,sizeof(dstgw) );

	FILE* f = fopen("/proc/net/route","rb");
	if (f == NULL)
	{
		perror("/proc/net/route");
		return -1;
	}
	fread( buf, sizeof(buf), 1, f );
	fclose( f );

	while ( p < buf + sizeof(buf) )
	{
		if ( *p == '\n' )
		{
			if ( strncmp( q, "Iface", 5 ) != 0 )
			{
				//find #2
				s = q;
				tab = 0;
				while ( q < p )
				{
					if ( *q=='\t' )
					{
						tab++;
						if ( tab == 1 )
						{
							strncpy( ifname, s, q - s );
							s = q + 1;
						}
						if ( tab == 2 )
						{
							strncpy( dstnet, s, q - s );
							s = q + 1;
						}
						if ( tab == 3 )
						{
							strncpy( dstgw, s, q - s );
							break;
						}
					}
					q++;
				}
				if ( strncmp( dstnet, "00000000", 8)==0 )
				{
					strcpy( global_default_route_interface, ifname );
					//printf("route:%s\n",global_default_route_interface);
					//global_default_route_gw = strtol( dstgw, 0, 16 );
					unsigned int ngw = strtol( dstgw, 0, 16 );
					return ngw;
				}
			}
			q = p+1;
		}
		p++;
	}
	return 0;
}
int get_local_mac_string(char * ifname, char *buf)
{
	int fd;
	struct ifreq ifr;
	
	if (buf == NULL)
		return -1;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
	{
		close(fd);
		return -1;
	}
	
	close(fd);
	char * tmp = ifr.ifr_hwaddr.sa_data;
	if(tmp != NULL){
		for(int i = 0;i<MAC_LEN;i++)
		{
			sprintf(buf+strlen(buf),"%x",(unsigned char)*(tmp+i));
			if(i!=MAC_LEN-1)
				sprintf(buf+strlen(buf),":");
		}
		printf("%s\n",buf);
		return 0;
	}
	else
		return -1;
}
int get_local_mac_hex(const char * ifname, char *buf)
{
	int fd;
	struct ifreq ifr;
	
	if (buf == NULL)
		return -1;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	memset(buf, 0, MAC_LEN);
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
	{
		close(fd);
		return -1;
	}
	
	close(fd);
	memcpy(buf, ifr.ifr_hwaddr.sa_data, 6);	
	return 0;
}
int get_local_mask_string(char * ifname, char * mask)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *psin = NULL;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return 0;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	ifr.ifr_addr.sa_family = AF_INET;
	
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) == -1)
	{
		close(fd);
		return 0;
	}
	
	close(fd);
	psin = (struct sockaddr_in *)(&(ifr.ifr_addr));
	char * tmpip =NULL;
	tmpip  = inet_ntoa(psin->sin_addr);
	if(tmpip != NULL){
		memcpy(mask,tmpip,strlen(tmpip));
		memset(mask+strlen(tmpip),0,1);
		return 0;
	}
	else
		return -1;
}
unsigned int get_local_mask_hex(const char * ifname)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *psin = NULL;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return 0;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	ifr.ifr_addr.sa_family = AF_INET;
	
	if (ioctl(fd, SIOCGIFNETMASK, &ifr) == -1)
	{
		close(fd);
		return 0;
	}
	
	close(fd);
	psin = (struct sockaddr_in *)(&(ifr.ifr_addr));
	return psin->sin_addr.s_addr;
}


unsigned int get_default_route()
{
	static char buf[4096];
	char *p = buf;
	char *q = buf;
	char *s = buf;
	static char ifname[256];
	static char dstnet[32];
	static char dstgw[32];
	int tab = 0;

	memset( buf, 0 ,sizeof(buf) );
	memset( ifname, 0 ,sizeof(ifname) );
	memset( dstnet, 0, sizeof(dstnet) );
	memset( dstgw, 0,sizeof(dstgw) );

	FILE* f = fopen("/proc/net/route","rb");
	if (f == NULL)
	{
		perror("/proc/net/route");
		return -1;
	}
	fread( buf, sizeof(buf), 1, f );
	fclose( f );

	while ( p < buf + sizeof(buf) )
	{
		if ( *p == '\n' )
		{
			if ( strncmp( q, "Iface", 5 ) != 0 )
			{
				//find #2
				s = q;
				tab = 0;
				while ( q < p )
				{
					if ( *q=='\t' )
					{
						tab++;
						if ( tab == 1 )
						{
							strncpy( ifname, s, q - s );
							s = q + 1;
						}
						if ( tab == 2 )
						{
							strncpy( dstnet, s, q - s );
							s = q + 1;
						}
						if ( tab == 3 )
						{
							strncpy( dstgw, s, q - s );
							break;
						}
					}
					q++;
				}
				if ( strncmp( dstnet, "00000000", 8)==0 )
				{
					//strcpy( global_default_route_interface, ifname );
					unsigned int ngw = strtol( dstgw, 0, 16 );
					return ngw;
				}
			}
			q = p+1;
		}
		p++;
	}
	return 0;
}

int set_local_ip(const char * ifname, unsigned int ip)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *psin = NULL;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	ifr.ifr_addr.sa_family = AF_INET;
	psin = (struct sockaddr_in *)(&(ifr.ifr_addr));
	psin->sin_addr.s_addr = htonl(ip);
	
	if (ioctl(fd, SIOCSIFADDR, &ifr) == -1)
	{
		close(fd);
		return -1;
	}
	
	close(fd);	
	return 0;
}
int set_local_mask(const char * ifname, unsigned int mask)
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *psin = NULL;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	ifr.ifr_addr.sa_family = AF_INET;
	psin = (struct sockaddr_in *)(&(ifr.ifr_addr));
	psin->sin_addr.s_addr = htonl(mask);
	
	if (ioctl(fd, SIOCSIFNETMASK, &ifr) == -1)
	{
		close(fd);
		return -1;
	}
	
	close(fd);	
	return 0;
}
int set_default_gateway(const char * ifname, unsigned int gw)
{
	int fd;
	struct rtentry rte;
	struct sockaddr_in *psin = NULL;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	/* See if adding a route fails */
	memset(&rte, 0, sizeof(rte));
	rte.rt_flags = RTF_UP | RTF_GATEWAY;
	rte.rt_dst.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_dst;
	psin->sin_addr.s_addr = htonl(0x00000000);
	rte.rt_genmask.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_genmask;
	psin->sin_addr.s_addr = htonl(0x00000000);
	rte.rt_gateway.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_gateway;
	psin->sin_addr.s_addr = htonl(gw);
	rte.rt_dev = (char *)ifname;
	
	if (ioctl(fd, SIOCADDRT, &rte) == -1)
	{
		close(fd);
		return -2;
	}
	
	/* Delete all routes */
	memset(&rte, 0, sizeof(rte));
	rte.rt_flags = RTF_UP | RTF_GATEWAY;
	rte.rt_dst.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_dst;
	psin->sin_addr.s_addr = htonl(0x00000000);
	rte.rt_genmask.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_genmask;
	psin->sin_addr.s_addr = htonl(0x00000000);
	
	while (ioctl(fd, SIOCDELRT, &rte) == 0)
		continue;
	
	/* Add the route */
	memset(&rte, 0, sizeof(rte));
	rte.rt_flags = RTF_UP | RTF_GATEWAY;
	rte.rt_dst.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_dst;
	psin->sin_addr.s_addr = htonl(0x00000000);
	rte.rt_genmask.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_genmask;
	psin->sin_addr.s_addr = htonl(0x00000000);
	rte.rt_gateway.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_gateway;
	psin->sin_addr.s_addr = htonl(gw);
	rte.rt_dev = (char *)ifname;
	
	if (ioctl(fd, SIOCADDRT, &rte) == -1)
	{
		close(fd);
		return -2;
	}
	
	close(fd);
	return 0;
	
}

int set_interface_up(const char * ifname)
{
	int fd;
	struct ifreq ifr;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, (sizeof(ifr.ifr_name) - 1));
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) == -1)
	{
		close(fd);
		return -1;
	}
	
	ifr.ifr_flags |= IFF_UP;
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) == -1)
	{
		close(fd);
		return -1;
	}
	
	close(fd);	
	return 0;
}

int set_broadcast_route(const char * ifname)
{
	int fd;
	struct rtentry rte;
	struct sockaddr_in *psin = NULL;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		return -1;
	
	memset(&rte, 0, sizeof(rte));
	rte.rt_flags = RTF_UP;
	rte.rt_dst.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_dst;
	psin->sin_addr.s_addr = htonl(0xFFFFFFFF);
	rte.rt_genmask.sa_family = AF_INET;
	psin = (struct sockaddr_in *)&rte.rt_genmask;
	psin->sin_addr.s_addr = htonl(0xFFFFFFFF);
	rte.rt_dev = (char *)ifname;
	
	if (ioctl(fd, SIOCADDRT, &rte) == -1)
	{
		close(fd);
		return -1;
	}
	
	close(fd);
	return 0;
}

int init_udpsend_socket(int size)
{
	int nSendBuf = size;
	unsigned char	ttl = 64;
	unsigned int	loopback = 0;

	int sendfd = socket(AF_INET, SOCK_DGRAM, 0);
//	setsockopt(sendfd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	setsockopt(sendfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	setsockopt(sendfd, IPPROTO_IP, IP_MULTICAST_LOOP, (char*)&loopback, sizeof(loopback));
	return sendfd;
}


int init_dd_udprecv_socket(const int size, const int port,  char *mip)
{
	struct sockaddr_in localaddr;
	int bufsize = size;

	int recvfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (recvfd == -1)
	{
		perror(__FUNCTION__);
		return -1;
	}
	localaddr.sin_family = AF_INET;
	unsigned int nip = 0;
	while((nip = get_bdcp_ip()) == 0) {
		printf("nip == 0\n");
		sleep(1);
	}
	localaddr.sin_addr.s_addr = nip;
//	localaddr.sin_addr.s_addr = inet_addr(local_ip);   /* Internet/IP */


	struct in_addr ina;
	ina.s_addr = localaddr.sin_addr.s_addr;
	if (setsockopt(recvfd, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&ina, sizeof(ina)) == -1) {
		fprintf(stderr, "[%s:%d]%s::setsockopt(..., IPPPROTO_IP, IP_MULTICAST_IF,...): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
	}
	printf("port = %d, mip=%s, local_ip=%s \n", port, mip, inet_ntoa(localaddr.sin_addr));
	localaddr.sin_port = htons(port);       /* server port */
	localaddr.sin_addr.s_addr = inet_addr(mip);
	//	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);

//	setsockopt(recvfd, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));
	int flag = 1;
	setsockopt(recvfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(int));

	flag = 1;
	if (setsockopt(recvfd, SOL_SOCKET, IP_MULTICAST_LOOP, &flag, sizeof(flag)) == -1)
	{
		printf("aaa\n");
	}

	if (bind(recvfd, (struct sockaddr *)&localaddr, sizeof(localaddr)) < 0)
	{
		printf("[%s %s:%d] bind error %s \n", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
		close(recvfd);
		return -1;
	}
	struct ip_mreq imr;
	imr.imr_multiaddr.s_addr = inet_addr(mip);	
	imr.imr_interface.s_addr = nip;
	if (setsockopt(recvfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&imr, sizeof(imr)) == -1)
	{
		fprintf(stderr, "[%s:%d]%s::setsockopt(..., IPPPROTO_IP, IP_ADDMEMBERSHIP,...): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
		close(recvfd);
	}
	printf("recvfd = %d\n", recvfd);
	return recvfd;
}
