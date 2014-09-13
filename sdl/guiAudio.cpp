/*****************************************************************************
  BeagleSNES - Super Nintendo Entertainment System (TM) emulator for the
  BeagleBoard-xM platform.

  See CREDITS file to find the copyright owners of this file.

  GUI front-end code (c) Copyright 2013 Andrew Henderson (hendersa@icculus.org)

  BeagleSNES homepage: http://www.beaglesnes.org/
  
  BeagleSNES is derived from the Snes9x open source emulator:  
  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute BeagleSNES in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  BeagleSNES is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for BeagleSNES or software derived 
  from BeagleSNES, including BeagleSNES or derivatives in commercial game 
  bundles, and/or using BeagleSNES as a promotion for your commercial 
  product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 *****************************************************************************/

#include <SDL.h>
#include <SDL/SDL_mixer.h>
#include "gui.h"

static Mix_Music *music = NULL;
static Mix_Chunk *overlaySnd = NULL;
static Mix_Chunk *selectSnd = NULL;

void initAudio(void) {
  if (!audioAvailable) return;

  if (Mix_OpenAudio(32000, AUDIO_S16SYS, 2, /*4096*/ 2048)) {
  //if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 8192)) {

    fprintf(stderr, "Unable to open audio!\n");
    audioAvailable = 0;
    return;
  }

  music = Mix_LoadMUS("snd/virility.wav");
  if (!music) fprintf(stderr, "Unable to load music\n");

  overlaySnd = Mix_LoadWAV("snd/coin.wav");
  if (!overlaySnd) fprintf(stderr, "Unable to load overlay sfx\n");

  selectSnd = Mix_LoadWAV("snd/1up.wav");
  if (!selectSnd) fprintf(stderr, "Unable to load select sfx\n");
  changeVolume();
  volumeOverlayCount = 0;
  Mix_FadeInMusic(music, -1, 2000);
}

void changeVolume(void) {
  if (!audioAvailable) return;
  if (music) Mix_VolumeMusic(currentVolume >> 1);
  if (overlaySnd) Mix_VolumeChunk(overlaySnd, currentVolume);
  if (selectSnd) Mix_VolumeChunk(selectSnd, currentVolume);
  volumeOverlayCount = 60;
}

void playOverlaySnd(void) {
  if (!audioAvailable) return;
  Mix_PlayChannel(-1, overlaySnd, 0);
}

void playSelectSnd(void) {
  if (!audioAvailable) return;
  Mix_PlayChannel(-1, selectSnd, 0);
}

void fadeAudio(void) {
  if (!audioAvailable) return;
  Mix_FadeOutChannel(-1, 1000);
  Mix_FadeOutMusic(1000);
}

