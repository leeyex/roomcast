#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <sys/time.h>
#include <sys/types.h>
#include <cstdlib>
#include <string.h>
#include <stdint.h>

#define MAGIC 0xabcdef

enum Message_Type{
	BASE = 0,
	TIME = 1,
	CONTROL =2,
	DATA =3	
};

#pragma pack(push,1)

struct tv
{
	tv()
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		sec = t.tv_sec;
		usec = t.tv_usec;
	}
	tv(struct timeval tv) : sec(tv.tv_sec), usec(tv.tv_usec) {};
	tv(int64_t _sec, int64_t _usec) : sec(_sec), usec(_usec) {};

	int64_t sec;
	int64_t usec;

	tv operator+(const tv& other) const
	{
		tv result(*this);
		result.sec += other.sec;
		result.usec += other.usec;
		if (result.usec > 1000000)
		{
			result.sec += result.usec / 1000000;
			result.usec %= 1000000;
		}
		return result;
	}

	tv operator-(const tv& other) const
	{
		tv result(*this);
		result.sec -= other.sec;
		result.usec -= other.usec;
		while (result.usec < 0)
		{
			result.sec -= 1;
			result.usec += 1000000;
		}
		return result;
	}

	int64_t getUs()
	{
		return this->sec * 1000000 + this->usec;
	}

	int64_t getMs()
	{
		return this->sec * 1000 + this->usec/1000;
	}

	inline static void addUs(timeval& tv, int us)
	{
		if (us < 0)
		{
			timeval t;
			t.tv_sec = -us / 1000000;
			t.tv_usec = (-us % 1000000);
			timersub(&tv, &t, &tv);
			return;
		}
		tv.tv_usec += us;
		tv.tv_sec += (tv.tv_usec / 1000000);
		tv.tv_usec %= 1000000;
	}
};

typedef struct msg_head
{
	uint32_t magic;
	uint16_t type;
	uint32_t id;
	tv  sent;
	tv  received;
	uint32_t size; 
}Msg_Head; 

#pragma pack(pop)

class BaseMessage
{
public:
	Msg_Head head;
public:
	BaseMessage(uint16_t type_)
	{
		head.magic = MAGIC;
		head.type = type_;
		head.size = 0;
		head.id = 0;
	}
	BaseMessage()
	{
		head.magic = MAGIC;
		head.size = 0;
		head.id = 0;
	}
	virtual ~BaseMessage()
	{
	}
	virtual int serialize(unsigned char *buf)	   
	{
		if(buf == NULL)
			return 0;
		memcpy(buf, &head, sizeof(Msg_Head));
		return sizeof(Msg_Head);
	}
	void deSerialize(unsigned char *payload)
	{
		if(payload == NULL)
			return;
		memcpy(&head, payload, sizeof(Msg_Head));
		readExt(payload +sizeof(Msg_Head), head.size);
	}

	void deSerialize(const BaseMessage& baseMessage, unsigned char *payload)
	{
		
		memcpy(&head, &baseMessage.head, sizeof(Msg_Head));
		readExt(payload, head.size);
	}

	
private:
	virtual void readExt(unsigned char *buf, uint32_t len)
	{
			
	}
	
	virtual int getSize()
	{
		return sizeof(Msg_Head);
	}
};





#endif
