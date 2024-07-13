#include "prototypes.h"
#include "variables.h"

/**************** updatemoney *****************************/
void updatemoney(short mode) {
  SetPortDialogPort(gswap);
  gCurrent = gswap;
  TextSize(22);
  TextFont(font);
  ForeColor(yellowColor);
  DialogNumLong(11, moneypool[0]);
  DialogNumLong(12, moneypool[1]);
  DialogNumLong(13, moneypool[2]);
  TextSize(14);
  if (!mode) {
    for (t = 0; t <= charnum; t++) {
      DialogNum(35 + 4 * t, c[t].money[0]);
      DialogNum(36 + 4 * t, c[t].money[1]);
      DialogNum(37 + 4 * t, c[t].money[2]);
      DialogNum(38 + 4 * t, c[t].movementmax);
    }
  }
}
