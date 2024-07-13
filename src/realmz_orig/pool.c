#include "prototypes.h"
#include "variables.h"

/**************************** pool *************************/
void pool(void) {
  short t;
  long tempjew;

  for (t = 0; t <= charnum; t++) {
    moneypool[0] += c[t].money[0];
    moneypool[1] += c[t].money[1];
    moneypool[2] += c[t].money[2];
    c[t].load -= c[t].money[0];
    c[t].load -= c[t].money[1];
    tempjew = c[t].money[2] * 15;
    c[t].load -= tempjew;
    c[t].money[0] = c[t].money[1] = c[t].money[2] = 0;
    movecalc(t);
    characterl = c[cl];
    if (cr != -1)
      characterr = c[cr];
  }
  sound(128);
}
