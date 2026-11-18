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
        if (fscanf(f, " %31[^;];%d", ranking[i].nome, &ranking[i].pontuacao) != 2) {
            break;
        }
    }
    fclose(f);
}

void SalvarRanking(EntradaRanking ranking[MAX_RANKING]) {
    FILE *f = fopen("ranking.txt", "w");
    if (f == NULL) return;

    for (int i = 0; i < MAX_RANKING; i++) {
        fprintf(f, "%s;%d\n", ranking[i].nome, ranking[i].pontuacao);
    }
    fclose(f);
}

void AtualizarRanking(EntradaRanking ranking[MAX_RANKING], const char *nome, int pontuacao) {
    EntradaRanking temp[MAX_RANKING + 1];

    for (int i = 0; i < MAX_RANKING; i++) {
        temp[i] = ranking[i];
    }
    snprintf(temp[MAX_RANKING].nome, sizeof(temp[MAX_RANKING].nome), "%s", nome);
    temp[MAX_RANKING].pontuacao = pontuacao;

    for (int i = 0; i < MAX_RANKING + 1; i++) {
        for (int j = i + 1; j < MAX_RANKING + 1; j++) {
            if (temp[j].pontuacao > temp[i].pontuacao) {
                EntradaRanking aux = temp[i];
                temp[i] = temp[j];
                temp[j] = aux;
            }
        }
    }

    for (int i = 0; i < MAX_RANKING; i++) {
        ranking[i] = temp[i];
    }

    SalvarRanking(ranking);
}

void DesenharRankingUI(EntradaRanking ranking[MAX_RANKING], int startX, int startY) {
    DrawRectangle(startX - 10, startY - 10, 260, 180, Fade(BLACK, 0.7f));
    DrawRectangleLines(startX - 10, startY - 10, 260, 180, LIGHTGRAY);

    DrawText("=== MELHORES JOGADORES ===", startX, startY, 16, YELLOW);

    for (int i = 0; i < MAX_RANKING; i++) {
        char linha[64];
        snprintf(linha, sizeof(linha), "%d. %-12s %d dia", i + 1, ranking[i].nome, ranking[i].pontuacao);

        Color cor = (i == 0) ? GOLD : WHITE;
        DrawText(linha, startX, startY + 30 + (i * 25), 16, cor);
    }
}
