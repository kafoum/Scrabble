// scrabble_functions.c

#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // Ajout de la biblioth�que time pour initialiser le g�n�rateur de nombres pseudo-al�atoires
#include <string.h>
#include <unistd.h> // n�cessaire pour usleep
#include <ctype.h>
#include <windows.h>
#include <stdbool.h>

// Codes de couleurs ANSI
#define ANSI_RESET "\x1b[0m"
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_PURPLE "\x1b[35m"
#define NOMBRE_LETTRES_ALPHABET 26
#define NOMBRE_LETTRES_PIOCHE 7
#define BONUS_LETTRE_DOUBLE 'L'
#define BONUS_LETTRE_TRIPLE 'l'
#define BONUS_MOT_DOUBLE 'M'
#define BONUS_MOT_TRIPLE 'm'
#define NOMBRE_PARTIES 1000
#define NOMBRE_TOURS 1000

#define TEMPS_MAX 60 // temps en secondes
#define DICTIONARY_FILE_PATH "liste_francais.txt"  // Remplacez ceci par le chemin r�el de votre fichier texte
// Dur�e maximale d'un tour en secondes (60 secondes pour une minute)
#define DUREE_TOUR 60
int obtenirValeurLettre(const struct SacDeJetons *sac, char lettre) {
    for (int i = 0; i < NOMBRE_TOTAL_DE_JETONS; ++i) {
        if (sac->jetons[i].lettre == lettre) {
            return sac->jetons[i].valeur;
        }
    }
    // Gestion d'erreur : lettre non trouv�e dans le sac
    fprintf(stderr, "Erreur : Lettre '%c' non trouv�e dans le sac de jetons.\n", lettre);
    exit(EXIT_FAILURE);
}

int calculerScoreMot(struct Plateau *plateau, struct Joueur *joueur, struct SacDeJetons *sac) {
    int scoreMot = 0;
    int multiplicateurMot = 1;

    // Calculer le score du mot en tenant compte des cases bonus sur le plateau
    for (int i = 0; i < strlen(joueur->dernierMot); ++i) {
        int valeurLettre = obtenirValeurLettre(joueur->dernierMot[i], sac);

        // Obtenir les coordonn�es de la lettre sur le plateau
        int ligne = joueur->dernierLigne;
        int colonne = joueur->dernierColonne + i;

        // V�rifier les cases bonus (� ajuster selon les r�gles du Scrabble)
        switch (plateau->bonus[ligne][colonne]) {
            case '�':  // Mot triple
                multiplicateurMot *= 3;
                scoreMot += valeurLettre;
                break;
            case '@':  // Mot double
                scoreMot += valeurLettre;
                break;
            case '&':  // Lettre double
                scoreMot += valeurLettre * 2;
                break;
            case '%':  // Lettre triple
                scoreMot += valeurLettre * 3;
                break;
            default:
                scoreMot += valeurLettre;
                break;
        }
    }

    // Appliquer le multiplicateur de mot
    scoreMot *= multiplicateurMot;

    return scoreMot;
}

// Impl�mentation de la fonction motValide dans scrabble_functions.c
bool motValide(const char *mot) {
    FILE *dictionnaire = fopen(DICTIONARY_FILE_PATH, "r");
    if (dictionnaire == NULL) {
        perror("Erreur lors de l'ouverture du dictionnaire");
        exit(EXIT_FAILURE);
    }

    char motDictionnaire[50];
    while (fgets(motDictionnaire, sizeof(motDictionnaire), dictionnaire) != NULL) {
        // Retirez le caract�re de nouvelle ligne de la fin du mot
        strtok(motDictionnaire, "\n");

        // Comparez le mot avec celui du dictionnaire (ignorant la casse)
        if (strcasecmp(mot, motDictionnaire) == 0) {
            fclose(dictionnaire);
            return true;  // Le mot est dans le dictionnaire
        }
    }

    fclose(dictionnaire);
    return false;  // Le mot n'est pas dans le dictionnaire
}


void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void resetColor() {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), RESET_COLOR);
}

int getColorForBonus(char bonusSymbol) {
    switch (bonusSymbol) {
        case '#': return ROUGE_COLOR;
        case '�': return VERT_COLOR;
        case '@': return BLEU_COLOR;
        case '&': return JAUNE_COLOR;
        case '%': return VIOLET_COLOR;
        default: return RESET_COLOR;  // Couleur par d�faut
    }
}




// Structure pour stocker les informations du chronom�tre


void* chronometre(void *arg) {
    struct ChronometreInfo *chronoInfo = (struct ChronometreInfo *)arg;
    time(&chronoInfo->debutChrono);

    while (1) {
        sleep(1);  // Attendez une seconde

        time_t maintenant;
        time(&maintenant);

        // V�rifiez si la dur�e maximale du tour est atteinte
        if (difftime(maintenant, chronoInfo->debutChrono) >= DUREE_TOUR) {
            printf("\nLe temps est �coul�!\n");
            break;  // Sortez de la boucle si la dur�e maximale du tour est atteinte
        }
    }

    pthread_exit(NULL);
}

void reinitialiserChronometre(struct ChronometreInfo *chronoInfo) {
    pthread_cancel(chronoInfo->threadChrono);
    pthread_create(&chronoInfo->threadChrono, NULL, chronometre, (void *)chronoInfo);
}

void demarrerChronometre(struct ChronometreInfo *chronoInfo) {

    pthread_create(&chronoInfo->threadChrono, NULL, chronometre, (void *)chronoInfo);
}

void arreterChronometre(struct ChronometreInfo *chronoInfo) {
    pthread_cancel(chronoInfo->threadChrono);
}
void afficherChronometre(struct ChronometreInfo *chronoInfo) {
    time_t maintenant;
    time(&maintenant);

    int tempsRestant = DUREE_TOUR - difftime(maintenant, chronoInfo->debutChrono);

    if (tempsRestant > 0) {
        printf("Temps restant : %d secondes\n", tempsRestant);
    } else {
        printf("Temps �coul�!\n");
    }
}
void afficherMotSurPlateau(struct Plateau *plateau, const struct Joueur *joueur) {
    // V�rifiez si le joueur a jou� un mot
    if (joueur->dernierMot[0] != '\0') {
        // Affichez le mot sur le plateau en fonction de l'orientation
        int ligne = joueur->dernierLigne;
        int colonne = joueur->dernierColonne;

        for (int i = 0; joueur->dernierMot[i] != '\0'; ++i) {
            if (joueur->dernierSens == 'H') {
                // Utilisez une variable temporaire pour �viter les probl�mes potentiels
                char temp = toupper(joueur->dernierMot[i]);
                plateau->cases[ligne + i][colonne] = temp;
            } else if (joueur->dernierSens == 'V') {
                // Utilisez une variable temporaire pour �viter les probl�mes potentiels
                char temp = toupper(joueur->dernierMot[i]);
                plateau->cases[ligne][colonne + i] = temp;
            }
        }

        // Ajoutez le mot � la liste des mots d�j� plac�s sur le plateau
        struct MotPlace nouveauMot;
        strcpy(nouveauMot.mot, joueur->dernierMot);
        nouveauMot.ligne = ligne;
        nouveauMot.colonne = colonne;
        nouveauMot.sens = joueur->dernierSens;

        plateau->motsPlaces[plateau->nbMotsPlaces] = nouveauMot;
        plateau->nbMotsPlaces++;
    }
}


void afficherPlateauAvecMots(const struct Plateau *plateau) {
    // Affichez le plateau avec les mots plac�s
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            printf("%c ", plateau->cases[i][j]);
        }
        printf("\n");
    }
}


void saisirMotPlacement(struct Joueur *joueur) {
    printf("%s, saisissez le mot que vous voulez placer : ", joueur->nom);
    scanf("%s", joueur->dernierMot);
}

void saisirEmplacement(struct Joueur *joueur, int *ligne, int *colonne) {
    printf("%s, saisissez l'emplacement sur le plateau (ligne colonne) : ", joueur->nom);
    scanf("%d %d", ligne, colonne);

    // Assurez-vous que joueur->dernierLigne et joueur->dernierColonne sont correctement mis � jour
    joueur->dernierLigne = *ligne;
    joueur->dernierColonne = *colonne;
}

void saisirSensMot(struct Joueur *joueur) {
    printf("%s, choisissez l'orientation du mot (H pour horizontal, V pour vertical) : ", joueur->nom);
    scanf(" %c", &joueur->dernierSens);
}



void initialiserPlateau(struct Plateau *plateau) {
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            plateau->cases[i][j] = ' ';
            // Initialiser les bonus selon votre logique
            plateau->bonus[i][j] = ' ';

            // Ajout des symboles pour les cases sp�ciales
            if ((i == 0 || i == 7 || i == 14) && (j == 0 || j == 7 || j == 14)) {
                plateau->cases[i][j] = '#'; // Point de d�part
            } else if ((i == 1 || i == 13) && (j == 5 || j == 9)) {
                plateau->cases[i][j] = '@'; // Mot double
            } else if ((i == 2 || i == 12) && (j == 6 || j == 8)) {
                plateau->cases[i][j] = '&'; // Lettre double
            } else if ((i == 3 || i == 11) && (j == 0 || j == 7 || j == 14)) {
                plateau->cases[i][j] = '�'; // Mot triple
            } else if ((i == 5 || i == 9) && (j == 1 || j == 13)) {
                plateau->cases[i][j] = '@'; // Mot double
            } else if ((i == 6 || i == 8) && (j == 2 || j == 6 || j == 8 || j == 12)) {
                plateau->cases[i][j] = '%'; // Lettre triple
            } else if ((i == 7) && (j == 3 || j == 11)) {
                plateau->cases[i][j] = '�'; // Mot triple
            } else if ((i == 14) && (j == 3 || j == 11)) {
                plateau->cases[i][j] = '&'; // Lettre double
            } else if ((i == 3 || i == 11) && (j == 3 || j == 11)) {
                plateau->cases[i][j] = '@'; // Mot double
            } else if ((i == 6 || i == 8) && (j == 2 || j == 12)) {
                plateau->cases[i][j] = '%'; // Lettre triple
            }
        }
    }
}

void afficherPlateau(const struct Plateau *plateau) {
    printf("   A B C D E F G H I J K L M N O\n");
    for (int i = 0; i < LARGEUR_PLATEAU; i++) {
        printf("%2d ", i + 1);
        for (int j = 0; j < HAUTEUR_PLATEAU; j++) {
            char character = plateau->cases[i][j];
            char bonusSymbol = plateau->bonus[i][j];

            const char* color = ANSI_RESET;
            switch (bonusSymbol) {
                case '#': color = ANSI_RED; break;
                case '�': color = ANSI_GREEN; break;
                case '@': color = ANSI_BLUE; break;
                case '&': color = ANSI_YELLOW; break;
                case '%': color = ANSI_PURPLE; break;
                default: break;
            }

            setColor(color);
            printf("%c ", character);
            resetColor();
        }
        printf("\n");
    }
}

void afficherPlateauAvecBordures(const struct Plateau *plateau) {
     // Affiche la ligne sup�rieure avec les lettres A � O
    printf("    A   B   C   D   E   F   G   H   I   J   K   L   M   N   O\n");
    // Affiche la bordure sup�rieure
    printf("   +------------------------------------------------------------+\n");

    for (int i = 0; i < LARGEUR_PLATEAU; i++) {
        // Affiche les indices de ligne
        printf("%2d |", i + 1);

        for (int j = 0; j < HAUTEUR_PLATEAU; j++) {
            char character = plateau->cases[i][j];
            char bonusSymbol = plateau->bonus[i][j];

            const char* color = ANSI_RESET;
            switch (bonusSymbol) {
                case '#': color = ANSI_RED; break;
                case '�': color = ANSI_GREEN; break;
                case '@': color = ANSI_BLUE; break;
                case '&': color = ANSI_YELLOW; break;
                case '%': color = ANSI_PURPLE; break;
                default: break;
            }

            setColor(color);
            // Affiche les bordures gauche et droite autour de chaque case de lettre
            printf(" %c |", character);
            resetColor();
        }

        // Affiche la bordure inf�rieure de chaque ligne
        printf("\n   +-------------------------------------------------------------+\n");
    }
}
int saisirNombreJoueurs() {
    int nombreJoueurs;
    do {
        printf("Entrez le nombre de joueurs (2, 3, ou 4) : ");
        scanf("%d", &nombreJoueurs);

        if (nombreJoueurs < 2 || nombreJoueurs > 4) {
            printf("Nombre de joueurs invalide. Veuillez entrer 2, 3, ou 4.\n");
        }
    } while (nombreJoueurs < 2 || nombreJoueurs > 4);

    return nombreJoueurs;
}

struct Joueur *saisirNomsJoueurs(int nombreJoueurs) {
    struct Joueur *joueurs = malloc(sizeof(struct Joueur) * nombreJoueurs);
    if (joueurs == NULL) {
        perror("Erreur d'allocation m�moire");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nombreJoueurs; ++i) {
        printf("Entrez le nom du Joueur %d : ", i + 1);
        scanf("%s", joueurs[i].nom);
        joueurs[i].score = 0;


    }

    return joueurs;
}

void libererMemoireJoueurs(struct Joueur *joueurs) {
    free(joueurs);
}

void initialiserChevalet(struct Joueur *joueur) {
    // Initialiser le g�n�rateur de nombres pseudo-al�atoires avec le temps actuel
    srand(time(NULL));

    // Lettres disponibles dans le jeu de Scrabble
    char lettresDisponibles[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    for (int i = 0; i < 7; ++i) {
        // G�n�rer un indice al�atoire pour choisir une lettre parmi les lettres disponibles
        int indiceAleatoire = rand() % 26;

        // Assigner la lettre au chevalet du joueur
        joueur->chevalet[i] = lettresDisponibles[indiceAleatoire];
    }

    // Ajouter le caract�re de fin de cha�ne � la fin du chevalet
    joueur->chevalet[7] = '\0';
}




void afficherInformationJoueur(const struct Joueur *joueur) {
    printf("\n=== Information du Joueur ===\n");
    printf("Nom du Joueur : %s\n", joueur->nom);
    printf("Score : %d\n", joueur->score);
    printf("Chevalet : %s\n", joueur->chevalet);
}
void jouerParties() {
    struct Plateau plateau;
    struct Joueur *joueurs;
    int nombreJoueurs;

    // Initialisation du plateau et affichage
    initialiserPlateau(&plateau);
    afficherPlateauAvecBordures(&plateau);

    // Saisie du nombre de joueurs et de leurs noms
    nombreJoueurs = saisirNombreJoueurs();
    joueurs = saisirNomsJoueurs(nombreJoueurs);

    // Boucle principale pour les parties
    for (int partie = 1; partie <= NOMBRE_PARTIES; ++partie) {
        printf("\n--- Partie %d ---\n", partie);

        // Initialisez le chevalet de chaque joueur
        for (int i = 0; i < nombreJoueurs; ++i) {
            initialiserChevalet(&joueurs[i]);
            joueurs[i].finPartie = 0;  // R�initialisez la variable finPartie pour chaque joueur
        }

        // Boucle pour les tours de jeu
        int finDePartie = 0;
        for (int tour = 1; tour <= NOMBRE_TOURS && !finDePartie; ++tour) {
            printf("\n--- Tour %d ---\n", tour);

            // Tours pour chaque joueur
            for (int i = 0; i < nombreJoueurs; ++i) {
                if (joueurs[i].finPartie) {
                    continue;  // Passez au joueur suivant si le joueur a termin� la partie
                }

                printf("\nC'est le tour de %s.\n", joueurs[i].nom);
                tourDeJeu(&sac, &plateau, joueurs, i);

                // V�rifiez si le joueur a atteint un score de 100 ou a abandonn�
                if (joueurs[i].score >= 100 || joueurs[i].finPartie) {
                    printf("%s a termin� la partie.\n", joueurs[i].nom);
                    finDePartie = 1;
                    break;  // Sortez de la boucle des joueurs si la partie est termin�e
                }
            }

            // ... (d'autres �tapes de gestion entre les tours, v�rification de la fin de partie, etc.)
        }

        // ... (d'autres op�rations � la fin d'une partie)
    }

    // Lib�rez la m�moire � la fin du programme
    libererMemoireJoueurs(joueurs);
}

void initialiserSacDeJetons(struct SacDeJetons *sac) {
    // D�finir les lettres et leurs valeurs associ�es (� ajuster selon les r�gles)
    char lettres[NOMBRE_LETTRES_ALPHABET] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
    };
    int valeurs[NOMBRE_LETTRES_ALPHABET] = {
        1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 10, 1, 2, 1, 1, 3, 8, 1, 1, 1, 1, 4, 10, 10, 10, 10
    };

    // Fr�quence d'utilisation des lettres dans la langue anglaise (� ajuster)
    int occurrences[NOMBRE_LETTRES_ALPHABET] = {
        9, 2, 2, 4, 12, 2, 3, 2, 9, 1, 1, 4, 2, 6, 8, 2, 1, 6, 4, 6, 4, 2, 2, 1, 2, 1
    };

    // Initialiser le g�n�rateur de nombres al�atoires avec une graine bas�e sur le temps
    srand((unsigned int)time(NULL));

    // Remplir le sac avec les lettres et les valeurs
    int index = 0;
    for (int i = 0; i < NOMBRE_LETTRES_ALPHABET; ++i) {
        // Ajouter plusieurs occurrences de chaque lettre en fonction de leur fr�quence dans le jeu
        for (int j = 0; j < occurrences[i]; ++j) {
            sac->jetons[index].lettre = lettres[i];
            sac->jetons[index].valeur = valeurs[i];
            ++index;
        }
    }

    // M�langer les jetons dans le sac (algorithme de m�lange de Fisher-Yates)
    for (int i = NOMBRE_TOTAL_DE_JETONS - 1; i > 0; --i) {
        int j = rand() % (i + 1);

        // �changer les positions des jetons i et j
        struct Jeton temp = sac->jetons[i];
        sac->jetons[i] = sac->jetons[j];
        sac->jetons[j] = temp;
    }

    // D�finir le nombre initial de jetons restants dans le sac
    sac->nbJetonsRestants = NOMBRE_TOTAL_DE_JETONS;
}

// Fonction pour piocher des lettres depuis le sac
void piocherLettres(struct SacDeJetons *sac, struct Joueur *joueur, int nbLettres) {
    // V�rifier si le sac contient suffisamment de lettres
    if (sac->nbJetonsRestants < nbLettres) {
        printf("Le sac ne contient pas suffisamment de lettres.\n");
        return;
    }

    // Piocher le nombre sp�cifi� de lettres
    for (int i = 0; i < nbLettres; ++i) {
        // V�rifier si le sac n'est pas vide
        if (sac->nbJetonsRestants > 0) {
            // Ajouter une lettre au chevalet du joueur depuis le sac
            joueur->chevalet[i] = sac->jetons[sac->nbJetonsRestants - 1].lettre;
            // D�cr�menter le nombre de jetons restants dans le sac
            --sac->nbJetonsRestants;
        } else {
            printf("Le sac est vide. Aucune lettre disponible pour piocher.\n");
            return;
        }
    }

    // Indiquer la fin du chevalet avec un caract�re nul
    joueur->chevalet[nbLettres] = '\0';

    // Informer le joueur des lettres pioch�es
    printf("%s a pioch� les lettres : %s\n", joueur->nom, joueur->chevalet);
}


void tourDeJeu(struct SacDeJetons *sac, struct Plateau *plateau, struct Joueur *joueurs, int joueurActuel) {
    struct Joueur *joueur = &joueurs[joueurActuel];
    struct ChronometreInfo chronoInfo;
    // Affichez le nom du joueur actuel
    printf("\nTour de %s\n", joueur->nom);

    // Piocher des lettres au d�but du tour
    piocherLettres(sac, joueur, 7); // Piocher 7 lettres pour un tour



    // Affichez le chevalet du joueur
    printf("%s, voici votre chevalet : %s\n", joueur->nom, joueur->chevalet);

    demarrerChronometre(&chronoInfo);

    afficherChronometre(&chronoInfo);

    // Saisie du mot
    saisirMotPlacement(joueur);



    // V�rification si le mot est valide
    if (!motValide(joueur->dernierMot)) {
        printf("Le mot '%s' n'est pas dans le dictionnaire. Veuillez choisir un autre mot.\n", joueur->dernierMot);
        // Ajoutez ici une logique pour permettre au joueur de saisir un autre mot si n�cessaire
        // Annulez le chronom�tre lorsque le joueur a termin� son tour
        pthread_cancel(chronoInfo.threadChrono);
        return;  // Quitte la fonction si le mot n'est pas valide
    }

    // Annulez le chronom�tre lorsque le joueur a termin� son tour
    pthread_cancel(chronoInfo.threadChrono);

    // Saisie de l'emplacement et du sens du mot
    int ligne, colonne;
    saisirEmplacement(joueur, &ligne, &colonne);
    joueur->dernierLigne = ligne;
    joueur->dernierColonne = colonne;
    saisirSensMot(joueur);

      // Arr�tez le chronom�tre lorsque le joueur a termin� son tour
    arreterChronometre(&chronoInfo);


    // Affichage du mot sur le plateau
    afficherMotSurPlateau(plateau, joueur);
    afficherPlateauAvecMots(plateau); // Utilisez la nouvelle fonction pour afficher le plateau avec les mots
    afficherPlateauAvecBordures(plateau);

    // ... (d'autres �tapes du tour de jeu, comme le calcul du score, etc.)

    // Mettez � jour la variable finPartie si le joueur a atteint un score de 100
    if (joueur->score >= 100) {
        joueur->finPartie = 1;
        printf("%s a remport� la partie avec un score de %d!\n", joueur->nom, joueur->score);
    }

    // Mettez � jour la variable finPartie si le joueur a abandonn�
    if (joueur->finPartie == 0) {
        char choixAbandon;
        printf("Voulez-vous abandonner le jeu ? (O/N): ");
        scanf(" %c", &choixAbandon);

        if (choixAbandon == 'O' || choixAbandon == 'o') {
            abandonnerJeu(sac, plateau, joueur);  // Appeler la fonction d'abandon
            joueur->finPartie = 1;
            printf("%s a abandonn� la partie.\n", joueur->nom);
        }
    }
    // Ajoutez ici la logique pour passer au joueur suivant
}


void afficherScoreJoueur(const struct Joueur *joueur) {
    printf("%s, votre score actuel est : %d\n", joueur->nom, joueur->score);
}


int Menu() {
    int choixMenu;  // Ajoutez la d�claration de la variable ici

    do {
        printf("\nMenu Principal\n");
        printf("1. Nouvelle Partie\n");
        printf("2. Afficher l'aide\n");
        printf("3. Afficher les scores\n");
        printf("4. Quitter\n");
        printf("Choix : ");
        scanf("%d", &choixMenu);
        switch (choixMenu) {
            case 1:
                jouerParties();
                break;
            case 2:
                // Fonction pour afficher l'aide
                break;
            case 3:
                // Fonction pour afficher les scores
                break;
            case 4:
                printf("Merci d'avoir jou�. Au revoir!\n");
                break;
            default:
                printf("Choix invalide. Veuillez choisir une option valide.\n");
        }
    } while (choixMenu != 4);

    return 0;
}



void abandonnerJeu(struct SacDeJetons *sac, struct Plateau *plateau, struct Joueur *joueur) {
    // Affiche un message d'abandon
    printf("Vous avez abandonn� le jeu. Retour au menu principal.\n");

    // Ajoutez ici le code n�cessaire pour revenir au menu principal, par exemple, en appelant une fonction du menu.
    // Assurez-vous de lib�rer toute m�moire allou�e et de r�initialiser les structures du jeu si n�cessaire.

    // R�initialiser le chevalet du joueur
    for (int i = 0; i < NOMBRE_LETTRES_PIOCHE; ++i) {
        joueur->chevalet[i] = ' ';
    }

    // R�initialiser le dernier mot du joueur
    joueur->dernierMot[0] = '\0';

    // R�initialiser le dernier mot du joueur
    joueur->dernierLigne = -1;
    joueur->dernierColonne = -1;

    // R�initialiser le plateau
    initialiserPlateau(plateau);

    // M�langer les jetons dans le sac
    initialiserSacDeJetons(sac);

    // Retour au menu principal
    Menu();  // Assurez-vous d'ajuster cette fonction selon votre structure de menu.
}
