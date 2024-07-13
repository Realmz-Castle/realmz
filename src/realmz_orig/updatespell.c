#include "prototypes.h"
#include "variables.h"

/***************************** updatespell ***********************/
void updatespell(short force) {
  static short oldrange;
  Rect box;
  CCrsrHandle compassnew;
  short range;

  checkrange(charup, spellx, spelly);
  range = currange;

  if ((oldrange != range) || (force)) {
    SetPort(GetWindowPort(screen));
    TextSize(18);
    TextFace(bold);
    ForeColor(whiteColor);

    box.top = 427 + downshift;
    box.bottom = 445 + downshift;

    box.left = 479 + leftshift;
    box.right = box.left + 25;
    MoveTo(479 + leftshift, 443 + downshift);
    EraseRect(&box);
    string(range);

    MoveTo(605 + leftshift, 443 + downshift);
    string(spellrange);

    compassnew = GetCCursor(147 + numoftar - targetnum);
    SetCCursor(compassnew);
    DisposeCCursor(compassnew);
    SetPort(GetWindowPort(look));
  }
  oldrange = currange;
}
