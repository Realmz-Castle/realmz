#include "prototypes.h"
#include "variables.h"

/***************************** tickcheck *********************/
void tickcheck(void) {
  if (!look)
    return;

  if ((defaultfont != ID_FONT_DEFAULT) && (defaultfont != 3))
    defaultfont = 3;

  if (TickCount() > oldflametick + 10) {
    SetPort(GetWindowPort(screen));
    oldflametick = TickCount();
    if (partycondition[0]) {
      icon.top = downshift + 367 - partycondition[0] / 4;
      if (icon.top < (331 + downshift))
        icon.top = 331 + downshift;
      icon.left = 526 + leftshift;
      icon.right = icon.left + 16;
      icon.bottom = icon.top + 30;
      ploticon3(146 + flamestage, icon);
    }
    icon.top = 330 + downshift;
    icon.left = 328 + leftshift;
    for (t = 1; t < 9; t++) {
      OffsetRect(&icon, 39, 0);
      if (t == 5)
        OffsetRect(&icon, -156, 40);
      if (partycondition[t])
        ploticon2(t * 8 + 13992 + flamestage);
    }
    flamestage++;
    if (flamestage > 7)
      flamestage = 0;
  }
}

/********************************** updatetorch *********************************/
void updatetorch(void) {
  Rect mainrect;
  char stop = 0;
  RGBBackColor(&greycolor);
  mainrect.left = 530 + leftshift;
  mainrect.right = mainrect.left + 8;

  GetControlBounds(torch, &buttonrect);
  // buttonrect = *&(**(torch)).contrlRect;

  if (checkforitem(805, FALSE, -1))
    upbutton(TRUE);

  if (partycondition[0]) {
    stop = partycondition[0] / 31 + 1;
  } else if (checkforitem(805, FALSE, -1)) {
    stop = 2;
    mainrect.left -= 6;
    mainrect.right += 7;
    mainrect.top = 331 + downshift;
    mainrect.bottom = mainrect.top + 70;
    EraseRect(&mainrect);
  }
  mainrect.left = 530 + leftshift;
  mainrect.right = mainrect.left + 8;
  mainrect.top = 391 + downshift;

  if (stop) {
    for (t = 0; t < stop; t++) {
      mainrect.top -= 7;
      mainrect.bottom = mainrect.top + 16;
      ploticon3(154, mainrect);
    }
  } else {
    mainrect.left -= 6;
    mainrect.right += 7;
    mainrect.top = 331 + downshift;
    mainrect.bottom = mainrect.top + 70;
    EraseRect(&mainrect);
  }
}

/******************************** upbutton *********************/
void upbutton(short newport) {
  GrafPtr oldport;

  GetPort(&oldport);
  if (newport)
    SetPort(GetWindowPort(screen));

  // RGBForeColor(&lightgrey);
  PenPixPat(light);
  MoveTo(buttonrect.left, buttonrect.bottom);
  LineTo(buttonrect.left, buttonrect.top);
  LineTo(buttonrect.right, buttonrect.top);

  // RGBForeColor(&darkgrey);
  PenPixPat(dark);
  LineTo(buttonrect.right, buttonrect.bottom);
  LineTo(buttonrect.left, buttonrect.bottom);

  RGBBackColor(&greycolor);
  ForeColor(yellowColor);
  SetPort(oldport);
}
