#include "prototypes.h"
#include "variables.h"

/******************************** cast *******************/
void cast(short targetnum, short who) {
  short newhit;
  short t, targetx, targety;
  short times = 128;
  short penwidth;

  Rect orig, dest, oldorig, store, test, copyrect;
  float deltx, delty, stepx, stepy, partx, party;
  Boolean first, bad;

  store.left = 0;
  store.top = 352;
  store.right = 32;
  store.bottom = 384;

  SetCCursor(sword);

  SetRect(&copyrect, 33, 33, 287, 287);
  blankround = 0;

  center(who);

  SetPort((GrafPtr)GetWindowPort(screen));
  ForeColor(blackColor);

  if (spellinfo.spellclass != 9)
    ploticon3(11992 + spellinfo.spelllook1 * 8, spell);
  times = 128 + delayspeed;

  // NOTE(fuzziqersoftware): Made fast spell casting apply to missile weapons
  // and single-target spells too, since recompositing is too slow otherwise.
  if ((showcast) /* && (spellinfo.targettype != 6) && (spellinfo.spellclass != 9) && (targetnum > 1) */)
    times = 30 + delayspeed;

  curControlValue = spellinfo.cost * powerlevel; /***** spell energy used *****/
  if (spellinfo.targettype == 9)
    curControlValue = 0;

  if (powerlevel < 1)
    powerlevel = 1;

  if (spellinfo.targettype == 0)
    curControlValue /= powerlevel;

  for (loop = 0; loop < targetnum; loop++) /****** end ******/
  {
    // NOTE(fuzziqersoftware): This makes spell casting and missile weapons smoother.
    int enable_recomposite = WindowManager_SetEnableRecomposite(0);

    first = bad = 0;
    SetPort((GrafPtr)GetWindowPort(look));
    ForeColor(blackColor);
    BackColor(whiteColor);

  recalc:

    if (who < 9) {
      orig.top = pos[charup][1] * 32;
      orig.left = pos[charup][0] * 32;
    } else {
      orig.top = monpos[monsterup][1] * 32;
      orig.left = monpos[monsterup][0] * 32;
      if ((monster[monsterup].size == 1) || (monster[monsterup].size == 3))
        orig.top -= 16;
      if (monster[monsterup].size > 1)
        orig.left -= 16;
    }
    orig.bottom = orig.top + 32;
    orig.right = orig.left + 32;

    if (first) {
      orig.top += stepy * t;
      orig.left += stepx * t;
      partx = orig.left;
      party = orig.top;
      orig.bottom = orig.top + 32;
      orig.right = orig.left + 32;
      goto returncalc;
    }

    if ((spellinfo.targettype > 1) || ((spellinfo.size) && (spellinfo.targettype < 2))) {
      dest.top = target[loop][1] * 32 - 32 * fieldy;
      dest.left = target[loop][0] * 32 - 32 * fieldx;
      goto bypass;
    } else {
      if (target[loop][1] == 2) {
        temp = target[loop][0] - 10;
        dest.top = monpos[temp][1] * 32;
        dest.left = monpos[temp][0] * 32;
        temp = monster[temp].size;
        if ((temp == 1) || (temp == 3))
          dest.top -= 16;
        if (temp > 1)
          dest.left -= 16;
      } else {
        dest.top = pos[target[loop][0]][1] * 32;
        dest.left = pos[target[loop][0]][0] * 32;
      }
    }

  bypass:
    targetx = dest.left / 32;
    targety = dest.top / 32;

    dest.bottom = dest.top + 32;
    dest.right = dest.left + 32;

    sound(600 + spellinfo.sound1);

    deltx = dest.left - orig.left;
    delty = dest.top - orig.top;

    if (times < 1)
      times = 1;

    stepx = deltx / times;
    stepy = delty / times;

    partx = orig.left;
    party = orig.top;
    if (SectRect(&orig, &copyrect, &test)) {
      BitMapPtr src = GetPortBitMapForCopyBits(GetQDGlobalsThePort());
      BitMapPtr dst = GetPortBitMapForCopyBits(gthePixels);
      CopyBits(src, dst, &orig, &store, 0, NIL);
    }
    oldorig = orig;

    switch (spellinfo.spelllook1) /********** new lazer type weapons ********/
    {

      case 20:
        PenMode(2); /********* Single laser beam *****/
        temp = -1;
        for (penwidth = -4; penwidth < 4; penwidth++) {
          temp++;
          PenSize(abs(penwidth), abs(penwidth));
          MoveTo(orig.left + 16, orig.top + 16);
          LineTo(dest.left + 16, dest.top + 16);
          MoveTo(orig.left + 16, orig.top + 16);
          delay(1);
          LineTo(dest.left + 16, dest.top + 16);
        }
        goto lazeguns;
        break;

      case 21:
        PenMode(2); /********* Scatter laser beams *****/
        temp = -1;
        for (penwidth = -4; penwidth < 4; penwidth++) {
          temp++;
          dest.left += Rand(10) - 5;
          dest.top += Rand(10) - 5;
          PenSize(abs(penwidth), abs(penwidth));
          MoveTo(orig.left + 16, orig.top + 16);
          LineTo(dest.left + 16, dest.top + 16);
          MoveTo(orig.left + 16, orig.top + 16);
          delay(1);
          LineTo(dest.left + 16, dest.top + 16);
        }
        goto lazeguns;
        break;

      case 22:
        PenMode(2); /********* Expand Ring Circles *****/
        PenSize(4, 4);
        oldorig = dest;
        InsetRect(&dest, 12, 12);
        for (t = 0; t < 8; t++) {
          FrameOval(&dest);
          delay(1);
          FrameOval(&dest);
          InsetRect(&dest, -4, -4);
        }
        goto lazeguns;
        break;

      case 23:
        PenMode(2); /********* Close Ring Circles *****/
        PenSize(4, 4);
        oldorig = dest;
        InsetRect(&dest, -32, -32);
        for (t = 0; t < 8; t++) {
          FrameOval(&dest);
          delay(1);
          FrameOval(&dest);
          InsetRect(&dest, 4, 4);
        }
        goto lazeguns;
        break;

      case 24:
        PenMode(2); /********* Expand & Close Ring Circles *****/
        PenSize(4, 4);
        oldorig = dest;
        InsetRect(&dest, -32, -32);
        for (t = 0; t < 8; t++) {
          FrameOval(&dest);
          InsetRect(&dest, 32 - 8 * t, 32 - 8 * t);
          FrameOval(&dest);
          delay(1);
          FrameOval(&dest);
          InsetRect(&dest, -32 + 8 * t, -32 + 8 * t);
          FrameOval(&dest);
          InsetRect(&dest, 4, 4);
        }
        goto lazeguns;
        break;
    }

    for (t = 0; t < times; t++) {
      newhit = abs(field[fieldx + (orig.left + 16) / 32][fieldy + (orig.top + 16) / 32]);

      if ((spellinfo.targettype == 6) && (spellinfo.range1 + spellinfo.range2 > 0)) {
        if (newhit > 1000) {
          if (mapstats[newhit - 1000].los) {
            sound(-10082);
            if (target[loop][1] < 3) {
              bodyground(target[loop][0], 0);
              drawbody(target[loop][0], 0, 0);
            }
            target[loop][0] = target[loop][1] = -1;
            t = 1000;
            bad = TRUE;
          }
        }
      }

    lazeguns:

      if (spellinfo.spelllook1 < 20) {
        if (spellinfo.targettype == 6) {
          if (q[up] > -1)
            track[q[up]] = 0;
          if ((newhit <= maxloop) && (newhit != q[up])) {
            if (!track[newhit]) {
              track[newhit] = 1;
              if (SectRect(&oldorig, &copyrect, &test)) {
                BitMapPtr src = GetPortBitMapForCopyBits(gthePixels);
                BitMapPtr dst = GetPortBitMapForCopyBits(GetWindowPort(look));
                CopyBits(src, dst, &store, &oldorig, 0, NIL);
              }
              resolvespell();
              if (SectRect(&orig, &copyrect, &test)) {
                BitMapPtr dst = GetPortBitMapForCopyBits(gthePixels);
                BitMapPtr src = GetPortBitMapForCopyBits(GetWindowPort(look));
                CopyBits(src, dst, &orig, &store, 0, NIL);
              }
              track[newhit] = -1;
            }
          }
        }

        party += stepy;
        partx += stepx;

        orig.top = party;
        orig.left = partx;
        orig.right = orig.left + 32;
        orig.bottom = orig.top + 32;

        if (SectRect(&oldorig, &copyrect, &test)) {
          BitMapPtr src = GetPortBitMapForCopyBits(gthePixels);
          BitMapPtr dst = GetPortBitMapForCopyBits(GetWindowPort(look));
          CopyBits(src, dst, &store, &oldorig, 0, NIL);
        }

        if (!SectRect(&orig, &copyrect, &test)) {
          if (!first) {
            centerfield(targetx, targety);
            SetPort((GrafPtr)GetWindowPort(look));
            showtargets(0);
            first = TRUE;
            oldorig = orig;
            goto recalc;
          }
          goto pushon;
        }
      returncalc:
        if (SectRect(&orig, &copyrect, &test)) {
          BitMapPtr dst = GetPortBitMapForCopyBits(gthePixels);
          BitMapPtr src = GetPortBitMapForCopyBits(GetWindowPort(look));
          CopyBits(src, dst, &orig, &store, 0, NIL);
          fastplot(200 + spellinfo.spelllook1, orig, 36, 0);
        }
      pushon:
        oldorig = orig;
      }

      WindowManager_RecompositeAlways();
    }
    if (SectRect(&oldorig, &copyrect, &test)) {
      BitMapPtr src = GetPortBitMapForCopyBits(gthePixels);
      BitMapPtr dst = GetPortBitMapForCopyBits(GetWindowPort(look));
      CopyBits(src, dst, &store, &oldorig, 0, NIL);
    }

    WindowManager_SetEnableRecomposite(enable_recomposite);

    if ((spellinfo.targettype != 6) && (!bad))
      spelltargets(loop, -1, 0, 0);
    else if (spellinfo.targettype == 6) {
      inspell = FALSE;
      if (q[up] < 10)
        centerfield(5 + (2 * screensize), 5 + screensize);
    }
    target[loop][0] = target[loop][1] = -1;
    for (t = 0; t < maxloop; t++)
      track[t] = 0;
  }
  SetPort((GrafPtr)GetWindowPort(look));
}
