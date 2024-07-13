#include "prototypes.h"
#include "variables.h"

/***************************** share *************************/
void share(void) {
  Boolean taken;
  short type = 2;

  taken = TRUE;

  for (type = 2; type > -1; type--) {
    while (moneypool[type] > 0) {
      taken = FALSE;
      for (t = 0; t <= charnum; t++) {
        if ((c[t].load < c[t].loadmax) && (moneypool[type])) {
          taken = TRUE;
          if (type == 2)
            c[t].load += 15;
          else
            c[t].load++;

          c[t].money[type]++;
          moneypool[type]--;
        }
      }
      if (taken == FALSE)
        break;
    }
  }

  characterl.money[0] = c[cl].money[0];
  characterl.money[1] = c[cl].money[1];
  characterl.money[2] = c[cl].money[2];

  characterl.load = c[cl].load;

  for (t = 0; t <= charnum; t++)
    movecalc(t);

  sound(128);
}

/****************************** Rand ********************************/
short Rand(short range) {
  long rawResult;

  rawResult = Random();
  if (rawResult < 0)
    rawResult *= -1;
  return (1 + (rawResult * range) / 32768); /*** returns between 1 and range  ***/
}

/****************************** DialogNumLong ********************************/
void DialogNumLong(short theItem, long theNum) {
  GetDialogItem(gCurrent, theItem, &itemType, &itemHandle, &itemRect);
  MyrNumToString(theNum, myString);
  CtoPstr((Ptr)myString);
  SetDialogItemText(itemHandle, myString);
}

/****************************** DialogNum ********************************/
void DialogNum(short theItem, short theNum) {
  GetDialogItem(gCurrent, theItem, &itemType, &itemHandle, &itemRect);
  MyrNumToString(theNum, myString);
  CtoPstr((Ptr)myString);
  SetDialogItemText(itemHandle, myString);
}

/******************** getmovecost *****************************/
short getmovecost(short hit) {
  short movecost = 0;
  if (hit > 999)
    hit -= 1000;
  movecost = (mapstats[hit].time / 2) - 1;
  if (movecost < 1)
    movecost = 0;
  if (deltax)
    movecost++;
  if (deltay)
    movecost++;
  return (movecost);
}
