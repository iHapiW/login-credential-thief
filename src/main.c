#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winhttp.h>

#include CONFIG
#include "connection.h"

int main() {

#ifdef DEBUG
  printf("Hostname: %s\n", HOSTNAME);
  printf("Port: %s\n", PORT);
  printf("PATH: %s\n", PATH);
#endif

  HINTERNET Session = InitSession();

  if(Session != NULL) {
#ifdef DEBUG
    printf("Initiated\n");
#endif
    HINTERNET Connection = Connect(Session, HOSTNAME, atoi(PORT));
    if(Connection != NULL) {
#ifdef DEBUG
      printf("Connected\n");
#endif
      HINTERNET Request = Send(Connection, PATH, PARAM, "sabzlearn.ir:ihapiw:1234\n");
      if(Request != NULL) {
#ifdef DEBUG
        printf("Sent\n");
#endif
      }
#ifdef DEBUG
      else
        printf("Could'nt Send: %ld\n", GetLastError());
#endif
    }
#ifdef DEBUG
    else
      printf("Could'nt Connect: %ld\n", GetLastError());
#endif
  }
#ifdef DEBUG
  else
    printf("Could'nt Initiate: %ld\n", GetLastError());
#endif

  return 0;
}
