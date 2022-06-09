#ifndef CONNECTION_H
#define CONNECTION_H

#include <windows.h>
#include <winhttp.h>

HINTERNET InitSession();
HINTERNET Connect(HINTERNET hSession, char* hostname, int port);
int Send(HINTERNET hConnect,char* path, const char* data);

#endif //CONNECTION_H