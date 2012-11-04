#ifndef __DEBUGLOG_H__
#define __DEBUGLOG_H__

#include <stdarg.h>

inline void dlog_int(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);

	vprintf(fmt, arg);

	printf("\n");

	va_end(arg);
}

#if 1

#define L dlog_int
#define LL() dlog_int("%s::%s:%d", __FILE__, __FUNCTION__, __LINE__) 

#else

#define L(...) do {} while (0) 
#define LL() do {} while (0)

#endif



#endif
