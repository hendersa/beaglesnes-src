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
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "gui.h"

#if defined(BEAGLEBONE_BLACK) && !defined(CAPE_LCD3)
#define X_POS 40
#else
#define X_POS 0
#endif

#if defined(CAPE_LCD3)
#define Y_POS 15
#define MAX_GAMES_PER_SCREEN 5
#else
#define Y_POS 95
#define MAX_GAMES_PER_SCREEN 6
#endif

#if defined(CAPE_LCD3)
#define TRACK_HEIGHT 187
#else
#define TRACK_HEIGHT 232
#endif

static SDL_Surface *selectOverlay = NULL;
static SDL_Surface **itemText;
static SDL_Surface *headerText;

static TTF_Font *itemListFont;
static TTF_Font *headerTextFont;
static SDL_Rect headerTextDstRect = {X_POS+30, Y_POS, 0, 0};
static SDL_Color itemTextColor={255,255,255};
static SDL_Color headerTextColor = {155, 152, 152};
#if defined(CAPE_LCD3)
static SDL_Rect overlay1DstRect={X_POS, 36, 0, 0};
#else
static SDL_Rect overlay1DstRect={X_POS - 40, 36, 0, 0};
#endif
static SDL_Rect itemTextDstRect={X_POS+30, 0, 0, 0};
static SDL_Rect lineRect={X_POS - 40, 0, 340, 1};
#if defined(CAPE_LCD3)
static SDL_Rect trackRect={X_POS + 305, Y_POS + 37, 1, TRACK_HEIGHT};
#else
static SDL_Rect trackRect={X_POS - 40 + 325, Y_POS + 37, 1, TRACK_HEIGHT}; 
#endif
static int overlayFrame = 0;
#if defined(BEAGLEBONE_BLACK)
#if defined(CAPE_LCD3)
static SDL_Rect backgroundRect = {0, 50, 320, 240-50};
#else
static SDL_Rect backgroundRect = {0, 80, 350, 480-80};
#endif /* CAPE_LCD3 */
#else
static SDL_Rect backgroundRect = {0, 80, 310, 480-80};
#endif
static int currentIndex = 0; // Selected index in the list
static int topIndex = 0; // Index displayed at the top of the list
static int nextIndex = 0;
static int shiftFrame = 0;
static int shiftFrameFactor = 1; //3;
static SDL_Surface *itemListSurface;
static SDL_Rect itemListSurfaceRect = {0, 0, 0, 0};
static SDL_Surface *thumbOrig = NULL;
static SDL_Surface *thumb = NULL;
static SDL_Rect thumbDstRect = {0, 0, 0, 0};

void loadGameList(void)
{
  int i = 0, thumbHeight = 0;
  SDL_Rect thumbSrcRect = {0, 0, 0, 0};
  gameInfo_t *currentNode = gameInfo;

  itemListFont = TTF_OpenFont("fonts/FreeSans.ttf", 25);
  headerTextFont = TTF_OpenFont("fonts/FreeSansBold.ttf", 25);
  headerText = TTF_RenderText_Blended(headerTextFont, "Game List", headerTextColor);
  itemText = (SDL_Surface **)calloc(totalGames, sizeof(SDL_Surface *));
  for (i=0; i < totalGames; i++) {
    currentNode = currentNode->next;   
    itemText[i] = TTF_RenderText_Blended(itemListFont, currentNode->gameTitle, itemTextColor);
  }
  selectOverlay = IMG_Load("gfx/overlay_bar.png");
  itemListSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 290, 8*(selectOverlay->h - 8), 16, 0x7C00, 0x03E0, 0x001F, 0);  

  if (totalGames <= MAX_GAMES_PER_SCREEN) return;

  /* Construct the thumb for the slider */
  thumbOrig = IMG_Load("gfx/thumb.png");

  /* Figure out how tall the thumb must be */
  thumbHeight = (TRACK_HEIGHT - 4) - ((totalGames - MAX_GAMES_PER_SCREEN) / (float)totalGames) * (TRACK_HEIGHT - 4);
  if (thumbHeight < 14) thumbHeight = 14;
  thumb = SDL_CreateRGBSurface(SDL_SWSURFACE, thumbOrig->w, thumbHeight, 16, 0x7C00, 0x03E0, 0x001F, 0);
  /* Top of thumb */
  SDL_BlitSurface(thumbOrig, NULL, thumb, NULL);
  /* Bottom of thumb */ 
  thumbSrcRect.w = thumbOrig->w;
  thumbSrcRect.h = 7;
  thumbSrcRect.y = 8;
  thumbDstRect.y = thumbHeight - thumbSrcRect.h;
  SDL_BlitSurface(thumbOrig, &thumbSrcRect, thumb, &thumbDstRect);
  /* Thumb slices */
  thumbSrcRect.y = 7;
  thumbSrcRect.h = 1;
  for (i=0; i<(thumbHeight-14); i++) {
    thumbDstRect.y = i+7;
    SDL_BlitSurface(thumbOrig, &thumbSrcRect, thumb, &thumbDstRect);
  }
  thumbDstRect.x = trackRect.x - 7;
  thumbDstRect.y = trackRect.y;
  SDL_SetColorKey(thumb, SDL_SRCCOLORKEY, 0x7C1F);
}

void renderThumb(SDL_Surface *screen) {
  //SDL_FillRect(screen, &trackRect, 0xFFFF);
  thumbDstRect.y = trackRect.y + 2 + ((topIndex / (float) (totalGames-MAX_GAMES_PER_SCREEN)) * (TRACK_HEIGHT - 4 - thumb->h));

  // If the text list isn't scrolling...
  if ( (nextIndex == currentIndex) ||
    ((nextIndex >= topIndex) && (nextIndex < currentIndex)) ||
    ((nextIndex <= (topIndex + (MAX_GAMES_PER_SCREEN-1))) && (nextIndex > currentIndex)))
  {
    // Nothing
  // Text is scrolling up...
  } else if (nextIndex > currentIndex) {
    thumbDstRect.y += (((shiftFrame / (float)(selectOverlay->h - 8)) / (float) (totalGames-MAX_GAMES_PER_SCREEN)) * (TRACK_HEIGHT - 4 - thumb->h));
  // Text is scrolling down...
  } else if (nextIndex < currentIndex) {
    thumbDstRect.y -= (((shiftFrame / (float)(selectOverlay->h - 8)) / (float) (totalGames-MAX_GAMES_PER_SCREEN)) * (TRACK_HEIGHT - 4 - thumb->h));
  }
  SDL_BlitSurface(thumb, NULL, screen, &thumbDstRect);
}

void renderGameList(SDL_Surface *screen) {
  int i, vertical_offset, frameOffset;
  SDL_Rect clip_rect = {0, 0, 0, itemTextDstRect.h};

  SDL_FillRect(screen, &backgroundRect, 0x0);
#if !defined(CAPE_LCD3)
  SDL_BlitSurface(headerText, NULL, screen, &headerTextDstRect);
#endif /* CAPE_LCD3 */

#if defined(BEAGLEBONE_BLACK)
  overlay1DstRect.x = X_POS - 40;
#else
  overlay1DstRect.x = X_POS;
#endif

#if !defined(CAPE_LCD3)
  lineRect.y = Y_POS + 35;
  SDL_FillRect(screen, &lineRect, 0xFFFF);
  lineRect.y = Y_POS + 270;
  SDL_FillRect(screen, &lineRect, 0xFFFF);
#endif 
  // First case: all games fit on a single screen
  if (totalGames <= MAX_GAMES_PER_SCREEN) {
    vertical_offset = (MAX_GAMES_PER_SCREEN-totalGames) * (selectOverlay->h - 8) / 2;
    for (i=0; i < totalGames; i++) {
      itemTextDstRect.y = (Y_POS + 60-15+vertical_offset) + i*(selectOverlay->h - 8);
      SDL_BlitSurface(itemText[i], NULL, screen, &itemTextDstRect);
    }
    overlay1DstRect.y = (Y_POS + 55-15+vertical_offset) + currentIndex*(selectOverlay->h - 8);

    /* Are we shifting down? */
    if (currentIndex < nextIndex)
      overlay1DstRect.y += shiftFrame;
    /* Are we shifting up? */
    if (currentIndex > nextIndex)
      overlay1DstRect.y -= shiftFrame;

    SDL_BlitSurface(selectOverlay, NULL, screen, &overlay1DstRect);
  }
  // Second case: > MAX_GAMES_PER_SCREEN titles, nothing is moving
  else if (nextIndex == currentIndex) {
    for (i=0; i < MAX_GAMES_PER_SCREEN; i++) {
      itemTextDstRect.y = (Y_POS + 60-15) + i*(selectOverlay->h - 8);
      SDL_BlitSurface(itemText[i + topIndex], NULL, screen, &itemTextDstRect);
    }
    overlay1DstRect.y = (Y_POS + 55-15) + (currentIndex - topIndex)*(selectOverlay->h - 8);
    SDL_BlitSurface(selectOverlay, NULL, screen, &overlay1DstRect);
    renderThumb(screen);
  }
  // Third case: > MAX_GAMES_PER_SCREEN titles, highlight bar moves
  else if 
    (((nextIndex >= topIndex) && (nextIndex < currentIndex)) ||
    ((nextIndex <= (topIndex + (MAX_GAMES_PER_SCREEN-1))) && (nextIndex > currentIndex)))  
  {
    for (i=0; i < MAX_GAMES_PER_SCREEN; i++) {
      itemTextDstRect.y = (Y_POS + 60-15) + i*(selectOverlay->h - 8);
      SDL_BlitSurface(itemText[i + topIndex], NULL, screen, &itemTextDstRect);
    }
    overlay1DstRect.y = (Y_POS + 55-15) + (currentIndex-topIndex)*(selectOverlay->h - 8);

    /* Are we shifting down? */
    if (currentIndex < nextIndex)
      overlay1DstRect.y += shiftFrame;
    /* Are we shifting up? */
    if (currentIndex > nextIndex)
      overlay1DstRect.y -= shiftFrame;

    SDL_BlitSurface(selectOverlay, NULL, screen, &overlay1DstRect);
    renderThumb(screen);
  }
  // Fourth case: > MAX_GAMES_PER_SCREEN titles, text scrolls down
  else if ((currentIndex == topIndex) && (nextIndex < currentIndex)) {
    clip_rect.w = itemListSurface->w;
    clip_rect.h = (selectOverlay->h - 8) * MAX_GAMES_PER_SCREEN;
    clip_rect.y = (selectOverlay->h - 8) - shiftFrame;
    clip_rect.x = 0;

    /* Render the entries */
    SDL_FillRect(itemListSurface, NULL, 0x0); 
    for (i=-1; i <= (MAX_GAMES_PER_SCREEN - 1); i++) {
      itemListSurfaceRect.y = (i+1)*(selectOverlay->h - 8);
      SDL_BlitSurface(itemText[i + topIndex], NULL, itemListSurface, &itemListSurfaceRect);
    }
    itemListSurfaceRect.y = (Y_POS + 60-15);
#if defined(BEAGLEBONE_BLACK) && !defined(CAPE_LCD3)
    itemListSurfaceRect.x = 35;
#else
    itemListSurfaceRect.x = 15;
#endif
    SDL_BlitSurface(itemListSurface, &clip_rect, screen, &itemListSurfaceRect);

    overlay1DstRect.y = (Y_POS + 55-15);
    SDL_BlitSurface(selectOverlay, NULL, screen, &overlay1DstRect);
    renderThumb(screen);
  }
  // Fifth case: >6 titles, text scrolls up
  else if ((currentIndex == (topIndex+(MAX_GAMES_PER_SCREEN-1))) && (nextIndex > currentIndex)) {
    clip_rect.w = itemListSurface->w;
    clip_rect.h = (selectOverlay->h - 8) * MAX_GAMES_PER_SCREEN;
    clip_rect.y = shiftFrame + (selectOverlay->h - 8);
    clip_rect.x = 0;
#if defined(BEAGLEBONE_BLACK) && !defined(CAPE_LCD3)
    itemListSurfaceRect.x = 35;
#else
    itemListSurfaceRect.x = 15;
#endif
    /* Render the entries */
    SDL_FillRect(itemListSurface, NULL, 0x0);
    for (i=0; i <= MAX_GAMES_PER_SCREEN; i++) {
      itemListSurfaceRect.y = (i+1)*(selectOverlay->h - 8);
      SDL_BlitSurface(itemText[i + topIndex], NULL, itemListSurface, &itemListSurfaceRect);
    }
    itemListSurfaceRect.y = (Y_POS + 60-15);
    SDL_BlitSurface(itemListSurface, &clip_rect, screen, &itemListSurfaceRect);

    overlay1DstRect.y = (Y_POS + 55-15) + (MAX_GAMES_PER_SCREEN-1)*(selectOverlay->h - 8);
    SDL_BlitSurface(selectOverlay, NULL, screen, &overlay1DstRect);
    renderThumb(screen);
  }

}

void incrementGameListFrame(void)
{
  //overlayFrame+=4;
  //if (overlayFrame > selectOverlay->w) overlayFrame = 0;

  if (currentIndex != nextIndex) {
    if (shiftFrame < 6) {
      shiftFrame+=shiftFrameFactor;
      shiftFrameFactor++;
    } else if (shiftFrame < 28) {
      shiftFrame+= shiftFrameFactor;
    } else {
      shiftFrame+=shiftFrameFactor;
      shiftFrameFactor++; 
    }
    //shiftFrame+=4;
  }

  if (shiftFrame >= (selectOverlay->h - 6)) {
    currentIndex = nextIndex;
    if (nextIndex < topIndex) topIndex = nextIndex;
    else if (nextIndex >= (topIndex + MAX_GAMES_PER_SCREEN)) topIndex++; 
    shiftFrame = 0;
    shiftFrameFactor = 1; // 3
  }
}

void shiftSelectedGameDown(void)
{
  /* Already shifting to a new index? */
  if (currentIndex != nextIndex)
    return;

  /* Are we at the end of the list? */
  if (currentIndex == (totalGames-1))
    return;

  /* Start the shifting process */
  nextIndex = currentIndex + 1;
  playOverlaySnd();
}

void shiftSelectedGameUp(void)
{
    /* Already shifting to a new index? */
  if (currentIndex != nextIndex)
    return;

  /* Are we at the start of the list? */
  if (currentIndex == 0)
    return;

  /* Start the shifting process */
  nextIndex = currentIndex - 1;
  playOverlaySnd();
}

int currentSelectedGameIndex(void)
{
  return currentIndex;
}

int acceptButton(void)
{
  return (currentIndex == nextIndex);
}
