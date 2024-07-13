#include "prototypes.h"
#include "variables.h"

/************************* number ************************/
void number(void) {
  if (inspell)
    return;

  TextSize(20);
  ForeColor(yellowColor);
  TextMode(1);
  TextFont(font);

  MoveTo(430 + leftshift, 440 + downshift);
  MyrDrawCString("Enemies Left");
  MoveTo(580 + leftshift, 440 + downshift);
  string(numenemy - killmon);
}
