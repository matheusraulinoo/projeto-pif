#ifndef TELAS_H
#define TELAS_H

#include "raylib.h"
#include "config.h"

// Estrelas do céu da tela de início
void IniciarEstrelas(void);

// Tela de início (menu) e tela de morte
void DesenhaTelaInicio(float tempo, Texture2D texturaPersonagem);
void DesenhaTelaMorte(float tempoMorte, float tempoRestante, int dia);

// Campo para digitar o nome na tela de morte
void AtualizarCampoNome(char *nome, int *contador);
void DesenhaCaixaNome(const char *nome, int contador);

// Utilitário de texto usado em várias telas
void DesenhaTextoCentralizado(const char *texto, float y, float tamanho,
                              float espacamento, Color cor);

// Ajuste da janela para a resolução interna do jogo
Rectangle AjustarTela(void);
void ApresentarTela(RenderTexture2D tela, Rectangle destino);

#endif
