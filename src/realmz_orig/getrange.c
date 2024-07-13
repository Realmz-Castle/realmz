#include "prototypes.h"
#include "variables.h"

/****************** getrange *****************/
short getrange(short body, short side, short flip) {
  short t, fromx, fromy, total = 0;
  Boolean bad;

  if (body < 10) {
    fromx = pos[body][0];
    fromy = pos[body][1];
  } else {
    fromx = monpos[body - 10][0];
    fromy = monpos[body - 10][1];
  }

  for (t = 0; t < maxloop + 2; t++)
    range[t] = 127;
  for (t = 0; t <= charnum; t++) {
    bad = FALSE;
    if (c[t].inbattle) {
      if ((c[t].stamina > 0) && (((c[t].traiter != side) && (flip != 4)) || ((c[t].traiter == side) && (flip == 4)))) {
        if (checkrange(body, pos[t][0], pos[t][1])) {
          if ((spellinfo.range1 + spellinfo.range2) > 0) /**** can be blocked ***/
          {
            if (!cansee(fromx, fromy, pos[t][0], pos[t][1]))
              bad = TRUE;
          }
          if (!bad) {
            range[t] = currange;
            total++;
          }
        }
        if (currange < range[maxloop + 1])
          range[maxloop + 1] = currange;
      }
    }
  }
  for (t = 0; t < nummon; t++) {
    bad = FALSE;
    if ((monster[t].stamina > 0) && (((monster[t].traiter != side) && (flip != 4)) || ((monster[t].traiter == side) && (flip == 4)))) {
      checkrange(body, monpos[t][0], monpos[t][1]);

      if ((spellinfo.range1 + spellinfo.range2) > 0) /**** can be blocked ***/
      {
        if (!cansee(fromx, fromy, monpos[t][0], monpos[t][1]))
          bad = TRUE;
      }

      if ((currange <= spellrange) && (!bad)) {
        range[t + 10] = currange;
        total++;
      }
      if (currange < range[maxloop + 1])
        range[maxloop + 1] = currange;
    }
  }
  return (total);
}
