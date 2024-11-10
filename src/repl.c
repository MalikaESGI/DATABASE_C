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
    STATEMENT_SHOW_TABLES,
    STATEMENT_SELECT_WHERE,
    STATEMENT_DELETE_BY_ID,
    STATEMENT_DELETE_ALL,
    STATEMENT_DROP_TABLE   
    
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
        // Vérifier si la commande contient "WHERE" après "SELECT * FROM"
        char* where_clause = strstr(input_buffer->buffer, "WHERE");
        
        if (where_clause != NULL) {
            statement->type = STATEMENT_SELECT_WHERE;
        } else {
            statement->type = STATEMENT_SELECT;
        }
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "CREATE TABLE", 12) == 0) {
        statement->type = STATEMENT_CREATE_TABLE;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "SHOW TABLES", 11) == 0) {
        statement->type = STATEMENT_SHOW_TABLES;
        return PREPARE_SUCCESS;
    }

    if (strncmp(input_buffer->buffer, "DELETE FROM", 11) == 0) 
    {
        if (strstr(input_buffer->buffer, "WHERE")) {
            statement->type = STATEMENT_DELETE_BY_ID;
        } else {
            statement->type = STATEMENT_DELETE_ALL;
        }
        return PREPARE_SUCCESS;
    }

    if (strncmp(input_buffer->buffer, "DROP TABLE", 10) == 0) {
        statement->type = STATEMENT_DROP_TABLE;
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
            
            create_backup_file(table_name, table);
            break;
        }

        case (STATEMENT_INSERT): {
            char table_name[100];
            char values[100];

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

            // Le nombre de champs dans la table
            int expected_values_count = table->num_fields;

            // tableau pour styocker les valeurs après la découpe
            char* values_array[expected_values_count];
            int index = 0;

            //strtok pour découper les valeurs
            char* token = strtok(values, ",");
            while (token != NULL) {
                while (*token == ' ') token++;  // Retirer les espaces autour de chaque valeur
                values_array[index] = strdup(token);
                index++;
                token = strtok(NULL, ",");
            }
            printf("%d, %d\n", index, expected_values_count); 

            // Vérification du nombre de valeurs saisies
            if (index != expected_values_count) {
                printf("Erreur : le nombre de valeurs ne correspond pas aux champs de la table.\n");
                
                // Libérer la mémoire allouée
                for (int i = 0; i < index; i++) {
                    free(values_array[i]);
                }
                
                return;
            }

            if (insert_record(table, values_array, expected_values_count) == 0) {
                printf("Insertion réussie dans la table '%s'.\n", table_name);
                save_record_to_file(table, values_array, expected_values_count);
            }
            for (int i = 0; i < expected_values_count; i++) {
                free(values_array[i]);
            }

            break;
        }

        case (STATEMENT_SHOW_TABLES):{
            printf("Liste des tables dans la base de donnees :\n");
            show_tables(btree->root);  
            break;
        }

        case (STATEMENT_SELECT): {
        char table_name[100];

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
        select_from_table(table);
        break;
    }

    case (STATEMENT_SELECT_WHERE): {
    
            char table_name[100], field_name[100], value[100];
            int matched = sscanf(input_buffer->buffer, "SELECT * FROM %99s WHERE %99s = %99s", table_name, field_name, value);

            if (matched != 3) {
                printf("Erreur de syntaxe dans la commande SELECT avec WHERE.\n");
                return;
            }

            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("Erreur : la table '%s' n'existe pas.\n", table_name);
                return;
            }

            select_from_table_where(table, field_name, value);
            break;
       }

        // supprimer tous les enregistrements
            case (STATEMENT_DELETE_ALL): {
            char table_name[100];
            
            int matched = sscanf(input_buffer->buffer, "DELETE FROM %99s", table_name);
            if (matched != 1) {
                printf("Erreur de syntaxe dans la commande DELETE.\n");
                return;
            }

            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("Erreur : la table '%s' n'existe pas.\n", table_name);
                return;
            }

            delete_all_records(table);
            break;
        }

        //supprimer la table dans la bdd
        case (STATEMENT_DROP_TABLE): {
            char table_name[100];
            
            int matched = sscanf(input_buffer->buffer, "DROP TABLE %99s", table_name);
            if (matched != 1) {
                printf("Erreur de syntaxe dans la commande DROP TABLE.\n");
                return;
            }

            delete_table(btree, table_name);
            break;
       }
    

        default:
            printf("Commande non reconnue.\n");
            break;
    }
}

void repl(BTree* tree) {
    input_buffer = new_input_buffer();
    btree = tree;

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
