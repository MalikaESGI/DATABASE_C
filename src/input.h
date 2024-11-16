#ifndef INPUT_H
#define INPUT_H

#include <unistd.h>
#include <stdio.h>

// Déclaration de la structure InputBuffer
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

// Prototypes des fonctions
InputBuffer* new_input_buffer();
ssize_t my_getline(char **lineptr, size_t *n, FILE *stream);
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer);
void to_uppercase(char* str);

#endif // INPUT_H
