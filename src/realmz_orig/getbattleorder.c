#include "prototypes.h"
#include "variables.h"

/***************** getbattleorder *******************/
void getbattleorder(short suprise) {
  short temp, temp2, t, test, possible[6], possibleindex, left, adjust;

  left = charnum - killparty;

  test = possibleindex = up = killmon = adjust = 0;
  for (t = 0; t <= charnum; t++) {
    if (c[t].stamina > 0) {
      possible[possibleindex++] = t;
      if (c[t].condition[25]) {
        left++;
        adjust++;
      }
    }
  }

  aimindex = 2;

  for (t = 0; t < maxloop; t++)
    q[t] = -1;

  switch (suprise) {

    case 1: /****** good suprise *****/

      for (t = 0; t < possibleindex; t++)
        q[t + 1] = possible[t];
      goto pushon;
      break;

    case -1: /****** bad suprise *****/

      for (t = 0; t < possibleindex; t++)
        q[maxloopminus - t] = possible[t];
      goto pushon;
      break;

    default: /***** nosuprise *****/

      while (left + 1) {
        temp = Rand(charnum + nummon - killparty + 1 + adjust);
        if (q[temp] == -1) {
          q[temp] = possible[possibleindex-- - 1];
          left--;
        }
      }
    pushon:
      left = nummon;
      while (left) {
        temp = Rand(charnum + nummon - killparty + 1 + adjust);
        if (q[temp] == -1)
          q[temp] = 9 + left--;
      }

      if (!suprise) /**** shift for Agility *******/
      {
        for (t = 1; t <= nummon + charnum; t++) {
          for (tt = t; tt <= nummon + charnum; tt++) {
            if (q[tt] > -1) {
              if (q[tt] < 10)
                temp = c[q[tt]].de;
              else
                temp = monster[q[tt] - 10].dx;

              temp2 = 0;

              if ((q[tt + 1] < 10) && (q[tt + 1] > -1))
                temp2 = c[q[tt + 1]].de;
              else if (q[tt + 1] > 9)
                temp2 = monster[q[tt + 1] - 10].dx;

              if (temp2 > temp) {
                temp = q[tt];
                q[tt] = q[tt + 1];
                q[tt + 1] = temp;
              }
            }
          }
        }
      }
      break;
  }

  for (t = 1; t < maxloop; t++)
    if (q[t] < 6) {
      if (q[t] > -1)
        c[q[t]].position = t;
    }
}
