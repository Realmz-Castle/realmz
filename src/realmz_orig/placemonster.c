#include "variables.h"

/******************* placemonster ***********************************/
void placemonster(short x, short y, short monsterup) {
  x += fieldx;
  y += fieldy;

  size = monster[monsterup].size;

  monster[monsterup].underneath[1][1] = field[x][y];
  field[x][y] = monsterup + 10;

  if (size == 3) {
    monster[monsterup].underneath[0][0] = field[x - 1][y - 1];
    field[x - 1][y - 1] = -(monsterup + 10);
  } else
    monster[monsterup].underneath[0][0] = 0;

  if ((size == 1) || (size == 3)) {
    monster[monsterup].underneath[1][0] = field[x][y - 1];
    field[x][y - 1] = -(monsterup + 10);
  } else
    monster[monsterup].underneath[1][0] = 0;

  if (size > 1) {
    monster[monsterup].underneath[0][1] = field[x - 1][y];
    field[x - 1][y] = -(monsterup + 10);
  } else
    monster[monsterup].underneath[0][1] = 0;
}
