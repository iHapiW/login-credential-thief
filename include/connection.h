#ifndef CONNECTION_H
#define CONNECTION_H

#include <windows.h>
#include <winhttp.h>
#include CONFIG

HINTERNET InitSession();
HINTERNET Connect(HINTERNET hSession, char* hostname, int port);
HINTERNET Send(HINTERNET hConnect,char* path, char* parameter, const char* data);

#endif