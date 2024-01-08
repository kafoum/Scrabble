// main.c

#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Ajout de la biblioth�que time pour initialiser le g�n�rateur de nombres pseudo-al�atoires
#include <string.h>
#include <unistd.h> // n�cessaire pour usleep
#include <ctype.h>
#include <windows.h>
#include <stdbool.h>
// scrabble.c



int main() {
    // Initialisation des structures n�cessaires (sac, plateau, joueur, etc.)
    struct SacDeJetons sac;
    struct Plateau plateau;
    struct Joueur joueur;

    // ... (initialisez d'autres structures ou configurations n�cessaires)

    int choixMenu;

    do {
        // Affichage du menu principal
        printf("Menu Principal:\n");
        printf("1. Nouveau Jeu\n");
        printf("2. Charger Partie\n");
        printf("3. Quitter\n");
        printf("Choisissez une option : ");
        scanf("%d", &choixMenu);

        switch (choixMenu) {
            case 1:
                // Nouveau Jeu
                Menu();
                break;

            case 2:
                // Charger Partie (ajoutez votre logique de chargement ici)
                printf("Fonctionnalit� de chargement de partie non encore impl�ment�e.\n");
                break;

            case 3:
                // Quitter
                printf("Au revoir !\n");
                return 0;

            default:
                printf("Option invalide. Veuillez choisir une option valide.\n");
                break;
        }

    } while (1);  // Boucle infinie pour le menu principal

    return 0;
}
