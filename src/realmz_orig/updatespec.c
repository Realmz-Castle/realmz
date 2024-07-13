#include "prototypes.h"
#include "variables.h"

/******************************* updatespec **************************/
void updatespec(short mode) {
  char demodif[12][19] = {
      // clang-format off
      /**  3    4    5    6    7   17   18   19   20   21   22   23   24   25   26   27   28   29   30 **/
          -5,  -4,  -3,  -2,  -1,   1,   1,   2,   2,   3,   3,   4,   4,   4,   5,   5,   5,   5,   5, /*** Sneak Attack ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Hide In Shadows ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Ressurrect ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Major Wound ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Detect Secret ****/
         -20, -15, -10,  -5,  -2,   5,   8,  11,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65, /*** Acrobatic Act ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Detect Trap ****/
         -25, -20, -15, -10,  -5,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70, /*** Disarm Trap ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Hear Noise ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Force Door ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Move Silently ****/
         -25, -20, -15, -10,  -5,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70, /*** Pick Lock ****/
      // clang-format on
  };

  char range1[19] = {
      // clang-format off
      /** 3  4  5  6  7  17  18  19  20  21  22  23  24  25  26  27  28  29  30 **/
          3, 4, 5, 6, 7, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, /**** Brawn/DX Ranges ***/
      // clang-format on
  };

  char stmodif[12][19] = {
      // clang-format off
      /**  3    4    5    6    7   17   18   19   20   21   22   23   24   25   26   27   28   29   30 **/
          -5,  -4,  -3,  -2,  -1,   1,   1,   1,   2,   2,   2,   3,   3,   3,   4,   4,   4,   4,   4, /*** Sneak Attack ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Hide In Shadows ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Ressurrect ****/
          -5,  -4,  -3,  -2,  -1,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, /*** Major Wound ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Detect Secret ****/
         -75, -60, -45, -30, -15,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70, /*** Acrobatic Act ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Detect Trap ****/
         -10,  -8,  -6,  -4,  -2,   2,   4,   6,   8,  10,  12,  14,  16,  18,  20,  22,  24,  26,  28, /*** Disarm Trap ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Hear Noise ****/
         -75, -60, -45, -30, -15,   5,  10,  15,  20,  25,  30,  35,  40,  45,  50,  55,  60,  65,  70, /*** Force Lock ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*** Move Silently ****/
           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 /*** Pick Lock ****/
      // clang-format on
  };

  loadprofile(characterl.race, characterl.caste);

  switch (mode) {
    case 1: /****** initialize ******/

      for (t = 0; t < 15; t++) // v7.1
      {
        if (caste.specialability[0][t])
          characterl.spec[t] = caste.specialability[0][t] + races.specialability[t];
        else
          characterl.spec[t] = 0;
      }

      for (tt = 0; tt < 19; tt++) // v7.1
      {
        if (range1[tt] == characterl.st) {
          for (t = 0; t < 14; t++)
            if (caste.specialability[0][t])
              characterl.spec[t] += stmodif[t][tt];
        }
      }

      for (tt = 0; tt < 19; tt++)
        if (range1[tt] == characterl.de) {
          for (t = 0; t < 14; t++)
            if (caste.specialability[0][t])
              characterl.spec[t] += demodif[t][tt];
        }

      break;

    default:

      for (t = 0; t < 15; t++) // v7.1
      {
        if (caste.specialability[1][t])
          characterl.spec[t] += Rand(caste.specialability[1][t]);
      }

      break;
  }

  for (t = 0; t < 12; t++)
    characterl.spec[t] = pin(characterl.spec[t], 0, 100);

  characterl.dodge = pin(characterl.dodge, 0, 100);
  characterl.missile = pin(characterl.missile, 0, 100);
  characterl.magres = pin(characterl.magres, 0, 100);
  characterl.twohand = pin(characterl.twohand, 0, 100);
  characterl.damage = pin(characterl.damage, 0, 200);
  characterl.handtohand = pin(characterl.handtohand, 0, 200);
}
