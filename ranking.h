#ifndef RANKING_H
#define RANKING_H

#define MAX_RANKING 5

typedef struct {
    char nome[32];
    int pontuacao;
} EntradaRanking;

void CarregarRanking(EntradaRanking ranking[MAX_RANKING]);
void SalvarRanking(EntradaRanking ranking[MAX_RANKING]);
void AtualizarRanking(EntradaRanking ranking[MAX_RANKING], const char *nome, int pontuacao);
void DesenharRankingUI(EntradaRanking ranking[MAX_RANKING], int startX, int startY);

#endif
