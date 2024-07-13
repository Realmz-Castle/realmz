#include "variables.h"

/********************* movemonster *************************/
short movemonster(void) {
  short tempcost, cost, temptarget, monhit[4], t, retreat = 1;
  short targetx, targety, monx, mony, numtry = 0;

  if (monster[monsterup].condition[0]) {
    retreat = -1;
    checkforenemy(1); /****** first check for retreat *******/
  }

  tempcost = cost = 0;
  temptarget = monster[monsterup].target;

  if (temptarget < 9) {
    targetx = pos[temptarget][0];
    targety = pos[temptarget][1];
  } else {
    targetx = monpos[temptarget - 10][0];
    targety = monpos[temptarget - 10][1];
  }

  monx = monpos[monsterup][0];
  mony = monpos[monsterup][1];

  size = monster[monsterup].size;

  deltax = deltay = 0;

  if (monx < targetx)
    deltax = 1 * retreat;
  if (monx > targetx)
    deltax = -1 * retreat;
  if (mony < targety)
    deltay = 1 * retreat;
  if (mony > targety)
    deltay = -1 * retreat;

  monster[monsterup].lr = deltax;
  monster[monsterup].up = deltay;

goback:

  if (numtry++ > 20) {
    monster[monsterup].target = -1;
    monster[monsterup].movement = 0;
    return (FALSE);
  }

  monhit[0] = monhit[1] = monhit[2] = monhit[3] = basetile[lastpix];

  monhit[0] = abs(field[monx + fieldx + deltax][mony + fieldy + deltay]);

  if ((size == 1) || (size == 3))
    monhit[1] = abs(field[monx + fieldx + deltax][mony + fieldy + deltay - 1]);
  if (size > 1)
    monhit[2] = abs(field[monx + fieldx + deltax - 1][mony + fieldy + deltay]);
  if (size == 3)
    monhit[3] = abs(field[monx + fieldx + deltax - 1][mony + fieldy + deltay - 1]);

  for (t = 0; t <= size; t++) {
    if ((!deltax) && (!deltay))
      goto shift;
    if ((size == 2) && (t == 1))
      t++;
    if (monhit[t] == q[up])
      monhit[t] = basetile[lastpix] + 1000;

    if (monhit[t] < 1000)
      goto shift; /*** hit some other creatures or player ***/
    else if (mapstats[(monhit[t] - 1000)].solid) /**** hit something solid on battlefield ******/
    {
      if ((!size) || ((mapstats[(monhit[t] - 1000)].solid > 1) && (size))) /****** allows size > 0 creatures to stomp over trees but not mountains and such ******/
      {
      shift:
        shift(monpos[monsterup][0], monpos[monsterup][1], temptarget);
        goto goback;
      }
    }

    tempcost = getmovecost(monhit[t]);
    if (tempcost > cost)
      cost = tempcost;
    if (cost > monster[monsterup].movement)
      goto goback;
  }

  if (monster[monsterup].condition[0])
    checkforenemy(2); /***** second check for retreat ******/

  bodyground(monsterup + 10, 0);
  bodyfield(monsterup + 10);
  monpos[monsterup][0] = monx + deltax;
  monpos[monsterup][1] = mony + deltay;
  monster[monsterup].movement -= cost;
  placemonster(monx + deltax, mony + deltay, monsterup);
  drawbody(monsterup + 10, 0, 0);

  if (monster[monsterup].underneath[1][1] > 999)
    sound(mapstats[monster[monsterup].underneath[1][1] - 1000].sound);

  /************** put damage for spells here ***********/

  collide(q[up], FALSE);
  delay(delayspeed / 2);

  if ((fieldx + monx + deltax < 2) || (fieldy + mony + deltay < 2) || (fieldx + monx + deltax > 87) || (fieldy + mony + deltay > 87)) {
    if (monster[monsterup].cansum == -1) /**** dont let Allies leave battle *****/
    {
      deltax *= -1;
      deltay *= -1;
      goto pushintogeorgebush;
    }

    monster[monsterup].movement = monster[monsterup].guarding = 0;
    up--;
    monster[monsterup].surrenderpercent = 101; /******* flees in panic **********/
  }

pushintogeorgebush:

  if ((monpos[monsterup][0] < 1) || (monpos[monsterup][0] > 8) || (monpos[monsterup][1] < 1) || (monpos[monsterup][1] > 8)) {
    centerfield(monpos[monsterup][0], monpos[monsterup][1]);
  }
  return (TRUE);
}
