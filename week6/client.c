#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: ./client <Server IP> <PortNumber>\n");
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(1);
    }

    while (1)
    {
        char username[50], password[50];
        printf("Enter username: ");
        fgets(username, sizeof(username), stdin);
        username[strcspn(username, "\n")] = 0;

        if (username[0] == '\0')
        {
            printf("Exiting...\n");
            break;
        }

        printf("Enter password: ");
        scanf("%s", password);
        getchar();

        sprintf(buffer, "%s %s", username, password);
        send(sockfd, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(sockfd, buffer, BUFFER_SIZE, 0);
        printf("Server response: %s\n", buffer);

        if (strcmp(buffer, "OK") == 0)
        {
            while (1)
            {
                printf("Enter command (new password, homepage, or bye): ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;

                if (buffer[0] == '\0')
                {
                    printf("Exiting...\n");
                    close(sockfd);
                    return 0;
                    break;
                }

                send(sockfd, buffer, strlen(buffer), 0);
                memset(buffer, 0, BUFFER_SIZE);
                recv(sockfd, buffer, BUFFER_SIZE, 0);
                if (strncmp(buffer, "Goodbye", 7) == 0)
                {
                    printf("Server response: %s\n", buffer);
                    break;
                }
                else
                {
                    printf("Server response: %s\n", buffer);
                }
            }
        }
        else
        {
            printf("Invalid username or password. Please try again.\n");
        }
    }

    close(sockfd);
    return 0;
}
