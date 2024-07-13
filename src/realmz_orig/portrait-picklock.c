#include "prototypes.h"
#include "variables.h"

/******************** portrait *****************************/
void portrait(short mode) {
  DialogRef portrait;
  short lastclick, racenameindex;
  GrafPtr oldport;
  short index = 0;
  short recindex = 0;
  Boolean rollover = 0;

  compactheap();

  GetPort(&oldport);
  MyrParamText((Ptr) "Select Character Portrait", (Ptr) "", (Ptr) "", (Ptr) "");
  portrait = GetNewDialog(170, 0L, (WindowPtr)-1L);

  gCurrent = portrait;
  SetPortDialogPort(portrait);
  BackPixPat(base);
  TextFont(font);
  TextFace(bold);
  TextSize(16);
  ForeColor(yellowColor);

  needdungeonupdate = TRUE;

  MoveWindow(GetDialogWindow(portrait), GlobalLeft, GlobalTop, FALSE);
  ShowWindow(GetDialogWindow(portrait));
  ErasePortRect();
  DrawDialog(portrait);

  GetDialogItem(gCurrent, 72, &itemType, &itemHandle, &itemRect);
  pict(213, itemRect);

  GetDialogItem(gCurrent, 69, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);
  GetDialogItem(gCurrent, 70, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

updaterecomended:

  loadprofile(characterl.race, 0);
  recindex = (races.defaulticonset * 6) - 6;

  // recindex = (characterl.race * 6)-6;

  if (characterl.gender == 1)
    MyrCDiStr(73, (StringPtr) "Male");
  else
    MyrCDiStr(73, (StringPtr) "Female");
  GetIndString(myString, 129, characterl.race);
  MyrPascalDiStr(74, myString);
  GetIndString(myString, 131, characterl.caste);
  MyrPascalDiStr(75, myString);

  for (t = 3; t < 9; t++) {
    GetDialogItem(portrait, t, &itemType, &itemHandle, &buttonrect);
    iconhand = GetCIcon(t + 254 + recindex);

    if (iconhand != NIL) {
      if ((**iconhand).iconBMap.bounds.bottom == 32)
        InsetRect(&buttonrect, 6, 6);
      PlotCIcon(&buttonrect, iconhand);
      if ((**iconhand).iconBMap.bounds.bottom == 32)
        InsetRect(&buttonrect, -6, -6);
      DisposeCIcon(iconhand);
    }
    upbutton(FALSE);
    if ((t + 254 + recindex) == characterl.pictid) {
      downbutton(FALSE);
      lastclick = t;
    }
  }

  gStop = FALSE;

  if (!mode) {
    for (t = 0; t <= charnum; t++) {
      GetDialogItem(portrait, 45 + t, &itemType, &itemHandle, &itemRect);
      plotportrait(c[t].pictid, itemRect, c[t].caste, -1);

      GetDialogItem(portrait, 51 + t, &itemType, &itemHandle, &itemRect);
      ploticon3(c[t].iconid, itemRect);

      if (t == charselectnew) {
        GetDialogItem(portrait, 33 + t, &itemType, &itemHandle, &itemRect);
        ploticon3(129, itemRect);
      }

      CtoPstr(c[t].name);
      ForeColor(yellowColor);
      MyrPascalDiStr(t + 63, (StringPtr)c[t].name);
      PtoCstr((StringPtr)c[t].name);
      GetDialogItem(portrait, 57 + t, &itemType, &itemHandle, &buttonrect);
      downbutton(FALSE);
    }
  } else {
    GetDialogItem(portrait, 33, &itemType, &itemHandle, &itemRect);
    pict(154, itemRect);

    GetDialogItem(portrait, 45, &itemType, &itemHandle, &itemRect);
    plotportrait(characterl.pictid, itemRect, characterl.caste, -1);

    CtoPstr(characterl.name);
    ForeColor(yellowColor);
    MyrPascalDiStr(63, (StringPtr)characterl.name);
    PtoCstr((StringPtr)characterl.name);
    GetDialogItem(portrait, 57, &itemType, &itemHandle, &buttonrect);
    downbutton(FALSE);
  }

updatelight:

  racenameindex = index / 24;
  for (tt = 76; tt < 80; tt++) {
    GetIndString(myString, 129, racenameindex * 4 + (tt - 75));
    MyrPascalDiStr(tt, myString);
  }
  if (index == 96)
    MyrCDiStr(79, (StringPtr) "Misc. Humanoid");

  for (t = 9; t <= 32; t++) {
    GetDialogItem(portrait, t, &itemType, &itemHandle, &buttonrect);
    iconhand = GetCIcon(t + 248 + index);
    EraseRect(&buttonrect);
    if (iconhand != NIL) {
      if ((**iconhand).iconBMap.bounds.bottom == 32)
        InsetRect(&buttonrect, 6, 6);
      PlotCIcon(&buttonrect, iconhand);
      if ((**iconhand).iconBMap.bounds.bottom == 32)
        InsetRect(&buttonrect, -6, -6);
      DisposeCIcon(iconhand);
    } else
      rollover = TRUE;

    upbutton(FALSE);

    if ((t + 248 + index) == characterl.pictid) {
      downbutton(FALSE);
      lastclick = t;
    }
  }

  BeginUpdate(GetDialogWindow(portrait));
  EndUpdate(GetDialogWindow(portrait));

  while (gStop == FALSE) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(portrait, itemHit, &itemType, &itemHandle, &buttonrect);

    if (itemHit < 3) {
      GetDialogItem(portrait, 2, &itemType, &itemHandle, &buttonrect);
      ploticon3(133, buttonrect);
      gStop = TRUE;
    }

    if ((itemHit > 2) && (itemHit < 9)) /**** recomende set *****/
    {
      GetDialogItem(portrait, lastclick, &itemType, &itemHandle, &buttonrect);
      upbutton(FALSE);

      GetDialogItem(portrait, itemHit, &itemType, &itemHandle, &buttonrect);
      downbutton(FALSE);

      sound(130);
      characterl.pictid = itemHit + recindex + 254;
      GetDialogItem(portrait, charselectnew + 33, &itemType, &itemHandle, &itemRect);
      pict(154, itemRect);
      GetDialogItem(portrait, charselectnew + 45, &itemType, &itemHandle, &itemRect);
      plotportrait(characterl.pictid, itemRect, characterl.caste, -1);

      lastclick = itemHit;
    }

    if ((itemHit > 8) && (itemHit < 33)) /**** alternet set *****/
    {
      GetDialogItem(portrait, lastclick, &itemType, &itemHandle, &buttonrect);
      upbutton(FALSE);

      GetDialogItem(portrait, itemHit, &itemType, &itemHandle, &buttonrect);
      downbutton(FALSE);

      sound(130);
      characterl.pictid = itemHit + index + 257 - 9;
      GetDialogItem(portrait, charselectnew + 33, &itemType, &itemHandle, &itemRect);
      pict(154, itemRect);
      GetDialogItem(portrait, charselectnew + 45, &itemType, &itemHandle, &itemRect);
      plotportrait(characterl.pictid, itemRect, characterl.caste, -1);

      lastclick = itemHit;
    }

    if (itemHit == 71) /***** change alternet set ****/
    {
      sound(130);
      if ((rollover) || (index > 95))
        index = 0;
      else
        index += 24;
      rollover = FALSE;
      goto updatelight;
    }

    if (((itemHit > 32) && (itemHit < 39)) && (!mode)) {
      sound(141);

      GetDialogItem(portrait, lastclick, &itemType, &itemHandle, &buttonrect);
      upbutton(FALSE);

      GetDialogItem(portrait, charselectnew + 33, &itemType, &itemHandle, &itemRect);
      ploticon3(130, itemRect);

      GetDialogItem(portrait, itemHit, &itemType, &itemHandle, &itemRect);
      ploticon3(129, itemRect);

      c[charselectnew] = characterl;
      charselectnew = itemHit - 33;
      characterl = c[charselectnew];

      GetDialogItem(portrait, characterl.pictid - 254, &itemType, &itemHandle, &buttonrect);
      downbutton(FALSE);

      lastclick = characterl.pictid - 254;
      goto updaterecomended;
    }
  }
  sound(141);
  c[charselectnew] = characterl;
  DisposeDialog(portrait);
  SetPort(oldport);
}

/*********************** picklock ****************/
short picklock(short who, short type) {
  GrafPtr oldPort;
  Rect r1, temprect;
  short one[6], delta, jump, chance;
  short yellow, green, tumblers, temp, temp2;
  long start, limit;

  if ((who < 0) || (who > charnum))
    return (0);

  tumblers = thief.tumblers;
  if (tumblers > 6)
    tumblers = 6;

  chance = c[who].spec[5 + type] + thief.modifer[type];

  jump = 20;

  if (chance > 90)
    chance = 90;

  if (jump < 5)
    jump = 5;

  GetPort(&oldPort);
  yellow = 200 - 2 * chance;
  green = 200 - chance;

  for (t = 0; t < 6; t++)
    one[t] = 8 + yellow / 2;

  gGeneration = GetNewDialog(142, 0L, (WindowPtr)-1L);
  SetPortDialogPort(gGeneration);
  BackPixPat(base);
  gCurrent = gGeneration;
  ErasePortRect();
  ForeColor(yellowColor);
  SizeWindow(GetDialogWindow(gGeneration), 215, 35 + 35 * tumblers, FALSE);
  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + 35, GlobalTop + 5, FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  DrawDialog(gGeneration);
  GetDialogItem(gGeneration, 1, &itemType, &itemHandle, &temprect);

  while (Button()) {
  }

  FlushEvents(everyEvent, 0);
  start = TickCount();
  limit = (start + tumblers * 120) + 180;

  while (!Button()) {
    delay(2);
    temp = ((limit - TickCount()) / 60);
    ForeColor(yellowColor);
    if (temp != temp2) {
      DialogNum(15, temp);
      sound(10129);
    }

    temp2 = temp;
    if (!temp)
      goto out;

    for (t = 0; t < tumblers; t++) {
      if (one[t] < green) {
        GetDialogItem(gGeneration, 1 + t, &itemType, &itemHandle, &r1);
        temprect = r1;
        RGBForeColor(&greycolor);
        delta = Rand(jump);
        if (Rand(100) <= 55) {
          one[t] += delta;
          if (one[t] > 208)
            one[t] = 208;
        } else {
          one[t] -= delta;
          if (one[t] < 10)
            one[t] = 10;
          temprect.left = one[t];
          EraseRect(&temprect);
        }

        ForeColor(redColor);
        r1.right = 8 + one[t];
        if (r1.right > yellow)
          r1.right = yellow;
        PaintRect(&r1);

        if (one[t] > yellow) {
          ForeColor(yellowColor);
          r1.right = 8 + one[t];
          if (r1.right > green)
            r1.right = green;
          r1.left = yellow;
          PaintRect(&r1);
        }

        if (one[t] >= green) {
          ForeColor(greenColor);
          r1.right = 208;
          r1.left = green;
          PaintRect(&r1);
        }
      }
    }
  }
out:
  delay(50);
  reply = TRUE;
  for (t = 0; t < tumblers; t++)
    if (one[t] < yellow)
      reply = FALSE;
  DisposeDialog(gGeneration);
  SetPort(oldPort);
  return (reply);
}
