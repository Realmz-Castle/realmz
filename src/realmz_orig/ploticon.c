#include "prototypes.h"
#include "variables.h"

/***************** ploticon2 *****************************/
void ploticon2(short tempid) {
  icon.right = icon.left + 32;
  icon.bottom = icon.top + 32;
  iconhand = NIL;
  iconhand = GetCIcon(tempid);
  if (iconhand) {
    PlotCIcon(&icon, iconhand);
    DisposeCIcon(iconhand);
  }
}

/***************** ploticon3 *****************************/
void ploticon3(short tempid, Rect therect) {
  iconhand = NIL;
  iconhand = GetCIcon(tempid);
  if (iconhand) {
    PlotCIcon(&therect, iconhand);
    DisposeCIcon(iconhand);
  }
}

/**************** ploticon *****************************/
void ploticon(short tempid, short showcurse) {
  short idStr;

  loaditemnotype(tempid);

  if (theControl == shopitemsvert) {
    icon.left = 390 + (leftshift / 2);
    if ((!tempid) && (cr != -1))
      item.iconid = 0;
  } else {
    icon.left = 10;
    if (!tempid)
      item.iconid = 0;
  }

  icon.bottom = icon.top + 32;
  icon.right = icon.left + 250;

  MoveTo(icon.left + 34, icon.bottom - 15);
  BackPixPat(whitepat);
  EraseRect(&icon);
  icon.right = icon.left + 32;

  if (item.iconid) {
    if (shopequip) {
      if ((item.type == 25) || (item.type == 23) || (item.type < 0))
        ploticon3(131, icon);
      else
        ploticon3(176, icon);
    }
    iconhand = NIL;

    iconhand = GetCIcon(lookupicon(item.iconid, lg));

    if (iconhand) {
      PlotCIcon(&icon, iconhand);
      DisposeCIcon(iconhand);
    }
    ForeColor(blackColor);

    idStr = getselection(item.itemid) + lg; // Myriad, the old is **not** legal
    GetIndString(myString, idStr, item.itemid - tempselection + 1);

    if (item.iscurse) {
      temp = getselection(item.iscurse); // Fantasoft 7.1
      if ((!shopequip) && (!showcurse))
        GetIndString(myString, temp + lg, item.iscurse - temp + 1); // Fantasoft 7.1
    }

    if (!lg)
      TextFace(outline);

    if (initems)
      SetPort(GetWindowPort(itemswindow));

    TextSize(16);
    TextFace(0);
    PtoCstr(myString);
    MyrDrawCString((Ptr)myString);
  }
  TextFace(0);
}

/***************** updateport *****************************/
void updateport(void) {
  short t;
  CIconHandle portrait;
  Rect checkrect;

  SetPort(GetWindowPort(gshop));
  BackPixPat(base);

  checkrect.top = 388;
  checkrect.left = 262 + (leftshift / 2);
  checkrect.right = checkrect.left + 22;
  checkrect.bottom = checkrect.top + 22;
  RGBBackColor(&greycolor);

  for (t = 0; t <= charnum; t++) {
    if (t == 3) {
      OffsetRect(&checkrect, 24, -71);
    }

    portrait = NIL;
    portrait = GetCIcon(c[t].pictid);

    if (portrait) {
      EraseRect(&checkrect);
      if (t != cr)
        PlotCIcon(&checkrect, portrait);
    }

    if (t == cl)
      ploticon3(145, checkrect);

    OffsetRect(&checkrect, 52, 0);

    if (portrait) {
      EraseRect(&checkrect);
      if (t != cl)
        PlotCIcon(&checkrect, portrait);
    }

    if (t == cr)
      ploticon3(145, checkrect);

    if (portrait)
      DisposeCIcon(portrait);
    OffsetRect(&checkrect, -52, 24);
  }
  BackPixPat(whitepat);
}

/***************** plotportrait *****************************/
void plotportrait(short tempid, Rect where, short class, short who) {
  short oldfont;
  Rect hold;
  Style oldstyle;
  short oldmode, oldsize;
  char showname[256];
  short poison, physical, charm, movement, magic, attdef, noshow = 0;

  where.right = where.left + 32;
  where.bottom = where.top + 32;
  iconhand = NIL;
  iconhand = GetCIcon(tempid);

  if ((**iconhand).iconBMap.bounds.bottom == 44)
    InsetRect(&where, -6, -6);

  if (iconhand) {
    PlotCIcon(&where, iconhand);
    DisposeCIcon(iconhand);
  }

  if (who > -1) {
    CGrafPtr thePort = GetQDGlobalsThePort();
    noshow = 0;

    hold = where;
    if ((thePort == GetWindowPort(look)) ||
        (thePort == GetWindowPort(screen)))
      where.left = 574 + leftshift;
    if (thePort == GetWindowPort(itemswindow)) {
      where.left = 629 + leftshift;
      where.top += 47;
    } else if (thePort == GetDialogPort(party)) {
      where.left = 575 + leftshift;
      where.top += 1;
    } else if (thePort == GetWindowPort(gshop)) {
      where.left = hold.left + 245;
      where.top += 39;
    } else if (thePort == GetDialogPort(gGeneration)) {
      where.left = 631;
      where.top += 20;
    } else if (thePort != GetWindowPort(screen))
      noshow = TRUE;

    where.top += 3;
    where.bottom = where.top + 16;

    poison = physical = charm = movement = magic = attdef = 0;

    if ((c[who].condition[9]) || (c[who].condition[28]) || (c[who].condition[2]))
      poison = TRUE;
    if ((c[who].condition[25] < 0) || (doauto[who]) || (c[who].traiter))
      charm = TRUE;
    if ((c[who].condition[0]) || (c[who].condition[2]) || (c[who].condition[6]))
      movement = TRUE;
    if ((c[who].condition[1]) || (c[who].condition[26]) || (c[who].condition[27]))
      physical = TRUE;
    if ((c[who].condition[5]) || (c[who].condition[29]) || (c[who].condition[39]))
      magic = TRUE;
    if ((c[who].condition[36]) || (c[who].condition[37]) || (c[who].condition[3]))
      attdef = TRUE;

    if (!noshow) {
      if (movement) {
        where.left -= 16;
        where.right = where.left + 16;
        ploticon3(180, where);
      }

      if (physical) {
        where.left -= 16;
        where.right = where.left + 16;
        ploticon3(183, where);
      }

      if (poison) {
        where.left -= 16;
        where.right = where.left + 16;
        ploticon3(181, where);
      }

      if (charm) {
        where.left -= 16;
        where.right = where.left + 16;
        ploticon3(182, where);
      }

      if (attdef) {
        where.left -= 16;
        where.right = where.left + 16;
        ploticon3(185, where);
      }

      if (magic) {
        where.left -= 16;
        where.right = where.left + 16;
        ploticon3(184, where);
      }
    }

    where = hold;
  }

  if (showcaste) {
    CGrafPtr thePort = GetQDGlobalsThePort();
    oldfont = GetPortTextFont(thePort); //->txFont;
    oldstyle = GetPortTextFace(thePort); //->txFace;
    oldsize = GetPortTextSize(thePort); //->txSize;
    oldmode = GetPortTextMode(thePort); //->txMode;

    MoveTo(where.left, where.bottom - 3);
    TextFont(1);
    TextMode(1);
    TextFace(bold);
    TextSize(10);
    ForeColor(whiteColor);

    GetIndString(myString, 131, class);
    PtoCstr(myString);
    strcpy(showname, myString);

    showname[7] = 0;
    // Myriad : Fit the name in the box
    while (TextWidth(showname, 0, strlen(showname)) > 46)
      showname[strlen(showname) - 1] = 0;

    MyrDrawCString(showname);

    TextFont(oldfont);
    TextFace(oldstyle);
    TextSize(oldsize);
    TextMode(oldmode);
  }
}
