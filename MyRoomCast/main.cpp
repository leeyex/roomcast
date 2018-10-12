#include <cstdio>
#include <signal.h>
#include <iostream>


#include "TcpClient.h"
#include "TcpServer.h"
#include "fifo_data.h"

#include "FlacEncode.h"

#define READ_PIPE "/tmp/snapfifo"

void get_sig_pipe(int sig)
{
	printf("get signal pipe %d\n", sig);
	
	return;
}

/*int main(int argc, char *argv[])
{
	unsigned char buf[4096];
	unsigned char flac_buf[4096*2];
	unsigned int flac_len = 0;

	unsigned int read_len, rt;
	/*	FlacDecode decode;
	decode.decodeInit(44100, 16, 2);
	while (1) {
		decode.Decode(flac_buf, flac_len, buf, &read_len);
	}
	return 0;		
	

	FlacEncode encode;
	encode.encoderInit(44100, 16, 2);
	FILE *fp = fopen("./test.pcm", "rb");
	FILE *fp1 = fopen("./test_sm.pcm", "rb");
	while (1) {
		rt = fread(buf, 1, 4096, fp);
		if (rt <= 0)
			return -1;
		encode.Encode(buf, rt, flac_buf, &flac_len);
		fwrite(buf, 1, rt, fp1);
	}
	return 0;	  
} 	 	*/

int main(int argc, char *argv[])
{

	signal(SIGPIPE, get_sig_pipe);

	std::cout << "hello from MyRoomCast!\n" << std::endl;
	std::cout << argv[0] << " be a server usage: 0  protocol  // protocol: mp2/flac default protocol mp2" << std::endl;
	std::cout << argv[0] << " be a client usage: 1 server_ip  " << std::endl;

	int rt, i = 0;
	char *ip = NULL;
	char *channel = NULL;
	char *protocol = NULL;
	if (argc >= 2) {
		i = atoi(argv[1]);
	}
	else {
		scanf("%d", &i);
	}
	
	if (0 == i){
		rt =  datafifo_init(READ_PIPE);
		if (-1 == rt) {
			printf("init open READ_PIPE %s pipe error\n", READ_PIPE);
			return 0;
		}   			   
		TcpServer tcp_server(NULL, 1900);
		if(argc > 2)
			protocol = argv[2];
		if (protocol) {
			tcp_server.setProtocol(protocol);
		}
		if (argc > 3)
			channel = argv[3];
		if (channel) {
			tcp_server.setChannel(channel);
		}

		rt = tcp_server.startServer(THREAD, 10);
		if (-1 == rt) {
			printf("start tcp Server error\n");
		}
/*		while (1) {
			printf("restart the tcp_server\n");
			sleep(40);
			tcp_server.stopServer();
			//sleep(5);
			tcp_server.startServer(THREAD, 10);
			system("echo \"loadfile http://mr1.doubanio.com/476322a6c49762eff43ce3fc5cd53e72/0/fm/song/p1488212_128k.mp3\" > /tmp/mpfifo");
		}	  */

	}
	else if (1 == i) {
		if (argc >= 3) {
			ip = argv[2];
		}
		else {
			std::cout << "please input the server ip\n";
		}
		TcpClient tcp_client;
		if (argc > 3)
			channel = argv[3];
		if (channel) {
			tcp_client.setChannel(channel);
		}
	//	tcp_client.startClient("192.168.1.101", 1900);
		if (ip == NULL) {
			tcp_client.startClient("192.168.1.102", 1900);
		}
		else {
			tcp_client.startClient(ip, 1900);
		}
/*		while (1) {
			sleep(40);
			tcp_client.stopClient();
//			usleep(100 * MS);
			tcp_client.startClient(ip, 1900);
		}	*/
	
	}
	else {
		return 0;
	}
	while (1) {
		sleep(100);
		
	}
    return 0; 
}  