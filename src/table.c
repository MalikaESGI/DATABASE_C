#include "table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

Table* create_table(const char* table_name) {
    Table* table = (Table*)calloc(1, sizeof(Table)); //calloc pour initialiser à zéro
    if (table == NULL) {
        printf("Erreur d'allocation mémoire pour la table.\n");
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
        printf("Erreur d'allocation mémoire pour les champs.\n");
        return;
    }
    table->fields[table->num_fields - 1].field_name = strdup(field_name);
    table->fields[table->num_fields - 1].field_type = strdup(field_type);
}


void print_table(Table* table) {
    printf("+--------------------------------+\n");
    printf("|         Table: %s         |\n", table->table_name);
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
        printf("Erreur : le nombre de valeurs ne correspond pas aux champs de la table.\n");
        return -1;
    }

    for (int i = 0; i < table->num_records; i++) {
        if (strcmp(table->records[i].values[0], values[0]) == 0) {  // l'ID est la première valeur
            printf("Erreur : L'ID %s existe déjà dans la table.\n", values[0]);
            return -1;
        }
    }

    // Allocation du tableau `records` s'il est vide
    if (table->records == NULL) {
        table->records = calloc(1, sizeof(Record));
        if (table->records == NULL) {
            printf("Erreur d'allocation mémoire pour les enregistrements.\n");
            return -1;
        }
    } else {
        // Redimensionner pour ajouter un nouvel enregistrement
        Record* temp_records = realloc(table->records, sizeof(Record) * (table->num_records + 1));
        if (temp_records == NULL) {
            printf("Erreur d'allocation mémoire pour les enregistrements.\n");
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
        printf("Erreur d'allocation mémoire pour les valeurs de l'enregistrement.\n");
        return -1;
    }

    // Copier chaque valeur dans le nouvel enregistrement
    for (int i = 0; i < num_values; i++) {
        new_record->values[i] = strdup(values[i]);
        if (new_record->values[i] == NULL) {
            printf("Erreur d'allocation mémoire pour la valeur du champ %d.\n", i);
            return -1;
        }
    }
    table->num_records++;  // Incrémenter le nombre d'enregistrements
    return 0;
}


void save_record_to_file(Table* table, char** values, int num_values) {
    // Ouvrir le fichier en mode ajout
    FILE *file = fopen("db.txt", "a");
    if (file == NULL) {
        printf("Erreur d'ouverture du fichier pour la sauvegarde.\n");
        return;
    }

    // Ajouter un enregistrement pour la table spécifiée
    fprintf(file, "----------%d---------\n", table->num_records);
    for (int i = 0; i < num_values; i++) {
        fprintf(file, "%s: %s\n", table->fields[i].field_name, values[i]);
    }
    fprintf(file, "---------------------\n");

    fclose(file);
    printf("Enregistrement sauvegarde dans la table '%s'.\n", table->table_name);
}


void select_from_table(Table* table) {
    if (table->num_records == 0) {
        printf("Aucun enregistrement dans la table '%s'.\n", table->table_name);
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
        printf("Erreur : Champ '%s' non trouvé dans la table '%s'.\n", field_name, table->table_name);
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
        printf("Aucun enregistrement trouvé pour %s = %s.\n", field_name, value);
    }
    
    printf("+");
    for (int i = 0; i < table->num_fields; i++) {
        printf("---------------+");
    }
    printf("\n");
}





