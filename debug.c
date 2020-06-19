#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

void debug(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	if (__DEBUG__)
	{
		vfprintf(stderr, format, args);
		fflush(stderr);
	}
	va_end(args);
}
