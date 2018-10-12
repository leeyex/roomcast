#ifndef	__DEBUG_LOG__
#define	__DEBUG_LOG__

#include <stdio.h>
#include<stdarg.h>

#define DEBUGLEVEL0 0
#define DEBUGLEVEL1 1
#define DEBUGLEVEL2 2
#define DEBUGLEVEL3 3

#define DEBUG_LEVEL 3

void __print(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

#define __DEBUG(level, fmt, arg...)	\
         if(level <= DEBUG_LEVEL) __print(fmt, ##arg)

#define DEBUG_LOG(level, format, ...) \
	do{\
		switch(level){\
			case DEBUGLEVEL3:\
			case DEBUGLEVEL2:\
				__DEBUG(level,"#File:%s#Func:%s#Line:%d#"format"",__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);\
				break;\
			case DEBUGLEVEL1:\
				__DEBUG(level,format,##__VA_ARGS__);\
				break;\
		}\
	} while(0)  

#endif
