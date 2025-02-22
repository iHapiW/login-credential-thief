#ifndef CONNECTION_H
#define CONNECTION_H

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>

#include "Config.h"

HINTERNET InitSession();
HINTERNET Connect(HINTERNET hSession, char* hostname, int port);
//int Send(HINTERNET hConnect,char* path, const char* data);
int Upload(HINTERNET hConnect, char* data, char* uname, char* pcname);

#endif //CONNECTION_H
