#include "prototypes.h"
#include "variables.h"

/******************** takegold ******************/
short takegold(short gold, short type) {
  short t;
  char charindex = 0;
  unsigned short maxgold[2];
  unsigned short tempgold = gold;

  if (gold < moneypool[type]) {
    moneypool[type] -= gold;
    return (TRUE);
  } else {
    maxgold[0] = maxgold[1] = 0;

    for (t = 0; t <= charnum; t++) {
      maxgold[0] += c[t].money[0];
      maxgold[1] += c[t].money[1];
    }

    if (gold > maxgold[type] + moneypool[type]) {
      warn(50);
      return (FALSE);
    }

    tempgold -= moneypool[type];
    moneypool[type] = 0;
    do {
      if (c[charindex].money[type]) {
        tempgold--;
        c[charindex].money[type]--;
        c[charindex].load--;
      }
      charindex++;
      if (charindex > charnum)
        charindex = 0;
    } while (tempgold);
  }
  return (TRUE);
}
