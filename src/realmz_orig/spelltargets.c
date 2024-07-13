#include "prototypes.h"
#include "variables.h"

/******************* spelltargets **********************/
short spelltargets(short targetnum, short who, short mode, short noreflect) {
  register short t, tt;
  short targetcount = 0;
  short startx, starty, temptraiter, miss, hit, temp, spellsize = 1;

redo:

  hit = miss = 0;

  for (t = 0; t < maxloop; t++)
    track[t] = 0;

  if (spellinfo.targettype == 4)
    spellsize = powerlevel;
  if (spellinfo.targettype == 3)
    spellsize = spellinfo.size;
  if (spellinfo.canrotate)
    spellsize += rotate;

  if ((spellinfo.queicon) && (!mode)) /******** add que spell ***********/
  {
    for (t = 0; t < 60; t++) {
      if (!que[t].duration) {
        que[t].phase = up;
        que[t].powerlevel = powerlevel;
        que[t].castlevel = castlevel;
        que[t].rotate = 0;
        if (spellinfo.canrotate)
          que[t].rotate = rotate;
        que[t].size = spellsize;
        que[t].x = target[0][0] - 3;
        que[t].y = target[0][1] - 3;
        que[t].bounds.top = que[t].y;
        que[t].bounds.left = que[t].x;
        que[t].bounds.right = que[t].bounds.left + 7;
        que[t].bounds.bottom = que[t].bounds.top + 7;
        que[t].spellnum = 1000 * castcaste + 100 * castlevel + castnum + 1101;
        que[t].duration = randrange(spellinfo.duration1, spellinfo.duration2);
        for (tt = 0; tt < powerlevel; tt++)
          que[t].duration += randrange(spellinfo.powerdur1, spellinfo.powerdur2);
        que[t].icon = spellinfo.queicon;
        centerfield(5 + (2 * screensize), 5 + screensize);
        goto passout;
      }
    }
  }

passout:

  if (spellinfo.targettype > 1) {
    startx = target[0][0] - 3;
    starty = target[0][1] - 3;
  } else {
    if (target[targetnum][1] == 2) {
      startx = monpos[target[targetnum][0] - 10][0] - 3 + fieldx;
      starty = monpos[target[targetnum][0] - 10][1] - 3 + fieldy;
    } else {
      startx = pos[target[targetnum][0]][0] - 3 + fieldx;
      starty = pos[target[targetnum][0]][1] - 3 + fieldy;
    }
  }

  /*********  Adds ability to use type 9 and 10 and 12 spells for monster macros ***/
  if (spellinfo.targettype == 9) /****** all friendly ******/
  {
    for (t = 0; t <= charnum; t++)
      if ((!c[t].traiter) && (c[t].inbattle))
        track[t] = TRUE;
    for (t = 0; t < nummon; t++)
      if ((!monster[t].traiter) && (monster[t].stamina > 0))
        track[t + 10] = TRUE;
  } else if (spellinfo.targettype == 10) /****** all enemy ******/
  {
    for (t = 0; t <= charnum; t++)
      if ((c[t].traiter) && (c[t].inbattle))
        track[t] = TRUE;
    for (t = 0; t < nummon; t++)
      if ((monster[t].traiter) && (monster[t].stamina > 0))
        track[t + 10] = TRUE;
  } else if (spellinfo.targettype == 12) /**** everybody *****/
  {
    for (t = 0; t <= charnum; t++)
      if (c[t].inbattle)
        track[t] = TRUE;
    for (t = 0; t < nummon; t++)
      if (monster[t].stamina > 0)
        track[t + 10] = TRUE;
  } else {

    for (tt = 0; tt < 7; tt++) {
      for (t = 0; t < 7; t++) {
        if (spellarea[spellsize - 1][tt][t]) {
          temp = abs(field[t + startx][tt + starty]);

          if (((temp < maxloop) && (who == -1)) || (temp == who)) {
            if (spellinfo.spellclass != 9) {
              if ((temp < 9) && (c[temp].condition[30]) && (Rand(100) < 34) && (!noreflect))
                track[q[up]] = TRUE;
              else if ((temp > 9) && (monster[temp - 10].condition[30]) && (Rand(100) < 34) && (!noreflect))
                track[q[up]] = TRUE;
              else if ((temp > 9) && (monster[temp - 10].magres > 100))
                track[temp] = FALSE; /*** immune to ALL magic ***/
              else
                track[temp] = TRUE;
            } else if ((temp > 9) && (monster[temp - 10].magres > 100))
              track[temp] = FALSE; /*** immune to ALL magic ***/
            else
              track[temp] = TRUE;
          }
        }
      }
    }
  }

  if (mode) {
    temptraiter = monster[monsterup].traiter;

    for (t = 0; t <= charnum; t++) {
      if (track[t]) {
        if (c[t].traiter != temptraiter) {
          hit++;
          if (c[t].spellpoints)
            hit++;
        } else
          miss++;
      }
    }

    for (t = 10; t < 60; t++) {
      if (track[t]) {
        if (monster[t - 10].traiter != temptraiter) {
          hit += (1 + monster[t - 10].type[0]);
        } else
          miss++;
      }
    }

    if (spellinfo.cannot == 4) /**** good spell, flip results *******/
    {
      temp = hit;
      hit = miss;
      miss = temp;
    }

    for (t = 0; t < 60; t++)
      track[t] = 0;
    return (hit - miss);
  }
  inspell = TRUE;
  resolvespell();
  inspell = FALSE;
  if (q[up] < 10)
    centerfield(5 + (2 * screensize), 5 + screensize);
  return (0);
}
