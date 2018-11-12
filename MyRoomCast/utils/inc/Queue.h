#pragma once

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <map>

using namespace std;

template<typename T>
class Queue
{
public:
	static Queue *instance()
	{
		static Queue *queue = NULL;
		if (queue == NULL) {
			queue = new Queue;
		}
		return queue;
	}

	int reInit()
	{
		pthread_mutex_lock(&lock);
		read_map.clear();
		write_index = 0;
		pthread_mutex_unlock(&lock);
	}

	int read(int map_index, T* p)
	{
		if (map_index <= 0) {
			cout << "error map_index, must greater than 0" << endl;
			return -1;
		}
		pthread_mutex_lock(&lock);
		map<int, int>::iterator iter;
		int read_index = 0;
		iter = read_map.find(map_index);
		if (iter != read_map.end()) {
//			cout << "Find, the value is " << iter->second << endl;
			read_index = iter->second;
		}
		else {
//			cout << "Do not Find, insert it" << endl;
			read_map[map_index] =  write_index;
			pthread_mutex_unlock(&lock);
			return -1;
		}
		if (read_index == write_index) {
//			cout << "queue is empty for me, wait to write"<< endl;
			pthread_mutex_unlock(&lock);
			usleep(50000);
			return -1;
		}
		memcpy(p, &p_head[read_index], sizeof(T));
		read_index++;
		read_index %= count;
		read_map[map_index] = read_index;
		pthread_mutex_unlock(&lock);
		return 0;

	}

	int write(T *p)
	{
		pthread_mutex_lock(&lock);
		map<int, int>::iterator iter;
		static int tag = 0;
		for (iter = read_map.begin(); iter != read_map.end(); iter++)
		{
//			cout << iter->first << ' ' << iter->second << endl;
			if ((write_index + 1)%count == iter->second) {
				if (tag++ % 10 == 0) {
					cout << "queue is full, write failed" << endl;
				}
				pthread_mutex_unlock(&lock);
				usleep(50000);
				return -1;
			}
		}
		memcpy(&p_head[write_index], p, sizeof(T));
		write_index++;
		write_index %= count;
		pthread_mutex_unlock(&lock);
		return 0;
	}

	int removeVisitor(unsigned int map_index)
	{
		pthread_mutex_lock(&lock);
		map<int, int>::iterator iter;
		iter = read_map.find(map_index);
		if (iter != read_map.end()) {
			cout << "Find client socket to remove, the value is " << iter->second << endl;
			read_map.erase(iter);
		}
		else {
			cout << "Do not Find, don't need to remove" << endl;
		}
		pthread_mutex_unlock(&lock);
		return 0;

	}

protected:
	Queue(int count_ = 500) :p_head(NULL), write_index(0)
	{
		count = count_;
		p_head = new T[count];		
		if (p_head == NULL) return ;
		pthread_mutex_init(&lock, NULL);
	}
	~Queue()
	{
		pthread_mutex_lock(&lock);
		if (p_head)
			delete p_head;
		p_head = NULL;
		read_map.clear();
		pthread_mutex_unlock(&lock);

	}
	Queue(const Queue&) {} //防止拷贝构造一个实例
	Queue& operator=(const Queue&) {} //防止赋值出另一个实例

private:
	unsigned int count;
	map<int, int> read_map;				
	int write_index;
	T *p_head;
	pthread_mutex_t lock;

};

