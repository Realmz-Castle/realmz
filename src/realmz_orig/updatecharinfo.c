#include "prototypes.h"
#include "variables.h"

/*************************** updatecharinfo *******************/
void updatecharinfo(void) {
  int enable_recomposite = WindowManager_SetEnableRecomposite(0);

  GrafPtr oldport;
  short temp, conditionindex = 0, equipped;
  struct itemattr saveditem;
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
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
  * NOTE(chromancer): Original displayed attack bonus (to-hit) as "(some) conditions + character.damage * 5",
  * not accurate to game combat logic or intended behavior. Following original intent, we:
  * Calculate the attack bonus from conditions, base to-hit, then 5 * equipped (double for penetration)
  * Add absolute values for conditions that use negative values to indicate permanent status.
  * Complete the defense bonus to match the manual's "all spell effects, conditions".
  */
  saveditem = item;
  equipped = 0;
  for (t = 0; (t < c[charselectnew].numitems) && (t < 30); t++)
    if (c[charselectnew].items[t].equip) {
      loaditem(c[charselectnew].items[t].id);
      equipped += item.damage;
      if (item.sp1 == 121)
        equipped += item.damage; // Double to-hit for penetration
    }
  item = saveditem;

  temp = 0;
  if (c[charselectnew].condition[COND_STRONG])
    temp += 15; /**** strong ***/
  if (c[charselectnew].condition[COND_SLOW])
    temp -= 15; /**** slow ***/
  if (c[charselectnew].condition[COND_CONFUSED])
    temp -= 10;
  if (c[charselectnew].condition[COND_BLIND])
    temp -= 15;
  if (c[charselectnew].condition[COND_MAGIC_AURA])
    temp += 5;
  if (c[charselectnew].condition[COND_CURSED])
    temp -= 5; /**** curse ***/
  if (c[charselectnew].condition[COND_TANGLED])
    temp -= abs(c[charselectnew].condition[COND_TANGLED]); /*** tangled ***/
  if (c[charselectnew].condition[COND_HINDERED_ATTACKS])
    temp -= abs(c[charselectnew].condition[COND_HINDERED_ATTACKS]); /*** Hinder atk ***/
  temp += c[charselectnew].tohit + (5 * equipped);
  if (temp > 99)
    TextSize(16);
  DialogNum(11, temp); // Total displayed to-hit bonus (UI label "Attack Bonus").
  TextSize(20);

  temp = 0;
  if (c[charselectnew].condition[COND_INVISIBLE])
    temp += 10; /*** invisible ***/
  if (c[charselectnew].condition[COND_SLOW])
    temp -= 15; /*** slow ***/
  if (c[charselectnew].condition[COND_CONFUSED])
    temp -= 10;
  if (c[charselectnew].condition[COND_BLIND])
    temp -= 15;
  if (c[charselectnew].condition[COND_MAGIC_AURA])
    temp += 5;
  if (c[charselectnew].condition[COND_CURSED])
    temp -= 5; /*** curse ***/
  if (c[charselectnew].condition[COND_TANGLED])
    temp -= abs(c[charselectnew].condition[COND_TANGLED]);
  if (c[charselectnew].condition[COND_HINDERED_DEFENSE])
    temp -= abs(c[charselectnew].condition[COND_HINDERED_DEFENSE]); /*** hinder defense ***/
  if (c[charselectnew].condition[COND_DEFENSE_BONUS])
    temp += abs(c[charselectnew].condition[COND_DEFENSE_BONUS]);
  temp += c[charselectnew].ac;
  if (c[charselectnew].condition[COND_SHIELD_FROM_HITS])
    temp += 2 * abs(c[charselectnew].condition[COND_SHIELD_FROM_HITS]);
  if (temp > 99)
    TextSize(16);
  DialogNum(12, temp); // Total displayed defense bonus (UI label "Defense Bonus").
  
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
