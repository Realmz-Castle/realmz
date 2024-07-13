#include "prototypes.h"
#include "variables.h"

/************************* GWorldInit ******************************/
void GWorldInit(void) {
  PicHandle picture;
  Rect aire; // MYRIAD PC : Can' use directly the frameRect, must convert

  gHilite = GetPixPat(128);
  gNeutral = GetPixPat(129);
  gShadow = GetPixPat(130);

  // horizontal Rectangles

  SetRect(&WALLDEST[0], -62, 45, 14, 102);
  SetRect(&WALLDEST[1], 14, 45, 90, 102);
  SetRect(&WALLDEST[2], 90, 45, 166, 102);
  SetRect(&WALLDEST[3], 166, 45, 242, 102);
  SetRect(&WALLDEST[4], 242, 45, 318, 102);

  SetRect(&WALLDEST[5], -16, 40, 80, 112);
  SetRect(&WALLDEST[6], 80, 40, 176, 112);
  SetRect(&WALLDEST[7], 176, 40, 272, 112);

  SetRect(&WALLDEST[8], -64, 32, 64, 128);
  SetRect(&WALLDEST[9], 64, 32, 192, 128);
  SetRect(&WALLDEST[10], 192, 32, 320, 128);

  SetRect(&WALLDEST[11], -160, 16, 32, 160);
  SetRect(&WALLDEST[12], 32, 16, 224, 160);
  SetRect(&WALLDEST[13], 224, 16, 416, 160);

  // Wall1 source rectangles

  SetRect(&WALL1[0], 0, 0, 192, 144);
  SetRect(&WALL1[1], 128, 225, 320, 369);
  SetRect(&WALL1[2], 320, 0, 512, 144);
  SetRect(&WALL1[3], 448, 225, 640, 369);

  // Wall2 source rectangels

  SetRect(&WALL2[0], 0, 144, 128, 240);
  SetRect(&WALL2[1], 192, 129, 320, 225);
  SetRect(&WALL2[2], 320, 144, 448, 240);
  SetRect(&WALL2[3], 512, 129, 640, 225);

  // Wall3 source rectangels

  SetRect(&WALL3[0], 0, 240, 96, 312);
  SetRect(&WALL3[1], 224, 57, 320, 129);
  SetRect(&WALL3[2], 320, 240, 416, 312);
  SetRect(&WALL3[3], 544, 57, 640, 129);

  // Wall4 source rectangels

  SetRect(&WALL4[0], 0, 312, 57, 369);
  SetRect(&WALL4[1], 244, 0, 320, 57);
  SetRect(&WALL4[2], 320, 312, 396, 369);
  SetRect(&WALL4[3], 564, 0, 640, 57);

  // perspective walls dest rectangles

  SetRect(&VERTDEST[0], 90, 45, 98, 102);
  SetRect(&VERTDEST[1], 158, 45, 166, 102);
  SetRect(&VERTDEST[2], 80, 40, 90, 112);
  SetRect(&VERTDEST[3], 166, 40, 176, 112);
  SetRect(&VERTDEST[4], 64, 32, 80, 128);
  SetRect(&VERTDEST[5], 176, 32, 192, 128);
  SetRect(&VERTDEST[6], 32, 16, 64, 160);
  SetRect(&VERTDEST[7], 192, 16, 224, 160);
  SetRect(&VERTDEST[8], 0, 0, 32, 192);
  SetRect(&VERTDEST[9], 224, 0, 256, 192);

  // perspective walls source rectangles

  for (tt = 0; tt < 3; tt++) {
    for (t = 0; t < 10; t++) {
      VERTSOURCE[tt][t] = VERTDEST[t];
      OffsetRect(&VERTSOURCE[tt][t], 0, 192 * tt);
    }

    OffsetRect(&VERTSOURCE[tt][1], -60, 0);
    OffsetRect(&VERTSOURCE[tt][3], -60, 0);
    OffsetRect(&VERTSOURCE[tt][5], -60, 0);
    OffsetRect(&VERTSOURCE[tt][7], -60, 0);
    OffsetRect(&VERTSOURCE[tt][9], -60, 0);
  }

  // pillar rects

  SetRect(&PILLAR_RECT_SOURCE[0], 0, 0, 32, 150);
  SetRect(&PILLAR_RECT_SOURCE[1], 32, 0, 48, 96);
  SetRect(&PILLAR_RECT_SOURCE[2], 48, 0, 60, 72);
  SetRect(&PILLAR_RECT_SOURCE[3], 60, 0, 70, 57);

  SetRect(&LEFT_PILLAR_RECT[0], 16, 16, 48, 166);
  SetRect(&LEFT_PILLAR_RECT[1], 56, 32, 72, 128);
  SetRect(&LEFT_PILLAR_RECT[2], 74, 40, 86, 112);
  SetRect(&LEFT_PILLAR_RECT[3], 86, 45, 96, 102);

  SetRect(&RIGHT_PILLAR_RECT[0], 208, 16, 240, 166);
  SetRect(&RIGHT_PILLAR_RECT[1], 184, 32, 200, 128);
  SetRect(&RIGHT_PILLAR_RECT[2], 170, 40, 182, 112);
  SetRect(&RIGHT_PILLAR_RECT[3], 162, 45, 172, 102);

  GetGWorld(&savedPort, &savedDevice);
  SetRect(&itemRect, 0, 0, 320 + leftshift, 320 + downshift);

  /******************** Setup dark land mask buffer (Black and white mask) ********/
  if (NewGWorld(&gmaps, 1, &itemRect, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gmaps, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);
  maps = GetGWorldPixMap(gmaps); /***** get GWorld PixMap handle address ***/
  LockPixels(maps); /****** lock GWorld for drawing functions ******/

  /******************** Setup Edge Mark Buffer ********/
  if (NewGWorld(&gedge, 8, &itemRect, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gedge, NIL);
  PenSize(3, 3);
  ForeColor(blackColor);
  BackColor(whiteColor);
  edge = GetGWorldPixMap(gedge);
  LockPixels(edge); /****** lock GWorld for cb functions ******/

  /******************** Screen buffer layer 1 ********/
  if (NewGWorld(&gbuff, 8, &itemRect, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gbuff, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);
  buff = GetGWorldPixMap(gbuff);
  LockPixels(buff); /****** lock GWorld for cb functions ******/

  /******************** Screen buffer layer 2 ********/
  if (NewGWorld(&gbuff2, 8, &itemRect, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gbuff2, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);
  buff2 = GetGWorldPixMap(gbuff2);
  LockPixels(buff2); /****** lock GWorld for cb functions ******/

  /******************** Land tile screen buffer & combat icons data ********/

  picture = GetPicture(302);
  aire = ((**picture).picFrame);
  SetRect(&itemRect, 0, 0, 640, 640);
  if (NewGWorld(&gthePixels, 8, &itemRect, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gthePixels, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);

  thePixels = GetGWorldPixMap(gthePixels);
  LockPixels(thePixels); /****** lock GWorld for cb functions ******/
  DrawPicture(picture, &aire);

  /******************** Back PICT ********/
  picture = GetPicture(50);
  aire = ((**picture).picFrame);
  if (NewGWorld(&gBackWorld, 8, &aire, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gBackWorld, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);

  gBackPix = GetGWorldPixMap(gBackWorld);
  LockPixels(gBackPix); /****** lock GWorld for cb functions ******/
  DrawPicture(picture, &aire);

  /******************** floor PICT ********/
  if (NewGWorld(&gFloorWorld, 8, &aire, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gFloorWorld, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);

  gFloorPix = GetGWorldPixMap(gFloorWorld);
  LockPixels(gFloorPix); /****** lock GWorld for cb functions ******/
  DrawPicture(picture, &aire);

  /******************** h wall PICT ********/
  picture = GetPicture(51);
  aire = ((**picture).picFrame);

  if (NewGWorld(&gHWallsWorld, 8, &aire, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gHWallsWorld, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);

  gHWallsPix = GetGWorldPixMap(gHWallsWorld);
  LockPixels(gHWallsPix); /****** lock GWorld for cb functions ******/
  DrawPicture(picture, &aire);

  /******************** v wall PICT ********/
  picture = GetPicture(52); /**** 56 ****/
  aire = ((**picture).picFrame);

  if (NewGWorld(&gVWallsWorld, 8, &aire, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gVWallsWorld, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);

  gVWallsPix = GetGWorldPixMap(gVWallsWorld);
  LockPixels(gVWallsPix); /****** lock GWorld for cb functions ******/
  DrawPicture(picture, &aire);

  /******************** pillars PICT ********/
  picture = GetPicture(55);
  aire = ((**picture).picFrame);

  if (NewGWorld(&gPillarsWorld, 8, &aire, 0L, 0L, 0L))
    scratch2(2); /*** could not create gworld ****/
  SetGWorld(gPillarsWorld, NIL);
  ForeColor(blackColor);
  BackColor(whiteColor);

  gPillarsPix = GetGWorldPixMap(gPillarsWorld);
  LockPixels(gPillarsPix); /****** lock GWorld for cb functions ******/
  DrawPicture(picture, &aire);

  SetGWorld(savedPort, savedDevice);
}
