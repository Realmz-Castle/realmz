#include "prototypes.h"
#include "variables.h"

/******************** emptyque ******************/
void emptyque(void) {
  short t;
  doingque = TRUE;
  for (t = 0; t < todoqueindex; t++) {
    if (todoque[t].doorid)
      killbody(0, t + 1);
    todoque[t].doorid = 0;
  }
  doingque = quetodo = todoqueindex = 0;
}
