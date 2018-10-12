#pragma once

#include "BaseMessage.h"
#include "DataList.h"

#pragma pack(push,1)

typedef struct data_info
{
	int type;
	tv time_stamp;
	int data_size;
}Data_Info;

#pragma pack(pop)

typedef enum {
	PCM_DATA = 100,
	MP2_DATA,
	FLAC_DATA,
};


class DataMessage : public BaseMessage
{
public:
	PcmChunk pcm_chunk;
	Mp2Chunk mp2_chunk;
	FlacChunk flac_chunk;
	Data_Info data_info;
	char *payload;				// type + tv + data_size + payload

	DataMessage() :BaseMessage(DATA),payload(NULL)
	{

	}
	DataMessage(BaseMessage &baseMsg) :BaseMessage(baseMsg), payload(NULL)
	{

	}

	~DataMessage()
	{

	}

	int serializePcm(unsigned char* buf)
	{
		if (buf == NULL)
			return 0;
		this->head.size = sizeof(PcmChunk);
		memcpy(buf, &this->head, sizeof(Msg_Head));
		memcpy(buf + sizeof(Msg_Head), &this->pcm_chunk,  sizeof(PcmChunk));
		return this->head.size + sizeof(Msg_Head);
	}
	
	int serializeMp2(unsigned char* buf)
	{
		if (buf == NULL)
			return 0;
		this->head.size = sizeof(Mp2Chunk);
		memcpy(buf, &this->head, sizeof(Msg_Head));
		memcpy(buf + sizeof(Msg_Head), &mp2_chunk, sizeof(Mp2Chunk));
		return this->head.size + sizeof(Msg_Head);
	}
	int serialize(unsigned char *buf)
	{
		if (buf == NULL)
			return 0;
		this->head.size = sizeof(Data_Info) + data_info.data_size;
		memcpy(buf, &this->head, sizeof(Msg_Head));
		memcpy(buf + sizeof(Msg_Head), &data_info, sizeof(Data_Info));
		memcpy(buf + sizeof(Msg_Head) + sizeof(Data_Info), payload, data_info.data_size);
		return this->head.size + sizeof(Msg_Head);
	}

private:
	void readExt(unsigned char *buf, uint32_t buf_len)
	{
/*		data_size = len > 1024 ? 1024 : len;
		if(len > 1024){
			printf("too large data , droped %d\n", len - 1024);
		}	
		*/
		
		
			memcpy(&data_info, buf, sizeof(Data_Info));
			payload = (char *)buf + sizeof(Data_Info);
	
	}


};