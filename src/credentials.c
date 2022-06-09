#include "credentials.h"
#include "json/json.h"

void deallocCreds(credList* Credentials)
{
        for(register int i = 0; i < Credentials->size; i++)
        {
            free(Credentials->creds[i].url);
            free(Credentials->creds[i].username);
            free(Credentials->creds[i].password);
        }

        free(Credentials->creds);
        free(Credentials);
}


void printCreds(credList* Credentials)
{
        for(register int i = 0; i < Credentials->size; i++)
        {
            printf("URL: %s\n", Credentials->creds[i].url);
            printf("Username: %s\n", Credentials->creds[i].username);
            printf("Password: %s\n", Credentials->creds[i].password);
            printf("----------\n");
        }
}

char* credentialToJson(Credential* cred)
{
    JsonNode* root = json_mkobject();
    JsonNode* url = json_mkstring(cred->url);
    JsonNode* username = json_mkstring(cred->username);
    JsonNode* password = json_mkstring(cred->password);

    json_append_member(root, "url", url);
    json_append_member(root, "username", username);
    json_append_member(root, "password", password);
    char* json = json_encode(root);

    json_delete(root);

    return json;
}
