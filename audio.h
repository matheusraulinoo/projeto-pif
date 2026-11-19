#ifndef AUDIO_H
#define AUDIO_H

#include "raylib.h"

extern Sound sndAmanhecer;
extern Sound sndAndarCasa;
extern Sound sndAndarIlha;
extern Sound sndAndarCaverna;
extern Sound sndApanhar;
extern Sound sndAtingiu;
extern Sound sndBater;
extern Sound sndCraft;
extern Sound sndEntrarCasa;
extern Sound sndEntrarCaverna;
extern Sound sndEscudo;
extern Sound sndMorrer;
extern Sound sndSelecionar;

void Audio_Init(void);
void Audio_Unload(void);

// Toca o som se o dispositivo de áudio estiver pronto
void TocarSom(Sound som);

#endif
