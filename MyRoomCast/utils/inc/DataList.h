#pragma once

#pragma once

#include <iostream>
#include <pthread.h>
#include "BaseMessage.h"
#include "ArrayPool.h"

//#define  PCM_SIZE		1152*4	  // adapt to encode  mp2			  

//#define  PCM_SIZE	3528  // 20MS 44100 16 2

#define PCM_SIZE 4608			 //one mp2 packet need pcm size

#define FLAC_PCM_SIZE 4096			 //flac packet need pcm size

#define  CACHE_MS	5200		 //5.2 sec

#define  MP2_SIZE	852			 //one mp2 size  851/852 


#pragma pack(push,1)
typedef struct {
	tv time_stamp;
	unsigned char data_buf[PCM_SIZE];
	int data_size;
}PcmChunk;


typedef struct {
	tv time_stamp;
	unsigned char data_buf[MP2_SIZE];
	int data_size;
}Mp2Chunk;

typedef struct {
	tv time_stamp;
	unsigned char data_buf[PCM_SIZE];
	int data_size;
//	double duaration;
}FlacChunk;

#pragma pack(pop)

using namespace std;

template<typename T>
class DataList
{
public:
	static DataList *instance()
	{
		static DataList *dataList = NULL;
		if (dataList == NULL) {
			dataList = new DataList;
		}
		return dataList;
	}

	int reInit()
	{
		pthread_mutex_lock(&mutex);
		data_list.clear();
		pool.reinit(count);
		pthread_mutex_unlock(&mutex);
	}

	int writeToHead(T *p)
	{

		if (p == NULL)
			return -1;
		pthread_mutex_lock(&mutex);
		T *_p = pool.malloc();
		if (_p == NULL) {
			cout << "error: pool is full\n";
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		//		cout << "p.sec=" << p->time_stamp.sec << " p.usec=" << p->time_stamp.usec << "\n";
		memcpy(_p, p, sizeof(T));
		data_list.push_front(_p);
		pthread_mutex_unlock(&mutex);
		return 0;
	}

	int write(unsigned char* buf, int buf_len, tv now)
	{
		if (buf == NULL)
			return -1;
		pthread_mutex_lock(&mutex);
		T *p = pool.malloc();
		if (p == NULL) {
			//			cout << "error: pool is full\n";
			pthread_mutex_unlock(&mutex);
			return -2;
		}
		memcpy(p->data_buf, buf, sizeof(p->data_buf));
		//		cout << "now.sec=" << now.sec << " now.usec=" << now.usec << "\n";
		p->time_stamp = now;
		p->data_size = buf_len;
		data_list.push_back(p);
//		if (data_list.size() % 20 == 0)
//			cout << "current pcm list size =" << data_list.size() << endl;
		pthread_mutex_unlock(&mutex);
		return 0;
	}
	int write(T* p)
	{
		if (p == NULL)
			return -1;
		pthread_mutex_lock(&mutex);
		T *_p = pool.malloc();
		if (_p == NULL) {
			cout << "error: pool is full\n";
			pthread_mutex_unlock(&mutex);
			return -2;
		}
		//		cout << "p.sec=" << p->time_stamp.sec << " p.usec=" << p->time_stamp.usec << "\n";
		memcpy(_p, p, sizeof(T));
		data_list.push_back(_p);
		pthread_mutex_unlock(&mutex);
		return 0;
	}

	int writeToTail(T* p)
	{
		if (p == NULL)
			return -1;
		pthread_mutex_lock(&mutex);
		T *_p = pool.malloc();
		if (_p == NULL) {
/*			cout << "writeToTail : pool is full, erase first\n"; */
			_p = data_list.front();
			data_list.pop_front();
			pool.free(_p);
			_p = pool.malloc();	
/*			cout << "writeToTail : pool is full, erase all\n";
			data_list.clear();
			pool.reinit(count);
			_p = pool.malloc();*/
		}
		//		cout << "p.sec=" << p->time_stamp.sec << " p.usec=" << p->time_stamp.usec << "\n";
		memcpy(_p, p, sizeof(T));
		data_list.push_back(_p);
		pthread_mutex_unlock(&mutex);
		return 0;
	}

	int read(unsigned char* buf)
	{
		if (buf == NULL)
			return -1;
		pthread_mutex_lock(&mutex);
		if (data_list.size() == 0) {
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		T *p = data_list.front();
		data_list.pop_front();
		memcpy(buf, p, sizeof(T));
		pool.free(p);
		pthread_mutex_unlock(&mutex);
		return 0;
	}

	int read(T* p)
	{
		if (p == NULL)
			return -1;
		pthread_mutex_lock(&mutex);
		if (data_list.size() == 0) {
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		T *_p = data_list.front();
		data_list.pop_front();
		memcpy(p, _p, sizeof(T));
		pool.free(_p);
		pthread_mutex_unlock(&mutex);
		return 0;
	}

	//check DataTime
	int checkData(int cache_ms, T* p)
	{
		if (p == NULL)
			return -1;
		pthread_mutex_lock(&mutex);
		if (data_list.size() == 0) {
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		T *_p = data_list.front();
		tv now;
		int64_t us = (now - _p->time_stamp).getUs();
		int64_t timeout = us / 1000 - cache_ms;
		if (timeout > 50 || timeout < -50) {
			if (timeout > 50) {
				cout << "drop --timeout = " << timeout << endl;
				data_list.pop_front();
				pool.free(_p);
			}
			pthread_mutex_unlock(&mutex);
			return -1;
		}
		data_list.pop_front();
		memcpy(p, _p, sizeof(T));
		pool.free(_p);
		pthread_mutex_unlock(&mutex);
		return 0;

	}

	int getSize()
	{
		pthread_mutex_lock(&mutex);
		int size = data_list.size();
		pthread_mutex_unlock(&mutex);
		return size;

	}
protected:
	DataList(int count_ = CACHE_MS / 13 * 2) : count(count_)
	{
		cout << "data list constructor\n";
		pool.init(count_);
		pthread_mutex_init(&mutex, NULL);
	}
	~DataList()
	{
		data_list.clear();
	}
	DataList(const DataList&) {} //防止拷贝构造一个实例
	DataList& operator=(const DataList&) {} //防止赋值出另一个实例

private:
	pthread_mutex_t mutex;
	ArrayPool<T> pool;
	list<T *> data_list;
	int count;
};