#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "btree.h"
#include "table.h"
#include "auth.h"

// Test l'insertion dans un B-tree
void test_insert_btree() {
    BTree* tree = create_btree(3); // Facteur d'ordre 3
    Table* table1 = create_table("test_table");
    insert_btree(tree, table1);

    Table* result = search_btree(tree, "test_table");
    assert(result != NULL);
    assert(strcmp(result->table_name, "test_table") == 0);

    printf(" Test passed: Insertion in B-tree\n");
}

// Test la suppression dans un B-tree
void test_delete_btree() {
    BTree* tree = create_btree(3);
    Table* table1 = create_table("test_table");
    insert_btree(tree, table1);

    delete_table(tree, "test_table");
    assert(search_btree(tree, "test_table") == NULL);

    printf("✅ Test passed: Deletion from B-tree\n");
}

// Test la création de table et ajout de champs
void test_create_table() {
    Table* table = create_table("users");
    add_field(table, "id", "int");
    add_field(table, "name", "varchar");

    assert(strcmp(table->table_name, "users") == 0);
    assert(table->num_fields == 2);
    assert(strcmp(table->fields[0].field_name, "id") == 0);
    assert(strcmp(table->fields[1].field_name, "name") == 0);

    printf("✅ Test passed: Table creation and field addition\n");
}

// Test l'insertion d'enregistrements
void test_insert_records() {
    Table* table = create_table("test_table");
    add_field(table, "id", "int");
    add_field(table, "name", "varchar");

    char* values1[] = {"1", "Alice"};
    char* values2[] = {"8", "Bob"};
    char* values_duplicate[] = {"1", "Charlie"};

    // Test d'insertion réussie
    assert(insert_record(table, values1, 2) == 0);
    assert(insert_record(table, values2, 2) == 0);

    // Test de duplication d'ID
    assert(insert_record(table, values_duplicate, 2) == -1);

    // Validation du contenu de la table
    assert(table->num_records == 2);
    assert(strcmp(table->records[0].values[1], "Alice") == 0);
    assert(strcmp(table->records[1].values[1], "Bob") == 0);

    printf("✅ Test passed: insert_record\n");
}


// Test la suppression avec condition
void test_delete_with_condition() {
    Table* table = create_table("users");
    add_field(table, "id", "int");
    add_field(table, "name", "varchar");

    char* values1[] = {"1", "Alice"};
    char* values2[] = {"2", "Bob"};

    insert_record(table, values1, 2);
    insert_record(table, values2, 2);

    delete_from_table_with_condition(table, "id", "1");
    assert(table->num_records == 1);
    assert(strcmp(table->records[0].values[1], "Bob") == 0);

    printf("✅ Test passed: Delete with condition\n");
}


void test_save_record_to_file() {
    Table* table = create_table("test_table");
    add_field(table, "id", "int");
    add_field(table, "name", "varchar");

    char* values1[] = {"1", "Alice"};
    char* values2[] = {"2", "Bob"};

    insert_record(table, values1, 2);
    insert_record(table, values2, 2);

    save_record_to_file(table, values1, 2);
    save_record_to_file(table, values2, 2);

    // Voir le contenu de "sauvegarde/test_table.txt"
    printf("✅ Test passed: save_record_to_file\n");
}


// Test l'authentification
void test_authentication() {
    Credentials creds;
    strcpy(creds.admin_username, "admin");
    strcpy(creds.admin_password, "admin123");

    assert(authenticate_user(&creds) == true);

    strcpy(creds.user_username, "user");
    strcpy(creds.user_password, "wrong_password");
    assert(authenticate_user(&creds) == false);

    printf("✅ Test passed: Authentication\n");
}

int main() {
    printf("Running tests...\n");

    test_insert_btree();
    test_delete_btree();
    test_create_table();
    test_insert_records();
    test_delete_with_condition();
    test_save_record_to_file();
    test_authentication();

    printf("All tests passed successfully!\n");
    return 0;
}
