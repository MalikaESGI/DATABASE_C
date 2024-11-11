#ifndef BTREE_H
#define BTREE_H

#include "table.h"

// Structure d'un nœud de B-tree où chaque nœud représente une table unique
typedef struct BTreeNode {
    Table* table;                // Pointeur vers une table unique
    struct BTreeNode** children; // Tableau de pointeurs vers les enfants
    int num_children;            // Nombre d'enfants dans ce nœud
    int is_leaf;                 // 1 si c'est une feuille, 0 sinon
} BTreeNode;

typedef struct {
    BTreeNode* root;
    int t;  // le minimum du B-tree
} BTree;

BTree* create_btree(int t);
void delete_table(BTree* tree, const char* table_name);
void insert_btree(BTree* tree, Table* table);
int table_exists(BTree* tree, const char* table_name);
void show_tables(BTreeNode* node);
Table* search_btree(BTree* tree, const char* table_name);
void sauvegarde(BTree* tree);
// Déclaration de la fonction search_btree_recursive
Table* search_btree_recursive(BTreeNode* node, const char* table_name);

// Déclaration de la fonction delete_table_recursive
int delete_table_recursive(BTree* tree, BTreeNode* node, BTreeNode* parent, const char* table_name);


#endif
