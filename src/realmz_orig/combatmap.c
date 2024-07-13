#include "prototypes.h"
#include "variables.h"

/********************************* combatmap ***********************/
void combatmap(void) {
  FILE* fp = NULL;
  short t, tt, ttt, tttt, x, y, temp2, temptop, tempbottom;
  struct mapstats top, bottom;

  short dungeonbuild[4][3][3] = {

      234, 234, 234,
      234, 234, 234, /****** dungeon wall ****/
      234, 234, 234,

      232, 232, 232,
      232, 232, 232, /****** dungeon floor tile 1 ****/
      232, 232, 232,

      252, 252, 252,
      252, 252, 252, /******  black square *****/
      252, 252, 252,

      233, 233, 233,
      233, 233, 233, /**** 61 Default for less than 256 color ground ******/
      233, 233, 233};

  mapshiftx = mapshifty = 0;

  if (!indung) {
    x = lookx - 15 + partyx;
    y = looky - 15 + partyy;
  } else {
    x = floorx - 15;
    y = floory - 15;
  }

  if (x < 0) {
    mapshiftx = x * 3;
    x = 0;
  }

  if (y < 0) {
    mapshifty = y * 3;
    y = 0;
  }

  if (x > 60) {
    mapshiftx = (x - 60) * 3;
    x = 60;
  }

  if (y > 60) {
    mapshifty = (y - 60) * 3;
    y = 60;
  }

  if (indung) {
    for (t = 0; t < 30; t++) {
      for (tt = 0; tt < 30; tt++) {
        temp2 = 1;
        temp = field[tt + y][t + x];

        if (MyrBitTstShort(&temp, 15))
          temp2 = 0; /***** wall *****/

        MyrBitClrShort(&temp, 2); /**** removes marked path *****/
        if (BitAnd(temp, 20238))
          temp2 = 1; /**** all type that leave a hole ***/

        if (!fullcolor)
          temp2 = 3;

        for (ttt = 0; ttt < 3; ttt++) {
          for (tttt = 0; tttt < 3; tttt++) {
            tempfield[t * 3 + ttt][tt * 3 + tttt] = dungeonbuild[temp2][tttt][ttt];
          }
        }
      }
    }
  } else {
    for (tt = 0; tt < 30; tt++) {
      for (t = 0; t < 30; t++) {
        for (tttt = 0; tttt < 3; tttt++) {
          for (ttt = 0; ttt < 3; ttt++) {
            temp = field[t + x][tt + y];

            MyrBitClrShort(&temp, 1); /***** clear notes ****/
            MyrBitClrShort(&temp, 2); /***** clear path ****/

            if (temp > 999) {
              temp -= 1000;
              if (temp > 999) {
                temp -= 1000;
                if (temp > 999)
                  temp -= 1000;
              }
            } else if (temp < -999)
              temp = basetile[lastpix];

            if ((temp > 0) && (temp < 201))
              tempfield[t * 3 + ttt][tt * 3 + tttt] = mapstats[temp].build[tttt][ttt];
            else
              tempfield[t * 3 + ttt][tt * 3 + tttt] = mapstats[basetile[lastpix]].build[tttt][ttt];
          }
        }
      }
    }
  }
  /***************** add in trees and rubble ***************************/
  for (t = 2; t < 88; t++) {
    for (tt = 2; tt < 88; tt++) {
      if (indung) {
        if ((Rand(100) < 10) && (!mapstats[tempfield[tt][t]].solid))
          tempfield[tt][t] = 200 + randrange(141, 158);
      } else {
        bottom = mapstats[tempfield[tt][t]];
        top = mapstats[tempfield[tt][t - 1]];
        if (bottom.forest) {
          /********* make sure both squares are valid for trees *************/
          tempbottom = bottom.solid + bottom.shore + bottom.needboad + bottom.los + bottom.ispath;
          temptop = top.solid + top.shore + top.needboad + top.los + bottom.ispath;

          if ((!tempbottom) && (!temptop)) {
            switch (bottom.forest) {
              case 1: /********** plains trees **********/
                if (Rand(100) < 20) {
                  tempfield[tt][t] = 200 + randrange(67, 71); /** bottom **/
                  if (!twixt(tempfield[tt][t - 1], 267, 271))
                    tempfield[tt][t - 1] = tempfield[tt][t] - 6;
                  else
                    tempfield[tt][t - 1] = 266; /***** overlaping trunk ****/
                } else if (Rand(100) < 10)
                  tempfield[tt][t] = 200 + randrange(81, 91);
                else
                  tempfield[tt][t] = basetile[lastpix];
                break;

              case 2: /********** desert trees **********/
                if (Rand(100) < 20) {
                  tempfield[tt][t] = 200 + randrange(107, 111); /** bottom **/
                  if (!twixt(tempfield[tt][t - 1], 307, 311))
                    tempfield[tt][t - 1] = 200 + randrange(101, 105); /** top **/
                  else
                    tempfield[tt][t - 1] = 306; /***** overlaping trunk ****/
                } else if (Rand(100) < 10)
                  tempfield[tt][t] = 200 + randrange(121, 131);
                else
                  tempfield[tt][t] = basetile[lastpix];
                break;

              case 3: /********** shrooms **********/
                if ((Rand(100) < 20) && (!twixt(tempfield[tt][t - 1], 273, 280))) {
                  tempfield[tt][t] = 200 + randrange(77, 80);
                  tempfield[tt][t - 1] = tempfield[tt][t] - 4;
                } else if (Rand(100) < 10)
                  tempfield[tt][t] = 200 + randrange(92, 100);
                else
                  tempfield[tt][t] = basetile[lastpix];
                break;

              case 4: /********** swamp trees **********/
                if ((Rand(100) < 20) && (!twixt(tempfield[tt][t - 1], 381, 391))) {
                  tempfield[tt][t] = 200 + randrange(186, 190); /** bottom **/
                  if (!twixt(tempfield[tt][t - 1], 307, 311))
                    tempfield[tt][t - 1] = tempfield[tt][t] - 5; /** top **/
                } else if (Rand(100) < 10)
                  tempfield[tt][t] = 200 + randrange(159, 171);
                else
                  tempfield[tt][t] = basetile[lastpix];
                break;

              case 5: /********** snow trees **********/
                if ((Rand(100) < 20) && (!twixt(tempfield[tt][t - 1], 391, 400))) {
                  tempfield[tt][t] = 200 + randrange(196, 200); /** bottom **/
                  if (!twixt(tempfield[tt][t - 1], 317, 321))
                    tempfield[tt][t - 1] = tempfield[tt][t] - 5; /** top **/
                } else if (Rand(100) < 10)
                  tempfield[tt][t] = 200 + randrange(172, 180);
                else
                  tempfield[tt][t] = basetile[lastpix];
                break;
            }
          } else if (bottom.forest)
            tempfield[tt][t] = basetile[lastpix];
        } else if (tempfield[tt][t] == basetile[lastpix]) {
          switch (lastpix) {
            case 0: /********** plains rubble *********/
              if (Rand(100) < 20)
                tempfield[tt][t] = 200 + randrange(81, 91);
              break;

            case 3: /********** underground rubble *********/
              if (Rand(100) < 20)
                tempfield[tt][t] = 200 + randrange(92, 100);
              break;

            case 5: /********** desert rubble *********/
              if (Rand(100) < 20)
                tempfield[tt][t] = 200 + randrange(121, 131);
              break;

            case 9: /********** swamp rubble *********/
              if (Rand(100) < 20)
                tempfield[tt][t] = 200 + randrange(159, 171);
              break;

            case 10: /********** snow rubble *********/
              if (Rand(100) < 20)
                tempfield[tt][t] = 200 + randrange(172, 180);
              break;
          }
        }
      }
    }
  }
  /*********************** bump 1000 **************/
  for (t = 0; t < 90; t++) {
    for (tt = 0; tt < 90; tt++) {
      field[tt][t] = tempfield[tt][t] + 1000;
    }
  }
}
