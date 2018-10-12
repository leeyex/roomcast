#include<stdio.h>  
#include<sys/types.h>  
#include<unistd.h>  
#include<stdlib.h>  
#include<sys/stat.h>  
#include<fcntl.h>  
#include<sys/ioctl.h>  
#include<sys/soundcard.h> 
#include <string.h>
#include <sys/types.h>
#include <linux/soundcard.h>

#include "fifo_data.h"

typedef struct _WAVEFILE_HEADER//波形文件头结构体  
{  
	char ChunkID[4];//"RIFF"  
	int64_t ChunkSize;//整个文件大小-8bytes  
	char Format[4];//"wave"  
	char Subchunk1ID[3];//"fmt" 表示以下是format chunck  
	int64_t Subchunk1Size;//格式数据的字节数 一般是16，特殊的也有18  
	short AudioFormat;//编码方式  
	short NumChannels;//声道数量  
	int64_t SampleRate;//采样率  
	int64_t ByteRate;//每秒所需字节数  
	short BlockAlign;//每次采样需要的字节数  
	short BitsPerSample;//每个采样需要的bit数  
	char Subchunk2ID[3];//'dat'字符串，标识以下是数据  
	int64_t Subchunk2Size;//数据的大小  
}WAVEFILE_HEADER;

static FILE * fifo_fd;
static unsigned long read_count = 0;

int datafifo_init(char *fifo_name)
{
	int res;
	WAVEFILE_HEADER head;
/*#if 0	
	if(access(fifo_name, F_OK) == -1)
	{
		printf ("Create the fifo pipe.\n");
		res = mkfifo(fifo_name, 0777);
		
		if(res != 0)
		{
			fprintf(stderr, "Could not create fifo %s\n", fifo_name);
		}
	}
#endif*/
	printf("---File:[%s]---Line:[%d]---\n",__FILE__,__LINE__);
	fifo_fd = fopen(fifo_name, "r");
	printf("---File:[%s]---Line:[%d]---\n",__FILE__,__LINE__);
	if (fifo_fd <= 0)return -1;
	//fifo_fd = fopen(fifo_name, O_RDONLY | O_NONBLOCK);
	res = fread(&head, sizeof(head), 1, fifo_fd);
	if (res < 0) {
		//perror("fread head err");  
		return -1;
	}

	//验证文件头  
	if (memcmp(&head.ChunkID, "RIFF", 4) != 0)
	{
		//perror("music head chunkId err");  
		return -1;
	}

	//验证比特率  
	if (head.BitsPerSample == 32)
	{
		//perror("unsuport samply size ");  
		return -1;
	}

	//music_len = head.ChunkSize+8;  
	//printf("music_len :%d\n",music_len);
	read_count = head.Subchunk2Size;
	printf("read_count = %ul\n", read_count);
	return 1;
	

}

int datafifo_read_head()
{
	int res;
	WAVEFILE_HEADER head;
	
}

int datafifo_read(char *buf,int read_size)
{
	int n_size = 0;
/*	if (read_count == 0) {
		return 0;
	}*/
	n_size = fread(buf,1,read_size,fifo_fd);
//	printf("+++File:[%s]+++Line:[%d]+++n_size:[%d]\n",__FILE__,__LINE__,n_size);
	if (n_size <= 0) 
		perror("fread");
//	read_count -= read_size;
	return n_size;
}

void datafifo_deinit()
{
	fclose(fifo_fd);
}
