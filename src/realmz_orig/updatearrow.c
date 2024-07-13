#include "prototypes.h"
#include "variables.h"

/*************** updatearrow **************/
void updatearrow(short mode) {
  CCrsrHandle compassnew;
  tagnew = FALSE;
  if (mode) /***** mode 1 = party else combat ****/
  {
    if (point.v > partyy * 32)
      tagnew += 3;
    if (point.v > partyy * 32 + 32)
      tagnew += 3;
    if (point.h > partyx * 32)
      tagnew++;
    if (point.h > partyx * 32 + 32)
      tagnew++;
  } else {
    if (point.v > pos[charup][1] * 32)
      tagnew += 3;
    if (point.v > pos[charup][1] * 32 + 32)
      tagnew += 3;
    if (point.h > pos[charup][0] * 32)
      tagnew++;
    if (point.h > pos[charup][0] * 32 + 32)
      tagnew++;
  }

  if (tagnew != tagold) {
    compassnew = GetCCursor(tagnew + 138);
    SetCCursor(compassnew);
    DisposeCCursor(compassnew);
    tagold = tagnew;
  }
}
