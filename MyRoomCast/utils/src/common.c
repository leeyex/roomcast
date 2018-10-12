#include "common.h"


int initTcpSocket() 
{
	int keepalive = 1;
	int listen_fd;
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1)
	{
		fprintf(stderr, "[%s:%d]%s::socket(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
		return -1;
	}
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&keepalive, sizeof(int)) < 0)
	{
		fprintf(stderr, "[%s:%d]%s::setsockopt(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
		close(listen_fd);
		return  -1;
	}
	if (setsockopt(listen_fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&keepalive, sizeof(int)) < 0)
	{
		fprintf(stderr, "[%s:%d]%s::setsockopt(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
		close(listen_fd);
		return  -1;
	}
/*	if (setsockopt(listen_fd, IPPROTO_TCP, TCP_NODELAY, (void *)&keepalive, sizeof(int)) < 0)
	{
		fprintf(stderr, "[%s:%d]%s::setsockopt(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno));
		close(listen_fd);
		return  -1;
	} */

//	int keepidle = 3600 * 24 * 7;
	int keepidle = 10;
	int keepcnt = 10;
	int keepintvl = 10;
	setsockopt(listen_fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*)&keepidle, sizeof(keepidle));
	setsockopt(listen_fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&keepcnt, sizeof(keepcnt));
	setsockopt(listen_fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&keepintvl, sizeof(keepcnt));

	int buf_len = 0;
	static FILE *fp = NULL;
	if (fp == NULL)
	{
		fp = fopen("./tmp.ini", "r");
	}
	if (buf_len == 0) {
		char buf[8];
		memset(buf, 0, 8);
		if (fp) {
			fread(buf, 1, 8, fp);
			buf_len = atoi(buf);
			printf("buf_len == %d\n", buf_len);
			fclose(fp);
		}
		else {
			buf_len = 32 * 1024;
		}	
	}


	//	int nRecvBuf = 32 * 1024;//设置为32K
	int nRecvBuf = buf_len;
	setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	//发送缓冲区
	//	int nSendBuf = 32 * 1024;//设置为32K
	int nSendBuf = buf_len;
	setsockopt(listen_fd, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	return listen_fd;

}

int select_socket(int Sock, int IsRead, int Min, int Sec)
{
	struct timeval timeout;
	fd_set fds;
	int ret = -1;

	timeout.tv_sec = Min;
	timeout.tv_usec = Sec;

	FD_ZERO(&fds);
	FD_SET(Sock, &fds);

	do {
		if (IsRead)
			ret = select(Sock + 1, &fds, NULL, NULL, &timeout);
		else
			ret = select(Sock + 1, NULL, &fds, NULL, &timeout);

		if (ret <= 0) {
			usleep(10000);
			goto out;
		}
		else {
			if (FD_ISSET(Sock, &fds))
				return Sock;
			else
				return -1;
		}

	} while (0);

out:
	return ret;
}

int select_socket_block(int Sock, int IsRead)
{
	fd_set fds;
	int ret = -1;

	FD_ZERO(&fds);
	FD_SET(Sock, &fds);

	do {
		if (IsRead)
			ret = select(Sock + 1, &fds, NULL, NULL, NULL);
		else
			ret = select(Sock + 1, NULL, &fds, NULL, NULL);

		if (ret <= 0) {
			usleep(10000);
			perror("select");
			printf("socket = %d\n", Sock);
			goto out;
		}
		else {
			if (FD_ISSET(Sock, &fds))
				return Sock;
			else
				return -1;
		}

	} while (0);

out:
	return ret;
}

int socket_recv(int Sock, char *Buf, int Len)
{
	int Offset = 0;
	int RecvSize = 0;
	int Count = 0;
	int FreeSize = 0;

	int ret = 0;

	do {
//		ret = select_socket(Sock, 1, 0, 100);
		ret = select_socket_block(Sock, 1);
		if (ret < 0)
			break;
/*		else if (ret == 0) {
			if (Count++ >= 10) {
				printf("time out return\n");
				break;
			}
			else {
				usleep(100);
				printf("recv timeout = 2, retry !!!\n");
				continue;
			}
		}	   */

		Count = 0;
		FreeSize = Len - Offset;
		RecvSize = FreeSize > 1024 ? 1024 : FreeSize;

		do {
			ret = recv(Sock, &Buf[Offset], RecvSize, 0);
			if (ret == 0) {
				printf("offset = %d\n", Offset);
				perror("recv");
				return Offset;
			}
			if (ret > 0)
				Offset += ret;

		} while (ret == -1 && errno == EINTR);

		if (ret < 0) {
			perror("recv");
			break;
		}

	} while (Offset != Len);

	return Offset;
}


int socket_send(int Sock, char *Buf, int Len)
{
	int Offset = 0;
	int SendSize = 0;
	int FreeSize = 0;

	int ret = 0;
	int count = 0;
	do {
		ret = select_socket(Sock, 0, 0, 100);
		if (ret < 0)
			return ret;
		else if (ret == 0) {
			count++;
			usleep(100);
			if (count % 100 == 0)
				printf("i am in select send while\n");
			continue;
		}
		FreeSize = Len - Offset;
		SendSize = FreeSize > 1024 ? 1024 : FreeSize;
		do {
			ret = send(Sock, &Buf[Offset], SendSize, 0);
			if (ret == 0)
				return Offset;
			if (ret > 0)
				Offset += ret;


		} while (ret == -1 && errno == EINTR);

		if (ret < 0)
			return ret;
	} while (Offset != Len);

	return Offset;
}


int socket_send_timeout(int Sock, char *Buf, int Len)
{
	int Offset = 0;
	int SendSize = 0;
	int FreeSize = 0;

	int ret = 0;
	int count = 0;
	do {
		ret = select_socket(Sock, 0, 0, 100);
		if (ret < 0)
			return ret;
		else if (ret == 0) {
			count ++;
			usleep(100);
			if (count % 100 == 0)
				printf("i am in select send while\n");
			if (count == 1000)
				return 0;			//time out
			continue;
		}
		FreeSize = Len - Offset;
		SendSize = FreeSize > 1024 ? 1024 : FreeSize;

		do {
			ret = send(Sock, &Buf[Offset], SendSize, 0);
			if (ret == 0)
				return Offset;
			if (ret > 0)
				Offset += ret;


		} while (ret == -1 && errno == EINTR);

		if (ret < 0)
			return ret;
	} while (Offset != Len);

	return Offset;
}

int write_to_file(char *filename, unsigned char *buf, int len)
{
	if (filename == NULL || buf == NULL) {
		return -1;
	}
	static FILE *fp = NULL;
	if (fp == NULL) {
		fp = fopen(filename, "wb");
		if (fp == NULL)
			perror("fopen\n");
	}
	int rt = fwrite(buf, 1, len, fp);
	if (rt <= 0) {
		perror("fwrite\n");
//		fclose(fp);
	}
//	return	fwrite(buf, 1, len, fp);
	
}
