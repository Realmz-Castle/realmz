#include "prototypes.h"
#include "variables.h"

extern Boolean tagger;

/********************* moveparty ************************/
void moveparty(short forward) {

  MyrCheckMemory(1);

  updatemusic();

  if ((moneypool[0] || moneypool[1] || moneypool[2]) && (forward)) {
    if (bankavailable) {
      bank[0] += moneypool[0];
      bank[1] += moneypool[1];
      bank[2] += moneypool[2];
      moneypool[0] = moneypool[1] = moneypool[2] = 0;
    } else {
      if (question(6)) {
        in();
        swap();
        moneypool[0] = moneypool[1] = moneypool[2] = 0;
        updatemain(FALSE, -1);
        return;
      } else
        moneypool[0] = moneypool[1] = moneypool[2] = 0;
    }
  }

  if (forward)
    bankavailable = 0;

  if (indung)
    return;

  if (forward) {
    temp = abs(field[partyx + lookx + deltax][partyy + looky + deltay]);
    if (temp > 999)
      temp -= 1000;
    if (temp > 999)
      temp -= 1000;
    if (temp > 999)
      temp -= 1000;

    if ((temp > 1) && (temp < 201)) {
#if CHECK_ILLEGAL_ACCESS > 0
      if (partyx + lookx + deltax < 0 || partyx + lookx + deltax >= 90 ||
          partyy + looky + deltay < 0 || partyy + looky + deltay >= 90)
        AcamErreur("moveparty field overflow");
#endif
      if (mapstats[temp].ispath)
        MyrBitSetShort(&field[partyx + lookx + deltax][partyy + looky + deltay], 2); /**** set path ****/
    }

    if ((shopavail) || (templeavail)) {
      campavail = TRUE;
      shopavail = inshop = currentshop = intemple = templeavail = FALSE;
      updatecontrols();
    }
    campavail = TRUE;
    shopavail = inshop = currentshop = intemple = templeavail = FALSE;
  }

  partyx += deltax * forward;
  partyy += deltay * forward;

  lastdeltax = deltax;
  lastdeltay = deltay;

  centerpict();
  SetPort(GetWindowPort(screen));
}
