#include "prototypes.h"
#include "variables.h"

/***************************** Caste ********************************/
void Caste(short mode) /********** if mode == 1 then view only **********/
{
  DialogRef gGeneration;
  short conditionindex;
  short popupcons[30];

  gGeneration = GetNewDialog(131, 0L, (WindowPtr)-1L);
  SetPortDialogPort(gGeneration);
  gCurrent = gGeneration;
  MoveWindow(GetDialogWindow(gGeneration), GlobalLeft + (leftshift / 2), GlobalTop + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(gGeneration));
  DrawDialog(gGeneration);
  BeginUpdate(GetDialogWindow(gGeneration));
  EndUpdate(GetDialogWindow(gGeneration));

  if (!mode) {
    characterl.caste = 1;
    characterl.pictid = 257;
  }

  for (t = 0; t < 30; t++) {
    loadprofile(0, t + 1); /**************** popup icon values ***************/
    popupcons[t] = caste.defaulticon;
  }

update:

  GetDialogItem(gGeneration, 74, &itemType, &itemHandle, &itemRect);
  plotportrait(characterl.pictid, itemRect, characterl.caste, -1);

  TextFace(bold);
  TextFont(font);
  ForeColor(yellowColor);
  BackPixPat(base);
  TextSize(20);

  loadprofile(0, characterl.caste); /**************** load caste profile ***************/

  for (t = 2; t <= 13; t++)
    DialogNum(t, caste.minmax[t - 2]);
  for (t = 14; t <= 19; t++)
    DialogNumNZ(t, caste.attbonus[t - 14]);

  GetIndString(myString, 131, characterl.caste);
  MyrPascalDiStr(58, myString);

  conditionindex = 0;

  DialogNumNZ(44, caste.movebonus);
  DialogNum(45, caste.magres);
  DialogNumNZ(46, caste.twohand);

  TextSize(16);

  switch (caste.minimumagegroup) {
    default:
      MyrCDiStr(47, (StringPtr) "Youth");
      break;

    case 2:
      MyrCDiStr(47, (StringPtr) "Young");
      break;

    case 3:
      MyrCDiStr(47, (StringPtr) "Prime");
      break;

    case 4:
      MyrCDiStr(47, (StringPtr) "Adult");
      break;

    case 5:
      MyrCDiStr(47, (StringPtr) "Senior");
      break;
  }

  switch (caste.bonusattacks) {
    default:
      MyrCDiStr(49, (StringPtr) "0");
      break;

    case 1:
      MyrCDiStr(49, (StringPtr) "1/2");
      break;

    case 2:
      MyrCDiStr(49, (StringPtr) "1");
      break;

    case 3:
      MyrCDiStr(49, (StringPtr) "1 1/2");
      break;

    case 4:
      MyrCDiStr(49, (StringPtr) "2");
      break;
  }

  TextSize(20);
  DialogNum(50, caste.maxattacks);
  DialogNum(51, caste.maxstaminabonus);

  if (caste.canusemissile)
    MyrCDiStr(37, (StringPtr) "Yes");
  else
    MyrCDiStr(37, (StringPtr) "No");
  if (caste.getsmissilebonus)
    MyrCDiStr(38, (StringPtr) "Yes");
  else
    MyrCDiStr(38, (StringPtr) "No");

  DialogNum(39, caste.stamina[0]);
  DialogNum(89, caste.stamina[1]);
  DialogNum(40, caste.tohit[0]);
  DialogNum(90, caste.tohit[1]);
  DialogNum(41, caste.missile[0]);
  DialogNum(91, caste.missile[1]);
  DialogNum(42, caste.dodge[0]);
  DialogNum(92, caste.dodge[1]);
  DialogNum(43, caste.hand2hand[0]);
  DialogNum(93, caste.hand2hand[1]);

  if (caste.spellcasters[0][0]) {
    DialogNum(94, caste.spellcasters[0][1]);
    DialogNum(97, caste.spellcasters[0][2]);
  } else {
    MyrCDiStr(94, (StringPtr) "");
    MyrCDiStr(97, (StringPtr) "");
  }

  if (caste.spellcasters[1][0]) {
    DialogNum(95, caste.spellcasters[1][1]);
    DialogNum(98, caste.spellcasters[1][2]);
  } else {
    MyrCDiStr(95, (StringPtr) "");
    MyrCDiStr(98, (StringPtr) "");
  }

  if (caste.spellcasters[2][0]) {
    DialogNum(96, caste.spellcasters[2][1]);
    DialogNum(99, caste.spellcasters[2][2]);
  } else {
    MyrCDiStr(96, (StringPtr) "");
    MyrCDiStr(99, (StringPtr) "");
  }

  for (t = 59; t <= 72; t++) {
    DialogNum(t, caste.specialability[0][t - 59]);
    DialogNumNZ(t + 16, caste.specialability[1][t - 59]);
  }

  TextSize(16);

  for (t = 28; t <= 35; t++)
    DialogNumNZ(t, caste.drvbonus[t - 28]);

  TextSize(14);

  for (t = 0; t < 4; t++) {
    MyrCDiStr(52 + t, (StringPtr) "");
    MyrCDiStr(100 + t, (StringPtr) "");
  }

  for (t = 0; t < 40; t++) {
    if (caste.conditions[t]) {
      GetIndString(myString, 133, t + 1);
      MyrPascalDiStr(52 + conditionindex, myString);
      DialogNum(100 + conditionindex++, caste.conditions[t]);
      if (conditionindex == 4)
        goto moveon;
    }
  }

moveon:

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);

    if ((itemHit == 57) || (itemHit == 74)) {
      popup = GetMenu(131);
      InsertMenu(popup, -1);

      for (t = 1; t <= 30; t++) {
        GetIndString(myString, 131, t);

        if (StringWidth(myString)) {
          AppendMenu(popup, myString);
          SetItemIcon(popup, t, popupcons[t - 1]);
        }
      }

      GetMouse(&point);
      LocalToGlobal(&point);
      itemHit = PopUpMenuSelect(popup, point.v, point.h, 0);
      if (itemHit)
        characterl.pictid = popupcons[itemHit - 1];
      DeleteMenu(131);
      ReleaseResource((Handle)popup);

      if (itemHit) {
        characterl.caste = itemHit;
        goto update;
      }
    }

    if ((itemHit == 56) || (itemHit == 1)) {
      GetDialogItem(gGeneration, 56, &itemType, &itemHandle, &buttonrect);
      ploticon3(133, buttonrect);
      sound(6001);
      goto out;
    }
  }
out:
  if (background != NIL)
    SetPortDialogPort(background);
  DisposeDialog(gGeneration);
  if (background != NIL)
    DrawDialog(background);
}
