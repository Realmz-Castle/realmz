#include "prototypes.h"
#include "variables.h"

/**************************** drawbody *************************/
void drawbody(short body, short force, short where) /***** 0 = look, 1 = buff ******/
{
  char size = 0;
  Rect bound;
  Boolean helpless = 0;

  bound.top = 384;
  bound.left = 480;
  bound.right = 512;
  bound.bottom = 416;

  if (!incombat)
    return;
  if (body < 0)
    return;

  iconhand = NIL;

  if (body > 9) {
    if (monster[body - 10].stamina < 1)
      return;
    if (monster[body - 10].condition[1])
      helpless = TRUE;

    size = monster[body - 10].size;
    temp = monster[body - 10].iconid;
    if (monster[body - 10].lr == 1)
      temp += 308;
    iconhand = GetCIcon(temp);
    bodyrect.top = 32 * monpos[body - 10][1];
    bodyrect.left = 32 * monpos[body - 10][0];
    if (monster[body - 10].traiter) {
      bound.left += 64;
      bound.right += 64;
    }
  } else {
    if (c[body].stamina < 1)
      return;
    if (c[body].condition[1])
      helpless = TRUE;

    iconhand = GetCIcon(c[body].iconid + (500 * c[body].face));
    bodyrect.top = 32 * pos[body][1];
    bodyrect.left = 32 * pos[body][0];
    if (c[body].traiter) {
      bound.left += 64;
      bound.right += 64;
    }
  }

  bodyrect.bottom = bodyrect.top + 32;
  bodyrect.right = bodyrect.left + 32;

  if ((size == 1) || (size == 3)) {
    bodyrect.top -= 32;
    bound.top -= 32;
  }

  if (size > 1) {
    bodyrect.left -= 32;
    bound.left -= 32;
  }

  SetPort(GetWindowPort(look));
  if ((body == q[up]) || (force) || (helpless)) /***** 0 = look, 1 = buff ******/
  {
    if (helpless) {
      bound.top += 128;
      bound.bottom += 128;
      if (!where) {
        BitMap* src = GetPortBitMapForCopyBits(gthePixels);
        BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(look));
        CopyBits(src, dst, &bound, &bodyrect, 0, NIL);
      } else {
        BitMap* src = GetPortBitMapForCopyBits(gthePixels);
        BitMap* dst = GetPortBitMapForCopyBits(gbuff);
        CopyBits(src, dst, &bound, &bodyrect, 0, NIL);
      }
      bound.top -= 128;
      bound.bottom -= 128;
    } else {
      if (!where) {
        BitMap* src = GetPortBitMapForCopyBits(gthePixels);
        BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(look));
        CopyBits(src, dst, &bound, &bodyrect, 0, NIL);
      } else {
        BitMap* src = GetPortBitMapForCopyBits(gthePixels);
        BitMap* dst = GetPortBitMapForCopyBits(gbuff);
        CopyBits(src, dst, &bound, &bodyrect, 0, NIL);
      }
    }
  }

  if ((!where) && (iconhand))
    PlotCIcon(&bodyrect, iconhand); /***** look *****/
  else if (iconhand) /***** place icon into offscreen GWorld *****/
  {
    GetGWorld(&savedPort, &savedDevice); /********* get current drawing port info ****/
    SetGWorld(gbuff, NIL); /******** prepare to place icon in offscreen GWorld ******/
    PlotCIcon(&bodyrect, iconhand);
    SetGWorld(savedPort, savedDevice); /********* reset drawing port to what it was before ****/
  }
  if (iconhand)
    DisposeCIcon(iconhand);
}
