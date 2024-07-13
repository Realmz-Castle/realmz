#include "variables.h"

/******************************** pushonstack ****************************/
void pushonstack(struct door pushdoor, short counter) {
  stackdoor[stackindex] = pushdoor;
  stackcounter[stackindex++] = counter;
}

/******************************** pulloffstack ****************************/
short pulloffstack(void) {
  short index;

  if (stackindex == 0)
    return (8); //*** stack is already empty

  stackindex--;
  infodoor = stackdoor[stackindex];
  index = stackcounter[stackindex];
  return (index);
}
