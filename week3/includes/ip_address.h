#ifndef IP_ADDRESS_H
#define IP_ADDRESS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#define _XOPEN_SOURCE 700

bool isValidIPAddress(char *IPAddress)
{
  if (strlen(IPAddress) == 0)
  {
    return false;
  }
  if (!strstr(IPAddress, "."))
  {
    return false;
  }
  char *octets[4];
  int num_octets = 0;
  char *token = strtok(IPAddress, ".");
  while (token != NULL)
  {
    octets[num_octets++] = token;
    token = strtok(NULL, ".");
  }
 if (num_octets != 4)
  {
    return false;
  }
  for (int i = 0; i < num_octets; i++)
  {
    for (int j = 0; j < strlen(octets[i]); j++)
    {
      if (!isdigit(octets[i][j]))
      {
        return false;
      }
    }
  }
 
  return true;
}

void lookupIP(char *IPAddress)
{
    struct in_addr addr;
    if (inet_pton(AF_INET, IPAddress, &addr) <= 0) {
        printf("Invalid IP address format.\n");
        return;
    }

    struct hostent *host = gethostbyaddr(&addr, sizeof(addr), AF_INET);
    if (host == NULL) {
        printf("No information found");
        return;
    }

    printf("Main name: %s\n", host->h_name);
    
    if (host->h_aliases != NULL) {
        printf("Alternative names: ");
        for (char **alias = host->h_aliases; *alias != NULL; alias++) {
            printf("%s ", *alias);
        }
        printf("\n");
    }
}
#endif