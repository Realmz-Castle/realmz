#ifndef VARIABLES_H
#define VARIABLES_H

#if defined(REALMZ_COCOA)
#include <time.h>
#endif

#include "prototypes.h"
#include "structs.h"
#include <stdint.h>
#include <stdio.h>

#define MyrBitSetShort(x, b) (*((uint16_t*)(x)) |= (1 << (15 - b)))
#define MyrBitClrShort(x, b) (*((uint16_t*)(x)) &= ~(1 << (15 - b)))
#define MyrBitTstShort(x, b) ((*((uint16_t*)(x)) & (1 << (15 - b))) != 0)
#define MyrBitSetLong(x, b) (*((uint32_t*)(x)) |= (1 << (31 - b)))
#define MyrBitClrLong(x, b) (*((uint32_t*)(x)) &= ~(1 << (31 - b)))
#define MyrBitTstLong(x, b) ((*((uint32_t*)(x)) & (1 << (31 - b))) != 0)

#if defined(REALMZ_CLASSIC)
#include "Folders.h"
#include "Palettes.h"
#include "Sound.h"
#include "Time.h"
#include "math64.h"
#endif

#include "fade.h"

#define ID_FONT_DEFAULT 1601

extern Boolean musicplaying;

#define fadeout 1
#define fadein 0
#define NIL 0L
#define maxmon 100
#define maxloop 110
#define maxloopminus 109
#define genevafont 10
#define STR_LIST_ID 6002
#define PREF_STR_INDEX 3
#define PREF_RES_ID 128

#define screensize 1

extern Rect AHEAD_RECT, BACK_RECT, TURN_LEFT_RECT, TURN_RIGHT_RECT;

extern short topfantasoftsceanrio; /*** v7.1 ****/
extern short customspellresnum; /*** v7.1 ****/
extern short stackcounter[20]; /*** v7.1 ****/
extern short stackindex; /*** v7.1 ****/
extern Boolean gosub, tagger;
extern short acceptlowrange1, acceptlowrange2, accepthighrange1, accepthighrange2, sMenuBarHeight;
extern RgnHandle sOriginalGrayRgn;

extern GWorldPtr gedge, gmaps, gbuff, gbuff2, gthePixels, gBackWorld, gFloorWorld, gVWallsWorld, gHWallsWorld, gPillarsWorld;
extern PixMapHandle edge, maps, buff, buff2, thePixels, gBackPix, gFloorPix, gVWallsPix, gHWallsPix, gPillarsPix;

extern SndChannelPtr cool[4];
extern struct restrictinfo restrictinfo;

extern short hm1, hm2, hm3, hm4;
extern short hp1, hp2, hp3, hp4;
extern short vm1, vm2, vm3, vm4;
extern short vp1, vp2, vp3, vp4;
extern short viewdung[15], vviewdung[10];
extern Rect WALL1[4], WALL2[4], WALL3[4], WALL4[4];
extern Rect WALLDEST[14], VERTDEST[10], VERTSOURCE[3][10];
extern Rect PILLAR_RECT_SOURCE[4], LEFT_PILLAR_RECT[4], RIGHT_PILLAR_RECT[4];
extern short globalmacro[30];

extern struct contactdata contactinfo;

extern struct door door[100], extenddoor, infodoor, stackdoor[20];

extern short list[1000];
extern short leftindex, downindex, bootyitemindex;
extern Boolean revivepartyflag, idtoggle, suspendallies;

extern Handle codeHandle;

extern struct race races;

extern struct caste caste;

extern struct item blank100;

extern struct storage storage[6];

extern short storageitems[180];
extern long wealthstore[3];
extern short tempfield[90][90];

extern struct mapstats mapstats[402], holdstatbase;

extern short cantuseflag, cantuseflag2, basetile[20];
extern short basescale[20];

extern short npcid, questid, itemnumid; /**** used for warp items ****/
extern short layout[8][16];
extern Boolean memoryspell, cleanage, solids[1024];
extern Str255 codename;
extern short downshift, leftshift;
extern char racebasemove[30];
extern short tempport, temptact;
extern Str255 codeseg1, codeseg2;
extern Boolean face, collidecheck[61];
extern Boolean usequickshow, collideflag;
extern short usecustomnames;
extern char lastdeltax, lastdeltay;
extern short musictoggle[20];
extern long musicpostionsave[20];
extern PixPatHandle gHilite, gNeutral, gShadow, base, light, dark, whitepat;
extern Boolean notes[3000];
extern CGrafPtr savedPort;
extern GDHandle savedDevice, curGDev, testdevice;
extern short bufftrash[100]; /**** used to keep the edgest from running over ****/
extern Boolean site[90][90];
extern short bufftrash2[100]; /**** used to keep the edgest from running over ****/

extern short definespells[6][10][4];
extern short musictoggle[20];
extern Boolean doauto[6], Stopmusic, fastspell, whichset;
extern TEHandle textHand;
extern short charunder[6], oldpowerlevel[6], currentplay, hitland;
extern Rect tierrect;
extern short quetodo, tier;

extern struct todoque todoque[100];

extern short todoqueindex, macromonster;
extern Str255 codeseg3, codeseg4;
extern Boolean spellarea[18][7][7];
extern short fumque[20], fumtotal, fumloop;
extern long appnum;
extern Boolean physical, battlemacro, doingque, macro, cycle, nologo, nofade;

extern char autonote, portraitchoice, currentscenariohold, blank3; /****** additional preference varialbles *******/
extern short journalindex2, blank5, blank6, blank7, blank8, blank9, blank10;

extern Handle data_handle;
extern short Appl_Rsrc_Fork_Ref_Num;
extern long serial, showserial;
extern Str255 Name_String;
extern MenuHandle musicmenu, gNPC, gManual;
extern Boolean okout, lowHD, cleanup;
extern OSErr bug;
extern WindowPtr gWindow;
extern Boolean needdungeonupdate, revertgame, nodispose, reRender, coward, canpriestturn, fullcolor;

extern struct Player gPlayer;

extern Boolean usescroll, innotes;
extern char viewtype;
extern char smallreply;
extern MenuHandle copy, prefer, gApple, gFile, gGame, gOptions, gSpelldelay, gSpeed, gSound, gParty, gBeast, popup, gScenario;
extern Rect buttons;
extern char currentDepth, undox, undoy;
extern short resetvolume;
extern long menuChoice;
extern short theMenu, theItem, nummon, numfoes, battlenum, adjdam;

extern short gDone, gStop, skip, middle, ischar, first, shop, skiptest, inspell, blank, lg, poss, regenerate;
extern short actiontaken[8];
extern short incombat, monsterturn, scribing, undead, didattack, nospell, infocombat, poisoned, canundo;

extern WindowPtr gshop, screen, bootywindow, mat, itemswindow, look;
extern DialogPtr about, flashwindow;
extern Str255 theString;
extern char filename[256], gotword[40], registrationname[40];
extern Str255 buffer[9];

extern ControlHandle melee, leftarrowr, rightarrowr, shopitemsvert, charitemsvert, theControl, weapons, armor, helms, supplies, magic;
extern ControlHandle leftarrow, rightarrow, rightcharacter, leftcharacter, combatitem, shopbut;
extern ControlHandle moneybut, turn, charmainbut, tradebut, overviewbut, swapbut, movelook, campbut, itemsbut, goitems;
extern ControlHandle departshop, poolshop, shareshop, castspellsbut, viewspellsbut, rest, barbut;
extern ControlHandle torch, showitems, search, attacks, condition, monsterbut, showitembut, showconditionbut;
extern ControlHandle autoone[6];
extern short cancamp, storeditems;
extern Boolean spellcasting, spellcharging, monstercasting, spareboolean;
extern char savedpostion[2], savedlandtype, savedlandlevel;
extern short sparesavedgamevariables[4];
extern DialogPtr gCurrent, gGeneration, dummy, background;
extern DialogPtr charstat, gswap, party;
extern DialogPtr scroll, spellwindow, info3, info2, templewindow;
extern Handle sndhandle[4];
extern CCrsrHandle sword, ask, stop, mouse, nokeys, notecursor;
extern PicHandle battlepict, grey, gen, showpict, on, non, off, marker, mainpict;
extern PixMapHandle thePixels, buff, buff2;
extern Boolean forcesmall;

extern short delayspeed, oldspeed, musicvolume, volume;
extern char defaultspell, showcast, usehashmarks, numchannel, lastgame, horseicon, dropitemprotection, forgettreasure;
extern char fasttrade, autocash, autojoin, autoid, usedefaultfont, colormenus, showcaste, reducesound, showdescript, quickshow, hidedesktop, manualbandage, showbleedmessage, shownextroundmessage;
extern char auto256, iteminfo, autoweapswitch, nomusic, usenpc, castonfriends, allowfumble, allowunique;
extern short defaultfont;

extern CIconHandle iconhand, showmagic;

extern Boolean errorlevel, sorted, seemless, gotegg;
extern short width, depth, GlobalTop, GlobalLeft;
extern short menupos[251], oldvolume;

extern struct battle battle;

extern short count;
extern Boolean seenit, showreg;
extern FInfo fileinfo;
extern Rect bigspellrect;

extern struct timeencounter dotime;

extern struct monster monster[100], monpick, holdover[20], blankmonster;

extern int monsternameid[100], tempmonsternameid; //*** used so critters over 256 route correctly and can be use in "if NPC present" checks.

extern float percent, hardpercent;
extern char deltax, deltay, scenarioname[256];
extern char killparty, charnum, head, currentshop, quest[128], commandkey, cl, cr, charselectnew, map[20];
extern Boolean inscroll, indung, view, editon, incamp, initems, inswap, inbooty, shopavail, campavail;
extern Boolean intemple, inshop, swapavail, templeavail, tradeavail, canshop, shopequip, lastcaste;
extern char lastspell[6][2], combatround, lastpix;
extern short currentscenario, howhard, fat, partycondition[10];
extern long x, y, wallx, wally, dunglevel, partyx, partyy, reclevel, maxlevel, landlevel, lookx, looky, fieldx, fieldy, floorx, floory;
extern long moneypool[3];
extern struct tm tyme;
extern struct tm magic_tyme;
extern char multiview, updatedir;
extern char monsterset, bankavailable;
extern long bank[3];
extern short templecost;
extern char inboat, boatright, canencounter;
extern char xydisplayflag;
extern char blanksavegamevariables[14]; //**** only 18 are left,  used two for templecost move.
extern struct monster holdover[20];
extern short heldover, deduction;
extern short duration;
extern Boolean displaytag, hide;
extern short blankround;
extern long mouseuptime;
extern Handle myMenuBar, copywright;
extern long templong;
extern short select[6];
extern Boolean needupdate, shortupdateneed, putup;
extern short mapshiftx, mapshifty, divineref, itemrefnum, refnum, jewelsrefnum, portraitrefnum;
extern short tacticalrefnum;

extern struct note note;

extern long oldtime;
extern Rect tiny[24];

extern struct maps themap;

extern Boolean showzero;
extern short laststring, doornum, extradoornum, extraid, soundnum, stringindex;

extern struct thief thief;

extern Rect buttonrect;
extern short trace[3][maxloop];
extern short specdamage, numitems;
extern short flamestage, enctry;
extern long oldflametick;
extern short loop;
extern short font;

extern struct que que[60];

extern struct randlevel randlevel;

extern short extracode[5], holdcode[5]; // Fantasoft v7.1   Added holdcode[5]
extern short messageafter, key, scanCode, lastshown;
extern short rotate, attackfriend;
extern struct treasure treasure;

extern short oldup, channel;
extern RGBColor greencolor, greycolor, cyancolor, goldcolor, lightgrey;
extern Rect sizestore, fieldstore;
extern short oldspellx, oldspelly;
extern long totalexp;

extern struct encount2 enc2;

extern struct encount enc;

extern struct scroll blankscroll;

extern double currange;
extern FILE *fp, *op;
extern short t, tt, ttt;
extern short itemType, itemHit;
extern short tempid, temp, damage;
extern Rect itemRect;
extern char behind;
extern short enemy[12];
extern Handle itemHandle;
extern char track[maxloop], spellrange, numoftar, targetnum, aimindex, numenemy;
extern char castcaste, castlevel, castnum, powerlevel;
extern char size, charselectold;
extern char charup, attackloop, range[maxloop + 2];
extern Rect spell, spellrect, spellrect2, infosmall, showspell, bodyrect;
extern char target[7][2], killmon;

extern struct spell spellinfo, spelldata[5][7][15];

extern double dist;
extern char monpos[100][2], pos[6][2];
extern short curControlValue, minControlValue, maxControlValue;
extern char q[maxloop], up, monsterup, encountflag;
extern short itemused, incr;
extern char spellx, spelly;
extern short field[90][90];
extern char tagnew, tagold;
extern short delta, thePart, shopselection;
extern char itemselectnew;
extern Rect charselectrect, box3, lookrect, box, info, textrect, greybox;
extern long tempvalue;
extern short value, gshopitem, selection, whichchar;
extern short theCode, tempindex, tempselection, reply, pick;
extern Point point;
extern Rect icon, chr, selectrect;
extern EventRecord gTheEvent;

extern struct character characterl, characterr, c[6];

extern struct itemattr item, allweapons[200], allarmor[200], allhelms[200], allmagic[200], allsupply[200];

extern long doorid;

extern struct shop theshop;

extern ControlHandle goshopinshop;
extern Str255 myString;

extern QuickDrawGlobals qd;

#endif // VARIABLES_H
