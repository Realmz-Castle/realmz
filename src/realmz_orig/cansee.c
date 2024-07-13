#include "prototypes.h"
#include "variables.h"

/******************************** cansee *******************/
short cansee(short fromx, short fromy, short tox, short toy) {
  short newhit, t;
  Rect orig, dest;
  float deltx, delty, stepx, stepy, partx, party;

  orig.top = fromy * 32;
  orig.left = fromx * 32;

  dest.top = toy * 32;
  dest.left = tox * 32;

  deltx = dest.left - orig.left;
  delty = dest.top - orig.top;

  stepx = deltx / (128 + delayspeed);
  stepy = delty / (128 + delayspeed);

  partx = orig.left;
  party = orig.top;

  for (t = 0; t < 128; t++) {
    newhit = field[fieldx + (orig.left + 16) / 32][fieldy + (orig.top + 16) / 32];

    MyrBitClrShort(&newhit, 1); //**** removes note marker
    MyrBitClrShort(&newhit, 2); //**** removes path marker

    if (newhit > 999) {
      newhit -= 1000;
      if (newhit > 999)
        newhit -= 1000;
      if (newhit > 999)
        newhit -= 1000;

      if (newhit > 0) {
        if (mapstats[newhit].los)
          return (FALSE);
      }
    }
    if (newhit > 399)
      newhit = basetile[lastpix]; // Myriad

    party += stepy;
    partx += stepx;

    orig.top = party;
    orig.left = partx;
  }
  return (TRUE);
}

/******************************** cansee2 *******************/
short cansee2(long fromx, long fromy) {
  short t, tt, curx, cury, deltax, deltay, stopx, stopy, hit;

  stopx = -5;

  for (deltay = -1; deltay < 2; deltay += 2) {
    for (t = 0; t < 10; t++) /**** top / bottom section ****/
    {
      cury = fromy;
      curx = fromx;
      stopx++;

      if (t < 5)
        deltax = -1;
      else if (t > 5)
        deltax = 1;
      else
        deltax = 0;

      for (tt = 0; tt < 6; tt++) /**** top / bottom section ****/
      {
        if (curx == t)
          deltax = 0;

        if ((lookx + curx > -1) && (lookx + curx < 90) && (looky + cury > -1) && (looky + cury < 90)) {
          hit = field[lookx + curx][looky + cury];

          MyrBitClrShort(&hit, 1); //**** removes note marker
          MyrBitClrShort(&hit, 2); //**** removes path marker

          if (hit > 999) {
            hit -= 1000;
            if (hit > 999)
              hit -= 1000;
            if (hit > 999)
              hit -= 1000;
          }
          if (hit > 399)
            hit = basetile[lastpix]; // Myriad

          if (!(partycondition[4]) && (hit > 0)) {
#if CHECK_ILLEGAL_ACCESS > 0
            if (hit >= 402 || hit < 0)
              AcamErreur("cansee2");
#endif
            if ((mapstats[hit].los == 1) && (cury != fromy))
              tt = 10;
          }

#if CHECK_ILLEGAL_ACCESS > 0
          if (lookx + curx < 0 || lookx + curx >= 90 || looky + cury < 0 || looky + cury >= 90)
            AcamErreur("cansee : site overflow");

#endif
          site[lookx + curx][looky + cury] = TRUE;
        }

        curx += deltax;
        cury += deltay;
      }
    }
  }

  for (deltax = -1; deltax < 2; deltax += 2) {
    stopy = -5;

    for (t = 0; t < 10; t++) /**** right/left section ****/
    {
      cury = fromy;
      curx = fromx;
      stopy++;

      if (t < 5)
        deltay = -1;
      else if (t > 5)
        deltay = 1;
      else
        deltay = 0;

      for (tt = 0; tt < 6; tt++) /**** right/left section ****/
      {
        if (cury == t)
          deltay = 0;

        if ((lookx + curx > -1) && (lookx + curx < 90) && (looky + cury > -1) && (looky + cury < 90)) {

          hit = field[lookx + curx][looky + cury];
          MyrBitClrShort(&hit, 1); //**** removes note marker
          MyrBitClrShort(&hit, 2); //**** removes path marker

          if (hit > 999) {
            hit -= 1000;
            if (hit > 999)
              hit -= 1000;
            if (hit > 999)
              hit -= 1000;
          }
          if (hit > 399)
            hit = basetile[lastpix]; // Myriad

          if ((!partycondition[4]) && (hit > 0)) {
            if ((mapstats[hit].los == 1) && (curx != fromx))
              tt = 10;
          }
#if CHECK_ILLEGAL_ACCESS > 0
          if (lookx + curx < 0 || lookx + curx >= 90 || looky + cury < 0 || looky + cury >= 90)
            AcamErreur("cansee : site overflow");

#endif
          site[lookx + curx][looky + cury] = TRUE;
        }

        curx += deltax;
        cury += deltay;
      }
    }
  }
  return (NIL);
}
