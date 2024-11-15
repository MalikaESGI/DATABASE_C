#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auth.h"

// Variable globale pour le rôle actuel admin ou user
char current_role[10]; 


int load_env(Credentials* creds) {
    FILE* file = fopen("./.env", "r");
    if (file == NULL) {
        printf("Error: Unable to open .env file.\n");
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Supprimer les espaces ou les retours à la ligne
        line[strcspn(line, "\n")] = '\0';

        // Identifier les champs et les valeurs
        if (strncmp(line, "ADMIN_USERNAME=", 15) == 0) {
            strcpy(creds->admin_username, line + 15);
        } else if (strncmp(line, "ADMIN_PASSWORD=", 15) == 0) {
            strcpy(creds->admin_password, line + 15);
        } else if (strncmp(line, "USER_USERNAME=", 14) == 0) {
            strcpy(creds->user_username, line + 14);
        } else if (strncmp(line, "USER_PASSWORD=", 14) == 0) {
            strcpy(creds->user_password, line + 14);
        }
    }

    fclose(file);
    return 1;
}

// Fonction d'authentification
int authenticate_user(Credentials* creds) {
    char username[50], password[50];

    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    if (strcmp(username, creds->admin_username) == 0 && strcmp(password, creds->admin_password) == 0) {
        strcpy(current_role, "admin");
        printf("Welcome, admin!\n");
        return 1;
    }
    if (strcmp(username, creds->user_username) == 0 && strcmp(password, creds->user_password) == 0) {
        strcpy(current_role, "user");
        printf("Welcome, user!\n");
        return 1;
    }
    return 0;
}

// Vérification des permissions
int has_permission(const char* required_role) {
    if (strcmp(current_role, required_role) == 0) {
        return 1;
    }
    return 0;
}