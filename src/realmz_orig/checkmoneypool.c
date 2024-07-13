#include "prototypes.h"
#include "variables.h"

/************************ checkmoneypool ****************/
void checkmoneypool(void) {

  if (moneypool[0] || moneypool[1] || moneypool[2]) {
    if (bankavailable) {
      bank[0] += moneypool[0];
      bank[1] += moneypool[1];
      bank[2] += moneypool[2];
      moneypool[0] = moneypool[1] = moneypool[2] = 0;
      bankavailable = 0;
    } else {
      in();
      if (question(6)) {
        swap();
      }
      moneypool[0] = moneypool[1] = moneypool[2] = 0;
      updatemain(FALSE, -1);
    }
  }
}
