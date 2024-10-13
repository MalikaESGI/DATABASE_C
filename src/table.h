#ifndef TABLE_H
#define TABLE_H

typedef struct {
    char* field_name;
    char* field_type;
} Field;

typedef struct {
    int id;             // L'identifiant auto-incrémenté
    char** values;      // Tableau pour stocker les valeurs insérées (sans le champ `id`)
    int num_values;     // Nombre de valeurs (sans compter `id`)
} Record;

typedef struct {
    char* table_name;   // Nom de la table
    Field* fields;      // Tableau des champs (structure de la table)
    int num_fields;     // Nombre de champs (sans compter `id`)
    Record* records;    // Tableau des enregistrements
    int num_records;    // Nombre d'enregistrements dans la table
    int next_id;        // Le prochain identifiant auto-incrémenté
} Table;

Table* create_table(const char* table_name);
void add_field(Table* table, const char* field_name, const char* field_type);
void print_table(Table* table);
void insert_record(Table* table, char** values, int num_values);
void print_records(Table* table) ;

#endif
