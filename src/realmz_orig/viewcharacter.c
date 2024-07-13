#include "variables.h"

/************************* viewcharacter *************************/
void viewcharacter(short view, short mode) {
  DialogRef aging;
  long prestige = 0;
  Boolean changename = 0;

  if (!reducesound)
    sound(647);
  gGeneration = GetNewDialog(138, 0L, (WindowPtr)-1L);

refresh:

  SetPortDialogPort(gGeneration);
  TextFont(font);
  BackPixPat(base);
  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + (leftshift / 2), GlobalTop + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  ErasePortRect();
  DrawDialog(gGeneration);
  BeginUpdate(GetDialogWindow(gGeneration));
  EndUpdate(GetDialogWindow(gGeneration));

  gCurrent = gGeneration;
  SetCCursor(sword);

  calcw(view);

  characterl = c[view];

  compactheap();

  charselectnew = view;
  ShowStats(1);

  GetDialogItem(gGeneration, 68, &itemType, &itemHandle, &itemRect);
  plotportrait(c[view].pictid, itemRect, c[view].caste, view);

  GetDialogItem(gGeneration, 80, &itemType, &itemHandle, &itemRect);
  ploticon3(c[view].iconid, itemRect);

  for (t = 0; t <= charnum; t++) {
    GetDialogItem(gGeneration, 73 + t, &itemType, &itemHandle, &itemRect);
    if (t == view)
      ploticon3(129, itemRect);
    InsetRect(&itemRect, 9, 9);
    plotportrait(c[t].pictid, itemRect, c[t].caste, -1);
    GetDialogItem(gGeneration, 90 + t, &itemType, &itemHandle, &buttonrect);
    downbutton(FALSE);
    GetDialogItem(gGeneration, 84 + t, &itemType, &itemHandle, &itemRect);
    ploticon3(c[t].iconid, itemRect);
  }

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(gGeneration, itemHit, &itemType, &itemHandle, &buttonrect);

    if ((changename) && (itemHit != 67))
      goto changenamenow;

    if ((itemHit == 3) && (look != NIL)) {
      downbutton(FALSE);
      if (showcondition(view, view, 1, 0, view) > -1) {
        characterl = c[view];
        ShowStats(1);
      }
      upbutton(FALSE);
    }

    if ((itemHit == 2) && (look != NIL)) {
      downbutton(FALSE);
      showcondition(view, view, 0, 0, view);
      upbutton(FALSE);
    }

    if ((itemHit == 4) && (look != NIL)) {
      downbutton(FALSE);
      characterl = c[view];
      showspecial();
      gCurrent = gGeneration;
      ShowStats(1);
    }

    if ((itemHit == 5) && (look != NIL)) /***** show special Plus Abilities ****/
    {
      downbutton(FALSE);
      showcondition(view, view, 2, 0, view);
      upbutton(FALSE);
    }

    if ((itemHit > 83) && (itemHit < 90) && (mode != 1) && ((itemHit - 84) <= charnum)) {
      itemHit -= 11;
    }

    if (itemHit == 97) /******** view race **********/
    {
      characterl = c[view];
      Race(1); /********** mode == 1 = view only *********/
      goto refresh;
    }

    if (itemHit == 96) /******** view caste **********/
    {
      characterl = c[view];
      Caste(1); /********** mode == 1 = view only *********/
      goto refresh;
    }

    if (itemHit == 100) /******** view prestige **********/
    {
      if (!reducesound)
        sound(3000);
      aging = GetNewDialog(177, 0L, (WindowPtr)-1L);
      SetPortDialogPort(aging);
      gCurrent = aging;
      BackPixPat(base);

      MoveWindow(GetDialogWindow(aging), GlobalLeft + (leftshift / 2), GlobalTop + (downshift / 2), FALSE);
      ShowWindow(GetDialogWindow(aging));
      ErasePortRect();

      ForeColor(yellowColor);
      DrawDialog(aging);
      BeginUpdate(GetDialogWindow(aging));
      EndUpdate(GetDialogWindow(aging));

      for (t = 0; t <= charnum; t++) {
        GetDialogItem(aging, 78 + t, &itemType, &itemHandle, &itemRect);
        plotportrait(c[t].pictid, itemRect, c[t].caste, -1);
        if (t == view) {
          GetDialogItem(aging, 72 + t, &itemType, &itemHandle, &itemRect);
          ploticon3(129, itemRect);
        }
      }

    updatestuff:

      GetDialogItem(aging, 86, &itemType, &itemHandle, &itemRect);
      EraseRect(&itemRect);
      plotportrait(c[view].pictid, itemRect, c[view].caste, view);

      GetDialogItem(aging, 87, &itemType, &itemHandle, &itemRect);
      EraseRect(&itemRect);
      ploticon3(c[view].iconid, itemRect);

      prestige = 0;

      prestige += (c[view].hitsgiven - (2 * c[view].hitstaken));
      prestige += (c[view].umissed - (2 * c[view].imissed));
      prestige += (-3 * c[view].spellscast) + (3 * c[view].turns) + (2 * c[view].destroyed);
      prestige += (3 * c[view].kills) + (-75 * c[view].deaths) + (-35 * c[view].knockouts);

      prestige += ((c[view].damagegiven - c[view].damagetaken) / 20);
      prestige -= (c[view].prestigepenelty);

      DialogNumLong(6, prestige);

      RGBForeColor(&greencolor);
      DialogNumLong(26, c[view].damagegiven);
      DialogNumLong(27, c[view].hitsgiven);
      DialogNumLong(42, c[view].umissed);
      DialogNumLong(28, c[view].kills);
      DialogNumLong(30, c[view].destroyed);
      DialogNumLong(35, c[view].turns);

      ForeColor(whiteColor);
      DialogNumLong(71, -(c[view].prestigepenelty));
      DialogNumLong(33, c[view].spellscast);
      DialogNumLong(31, c[view].damagetaken);
      DialogNumLong(32, c[view].hitstaken);
      DialogNumLong(43, c[view].imissed);
      DialogNumLong(29, c[view].knockouts);
      DialogNumLong(34, c[view].deaths);

      FlushEvents(everyEvent, 0);

      ForeColor(yellowColor);
      TextSize(20);
      TextFace(bold);
      TextFont(font);

      CtoPstr(c[view].name);
      MyrPascalDiStr(37, (StringPtr)c[view].name);
      PtoCstr((StringPtr)c[view].name);

      TextSize(12);
      TextFace(0);
      TextFont(genevafont);

      for (;;) {
        ModalDialog(0L, &itemHit);

        if ((itemHit > 71) && (itemHit < 78) && ((itemHit - 72) <= charnum)) {
          GetDialogItem(aging, itemHit, &itemType, &itemHandle, &buttonrect);
          GetDialogItem(aging, 72 + view, &itemType, &itemHandle, &itemRect);
          ploticon3(130, itemRect);
          ploticon3(129, buttonrect);
          sound(141);

          view = itemHit - 72;
          goto updatestuff;
        }

        if (itemHit == 58) {
          DisposeDialog(aging);
          goto refresh;
        }
      }
    }

    if (itemHit == 67) {
      changename = TRUE; /******** change character name ********/
      ForeColor(whiteColor);
      GetDialogItem(gGeneration, 67, &itemType, &itemHandle, &buttonrect);
      GetDialogItemText(itemHandle, myString);
      MyrPascalDiStr(67, myString);
    } else if (changename == TRUE) {

    changenamenow:

      changename = FALSE;
      if (question(-7)) {
        minus(c[view].name, 0);
        GetDialogItem(gGeneration, 67, &itemType, &itemHandle, &buttonrect);
        GetDialogItemText(itemHandle, myString);
        PtoCstr(myString);

        characterl = c[view];
        strcpy(characterl.name, myString);
        savecharacter(-1);
        c[view] = characterl;
        ShowStats(1);
        needupdate = TRUE;
        flashmessage((StringPtr) "You should save your game now to make changes permient.", 50, 50, 0, 6000);
        goto refresh;
      }
    }

    if (itemHit == 98) /******** view aging **********/
    {
      if (!reducesound)
        sound(3000);
      loadprofile(c[view].race, 0);
      aging = GetNewDialog(191, 0L, (WindowPtr)-1L);
      SetPortDialogPort(aging);
      gCurrent = aging;
      MoveWindow(GetDialogWindow(aging), GlobalLeft + (leftshift / 2), GlobalTop + (downshift / 2), FALSE);
      ShowWindow(GetDialogWindow(aging));
      ErasePortRect();

      TextFace(bold);
      TextFont(font);
      ForeColor(yellowColor);
      BackPixPat(base);
      TextSize(18);

      DrawDialog(aging);
      BeginUpdate(GetDialogWindow(aging));
      EndUpdate(GetDialogWindow(aging));

      MyrCDiStr(7, (StringPtr) "Youth");
      MyrCDiStr(8, (StringPtr) "Young");
      MyrCDiStr(9, (StringPtr) "Prime");
      MyrCDiStr(10, (StringPtr) "Adult");
      MyrCDiStr(11, (StringPtr) "Senior");

      for (t = 0; t < 5; t++) {
        temp = c[view].age / 365;
        if (twixt(temp, races.agerange[t][0], races.agerange[t][1])) {
          ForeColor(whiteColor);
          c[view].currentagegroup = t + 1;
        } else
          ForeColor(yellowColor);
        DialogNum(12 + t * 17, races.agerange[t][0]);
        DialogNum(13 + t * 17, races.agerange[t][1]);

        for (tt = 0; tt < 15; tt++)
          DialogNumNZ(14 + (t * 17) + tt, races.agechange[t][tt]);
      }

      FlushEvents(everyEvent, 0);

      for (;;) {
        ModalDialog(0L, &itemHit);

        if ((itemHit > 72) && (itemHit < 79) && (mode != 1) && ((itemHit - 73) <= charnum)) {
          GetDialogItem(gGeneration, itemHit, &itemType, &itemHandle, &buttonrect);
          GetDialogItem(gGeneration, 73 + view, &itemType, &itemHandle, &itemRect);
          ploticon3(130, itemRect);
          ploticon3(129, buttonrect);
          sound(141);

          view = itemHit - 73;

          GetDialogItem(gGeneration, 15, &itemType, &itemHandle, &itemRect);
          pict(154, itemRect);

          calcw(view);
          characterl = c[view];
          ShowStats(1);
          GetDialogItem(gGeneration, 68, &itemType, &itemHandle, &itemRect);
          plotportrait(c[view].pictid, itemRect, c[view].caste, view);
          GetDialogItem(gGeneration, 79, &itemType, &itemHandle, &itemRect);
          pict(154, itemRect);
          GetDialogItem(gGeneration, 80, &itemType, &itemHandle, &itemRect);
          ploticon3(c[view].iconid, itemRect);
        } else {
          DisposeDialog(aging);
          goto refresh;
        }
      }
    }

    if ((itemHit > 72) && (itemHit < 79) && (mode != 1) && ((itemHit - 73) <= charnum)) {
      GetDialogItem(gGeneration, itemHit, &itemType, &itemHandle, &buttonrect);
      GetDialogItem(gGeneration, 73 + view, &itemType, &itemHandle, &itemRect);
      ploticon3(130, itemRect);
      ploticon3(129, buttonrect);
      sound(141);

      view = itemHit - 73;

      GetDialogItem(gGeneration, 15, &itemType, &itemHandle, &itemRect);
      pict(154, itemRect);

      calcw(view);
      characterl = c[view];
      ShowStats(1);
      GetDialogItem(gGeneration, 68, &itemType, &itemHandle, &itemRect);
      plotportrait(c[view].pictid, itemRect, c[view].caste, view);
      GetDialogItem(gGeneration, 79, &itemType, &itemHandle, &itemRect);
      pict(154, itemRect);
      GetDialogItem(gGeneration, 80, &itemType, &itemHandle, &itemRect);
      ploticon3(c[view].iconid, itemRect);
    }

    if ((itemHit == 72) || (itemHit == 1)) {
      GetDialogItem(gGeneration, 72, &itemType, &itemHandle, &itemRect);
      ploticon3(129, itemRect);
    out:
      sound(141);
      DisposeDialog(gGeneration);
      charselectnew = view;
      return;
    }
  }
}
