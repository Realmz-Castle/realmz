#include "prototypes.h"
#include "variables.h"

/**************** showresults ************************/
void showresults(short tar, short special, short from) {
  short t = 0;
  short type = 0;
  Rect monrect;
  short soundtoplay;

  iconhand = NIL;
  /****************** special codes ***********
  -52= Creature Disarms;
  -51= Need Sharp Weapon;
  -50= Need Blunt Weapon;
  -49= Stoned;
  -48= Age PC;
  -47= Goes Blind;
  -46= Fumble Weapon;
  -45= Need Special Weapon;
  -40= Weapon adds condition;
  -24= Saved Vs.  No Effect:
  -25= Saved Vs.Caster:
  -18= Drain Victory:
  -17= Drain Spell points:
  -15= Need Magical Weapon:
  -14= Destroyed:
  -13= Turned:
  -12= Summoned:
  -11= Unconsious:
  -10= Dodge Missile:
  -9 = Spell Failed:
  -8 = Flees From Battle:
  -7 = Surrenders:
  -6 = Run Away:
  -5 = Guarding:
  -4 = Immune:
  -3 = Resist Magic:
  -2 = Death:
  -1 = Miss:
  0 = Damage:
  1 = In Retreat:
  2 = Helpless:
   *********************************************/
  canundo = rotate = 0;

  SetPort(GetWindowPort(screen));
  if (incombat) {
    if (damage)
      HideCursor();
    pict(156, infosmall);
    monrect = spellrect;

    if ((spellinfo.targettype != 9) && (spellinfo.targettype != 10) && (spellinfo.targettype != 6))
      center(tar);

    if (tar > 9) {
      temp = monster[tar - 10].iconid;
      if (monster[tar - 10].lr == 1)
        temp += 308;
      iconhand = GetCIcon(temp);
      size = monster[tar - 10].size;
    } else {
      size = 0;
      iconhand = GetCIcon(c[tar].iconid);
    }

    if ((size == 2) || (size == 0)) {
      monrect.top += 16;
      monrect.bottom -= 16;
    }
    if ((size == 1) || (size == 0)) {
      monrect.left += 16;
      monrect.right -= 16;
    }
    pict(207, spellrect);
    if (iconhand) {
      PlotCIcon(&monrect, iconhand);
      DisposeCIcon(iconhand);
    }
  } else {
    TextFace(bold);
    RGBBackColor(&greycolor);
    EraseRect(&textrect);
  }

  TextMode(1);
  TextSize(21);
  TextFont(font);

  if (incombat)
    MoveTo(420 + leftshift, 440 + downshift);
  else {
    MoveTo(30, 425 + downshift);
    icon.top = 340 + downshift;
    icon.left = 24;
    icon.right = icon.left + 64;
    icon.bottom = icon.top + 64;
    EraseRect(&icon);
    iconhand = NIL;
    if (tar < 6) {
      icon.left += 16;
      icon.right -= 16;
      icon.top += 16;
      icon.bottom -= 16;
      iconhand = GetCIcon(c[tar].iconid);
    } else {
      iconhand = GetCIcon(monster[tar - 10].iconid);
      size = monster[tar - 10].size;

      if ((size == 2) || (size == 0)) {
        icon.top += 16;
        icon.bottom -= 16;
      }
      if ((size == 1) || (size == 0)) {
        icon.left += 16;
        icon.right -= 16;
      }

      ForeColor(yellowColor);
      MoveTo(30, 425 + downshift);
    }
    if (iconhand) {
      PlotCIcon(&icon, iconhand);
      DisposeCIcon(iconhand);
    }
  }

  ForeColor(yellowColor);

  switch (special) {

    case 0:

      string(abs(damage));
      ForeColor(redColor);

      if (poisoned) {
        sound(684);
        MyrDrawCString(" Pts from Poison!");
        specdamage = damage;
        type = 165;
        goto bypass;
      }

      if (regenerate) {
        sound(626);
        regenerate = FALSE;
        MyrDrawCString(" Pts Regeneration!");
        goto bypass;
      }

      if ((!inspell) && (incombat)) {
        SetPort(GetWindowPort(screen));
        if (from > 9) {
          if (monster[from - 10].weapon) {
            if (item.blunt == -2)
              sound(randrange(635, 637)); // sound(item.sound + 600);
            else if (Rand(100) < 50)
              sound(632);
            else
              sound(639);
            type = 160;
          } else {
            type = 161;
            soundtoplay = 600 + monster[from - 10].attacks[0][2]; // behind sound
            if (!behind) {
              if (monster[from - 10].attacks[attackloop][0])
                soundtoplay = (600 + monster[from - 10].attacks[attackloop][2]);
              else
                soundtoplay = (600 + monster[from - 10].attacks[0][2]);
            }

            if (soundtoplay == 631)
              soundtoplay = 632;

            sound(soundtoplay);
          }
        } else {
          if (behind > 0) {
            if (behind == 2)
              MyrDrawCString(" Pts Major Wound!");
            else
              MyrDrawCString("    Pts from Behind!");
            behind = FALSE;
            goto showhit;
          } else if (behind < 0) {
            switch (behind) {
              case -1:
                MyrDrawCString(" Pts Sneak Attack!");
                goto showhit;
                break;

              case -2:
                MyrDrawCString(" Pts Major Wound!");
                goto showhit;
                break;
            }
            behind = FALSE;
          } else if ((from <= charnum) && (from > -1)) {
          showhit:
            sound(600 + c[from].weaponsound);
            if (c[from].armor[2])
              type = 160;
            else
              type = 161;
            if (behind == 1) {
              MyrDrawCString(" Pts from Behind!");
              goto bypass;
            }
          }
        }
      }

      if (damage > -1)
        MyrDrawCString(" Points Damage");
      else if (damage < 0) {
        MyrDrawCString(" Points Healing");
        if (tar > 9) {
          if (monster[tar - 10].stamina > monster[tar - 10].staminamax)
            monster[tar - 10].stamina = monster[tar - 10].staminamax;
        }
      }

    bypass:

      if (damage) {
        if (tar < 9) {
          c[tar].beenattacked = TRUE;
          updatecharshort(tar, FALSE);
          if (tar == q[up])
            updatecontrols();
        } else
          monster[tar - 10].beenattacked = TRUE;
      }

      break;

    case -40: /**** weapon adds condition *****/
      GetIndString(myString, 134, item.sp3 - 19);
      PtoCstr(myString);
      MyrDrawCString((Ptr)myString);
      sound(item.sound + 600);
      type = 159;
      break;

    case -34:
      MyrDrawCString("Diseased!");
      if (tar < 9)
        updatechar(tar, 3);
      sound(684);
      type = 165;
      break;

    case -33:
      MyrDrawCString("Frightened!");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 166;
      break;

    case -32:
      MyrDrawCString("Paralyzed!");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 166;
      break;

    case -31:
      MyrDrawCString("Cursed");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 166;
      break;

    case -30:
      MyrDrawCString("Stupid");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 166;
      break;

    case -29:
      MyrDrawCString("Entangled");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 166;
      break;

    case -28:
      MyrDrawCString("You are poisoned");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 165;
      break;

    case -27:
      MyrDrawCString("Confused");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 166;
      break;

    case -26:
      MyrDrawCString("Charmed");
      if (tar < 9)
        updatechar(tar, 3);
      sound(651);
      type = 166;
      break;

    case -25:
      MyrDrawCString("Resists Spell");
      sound(133);
      bodyground(tar, 0);
      drawbody(tar, FALSE, 0);
      delay(0);
      delay(20);
      if (!incombat)
        EraseRect(&textrect);
      break;

    case -24:
      MyrDrawCString("DRVs Successful");
      sound(133);
      bodyground(tar, 0);
      drawbody(tar, FALSE, 0);
      delay(0);
      delay(20);
      if (!incombat)
        EraseRect(&textrect);
      break;

    case -23:
      string(specdamage);
      ForeColor(redColor);
      MyrDrawCString(" Pts Mental Damage");
      type = 166;
      break;

    case -22:
      string(specdamage);
      ForeColor(redColor);
      MyrDrawCString(" Pts Chemical Burn");
      type = 165;
      break;

    case -21:
      string(specdamage);
      ForeColor(redColor);
      MyrDrawCString(" Pts Shock Damage");
      type = 164;
      break;

    case -20:
      string(specdamage);
      ForeColor(redColor);
      MyrDrawCString(" Pts Cold Damage");
      type = 163;
      break;

    case -19:
      string(specdamage);
      ForeColor(redColor);
      MyrDrawCString(" Pts Fire Damage");
      type = 162;
      break;

    case -18:
      MyrDrawCString("Lost Victory Points");
      sound(693);
      delay(40);
      break;

    case -17:
      MyrDrawCString("Lost Spell Points");
      sound(693);
      delay(40);
      break;

    case 29:
      string(damage);
      ForeColor(redColor);
      MyrDrawCString(" Pts from Disease");
      sound(684);
      if (tar < 9)
        updatechar(tar, 3);
      delay(0);
      break;

    case -15:
      MyrDrawCString("Need Stronger Magic!");
      sound(698);
      delay(60);
      break;

    case -52:
      MyrDrawCString("Creature Disarms");
      sound(655);
      delay(60);
      break;

    case -51:
      MyrDrawCString("Need Sharp Weapon");
      sound(639);
      delay(60);
      break;

    case -50:
      MyrDrawCString("Need Blunt Weapon");
      sound(621);
      delay(60);
      break;

    case -45:
      MyrDrawCString("Need Special Weapon");
      sound(698);
      delay(60);
      break;

    case -46:
      if (allowfumble)
        MyrDrawCString("Fumbled Weapon");
      else
        MyrDrawCString("Fumble Disabled");
      delay(60);
      break;

    case -47:
      MyrDrawCString("Goes Blind");
      if (tar < 9)
        updatechar(tar, 3);
      sound(631);
      break;

    case -48:
      sound(605);
      MyrDrawCString("Grows Older");
      delay(60);
      break;

    case -49:
      sound(686);
      MyrDrawCString("Turned to Stone");
      if (tar < 9)
        updatechar(tar, 3);
      delay(60);
      break;

    case -14:
      MyrDrawCString(" Destroyed!");
      sound(132);
      if (tar < 9)
        updatecharshort(tar, TRUE);
      break;

    case -13:
      MyrDrawCString(" Turned");
      spellinfo.spelllook2 = 8;
      break;

    case -12:
      MyrDrawCString(" Summoned");
      break;

    case -11:
      ForeColor(yellowColor);
      if (!initems)
        MyrDrawCString(" Unconscious");
      sound(132);
      if (tar < 9) {
        c[tar].traiter = 0;
        updatecharshort(tar, TRUE);
      }
      break;

    case -10:
      MyrDrawCString(" Dodged Missile!");
      bodyground(tar, 0);
      drawbody(tar, FALSE, 0);
      break;

    case -9:
      MyrDrawCString(" Spell Failed!");
      break;

    case -8:
      MyrDrawCString("Flees from Battle!");
      break;

    case -7:
      MyrDrawCString(" Surrenders");
      break;

    case -6:
      MyrDrawCString(" Running Away!");
      if (tar < 9)
        updatechar(tar, 3);
      break;

    case -5:
      MyrDrawCString(" Guarding");
      break;

    case -4:
      MyrDrawCString(" Immune");
      sound(133);
      bodyground(t, 0);
      drawbody(t, FALSE, 0);
      break;

    case -3:
      MyrDrawCString(" Resisted Magic");
      bodyground(tar, 0);
      drawbody(tar, FALSE, 0);
      break;

    case -2:
      MyrDrawCString("    Death");
      sound(132);
      if (tar < 9)
        updatecharshort(tar, TRUE);
      break;

    case -1:
      MyrDrawCString(" Miss");
      if (behind) {
        ForeColor(redColor);
        MyrDrawCString(" from Behind!");
        ForeColor(blackColor);
      }
      sound(650);
      break;

    default:
      if (!initems) {
        GetIndString(myString, 134, special);
        PtoCstr(myString);
        MyrDrawCString((Ptr)myString);
        delay(2);
      }
      break;
  }
  if ((!inspell) && (incombat)) {
    if (type) {
      if (type == 159)
        showdamage(tar, type, item.sp4);
      else if (type > 161)
        showdamage(tar, type, specdamage);
      else
        showdamage(tar, type, damage);

      delay(0);
      delay(30 - oldspeed * 2);
      bodyground(tar, 0);
      drawbody(tar, FALSE, 0);
    } else
      delay(0);
  }

  poisoned = behind = type = 0;
  ShowCursor();
}
