#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

typedef struct {
    char admin_username[50];
    char admin_password[50];
    char user_username[50];
    char user_password[50];
} Credentials;

int has_permission(const char* required_role);
int authenticate_user(Credentials* creds);
int load_env(Credentials* creds);

#endif
