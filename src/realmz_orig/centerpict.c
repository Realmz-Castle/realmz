#include "prototypes.h"
#include "variables.h"

/*********************** updatemusic *****************/
void updatemusic(void) {
  short playlist;

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(iSynic): Restore the playlist selection used by the surviving music
   * call sites.
   */
  if (incombat)
    playlist = 11;
  else if (incamp)
    playlist = 9;
  else if (initems)
    playlist = 6;
  else if (inbooty)
    playlist = 7;
  else if (indung)
    playlist = 2;
  else if (lastpix == 3)
    playlist = 4;
  else if (lastpix == 5)
    playlist = 12;
  else if (lastpix == 6)
    playlist = 15;
  else if (lastpix == 7)
    playlist = 16;
  else if (lastpix == 8)
    playlist = 17;
  else if (lastpix == 9)
    playlist = 13;
  else if (lastpix == 10)
    playlist = 14;
  else if ((lastpix >= 0) && (lastpix < 20) && basescale[lastpix])
    playlist = 3;
  else
    playlist = 1;

  music(playlist);
  /* *** END CHANGES *** */
}

/*********************** centerpict *****************************/
void centerpict(void) {
  Boolean isnote = 0;
  Boolean ispath = 0;
  Boolean issecret = 0;
  int32_t tempy;
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

    /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
     * NOTE(fuzziqersoftware): The original logic was hard to follow and centered the darkness mask in the wrong place
     * after the expansion to 800x600, so I've rewritten it.
     */

    // The upper-left corner of the party icon should be at the center of the darkness mask. That is, the origin of the
    // rect that we're going to pass to CopyMask should be 160 pixels left and above the party's tile's origin. The
    // darkness mask is 320x320, so the rect's size is always the same. In the original logic, the darkness mask was
    // stretched if the screen size is 800x600, but we don't do that - we always reveal the same amount of the
    // playfield, regardless of the window/screen size.
    Rect playfield_rect;
    playfield_rect.left = partyx * 32 - 160;
    playfield_rect.top = partyy * 32 - 160;
    playfield_rect.right = playfield_rect.left + 320;
    playfield_rect.bottom = playfield_rect.top + 320;

    Rect mask_rect = {downshift / 2, leftshift / 2, downshift / 2 + 320, leftshift / 2 + 320};

    // If the playfield rect is out of bounds, clip both rects
    if (playfield_rect.left < 0) {
      mask_rect.left -= playfield_rect.left;
      playfield_rect.left = 0;
    }
    if (playfield_rect.top < 0) {
      mask_rect.top -= playfield_rect.top;
      playfield_rect.top = 0;
    }
    int32_t right_delta = playfield_rect.right - (look->portRect.right - look->portRect.left);
    int32_t bottom_delta = playfield_rect.bottom - (look->portRect.bottom - look->portRect.top);
    if (right_delta > 0) {
      mask_rect.right -= right_delta;
      playfield_rect.right -= right_delta;
    }
    if (bottom_delta > 0) {
      mask_rect.bottom -= bottom_delta;
      playfield_rect.bottom -= bottom_delta;
    }

    BitMap* tiles_buffer = GetPortBitMapForCopyBits(gthePixels);
    BitMap* playfield_buffer = GetPortBitMapForCopyBits(gbuff); // Rendered playfield (from above)
    BitMap* temp_buffer = GetPortBitMapForCopyBits(gbuff2); // Temporary buffer
    BitMap* darkness_mask = GetPortBitMapForCopyBits(gmaps); // Darkness mask
    BitMap* window_buffer = GetPortBitMapForCopyBits(GetWindowPort(look)); // Window image

    // Clear playfield with black. This copies from tiny[15] in gthePixels; `tiny` is an array of Rects that defines
    // the dungeon overhead tileset (part of PICT 302, on the right side). `tiny[15]` is simply a 16x16 black square.
    // TODO: Use PaintRect or something faster than CopyBits here
    CopyBits(tiles_buffer, temp_buffer, &tiny[15], &lookrect, 0, NIL);

    // Copy the rendered tilemap back into the render buffer through the darkness mask
    CopyMask(playfield_buffer, darkness_mask, temp_buffer, &playfield_rect, &mask_rect, &playfield_rect);

    // Copy the rendered playfield (render buffer) to the window
    CopyBits(temp_buffer, window_buffer, &lookrect, &lookrect, 0, NIL);

    /* *** END CHANGES *** */
  }

  SetPort((GrafPtr)GetWindowPort(screen));
}
