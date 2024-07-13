#include "prototypes.h"
#include "variables.h"

/************************** levelup *****************************/
short levelup(short character, short mode) {
  short stamina, magres, spellpoints, tohit, temp, tempdice, t;

  DialogRef levelup;
  if (!mode)
    sound(3002);

  if ((c[character].level > 6) && (!doreg())) {
    warn(40);
    return (0);
  }

  c[character].level++;
  characterl = c[character];

  stamina = spellpoints = tohit = magres = 0;

  needupdate = TRUE;

  if (Rand(100) < 15) {
    if ((!doreg3()) && (currentscenario > 10))
      scratch(452);
  }

  loadprofile(c[character].race, c[character].caste);

  /********** recalc number of attacks each time ***************/
  characterl.normattacks = races.numofattacks[0] + caste.bonusattacks;
  for (t = 0; t < 10; t++)
    if ((caste.attacks[t] <= characterl.level) && (caste.attacks[t]))
      characterl.normattacks++;

  /********** check for new conditions for caste due to new level ***************/
  for (t = 0; t < 40; t++) {
    if (caste.conditions[t] == characterl.level) {
      if (characterl.condition[t] > -1)
        characterl.condition[t] = -1;
      else
        characterl.condition[t]--;
    }
  }

  /************** check race limitation on max attacks **************/

  if (characterl.normattacks > (2 * races.numofattacks[1]))
    characterl.normattacks = (2 * races.numofattacks[1]);

  tohit = caste.tohit[1];
  characterl.tohit += caste.tohit[1];
  characterl.dodge += caste.dodge[1];
  characterl.handtohand += caste.hand2hand[1];
  if (caste.missile[1])
    characterl.missile += Rand(caste.missile[1]);

  /*********** set spellcastertype for level up for higher starters ************/
  if (caste.spellcasters[0][1])
    characterl.spellcastertype = 1;
  else if (caste.spellcasters[1][1])
    characterl.spellcastertype = 2;
  else if (caste.spellcasters[2][1])
    characterl.spellcastertype = 3;

  switch (characterl.spellcastertype) /********** only award spell points if they are above neccesary level but not 1st level *********/
  {
    case 1: /***** Sorcerer ****/
      if ((caste.spellcasters[0][1] <= characterl.level) && (characterl.level > 1))
        spellpoints = characterl.level + Rand(characterl.in + (characterl.wi / 2));
      break;

    case 2: /***** Priest ******/
      if ((caste.spellcasters[1][1] <= characterl.level) && (characterl.level > 1))
        spellpoints = characterl.level + Rand(characterl.wi + (characterl.in / 2));
      break;

    case 3: /***** Enchanter ****/
      if ((caste.spellcasters[2][1] <= characterl.level) && (characterl.level > 1))
        spellpoints = characterl.level + Rand(characterl.wi + (characterl.in / 2));
      break;
  }

  characterl.spellpoints += spellpoints;
  characterl.spellpointsmax += spellpoints;

  temp = 0;

  tempdice = caste.stamina[1];
  stamina = Rand(tempdice);

  if (characterl.co > 16) {
    temp = characterl.co - 16;
    if (temp > caste.maxstaminabonus)
      temp = caste.maxstaminabonus;
  }

  characterl.stamina += (stamina + temp);
  characterl.staminamax += (stamina + temp);

  stamina += temp; /******** need to show for levelup ***********/

  temp = characterl.wi + characterl.in + characterl.co;
  if (Rand(100) <= temp)
    magres++;
  characterl.magres += magres;

  if (!mode) {
    levelup = GetNewDialog(150, 0L, (WindowPtr)-1L);
    SetPortDialogPort(levelup);
    BackPixPat(base);
    MoveWindow(GetDialogWindow(levelup), GlobalLeft + 18, GlobalTop + 44, FALSE);
    ShowWindow(GetDialogWindow(levelup));
    ErasePortRect();
    gCurrent = levelup;
    TextFace(bold);
    TextFont(font);
    TextSize(16);
    ForeColor(yellowColor);

    DrawDialog(levelup);
    BeginUpdate(GetDialogWindow(levelup));
    EndUpdate(GetDialogWindow(levelup));

    GetDialogItem(levelup, 2, &itemType, &itemHandle, &itemRect);
    plotportrait(characterl.pictid, itemRect, characterl.caste, -1);
    ForeColor(yellowColor);
    CtoPstr(characterl.name);
    MyrPascalDiStr(4, (StringPtr)characterl.name);
    PtoCstr((StringPtr)characterl.name);
    DialogNum(5, stamina);
    DialogNum(6, spellpoints);
    DialogNum(7, magres);
    DialogNum(8, tohit);

    if (characterl.normattacks > 12)
      characterl.normattacks = 12;

    switch (characterl.normattacks) {
      case 0:
        MyrCDiStr(9, (StringPtr) "0 / 1");
        break;
      case 1:
        MyrCDiStr(9, (StringPtr) "1 / 2");
        break;
      case 2:
        MyrCDiStr(9, (StringPtr) "1 / 1");
        break;
      case 3:
        MyrCDiStr(9, (StringPtr) "3 / 2");
        break;
      case 4:
        MyrCDiStr(9, (StringPtr) "2 / 1");
        break;
      case 5:
        MyrCDiStr(9, (StringPtr) "5 / 2");
        break;
      case 6:
        MyrCDiStr(9, (StringPtr) "3 / 1");
        break;
      case 7:
        MyrCDiStr(9, (StringPtr) "7 / 2");
        break;
      case 8:
        MyrCDiStr(9, (StringPtr) "4 / 1");
        break;
      case 9:
        MyrCDiStr(9, (StringPtr) "9 / 2");
        break;
      case 10:
        MyrCDiStr(9, (StringPtr) "5 / 1");
        break;
      case 11:
        MyrCDiStr(9, (StringPtr) "11 / 2");
        break;
      case 12:
        MyrCDiStr(9, (StringPtr) "6 / 1");
        break;
    }
  }

  updatespec(0); /****** 0 = Levelup ******/

  if (!mode) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    DisposeDialog(levelup);
  }

  c[character] = characterl;
  return (1);
}
