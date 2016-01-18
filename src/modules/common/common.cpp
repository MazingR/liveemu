#include <common.hpp>

#include <stdio.h>
#include <windows.h>

int __cdecl vs_printf(const char *format, ...)
{
	char str[1024];

	va_list argptr;
	va_start(argptr, format);
	int ret = vsnprintf_s(str, sizeof(str), format, argptr);
	va_end(argptr);

	OutputDebugStringA(str);

	return ret;
}

char LastError[2048] = "";

void FeSetLastError(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(LastError, fmt, argptr);
	va_end(argptr);
}
const char* FeGetLastError()
{
	return LastError;
}