#include "prototypes.h"
#include "variables.h"

/************************ combaterrors **************************/
void combaterrors(short error) {
  SetPort(GetWindowPort(screen));
  TextSize(18);
  TextMode(1);
  MoveTo(420 + leftshift, 440 + downshift);
  ForeColor(yellowColor);

  pict(156, infosmall);
  switch (error) {
    case 1:
      MyrDrawCString("Need To Select Target");
      break;

    case 2:
      MyrDrawCString("    Out Of Range");
      break;

    case 3:
      MyrDrawCString("Target Is Blocked");
      break;

    case 4:
      MyrDrawCString("  Not A Valid Target");
      break;

    case 5:
      MyrDrawCString("  Too Many Targets");
      break;
  }
  sound(-143);
  pict(156 + 18 * inspell, infosmall);
  if (inspell)
    updatespell(1);
  else
    number();
  upbutton(FALSE);
}
