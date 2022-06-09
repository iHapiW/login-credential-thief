#ifndef CREDENTIALS_H
#define CREDENTIALS_H

#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char* url;
    char* username;
    char* password;
} Credential;

typedef struct {
    Credential* creds;
    size_t size;
} credList;

void deallocCreds(credList* Credentials);
void printCreds(credList* Credentials);
char* credentialToJson(Credential* cred);

#endif //BROSER_H