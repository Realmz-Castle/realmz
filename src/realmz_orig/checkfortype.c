#include "prototypes.h"
#include "variables.h"

/**************************** checkfortype **********************/
short checkfortype(short who, short type, short aliveonly) /******** who = -1 = everyone **********/
{
  short start, stop, reply = 0;

  start = stop = who;

  if (who == -1) {
    start = 0;
    stop = charnum;
  }

  for (t = start; t <= stop; t++) {
    if (c[t].armor[type]) {
      if (!aliveonly)
        reply = TRUE;
      else if (c[t].stamina > 0)
        reply = TRUE;
    }
  }

  return (reply);
}
