#pragma once   

#include <list>

using namespace std;

template <typename T>
class ArrayPool
{
public:
	ArrayPool() :p_head(NULL)
	{
	}
	ArrayPool(int count):p_head(NULL)
	{
		init(count);
	}
	~ArrayPool() 
	{
		if (p_head) delete p_head;
	}
	bool init(int count)
	{
		p_head = new T[count];

		if (p_head == NULL) return false;

		T *p = p_head;

		for (unsigned int i = 0; i < count; ++i)
		{
			pool.push_back(p);
			++p;
		}

		return true;
	}

	bool reinit(unsigned int count)
	{
		if (p_head != NULL) delete[] p_head;

		p_head = new T[count];

		if (p_head == NULL) return false;

		pool.clear();

		T *p = p_head;

		for (unsigned int i = 0; i < count; ++i)
		 {
			pool.push_back(p);
			++p;
		}

		return true;
	}

	T* malloc()
	{
		if (pool.empty()) return NULL;

		T *p = pool.front();
		pool.pop_front();
		return p;
	}

	void free(T* p)
	{
		pool.push_back(p);
	}

	int size()
	{
		return pool.size();
	}

private:
	T * p_head;
	list<T*> pool;

};