#include "prototypes.h"
#include "variables.h"

/************************** bodycount *******************/
void bodycount(void) {
  DialogRef bodyc;
  short count = 0;
  short maxtochose = 4;
  CCrsrHandle compassnew;

  short select[maxloop], chosen = 0;

  for (t = 0; t < maxloop; t++)
    select[t] = 0;

  for (t = 0; t < nummon; t++) {
    monster[t].beenattacked = monster[t].condition[0] = monster[t].condition[1] = monster[t].condition[2] = monster[t].condition[5] = monster[t].condition[29] = monster[t].condition[38] = 0;
    if (monster[t].cansum < 0)
      monster[t].traiter = 0;
    if ((monster[t].stamina > 0) && (!monster[t].traiter) && (monster[t].cansum))
      select[count++] = t + 1;
  }

  if (!count) {
    updatenpcmenu();
    return;
  }

  compactheap();

  bodyc = GetNewDialog(173, 0L, (WindowPtr)-1L);

  gCurrent = bodyc;
  SetPortDialogPort(bodyc);
  BackPixPat(base);
  ForeColor(yellowColor);
  MoveWindow(GetDialogWindow(bodyc), GlobalLeft + (leftshift / 2), GlobalTop + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(bodyc));
  ErasePortRect();
  DrawDialog(bodyc);
  ForeColor(blackColor);
  BackColor(whiteColor);
  TextFont(genevafont);
  TextSize(10);

  sound(29999 + Rand(4));

  for (t = 0; t < count - 1; t++) {
    for (tt = 0; tt < count - 1; tt++) {
      if ((monster[select[tt] - 1].stamina < monster[select[tt + 1] - 1].stamina) || (monster[select[tt + 1] - 1].cansum == -1)) {
        temp = select[tt];
        select[tt] = select[tt + 1];
        select[tt + 1] = temp;
      }
    }
  }

  for (t = 0; t < 32; t++) /***** display the mandatory selection ******/
  {
    if (select[t]) {
      GetDialogItem(bodyc, t + 3, &itemType, &itemHandle, &buttonrect);
      upbutton(FALSE);
      monsterupdate(select[t] - 1, buttonrect);

      if ((monster[abs(select[t]) - 1].cansum == -1) && (chosen < 18)) {
        downbutton(FALSE);
        select[t] = -(abs(select[t]));
        chosen++;
      }
    }
  }

  for (t = 0; t < 32; t++)
    if (select[t] < 0)
      maxtochose++;
  if (maxtochose > 18)
    maxtochose = 18;

  for (t = 0; t < 10; t++) /***** add secondary selection ****/
  {
    if (select[t] > 0) {
      GetDialogItem(bodyc, t + 3, &itemType, &itemHandle, &buttonrect);

      if (chosen < maxtochose) {
        downbutton(FALSE);
        select[t] = -(abs(select[t]));
        chosen++;
      }
    }
  }

over:

  gCurrent = bodyc;
  SetPortDialogPort(bodyc);

  sound(29999 + Rand(4));

  ForeColor(blackColor);
  BackColor(whiteColor);

  BeginUpdate(GetDialogWindow(bodyc));
  EndUpdate(GetDialogWindow(bodyc));

  compassnew = NIL;

  if (((147 - chosen + maxtochose) > 146) && (147 - chosen + maxtochose < 155))
    compassnew = GetCCursor(147 - chosen + maxtochose);
  else
    compassnew = GetCCursor(147);

  if (compassnew) {
    SetCCursor(compassnew);
    DisposeCCursor(compassnew);
  }

  for (;;) {
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    GetDialogItem(bodyc, itemHit, &itemType, &itemHandle, &buttonrect);

    if (itemHit == 2)
      goto out;

    if ((itemHit > 2) && (itemHit < 35)) {
      if (((chosen > (maxtochose - 1)) && (select[itemHit - 3] > 0)) || (!select[itemHit - 3])) {
      toomany:
        TextMode(1);
        TextSize(12);
        ForeColor(yellowColor);
        MyrCDiStr(41, (StringPtr) "You have already bagged your limit -OR- there is no monster here.");
        sound(6000);
        delay(80);
        MyrCDiStr(41, (StringPtr) "");
        goto over;
      } else if (select[itemHit - 3] < 0) {
        if (monster[abs(select[itemHit - 3]) - 1].cansum == -1) {
          upbutton(FALSE);
          sound(141);
          select[itemHit - 3] = abs(select[itemHit - 3]);
          chosen--;

          TextMode(1);
          TextSize(12);
          ForeColor(yellowColor);
          RGBBackColor(&greycolor);
          MyrCDiStr(41, (StringPtr) "This creature may be important to the scenario.  Leave behind at your own risk.");
          sound(6000);
          delay(80);
          MyrCDiStr(41, (StringPtr) "");
          goto over;
        }

        upbutton(FALSE);
        sound(141);
        select[itemHit - 3] = abs(select[itemHit - 3]);
        chosen--;
      } else if (chosen < maxtochose) {
        downbutton(FALSE);
        sound(141);
        select[itemHit - 3] = -(abs(select[itemHit - 3]));
        chosen++;
      } else
        goto toomany;
    }

    if (((147 - chosen + maxtochose) > 146) && (147 - chosen + maxtochose < 155))
      compassnew = GetCCursor(147 - chosen + maxtochose);
    else
      compassnew = GetCCursor(147);
    SetCCursor(compassnew);
    DisposeCCursor(compassnew);
  }

out:
  if (chosen > 18)
    chosen = 18;
  heldover = chosen;

  for (t = 0; t < 32; t++) {
    if (select[t] < 0) {
      chosen--;
      if (chosen > -1) {
        holdover[chosen] = monster[abs(select[t]) - 1];
        for (tt = 0; tt < 4; tt++)
          if (holdover[chosen].condition[tt] > 0)
            holdover[chosen].condition[tt] = 0;
        holdover[chosen].condition[35] = 0;
#if CHECK_ILLEGAL_ACCESS > 0
        if (abs(select[t]) - 1 < 0 || abs(select[t]) - 1 >= 100)
          AcamErreur("monster bad index");
#endif
        monster[abs(select[t]) - 1].stamina = monster[abs(select[t]) - 1].hd = 0;
        for (tt = 0; tt < 6; tt++)
          monster[abs(select[t]) - 1].items[tt] = 0;
        for (tt = 0; tt < 3; tt++)
          monster[abs(select[t]) - 1].money[tt] = 0;
      }
    }
  }
  for (t = 0; t < maxmon; t++)
    monster[t].stamina = 0;
  DisposeDialog(bodyc); // Myriad
  updatenpcmenu();
}

/********************** monsterupdate *****************************/
void monsterupdate(short who, Rect where) {

  ForeColor(yellowColor);

  MoveTo(where.left + 5, where.bottom + 14);
  string(monster[who].stamina);
  MoveTo(where.left + 40, where.bottom + 14);
  string(monster[who].staminamax);

  ForeColor(whiteColor);
  MoveTo(where.left + 5, where.bottom + 29);
  string(monster[who].spellpoints);

  MoveTo(where.left + 40, where.bottom + 29);
  string(monster[who].maxspellpoints);

  if (monster[who].cansum == -1) /***** Show Allies *******/
  {
    icon.top = where.top;
    icon.left = where.left;
    icon.right = where.left + 64;
    icon.bottom = where.top + 64;
    ploticon2(2003);
  }

  iconhand = NIL;

  iconhand = GetCIcon(monster[who].iconid);

  if (!monster[who].size)
    InsetRect(&where, 19, 19);
  if (monster[who].size == 2)
    InsetRect(&where, 3, 19);
  if (monster[who].size == 1)
    InsetRect(&where, 19, 3);
  if (monster[who].size == 3)
    InsetRect(&where, 3, 3);

  if (iconhand) {
    PlotCIcon(&where, iconhand);
    DisposeCIcon(iconhand);
  }
}

/**************************** updatenpcmenu *****************/
void updatenpcmenu(void) {
  short t;
  char monstername[255];

  DisableItem(gNPC, 0);

  strcpy(monstername, (StringPtr) " "); // Myriad
  CtoPstr(monstername); // Myriad
  for (t = 1; t <= 20; t++) {
    SetMenuItemText(gNPC, t, (StringPtr)monstername);
    DisableItem(gNPC, t);
    SetItemIcon(gNPC, t, 0);
    if (t >= heldover)
      holdover[t].name = 0; // Fantasoft v7.1   Clean out any junk that does not belong.
  }

  for (t = 1; t <= heldover; t++) {
    EnableItem(gNPC, 0);
    EnableItem(gNPC, t);
    strcpy((StringPtr)monstername, (StringPtr)holdover[t - 1].monname);
    CtoPstr(monstername);
    SetMenuItemText(gNPC, t, (StringPtr)monstername);
    if (holdover[t - 1].iconid < 512)
      SetItemIcon(gNPC, t, holdover[t - 1].iconid);
    holdover[t - 1].beenattacked = holdover[t - 1].beenattacked = holdover[t - 1].target = 0;
  }
  SetMenuBar(myMenuBar);
  InsertMenu(gSound, -1);
  InsertMenu(gSpeed, -1);
  DrawMenuBar();
}
