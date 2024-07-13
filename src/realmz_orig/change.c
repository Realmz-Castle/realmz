#include "prototypes.h"
#include "variables.h"

/**************** change ***********************************/
void change(short play, short mode) {
  Rect shade;
  chr.left = 17 + (leftshift / 2);
  chr.top = 402;
  chr.bottom = chr.top + 32;

  SetPort((GrafPtr)GetWindowPort(gshop));
  updateport();
  updateshopwings(cl, cr);

  if (c[cl].load < 0)
    c[cl].load = 0;
  characterl = c[cl];

  if (cr != -1) {
    if (c[cr].load < 0)
      c[cr].load = 0;
    characterr = c[cr];
  }

  RGBBackColor(&greycolor);

  if (mode)
    displaytag = TRUE;

  if (play)
    sound(141);

  if (theControl == rightarrow) {
    chr.left = 375 + (leftshift / 2);
    shop = TRUE;
    chr.right = chr.left + 32;
    theControl = shopitemsvert;

    if (!mode) {
      SetControlMaximum(shopitemsvert, 0);
      SetControlValue(shopitemsvert, 0);
      TextSize(16);
      TextMode(1);

      eraseshopname(1);

      ForeColor(yellowColor);

      if (cr == -1) {
        SetControlMaximum(shopitemsvert, 191);
        GetControlBounds(rightcharacter, &shade);
        // shade = *&(**(rightcharacter)).contrlRect;
        pict(0, shade);
        ploticon3(2005, chr);
        MyrDrawCString("Shop Keeper");
      } else {
        Rect r;
        MyrDrawCString(c[cr].name);
        GetControlBounds(goshopinshop, &r);
        pict(202, r);
      }
    }

    if (cr > -1) {
      shop = FALSE;
      GetControlBounds(rightcharacter, &shade);
      // shade = *&(**(rightcharacter)).contrlRect;
      pict(0, shade);

      plotportrait(characterr.pictid, chr, characterr.caste, cr);

      if (characterr.stamina < 1)
        ploticon3(2019, shade);

      if (characterr.stamina < -9)
        ploticon3(2015, chr);
      if (characterr.numitems > 9)
        SetControlMaximum(shopitemsvert, characterr.numitems - 9);
    }
    Display(mode);

  } else {
    chr.right = chr.left + 32;
    GetControlBounds(leftcharacter, &shade);
    // shade = *&(**(leftcharacter)).contrlRect;
    pict(0, shade);
    TextSize(16);
    TextMode(1);
    eraseshopname(0); /*** left ***/
    ForeColor(yellowColor);
    MyrDrawCString(characterl.name);

    plotportrait(characterl.pictid, chr, characterl.caste, cl);

    if (characterl.stamina < 1)
      ploticon3(2019, shade);

    if (characterl.stamina < -9)
      ploticon3(2015, chr);

    if (!mode) {
      SetControlValue(charitemsvert, 0);
      SetControlMaximum(charitemsvert, 0);
    }
  }

  if ((first == FALSE) || (theControl == leftarrow)) {
    if (characterl.numitems > 9)
      SetControlMaximum(charitemsvert, characterl.numitems - 9);
    theControl = charitemsvert;
    first = TRUE;
    skip = TRUE;
    Display(0);
    skip = FALSE;
    theControl = NIL;
  }
}
