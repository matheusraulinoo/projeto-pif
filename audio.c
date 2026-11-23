#include "audio.h"

void Audio_Init(void)
{
    InitAudioDevice();

    sndAmanhecer   = LoadSound("assets/sounds/amanhecer.wav");
    sndAndarCasa   = LoadSound("assets/sounds/andar_casa.wav");
    sndAndarIlha   = LoadSound("assets/sounds/andar_ilha.wav");
    sndAndarCaverna = LoadSound("assets/sounds/andar_pedra.wav");
    sndApanhar     = LoadSound("assets/sounds/apanhar.wav");
    sndAtingiu     = LoadSound("assets/sounds/atingiu.wav");
    sndBater       = LoadSound("assets/sounds/bater.wav");
    sndCraft       = LoadSound("assets/sounds/craft.wav");
    sndEntrarCasa  = LoadSound("assets/sounds/entrar_casa.wav");
    sndEntrarCaverna  = LoadSound("assets/sounds/entrar_caverna.wav");
    sndEscudo      = LoadSound("assets/sounds/escudo.wav");
    sndMorrer      = LoadSound("assets/sounds/morrer.wav");
    sndSelecionar  = LoadSound("assets/sounds/selecionar.wav");

    // Ajustes simples de volume
    SetSoundVolume(sndAndarCasa, 0.35f);
    SetSoundVolume(sndAndarIlha, 0.35f);
    SetSoundVolume(sndAndarCaverna, 0.35f);
    SetSoundVolume(sndAmanhecer, 0.7f);
}

void Audio_Unload(void)
{
    UnloadSound(sndAmanhecer);
    UnloadSound(sndAndarCasa);
    UnloadSound(sndAndarIlha);
    UnloadSound(sndAndarCaverna);
    UnloadSound(sndApanhar);
    UnloadSound(sndAtingiu);
    UnloadSound(sndBater);
    UnloadSound(sndCraft);
    UnloadSound(sndEntrarCasa);
    UnloadSound(sndEntrarCaverna);
    UnloadSound(sndEscudo);
    UnloadSound(sndMorrer);
    UnloadSound(sndSelecionar);

    CloseAudioDevice();
}

void TocarSom(Sound som)
{
    if (IsAudioDeviceReady()) PlaySound(som);
}
