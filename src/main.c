#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "repl.h"
#include "table.h"
#include "btree.h"
#include "auth.h"

int main(int argc, char* argv[], char* envp[]){
        
    Credentials creds;

    // Charger le .env
    if (!load_env(&creds)) {
        return 1; //arreter si le .env ne se charge pas
    }

    // Authentification avec 3 tentatives max
    int max_attempts = 3;
    int attempts = 0;

    while (attempts < max_attempts) {
        if (authenticate_user(&creds)) {
            break; // Auth réussie
        }

        attempts++;
        printf("Error: Invalid credentials. Please try again (%d/%d attempts).\n", attempts, max_attempts);

        if (attempts == max_attempts) {
            printf("Error: Maximum attempts reached. Exiting...\n");
            return 1; //quitter
        }
    }


    BTree* tree = create_btree(3); 
    sauvegarde(tree);
    repl(tree); 
}
