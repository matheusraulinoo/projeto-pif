#include <stdio.h>

#define MAX_RANKING 5

typedef struct {
    char nome[32];
    int pontuacao;
} EntradaRanking;

void CarregarRanking(EntradaRanking ranking[MAX_RANKING]) {
    for (int i = 0; i < MAX_RANKING; i++) {
        snprintf(ranking[i].nome, sizeof(ranking[i].nome), "---");
        ranking[i].pontuacao = 0;
    }

    FILE *f = fopen("ranking.txt", "r");
    if (f == NULL) return;

    for (int i = 0; i < MAX_RANKING; i++) {
        if (fscanf(f, "%31s %d", ranking[i].nome, &ranking[i].pontuacao) != 2) {
            break;
        }
    }
    fclose(f);
}
void SalvarRanking(EntradaRanking ranking[MAX_RANKING]) {
    FILE *f = fopen("ranking.txt", "w");
    if (f == NULL) return;

    for (int i = 0; i < MAX_RANKING; i++) {
        fprintf(f, "%s %d\n", ranking[i].nome, ranking[i].pontuacao);
    }
    fclose(f);
}