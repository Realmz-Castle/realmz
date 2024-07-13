#include "prototypes.h"
#include "variables.h"

/***************************** showcharge2 *******************/
void showcharge2(short charge, short ident) {
  if (!ident)
    return;

  if (charge > 0) {
    ForeColor(redColor);
    MyrDrawCString("  X ");
    string(charge);
  }
  ForeColor(blackColor);

  showitemstats(ident);
}
