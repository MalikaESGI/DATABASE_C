#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "table.h"
#include "btree.h"
#include "input.h"
#include "auth.h"


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
    STATEMENT_DELETE_WHERE,
    STATEMENT_DELETE_ALL,
    STATEMENT_DROP_TABLE,
    STATEMENT_UPDATE,
    STATEMENT_MENU,
    STATEMENT_EXIT,
    
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
            statement->type = STATEMENT_DELETE_WHERE;
        } else {
            statement->type = STATEMENT_DELETE_ALL;
        }
        return PREPARE_SUCCESS;
    }

    if (strncmp(input_buffer->buffer, "DROP TABLE", 10) == 0) {
        statement->type = STATEMENT_DROP_TABLE;
        return PREPARE_SUCCESS;
    }

    if (strncmp(input_buffer->buffer, "UPDATE", 6) == 0) {
    statement->type = STATEMENT_UPDATE;
    return PREPARE_SUCCESS;
    }

    if (strncmp(input_buffer->buffer, "MENU", 4) == 0) {
    statement->type = STATEMENT_MENU;
    return PREPARE_SUCCESS;
   }

   if (strcmp(input_buffer->buffer, "exit") == 0) {
        return STATEMENT_EXIT;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement) {
    switch (statement->type) {
        case (STATEMENT_CREATE_TABLE): {

            if (!has_permission("admin")){
            printf("Error: You do not have permission to CREATE TABLE.\n");
            return;
            } 

            char table_name[100];
            int matched = sscanf(input_buffer->buffer, "CREATE TABLE %99s", table_name);

            if (matched != 1) {
                printf("Error : Invalid table name.\n");
                return;
            }

            // Vérifier si la table existe déjà dans le B-tree
            if (table_exists(btree, table_name)) {
                printf("Error : The table '%s' already exists.\n", table_name);
                return;
            }

            // Si la table n'existe pas, crée la nouvelle table
            Table* table = create_table(table_name);
            add_field(table, "id", "int(AUTO)");
            printf("Table '%s' created successfully. Please specify the fields.\n", table->table_name);

            char field_name[100];
            char field_type[100];

            // Saisie des champs
            while (true) {
                printf("%s > Field name (or 'q' to finish) : ", table->table_name);
                scanf("%s", field_name);

                if (strcmp(field_name, "q") == 0) {
                    break;
                }

                printf("%s > Field type : ", table->table_name);
                scanf("%s", field_type);

                add_field(table, field_name, field_type);
            }

            insert_btree(btree, table);
            printf("\n<---Table %s created successfully--->\n", table->table_name);
            print_table(table);
            
            create_backup_file(table_name, table);
            break;
        }

        case (STATEMENT_INSERT): {
            char table_name[100];
            char values[100];

            if (!has_permission("admin")){
            printf("Error: You do not have permission to INSERT DATA.\n");
            return;
            } 

            int matched = sscanf(input_buffer->buffer, "INSERT INTO %99s VALUES (%99[^\n])", table_name, values);

            if (matched != 2) {
                printf("Error : Syntax error in the INSERT command.\n");
                return;
            }

            // Rechercher la table dans le B-tree
            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("Error : The table '%s' does not exist.\n", table_name);
                return;
            }

            // Le nombre de champs dans la table
            int expected_values_count = table->num_fields;

            // tableau pour stocker les valeurs après la découpe
            char* values_array[expected_values_count];
            int index = 0;

            //strtok pour découper les valeurs
            char* token = strtok(values, ",");
            while (token != NULL) {
                while (*token == ' ') token++;  //Retirer les espaces des valeurs sasies
                values_array[index] = strdup(token);
                index++;
                token = strtok(NULL, ",");
            }
            // printf("%d, %d\n", index, expected_values_count); 

            // Vérifier du nombre de valeurs saisies
            if (index != expected_values_count) {
                printf("Error : The number of values does not match the table's fields.\n");
                
                // Libérer la mémoire
                for (int i = 0; i < index; i++) {
                    free(values_array[i]);
                }
                
                return;
            }

            if (insert_record(table, values_array, expected_values_count) == 0) {
                printf("Successful insertion into the table '%s'.\n", table_name);
                save_record_to_file(table, values_array, expected_values_count);
            }
            for (int i = 0; i < expected_values_count; i++) {
                free(values_array[i]);
            }

            break;
        }

        case (STATEMENT_SHOW_TABLES):{
            printf("\n<--List of tables in the database--->\n");
            show_tables(btree->root);  
            break;
        }

        case (STATEMENT_SELECT): {
        char table_name[100];

        int matched = sscanf(input_buffer->buffer, "SELECT * FROM %99s", table_name);

        if (matched != 1) {
            printf("Error : Syntax error in the SELECT command.\n");
            return;
        }

        Table* table = search_btree(btree, table_name);
        if (table == NULL) {
            printf("The table '%s' does not exist.\n", table_name);
            return;
        }

        // Afficher les enregistrements de la table
        select_from_table(table);
        break;
    }

    case (STATEMENT_SELECT_WHERE): {
    
            char table_name[100], field_name[100], value[100];
            int matched = sscanf(input_buffer->buffer, "SELECT * FROM %99s WHERE %99s = %99[^\n]", table_name, field_name, value);

            if (matched != 3) {
                printf("Error : Syntax error in the SELECT command with WHERE.\n");
                return;
            }

            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("The table '%s' does not exist.\n", table_name);
                return;
            }

            select_from_table_where(table, field_name, value);
            break;
       }

        // supprimer tous les enregistrements
        case (STATEMENT_DELETE_ALL): {
            char table_name[100];
            if (!has_permission("admin")){
            printf("Error: You do not have permission to DELETE DATA.\n");
            return;
            } 
            
            int matched = sscanf(input_buffer->buffer, "DELETE FROM %99s", table_name);
            if (matched != 1) {
                printf("Error : Syntax error in the DELETE (FROM TABLE) command.\n");
                return;
            }

            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("The table '%s' does not exist.\n", table_name);
                return;
            }

            delete_all_records(table);
            break;
        }

        //supprimer la table dans la bdd
        case (STATEMENT_DROP_TABLE): {
            char table_name[100];

            if (!has_permission("admin")){
            printf("Error: You do not have permission to DROP TABLE.\n");
            return;
            } 
            
            int matched = sscanf(input_buffer->buffer, "DROP TABLE %99s", table_name);
            if (matched != 1) {
                printf("Error : Syntax error in the DROP TABLE command.\n");
                return;
            }

            delete_table(btree, table_name);
            break;
       }

       //supprimer un enregistrement dans la bdd selon une condition
       case STATEMENT_DELETE_WHERE: {
            char table_name[100], field_name[100], value[100];

            if (!has_permission("admin")){
            printf("Error: You do not have permission to DELETE DATA.\n");
            return;
            } 
            
            int matched = sscanf(input_buffer->buffer, "DELETE FROM %99s WHERE %99s = %99s", table_name, field_name, value);
            if (matched != 3) {
                printf("Error: Syntax error in the DELETE command.\n");
                return;
            }

            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("Error: Table '%s' not found.\n", table_name);
                return;
            }

            // Suppression des enregistrements correspondant à la condition
            delete_from_table_with_condition(table, field_name, value);
            break;
        }

        case (STATEMENT_UPDATE): {
            char table_name[100], field_to_update[100], new_value[100], where_field[100], where_value[100];

            if (!has_permission("admin")){
            printf("Error: You do not have permission to UPDATE TABLES.\n");
            return;
            } 
            
            int matched = sscanf(input_buffer->buffer, "UPDATE %99s SET %99[^=]=%99[^ ] WHERE %99[^=]=%99s",
                                table_name, field_to_update, new_value, where_field, where_value);

            if (matched != 5) {
                printf("Error : Syntax error in the UPDATE command.\n");
                return;
            }

            Table* table = search_btree(btree, table_name);
            if (table == NULL) {
                printf("The table '%s' does not exist.\n", table_name);
                return;
            }

            update_records(table, field_to_update, new_value, where_field, where_value);
            break;
        }

        case (STATEMENT_MENU): {
            printf("\n--- Database Command Menu ---\n");
            printf("1. CREATE TABLE table_name\n");
            printf("   Description: Creates a new table with fields specified by the user.\n\n");

            printf("2. INSERT INTO table_name VALUES(value1, value2, ...)\n");
            printf("   Description: Inserts a new record into the specified table.\n\n");

            printf("3. SELECT * FROM table_name\n");
            printf("   Description: Displays all records from the specified table.\n\n");

            printf("4. SELECT * FROM table_name WHERE field_name = value\n");
            printf("   Description: Displays records from the table matching the condition.\n\n");

            printf("5. DELETE FROM table_name WHERE field_name = value\n");
            printf("   Description: Deletes records from the table matching the condition.\n\n");

            printf("6. DELETE FROM table_name\n");
            printf("   Description: Deletes all records from the specified table.\n\n");

            printf("7. DROP TABLE table_name\n");
            printf("   Description: Deletes the specified table from the database.\n\n");

            printf("8. SHOW TABLES\n");
            printf("   Description: Displays all tables in the database.\n\n");

            printf("9. UPDATE table_name SET field_name=value WHERE field_name=value\n");
            printf("   Description: Updates records in the table matching the condition.\n\n");

            printf("9. EXIT\n");
            printf("   Description: Quit the database.\n\n");

            printf("--- End of Menu ---\n");
        break;
     }
      case (STATEMENT_EXIT): {
        printf("BY");
        return;
      }

        default:
            printf("Unrecognized command.\n");
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
