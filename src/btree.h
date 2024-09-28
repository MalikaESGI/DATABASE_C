#ifndef BTREE_H
#define BTREE_H

#include "table.h"

typedef struct BTreeNode {
    Table** tables;              // Tableau de pointeurs vers les tables
    struct BTreeNode** children; // Tableau de pointeurs vers les enfants
    int num_tables;              // Nombre de tables dans ce nœud
    int num_children;            // Nombre d'enfants dans ce nœud
    int is_leaf;                 // 1 si c'est une feuille, 0 sinon
} BTreeNode;

typedef struct {
    BTreeNode* root;
    int t;  // Le degré minimum du B-tree
} BTree;

BTree* create_btree(int t);
void insert_btree(BTree* tree, Table* table);
int table_exists(BTree* tree, const char* table_name);
void show_tables(BTreeNode* node);


#endif
