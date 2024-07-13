#include "variables.h"

/******************* showtag ***********************************/
void showtag(Str255 message, short monsterid, short position) {

  SetPort(GetWindowPort(look));
  TextFont(genevafont);
  TextSize(12);
  TextMode(0);

  MoveTo(20, 100);
  MyrDrawCString("                                                 ");
  MoveTo(20, 100);
  MyrDrawCString("Message: ");
  MyrDrawCString((Ptr)message);

  MoveTo(20, 120);
  MyrDrawCString("                                                 ");
  MoveTo(20, 120);
  MyrDrawCString("Battle ID: ");
  string(battlenum);

  MoveTo(20, 140);
  MyrDrawCString("                                                 ");
  MoveTo(20, 140);
  MyrDrawCString("Monster ID: ");
  string(monsterid);

  MoveTo(20, 160);
  MyrDrawCString("                                                 ");
  MoveTo(20, 160);
  MyrDrawCString("Position ID: ");
  string(position);

  if ((!monsterid) && (!position))
    flashmessage((StringPtr) "Click Mouse", 350, 100, 0, 0);
  else {
    sound(1000);
    delay(120);
  }

  TextMode(1);
}
