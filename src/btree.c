#include "btree.h"
#include <stdlib.h>
#include <stdio.h>

BTree* create_btree(int t) {
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    tree->root = NULL;
    tree->t = t;
    return tree;
}


void insert_btree(BTree* tree, Table* table) {
    if (tree->root == NULL) {
        // Si l'arbre est vide, on crée le premier nœud avec une seule table
        BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
        node->table = table;  // Associe la table au nœud
        node->children = NULL;  // Pas encore d'enfants
        node->num_children = 0;
        node->is_leaf = 1;  // C'est une feuille car il n'a pas encore d'enfants
        tree->root = node;
    } else {
        BTreeNode* node = tree->root;

        // Vérifier si le nœud actuel est une feuille
        if (node->is_leaf) {
            if (node->num_children == 0) {
                // Si c'est une feuille sans enfants, on ajoute le premier enfant
                node->children = (BTreeNode**)malloc(sizeof(BTreeNode*) * (2 * tree->t));  // Allouer des enfants
                BTreeNode* new_node = (BTreeNode*)malloc(sizeof(BTreeNode));
                new_node->table = table;
                new_node->children = NULL;
                new_node->num_children = 0;
                new_node->is_leaf = 1;

                node->children[0] = new_node;  // Ajouter un nouvel enfant
                node->num_children = 1;
                node->is_leaf = 0;  // Le nœud n'est plus une feuille
            } else {
                // Si le nœud a déjà des enfants, on ajoute le nouveau table dans un enfant
                BTreeNode* new_node = (BTreeNode*)malloc(sizeof(BTreeNode));
                new_node->table = table;
                new_node->children = NULL;
                new_node->num_children = 0;
                new_node->is_leaf = 1;

                node->children[node->num_children] = new_node;  // Ajouter le nouvel enfant
                node->num_children += 1;
            }
        } else {
            // Cas où le nœud actuel n'est pas une feuille, il faut gérer la scission
            printf("Erreur: le nœud n'est pas une feuille, gestion de la scission à implémenter.\n");
        }
    }
}



int table_exists(BTree* tree, const char* table_name) {
    BTreeNode* node = tree->root;

    // Parcourir l'arbre pour vérifier si la table existe déjà
    while (node != NULL) {
        // Vérifier la table dans ce nœud
        if (strcmp(node->table->table_name, table_name) == 0) {
            return 1;  // La table a été trouvée
        }

        // Si ce nœud est une feuille, on arrête la recherche
        if (node->is_leaf) {
            return 0;  // Si c'est une feuille et qu'on ne l'a pas trouvée, elle n'existe pas
        } else {
            // Sinon, parcourir les enfants pour continuer la recherche
            node = node->children[0];  // Exemple de traversée (à adapter)
        }
    }

    return 0;  // La table n'a pas été trouvée
}

void show_tables_recursive(BTreeNode* node) {
    // Afficher la table dans ce nœud
    printf("| %-13s |\n", node->table->table_name);

    // Si ce nœud a des enfants, les parcourir récursivement
    if (!node->is_leaf) {
        for (int i = 0; i < node->num_children; i++) {
            show_tables_recursive(node->children[i]);  // Parcourir chaque enfant
        }
    }
}

void show_tables(BTreeNode* node) {
    if (node == NULL) {
        printf("Aucune table n'existe dans la base de données.\n");
        return;
    }

    // Bordure supérieure - n'imprimer qu'une seule fois
    printf("-----------------\n");
    printf("|    DB         |\n");
    printf("-----------------\n");

    // Fonction récursive pour parcourir les nœuds
    show_tables_recursive(node);

    // Bordure inférieure - n'imprimer qu'une seule fois
    printf("-----------------\n");
}



Table* search_btree(BTree* tree, const char* table_name) {
    if (tree->root == NULL) {
        return NULL;  // L'arbre est vide
    }

    BTreeNode* current_node = tree->root;

    // Parcourir récursivement l'arbre
    while (current_node != NULL) {
        // Vérifier si le nœud actuel contient la table recherchée
        if (strcmp(current_node->table->table_name, table_name) == 0) {
            return current_node->table;  // Table trouvée
        }

        // Si c'est une feuille, on arrête ici
        if (current_node->is_leaf) {
            return NULL;  // Si c'est une feuille et qu'on ne l'a pas trouvée, elle n'existe pas
        } else {
            // Sinon, on parcourt les enfants pour continuer la recherche
            // Pour simplifier, on parcourt tous les enfants séquentiellement
            int found = 0;
            for (int i = 0; i < current_node->num_children; i++) {
                BTreeNode* child_node = current_node->children[i];
                if (child_node != NULL && strcmp(child_node->table->table_name, table_name) == 0) {
                    return child_node->table;  // Table trouvée dans l'un des enfants
                }
            }
            return NULL;  // Table non trouvée dans les enfants
        }
    }

    return NULL;  // Table non trouvée
}







