#include "variables.h"

/********************** updatepictbox ****************************/
void updatepictbox(short who, short mode, short noplotmode) {
  if ((who < 0) || (who > charnum))
    return;

  icon.top = 50 * who;
  icon.left = 320 + leftshift - (leftshift * forcesmall);
  icon.right = 380 + leftshift - (leftshift * forcesmall);
  icon.bottom = icon.top + 50;

  if (!mode) {
    pict(1, icon);
    icon.left += 10;
    InsetRect(&icon, 9, 9);
  } else {
    icon.left += 10;
    InsetRect(&icon, 9, 9);
  }

  if (!noplotmode)
    plotportrait(c[who].pictid, icon, c[who].caste, who);
  else
    plotportrait(c[who].pictid, icon, c[who].caste, -1);

  if (c[who].stamina < 1) {
    InsetRect(&icon, -9, -9);
    ploticon3(2019, icon);
    InsetRect(&icon, 9, 9);
    if (c[who].stamina < -9)
      ploticon3(2015, icon);
    else if ((!c[who].bleeding) && (incombat))
      ploticon3(177, icon);
  }
}
