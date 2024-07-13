#include "variables.h"

/************** updatecanlist ***********************/
void updatecanlist(void) {
  short loop; /************ update canheal,canidentify,candetect status ********/

  for (loop = 0; loop <= charnum; loop++) {
    c[loop].canheal = c[loop].canidentify = c[loop].candetect = 0;
    for (t = 0; t < 7; t++) {
      for (tt = 0; tt < 12; tt++) {
        if ((c[loop].cspells[t][tt]) && (c[loop].spellcastertype)) {
          loadspell(c[loop].spellcastertype - 1, t, tt);
          if (spellinfo.special == 57)
            c[loop].canheal = TRUE;
          if (spellinfo.special == 48)
            c[loop].canidentify = TRUE;
          if (spellinfo.special == 63)
            c[loop].candetect = TRUE;
        }
      }
    }
  }
}
