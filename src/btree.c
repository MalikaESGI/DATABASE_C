#include "btree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

BTree* create_btree(int t) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    tree->root = NULL;
    tree->t = t;
    return tree;
}


void insert_btree(BTree* tree, Table* table) {
    if (tree->root == NULL) {
        // Si l'arbre est vide, créer le premier nœud avec la table
        BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
        node->table = table;
        node->children = NULL;
        node->num_children = 0;
        node->is_leaf = 1;
        tree->root = node;
    } else {
        BTreeNode* node = tree->root;

        // Si le nœud est une feuille
        if (node->is_leaf) {
            // Allouer de l'espace pour les enfants
            if (node->children == NULL) {
                node->children = (BTreeNode**)malloc(sizeof(BTreeNode*) * (2 * tree->t));
                for (int i = 0; i < 2 * tree->t; i++) {
                    node->children[i] = NULL;
                }
            }

            // Trouver un emplacement vide dans les enfants
            int child_index = -1;
            for (int i = 0; i < node->num_children; i++) {
                if (node->children[i] == NULL) {
                    child_index = i;
                    break;
                }
            }

            // Si aucun emplacement vide, on en crée un nouveau
            if (child_index == -1) {
                child_index = node->num_children;
                node->num_children++;
            }

            // Ajouter la table dans un nouvel emplacement
            BTreeNode* new_node = (BTreeNode*)malloc(sizeof(BTreeNode));
            new_node->table = table;
            new_node->children = NULL;
            new_node->num_children = 0;
            new_node->is_leaf = 1;

            node->children[child_index] = new_node;
            node->is_leaf = 0;  // Le nœud n'est plus une feuille car il a des enfants
        } else {
            // Si le nœud n'est pas une feuille, insérer dans un enfant existant
            for (int i = 0; i < node->num_children; i++) {
                if (node->children[i] == NULL) {
                    node->children[i] = (BTreeNode*)malloc(sizeof(BTreeNode));
                    node->children[i]->table = table;
                    node->children[i]->children = NULL;
                    node->children[i]->num_children = 0;
                    node->children[i]->is_leaf = 1;
                    return;
                }
            }
            printf("Error: No space available to add the table.\n");
        }
    }
}



int table_exists(BTree* tree, const char* table_name) {
    BTreeNode* node = tree->root;

    // Parcourir l'arbre pour vérifier si la table existe déjà
    while (node != NULL) {
        // Vérifier la table dans ce nœud
        if (strcmp(node->table->table_name, table_name) == 0) {
            return 1; 
        }

        // Si ce nœud est une feuille, on arrête la recherche
        if (node->is_leaf) {
            return 0;  // Si c'est une feuille et qu'on ne l'a pas trouvée, elle n'existe pas
        } else {
            // Sinon, parcourir les enfants pour continuer la recherche
            node = node->children[0];
        }
    }

    return 0;  // la table n'existe pas
}

void show_tables_recursive(BTreeNode* node) {
    if (node == NULL || node->table == NULL) {
        return;
    }
    printf("| %-13s |\n", node->table->table_name);

    if (!node->is_leaf) {
        for (int i = 0; i < node->num_children; i++) {
            show_tables_recursive(node->children[i]);
        }
    }
}

void show_tables(BTreeNode* node) {
    if (node == NULL) {
        printf("No table exists in the database.\n");
        return;
    }
    printf("-----------------\n");
    printf("|    DB         |\n");
    printf("-----------------\n");

    show_tables_recursive(node);

    printf("-----------------\n");
}



Table* search_btree(BTree* tree, const char* table_name) {
    if (tree->root == NULL) {
        return NULL;  // L'arbre est vide
    }

    BTreeNode* current_node = tree->root;

    while (current_node != NULL) {
        // Vérifier si le nœud actuel contient la table recherchée
        if (strcmp(current_node->table->table_name, table_name) == 0) {
            return current_node->table;  // Table trouvée
        }

        // Si c'est une feuille, on arrête ici
        if (current_node->is_leaf) {
            return NULL;  // la table existe pas
        } else {
            // Sinon, on parcourt les enfants pour continuer la recherche
            for (int i = 0; i < current_node->num_children; i++) {
                BTreeNode* child_node = current_node->children[i];
                if (child_node != NULL && strcmp(child_node->table->table_name, table_name) == 0) {
                    return child_node->table;  // Table trouvée dans l'un des enfants
                }
            }
            return NULL;  //table non trouvée
        }
    }

    return NULL; 
}

void delete_table(BTree* tree, const char* table_name) {
    if (tree->root == NULL) {
        printf("Error : The database is empty.\n");
        return;
    }

    BTreeNode* node = tree->root;
    BTreeNode* parent = NULL;
    int found = 0;

    // Rechercher et supprimer la table dans le nœud courant
    while (node != NULL) {
        if (node->table != NULL && strcmp(node->table->table_name, table_name) == 0) {
            // Supprimer tous les enregistrements de la table
            delete_all_records(node->table);

            // Libérer la mémoire des champs de la table
            for (int i = 0; i < node->table->num_fields; i++) {
                free(node->table->fields[i].field_name);
                free(node->table->fields[i].field_type);
            }
            free(node->table->fields);
            free(node->table->table_name);
            free(node->table);

            node->table = NULL;
            found = 1;
            printf("Table '%s' deleted successfully\n", table_name);

            // Supprimer le fichier de sauvegarde de la table
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "sauvegarde/%s.txt", table_name);

                if (unlink(filepath) != 0) {
                    printf("Error: Unable to delete the backup file '%s'.\n", filepath);
                }

            // retirer si le nœud est vide et sans enfants
            if (node->num_children == 0) {
                if (parent == NULL) {
                    // Si le nœud est la racine et vide, supprimer l'arbre
                    free(node);
                    tree->root = NULL;
                } else {
                    for (int i = 0; i < parent->num_children; i++) {
                        if (parent->children[i] == node) {
                            free(parent->children[i]);
                            parent->children[i] = NULL;
                            break;
                        }
                    }
                }
            }
            break;
        }

        // Si la table n'a pas été trouvée, parcourir les enfants
        int child_found = 0;
        for (int i = 0; i < node->num_children; i++) {
            if (node->children[i] != NULL) {
                parent = node;
                node = node->children[i];
                child_found = 1;
                break;
            }
        }
        if (!child_found) {
            break;
        }
    }

    if (!found) {
        printf("Table '%s' not found in the database\n", table_name);
    }
}



// charger les tables à partir des fichiers de sauvegadre
void sauvegarde(BTree* tree) {
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir("sauvegarde")) != NULL) {
        // Parcourir tous les fichiers dans le dossier "sauvegarde"
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".txt") != NULL) {
                // Récupérer le nom de la table depuis le nom du fichier
                char table_name[100];
                sscanf(ent->d_name, "%[^.]", table_name);

                char filepath[256];
                snprintf(filepath, sizeof(filepath), "sauvegarde/%s", ent->d_name);
                FILE *file = fopen(filepath, "r");
                if (file == NULL) {
                    printf("Error: unable to open the file: %s.\n", filepath);
                    continue;
                }

                // Créer une nouvelle table avec le nom extrait
                Table* table = create_table(table_name);

                char line[256];
                bool reading_fields = true;

                // Lire la structure et les enregistrements de la table
                while (fgets(line, sizeof(line), file)) {
                    if (strncmp(line, "FIELDS:", 7) == 0 && reading_fields) {
                        // ajouter les champs à la table
                        reading_fields = false;

                        char field_name[100], field_type[100];
                        char *field = strtok(line + 7, ",");  // Passer après "FIELDS: "
                        while (field != NULL) {
                            sscanf(field, "%s %s", field_name, field_type);
                            add_field(table, field_name, field_type);
                            field = strtok(NULL, ",");
                        }
                    } else if (strncmp(line, "---------- Enregistrement", 25) == 0) {
                        // Lire un enregistrement
                        char* values[table->num_fields];
                        for (int i = 0; i < table->num_fields; i++) {
                            if (fgets(line, sizeof(line), file) == NULL) break;

                            // Extraire la valeur après le ':' et enlever les espaces
                            char *value_start = strchr(line, ':') + 2;
                            values[i] = strdup(value_start);

                            // Enlever le retour à la ligne en fin de valeur
                            values[i][strcspn(values[i], "\n")] = '\0';
                        }
                        insert_record(table, values, table->num_fields);
                    }
                }
                // Ajouter la table dans le B-tree
                insert_btree(tree, table);
                fclose(file);
            }
        }
        closedir(dir);
    } else {
        printf("Error: file not found.\n");
    }
}

