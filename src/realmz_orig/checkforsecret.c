#include "prototypes.h"
#include "variables.h"

/*********************** checkforsecret ********************************/
short checkforsecret(short force) /****** 2 = trap/pit one square only, 1 = Secret all around *****/
{
  short t, tt, chancesecret = 0;
  short secretcheck = 3840;
  reply = 0;
  if (!indung) {
    fieldx = partyx + lookx - 1;
    fieldy = partyy + looky - 1;
  } else {
    fieldy = floorx - 1;
    fieldx = floory - 1;
  }

  for (t = 0; t <= charnum; t++) {
    chancesecret += c[t].spec[4];
  }

  chancesecret /= (charnum + 1);

  if (partycondition[2])
    chancesecret = 100;

  if ((partycondition[5]) || (force)) {
    chancesecret = 100;
    timeclick(4, TRUE);
    if (revertgame)
      return (0);
  }

  for (tt = fieldy; tt < fieldy + 3; tt++) {
    for (t = fieldx; t < fieldx + 3; t++) {
      if ((t > -1) && (tt > -1) && (tt < 90) && (t < 90)) {
        reply++;
        if (!indung) {
          temp = abs(field[t][tt]);
          MyrBitClrShort(&temp, 1); /*** clear note ***/
          MyrBitClrShort(&temp, 2); /*** clear path ***/

          if (temp > 2999) {
            if (Rand(100) <= chancesecret) {
              if (field[t][tt] > 0)
                field[t][tt] -= 1000;
              else
                field[t][tt] += 1000;

              centerpict();
              GetIndString(myString, 138, reply);
              PtoCstr(myString); // Myriad
              flashmessage(myString, 30, 375, 0, 5000);
              if ((t == fieldx + 1) && (tt == fieldy + 1))
                return (TRUE);
            }
          }
        } else {
          if ((BitAnd(field[t][tt], secretcheck)) && (!MyrBitTstShort(&field[t][tt], 9))) {
            if (Rand(100) <= chancesecret) {
              MyrBitSetShort(&field[t][tt], 9); /**** show S ****/
              MyrBitSetShort(&field[t][tt], 2); /**** show arch ****/

              GetIndString(myString, 139, reply);
              PtoCstr(myString); // Myriad
              flashmessage(myString, 30, 375, 0, 5000);
              return (TRUE);
            }
          }
        }
      }
    }
  }
  return (FALSE);
}
