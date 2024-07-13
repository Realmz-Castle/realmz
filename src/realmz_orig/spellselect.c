#include "prototypes.h"
#include "variables.h"

/****************** spellselect ******************************/
void Spellselect(void) {
  short spelllevel, oldlevel;
  short oldhit, t, tt, maxlevel;
  Rect light;
  short spellselectpoints = 0;
  DialogRef spellwindow;
  short definelevel, definespell;
  struct character lock;
  Boolean skiptest;
  short costs[7] = {1, 3, 6, 10, 15, 21, 28};

  sound(3000);

  spellselectpoints = getnumspells(characterl.spellcastertype, characterl.caste, characterl.level);

  oldlevel = 0;

  lock = characterl;

  maxlevel = caste.spellcasters[0][2] + caste.spellcasters[1][2] + caste.spellcasters[2][2];

  spellwindow = GetNewDialog(135, 0L, (WindowPtr)-1L);
  MoveWindow(GetDialogWindow(spellwindow), GlobalLeft + 1, GlobalTop + 1, FALSE);
  ShowWindow(GetDialogWindow(spellwindow));
  SetPortDialogPort(spellwindow);
  BackPixPat(base);
  ErasePortRect();
  gCurrent = spellwindow;
  ForeColor(yellowColor);

  DrawDialog(spellwindow);
  BeginUpdate(GetDialogWindow(spellwindow));
  EndUpdate(GetDialogWindow(spellwindow));

  GetDialogItem(spellwindow, 34, &itemType, &itemHandle, &itemRect);
  plotportrait(characterl.pictid, itemRect, characterl.caste, -1);
  ForeColor(yellowColor);
  TextSize(20);
  TextFont(font);

  CtoPstr(characterl.name);
  MyrPascalDiStr(33, (StringPtr)characterl.name);
  PtoCstr((StringPtr)characterl.name);
  TextSize(12);
  TextFont(defaultfont);

  for (t = 0; t < 7; t++)
    DialogNum(24 + t, costs[t]);

  spelllevel = 0;
  oldhit = 10;
  GetDialogItem(spellwindow, 23, &itemType, &itemHandle, &light);

  pict(131, light);

  GetDialogItem(spellwindow, 9, &itemType, &itemHandle, &itemRect);
  pict(130, itemRect);
  GetDialogItem(spellwindow, 2, &itemType, &itemHandle, &buttonrect);
  downbutton(FALSE);

  light.top += 3;
  light.left = 152;
  light.right = 161;
  light.bottom = light.top + 9;

  MyrCDiStr(36, (StringPtr) "");

  for (t = 0; t < 12; t++) {
    GetDialogItem(spellwindow, 10 + t, &itemType, &itemHandle, &buttonrect);
    buttonrect.left -= 2;
    buttonrect.top--;
    if (characterl.cspells[spelllevel][t]) {
      DrawPicture(on, &light);
      downbutton(FALSE);
    } else {
      upbutton(FALSE);
      ForeColor(yellowColor);
    }

    GetIndString(myString, 1000 * (characterl.spellcastertype) + spelllevel, t + 1);
    MyrPascalDiStr(10 + t, myString);
    light.top += 17;
    light.bottom += 17;
  }

  for (t = 0; t < 7; t++) {
    for (tt = 0; tt < 12; tt++) {
      if ((characterl.cspells[t][tt]) && ((spellselectpoints - costs[t]) > -1)) {
        spellselectpoints -= costs[t];
      } else if (characterl.cspells[t][tt]) {
        characterl.cspells[t][tt] = 0; /*** too many spells, erase it ****/
      }
    }
  }

  DialogNum(40, spellselectpoints);

  for (;;) {
  over:
    SetPortDialogPort(spellwindow);
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(spellwindow, itemHit, &itemType, &itemHandle, &buttonrect);
    if ((itemHit == 1) || (itemHit == 32)) {
      if (spellselectpoints) {
        if (!question(17))
          goto over;
      }
      ploticon3(129, buttonrect);
      goto out;
    }
    if ((itemHit < 9) && (itemHit > 1)) {
      if (itemHit - 1 > maxlevel) {
        warn(74);
        goto over;
      }

      GetDialogItem(spellwindow, 9, &itemType, &itemHandle, &itemRect);
      pict(130, itemRect);

      downbutton(FALSE);
      sound(137);
      spelllevel = itemHit - 2;
      GetDialogItem(spellwindow, 23, &itemType, &itemHandle, &light);
      pict(131, light);
      light.top += 2;
      light.bottom = light.top + 9;
      light.left = 152;
      light.right = 161;
      ForeColor(yellowColor);
      for (t = 0; t < 12; t++) {
        GetDialogItem(spellwindow, 10 + t, &itemType, &itemHandle, &buttonrect);
        buttonrect.left -= 2;
        buttonrect.top--;
        if (characterl.cspells[spelllevel][t])
          downbutton(FALSE);
        else {
          upbutton(FALSE);
          ForeColor(yellowColor);
        }

        if (characterl.cspells[spelllevel][t])
          DrawPicture(on, &light);

        light.top += 17;
        light.bottom += 17;

        GetIndString(myString, 1000 * (characterl.spellcastertype) + spelllevel, t + 1);
        MyrPascalDiStr(10 + t, myString);
      }
    }

    if ((itemHit > 9) && (itemHit < 22)) {
      ForeColor(yellowColor);
      GetIndString(myString, -(1000 * (characterl.spellcastertype) + spelllevel), itemHit - 9);
      MyrPascalDiStr(39, myString);

      GetDialogItemText(itemHandle, myString);
      skiptest = FALSE;
      if (((!lock.cspells[spelllevel][itemHit - 10]) && (characterl.cspells[spelllevel][itemHit - 10]) || (characterl.cspells[spelllevel][itemHit - 10]))) {
        buttonrect.left -= 2;
        buttonrect.top--;
        upbutton(FALSE);
        ForeColor(yellowColor);
        GetIndString(myString, 1000 * (characterl.spellcastertype) + spelllevel, itemHit - 9);
        MyrPascalDiStr(itemHit, myString);
        sound(144);
        light.top = 204 + (itemHit - 10) * 17;
        light.left = 152;
        light.right = 161;
        light.bottom = light.top + 9;
        DrawPicture(non, &light);
        characterl.cspells[spelllevel][itemHit - 10] = 0;

        spellselectpoints += costs[spelllevel];
        DialogNum(40, spellselectpoints);
        characterl.nspells[spelllevel]++;
        skiptest = TRUE;
      }
      if ((characterl.cspells[spelllevel][itemHit - 10] == 0) && (spellselectpoints >= costs[spelllevel]) && (!skiptest)) {
        buttonrect.left -= 2;
        buttonrect.top--;
        downbutton(FALSE);
        GetIndString(myString, 1000 * (characterl.spellcastertype) + spelllevel, itemHit - 9);
        MyrPascalDiStr(itemHit, myString);
        ForeColor(yellowColor);
        sound(145);
        light.top = 204 + (itemHit - 10) * 17;
        light.bottom = light.top + 9;
        light.left = 152;
        light.right = 161;
        DrawPicture(on, &light);

        spellselectpoints -= costs[spelllevel];
        DialogNum(40, spellselectpoints);
        characterl.cspells[spelllevel][itemHit - 10] = 1;
        characterl.nspells[spelllevel]--;
      }
    }
  }
out:

  characterl.canheal = characterl.canidentify = characterl.candetect = 0;
  for (t = 0; t < 7; t++) /************ update canheal,canidentify,candetect status ********/
  {
    for (tt = 0; tt < 12; tt++) {
      if (characterl.cspells[t][tt]) {
        loadspell(characterl.spellcastertype - 1, t, tt);
        if (spellinfo.special == 57)
          characterl.canheal = TRUE;
        if (spellinfo.special == 48)
          characterl.canidentify = TRUE;
        if (spellinfo.special == 63)
          characterl.candetect = TRUE;
      }
    }
  }

  for (t = 0; t < 10; t++) {
    definelevel = characterl.definespells[t][1];
    definespell = characterl.definespells[t][2];
    if (!characterl.cspells[definelevel][definespell]) {
      characterl.definespells[t][0] = 0;
      characterl.definespells[t][1] = 0;
      characterl.definespells[t][2] = 0;
      characterl.definespells[t][3] = 0;
    }
  }

  DisposeDialog(spellwindow);
}
