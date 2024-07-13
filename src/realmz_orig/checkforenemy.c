#include "prototypes.h"
#include "variables.h"

/*********************** checkforenemy *****************************/
short checkforenemy(short code) {
  Boolean bad, tag, kill;
  char num, t, tt, loopx, loopy, monx, mony, size; /************* dont use global size ************/
  short temp;

  behind = kill = size = tag = 0;

  if (q[up] < 0)
    return (NIL); /*** this combatant is already dead ***/

newenemy:

  if (killmon >= numenemy)
    return (NIL); /**** killed last creature, bail out or lock up buddy ***/
  if (q[up] > 9) {
    monx = monpos[monsterup][0];
    mony = monpos[monsterup][1];
    size = monster[monsterup].size;
  } else {
    monx = pos[charup][0];
    mony = pos[charup][1];
  }

  if (code == 2) {
    monx += deltax;
    mony += deltay;
  }

  num = 0;
  for (t = 0; t < 12; t++)
    enemy[t] = 0;
  for (t = 0; t < maxloop; t++)
    trace[code][t] = 0;

  loopx = loopy = -1;

  if ((size == 1) || (size == 3))
    loopy = -2;
  if ((size == 2) || (size == 3))
    loopx = -2;

  for (tt = loopy; tt < 2; tt++) {
    for (t = loopx; t < 2; t++) {
      temp = abs(field[fieldx + monx + t][fieldy + mony + tt]);
      if (temp < maxloop) {
        bad = FALSE;
        if (q[up] < 9) {
          if (temp < 9) {
            if ((c[temp].traiter != c[charup].traiter) && (c[temp].inbattle))
              bad = TRUE;
          } else if ((monster[temp - 10].traiter != c[charup].traiter) && (monster[temp - 10].stamina > 0))
            bad = TRUE;
        } else if (temp > 9) {
          if ((monster[temp - 10].stamina > 0) && (monster[monsterup].traiter != monster[temp - 10].traiter) && (temp != q[up]))
            bad = TRUE;
        } else if ((monster[monsterup].traiter != c[temp].traiter) && (c[temp].inbattle))
          bad = TRUE;

        if (bad) {
          if (!trace[code][temp]) {
            enemy[num++] = temp;
            trace[code][temp] = 1;
          }
        }
      }
    }
  }

  if (code == 1)
    return (num);
  if (code == 2) {
    num = 0;

    if (q[up] < 9) {
      if (!c[charup].condition[24]) {
        for (t = 0; t < maxloop; t++) {
          if ((trace[1][t]) && (!trace[2][t]))
            enemy[num++] = t;
        }
        if (num)
          behind = TRUE;
      }
    } else if (!monster[monsterup].condition[24]) {
      for (t = 0; t < maxloop; t++) {
        if ((trace[1][t]) && (!trace[2][t]))
          enemy[num++] = t;
      }
      if (num)
        behind = TRUE;
    }
  }

  for (ttt = 0; ttt < num; ttt++) {
    if (enemy[ttt] < 9) {
      if (((c[enemy[ttt]].guarding) || (code == 2)) && (!c[enemy[ttt]].condition[1])) {
        tag = TRUE;
        combatupdate2(enemy[ttt]);
        c[enemy[ttt]].guarding = FALSE;
        if (attack(enemy[ttt], q[up])) {
          num = tag = 0;
          kill = TRUE;
        }
      }
    } else {
      if (((monster[enemy[ttt] - 10].guarding) || (code == 2)) && (!monster[enemy[ttt] - 10].condition[1])) {
        tag = TRUE;
        combatupdate2(enemy[ttt]);
        monster[enemy[ttt] - 10].guarding = FALSE;
        attackloop = 0;
        if (attack2(enemy[ttt], q[up], 0)) {
          num = tag = 0;
          kill = TRUE;
        }
      }
    }
  }
  if (tag)
    combatupdate2(q[up]);

  if ((num) && (q[up] > 9) && (code != 2)) {
    monster[monsterup].target = enemy[Rand(num) - 1];
    for (attackloop = monster[monsterup].attacknum; attackloop < (monster[monsterup].noofattacks + monster[monsterup].bonusattack); attackloop++) {
      monster[monsterup].attacknum++;
      monster[monsterup].guarding = 0;
      monster[monsterup].movement -= 3;

      if (monster[monsterup].target < 0)
        goto newenemy;
      if (attack2(monsterup + 10, monster[monsterup].target, attackloop))
        goto newenemy;
    }
  }
  return (kill);
}
