#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/********************************** setfree **************************/
void setfree(long templong) {
  Boolean bad = FALSE;
  short t;

  long badnum[150] = {11268864, 9242266, 9286002, 29705176, 13303883, 31418092, 1369115096, 616398176, 597431732,
      13579246, 13987296, 11388232, 274146500, 13707748, 14663721, 10207790, 12314976, 16305547,
      2098622237, 9093615, 31143576, 78901234, 3009663607, 1859939640, 8633048, 14663721, 11405349,
      9969539, 8467396, 26059396, 8850411, 11194050, 1717142171, 3306492234, 3863632428, 7716135,
      27920139, 1749344, 4009640, 1805236, 12345678, 13987297, 30951344, 13392474, 28743159,
      25552665, 18640472, 13039308, 45678901, 11603111, 7252048, 7252020, 40928, 17564, 139028,
      319316, 14427945, 9922849, 26542635, 1563960, 10010630, 11113461, 740408, 14982945, 164372,
      1697495, 11403250, 7376, 17663, 25524992, 307806044, 8740286, 14058072, 10589232, 26542635,
      1238331947, 11201520, 28596362, 27040295, 13612749, 13063202, 1888300270, 14067615, 15101166, 11161613,
      1778147749, 26738246, 1859106100, 350263228, 1658791322, 8558380, 11562676, 15388924, 999999999, 10563993,
      25503980, 11094306, 28384679, 9722181, 10315382, 11635675, 9576823, 11115049, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  for (t = 0; t < 150; t++) {
    if ((templong == badnum[t]) && (templong) && (badnum[t])) {
      openpref(10); // erase current preferences file
      bad = TRUE;
    }
  }

  if (bad == TRUE) {
    openpref(10); // erase current preferences file
    MyrBitSetLong(&serial, 8);
    MyrBitSetLong(&serial, 6 + divine);
    serial += (32000 + Rand(32000));
    showserial = serial;
    Name_String[0] = 0;
    savepref();
  }
}

/**************** setfree2 ********************/
void setfree2(short force) {
  short t, bad = 0;
  PtoCstr((StringPtr)gotword);
  for (t = 0; t < 40; t++)
    gotword[t] = tolower(gotword[t]);
  if (!strcmp(gotword, (StringPtr) "<Sharing Owner Name>"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "<Sharing Machine Name>"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Jonathan Bryce"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "John Bennett"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Nigel Davies"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Stuart Amelio"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Kevin Williams"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Hugh Banks"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Terry Oyston"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Ethel Gibson"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Matthew Thompson"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Cliff Keegan"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Lee Heseltine"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Nuouoldopvwq"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Tomdvrvvuayiynsu Wphvd"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "Nf Krq Ptx"))
    bad = TRUE;

  if (!strcmp(gotword, (StringPtr) "mihailescuv"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "victor mihailescu"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "moondark"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "moon dark"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "anonymouse"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "anonymous"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "reverse engineering"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "doom"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "fieldr"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "waldmanb"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "no one"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "everyone"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "beta testers"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "beta tester"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "mac users"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "unregistered"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "macattic"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "macuser"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "macworld"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "macweek"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "hacker"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "pirate"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "mactech"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "macattic"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "copland"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "gershwin"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "microsoft"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "bill gates"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "the devil"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "rupert murdoch"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "the shadows"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "fantasoft"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "apple"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "sean sayrs"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "tim phillips"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "jim foley"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "realmz"))
    bad = TRUE;
  if (!strcmp(gotword, (StringPtr) "bratburdr"))
    bad = TRUE;

  if ((bad == TRUE) || (force == TRUE)) /********* erase registration and trash files! **************/
  {
    openpref(10); // erase current preferences file
    MyrBitSetLong(&serial, 8);
    MyrBitSetLong(&serial, 6 + divine);
    showserial = serial;
    Name_String[0] = 0;
    quest[127] = 0;
    savepref();
    rename(":Data Files:Data S", ":Data Files:Data Spell");
    if (force)
      scratch(451);
    else
      scratch(453);
  }

  CtoPstr(gotword);
}
