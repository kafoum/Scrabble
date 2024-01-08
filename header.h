// scrabble.h

#ifndef SCRABBLE_H
#define SCRABBLE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h> // Ajout de la bibliothèque time pour initialiser le générateur de nombres pseudo-aléatoires
#include <string.h>
#include <unistd.h> // nécessaire pour usleep
#include <ctype.h>
#include <windows.h>
#define LARGEUR_PLATEAU 15
#define HAUTEUR_PLATEAU 15
#define RESET_COLOR      7
#define BONUS_LETTRE_DOUBLE 'L'
#define BONUS_LETTRE_TRIPLE 'l'
#define BONUS_MOT_DOUBLE 'M'
#define BONUS_MOT_TRIPLE 'm'

#define NOMBRE_TOTAL_DE_JETONS 100 // À ajuster en fonction des règles spécifiques du jeu
#define NOMBRE_LETTRES_ALPHABET 26
#define NOMBRE_LETTRES_PIOCHE 7
#define NOMBRE_JOUEURS 4

#define MAX_MOTS_DICTIONNAIRE 100000  // Ajustez cela en fonction de la taille de votre dictionnaire


#include <pthread.h>

// Structure pour représenter un jeton
struct Jeton {
    char lettre;
    int valeur;
};

// Structure pour représenter le sac de jetons
struct SacDeJetons  {
    struct Jeton jetons[NOMBRE_TOTAL_DE_JETONS]; // NOMBRE_TOTAL_DE_JETONS est à définir
    int nbJetonsRestants;
}sac;

struct Joueur {
    char nom[50];
    int score;
    char chevalet[7];
    char dernierMot[8];
    int dernierScore;
    int dernierLigne;
    int dernierColonne;
    char dernierSens;
    int finPartie;
};

struct MotPlace {
    char mot[15];
    int ligne;
    int colonne;
    char sens;
};

struct Plateau {
    char cases[LARGEUR_PLATEAU][HAUTEUR_PLATEAU];
    char bonus[LARGEUR_PLATEAU][HAUTEUR_PLATEAU];  // Ajout du tableau bonus
    struct MotPlace motsPlaces[50]; // Tableau pour stocker les mots déjà placés
    int nbMotsPlaces;               // Nombre de mots déjà placés
};

// Définissez vos codes couleur ici
#define ROUGE_COLOR 12
#define VERT_COLOR 10
#define BLEU_COLOR 9
#define JAUNE_COLOR 14
#define VIOLET_COLOR 13

struct ChronometreInfo {
    pthread_t threadChrono;
    time_t debutChrono;
};


void initialiserPlateau(struct Plateau *plateau);
void afficherPlateau(const struct Plateau *plateau);
int saisirNombreJoueurs();
struct Joueur *saisirNomsJoueurs(int nombreJoueurs);
void libererMemoireJoueurs(struct Joueur *joueurs);
void afficherInformationJoueur(const struct Joueur *joueur);
void initialiserChevalet(struct Joueur *joueur);
void saisirMotPlacement(struct Joueur *joueur);
void saisirEmplacement(struct Joueur *joueur, int *ligne, int *colonne);
void saisirSensMot(struct Joueur *joueur);
void afficherMotSurPlateau(struct Plateau *plateau, const struct Joueur *joueur);
void afficherPlateauAvecMots(const struct Plateau *plateau);
void setColor(int color);
void resetColor();
int getColorForBonus(char bonusSymbol);
void afficherPlateauAvecBordures(const struct Plateau *plateau);
void jouerParties();
void initialiserSacDeJetons(struct SacDeJetons *sac);
void piocherLettres(struct SacDeJetons *sac, struct Joueur *joueur, int nbLettres);
void tourDeJeu(struct SacDeJetons *sac, struct Plateau *plateau, struct Joueur *joueurs, int joueurActuel);
bool motValide(const char *mot);
int Menu();
void abandonnerJeu(struct SacDeJetons *sac, struct Plateau *plateau, struct Joueur *joueur);
// Fonctions pour le chronomètre
void* chronometre(void *arg);
void reinitialiserChronometre(struct ChronometreInfo *chronoInfo);
void demarrerChronometre(struct ChronometreInfo *chronoInfo);
void arreterChronometre(struct ChronometreInfo *chronoInfo);
void afficherChronometre(struct ChronometreInfo *chronoInfo);
#endif // SCRABBLE_H
