#include "prototypes.h"
#include "variables.h"

/************************* inwindow ********************/
short inwindow(short who) {

  Rect window;
  Point where;

  window.top = window.left = 1;
  window.bottom = window.right = 8;

  if (screensize) {
    window.bottom = 11;
    window.right = 13;
  }

  if (who < 9) {
    where.h = pos[who][0];
    where.v = pos[who][1];
  } else {
    where.h = monpos[who - 10][0];
    where.v = monpos[who - 10][1];
  }

  if (PtInRect(where, &window))
    return (1);
  else
    return (0);
}
