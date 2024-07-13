#include "prototypes.h"
#include "variables.h"

/******************* spelleffect ***************/
void spelleffect(short t, short mode) /*** mode 0 = normal, 1 = quickshow ****/
{
  short tt, start, body;
  Rect store, temprect;
  Boolean plotflag = FALSE;
  BitMap *src, *dst;

  store.top = store.left = 0;
  store.bottom = store.right = 64;

  if (!spellinfo.spelllook2)
    start = 12032;
  else
    start = (spellinfo.spelllook2 * 8) + 11992;
  if (!incombat) {
    bodyrect.top = 50 * t + 9;
    bodyrect.bottom = bodyrect.top + 32;
    bodyrect.left = 339;
    bodyrect.right = bodyrect.left + 32;
    if ((itemused) && (initems))
      SetPort(GetWindowPort(itemswindow));
    else
      SetPort(GetWindowPort(screen));
    ForeColor(blackColor);
    BackColor(whiteColor);
    src = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
    dst = GetPortBitMapForCopyBits(gbuff2);
    CopyBits(src, dst, &bodyrect, &store, 0, NIL);
    for (tt = start; tt < start + 8; tt++) {
      CopyBits(dst, src, &store, &bodyrect, 0, NIL);
      ploticon3(tt, bodyrect);
      delay(1 + delayspeed / 30);
    }
    updatechar(t, 3);
    RGBBackColor(&greycolor);
  } else if ((!mode) || (!usequickshow)) /**** dont use quickshow ******/
  {
    SetPort(GetWindowPort(look));
    bodyground(t, 0);
    drawbody(t, 0, 0);

    if ((spellinfo.targettype == 9) || (spellinfo.targettype == 10)) {
      drawbody(t, TRUE, 0);
      delay(10);
      drawbody(t, 0, 0);
      return;
    }

    src = GetPortBitMapForCopyBits(GetWindowPort(look));
    dst = GetPortBitMapForCopyBits(gbuff2);
    CopyBits(src, dst, &bodyrect, &store, 0, NIL);

    for (tt = start; tt < start + 8; tt++) {
      if (SectRect(&bodyrect, &lookrect, &temprect)) {
        CopyBits(dst, src, &store, &bodyrect, 0, NIL);
      }
      ploticon3(tt, bodyrect);
      delay(1 + delayspeed / 30);
    }
    if (SectRect(&bodyrect, &lookrect, &temprect)) {
      CopyBits(dst, src, &store, &bodyrect, 0, NIL);
    }
  } else if ((mode) || (usequickshow)) /**** use quickshow ****/
  {
    SetPort(GetWindowPort(look));
    ForeColor(blackColor);
    BackColor(whiteColor);
    src = GetPortBitMapForCopyBits(GetWindowPort(look));
    dst = GetPortBitMapForCopyBits(gbuff);
    CopyBits(src, dst, &lookrect, &lookrect, 0, NIL);

    for (tt = start; tt < start + 8; tt++) {
      iconhand = GetCIcon(tt);
      src = GetPortBitMapForCopyBits(gbuff2);
      CopyBits(dst, src, &lookrect, &lookrect, 0, NIL);

      for (loop = 0; loop < 110; loop++) {
        if (track[loop]) {
          body = loop;

          if ((body > 9) || ((body > -1) && (body <= charnum))) {
            if (body > 9) {
              size = monster[body - 10].size;
              bodyrect.top = 32 * monpos[body - 10][1];
              bodyrect.left = 32 * monpos[body - 10][0];
            } else {
              size = 0;
              bodyrect.top = 32 * pos[body][1];
              bodyrect.left = 32 * pos[body][0];
            }

            bodyrect.bottom = bodyrect.top + 32;
            bodyrect.right = bodyrect.left + 32;

            if ((size == 1) || (size == 3))
              bodyrect.top -= 32;
            if (size > 1)
              bodyrect.left -= 32;

            if (SectRect(&bodyrect, &lookrect, &temprect)) {
              if (!plotflag) {
                PlotCIcon(&bodyrect, iconhand);
                plotflag = TRUE;
              }
              GetGWorld(&savedPort, &savedDevice); /********* get current drawing port info ****/
              SetGWorld(gbuff2, NIL); /******** prepare to place icon in offscreen GWorld ******/
              PlotCIcon(&bodyrect, iconhand);
              SetGWorld(savedPort, savedDevice); /********* reset drawing port to what it was before ****/
            }
          }
        }
      }
      plotflag = FALSE;
      src = GetPortBitMapForCopyBits(gbuff2);
      dst = GetPortBitMapForCopyBits(GetWindowPort(look));
      CopyBits(src, dst, &lookrect, &lookrect, 0, NIL);
      if (iconhand)
        DisposeCIcon(iconhand);
      if (!collideflag)
        delay(4);
    }
    src = GetPortBitMapForCopyBits(gbuff);
    dst = GetPortBitMapForCopyBits(GetWindowPort(look));
    CopyBits(src, dst, &lookrect, &lookrect, 0, NIL);
  }
}
