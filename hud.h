#ifndef HUD_H
#define HUD_H

#include "raylib.h"
#include "config.h"
#include "entities.h"
#include "inventory.h"

// Corações de vida no canto da tela
void DesenhaVida(float vida);

// Painel de interação com a lava
void DesenhaPainelLava(Inventario *inv, const char *mensagem, Color corMensagem, float tempoMensagem);

// Slot visual do escudo equipado (com barra de durabilidade)
void DesenhaEscudoEquipado(Slot escudoEquipado);

// Inventário completo aberto (fundo, dica e destaque do slot sob o mouse)
void DesenhaInventarioAberto(Inventario *inv, int invX, int invY, int origemInv);

// Barra de vida de árvores/minérios (apenas quando danificados)
void DesenhaBarraVidaRecurso(Recursos *a);

#endif
