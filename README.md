#                   ######################################"README - Système de Base de Données en C ######################################


Ce projet est une implémentation d'un système de base de données en langage C. Il permet de créer, manipuler, et gérer des tables et des enregistrements, tout en offrant une structure sécurisée grâce à un système d'authentification basé sur des rôles (admin et user).

Le projet utilise des structures de données: <B-trees> pour organiser et accéder aux tables de manière performante, en plus d'un système d'authentification basé sur un fichier
 .env .


# 1 - Compilation du code :
    Pour compiler le projet, Il faut un GCC (version 14.2.0). Voici les étapes :
    1 - Créer un fichier .env à partir du modèle fourni (.env.example) et renseigner les identifiants des utilisateurs.

    2 - Utilisez la commande < make > pour compiler le projet.
        -> Cette commande génère un exécutable nommé database.exe
        -> Le fichier exécutable sera créé dans le répertoire courant du projet.
        -> Exécuter le fichier database.exe directement dans le terminal.

    3 - La commande < make clean > pour nettoyer les fichiers objets générés.


# 2 - Fonctionnement du Système
* Authentification
    - Lors du démarrage, l'utilisateur doit fournir un nom d'utilisateur et un mot de passe.
    - Les identifiants sont comparés avec ceux stockés dans le fichier .env.
    - En cas d'erreur, l'utilisateur peut réessayer jusqu'à trois fois.

* Commandes Disponibles
    Voici un résumé des commandes disponibles :
    - Gestion des Tables :
        <CREATE TABLE table_name> : Crée une nouvelle table (possiblité de création de plusieurs tables).
        <SHOW TABLES> : Affiche toutes les tables existantes.
        <DROP TABLE table_name> : Supprime une table et ses données.

    - Gestion des Données :
        <INSERT INTO table_name VALUES(val1, val2, ...)> : Insère un enregistrement.
        <SELECT * FROM table_name> : Affiche tous les enregistrements d'une table.
        <SELECT * FROM table_name WHERE field = value> : Filtre les enregistrements.
        <DELETE FROM table_name WHERE field = value> : Supprime les enregistrements correspondant à la condition.
        <UPDATE table_name SET field=value WHERE field=value> : Met à jour les enregistrements correspondant à une condition.

    - Menu d'Aide :
        <MENU> : Affiche la liste des commandes et leur syntaxe.

* Sauvegarde et persistance des Données
    Les données de chaque table sont sauvegardées dans un fichier texte au format sauvegarde/table_name.txt, et recharger automatiquement lors de l'execution.


# 3 - Caractéristiques Principales

#   Authentification:
    Il existe deux rôles :
*   Admin : Accès complet (création, modification, suppression de tables, etc.).
*   User : Accès en lecture seule (visualisation des tables et des enregistrements).
    Les identifiants sont stockés dans un fichier .env.
    Validation des utilisateurs avec la possibilité de réessayer en cas d'erreur (3 tentatives).

#   Fonctionnalités de Gestion des Données :
*   Création de Tables : Définir des champs dynamiquement.
*   Insertion de Données : Ajout des enregistrements aux tables.
*   Affichage des Données :
        - Afficher toutes les données ou appliquer des conditions avec WHERE sur un champ souhaité.
        - Mise à jour des Données : Modifier des enregistrements existants.
*   Suppression :
        - Supprimer des enregistrements spécifiques ou tous les enregistrements d'une table.
        - Supprimer une table entière.
*   Affichage des Tables : Lister toutes les tables dans la bdd.

#   Persistance :
*   Sauvegarde Automatique :
        - Les données de chaque table sont sauvegardées dans des fichiers individuels situés dans le répertoire sauvegarde/.
        - Par exemple, une table nommée users sera sauvegardée dans un fichier sauvegarde/users.txt.
*   Chargement au Démarrage :
        - Lors de l'exécution, le système charge automatiquement toutes les tables sauvegardées dans le répertoire sauvegarde/.
        - Les tables, leurs champs et leurs enregistrements sont reconstruits à partir des fichiers.
        - Cela garantit que toutes les données sont persistantes entre les sessions.



# 4 - Architecture des Données :
Les B-trees sont utilisés pour stocker et organiser les tables.
Les données des tables sont sauvegardées dans des fichiers texte dans un dossier sauvegarde/table_name.txt.
*   Pourquoi les B-tree:
    * Équilibrage Automatique:
    - Les B-trees garantissent que toutes les feuilles sont à la même profondeur, ce qui évite les problèmes de déséquilibre des arbres binaires, où la recherche pourrait devenir linéaire.
    * Support pour un Grand Nombre de Nœuds:
    - Contrairement aux binary trees, où chaque nœud a au maximum deux enfants, les B-trees peuvent avoir plusieurs enfants. Cela permet de réduire la profondeur de l’arbre et d’accélérer les recherches.
    * Adapté aux Bases de Données:
    - Les bases de données traditionnelles utilisent souvent des B+ trees, une variante des B-trees, car elles permettent d’organiser les données efficacement pour les opérations de recherche, insertion, et suppression.

# 5 - Structure des Fichiers :
* auth.c
    - Gère l'authentification des utilisateurs.
    - Charge les identifiants depuis un fichier .env.
    - Implémente un contrôle basé sur les rôles (admin et user).
* btree.c
    - Implémente les B-trees pour gérer les tables.
    - Inclut des fonctions pour insérer, chercher, supprimer des tables et des nœuds.
    - Implémente la sauvegarde.
* table.c :
    - Implémente les fonctionnalités de gestion des tables :
    - Ajout et suppression de champs.
    - Insertion, suppression, et mise à jour des enregistrements.
    - Sauvegarde des tables dans des fichiers texte.
* input.c :
    - Gère les interactions avec l'utilisateur :
    - Lecture des commandes.

* repl.c :
    - Interface utilisateur principale (boucle REPL - Read-Eval-Print-Loop).
    - Analyse les commandes saisies et les exécute via les fonctions appropriées.

* tests.c :
    - Implémenter des tests unitaires  pour valider les fonctionnalités principales.

* sauvegarde :
    - dossier pour sauvegarder les données.


# 6 - Tests et Validation
Le projet inclut des tests pour valider les fonctionnalités principales :
*   Tests inclus :
    - Authentification des utilisateurs avec différents rôles.
    - Création et suppression de tables.
    - Insertion et suppression d'enregistrements.
*   Exécution des tests : 
    - Utiliser la commande <make run-tests> pour éxecuter des tests