#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FICHIER_BANQUE "sogebank.txt"

typedef struct {
    char numero[13];
    char nom[64];
    char prenom[64];
    char date_naiss[11];
    char civilite;
    char sexe;
    double solde;
} Compte;

void generer_prochain_numero(char *dest) {
    FILE *f = fopen(FICHIER_BANQUE, "r");
    long long dernier_num = 100000000000;

    if (f != NULL) {
        char tmp_num[32], tmp_nom[64], tmp_prenom[64], date[32], civ, sexe;
        double solde;

        while (fscanf(f, "%s %s %s %s %c %c %lf", tmp_num, tmp_nom, tmp_prenom, date, &civ, &sexe, &solde) != EOF) {
            long long actuel = atoll(tmp_num);
            if (actuel > dernier_num) {
                dernier_num = actuel;
            }
        }
        fclose(f);
    }
    sprintf(dest, "%lld", dernier_num + 1);
}

void creer_compte() {
    Compte c;
    printf("\n=== OUVERTURE DE COMPTE ===\n");
    
    generer_prochain_numero(c.numero);
    printf("Numero de compte attribue : %s\n", c.numero);

    printf("Nom : ");
    scanf("%s", c.nom);
    printf("Prenom : ");
    scanf("%s", c.prenom);
    printf("Date de naissance (JJ/MM/AAAA) : ");
    scanf("%s", c.date_naiss);
    
    printf("Civilite (M, C, D, V) : ");
    scanf(" %c", &c.civilite);
    printf("Sexe (M, F) : ");
    scanf(" %c", &c.sexe);
    
    c.solde = 0.00;

    FILE *f = fopen(FICHIER_BANQUE, "a");
    if (f != NULL) {
        fprintf(f, "%s %s %s %s %c %c %.2f\n", 
                c.numero, c.nom, c.prenom, c.date_naiss, c.civilite, c.sexe, c.solde);
        fclose(f);
        printf("\n[Succes] Compte cree avec succes !\n");
    } else {
        printf("\n[Erreur] Impossible d'ouvrir le fichier.\n");
    }
}

void modifier_solde(int type_operation) {
    char cible[32];
    printf("\n=== %s UN COMPTE ===\n", type_operation == 1 ? "CREDITER" : "DEBITER");
    printf("Entrez le numero de compte : ");
    scanf("%s", cible);

    FILE *f = fopen(FICHIER_BANQUE, "r");
    if (f == NULL) {
        printf("[Erreur] Fichier introuvable.\n");
        return;
    }

    Compte comptes[100];
    int nb_comptes = 0;
    int trouve = -1;

    while (fscanf(f, "%s %s %s %s %c %c %lf", 
                  comptes[nb_comptes].numero, comptes[nb_comptes].nom, 
                  comptes[nb_comptes].prenom, comptes[nb_comptes].date_naiss, 
                  &comptes[nb_comptes].civilite, &comptes[nb_comptes].sexe, 
                  &comptes[nb_comptes].solde) != EOF) {
        if (strcmp(comptes[nb_comptes].numero, cible) == 0) {
            trouve = nb_comptes;
        }
        nb_comptes++;
    }
    fclose(f);

    if (trouve == -1) {
        printf("[Erreur] Compte %s introuvable.\n", cible);
        return;
    }

    double montant;
    printf("Client : %s %s (Solde actuel : %.2f FCFA)\n", comptes[trouve].nom, comptes[trouve].prenom, comptes[trouve].solde);
    printf("Entrez le montant : ");
    scanf("%lf", &montant);

    if (type_operation == 1) {
        comptes[trouve].solde += montant;
    } else {
        if (comptes[trouve].solde >= montant) {
            comptes[trouve].solde -= montant;
        } else {
            printf("[Erreur] Solde insuffisant.\n");
            return;
        }
    }

    f = fopen(FICHIER_BANQUE, "w");
    for (int i = 0; i < nb_comptes; i++) {
        fprintf(f, "%s %s %s %s %c %c %.2f\n", 
                comptes[i].numero, comptes[i].nom, comptes[i].prenom, 
                comptes[i].date_naiss, comptes[i].civilite, comptes[i].sexe, comptes[i].solde);
    }
    fclose(f);
    printf("[Succes] Operation effectuee. Nouveau solde : %.2f FCFA\n", comptes[trouve].solde);
}

void consulter_solde() {
    char cible[32];
    printf("\n=== CONSULTATION DE SOLDE ===\n");
    printf("Entrez le numero de compte : ");
    scanf("%s", cible);

    FILE *f = fopen(FICHIER_BANQUE, "r");
    if (f == NULL) {
        printf("[Erreur] Aucun compte enregistre.\n");
        return;
    }

    char tmp_num[32], tmp_nom[64], tmp_prenom[64], date[32], civ, sexe;
    double solde;
    int trouve = 0;

    while (fscanf(f, "%s %s %s %s %c %c %lf", tmp_num, tmp_nom, tmp_prenom, date, &civ, &sexe, &solde) != EOF) {
        if (strcmp(tmp_num, cible) == 0) {
            printf("\n-----------------------------------\n");
            printf("Titulaire : %s %s\n", tmp_nom, tmp_prenom);
            printf("Solde Actuel : %.2f FCFA\n", solde);
            printf("-----------------------------------\n");
            trouve = 1;
            break;
        }
    }
    fclose(f);

    if (!trouve) {
        printf("[Erreur] Compte introuvable.\n");
    }
}

void supprimer_compte() {
    char cible[32];
    printf("\n=== SUPPRESSION DE COMPTE ===\n");
    printf("Entrez le numero de compte a fermer : ");
    scanf("%s", cible);

    FILE *f = fopen(FICHIER_BANQUE, "r");
    if (f == NULL) {
        printf("[Erreur] Fichier introuvable.\n");
        return;
    }

    Compte comptes[100];
    int nb_comptes = 0;
    int trouve = 0;

    while (fscanf(f, "%s %s %s %s %c %c %lf", 
                  comptes[nb_comptes].numero, comptes[nb_comptes].nom, 
                  comptes[nb_comptes].prenom, comptes[nb_comptes].date_naiss, 
                  &comptes[nb_comptes].civilite, &comptes[nb_comptes].sexe, 
                  &comptes[nb_comptes].solde) != EOF) {
        if (strcmp(comptes[nb_comptes].numero, cible) == 0) {
            trouve = 1;
            continue; 
        }
        nb_comptes++;
    }
    fclose(f);

    if (!trouve) {
        printf("[Erreur] Aucun compte ne correspond a ce numero.\n");
        return;
    }

    f = fopen(FICHIER_BANQUE, "w");
    for (int i = 0; i < nb_comptes; i++) {
        fprintf(f, "%s %s %s %s %c %c %.2f\n", 
                comptes[i].numero, comptes[i].nom, comptes[i].prenom, 
                comptes[i].date_naiss, comptes[i].civilite, comptes[i].sexe, comptes[i].solde);
    }
    fclose(f);
    printf("[Succes] Le compte %s a bien ete supprime.\n", cible);
}

void afficher_tous_les_clients() {
    FILE *f = fopen(FICHIER_BANQUE, "r");
    
    printf("\n=== LISTE DES CLIENTS SOGEBANK ===\n");
    printf("%-15s %-12s %-12s %-12s %-5s %-5s %-10s\n", "NUMERO", "NOM", "PRENOM", "DATE_NAISS", "CIV", "SEX", "SOLDE");
    printf("------------------------------------------------------------------------------\n");

    if (f != NULL) {
        char tmp_num[32], tmp_nom[64], tmp_prenom[64], date[32], civ, sexe;
        double solde;

        while (fscanf(f, "%s %s %s %s %c %c %lf", tmp_num, tmp_nom, tmp_prenom, date, &civ, &sexe, &solde) != EOF) {
            printf("%-15s %-12s %-12s %-12s %-5c %-5c %.2f FCFA\n", tmp_num, tmp_nom, tmp_prenom, date, civ, sexe, solde);
        }
        fclose(f);
    } else {
        printf("Aucun historique disponible.\n");
    }
    printf("------------------------------------------------------------------------------\n");
}

int main(void) {
    int choix;
    
    do {
        printf("\n--- SYSTEME SOGEBANK CLINIQUE ---\n");
        printf("1. Creer un compte (Numero automatique)\n");
        printf("2. Crediter un compte\n");
        printf("3. Debiter un compte\n");
        printf("4. Consulter le solde d'un client\n");
        printf("5. Liste de tous les clients\n");
        printf("6. Supprimer un compte\n");
        printf("7. Quitter\n");
        printf("Votre choix : ");
        scanf("%d", &choix);
        
        switch(choix) {
            case 1: creer_compte(); break;
            case 2: modifier_solde(1); break;
            case 3: modifier_solde(2); break;
            case 4: consulter_solde(); break;
            case 5: afficher_tous_les_clients(); break;
            case 6: supprimer_compte(); break;
            case 7: printf("Deconnexion de Sogebank.\n"); break;
            default: printf("Option invalide.\n");
        }
    } while (choix != 7);

    return 0;
}