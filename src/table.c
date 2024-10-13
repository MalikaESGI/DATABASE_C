#include "table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Table* create_table(const char* table_name) {
    Table* table = (Table*)malloc(sizeof(Table));
    table->table_name = strdup(table_name);
    table->fields = NULL;
    table->num_fields = 0;
     table->next_id = 1;
    return table;
}

void add_field(Table* table, const char* field_name, const char* field_type) {
    table->num_fields++;
    table->fields = realloc(table->fields, table->num_fields * sizeof(Field));
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


void insert_record(Table* table, char** values, int num_values) {
    if (num_values != table->num_fields) {
        printf("Erreur : le nombre de valeurs ne correspond pas au nombre de champs dans la table (sans compter l'id).\n");
        return;
    }

    // Allouer de l'espace pour un nouvel enregistrement
    table->records = realloc(table->records, sizeof(Record) * (table->num_records + 1));
    Record* new_record = &table->records[table->num_records];

    // Attribuer l'id auto-incrémenté
    new_record->id = table->next_id;
    table->next_id++;  // Incrémenter l'id pour la prochaine insertion

    new_record->values = malloc(sizeof(char*) * num_values);
    new_record->num_values = num_values;

    // Stocker les valeurs dans le nouvel enregistrement (sans `id`)
    for (int i = 0; i < num_values; i++) {
        new_record->values[i] = strdup(values[i]);  // Dupliquer la valeur dans la structure
    }

    // Augmenter le nombre d'enregistrements dans la table
    table->num_records++;
}

void print_records(Table* table) {
    if (table->num_records == 0) {
        printf("Aucun enregistrement dans la table '%s'.\n", table->table_name);
        return;
    }

    printf("-----------------------------\n");
   

    // Afficher les noms des champs
    for (int i = 0; i < table->num_fields; i++) {
        printf("%s  |  ", table->fields[i].field_name);
    }
    printf("\n-----------------------------\n");

    // Afficher les enregistrements
    for (int i = 0; i < table->num_records; i++) {
        printf("| %-3d | ", table->records[i].id);  // Afficher l'id

        for (int j = 0; j < table->records[i].num_values; j++) {
            printf("%s  |  ", table->records[i].values[j]);  // Afficher les valeurs des champs
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}




