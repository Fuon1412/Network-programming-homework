#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#define ACCOUNT "./account.txt"
#define HISTORY "history.txt"

typedef struct User {
    char username[50];
    char password[50];
    char email[40];
    char phone[10];
    int status;
    struct User *next;
} User;

User *head = NULL;
User *currentUser = NULL;
char OTP[8] = "20225660";
bool isLoggedIn = false;

void loadUserFromFile(){
    FILE* f = fopen(ACCOUNT, "r");
    if(f == NULL){
        printf("Cannot find userfile %s\n", ACCOUNT);
        return;
    }
    char line[150];
    while(fgets(line, sizeof(line), f)){
        User *newUser = (User *)malloc(sizeof(User));
        sscanf(line, "%s %s %s %s %d", newUser->username, newUser->password, newUser->email, newUser->phone, &newUser->status);
        newUser->next = head;
        head = newUser;
        
    }
    fclose(f);
}

void saveUserToFile(){
    FILE* f = fopen(ACCOUNT, "w");  
    if(f == NULL){
        printf("Cannot open file %s to save\n", ACCOUNT);
        return;
    }
    User *current = head;
    while(current != NULL){
        fprintf(f, "%s %s %s %s %d\n", current->username, current->password, current->email, current->phone, current->status);
        current = current->next;
    }
    fclose(f);
}

void recordLoginTime(User *user) {
    FILE* f = fopen(HISTORY, "a");
    if (f == NULL) {
        printf("Cannot find history file %s\n", HISTORY);
        return;
    }
    time_t t = time(NULL);
    struct tm *localTime = localtime(&t);
    char dateStr[20];
    char timeStr[20];
    strftime(dateStr, sizeof(dateStr), "%d-%m-%Y", localTime);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localTime);
    fprintf(f, "%s | %s | %s\n", user->username, dateStr, timeStr);

    fclose(f);
}

bool isValidPhoneNumber(char phone[]) {
    if (strlen(phone) != 10) {
        return false;
    }
    for (int i = 0; i < strlen(phone); i++) {
        if (!isdigit(phone[i])) {
            return false;
        }
    }
    return true;
}


void registerNewUser(){
    User *newUser = (User *)malloc(sizeof(User));
    printf("Enter username: ");
    scanf("%s", newUser->username);
    printf("Enter password: ");
    scanf("%s", newUser->password);
    printf("Enter email: ");
    scanf("%s", newUser->email);
    do {
        printf("Enter phone number: ");
        scanf("%s", newUser->phone);
        if (!isValidPhoneNumber(newUser->phone)) {
            printf("Invalid phone number, a valid phone number must be 10 digits only\n");
        }
    } while (!isValidPhoneNumber(newUser->phone));
    newUser->status = 1;
    User *current = head;
    while(current != NULL){
        if(strcmp(current->username, newUser->username) == 0){
            printf("Username already exists\n");
            free(newUser);
            return;
        }
        current = current->next;
    }
    newUser->next = head;
    head = newUser;
    saveUserToFile();
}

void loginUser(){
    if (isLoggedIn){
        printf("Account is signed in\n");
        return;
    }
    char username[50];
    printf("Enter username: ");
    scanf("%s", username);
    User *current = head;
    while(current != NULL){
        if(strcmp(current->username, username) == 0){
            if(current->status == 1){
                int attempts = 0;
                while(attempts < 3){
                    char password[50];
                    printf("Enter password: ");
                    scanf("%s", password);
                    if(strcmp(current->password, password) == 0){
                        currentUser = current;
                        printf("Welcome\n");
                        recordLoginTime(current);
                        isLoggedIn = true;
                        return;
                    } else {
                        attempts++;
                        printf("Password is incorrect. You have %d attempts left\n", 3 - attempts);
                    }
                }
                if (attempts == 3){
                    current->status = 0;
                    saveUserToFile();
                    printf("Your account is blocked\n");
                    return;
                }
                return;
            }
            else printf("Your account is blocked\n");
        }
        current = current->next;
    }
    printf("Cannot find this username\n");
}

void changeUserPassword(){
    if (!isLoggedIn){
        printf("You have to sign in first!\n");
        loginUser();
        return;
    }
    char oldPassword[50];
    char newPassword[50];
    printf ("Enter old password: ");
    scanf("%s", oldPassword);
    User *current = head;
    while(current != NULL){
        if(strcmp(current->username, currentUser->username) == 0){
            if(strcmp(current->password, oldPassword) == 0){
                printf("Enter new password: ");
                scanf("%s", newPassword);
                strcpy(current->password, newPassword);
                saveUserToFile();
                printf("Password is changed\n");
                return;
            } else {
                printf("Old password or username is incorrect\n");
                return;
            }
        }
        current = current->next;
    }
}

void changeUserInformation(){
    if (!isLoggedIn){
        printf("You have to sign in first!\n");
        loginUser();
        return;
    }
    User *current = head;
    while(current != NULL){
        if(strcmp(current->username,currentUser->username) == 0){
            printf("Enter new email: ");
            scanf("%s", current->email);
            do {
                printf("Enter phone number: ");
                scanf("%s", current->phone);
            if (!isValidPhoneNumber(current->phone)) {
                printf("Invalid phone number, a valid phone number must be 10 digits only\n");
            }
            } while (!isValidPhoneNumber(current->phone));
            saveUserToFile();
            printf("Information is changed\n");
            return;
        }
        current = current->next;
    }
}

void resetPassword(){
    char username[50];
    printf("Enter username: ");
    scanf("%s", username);
    User *current = head;
    while(current != NULL){
        if(strcmp(current->username, username) == 0){
            printf("Enter OTP: ");
            char otp[8];
            scanf("%s", otp);
            if(strcmp(otp, OTP) == 0){
                printf("Enter new password: ");
                scanf("%s", current->password);
                saveUserToFile();
                printf("Password is reset\n");
                return;
            } else {
                printf("OTP is incorrect\n");
                return;
            }
        }
        current = current->next;
    }
    printf("Can not find this username\n");
}

void checkLoginHistory() {
    if (!isLoggedIn) {
        printf("You have to sign in first!\n");
        loginUser();
        return;
    }
    FILE *f = fopen(HISTORY, "r");
    if (f == NULL) {
        printf("Cannot find history file %s\n", HISTORY);
        return;
    }

    char line[200];
    bool hasHistory = false;
    while (fgets(line, sizeof(line), f) != NULL) {
        char tempUsername[50];
        char date[20], time[20];
        sscanf(line, "%s | %s | %s", tempUsername, date, time);
        if (strcmp(tempUsername,currentUser->username) == 0) {
            printf("%s|%s|%s\n", tempUsername, date, time);
            hasHistory = true;
        }
    }

    if (!hasHistory) {
        printf("No login history found for username: %s\n", currentUser->username);
    }

    fclose(f);
}

void signOut(){
    if (!isLoggedIn){
        printf("You have to sign in first!\n");
        loginUser();
        return;
    }
    isLoggedIn = false;
    printf("You have signed out\n");
}

int main(){
    loadUserFromFile();
    int choice;
    do{
        printf("USER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Change password\n");
        printf("4. Change information\n");
        printf("5. Reset password\n");
        printf("6. View login history\n");
        printf("7. Sign out\n");
        printf("Your choice (1-7, other to quit):\n");
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            registerNewUser();
            break;
        case 2:
            loginUser();
            break;
        case 3:
            changeUserPassword();
            break;
        case 4:
            changeUserInformation();
            break;
        case 5:
            resetPassword();
            break;
        case 6:
            checkLoginHistory();
            break;
        case 7:
            signOut();
            break;
        default:
            printf("Goodbye\n");
            break;
        }
    } while(choice >= 1 && choice < 8);
    return 0;
}


