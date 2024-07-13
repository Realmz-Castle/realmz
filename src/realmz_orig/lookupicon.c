#include "prototypes.h"
#include "variables.h"

short lookupicontable[25][3] = {
    1, 10, 2,
    12, 19, 12,
    20, 34, 20,
    35, 39, 35,
    50, 56, 50,
    82, 85, 82,
    89, 95, 89,
    517, 527, 527,
    545, 548, 546,
    6100, 6109, 6100,
    6110, 6121, 6110,
    6122, 6127, 6122,
    6137, 6139, 6137,
    6185, 6187, 6183,
    6190, 6195, 6190,
    6196, 6200, 6197,
    6202, 6206, 6202,
    6208, 6209, 12009,
    6162, 6163, 6162,
    6176, 6177, 6177,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0,
    0, 0, 0}; /* Scenario Item */

/***************** lookupicon *****************************/
short lookupicon(short iconid, short ident) {
  short lookloop;

  if (!ident) {
    for (lookloop = 0; lookloop < 25; lookloop++) {
      if (twixt(item.iconid, lookupicontable[lookloop][0], lookupicontable[lookloop][1])) {
        iconid = lookupicontable[lookloop][2];
      }
    }
  }
  return (iconid);
}
