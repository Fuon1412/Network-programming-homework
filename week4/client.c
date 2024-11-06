#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

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
	socklen_t addr_len = sizeof(server_addr);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Socket creation failed");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	while (1)
	{
		char username[50], password[50];
		printf("Enter username: ");
		fgets(username, sizeof(username), stdin);
		if (username[0] == '\n')
		{
			printf("Exiting...\n");
			exit(1);
		}
		printf("Enter password: ");
		scanf("%s", password);

		sprintf(buffer, "%s %s", username, password);
		sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&server_addr, addr_len);

		memset(buffer, 0, BUFFER_SIZE);
		recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
		printf("Server response: %s\n", buffer);

		if (strcmp(buffer, "OK") == 0)
		{
			while (1)
			{
				printf("Enter command (new password, homepage, or bye): ");
				scanf("%s", buffer);

				sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&server_addr, addr_len);

				memset(buffer, 0, BUFFER_SIZE);
				recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);

				printf("Server response: %s\n", buffer);
				if (strcmp(buffer, "Goodbye") == 0)
				{
					break;
				}
			}
		}
		else
		{

			printf("Invalid password. Please try again.\n");
			continue;
		}
	}

	close(sockfd);
	return 0;
}
