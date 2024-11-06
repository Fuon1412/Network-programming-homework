#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>
#define _XOPEN_SOURCE 700


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

int checkIPAdress(char *IPAdress)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, IPAdress, &(sa.sin_addr)) != 0;
}

void lookupIP(char *IPAdress)
{
    struct in_addr addr;
    if (inet_pton(AF_INET, IPAdress, &addr) <= 0) {
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

int main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(stderr,"Usage: ./lookup option param\n");
        return 1;
    }

    int option = atoi(argv[1]);
    char *param = argv[2];

    if (option == 1) {
        if (!checkIPAdress(param)) {
            printf("Invalid option\n");
        } else {
            lookupIP(param);
        }
    } else if (option == 2) {
        if (checkIPAdress(param)) {
            printf("Invalid option\n");
        } else {
            lookupDomain(param);
        }
    } else {
        printf("Invalid option\n");
    }

    return 0;

    
}