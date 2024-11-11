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
        node->children = (BTreeNode**)malloc(sizeof(BTreeNode*) * (2 * tree->t));
        for (int i = 0; i < 2 * tree->t; i++) {
            node->children[i] = NULL;
        }
        node->num_children = 0;
        node->is_leaf = 1;
        tree->root = node;
    } else {
        // Insérer dans le nœud racine ou un enfant
        BTreeNode* current = tree->root;
        while (!current->is_leaf) {
            // Aller au dernier enfant non nul pour trouver une place
            current = current->children[current->num_children - 1];
        }
        
        // Créer un nouveau nœud pour la table si l'espace le permet
        if (current->num_children < 2 * tree->t) {
            BTreeNode* new_node = (BTreeNode*)malloc(sizeof(BTreeNode));
            new_node->table = table;
            new_node->children = (BTreeNode**)malloc(sizeof(BTreeNode*) * (2 * tree->t));
            for (int i = 0; i < 2 * tree->t; i++) {
                new_node->children[i] = NULL;
            }
            new_node->num_children = 0;
            new_node->is_leaf = 1;

            current->children[current->num_children] = new_node;
            current->num_children++;
        } else {
            printf("Error: Maximum number of tables reached.\n");
        }
    }
}


int table_exists(BTree* tree, const char* table_name) {
    BTreeNode* node = tree->root;

    // Parcourir l'arbre pour vérifier si la table existe déjà
    while (node != NULL) {

        if (strcmp(node->table->table_name, table_name) == 0) {
            return 1; 
        }

        // Si ce nœud est une feuille, on arrête la recherche
        if (node->is_leaf) {
            return 0; 
        } else {
            // parcourir les enfants pour continuer la recherche
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

    // Parcours récursif pour chaque enfant
    for (int i = 0; i < node->num_children; i++) {
        show_tables_recursive(node->children[i]);
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
    return search_btree_recursive(tree->root, table_name);
}

Table* search_btree_recursive(BTreeNode* node, const char* table_name) {
    if (node == NULL) {
        return NULL;  //la table n'existe pas
    }

    // Vérifier si la table actuelle correspond
    if (node->table != NULL && strcmp(node->table->table_name, table_name) == 0) {
        return node->table;
    }

    // Parcourir les enfants
    for (int i = 0; i < node->num_children; i++) {
        Table* result = search_btree_recursive(node->children[i], table_name);
        if (result != NULL) {
            return result;  // Retourner si on a trouvé la table
        }
    }

    return NULL;  // La table pas trouvée
}


void delete_table(BTree* tree, const char* table_name) {
    if (tree->root == NULL) {
        printf("Error: The database is empty.\n");
        return;
    }

    int found = delete_table_recursive(tree, tree->root, NULL, table_name);
    if (found) {
        printf("Table '%s' deleted successfully\n", table_name);
    } else {
        printf("Table '%s' not found in the database\n", table_name);
    }
}

int delete_table_recursive(BTree* tree, BTreeNode* node, BTreeNode* parent, const char* table_name) {
    if (node == NULL) {
        return 0;
    }

    // Vérifier si le nœud contient la table
    if (node->table != NULL && strcmp(node->table->table_name, table_name) == 0) {

        delete_all_records(node->table);

        // Libérer la mémoire des champs de la table
        for (int i = 0; i < node->table->num_fields; i++) {
            free(node->table->fields[i].field_name);
            free(node->table->fields[i].field_type);
        }
        free(node->table->fields);
        free(node->table->table_name);
        free(node->table);

        node->table = NULL; // Marquer la table comme supprimée

        // Supprimer le fichier de sauvegarde de la table
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "sauvegarde/%s.txt", table_name);
        if (unlink(filepath) != 0) {
            printf("Error: Unable to delete the backup file '%s'.\n", filepath);
        }

        // Retirer le nœud de l'arbre
        if (node->num_children == 0 && parent != NULL) {
            for (int i = 0; i < parent->num_children; i++) {
                if (parent->children[i] == node) {
                    free(parent->children[i]);
                    parent->children[i] = NULL;
                    break;
                }
            }
        } else if (node == tree->root && node->table == NULL && node->num_children == 0) {
            free(tree->root);
            tree->root = NULL;
        }

        return 1; // Table trouvée et supprimée
    }

    // Parcourir récursivement les enfants pour chercher la table
    for (int i = 0; i < node->num_children; i++) {
        if (delete_table_recursive(tree, node->children[i], node, table_name)) {
            return 1;
        }
    }

    return 0; // La table n'a pas été trouvée dans ce nœud
}


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

                // Vérifier si la table existe déjà dans le B-tree
                if (search_btree(tree, table_name) != NULL) {
                    printf("Warning: Table '%s' already exists in the B-tree, skipping reload.\n", table_name);
                    fclose(file);
                    continue;
                }

                // Créer une nouvelle table avec le nom extrait
                Table* table = create_table(table_name);

                char line[256];
                bool reading_fields = true;

                // Lire les enregistrements de la table
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

                        // Libérer la mémoire allouée pour chaque enregistrement
                        for (int i = 0; i < table->num_fields; i++) {
                            free(values[i]);
                        }
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
