#include <cstdio>
#include <pthread.h>
#include "net_common.h"
#include "bdcp.h"
#include "gpio_manager.h"


//airsmart_gpio_dev_open(&fd_neihe_status, AIRSA_GPIO_INPUT_AUX);//aux 高电平，非aux 低电平


int main(int argc, char *argv[])
{
    printf("hello from Zubo!\n");
	int tag = 0;
	pthread_t thd, thd1, thd2;
	pthread_create(&thd, NULL, aux_handle, &tag);

	while (tag == 0) {
		sleep(1);
	}
	pthread_create(&thd2, NULL, task_bdcpd, NULL);
	pthread_create(&thd1, NULL, handle_bdcp_client, NULL);
	int send_fd;
	int count = 0;
	unsigned int current_mode;
	send_fd = init_udpsend_socket(1024 * 32);
	bdcp_t pkt;
	unsigned int nip = 0;
	while ((nip = get_bdcp_ip()) == 0) {
		printf("nip == 0\n");
		sleep(1);
	}

	while (1) {
		cre_bdcp_discover_pkt(&pkt);
		sendto_bdcp_server(send_fd, &pkt);
	//	sleep(2);
		usleep(20 * 1000);
		current_mode = query_current_mode();
		if (current_mode == UN_SET) {
			count++;
		}
		else {
			sleep(2);
		}
		if (count == 300) {
			current_mode = SERVER;
			set_current_mode(SERVER);
			printf("set myself be a server\n");
			system("mkfifo /tmp/mpfifo");
			system("mkfifo /tmp/mpfifor");
			system("mkfifo /tmp/snapfifo");
			system("killall airsmart-mplayer");
			system("airsmart-mplayer -novideo -slave -idle -quiet -cache 1024 -srate 44100 -ao pcm:file=/tmp/snapfifo -input file=/tmp/mpfifo & ");
			system("echo \"loadfile http://music.163.com/song/media/outer/url?id=513791211.mp3\" > /tmp/mpfifo");
			sleep(2);
			system("killall roomcast");
			if (argc > 1)
			{
				if (strcmp(argv[1], "mp2") == 0) {
					system("roomcast 0 mp2 &");
				}
				else if (strcmp(argv[1], "flac") == 0) {
					system("roomcast 0 flac &");
				}
				else {
					system("roomcast 0 mp2 &");
				}
			}
			else {
				system("roomcast 0 mp2 &");
			}
			while (1) {
				sleep(100);
			}	
		}
	}
    return 0;
}