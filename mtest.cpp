#include "mtest.h"
#include <stdarg.h>

mtest::mtest()
{
}


void mtest::operator ()(const char *msg, ...)
{
	char tmp[512];
	va_list argptr;
	va_start(argptr, msg);
	vsprintf(tmp, msg, argptr);
	printf("%s\n", tmp);
	va_end(argptr);
}
