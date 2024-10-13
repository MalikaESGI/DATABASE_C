#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "table.h"
#include "btree.h"
#include "input.h"


InputBuffer* input_buffer;
BTree* btree;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum { PREPARE_SUCCESS, PREPARE_UNRECOGNIZED_STATEMENT } PrepareResult;

typedef enum { 
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATEMENT_CREATE_TABLE,
    STATEMENT_SHOW_TABLES
 } StatementType;

typedef struct {
    StatementType type;
} Statement;

void print_prompt() { printf("db > "); }

MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    
    // Convertir la commande en majuscules
    to_uppercase(input_buffer->buffer);

    if (strncmp(input_buffer->buffer, "INSERT", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "SELECT * FROM", 13) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
        if (strncmp(input_buffer->buffer, "CREATE TABLE", 12) == 0) {
        statement->type = STATEMENT_CREATE_TABLE;
        return PREPARE_SUCCESS;
    }
    if (strcmp(input_buffer->buffer, "SHOW TABLES") == 0) {
        statement->type = STATEMENT_SHOW_TABLES;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement) {
    switch (statement->type) {
        case (STATEMENT_CREATE_TABLE): {
            char table_name[100];
            int matched = sscanf(input_buffer->buffer, "CREATE TABLE %99s", table_name);

            if (matched != 1) {
                printf("Erreur : nom de la table non valide.\n");
                return;
            }

            // Vérifier si la table existe déjà dans le B-tree
            if (table_exists(btree, table_name)) {
                printf("Erreur : la table '%s' existe deja.\n", table_name);
                return;
            }

            // Si la table n'existe pas, crée la nouvelle table
            Table* table = create_table(table_name);
            add_field(table, "id", "int(AUTO)");
            printf("Table '%s' creee. Merci d'indiquer les champs.\n", table->table_name);

            char field_name[100];
            char field_type[100];

            // Saisie des champs
            while (true) {
                printf("%s > Nom du champ (ou 'q' pour terminer) : ", table->table_name);
                scanf("%s", field_name);

                if (strcmp(field_name, "q") == 0) {
                    break;
                }

                printf("%s > Type du champ : ", table->table_name);
                scanf("%s", field_type);

                add_field(table, field_name, field_type);
            }

            insert_btree(btree, table);
            printf("\n<---Table %s creee avec succes.--->\n", table->table_name);
            print_table(table);

            // Écriture des informations de la table dans db.txt
            FILE *fp = fopen("db.txt", "a");
            if (fp == NULL) {
                printf("Erreur d'ouverture du fichier db.txt.\n");
                return;
            }

            // Écrire le nom de la table et les champs
            fprintf(fp, "TABLE: %s\nFIELDS: ", table->table_name);
            for (int i = 0; i < table->num_fields; i++) {
                fprintf(fp, "%s", table->fields[i].field_name);
                if (i != table->num_fields - 1) {
                    fprintf(fp, ", ");
                }
            }
            fprintf(fp, "\n");
            fclose(fp);
            break;
        }

        case (STATEMENT_INSERT): {
            char table_name[100];
            char values[100];

            // Exemple : INSERT INTO users VALUES ('malika', 30)
            int matched = sscanf(input_buffer->buffer, "INSERT INTO %99s VALUES (%99[^\n])", table_name, values);

            if (matched != 2) {
                printf("Erreur de syntaxe dans la commande INSERT.\n");
                return;
            }

            // Rechercher la table dans le B-tree
            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("Erreur : la table '%s' n'existe pas.\n", table_name);
                return;
            }

            // Sauvegarder l'insertion dans db.txt
            FILE *fp = fopen("db.txt", "a");
            if (fp == NULL) {
                printf("Erreur d'ouverture du fichier db.txt.\n");
                return;
            }

            // Incrémenter automatiquement l'ID
            int id = table->num_records + 1;
            fprintf(fp, "%d, %s\n", id, values);  // Écrire l'ID auto-incrémenté et les valeurs
            fclose(fp);

            table->num_records++;  // Incrémenter le nombre d'enregistrements
            printf("Insertion réussie dans la table '%s'.\n", table_name);
            break;
        }

        case (STATEMENT_SHOW_TABLES):
            printf("Liste des tables dans la base de donnees :\n");
            show_tables(btree->root);  
            break;


        case (STATEMENT_SELECT): {
        char table_name[100];

        // Exemple : SELECT * FROM users
        int matched = sscanf(input_buffer->buffer, "SELECT * FROM %99s", table_name);

        if (matched != 1) {
            printf("Erreur de syntaxe dans la commande SELECT.\n");
            return;
        }

        // Rechercher la table dans le B-tree
        Table* table = search_btree(btree, table_name);
        if (table == NULL) {
            printf("Erreur : la table '%s' n'existe pas.\n", table_name);
            return;
        }

        // Afficher les enregistrements de la table
        print_records(table);
        break;
    }

        default:
            printf("Commande non reconnue.\n");
            break;
    }
}

void repl(void) {
    input_buffer = new_input_buffer();
    btree = create_btree(3);  // Initialise le B-tree avec un ordre de 3

    while (true) {
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
                case META_COMMAND_SUCCESS:
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    printf("Unrecognized command '%s'\n", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case PREPARE_SUCCESS:
                printf("recognized statement\n");
                break;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
                continue;
        }

        execute_statement(&statement);
        printf("Executed.\n");
    }
}
