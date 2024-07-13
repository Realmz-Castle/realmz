#include "prototypes.h"
#include "variables.h"
// #include "fp.h"
#if defined(REALMZ_COCOA)
#include <math.h>
#endif

/******************** checkrange *************************/
short checkrange(short body, short targetx, short targety) {
  short origx, origy;
  float changex, changey;

  if (body > 9) {
    origx = monpos[body - 10][0];
    origy = monpos[body - 10][1];
  } else {
    origx = pos[body][0];
    origy = pos[body][1];
  }

  changex = targetx - origx;
  changey = targety - origy;

  currange = (changex * changex) + (changey * changey);
  currange = sqrt(currange);
  temp = currange;

  if (temp > spellrange)
    return (FALSE);
  else
    return (TRUE);
}

/******************* checkforitem ************************************/
short checkforitem(short id, short usecharge, short who) /***** who = -1 is anybody ****/
{
  short tt, ttt, start = 0;
  short finish = charnum;
  short reply = 0;

  if (who != -1)
    finish = start = who;

  for (tt = start; tt <= finish; tt++) {
    for (ttt = 0; ttt < c[tt].numitems; ttt++) {
      if (c[tt].items[ttt].id == id) {
        reply = tt + 1;
        theItem = ttt;
        if ((usecharge) && (c[tt].items[ttt].charge > 0)) {
          loaditem(id);
          c[tt].items[ttt].charge--;
          if (item.xcharge)
            c[tt].load -= item.xcharge;
          if ((!c[tt].items[ttt].charge) && (item.drop))
            dropitem(tt, id, ttt, TRUE, FALSE);
        } else if (usecharge) {
          reply = 0;
        }
        goto out;
      }
    }
  }
out:
  return (reply);
}

/******************* checkforcharge ************************************/
short checkforcharge(short id) /***** ID if items to add up charges ****/
{
  short tt, ttt, charge = 0;

  for (tt = 0; tt <= charnum; tt++) {
    for (ttt = 0; ttt < c[tt].numitems; ttt++) {
      if (c[tt].items[ttt].id == id) {
        charge += c[tt].items[ttt].charge;
      }
    }
  }

  return (charge);
}
