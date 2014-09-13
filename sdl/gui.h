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

#ifndef __GUI_H__
#define __GUI_H__

#include "beagleboard.h"

struct SDL_Surface;

#define NUM_JOYSTICKS 2

/* Map of js0, js1, etc. to the proper joystick (-1 if no joystick) */
extern int beagleSNESDeviceMap[NUM_JOYSTICKS];
void beagleSNESResetJoysticks(void);
void beagleSNESCheckJoysticks(void);
extern int beagleSNESControllerPresent[NUM_JOYSTICKS];

extern int doGui(void);
extern void loadGameInfo(void);
extern void loadGameList(void);
extern void loadInstruct(void);
extern void loadAudio(void);

/* guiParser.c */
extern int loadGameConfig(void);

extern void renderInstruct(SDL_Surface *screen, int gamepadPresent);
extern void renderGameList(SDL_Surface *screen);
extern void renderGameInfo(SDL_Surface *screen, int i);
#if defined (CAPE_LCD3)
extern void renderVolume(SDL_Surface *surface);
#endif /* CAPE_LCD3 */
extern int currentSelectedGameIndex(void);
extern void incrementGameListFrame(void);
extern void shiftSelectedGameUp(void);
extern void shiftSelectedGameDown(void);
extern void fadeAudio(void);
extern void playSelectSnd(void);
extern void playOverlaySnd(void);
extern void changeVolume(void);
extern int acceptButton(void);

#define DEFAULT_BOX_IMAGE "box_image.png"
#define DEFAULT_DATE_TEXT "19XX"
#define MAX_GENRE_TYPES 2
#define MAX_TEXT_LINES 5

#define GAME_TITLE_SIZE 64
#define ROM_FILE_SIZE 128
#define IMAGE_FILE_SIZE 128
#define INFO_TEXT_SIZE 64
#define DATE_TEXT_SIZE 5
#define GENRE_TEXT_SIZE 32

/* Linked list node for game information */
typedef struct _gameInfo {
  /* The name of the game */
  char gameTitle[GAME_TITLE_SIZE];
  /* Filename of the ROM image */
  char romFile[ROM_FILE_SIZE];
  /* Filename of the image of the game's box */
  char imageFile[IMAGE_FILE_SIZE];
  /* Lines of text that describe the game */
  char infoText[MAX_TEXT_LINES][INFO_TEXT_SIZE];
  /* Four digit year the game was released */
  char dateText[DATE_TEXT_SIZE];
  /* Short descriptive genre text */
  char genreText[MAX_GENRE_TYPES][GENRE_TEXT_SIZE];
  /* Link to next game in list */
  struct _gameInfo *next;
  /* Link to prev game in list */
  struct _gameInfo *prev;
} gameInfo_t;

#if 0 // AWH 
typedef struct {
  char gameTitle[64]; /* The name of the game */
  char romFile[128]; /* Filename of the ROM image */
  char boxImage[128]; /* Filename of the image of the game's box */
  char infoText[5][128]; /* Lines of text that describe the game */
  char dateText[32]; /* Four digit year the game was released */
  char genreText[64]; /* Short descriptive genre text */
} gameInfo_t;
#endif // AWH

extern gameInfo_t *gameInfo;
extern int totalGames;
extern int audioAvailable;

extern int currentVolume;
extern int volumePressDirection;
extern int volumeOverlayCount;

extern void shiftSelectedVolumeUp(void);
extern void shiftSelectedVolumeDown(void);

extern int selectButtonNum;
extern int startButtonNum;
#endif /* __GUI_H__ */

