#include "prototypes.h"
#include "variables.h"

/************************ shift **************************/
void shift(short curx, short cury, short target) {
  short temp;
  char netx, nety, tarx, tary;
  if (target < 9) {
    tarx = pos[target][0];
    tary = pos[target][1];
  } else {
    tarx = monpos[target - 10][0];
    tary = monpos[target - 10][1];
  }

  netx = abs(tarx - curx);
  nety = abs(tary - cury);

  if (netx > nety) {
    if ((deltay) && (Rand(2) == 1))
      deltay = 0;
    else {
      deltay = -1;
      temp = Rand(3);
      if (temp == 1)
        deltay = 1;
      if (temp == 2)
        deltay = 0;
      temp = Rand(3);
      if (temp == 1)
        deltax = -1;
      if (temp == 2)
        deltax = 1;
      if ((temp == 3) && (deltay != 0))
        deltax = 0;
      else {
        deltax = 1;
        if (Rand(2) == 1)
          deltax = -deltax;
      }
    }
  } else {
    if (deltax)
      deltax = 0;
    else {
      deltax = -1;
      temp = Rand(3);
      if (temp == 1)
        deltax = 1;
      if (temp == 2)
        deltax = 0;

      temp = Rand(3);
      if (temp == 1)
        deltay = -1;
      if (temp == 2)
        deltay = 1;
      if (temp == 3)
        deltay = 0;
    }
  }
}
