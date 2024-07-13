#include "prototypes.h"
#include "variables.h"

/*********************** updatemusic *****************/
void updatemusic(void) {
}

/*********************** centerpict *****************************/
void centerpict(void) {
  Boolean isnote = 0;
  Boolean ispath = 0;
  Boolean issecret = 0;
  long tempy;
  register short tt, t;
  short tempicon;
  Rect source, bitrect, copyrect;

  if (incombat) {
    centerfield(5 + (2 * screensize), 5 + screensize);
    return;
  }

  MyrCheckMemory(1);
  updatemusic();
  lookrect.left = okout = 0;

  if (indung) {
    if (editon == 1) /***** 1 == show map 2 == edit map ****/
    {
      templong = themap.startx;
      tempy = themap.starty;
      updatewalls(templong, tempy);
      editon = 0;
    } else if ((viewtype == 1) && (needdungeonupdate))
      UpdateWindow(0);
    else
      updatewalls(floorx - 10, floory - 10);

    SetPort((GrafPtr)GetWindowPort(screen));
    return;
  } else if ((FrontWindow() != look) && (FrontWindow() != templewindow)) {
    // SelectWindow(look);
    BringToFront(look);
    SetPort((GrafPtr)GetWindowPort(look));
  }

  lookx += (partyx - 8);
  looky += (partyy - 6);

  partyx = 8;
  partyy = 6;

  if (lookx < 0) {
    partyx += lookx;
    lookx = 0;
  } else if (lookx > 75) {
    partyx += lookx - 75;
    lookx = 75;
  }

  if (looky < 0) {
    partyy += looky;
    looky = 0;
  } else if (looky > 77) {
    partyy += looky - 77;
    looky = 77;
  }

  if (partyy > 13)
    partyy = 12;
  else if (partyy < 0)
    partyy = 0;
  if (partyx > 15)
    partyx = 14;
  else if (partyx < 0)
    partyx = 0;

  cansee2(partyx, partyy);

  xy(0);

  SetPort((GrafPtr)GetWindowPort(look));
  ForeColor(blackColor);
  BackColor(whiteColor);

  icon.top = -32;
  icon.bottom = 0;

  for (tt = looky; tt < 13 + looky; tt++) {
    icon.top += 32;
    icon.bottom += 32;
    icon.left = -32;
    icon.right = 0;

    for (t = lookx; t < 15 + lookx; t++) {
      icon.left += 32;
      icon.right += 32;

      if ((!site[t][tt]) && (randlevel.uselos))
        fastplot(252, icon, 0, 1); /**** showblack ****/
      else {
        tempicon = field[t][tt];

        if (tempicon < 0) /**** special icon *****/
        {
          if (tempicon < -1999)
            tempicon += 2000;
          else if (tempicon < -999)
            tempicon += 1000;
        monstericon:
          fastplot(basetile[lastpix], icon, 0, 1); /***** 0 = look, 1 = buff ******/
          iconhand = NIL;
          iconhand = GetCIcon(tempicon);
          if (iconhand) {
            if (tempicon > 200) //** adjust size for monster icon on screen
            {
              if ((**iconhand).iconBMap.bounds.bottom == 64)
                icon.top -= 32;
              if ((**iconhand).iconBMap.bounds.right == 64)
                icon.left -= 32;
            }
            GetGWorld(&savedPort, &savedDevice); /********* get current drawing port info ****/
            SetGWorld(gbuff, NIL); /******** prepare to place horse icon in offscreen GWorld ******/
            PlotCIcon(&icon, iconhand);

            if (tempicon > 200) //** adjust size for monster icon on screen
            {
              if ((**iconhand).iconBMap.bounds.bottom == 64)
                icon.top += 32;
              if ((**iconhand).iconBMap.bounds.right == 64)
                icon.left += 32;
            }

            DisposeCIcon(iconhand);
            SetGWorld(savedPort, savedDevice); /********* reset drawing port to what it was before ****/
          }
        } else {
          if (MyrBitTstShort(&tempicon, 1)) {
            MyrBitClrShort(&tempicon, 1);
            isnote = 1;
          }

          if (MyrBitTstShort(&tempicon, 2)) {
            MyrBitClrShort(&tempicon, 2);
            ispath = 1;
          }

          if (tempicon > 999) {
            tempicon -= 1000;

            if (tempicon > 999) {
              tempicon -= 1000;
              issecret = 1;

              if (tempicon > 999) {
                tempicon -= 1000;
                issecret = 0;
              }
            }
          }

          if (tempicon > 200)
            goto monstericon;

          tempicon--;

          source.top = tempicon / 20 * 32;
          source.left = 32 * (tempicon % 20);

          source.right = source.left + 32;
          source.bottom = source.top + 32;

          {
            BitMap* src = GetPortBitMapForCopyBits(gthePixels);
            BitMap* dst = GetPortBitMapForCopyBits(gbuff);
            CopyBits(src, dst, &source, &icon, 0, NIL);
          }

          if (issecret) {
            fastplot(251, icon, 36, 1);
            issecret = 0;
          }

          if (isnote) {
            fastplot(231, icon, 36, 1);
            isnote = 0;
          }

          if (ispath) {
            fastplot(253, icon, 36, 1);
            ispath = 0;
          }
        }
      }
    }
  }

  icon.top = partyy * 32;
  icon.left = partyx * 32;
  icon.right = icon.left + 32;
  icon.bottom = icon.top + 32;

  if (randlevel.isdark) {
    if (screensize) {
      copyrect.top = partyy * 32 - 208;
      copyrect.bottom = copyrect.top + 416;
      copyrect.left = partyx * 32 - 208;
      copyrect.right = copyrect.left + 416;
    } else {
      copyrect.top = partyy * 32 - 160;
      copyrect.bottom = copyrect.top + 320;
      copyrect.left = partyx * 32 - 160;
      copyrect.right = copyrect.left + 320;
    }

    SectRect(&lookrect, &copyrect, &copyrect);

    SetRect(&bitrect, 0, 0, 320, 320);

    temp = copyrect.bottom - copyrect.top;

    if (temp < (320 + downshift)) {
      if (partyy > 6 + (2 * screensize))
        bitrect.bottom = temp;
      else
        bitrect.top = (320 + downshift) - temp;
    }

    temp = copyrect.right - copyrect.left;

    if (temp < (320 + leftshift)) {
      if (partyx > (8 + (screensize * 3)))
        bitrect.right = temp;
      else
        bitrect.left = (320 + leftshift) - temp;
    }
  }
  iconhand = NIL;

  if (charnum > 5)
    charnum = 5;
  if (charnum < 0)
    charnum = 0;

  if (innotes)
    iconhand = GetCIcon(145);
  else if (inboat) {
    if (boatright)
      iconhand = GetCIcon(13400 + lastpix);
    else
      iconhand = GetCIcon(13300 + lastpix);
  } else if (incamp)
    iconhand = GetCIcon(178);
  else if (basescale[lastpix])
    iconhand = GetCIcon(9500 + horseicon - (500 * face));
  else
    iconhand = GetCIcon(175 + (11 * face));

  icon.top = partyy * 32;
  icon.left = partyx * 32;
  icon.bottom = icon.top + 32;
  icon.right = icon.left + 32;

  SetPort((GrafPtr)GetWindowPort(screen));
  if (iconhand)
    PlotCIcon(&icon, iconhand);
  SetPort((GrafPtr)GetWindowPort(look));

  if (iconhand) {
    GetGWorld(&savedPort, &savedDevice); /********* get current drawing port info ****/
    SetGWorld(gbuff, NIL); /******** prepare to place horse icon in offscreen GWorld ******/
    PlotCIcon(&icon, iconhand);
    DisposeCIcon(iconhand);
    SetGWorld(savedPort, savedDevice); /********* reset drawing port to what it was before ****/
  }

  if (!randlevel.isdark) {
    BitMap* src = GetPortBitMapForCopyBits(gbuff);
    BitMap* dst = GetPortBitMapForCopyBits(GetWindowPort(look));
    CopyBits(src, dst, &lookrect, &lookrect, 0, NIL);
    QDFlushPortBuffer(GetWindowPort(look), NULL);
  } else {
    BitMap* pix = GetPortBitMapForCopyBits(gthePixels);
    BitMap* buf = GetPortBitMapForCopyBits(gbuff);
    BitMap* buf2 = GetPortBitMapForCopyBits(gbuff2);
    BitMap* map = GetPortBitMapForCopyBits(gmaps);
    BitMap* lookpix = GetPortBitMapForCopyBits(GetWindowPort(look));

    CopyBits(pix, buf2, &tiny[15], &lookrect, 0, NIL);
    CopyMask(buf, map, buf2, &copyrect, &bitrect, &copyrect);
    CopyBits(buf2, lookpix, &lookrect, &lookrect, 0, NIL);
  }

  SetPort((GrafPtr)GetWindowPort(screen));
}
