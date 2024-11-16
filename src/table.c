#include "table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#ifdef _WIN32
  #include <direct.h>
#endif
#include <sys/types.h>
#include "btree.h"

Table* create_table(const char* table_name) {
    Table* table = (Table*)calloc(1, sizeof(Table)); //calloc pour initialiser à zéro
    if (table == NULL) {
        printf("Error: Memory allocation error for the table.\n");
        return NULL;
    }
    table->table_name = strdup(table_name);
    table->fields = NULL;
    table->num_fields = 0;
    table->records = NULL;
    table->num_records = 0;
    return table;
}

void add_field(Table* table, const char* field_name, const char* field_type) {
    table->num_fields++;
    table->fields = realloc(table->fields, table->num_fields * sizeof(Field));
    if (table->fields == NULL) {
        printf("Error: Memory allocation error for the fields");
        return;
    }
    table->fields[table->num_fields - 1].field_name = strdup(field_name);
    table->fields[table->num_fields - 1].field_type = strdup(field_type);
}


void print_table(Table* table) {
    printf("+--------------------------------+\n");
    printf("|         Table: %s           |\n", table->table_name);
    printf("+--------------------------------+\n");

    printf("| %-15s | %-12s |\n", "Field Name", "Field Type");
    printf("+-------------------+------------+\n");

    for (int i = 0; i < table->num_fields; i++) {
        printf("| %-15s | %-12s |\n", table->fields[i].field_name, table->fields[i].field_type);
    }

    printf("+-------------------+------------+\n");
}

int insert_record(Table* table, char** values, int num_values) {
    // Vérifie que le nombre de valeurs correspond au nombre de champs
    if (num_values != table->num_fields) {
        printf("Error: The number of values does not match the table fields.\n");
        return -1;
    }

    for (int i = 0; i < table->num_records; i++) {
        if (strcmp(table->records[i].values[0], values[0]) == 0) {  // l'ID est la première valeur
            printf("Error: The ID %s already exists in the table.\n", values[0]);
            return -1;
        }
    }

    // Allocation du tableau `records` s'il est vide
    if (table->records == NULL) {
        table->records = calloc(1, sizeof(Record));
        if (table->records == NULL) {
            printf("Error : Memory allocation error for records.\n");
            return -1;
        }
    } else {
        // Redimensionner pour ajouter un nouvel enregistrement
        Record* temp_records = realloc(table->records, sizeof(Record) * (table->num_records + 1));
        if (temp_records == NULL) {
            printf("Error : Memory allocation error for records.\n");
            return -1;
        }
        table->records = temp_records;
    }

    //nouvel enregistrement
    Record* new_record = &table->records[table->num_records];
    new_record->num_values = num_values;

    // Allouer la mémoire pour stocker toutes les valeurs
    new_record->values = calloc(num_values, sizeof(char*));
    if (new_record->values == NULL) {
        printf("Error : Memory allocation error for record values.\n");
        return -1;
    }

    // Copier chaque valeur dans le nouvel enregistrement
    for (int i = 0; i < num_values; i++) {
        new_record->values[i] = strdup(values[i]);
        if (new_record->values[i] == NULL) {
            printf("Error : Memory allocation error for the value of field %d.\n", i);
            return -1;
        }
    }
    table->num_records++;  // Incrémenter le nombre d'enregistrements
    return 0;
}


void save_record_to_file(Table* table, char** values, int num_values) {

    //chemin  /sauvegarde/nom_table.txt)
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "sauvegarde/%s.txt", table->table_name);

    //Ouvrir le fichier en mode ajout
    FILE *file = fopen(filepath, "a");
    if (file == NULL) {
        printf("Error opening file for table backup '%s'.\n", table->table_name);
        return;
    }

    //ajouter les données insérées
    fprintf(file, "---------- Enregistrement %d ---------\n", table->num_records);
    for (int i = 0; i < num_values; i++) {
        fprintf(file, "%s: %s\n", table->fields[i].field_name, values[i]);
    }
    fclose(file);
}


void select_from_table(Table* table) {
    if (table->num_records == 0) {
        printf("No records in the table '%s'.\n", table->table_name);
        return;
    }

    // les champs
    printf("+");
    for (int i = 0; i < table->num_fields; i++) {
        printf("---------------+");
    }
    printf("\n|");

    for (int i = 0; i < table->num_fields; i++) {
        printf(" %-13s |", table->fields[i].field_name);
    }
    printf("\n+");
    for (int i = 0; i < table->num_fields; i++) {
        printf("---------------+");
    }
    printf("\n");

    for (int i = 0; i < table->num_records; i++) {
        printf("|");
        for (int j = 0; j < table->records[i].num_values; j++) {
            printf(" %-13s |", table->records[i].values[j]);
        }
        printf("\n");
    }

    printf("+");
    for (int i = 0; i < table->num_fields; i++) {
        printf("---------------+");
    }
    printf("\n");
}


void select_from_table_where(Table* table, const char* field_name, const char* value) {
    // Trouver l'index du champ spécifié
    int field_index = -1;
    for (int i = 0; i < table->num_fields; i++) {
        if (strcasecmp(table->fields[i].field_name, field_name) == 0) {//strcasecmp pour ignorer la casse
            field_index = i;
            break;
        }
    }

    if (field_index == -1) {
        printf("Error : Field '%s' not found in the table '%s'.\n", field_name, table->table_name);
        return;
    }

    //champs : en-têtes de la table
    printf("+");
    for (int i = 0; i < table->num_fields; i++) {
        printf("---------------+");
    }
    printf("\n|");
    for (int i = 0; i < table->num_fields; i++) {
        printf(" %-13s |", table->fields[i].field_name);
    }
    printf("\n+");
    for (int i = 0; i < table->num_fields; i++) {
        printf("---------------+");
    }
    printf("\n");

    // afficher selon la condition WHERE
    int match_found = 0;
    for (int i = 0; i < table->num_records; i++) {
        if (strcasecmp(table->records[i].values[field_index], value) == 0) {
            printf("|");
            for (int j = 0; j < table->num_fields; j++) {
                printf(" %-13s |", table->records[i].values[j]);
            }
            printf("\n");
            match_found = 1;
        }
    }

    if (!match_found) {
        printf("No records found for %s = %s.\n", field_name, value);
    }

    printf("+");
    for (int i = 0; i < table->num_fields; i++) {
        printf("---------------+");
    }
    printf("\n");
}

void delete_all_records(Table* table) {
    // Libérer la mémoire de chaque enregistrement
    for (int i = 0; i < table->num_records; i++) {
        for (int j = 0; j < table->num_fields; j++) {
            free(table->records[i].values[j]);
        }
        free(table->records[i].values);
    }

    // Réinitialiser a zéro
    table->num_records = 0;

    // Vider le contenu du fichier de sauvegarde de la table indiqué lors du delete
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "sauvegarde/%s.txt", table->table_name);
    FILE *file = fopen(filepath, "w");
    if (file != NULL) {
        fclose(file);

    } else {
        printf("Error : Unable to clear the file content '%s'.\n", filepath);
    }

    printf("All records in the table '%s' have been deleted.\n", table->table_name);
}

void delete_from_table_with_condition(Table* table, const char* field_name, const char* value) {
    int field_index = -1;

    // Rechercher le champ dans la table
    for (int j = 0; j < table->num_fields; j++) {
        if (strcasecmp(table->fields[j].field_name, field_name) == 0) {
            field_index = j;
            break;
        }
    }
    if (field_index == -1) {
        printf("Error: Field '%s' not found in table '%s'.\n", field_name, table->table_name);
        return;
    }

    int new_num_records = 0;

    // Supprimer les enregistrements selon la condition
    for (int i = 0; i < table->num_records; i++) {
        if (strcmp(table->records[i].values[field_index], value) != 0) {
            // Conserver l'enregistrement
            table->records[new_num_records++] = table->records[i];
        } else {
            // Libérer la mémoire pour les enregistrements supprimés
            for (int k = 0; k < table->num_fields; k++) {
                free(table->records[i].values[k]);
            }
            free(table->records[i].values);
        }
    }

    // Mettre à jour le nombre d'enregistrements
    table->num_records = new_num_records;

    // Mettre à jour le fichier de sauvegarde
    update_backup_file(table);

    printf("Records matching the condition have been deleted from '%s'.\n", table->table_name);
}



void create_backup_file(const char* table_name, Table* table) {
    // vérifier si le dossier de sauvegarde existe 
    struct stat st = {0};
    if (stat("sauvegarde", &st) == -1) {
        #ifdef _WIN32
            mkdir("sauvegarde");
        #else
            mkdir("sauvegarde", 0700);
        #endif
    }
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "sauvegarde/%s.txt", table_name);

    // Créer le fichier pour la table
    FILE *file = fopen(filepath, "w");
    if (file == NULL) {
        printf("Error creating backup file for the table '%s'.\n", table_name);
        return;
    }

    fprintf(file, "TABLE: %s\n", table_name);
    fprintf(file, "FIELDS: ");
    for (int i = 0; i < table->num_fields; i++) {
        fprintf(file, "%s %s", table->fields[i].field_name, table->fields[i].field_type);
        if (i != table->num_fields - 1) {
            fprintf(file, ", ");
        }
    }
    fprintf(file, "\n");
    fclose(file);
}

void update_records(Table* table, const char* field_to_update, const char* new_value, const char* where_field, const char* where_value) {
    int field_to_update_index = -1;
    int where_field_index = -1;

        for (int i = 0; i < table->num_fields; i++) {
            if (strcasecmp(table->fields[i].field_name, field_to_update) == 0) {
                field_to_update_index = i;
            }
            if (strcasecmp(table->fields[i].field_name, where_field) == 0) {
                where_field_index = i;
            }
        }

    // Vérifi si leers champs existent
    if (field_to_update_index == -1) {
        printf("Error : The field '%s' to update was not found in the table.\n", field_to_update);
        return;
    }
    if (where_field_index == -1) {
        printf("Error : The condition field '%s' was not found in the table.\n", where_field);
        return;
    }

    // Parcourir les enregistrements pour effectuer la mise à jour
    int updated_count = 0;
    for (int i = 0; i < table->num_records; i++) {
        if (strcmp(table->records[i].values[where_field_index], where_value) == 0) {
            free(table->records[i].values[field_to_update_index]);
            table->records[i].values[field_to_update_index] = strdup(new_value);
            updated_count++;
        }
    }

    if (updated_count == 0) {
        printf("No records updated.\n");
    } else {
        printf("%d record(s) updated. \n", updated_count);
        update_backup_file(table); 
    }
}


void update_backup_file(Table* table) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "sauvegarde/%s.txt", table->table_name);
    
    FILE* file = fopen(filepath, "w");
    if (file == NULL) {
        printf("Error :  unable to open the backup file for update\n");
        return;
    }

    // Nom de la table et champs
    fprintf(file, "TABLE: %s\nFIELDS: ", table->table_name);
    for (int i = 0; i < table->num_fields; i++) {
        fprintf(file, "%s %s", table->fields[i].field_name, table->fields[i].field_type);
        if (i < table->num_fields - 1) {
            fprintf(file, ", ");
        }
    }
    fprintf(file, "\n");

    // les enregistrements mis à jour
    for (int i = 0; i < table->num_records; i++) {
        fprintf(file, "---------- Enregistrement %d ---------\n", i + 1);
        for (int j = 0; j < table->num_fields; j++) {
            fprintf(file, "%s: %s\n", table->fields[j].field_name, table->records[i].values[j]);
        }
    }
    fclose(file);
}




