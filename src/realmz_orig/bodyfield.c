#include "prototypes.h"
#include "variables.h"

/***************************** bodyfield **************************/
void bodyfield(short mon) {
  if (mon > 9) {
    mon -= 10;
    if (monster[mon].size == 3)
      field[monpos[mon][0] + fieldx - 1][monpos[mon][1] + fieldy - 1] = monster[mon].underneath[0][0];
    if (monster[mon].size > 1)
      field[monpos[mon][0] + fieldx - 1][monpos[mon][1] + fieldy] = monster[mon].underneath[0][1];
    if ((monster[mon].size == 3) || (monster[mon].size == 1))
      field[monpos[mon][0] + fieldx][monpos[mon][1] + fieldy - 1] = monster[mon].underneath[1][0];
    field[monpos[mon][0] + fieldx][monpos[mon][1] + fieldy] = monster[mon].underneath[1][1];
  } else
    field[pos[mon][0] + fieldx][pos[mon][1] + fieldy] = charunder[mon];
}
