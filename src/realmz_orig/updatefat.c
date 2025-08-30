#include "prototypes.h"
#include "variables.h"

/************************************ updatefat ***************/
void updatefat(short show, short num, short erase) {
  if ((FrontWindow() == look) || (FrontWindow() == gWindow)) {
    int enable_recomposite = WindowManager_SetEnableRecomposite(0);

    SetPort(GetWindowPort(screen));
    BackPixPat(base);

    if (erase) {
      buttonrect.top = 300 + downshift;
      buttonrect.bottom = 321 + downshift;
      buttonrect.left = 321 + leftshift;
      buttonrect.right = 640 + leftshift;
      RGBBackColor(&greycolor);
      EraseRect(&buttonrect);
    }

    if (incombat) {
      WindowManager_SetEnableRecomposite(enable_recomposite);
      return;
    }

    if (num < 0) {
      buttonrect.top = 304 + downshift;
      buttonrect.bottom = 318 + downshift;
      buttonrect.left = 500 + leftshift;
      buttonrect.right = 501 + fat + leftshift;
      RGBBackColor(&greycolor);
      EraseRect(&buttonrect);
    }

    fat += num;

    if (fat > 135)
      fat = 135;
    if (fat < 4)
      fat = 4;

    if (show) {
      buttonrect.top = 300 + downshift;
      buttonrect.bottom = 321 + downshift;
      buttonrect.left = 321 + leftshift;
      buttonrect.right = 640 + leftshift;
      pict(199, buttonrect);
    }

    buttonrect.top = 304 + downshift;
    buttonrect.bottom = 317 + downshift;
    buttonrect.left = 500 + leftshift;
    buttonrect.right = 500 + fat + leftshift;

    RGBBackColor(&greycolor);
    if (fat > 70)
      BackPixPat(gHilite);
    if (fat > 105)
      BackPixPat(gShadow);
    EraseRect(&buttonrect);
    downbutton(TRUE);
    BackPixPat(base);
    if (!num)
      xy(0);

    WindowManager_SetEnableRecomposite(enable_recomposite);
  }
}
