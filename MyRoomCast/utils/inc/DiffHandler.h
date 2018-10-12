#ifndef _DIFFER_HANDLER_H_
#define _DIFFER_HANDLER_H_

#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>

#include "DoubleBuffer.h"
#include "BaseMessage.h"

/*
	a time diffHandler, record the client diff from server
	calculate the sever current time
*/


class DiffHandler
{

public:
	DiffHandler():diff_to_server(0)
	{
		diff_buffer.setSize(20);
	}
	~DiffHandler()
	{

	}
	static DiffHandler *instance()
	{
		static DiffHandler *diff_handler = NULL;
		if (diff_handler == NULL)
		{
			diff_handler = new DiffHandler;
			return diff_handler;
		}
		return diff_handler;
	}
	void clearDiff()
	{
		diff_to_server = 0;
		diff_buffer.clear();
	}

	/**/
	void setDiff(const tv& c2s, const tv& s2c)
	{
//		printf("c2s.sec=%d, s2c.sec=%d\n", c2s.sec, s2c.sec);
		double diff = (c2s.sec/2.-s2c.sec/2.)*1000 + (c2s.usec/2.-s2c.usec/2.)/1000;
		setDiffToServer(diff);
	}
	double getDiffToServer()
	{
		return diff_to_server;
	}
	void setDiffToServer(double diff)
	{
		static int32_t lastTimeSync = 0;
		tv now;

		/// clear diffBuffer if last update is older than a minute
/*		if (!diff_buffer.empty() && (std::abs(now.sec - lastTimeSync) > 60))
		{
			std::cout << "Last time sync older than a 1 minute. Clearing time buffer\n";
			diff_to_server = diff ;
			diff_buffer.clear();
		}  */
		lastTimeSync = now.sec;
//		if (!diff_buffer.full()) {
			diff_buffer.add(diff);
			diff_to_server = diff_buffer.median(3);
//		}
//		std::cout<<"diff_to_server="<<diff_to_server<< std::endl;
	}
	
	/*unit ms*/
	int64_t serverNow()
	{
		if (diff_to_server == 0)
			return 0;
		tv current;																					   
		int64_t time;
		time = current.sec*1000 + current.usec/1000. + diff_to_server;
		return time;
	}

private:
	DoubleBuffer<double> diff_buffer;
	double diff_to_server;   //ms
};



#endif
