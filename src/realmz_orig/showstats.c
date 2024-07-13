#include "prototypes.h"
#include "variables.h"

/***************************** ShowStats ********************************/
void ShowStats(short showprestige) {
  short t, conditionindex = 0;
  long prestige = 0;
  char special[50], specialindex;

  gCurrent = gGeneration;
  SetPortDialogPort(gGeneration);
  BackPixPat(base);

  if (showprestige) /*********** calculate and show prestige points **********/
  {
    TextFont(defaultfont);
    ForeColor(whiteColor);
    TextSize(12);
    TextFace(0);
    prestige = 0;

    DialogNumLong(103, characterl.kills);
    DialogNumLong(104, characterl.damagegiven);

    prestige += (characterl.hitsgiven - (2 * characterl.hitstaken));
    prestige += (characterl.umissed - (2 * characterl.imissed));
    prestige += (-3 * characterl.spellscast) + (3 * characterl.turns) + (2 * characterl.destroyed);
    prestige += (3 * characterl.kills) + (-75 * characterl.deaths) + (-35 * characterl.knockouts);

    prestige += ((characterl.damagegiven - characterl.damagetaken) / 20);
    prestige -= (characterl.prestigepenelty);

    DialogNumLong(101, prestige);
  }

  TextFont(font);
  TextSize(20);
  TextFace(bold);
  RGBForeColor(&cyancolor);
  DialogNum(19, characterl.st + characterl.magst);
  DialogNum(20, characterl.in);
  DialogNum(21, characterl.wi);
  DialogNum(22, characterl.de);
  DialogNum(23, characterl.co + characterl.magco);
  DialogNum(24, characterl.lu + characterl.maglu);

  temp = 0;
  if (characterl.condition[21])
    temp += 15; /**** strong ***/
  if (characterl.condition[6])
    temp -= 15; /**** slow ***/
  if (characterl.condition[4])
    temp += 5; /**** bless ***/
  if (characterl.condition[3])
    temp -= 5; /**** curse ***/
  if (characterl.condition[2])
    temp -= characterl.condition[2]; /*** tangled ***/
  if (characterl.condition[36])
    temp -= characterl.condition[36]; /*** Hinder atk ***/
  temp += (characterl.damage * 5); /*** Hinder atk ***/

  if (temp > 99)
    TextSize(16);
  DialogNum(25, temp); /*** attack bonus ****/
  TextSize(20);

  temp = 0;
  if (characterl.condition[24])
    temp += 10; /*** invisible ***/
  if (characterl.condition[6])
    temp -= 15; /*** slow ***/
  if (characterl.condition[4])
    temp += 5; /*** bless ***/
  if (characterl.condition[3])
    temp -= 5; /*** curse ***/
  if (characterl.condition[37])
    temp -= characterl.condition[37]; /*** hinder defense ***/
  if (characterl.condition[38])
    temp += characterl.condition[38]; /*** defense bonus ***/
  temp += characterl.ac; /*** Hinder atk ***/
  if (temp > 99)
    TextSize(16);
  DialogNum(26, temp); /*** defense bonus ****/
  TextSize(20);

  templong = characterl.age / 365;
  DialogNumLong(71, templong);

  TextSize(20);

  ForeColor(yellowColor);
  DialogNum(64, characterl.money[0]);
  DialogNum(65, characterl.money[1]);
  DialogNum(66, characterl.money[2]);

  TextSize(16);
  DialogNumLong(45, characterl.exp);
  GetIndString(myString, 131, characterl.caste);
  MyrPascalDiStr(46, myString);
  GetIndString(myString, 129, characterl.race);
  MyrPascalDiStr(47, myString);
  DialogNum(48, characterl.movementmax);
  if (characterl.gender == 1)
    MyrCDiStr(49, (StringPtr) "Male");
  else
    MyrCDiStr(49, (StringPtr) "Female");

  DialogNum(50, characterl.load);
  DialogNum(51, characterl.loadmax);

  TextSize(20);
  TextFont(font);
  DialogNum(34, characterl.missile);
  DialogNum(35, characterl.dodge);

  TextSize(16);
  RGBForeColor(&cyancolor);
  DialogNum(36, characterl.handtohand);

  TextSize(20);
  DialogNum(32, characterl.level);
  if (characterl.spellpointsmax)
    DialogNum(33, getnumspells(characterl.spellcastertype, characterl.caste, characterl.level));
  else
    MyrCDiStr(33, (StringPtr) "\245\245\245\245");

  TextFont(font);
  CtoPstr(characterl.name);
  MyrPascalDiStr(67, (StringPtr)characterl.name);
  PtoCstr((StringPtr)characterl.name);

  TextFont(font);
  RGBForeColor(&cyancolor);
  DialogNum(52, characterl.damage);
  DialogNum(53, characterl.tohit);

  if ((characterl.spellcastertype) && (characterl.spellpointsmax)) {
    if (characterl.spellpointsmax > 999)
      TextSize(16);
    DialogNum(54, characterl.spellpoints);
    DialogNum(55, characterl.spellpointsmax);
  } else {
    MyrCDiStr(54, (StringPtr) "");
    MyrCDiStr(55, (StringPtr) "");
  }

  TextSize(20);
  TextFont(font);

  if (characterl.staminamax > 999)
    TextSize(16);
  DialogNum(56, characterl.stamina);
  DialogNum(57, characterl.staminamax);
  TextSize(20);
  DialogNum(58, characterl.ac);
  DialogNum(59, characterl.magres);

  switch (characterl.normattacks + characterl.attackbonus) {
    case 2:
      DialogNum(82, 1); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 3:
      DialogNum(82, 3); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 4:
      DialogNum(82, 2); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 5:
      DialogNum(82, 5); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 6:
      DialogNum(82, 3); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 7:
      DialogNum(82, 7); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 8:
      DialogNum(82, 4); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 9:
      DialogNum(82, 9); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 10:
      DialogNum(82, 5); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 11:
      DialogNum(82, 11); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 12:
      DialogNum(82, 6); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 13:
      DialogNum(82, 13); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 14:
      DialogNum(82, 7); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 15:
      DialogNum(82, 15); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 16:
      DialogNum(82, 8); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 17:
      DialogNum(82, 17); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    case 18:
      DialogNum(82, 9); /****** attacks per combatround **********/
      DialogNum(83, 1);
      break;

    case 19:
      DialogNum(82, 19); /****** attacks per combatround **********/
      DialogNum(83, 2);
      break;

    default:

      if (characterl.normattacks + characterl.attackbonus > 12) {
        MyrCDiStr(82, (StringPtr) ">"); /****** attacks per combatround **********/
        DialogNum(83, 10);
      } else {
        DialogNum(82, 1); /****** attacks per combatround **********/
        DialogNum(83, 1);
      }
  }

  TextSize(15);
  TextFont(font);
  ForeColor(yellowColor);
  specialindex = 0;

  for (t = 0; t < 8; t++)
    DialogNum(t + 37, characterl.save[t]);
  for (t = 0; t < 4; t++)
    MyrCDiStr(60 + t, (StringPtr) "");
  for (t = 0; t < 12; t++) {
    if (characterl.special[t]) {
      strcpy(special, (StringPtr) "");

      if (characterl.special[t] > 0)
        strcpy(special, (StringPtr) "+");
      else
        strcpy(special, (StringPtr) "");

      MyrNumToString(characterl.special[t], myString);
      // PtoCstr(myString);
      strncat(special, myString, 3);
      GetIndString(myString, 132, t + 1);
      PtoCstr(myString);
      strncat(special, myString, 45);
      CtoPstr(special);
      if (specialindex < 4)
        MyrPascalDiStr(60 + specialindex++, (StringPtr)special);
    }
  }
  TextSize(14);
  for (t = 0; t < 5; t++)
    MyrCDiStr(27 + t, (StringPtr) "");
  for (t = 0; t < 40; t++) {
    if (characterl.condition[t]) {
      GetIndString(myString, 133, t + 1);
      MyrPascalDiStr(27 + conditionindex++, myString);
      if (conditionindex == 5)
        return;
    }
  }
}
