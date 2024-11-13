#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "repl.h"
#include "table.h"
#include "btree.h"

int main(int argc, char* argv[], char* envp[]){
    BTree* tree = create_btree(3); 
    sauvegarde(tree);
    repl(tree); 
}
