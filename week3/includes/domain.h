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
#include "./ip_address.h"
#define _XOPEN_SOURCE 700

bool isValidDomainName(char *domain)
{
  if (strlen(domain) == 0)
  {
    return false;
  }
  for (int i = 0; i < strlen(domain); i++)
  {
    if (!isalnum(domain[i]) && domain[i] != '-' && domain[i] != '.')
    {
      return false;
    }
  }
  if (domain[0] == '-' || domain[strlen(domain) - 1] == '-')
  {
    return false;
  }
  for (int i = 0; i < strlen(domain) - 1; i++)
  {
    if (domain[i] == '-' && domain[i + 1] == '-')
    {
      return false;
    }
  }
  if (!strstr(domain, "."))
  {
    return false;
  }
  return true;
}

bool isDomainButNotIPAddress(char *domain) {
  if (!isValidDomainName(domain)) {
    return false;
  }
  if (isValidIPAddress(domain)) {
    return false;
  }
  return true;
}

void lookupDomain(char *domainName)
{
    struct addrinfo hints, *res, *p;
    char IPstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(domainName, NULL, &hints, &res);
    if (status != 0)
    {
        printf("No information found\n");
        return;
    }

    printf("Main IP:");
    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
        inet_ntop(p->ai_family, addr, IPstr, sizeof IPstr);
        printf("%s\n", IPstr);

        if (p->ai_next != NULL)
        {
            printf("Alternate IP: ");
        }
    }
    freeaddrinfo(res);
}