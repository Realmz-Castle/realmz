#include "prototypes.h"
#include "variables.h"

/********************************** finddxdy ******************/
void finddxdy(short from, short to) {
  short tempx, tempy, tempx1, tempy1;
  float dx = 0;
  float dy = 0;

  if (from > 9) {
    from -= 10;
    tempx1 = monpos[from][0];
    tempy1 = monpos[from][1];
  } else {
    tempx1 = pos[from][0];
    tempy1 = pos[from][1];
  }

  if (to < 9) {
    tempx = pos[to][0];
    tempy = pos[to][1];
  } else {
    to -= 10;
    tempx = monpos[to][0];
    tempy = monpos[to][1];
  }

  if (tempx < tempx1 - 1)
    dx = -1;
  if (tempx > tempx1 + 1)
    dx = 1;
  if (tempy < tempy1 - 1)
    dy = -1;
  if (tempy > tempy1 + 1)
    dy = 1;

  missileset(dx, dy);
}

/**************************************** toweap ***********************/
short toweap(short who) {

  if ((who < 0) || (who > charnum))
    return (NIL);

  c[who].toggle = 0;

  if (c[who].armor[2]) {
    loaditem(c[who].armor[2]);
    c[who].weaponsound = item.sound;
    return (0);
  }
  c[who].weaponsound = (30 + c[who].gender * 8);

  for (tt = 0; tt < c[who].numitems; tt++) {
    if (c[who].items[tt].id < 200) {
      loaditem(c[who].items[tt].id);
      if (abs(item.type) == 2) {
        if (wear(who, tt, 0))
          return (1);
      }
    }
  }
  return (0);
}

/**************************************** tomissle ***********************/
short tomissle(short who) {
  if ((who < 0) || (who > charnum))
    return (NIL);

  if (checkforenemy(1))
    return (FALSE);
  if (!c[who].armor[15])
    return (FALSE);

  loaditem(c[who].armor[15]);
  loadspell2(item.sp2);
  if (spellinfo.targettype != 1)
    return (FALSE);
  return (1);
}

/****************** missileset ********************/
void missileset(float dx, float dy) {
  Rect targetrect;

  SetPort(GetWindowPort(screen));
  targetrect.top = 320;
  targetrect.right = 0;
  targetrect.left = -32;
  targetrect.bottom = 352;
  OffsetRect(&targetrect, spellinfo.spelllook1 * 32, 0);
  iconhand = NIL;
  if (spellinfo.spelllook1 > 3)
    iconhand = GetCIcon(12032);
  else
    iconhand = GetCIcon(11992 + spellinfo.spelllook1 * 8 + direction(dx, dy));

  if (iconhand) {
    ForeColor(blackColor);
    BackColor(whiteColor);
    PlotCIcon(&spell, iconhand);

    GetGWorld(&savedPort, &savedDevice); /********* get current drawing port info ****/
    SetGWorld(gthePixels, NIL); /******** prepare to place horse icon in offscreen GWorld ******/
    EraseRect(&targetrect);

    PlotCIcon(&targetrect, iconhand);
    DisposeCIcon(iconhand);
    SetGWorld(savedPort, savedDevice); /********* reset drawing port to what it was before ****/

    RGBBackColor(&greycolor);
  }
}
