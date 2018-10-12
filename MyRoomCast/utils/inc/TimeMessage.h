#pragma once
#include "BaseMessage.h"


class TimeMessage : public BaseMessage
{
public:
	TimeMessage() :BaseMessage(TIME)
	{

	}
	TimeMessage(BaseMessage &baseMsg) :BaseMessage(baseMsg)
	{

	}
	~TimeMessage()
	{
	}
	int serialize(unsigned char* buf)
	{
		if (buf == NULL)
			return 0;
		this->head.size = getSize();
		memcpy(buf, &this->head, sizeof(Msg_Head));
		memcpy(buf + sizeof(Msg_Head), &latency, this->head.size);
		return this->head.size + sizeof(Msg_Head);
	}
	void readExt(unsigned char *buf, uint32_t len)
	{
		memcpy(&latency, buf, len);
	}
	int getSize()
	{
		return sizeof(tv);
	}
	tv latency;						// server set  latency= packet.recevied - packet.sent
	
	
};



