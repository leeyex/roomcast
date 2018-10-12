#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <stdio.h>

#include "gpio_manager.h"

#define AIRSMART_DEV "/dev/airsmart_gpio_dev"

static int fd_neihe_status = -1;


int airsmart_gpio_dev_open(int *fd_airsmart, airsmart_gpio_function_e gpio_function)
{
	int ret = AIRSMART_FAILED;
	unsigned int gpio_mode;
	int gpio_value;
	if (fd_airsmart == NULL)
	{
		return -1;
	}

	do {
		if ((*fd_airsmart = open(AIRSMART_DEV, O_RDWR)) < 0)  //打开input设备
		{
			//printf("---------FILE=[%s]-------LINE=[%d]----open error----\n",__FILE__,__LINE__); 
			break;
		}
		ret = ioctl(*fd_airsmart, gpio_function, gpio_function);  //
		if (ret != AIRSMART_SECCUSS)  //
		{
			//printf("---------FILE=[%s]-------LINE=[%d]----ioctl error----\n",__FILE__,__LINE__); 
			break;
		}
		return AIRSMART_SECCUSS;
	} while (0);

	return ret;
}

int airsmart_gpio_read(int fd_airsmart, char *gpio_value)
{
	int ret = AIRSMART_FAILED;

	if (gpio_value == NULL)
	{
		return -1;
	}

	do {
		ret = read(fd_airsmart, gpio_value, sizeof(char));
		//if (ret == AIRSMART_FAILED)  
		{
			// perror("read error");
			// break;
		}
		return AIRSMART_SECCUSS;
	} while (0);
	*gpio_value = 0xff;
	return ret;
}

int airsmart_get_gpio_neihe_status()
{

	char test_value = 0xff;

	airsmart_gpio_read(fd_neihe_status, &test_value);

	return (int)test_value;
}

void *aux_handle(void *tag)
{
	int neihe_status_gpio_value = 0;
	airsmart_gpio_dev_open(&fd_neihe_status, AIRSA_GPIO_INPUT_AUX);//aux 高电平，非aux 低电平

	int last_value = 0;
	while (1) {
		neihe_status_gpio_value = airsmart_get_gpio_neihe_status();
//		printf("neihe_status_gpio_value = %d\n", neihe_status_gpio_value);
		if (last_value == 0 && neihe_status_gpio_value == 1) {
			//start
			*(int *)tag = 1;
		}
		else if (last_value == 1 && neihe_status_gpio_value == 0) {
			system("reboot");
		}
		
		last_value = neihe_status_gpio_value;
		sleep(1);
	
	}
	airsmart_gpio_dev_close(fd_neihe_status);
	return NULL;
}
