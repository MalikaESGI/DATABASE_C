#include "btree.h"
#include <stdlib.h>

BTree* create_btree(int t) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    tree->root = NULL;
    tree->t = t;
    return tree;
}

void insert_btree(BTree* tree, Table* table) {
    if (tree->root == NULL) {
        // Si l'arbre est vide, on crée le premier nœud
        BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
        node->tables = (Table**)malloc(sizeof(Table*) * (2 * tree->t - 1));  // Capacité maximale
        node->tables[0] = table;  // Insère la première table
        node->num_tables = 1;
        node->children = NULL;  // Pas encore d'enfants
        node->num_children = 0;
        node->is_leaf = 1;  // C'est une feuille car il n'a pas encore d'enfants
        tree->root = node;
    } else {
        // Ajoute la table dans le nœud racine
        BTreeNode* node = tree->root;
        
        if (node->num_tables < (2 * tree->t - 1)) {
            // Il reste de la place dans ce nœud, insérons la table
            node->tables[node->num_tables] = table;
            node->num_tables += 1;
        } else {
            // Si le nœud est plein, il faudra le scinder 
            printf("Erreur: le nœud est plein, scission nécessaire (à implémenter).\n");
        }
    }
}



int table_exists(BTree* tree, const char* table_name) {
    BTreeNode* node = tree->root;

    // Parcourir l'arbre pour vérifier si la table existe déjà
    while (node != NULL) {
        // Parcourir les tables dans le nœud
        for (int i = 0; i < node->num_tables; i++) {
            if (strcmp(node->tables[i]->table_name, table_name) == 0) {
                // Si une table avec le même nom est trouvée
                return 1;
            }
        }

        // Si ce nœud est une feuille, on arrête car il n'a pas d'enfants
        if (node->is_leaf) {
            return 0;  // Si c'est une feuille et qu'on ne l'a pas trouvée, elle n'existe pas
        } else {
            // Parcourir les enfants pour continuer la recherche
            for (int i = 0; i < node->num_children; i++) {
                if (table_exists(tree, table_name)) {
                    return 1;
                }
            }
            // Si aucun enfant n'a été trouvé, on arrête
            node = NULL;
        }
    }

    return 0;  // La table n'a pas été trouvée
}


