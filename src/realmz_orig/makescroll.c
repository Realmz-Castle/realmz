#include "prototypes.h"
#include "variables.h"

/*********************** makescroll ************************/
void makescroll(void) {
  GrafPtr oldport;
  char numparch;
  short t;
  Boolean free;
  scribing = inscroll = TRUE;

  for (t = 0; t <= numchannel; t++)
    quiet(t);
  compactheap();

  if (!reducesound)
    sound(647);

  in();

  GetPort(&oldport);
  scroll = GetNewDialog(155, 0L, (WindowPtr)-1L);

bustout:
  gStop = numparch = FALSE;
  SetCCursor(sword);
  SetPortDialogPort(scroll);
  BackPixPat(base);
  TextFont(font);
  TextSize(16);
  TextFace(bold);
  RGBBackColor(&greycolor);

  MoveWindow(GetDialogWindow(scroll), GlobalLeft + 356 + leftshift, GlobalTop + 9, FALSE);
  ShowWindow(GetDialogWindow(scroll));
  ErasePortRect();
  DrawDialog(scroll);

bustout2:

  ForeColor(yellowColor);
  gCurrent = scroll;
  MyrCDiStr(19, (StringPtr) "");

  pict(0, itemRect);

  MyrCDiStr(26, (StringPtr) "");

  plotportrait(c[charselectnew].pictid, itemRect, c[charselectnew].caste, -1);
  ForeColor(yellowColor);
  CtoPstr((Ptr)c[charselectnew].name);
  MyrPascalDiStr(27, (StringPtr)c[charselectnew].name);
  PtoCstr((StringPtr)c[charselectnew].name);

  for (t = 0; t < 5; t++) {
    iconhand = NIL;
    GetDialogItem(scroll, t + 2, &itemType, &itemHandle, &itemRect);
    if (c[charselectnew].scrollcase[t].powerlevel) {
      iconhand = GetCIcon(167 + c[charselectnew].scrollcase[t].powerlevel);
    } else
      iconhand = GetCIcon(176);

    if (iconhand) {
      PlotCIcon(&itemRect, iconhand);
      DisposeCIcon(iconhand);
    }

    if (c[charselectnew].scrollcase[t].powerlevel) {
      GetIndString(myString, 1000 * (c[charselectnew].scrollcase[t].castcaste) + c[charselectnew].scrollcase[t].castlevel, c[charselectnew].scrollcase[t].castnum);
      MyrPascalDiStr(t + 21, myString);
    } else
      MyrCDiStr(t + 21, (StringPtr) "");
  }

  iconhand = NIL;
  iconhand = GetCIcon(608);
  GetDialogItem(scroll, 13, &itemType, &itemHandle, &itemRect);
  if (iconhand) {
    PlotCIcon(&itemRect, iconhand);
    DisposeCIcon(iconhand);
  }

  numparch = 0;
  for (t = 0; t < c[charselectnew].numitems; t++) {
    if (c[charselectnew].items[t].id == 806)
      numparch += c[charselectnew].items[t].charge;
  }
  iconhand = NIL;

  if (numparch > 7)
    iconhand = GetCIcon(179);
  else if (numparch)
    iconhand = GetCIcon(167 + numparch);
  else
    iconhand = GetCIcon(176);

  GetDialogItem(scroll, 14, &itemType, &itemHandle, &itemRect);

  if (iconhand) {
    PlotCIcon(&itemRect, iconhand);
    DisposeCIcon(iconhand);
  }

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(scroll, itemHit, &itemType, &itemHandle, &buttonrect);

    if ((itemHit == 12) || (itemHit == 1)) {
      ploticon3(129, buttonrect);
      sound(141);
      goto out;
    }

    if ((itemHit == 16) || (itemHit == 17)) {
      ploticon3(135, buttonrect);
      sound(141);
    more:
      charselectnew++;
      if (itemHit == 16)
        charselectnew -= 2;
      if (charselectnew > charnum)
        charselectnew = 0;
      if (charselectnew < 0)
        charselectnew = charnum;
      if (!c[charselectnew].armor[13])
        goto more;
      ploticon3(136, buttonrect);
      goto bustout2;
    }

    if (itemHit == 19) {
      ploticon3(129, buttonrect);
      sound(141);
      viewcharacter(charselectnew, 0);
      needupdate = TRUE;
      updatemain(FALSE, -1);
      SelectWindow(GetDialogWindow(scroll));
      needupdate = TRUE;
      goto bustout;
    }

    if (itemHit == 15) {
      free = FALSE;
      for (t = 0; t < 5; t++)
        if (!c[charselectnew].scrollcase[t].powerlevel)
          free = TRUE;

      if (!free) {
        SetPort(GetWindowPort(look));
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(28);
        SetPortDialogPort(scroll);
        goto bustout;
      }

      if (!numparch) {
        SetPort(GetWindowPort(look));
        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(14);
        SetPortDialogPort(scroll);
        goto bustout2;
      }
      ploticon3(129, buttonrect);
      scribing = 2;
      castspell();
      updatemain(TRUE, 0);
      needupdate = TRUE;
      if (castnum > -1) {
        for (t = 0; t < 5; t++) {
          if (!c[charselectnew].scrollcase[t].powerlevel) {
            sound(600 + spellinfo.sound1);
            c[charselectnew].scrollcase[t].castnum = castnum + 1;
            c[charselectnew].scrollcase[t].castcaste = castcaste + 1;
            c[charselectnew].scrollcase[t].castlevel = castlevel;
            c[charselectnew].scrollcase[t].powerlevel = powerlevel;
            for (tt = 0; tt < c[charselectnew].numitems; tt++) {
              if (c[charselectnew].items[tt].id == 806) {
                c[charselectnew].load--;
                c[charselectnew].items[tt].charge--;
                if (!c[charselectnew].items[tt].charge)
                  dropitem(charselectnew, 806, tt, 0, FALSE);
                goto bustout;
              }
            }
            goto bustout;
          }
        }
      }
      goto bustout;
    }
  }

out:
  DisposeDialog(scroll);
  scribing = inspell = inscroll = FALSE;
  updatemain(TRUE, -1);
  scroll = NIL;
  SetPort(oldport);
}
