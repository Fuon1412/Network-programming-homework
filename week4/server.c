#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_SIZE 1024
#define FILENAME "nguoidung.txt"

typedef struct User
{
	char username[50];
	char password[50];
	char homepage[100];
	int status;
	int attempt;
	struct User *next;
} User;

User *head = NULL;

void loadUsersFromFile()
{
	FILE *file = fopen(FILENAME, "r");
	if (file == NULL)
	{
		perror("Khong the mo file");
		exit(1);
	}
	char line[200];
	while (fgets(line, sizeof(line), file))
	{
		User *newUser = (User *)malloc(sizeof(User));
		sscanf(line, "%s %s %s %d", newUser->username, newUser->password, newUser->homepage, &newUser->status);
		newUser->attempt = 0;
		newUser->next = head;
		head = newUser;
	}
	fclose(file);
}

void saveUsersToFile()
{
	FILE *file = fopen(FILENAME, "w");
	if (file == NULL)
	{
		perror("Khong the mo file");
		exit(1);
	}
	User *current = head;
	while (current != NULL)
	{
		fprintf(file, "%s %s %s %d\n", current->username, current->password, current->homepage, current->status);
		current = current->next;
	}
	fclose(file);
}

User *findUser(char *username)
{
	User *current = head;
	while (current != NULL)
	{
		if (strcmp(current->username, username) == 0)
		{
			return current;
		}
		current = current->next;
	}
	return NULL;
}

int isValidPassword(char *password)
{
	for (int i = 0; i < strlen(password); i++)
	{
		if (!isalnum(password[i]))
		{
			return 0;
		}
	}
	return 1;
}

void encodePassword(char *password, char *letters, char *digits)
{
	int lettersIndex = 0, digitsIndex = 0;
	for (int i = 0; i < strlen(password); i++)
	{
		if (isalpha(password[i]))
		{
			letters[lettersIndex++] = password[i];
		}
		else if (isdigit(password[i]))
		{
			digits[digitsIndex++] = password[i];
		}
	}
	letters[lettersIndex] = '\0';
	digits[digitsIndex] = '\0';
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage: ./server <PortNumber>\n");
		return 1;
	}

	int port = atoi(argv[1]);
	loadUsersFromFile();

	int sockfd;
	struct sockaddr_in server_addr, client_addr;
	char buffer[BUFFER_SIZE];
	socklen_t addr_len = sizeof(client_addr);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Socket creation failed");
		exit(1);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Bind failed");
		close(sockfd);
		exit(1);
	}

	printf("Server is running on port %d...\n", port);

	while (1)
	{
		memset(buffer, 0, BUFFER_SIZE);
		recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

		char username[50], password[50];
		sscanf(buffer, "%s %s", username, password);

		User *user = findUser(username);
		if (user == NULL)
		{
			sendto(sockfd, "Account not found", strlen("Account not found"), 0, (struct sockaddr *)&client_addr, addr_len);
			continue;
		}

		if (user->status == 0)
		{
			sendto(sockfd, "Account is blocked", strlen("Account is blocked"), 0, (struct sockaddr *)&client_addr, addr_len);
			continue;
		}

		if (strcmp(user->password, password) == 0)
		{
			sendto(sockfd, "OK", strlen("OK"), 0, (struct sockaddr *)&client_addr, addr_len);
			user->attempt = 0;

			while (1)
			{
				memset(buffer, 0, BUFFER_SIZE);
				recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

				if (strcmp(buffer, "bye") == 0)
				{
					sendto(sockfd, "Goodbye", strlen("Goodbye"), 0, (struct sockaddr *)&client_addr, addr_len);
					break;
				}

				if (strcmp(buffer, "homepage") == 0)
				{
					sendto(sockfd, user->homepage, strlen(user->homepage), 0, (struct sockaddr *)&client_addr, addr_len);
					continue;
				}

				if (isValidPassword(buffer))
				{
					char letters[50], digits[50];
					encodePassword(buffer, letters, digits);
					char response[BUFFER_SIZE];
					sprintf(response, "Letters: %s, Digits: %s", letters, digits);
					sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
					strcpy(user->password, buffer);
				}
				else
				{
					sendto(sockfd, "Error", strlen("Error"), 0, (struct sockaddr *)&client_addr, addr_len);
				}
			}
		}
		else
		{
			user->attempt++;
			sendto(sockfd, "Not OK", strlen("Not OK"), 0, (struct sockaddr *)&client_addr, addr_len);
			if (user->attempt >= 3)
			{
				user->status = 0;
			}
		}

		saveUsersToFile();
	}

	close(sockfd);
	return 0;
}
