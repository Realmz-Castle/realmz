#include "prototypes.h"
#include "variables.h"

/******************************* showdamage ***********************/
void showdamage(short body, short how, short damage) {
  short size = 0;

  if (body > 9) {
    size = monster[body - 10].size;
    bodyrect.top = 32 * monpos[body - 10][1];
    bodyrect.left = 32 * monpos[body - 10][0];
  } else {
    bodyrect.top = 32 * pos[body][1];
    bodyrect.left = 32 * pos[body][0];
  }

  if ((size == 1) || (size == 3))
    bodyrect.top -= 16;
  if (size > 1)
    bodyrect.left -= 16;

  bodyrect.bottom = bodyrect.top + 32;
  bodyrect.right = bodyrect.left + 32;

  SetPort(GetWindowPort(look));
  ploticon3(how, bodyrect);
  ForeColor(whiteColor);
  switch (how) {
    break;
    case 159: /*** condition ****/
      ForeColor(redColor);
      break;
    case 162: /*** fire ****/
      ForeColor(blackColor);
      sound(645);
      break;
    case 163: /*** cold ****/
      ForeColor(blackColor);
      sound(686);
      break;
    case 164: /*** elec ****/
      ForeColor(blackColor);
      sound(681);
      break;
    case 165: /*** chem ****/
      ForeColor(whiteColor);
      sound(640);
      break;
    case 166: /*** ment ****/
      ForeColor(whiteColor);
      sound(630);
      break;
    case 167: /*** magic ****/
      ForeColor(blackColor);
      sound(644);
      break;
  }
  MoveTo(bodyrect.left + (bodyrect.right - bodyrect.left) / 2 - 8, 3 + bodyrect.top + (bodyrect.bottom - bodyrect.top) / 2);
  TextSize(12);
  TextFace(bold);
  TextFont(genevafont); // Myriad //Fantasoft   Old line: TextFont(geneva);
  if (damage > 9)
    TextSize(10);
  string(damage);
}

/****************************** loaddark ****************************/
void loaddark(short lightlevel) {
  Rect tempRect;
  PicHandle picturehandle;

  lightlevel = pin(lightlevel, 0, 6);

  picturehandle = GetPicture(350 + lightlevel); /****** load dark shading mask graphics into offscreen GWorld ***/

  SetRect(&tempRect, 0, 0, 320 + leftshift, 320 + downshift);
  GetGWorld(&savedPort, &savedDevice); /********* get current drawing port info ****/

  SetGWorld(gmaps, NIL);

  maps = GetGWorldPixMap(gmaps); /***** get GWorld PixMap handle address ***/

  DrawPicture(picturehandle, &tempRect);
  SetGWorld(savedPort, savedDevice); /********* reset drawing port to what it was before ****/
}
