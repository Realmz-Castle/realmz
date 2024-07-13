#include "prototypes.h"
#include "variables.h"

/************************ getscroll *************************/
short getscroll(void) {
  GrafPtr oldport;
  short itemHit;
  short t;
  Rect r;
  Boolean first = TRUE;
  GetPort(&oldport);

  for (t = 0; t <= numchannel; t++)
    quiet(t);

  compactheap();

  if (!reducesound)
    sound(647);
  inscroll = TRUE;

  if (!initems)
    in();
  scroll = GetNewDialog(154, 0L, (WindowPtr)-1L);
  SetPortDialogPort(scroll);
  BackPixPat(base);
  gStop = FALSE;
  gCurrent = scroll;
  SetCCursor(sword);
  TextFont(font);
  TextFace(bold);
  TextSize(16);
  ForeColor(yellowColor);

  MoveWindow(GetDialogWindow(scroll), GlobalLeft + 358 + leftshift, GlobalTop + 9, FALSE);
  ShowWindow(GetDialogWindow(scroll));

  GetPortBounds(GetQDGlobalsThePort(), &r);
  EraseRect(&r);

  if (incombat)
    charselectnew = charup;

  if (!checkfortype(charselectnew, 13, FALSE))
    goto more;

  first = FALSE;
over:

  SetPortDialogPort(scroll);
  RGBBackColor(&greycolor);
  gStop = FALSE;
  gCurrent = scroll;
  SetCCursor(sword);
  TextFont(font);
  TextFace(bold);
  TextSize(16);
  GetPortBounds(GetQDGlobalsThePort(), &r);
  EraseRect(&r);
  DrawDialog(scroll);
over2:
  ForeColor(yellowColor);
  CtoPstr((Ptr)c[charselectnew].name);
  MyrPascalDiStr(19, (StringPtr)c[charselectnew].name);
  PtoCstr((StringPtr)c[charselectnew].name);

  MyrCDiStr(13, (StringPtr) "");
  pict(0, itemRect);

  MyrCDiStr(16, (StringPtr) "");

  plotportrait(c[charselectnew].pictid, itemRect, c[charselectnew].caste, -1);
  ForeColor(yellowColor);
  cleartarget();

  iconhand = NIL;

  for (t = 0; t < 5; t++) {
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
      GetIndString(myString, 1000 * c[charselectnew].scrollcase[t].castcaste + c[charselectnew].scrollcase[t].castlevel, c[charselectnew].scrollcase[t].castnum);
      MyrPascalDiStr(t + 20, myString);
    } else
      MyrCDiStr(t + 20, (StringPtr) "");
  }

backup:
  SetPortDialogPort(scroll);

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(scroll, itemHit, &itemType, &itemHandle, &buttonrect);

    if (itemHit == 13) {
      ploticon3(129, buttonrect);
      sound(141);
      viewcharacter(charselectnew, 0);
      updatemain(FALSE, 0);

      if (incombat) {
        centerstage(0);
        combatupdate2(charup);
        updatecontrols();
      }
      needupdate = TRUE;
      SelectWindow(GetDialogWindow(scroll));
      goto over;
    }

    if ((itemHit == 12) || (itemHit == 1)) {
      ploticon3(129, buttonrect);
      sound(141);
      inspell = 0;
      goto out;
    }

    if ((itemHit == 14) || (itemHit == 15)) {
      ploticon3(135, buttonrect);
      sound(141);
    more:
      charselectnew++;
      if (itemHit == 14)
        charselectnew -= 2;
      if (charselectnew > charnum)
        charselectnew = 0;
      if (charselectnew < 0)
        charselectnew = charnum;
      if (!checkfortype(charselectnew, 13, FALSE))
        goto more;
      if (!first) {
        ploticon3(136, buttonrect);
        first = FALSE;
      }
      goto over2;
    }

    if (c[charselectnew].stamina < 1) {
      if (!initems)
        SetPort(GetWindowPort(look));
      else
        SetPort(GetWindowPort(itemswindow));

      ForeColor(blackColor);
      BackColor(whiteColor);
      warn(27);
      goto over;
    }

    if (c[charselectnew].condition[25]) {
      if (!initems)
        SetPort(GetWindowPort(look));
      else
        SetPort(GetWindowPort(itemswindow));

      ForeColor(blackColor);
      BackColor(whiteColor);
      warn(25);
      goto over;
    }

    if (c[charselectnew].scrollcase[itemHit - 7].powerlevel) {
      if ((incombat) && (charselectnew != charup)) {
        if (!initems)
          SetPort(GetWindowPort(look));
        else
          SetPort(GetWindowPort(itemswindow));

        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(10);
        goto over;
      }

      castnum = c[charselectnew].scrollcase[itemHit - 7].castnum - 1;
      powerlevel = c[charselectnew].scrollcase[itemHit - 7].powerlevel;
      castcaste = c[charselectnew].scrollcase[itemHit - 7].castcaste - 1;
      castlevel = c[charselectnew].scrollcase[itemHit - 7].castlevel;

      loadspell(castcaste, castlevel, castnum);

      usescroll = TRUE;

      if ((!incombat) && (!spellinfo.incamp) && (!encountflag)) {
        if (question(-5))
          c[charselectnew].scrollcase[itemHit - 7].powerlevel = 0;
        goto over;
      }

      if ((incombat) && (!spellinfo.incombat)) {
        if (!initems)
          SetPort(GetWindowPort(look));
        else
          SetPort(GetWindowPort(itemswindow));

        ForeColor(blackColor);
        BackColor(whiteColor);
        warn(23);
        goto over;
      }

      c[charselectnew].scrollcase[itemHit - 7].powerlevel = 0; /**** erase scroll used ****/
      if (!powerlevel)
        goto backup;

      inspell = TRUE;
      goto out;
    }
  }

out:

  DisposeDialog(scroll);
  if (!initems)
    updatemain(TRUE, -1);
  SetPort(oldport);
  inscroll = FALSE;
  scroll = NIL;
  if (incombat)
    charselectnew = charup;
  return (inspell);
}
