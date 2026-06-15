#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FICHIER_BANQUE "sogebank.txt"
static const char *s_http_addr = "http://127.0.0.1:8080";

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

static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
            mg_http_reply(c, 204, 
                "Access-Control-Allow-Origin: *\r\n"
                "Access-Control-Allow-Headers: Content-Type\r\n"
                "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n", "");
            return;
        }

        if (mg_match_pattern("/", hm->uri)) {
            struct mg_http_serve_opts opts = {.root_dir = "."};
            mg_http_serve_dir(c, hm, &opts);
            return;
        } 
        
        else if (mg_match_pattern("/api/creer", hm->uri) && mg_strcmp(hm->method, mg_str("POST")) == 0) {
            char nom_s[64] = {0}, prenom_s[64] = {0}, date_s[32] = {0}, civ_s[4] = {0}, sexe_s[4] = {0};
            
            struct mg_str s_nom = mg_json_get_str(hm->body, "$.nom");
            struct mg_str s_prenom = mg_json_get_str(hm->body, "$.prenom");
            struct mg_str s_date = mg_json_get_str(hm->body, "$.date_naiss");
            struct mg_str s_civ = mg_json_get_str(hm->body, "$.civilite");
            struct mg_str s_sexe = mg_json_get_str(hm->body, "$.sexe");

            if (s_nom.len > 0) snprintf(nom_s, sizeof(nom_s), "%.*s", (int)s_nom.len, s_nom.ptr);
            if (s_prenom.len > 0) snprintf(prenom_s, sizeof(prenom_s), "%.*s", (int)s_prenom.len, s_prenom.ptr);
            if (s_date.len > 0) snprintf(date_s, sizeof(date_s), "%.*s", (int)s_date.len, s_date.ptr);
            if (s_civ.len > 0) snprintf(civ_s, sizeof(civ_s), "%.*s", (int)s_civ.len, s_civ.ptr);
            if (s_sexe.len > 0) snprintf(sexe_s, sizeof(sexe_s), "%.*s", (int)s_sexe.len, s_sexe.ptr);

            char nouvel_id[32];
            generer_prochain_numero(nouvel_id);

            FILE *f = fopen(FICHIER_BANQUE, "a");
            if (f != NULL) {
                fprintf(f, "%s %s %s %s %c %c 0.00\n", nouvel_id, nom_s, prenom_s, date_s, civ_s[0], sexe_s[0]);
                fclose(f);
                
                char msg[128];
                sprintf(msg, "Compte cree avec succes ! Numero : %s", nouvel_id);
                mg_http_reply(c, 200, 
                    "Content-Type: text/plain; charset=utf-8\r\n"
                    "Access-Control-Allow-Origin: *\r\n", 
                    "%s", msg);
            } else {
                mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n", "Erreur fichier.");
            }
        } 
        
        else if (mg_match_pattern("/api/transaction", hm->uri) && mg_strcmp(hm->method, mg_str("POST")) == 0) {
            char cible[32] = {0}, type_s[16] = {0};
            double montant = 0;

            struct mg_str s_cible = mg_json_get_str(hm->body, "$.numero");
            struct mg_str s_type = mg_json_get_str(hm->body, "$.type");
            mg_json_get_num(hm->body, "$.montant", &montant);

            if (s_cible.len > 0) snprintf(cible, sizeof(cible), "%.*s", (int)s_cible.len, s_cible.ptr);
            if (s_type.len > 0) snprintf(type_s, sizeof(type_s), "%.*s", (int)s_type.len, s_type.ptr);

            FILE *f = fopen(FICHIER_BANQUE, "r");
            Compte comptes[100];
            int nb = 0, trouve = -1;

            if (f != NULL) {
                while (fscanf(f, "%s %s %s %s %c %c %lf", comptes[nb].numero, comptes[nb].nom, comptes[nb].prenom, comptes[nb].date_naiss, &comptes[nb].civilite, &comptes[nb].sexe, &comptes[nb].solde) != EOF) {
                    if (strcmp(comptes[nb].numero, cible) == 0) trouve = nb;
                    nb++;
                }
                fclose(f);
            }

            if (trouve == -1) {
                mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n", "Compte introuvable.");
            } else {
                if (strcmp(type_s, "credit") == 0) {
                    comptes[trouve].solde += montant;
                } else {
                    if (comptes[trouve].solde < montant) {
                        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n", "Solde insuffisant.");
                        return;
                    }
                    comptes[trouve].solde -= montant;
                }

                f = fopen(FICHIER_BANQUE, "w");
                for(int i = 0; i < nb; i++) {
                    fprintf(f, "%s %s %s %s %c %c %.2f\n", comptes[i].numero, comptes[i].nom, comptes[i].prenom, comptes[i].date_naiss, comptes[i].civilite, comptes[i].sexe, comptes[i].solde);
                }
                fclose(f);
                mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n", "Operation reussie !");
            }
        }

        else if (mg_match_pattern("/api/supprimer", hm->uri) && mg_strcmp(hm->method, mg_str("POST")) == 0) {
            char cible[32] = {0};
            struct mg_str s_cible = mg_json_get_str(hm->body, "$.numero");
            if (s_cible.len > 0) snprintf(cible, sizeof(cible), "%.*s", (int)s_cible.len, s_cible.ptr);

            FILE *f = fopen(FICHIER_BANQUE, "r");
            Compte comptes[100];
            int nb = 0, trouve = 0;

            if (f != NULL) {
                while (fscanf(f, "%s %s %s %s %c %c %lf", comptes[nb].numero, comptes[nb].nom, comptes[nb].prenom, comptes[nb].date_naiss, &comptes[nb].civilite, &comptes[nb].sexe, &comptes[nb].solde) != EOF) {
                    if (strcmp(comptes[nb].numero, cible) == 0) {
                        trouve = 1;
                        continue;
                    }
                    nb++;
                }
                fclose(f);
            }

            if (trouve) {
                f = fopen(FICHIER_BANQUE, "w");
                for(int i = 0; i < nb; i++) {
                    fprintf(f, "%s %s %s %s %c %c %.2f\n", comptes[i].numero, comptes[i].nom, comptes[i].prenom, comptes[i].date_naiss, comptes[i].civilite, comptes[i].sexe, comptes[i].solde);
                }
                fclose(f);
                mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n", "Compte supprime.");
            } else {
                mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n", "Compte introuvable.");
            }
        }
        
        else if (mg_match_pattern("/api/clients", hm->uri) && mg_strcmp(hm->method, mg_str("GET")) == 0) {
            FILE *f = fopen(FICHIER_BANQUE, "r");
            char json[8192] = "[";
            
            if (f != NULL) {
                char tmp_num[32], tmp_nom[64], tmp_prenom[64], date[32], civ, sexe;
                double solde;
                int premier = 1;

                while (fscanf(f, "%s %s %s %s %c %c %lf", tmp_num, tmp_nom, tmp_prenom, date, &civ, &sexe, &solde) != EOF) {
                    char item[512];
                    if (!premier) strcat(json, ",");
                    sprintf(item, "{\"numero\":\"%s\",\"nom\":\"%s\",\"prenom\":\"%s\",\"date\":\"%s\",\"civ\":\"%c\",\"sexe\":\"%c\",\"solde\":%.2f}", tmp_num, tmp_nom, tmp_prenom, date, civ, sexe, solde);
                    strcat(json, item);
                    premier = 0;
                }
                fclose(f);
            }
            strcat(json, "]");
            mg_http_reply(c, 200, 
                "Access-Control-Allow-Origin: *\r\n"
                "Content-Type: application/json\r\n", 
                "%s", json);
        } 
        else {
            mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type: text/plain\r\n", "Non trouve");
        }
    }
}

int main(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    printf("Serveur Sogebank en cours d'execution sur le port 8080...\n");
    if (mg_http_listen(&mgr, s_http_addr, fn, NULL) == NULL) return 1;
    for (;;) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
    return 0;
}