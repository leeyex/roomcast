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

typedef struct _WAVEFILE_HEADER//�����ļ�ͷ�ṹ��  
{  
	char ChunkID[4];//"RIFF"  
	int64_t ChunkSize;//�����ļ���С-8bytes  
	char Format[4];//"wave"  
	char Subchunk1ID[3];//"fmt" ��ʾ������format chunck  
	int64_t Subchunk1Size;//��ʽ���ݵ��ֽ��� һ����16�������Ҳ��18  
	short AudioFormat;//���뷽ʽ  
	short NumChannels;//��������  
	int64_t SampleRate;//������  
	int64_t ByteRate;//ÿ�������ֽ���  
	short BlockAlign;//ÿ�β�����Ҫ���ֽ���  
	short BitsPerSample;//ÿ��������Ҫ��bit��  
	char Subchunk2ID[3];//'dat'�ַ�������ʶ����������  
	int64_t Subchunk2Size;//���ݵĴ�С  
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

	//��֤�ļ�ͷ  
	if (memcmp(&head.ChunkID, "RIFF", 4) != 0)
	{
		//perror("music head chunkId err");  
		return -1;
	}

	//��֤������  
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
