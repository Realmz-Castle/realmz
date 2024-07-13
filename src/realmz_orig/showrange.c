#include "variables.h"

/******************************* showrange *************************************/
void showrange(short edgemode) {
  short x, y, x1, y1, size;

  GetGWorld(&savedPort, &savedDevice);

  if (!edgemode) {
    for (t = 0; t < nummon; t++)
      if (monster[t].stamina > 0)
        drawbody(t + 10, 1, 0);
    for (t = 0; t <= charnum; t++)
      if (c[t].inbattle)
        drawbody(t, 1, 0);
  }

  if (inspell)
    return;

  x = 32 * pos[charup][0] + 16;
  y = 32 * pos[charup][1] + 16;

  if (!edgemode)
    SetPort(look);
  else
    SetGWorld(gedge, NIL);

  PenMode(0);
  for (t = 0; t < nummon; t++) {
    if (monster[t].stamina > 0) {
      size = monster[t].size;
      x1 = 32 * monpos[t][0];
      y1 = 32 * monpos[t][1];
      if (size < 2)
        x1 += 16;
      if ((size == 2) || (size == 0))
        y1 += 16;
      ForeColor(redColor);
      if (!monster[t].traiter)
        RGBForeColor(&greencolor);
      if (monster[t].condition[1])
        ForeColor(blueColor);
      MoveTo(x, y);
      LineTo(x1, y1);
    }
  }

  for (t = 0; t <= charnum; t++) {
    if (c[t].inbattle) {
      RGBForeColor(&greencolor);
      if (c[t].traiter)
        ForeColor(redColor);
      if (c[t].condition[1])
        ForeColor(blueColor);
      MoveTo(x, y);
      LineTo(32 * pos[t][0] + 16, 32 * pos[t][1] + 16);
    }
  }
  ForeColor(blackColor);
  PenMode(2);
  if (!edgemode)
    sound(137);
  else {
    SetGWorld(savedPort, savedDevice);
    return;
  }
  FlushEvents(everyEvent, 0);
backup:
  WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);
#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif

  if ((gTheEvent.what != mouseDown) && (gTheEvent.what != keyDown))
    goto backup;
}
