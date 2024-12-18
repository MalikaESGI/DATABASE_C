# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# -Wall : Active tous les avertissements de compilation standard. 
#		 Permet d'identifier des problèmes potentiels dans le code, comme des variables inutilisées ou des conversions implicites

# -Wextra : Active des avertissements supplémentaires non inclus dans -Wall.
# 			Fournit des avertissements plus stricts pour repérer des erreurs subtiles.

# -g : Ajoute des informations de débogage dans l'exécutable.

# Target executable name
TARGET = database

# Test
TEST_TARGET = tests

# Source files
SOURCES = src/main.c src/btree.c src/table.c src/input.c src/repl.c src/auth.c
TEST_SOURCES = src/tests.c src/btree.c src/table.c src/input.c src/repl.c src/auth.c

# Object files
OBJECTS = $(SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Link the target executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile and link the test executable
tests: $(TEST_OBJECTS)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_OBJECTS)

# Run the tests
run-tests: tests
	./$(TEST_TARGET)

# Clean up build files
clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_OBJECTS) $(TEST_TARGET)

# Phony targets
.PHONY: all clean tests run-tests
