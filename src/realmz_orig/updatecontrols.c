#include "prototypes.h"
#include "variables.h"

/*********************** updatecontrols ********************/
void updatecontrols(void) {
  Rect controlrect;
  Boolean tag;

  SetPort(GetWindowPort(screen));
  ForeColor(blackColor);
  DrawPicture(grey, &selectrect);

  if ((incombat) && (q[up] > 5))
    return;

  if (incombat) {
    controlrect.left = 522 + leftshift;
    controlrect.right = controlrect.left + 34;
    controlrect.top = 366 + downshift;
    controlrect.bottom = controlrect.top + 39;

    charselectnew = charup;
    if ((!inspell) && (undead) && (canpriestturn) && (c[charup].spec[13] > 0) && (!c[charup].hasturned))
      pict(181, controlrect);
    else if (!inspell)
      EraseRect(&controlrect);
  }

  controlrect.left = 308 + leftshift;
  controlrect.right = 358 + leftshift;
  controlrect.top = 410 + downshift;
  controlrect.bottom = 460 + downshift;

  selectrect.top = charselectnew * 50;
  selectrect.bottom = selectrect.top + 50;
  selectrect.left = 321 + leftshift;
  selectrect.right = 330 + leftshift;
  DrawPicture(marker, &selectrect);
  charselectold = charselectnew;

  if ((cancast(charselectnew, 1)) && (!inspell))
    pict(162, controlrect);
  else
    pict(154, controlrect);

  controlrect.left += 50;
  controlrect.right += 50;

  tag = FALSE;

  if ((!inspell) && (checkfortype(charselectnew, 13, TRUE))) {
    ShowControl(viewspellsbut);
    pict(151, controlrect);
  } else {
    HideControl(viewspellsbut);
    pict(154, controlrect);
  }

  if (!incombat) {
    controlrect.left = 408 + leftshift;
    controlrect.right = controlrect.left + 50;

    if (!charnum)
      pict(154, controlrect);

    controlrect.left = 508 + leftshift;
    controlrect.right = 558 + leftshift;

    if (incamp) {
      HideControl(shopbut);
      ShowControl(rest);
      pict(157, controlrect);
    } else {
      if (canshop || shopavail) {
        HideControl(rest);
        campavail = FALSE;
        ShowControl(shopbut);
        pict(225, controlrect);
      } else if (templeavail) {
        HideControl(rest);
        ShowControl(shopbut);
        pict(200, controlrect);
      } else {
        HideControl(rest);
        ShowControl(shopbut);
        pict(180, controlrect);
      }
    }

    controlrect.left = 558 + leftshift;
    controlrect.right = 608 + leftshift;

    if (incamp) {
      if (checkfortype(charselectnew, 13, TRUE)) {
        ShowControl(overviewbut);
        pict(175, controlrect);
      } else {
        HideControl(overviewbut);
        pict(154, controlrect);
      }
    } else {
      ShowControl(overviewbut);
      pict(195, controlrect);
    }
  }
}
