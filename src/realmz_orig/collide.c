#include "prototypes.h"
#include "variables.h"

/**************************** collide ************************/
void collide(short who, short checkcontrol) {
  Point tall, wide, hugePoint;
  short size;
  Rect boxrect;
  short ttt;

  if (who < 0)
    return;

  if (who <= charnum) {
    if (!c[who].inbattle)
      return;
    point.v = pos[who][1] + fieldy;
    point.h = pos[who][0] + fieldx;

    tall = wide = hugePoint = point;
  } else if (who > 9) {
    size = monster[who - 10].size;

    point.v = monpos[who - 10][1] + fieldy;
    point.h = monpos[who - 10][0] + fieldx;
    tall = wide = hugePoint = point;

    if (size == 1)
      tall.v--;
    if (size == 2)
      wide.h--;
    if (size == 3) {
      hugePoint.v--;
      hugePoint.h--;
      tall.v--;
      wide.h--;
    }
  } else
    return;

  for (ttt = 0; ttt < 60; ttt++) {
    collideflag = ttt + 1;
    if ((que[ttt].duration) && (!collidecheck[ttt])) {
      boxrect = que[ttt].bounds;
      if ((PtInRect(point, &boxrect)) || (PtInRect(tall, &boxrect)) || (PtInRect(wide, &boxrect)) || (PtInRect(hugePoint, &boxrect))) {
        loadspell2(que[ttt].spellnum);
        powerlevel = que[ttt].powerlevel;
        castlevel = que[ttt].castlevel;
        cleartarget();
        if (spellinfo.spellclass == 9)
          spellinfo.damagetype = spellinfo.spellclass = 1;
        spellinfo.queicon = FALSE;
        target[0][0] = que[ttt].x + 3;
        target[0][1] = que[ttt].y + 3;
        rotate = que[ttt].rotate;
        spelltargets(0, who, 0, 1); /****** -1 = check everybody, 1 = NO REFLECTING ******/
        checkcontrol = TRUE;
      }
    }
  }
  collideflag = FALSE;
  if ((checkcontrol) && (q[up] == charup))
    updatecontrols();
}
