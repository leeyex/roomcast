#pragma once
#include "BaseMessage.h"




class ControlMessage : public BaseMessage
{
public:
	ControlMessage() :BaseMessage(CONTROL)
	{

	}
	~ControlMessage()
	{

	}

};
