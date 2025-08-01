#include "prototypes.h"
#include "realmzbuild.h"
#include "variables.h"

/********************* partyloss ******************************/
void partyloss(short mode) {
  WindowPtr tempwindow;
  char reviveparty = 0;

  compactheap();

  if (revivepartyflag)
    goto revive; // Fantasoft v7.1

  getfilename("Global"); /******** run global death macro *****/
  if ((fp = MyrFopen(filename, "rb")) != NULL) {
    fread(&globalmacro, sizeof globalmacro, 1, fp);
    CvtTabShortToPc(&globalmacro, 30);
    fclose(fp);
    incombat = 0;
    if (globalmacro[1]) {
      updatemain(TRUE, -1);
      reviveparty = newland(0L, 0L, 1, globalmacro[1], 0);
    }
    if ((reviveparty == -1) || (revivepartyflag == TRUE)) {
      revivepartyflag = FALSE;
    revive:
      coward = TRUE; /**** dont award treasure ******/
      killmon = numenemy;
      incombat = FALSE;
      return; /****** saved by a 119 code to rivive party *****/
    }
  }

  if (!mode) {
    for (t = 0; t < 20; t++) {
      fumque[t] = 0;
      tempwindow = NIL;
      tempwindow = FrontWindow();
      if (tempwindow != GetDialogWindow(background))
        DisposeWindow(tempwindow);
    }

    background = GetNewDialog(129 + (1000 * divine), 0L, (WindowPtr)-1L);
    MoveWindow(GetDialogWindow(background), GlobalLeft + 1 + (leftshift / 2), GlobalTop + 1 + (downshift / 2), FALSE);
    ShowWindow(GetDialogWindow(background));
    DrawDialog(background);

    look = NIL;
    gWindow = NIL;
    /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
     * NOTE(fuzziqersoftware): The original code doesn't set screen to NIL
     * here, so if the player dies or ends a game, then starts a new one, the
     * game will try to delete a window again that was already deleted (see
     * mainscreeninit in misc.c).
     */
    screen = NIL;
    /* *** END CHANGES *** */
  }
  for (t = 0; t < 60; t++)
    que[t].duration = que[t].phase = 0;

  DisableItem(gScenario, 0);
  DisableItem(gNPC, 0);
  EnableItem(gParty, 0);
  DisableItem(gParty, 1);
  DisableItem(gParty, 2);
  DisableItem(gParty, 3);
  DisableItem(gParty, 7);
  EnableItem(gParty, 5);
  EnableItem(gGame, 1);
  DisableItem(gBeast, 0);
  DisableItem(gGame, 3);
  EnableItem(gFile, 1);
  DisableItem(gFile, 3);
  DisableItem(gFile, 4);
  DisableItem(gFile, 2);
  DrawMenuBar();
  SetCCursor(sword);
  if ((!reducesound) && (!mode))
    sound(26260);
  charselectnew = incombat = initems = intemple = inbooty = inscroll = inswap = indung = editon = incamp = inspell = killparty = killmon = shopavail = campavail = canshop = fumtotal = FALSE;
  charnum = -1;
  fat = 4;
  cleartarget();
  for (t = 1; t < 20; t++)
    DisableItem(gScenario, t + 3);

  if (mode)
    return;

  MainLoop();
}
