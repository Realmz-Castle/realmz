#include "prototypes.h"
#include "variables.h"

/*************************** updatecharinfo *******************/
void updatecharinfo(void) {
  int enable_recomposite = WindowManager_SetEnableRecomposite(0);

  GrafPtr oldport;
  short temp, conditionindex = 0;
  GetPort(&oldport);
  SetPortDialogPort(charstat);

  gCurrent = charstat;
  RGBForeColor(&cyancolor);
  TextSize(20);
  TextFont(font);

  MyrCDiStr(4, (StringPtr) "");
  DialogNum(5, c[charselectnew].st + c[charselectnew].magst);
  DialogNum(6, c[charselectnew].in);
  DialogNum(7, c[charselectnew].wi);
  DialogNum(8, c[charselectnew].de);
  DialogNum(9, c[charselectnew].co + c[charselectnew].magco);
  DialogNum(10, c[charselectnew].lu + c[charselectnew].maglu);

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
  DialogNum(11, temp); /*** attack bonus ****/
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
  DialogNum(12, temp); /*** defense bonus ****/
  TextSize(20);
  ForeColor(yellowColor);
  DialogNum(26, c[charselectnew].movementmax);
  DialogNum(13, c[charselectnew].damage);

  if (c[charselectnew].staminamax > 999)
    TextSize(16);
  DialogNum(16, c[charselectnew].stamina);
  DialogNum(17, c[charselectnew].staminamax);
  TextSize(20);

  if (c[charselectnew].spellpointsmax) {
    if (c[charselectnew].spellpointsmax > 999)
      TextSize(16);
    DialogNum(14, c[charselectnew].spellpoints);
    DialogNum(15, c[charselectnew].spellpointsmax);
  } else {
    MyrCDiStr(14, (StringPtr) "");
    MyrCDiStr(15, (StringPtr) "");
  }

  TextSize(20);
  DialogNum(18, c[charselectnew].ac);
  DialogNum(19, c[charselectnew].magres);
  DialogNum(20, c[charselectnew].tohit);
  TextSize(16);
  DialogNum(25, c[charselectnew].load);
  DialogNum(28, c[charselectnew].loadmax);

  TextSize(14);
  RGBForeColor(&cyancolor);
  for (t = 29; t < 34; t++)
    MyrCDiStr(t, (StringPtr) "");

  for (t = 0; t < 40; t++) {
    if (c[charselectnew].condition[t]) {
      GetIndString(myString, 133, t + 1);
      MyrPascalDiStr(29 + conditionindex++, myString);
      if (conditionindex == 5)
        break;
    }
  }
  SetPort(oldport);

  WindowManager_SetEnableRecomposite(enable_recomposite);
}
