#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

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

void encryptPassword(char *password, char *letters, char *digits)
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

void *handleClient(void *arg)
{
    int client_fd = *((int *)arg);
    free(arg);

    char buffer[BUFFER_SIZE];
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (received <= 0) 
        {
            printf("Client disconnected.\n");
            close(client_fd);
            break;
        }

        char username[50], password[50];
        sscanf(buffer, "%s %s", username, password);

        User *user = findUser(username);
        if (user == NULL)
        {
            send(client_fd, "Account not found", strlen("Account not found"), 0);
            continue;
        }

        if (user->status == 0)
        {
            send(client_fd, "Account is blocked", strlen("Account is blocked"), 0);
            continue;
        }

        if (strcmp(user->password, password) == 0)
        {
            send(client_fd, "OK", strlen("OK"), 0);
            user->attempt = 0;

            while (1)
            {
                memset(buffer, 0, BUFFER_SIZE);
                received = recv(client_fd, buffer, BUFFER_SIZE, 0);
                if (received <= 0)
                {
                    printf("Client disconnected.\n");
                    close(client_fd);
                    break;
                }
                if (strcmp(buffer, "bye") == 0)
                {
                    char response[100];
                    snprintf(response, sizeof(response), "Goodbye %s", user->username);
                    send(client_fd, response, strlen(response), 0);
                    break;
                }
                else if (strcmp(buffer, "homepage") == 0)
                {
                    send(client_fd, user->homepage, strlen(user->homepage), 0);
                    continue;
                }
                else if (isValidPassword(buffer) == 1)
                {
                    char letters[50], digits[50];
                    strcpy(user->password, buffer);
                    saveUsersToFile();
                    encryptPassword(buffer, letters, digits);
                    char response[BUFFER_SIZE];
                    sprintf(response, "Password changed!\nLetters: %s\nDigits: %s\n", letters, digits);
                    send(client_fd, response, strlen(response), 0);
                }
                else
                {
                    send(client_fd, "Error", strlen("Error"), 0);
                }
            }
        }
        else
        {
            user->attempt++;
            send(client_fd, "Not OK", strlen("Not OK"), 0);
            if (user->attempt >= 3)
            {
                user->status = 0;
                saveUsersToFile();
            }
        }

        saveUsersToFile();
    }

    return NULL;
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

    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    listen(server_fd, 5);
    printf("Server is running on port %d...\n", port);

    while (1)
    {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        int *client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handleClient, (void *)client_fd_ptr) != 0)
        {
            perror("Failed to create thread");
            close(client_fd);
            free(client_fd_ptr);
        }

        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
