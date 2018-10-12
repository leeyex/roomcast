#ifndef FIFO_DATA_H
#define FIFO_DATA_H

#ifdef __cplusplus 
extern "C" {
#endif


int datafifo_init(char *fifo_name);
int datafifo_read(char *buf,int read_size);
int datafifo_read_head();
void datafifo_deinit();

#ifdef __cplusplus 
}
#endif

#endif
