#ifndef STRUCTS_H
#define STRUCTS_H

#include "../Types.h"

typedef struct {
  short delayspeed, oldspeed, oldvolume, volume;
  char defaultspell, showcast, usehashmarks, numchannel, lastgame, horseicon, dropitemprotection, forgettreasure;
  char fasttrade, autocash, autojoin, autoid, usedefaultfont, colormenus, showcaste, reducesound, showdescript, quickshow, hidedesktop, manualbandage, showbleedmessage, shownextroundmessage;
  char auto256, iteminfo, autoweapswitch, nomusic, usenpc, castonfriends, allowfumble, allowunique;
  short defaultfont;
  long serial;
  Str255 name_str;
  char autonote, portraitchoice, currentscenariohold, blank3;
  short journalindex2, blank5, blank6, blank7, blank8, blank9, blank10;
} PrefRecord, *PrefPtr, **PrefHandle;

typedef struct {
  short currentscenario;
  long serial;
  Str255 codename;
} DivinePrefRecord, *DivinePrefPtr, **DivinePrefHandle;

struct restrictinfo {
  Str255 description;
  short maxpc, maxlevel;
  char canrace[30];
  char cancaste[30];
};

struct contactdata {
  Str255 scenarioname;
  Str255 version;
  Str255 date;
  Str255 authorsname;
  Str255 email;
  Str255 web;
  Str255 fee;
  Str255 payinfo[5];
  Str255 titles[5];
  Str255 description;
};

struct door {
  long doorid;
  char landid;
  char landx, landy;
  char percent;
  short code[8];
  short id[8];
};

struct race {
  short plusminustohit[8];
  short specialability[14];
  short drvbonus[8];
  short attbonus[6];
  short minmax[12];
  short spare[8];
  short conditions[40];
  short maxage, doesnotdie, basemove, magres, twohand, missile, numofattacks[2];
  char cancaste[30];
  short agerange[5][2];
  char agechange[5][15];
  char canregenerate;
  short defaulticonset;
  long itemtypes[2];
  short descriptors;
  short spacer[31];
};

struct item {
  short id;
  char equip, ident;
  short charge;
};

struct storage {
  struct item items[30];
};

struct mapstats {
  short sound, time, solid, shore, needboad, ispath, los, flyfloat, forest, spare;
  short build[3][3];
  short clearlandid;
};

struct caste {
  short specialability[2][14];
  short drvbonus[8];
  short attbonus[6];
  short spellcasters[4][3];
  short minmax[12];
  short conditions[40];
  short canusemissile, getsmissilebonus;
  short stamina[2], strength[2], dodge[2], tohit[2], missile[2], hand2hand[2], spare1[2], spare2[2];
  short casteclass, minimumagegroup, movebonus, magres, twohand, maxstaminabonus, bonusattacks, maxattacks;
  long victory[30];
  short startmoney;
  short startitems[20];
  char attacks[10];
  long itemtypes[2];
  short defaulticon, maxspellsattacks, spellssofar;
  short spacer[63];
};

struct todoque {
  short frommonid, traiter, monsterpostion;
  short doorid;
  long x, y;
};

struct Player {
  Point loc;
  char pillar[8];
  char hWall[15];
  char vWall[10];
};

struct battle {
  short battle[13][13];
  char dist;
  short messagebefore;
  short messageafter;
  short battlemacro;
};

struct timeencounter {
  short day, increment, percent, door;
  short reclevel, recrect, recx, recy, recitem, recquest;
  short stuff[10];
};

struct monster {
  unsigned char hd, bonus, dx, name, movementmax;
  char ac, magres, dist;
  char traiter, size, type[8], noofattacks, noofmagattacks, attacks[5][4];
  char damplus, castpercent, runpercent, surrenderpercent, misslepercent, cansum;
  char save[6], spellimmune[6];
  short money[3], spells[10], items[6], weapon, iconid, spellpoints, exp;

  short stamina, staminamax, underneath[2][2];
  char target, guarding, notonmenu, beenattacked, movement, magtohit, condition[40], lr, up, attacknum, bonusattack;
  short todoondeath, maxspellpoints;
  char monname[40];
};

struct note {
  Str255 string;
  long id;
  Boolean isdung;
  Boolean wasdark;
  short x, y, level;
};

struct maps {
  short icon[10][3];
  short startx, starty, level;
  short pictid;
  short iconsize;
  short show, isdungeon;
  short spare;
  short rect[4];
  Str255 note;
};

struct thief {
  Boolean type[10];
  char modifer[8], codes[8], codef[8];
  short texts[8], textf[8], sounds[8], soundf[8];
  short spell, lowdamage, highdamage, tumblers;
  short prompt[3], sound[3];
};

struct que {
  Rect bounds;
  short spellnum, icon;
  char x, y, size, duration, powerlevel, rotate, phase, castlevel;
};

struct randlevel {
  Rect randrect[20];
  short percent[20];
  short battlerange[20][2];
  short randdoor[20][3];
  short randdoorpercent[20][3];
  char landlook;
  Boolean isdark;
  Boolean uselos;
  Boolean only[20];
  char option[20];
  short sound[20];
  short text[20];
};

struct treasure {
  short itemid[20];
  short exp, gold, gems, jewelry;
};

struct encount2 {
  char code[4][8];
  short id[4][8];
  char choiceresult;
  char wordresult;
  char group[8];
  short spellid[10];
  char spellresult[10];
  short itemid[5];
  char itemresult[5];
  Boolean canbackout;
  Boolean thief;
  char maxtimes, castesuccess, thiefsuccess, thieffail;
  short prompt;
};

struct encount {
  char code[4][8];
  short id[4][8];
  char choiceresult[4];
  Boolean canbackout;
  char maxtimes;
  char castesuccess;
  short prompt;
};

struct scroll {
  char castcaste;
  char castlevel;
  char castnum;
  char powerlevel;
};

struct spell {
  char range1, range2, queicon, tohitbonus, savebonus, fixedtargetnum, canrotate, saveadjust, cannot, resistadjust, cost, damage1;
  char damage2, powerdam1, powerdam2, duration1, duration2;
  char powerdur1, powerdur2, spelllook1, spelllook2, sound1, sound2, targettype;
  char size;
  unsigned char special;
  char damagetype;
  unsigned char spellclass, incombat, incamp;
};

struct character {
  short version, verify1, tohit, dodge, missile, twohand, traiter, normattacks, beenattacked, guarding, target;
  short numitems, weaponsound, underneath, face, attackbonus, magco, position, maglu, magst, magres;
  short movebonus, ac, damage, race, caste, spellcastertype, gender, level, movement, movementmax, attacks, nspells[7];
  short stamina, staminamax, pictid, iconid, spellpoints, spellpointsmax, nohands, weaponnum, missilenum, handtohand;
  short condition[40], special[12], armor[20], spec[15], save[8], currentagegroup, verify2;
  struct item items[30];
  struct scroll scrollcase[5];
  long age, exp;
  unsigned short load, loadmax, money[3];
  Boolean hasturned, canheal, canidentify, candetect, toggle, bleeding, inbattle;
  char st, in, wi, de, co, lu;
  char cspells[7][12];
  char name[30];
  short verify3;
  long damagetaken, damagegiven, hitsgiven, hitstaken, imissed, umissed;
  long kills, deaths, knockouts, spellscast, destroyed, turns, prestigepenelty;
  short definespells[10][4], maxspellsattacks, spellsofar;
  char spare[96];
};

struct itemattr {
  short st, itemid, iconid, type, blunt, nohands, lu, movement, ac, magres, damage, spellpoints, sound;
  short wieght, cost, charge, iscurse, ismagical;
  long itemcat[2];
  short racerestrictions, casterestrictions, specificrace, specificcaste, raceclassonly, casteclassonly, spare2[7];
  short vssmall, vslarge, heat, cold, electric, vsundead, vsdd, vsevil, sp1, sp2, sp3, sp4, sp5, xcharge, drop;
};

struct shop {
  short id[1000];
  char num[1000];
  short inflation;
};

#endif /* STRUCTS_H */
