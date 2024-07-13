#include "prototypes.h"
#include "variables.h"

/************* updatebooty *********************/
void updatebooty(short charnum) {
  Rect box;

  SetPort(GetWindowPort(bootywindow));
  BackPixPat(base);
  TextSize(14);
  TextMode(1);
  TextFace(bold);
  TextFont(1602);
  RGBForeColor(&cyancolor);
  RGBBackColor(&greycolor);

  box.left = 470 + leftshift;
  box.right = box.left + 20;
  box.top = 50 * charnum + 30;
  box.bottom = box.top + 15;

  movecalc(charnum);

  MoveTo(box.left, box.top + 10);
  EraseRect(&box);
  string(c[charnum].movementmax);

  box.left += 72;
  box.right += 93;

  MyrNumToString(c[charnum].load, myString);
  MoveTo(box.left, box.top + 10);
  EraseRect(&box);
  MyrDrawCString((Ptr)myString);

  MyrNumToString(c[charnum].loadmax, myString);
  MoveTo(box.left + 53, box.top + 10);
  MyrDrawCString((Ptr)myString);

  box.left += 70;
  box.right = 633 + leftshift;
  box.top -= 22;
  box.bottom -= 22;
  MoveTo(box.left, box.top + 10);
  EraseRect(&box);
  string(c[charnum].numitems);
}
