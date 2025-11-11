//      Realmz� is Copyright � 1994-99, by Tim Phillips
#if defined(REALMZ_CLASSIC)
#include "Folders.h" // holds the definition for FindFolder()
#include "Palettes.h"
#include "Sound.h"
#include "Time.h"
#include <Gestalt.h>
#include <Video.h>
#elif defined(REALMZ_COCOA)
#include "RealmzCocoa.h"
#include <SDL3/SDL_main.h>
#endif

#ifdef __MWERKS__
#include "math64.h"
#endif

#include "realmzbuild.h"
#include "stdio.h"
// #include "RDriver.h"
// #include "MAD.h"
#include "prototypes.h"
#include "structs.h"
#include "variables.h"

short specailabs(short i);

#define VAL_MAGIC 0x15621562L
#define fadeout 1
#define fadein 0
#define NIL 0L
#define maxmon 100
#define maxloop 110
#define maxloopminus 109
#define genevafont 10
#define MYRMAGIC 0x15621562L

#define screensize 1

#if MYR_CHECK > 0
unsigned long myrmagictab[10L * 1024L] = {0};
#endif

short topfantasoftsceanrio = 23; /*** v7.1 ****/
short customspellresnum; /*** v7.1 ****/
short stackcounter[20]; /*** v7.1 ****/
short stackindex; /*** v7.1 ****/
Boolean gosub, tagger = 0;

#ifndef PC
typedef struct OpaqueGammaInfo GammaInfo, *GammaInfoPtr, **GammaInfoHdl;
struct OpaqueGammaInfo {
  GammaInfoHdl next;

  GDHandle screenDevice;
  GammaTblHandle original;
  GammaTblHandle unmodified;
  GammaTblHandle hacked;
  UInt16 size;
  Boolean canDoColor;
};

GammaInfoHdl initialState;

Boolean fadestate;
// MADDriverSettings init;
// MADDriverRec *MADDriver;
// MADMusic *musicfile;
#endif
Boolean musicplaying = FALSE;
// SFReply SFReplyRecord;

long musicpostionsave[20];
CGrafPtr savedPort;
GDHandle savedDevice, curGDev, testdevice;
GWorldPtr gedge, gmaps, gbuff, gbuff2, gthePixels, gBackWorld, gFloorWorld, gVWallsWorld, gHWallsWorld, gPillarsWorld;
PixMapHandle edge, maps, buff, buff2, thePixels, gBackPix, gFloorPix, gVWallsPix, gHWallsPix, gPillarsPix;

/*
struct restrictinfo
{
Str255 description;
short maxpc,maxlevel;
char canrace[30];
char cancaste[30];
}
*/

struct restrictinfo restrictinfo;

struct tm tyme;

GDHandle testdevice;
GDHandle magic4;

short acceptlowrange1, acceptlowrange2, accepthighrange1, accepthighrange2, sMenuBarHeight;

RgnHandle sOriginalGrayRgn;

short hm1, hm2, hm3, hm4;

short hp1, hp2, hp3, hp4;

short vm1, vm2, vm3, vm4;

short vp1, vp2, vp3, vp4;

short viewdung[15], vviewdung[10];

Rect WALL1[4], WALL2[4], WALL3[4], WALL4[4];

Rect WALLDEST[14], VERTDEST[10], VERTSOURCE[3][10];

Rect PILLAR_RECT_SOURCE[4], LEFT_PILLAR_RECT[4], RIGHT_PILLAR_RECT[4];

Rect AHEAD_RECT, BACK_RECT, TURN_LEFT_RECT, TURN_RIGHT_RECT;

short globalmacro[30];

char charfilename[256];

/*
struct contactdata
{
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
}
*/
struct contactdata contactinfo;

short leftindex, downindex, bootyitemindex;
Boolean revivepartyflag, idtoggle, musicloaded, suspendallies;

Handle codeHandle;

/*
struct race
{
short plusminustohit[8];
short specialability[14];
short drvbonus[8];
short attbonus[6];
short minmax[12];
short spare[8];
short conditions[40];
short maxage,doesnotdie,basemove,magres,twohand,missile,numofattacks[2];
char cancaste[30];
short agerange[5][2];
char agechange[5][15];
char canregenerate;
short defaulticonset;
long itemtypes[2];
short descriptors;
short spacer[31];
}
*/

struct race races = {0};

long myrmagic_race = MYRMAGIC;
short regscen_pc_custom(void);

/*
struct caste
{
short specialability[2][14];
short drvbonus[8];
short attbonus[6];
short spellcasters[4][3];
short minmax[12];
short conditions[40];
short canusemissile,getsmissilebonus;
short stamina[2],strength[2],dodge[2],tohit[2],missile[2],hand2hand[2],spare1[2],spare2[2];
short casteclass,minimumagegroup,movebonus,magres,twohand,maxstaminabonus,bonusattacks,maxattacks;
long victory[30];
short startmoney;
short startitems[20];
char attacks[10];
long itemtypes[2];
short defaulticon,maxspellsattacks,spellssofar;
short spacer[63];
}
*/

struct caste caste = {0};

long myrmagic_caste = MYRMAGIC;

/*
struct item
{
short id;
char equip,ident;
short charge;
}
*/

struct item blank100 = {0};

long myrmagic_blank100 = MYRMAGIC;

/*
struct mapstats
{
short sound,time,solid,shore,needboad,ispath,los,flyfloat,forest,spare;
short build[3][3];
short clearlandid;
}
*/

struct mapstats mapstats[402] = {0}, holdstatbase;

long myrmagic_mapstats = MYRMAGIC;

short cantuseflag, cantuseflag2, basetile[20] = {0};
long myrmagic_basetile = MYRMAGIC;

short basescale[20] = {0};
long myrmagic_basescale = MYRMAGIC;

/*
typedef  struct
{
short currentscenario;
long serial;
Str255 codename;
} DivinePrefRecord, *DivinePrefPtr, **DivinePrefHandle;
*/

short npcid, questid, itemnumid; /**** used for warp items ****/
short layout[8][16] = {0};
long myrmagic_layout = MYRMAGIC;

Boolean memoryspell, cleanage, solids[1024] = {0};
long myrmagicSolids = MYRMAGIC;

Str255 codename = {0};
long myrmagic_codename = MYRMAGIC;
char racebasemove[30] = {0};
long myrmagic_racebasemove = MYRMAGIC;
short tempport = -1;
short temptact = -1;
Str255 codeseg1;
long myrmagic_codeseg1 = MYRMAGIC;
Str255 codeseg2;
long myrmagic_codeseg2 = MYRMAGIC;
Boolean face, collidecheck[61] = {0};
long myrmagic_collidecheck = MYRMAGIC;
Boolean usequickshow, collideflag;
short usecustomnames = FALSE;
char lastdeltax, lastdeltay;
PixPatHandle gHilite, gNeutral, gShadow, base, light, dark, whitepat;
Boolean notes[3000] = {0};
long myrmagic_notes = MYRMAGIC;
short bufftrash[100] = {0}; /**** used to keep the edgest from running over ****/
long myrmagic_bufftrash = MYRMAGIC;
Boolean site[90][90] = {0};
long myrmagic_site = MYRMAGIC;

short bufftrash2[100] = {0}; /**** used to keep the edgest from running over ****/
long myrmagic_bufftrash2 = MYRMAGIC;

short definespells[6][10][4] = {0};
long myrmagic_definespells = MYRMAGIC;

short musictoggle[20] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
Boolean doauto[6], Stopmusic, fastspell, whichset;
TEHandle textHand;
short charunder[6], oldpowerlevel[6], currentplay = -1;
Rect tierrect;
short quetodo, tier, hitland;

/*
struct todoque
{
short frommonid,traiter,monsterpostion;
short doorid;
long x,y;
}
*/
struct todoque todoque[100] = {0};
long myrmagic_todoque = MYRMAGIC;

short todoqueindex, macromonster;
Str255 codeseg3, codeseg4;
Boolean spellarea[18][7][7] = {0};
long myrmagic_spellarea = MYRMAGIC;
short fumque[20] = {0}, fumtotal, fumloop;
long myrmagic_fumque = MYRMAGIC;
long appnum = 0;
Boolean physical, battlemacro, doingque, macro, cycle, nologo, nofade;

/*
typedef  struct
{
short delayspeed,oldspeed,oldvolume,volume;
char defaultspell,showcast,usehashmarks,numchannel,lastgame,horseicon,dropitemprotection,forgettreasure;
char fasttrade,autocash,autojoin,autoid,usedefaultfont,colormenus,showcaste,reducesound,showdescript,quickshow,hidedesktop,manualbandage,showbleedmessage,shownextroundmessage;
char auto256,iteminfo,autoweapswitch,nomusic,usenpc,castonfriends,allowfumble,allowunique;
short defaultfont;
long serial;
Str255  name_str;
char autonote,portraitchoice,currentscenariohold,blank3;
short journalindex2,blank5,blank6,blank7,blank8,blank9,blank10;
} PrefRecord, *PrefPtr, **PrefHandle;
*/

char autonote, portraitchoice, currentscenariohold, blank3; /****** additional preference varialbles *******/
short journalindex2, blank5, blank6, blank7, blank8, blank9, blank10;
Handle data_handle;
short Appl_Rsrc_Fork_Ref_Num;
long serial, showserial;
Str255 Name_String;
MenuHandle musicmenu, gNPC, gManual;
Boolean okout, lowHD, cleanup;
OSErr bug;
WindowPtr gWindow;
Boolean needdungeonupdate, revertgame, nodispose;
Boolean reRender, coward, canpriestturn = TRUE;
Boolean fullcolor = TRUE;

/*
struct Player
{
Point loc;
char pillar[8];
char hWall[15];
char vWall[10];
}
*/

struct Player gPlayer = {0};
long myrmagic_gPlayer = MYRMAGIC;
Boolean usescroll, innotes;
char viewtype = 1;
char smallreply;
MenuHandle copy, prefer, gApple, gFile, gGame, gOptions, gSpelldelay, gSpeed, gSound, gParty, gBeast, popup, gScenario;
Rect buttons;
char currentDepth, undox, undoy;
short resetvolume;
long menuChoice;
short theMenu, theItem, nummon, numfoes, battlenum, adjdam;
short gDone, gStop, skip, middle, ischar, first, shop, skiptest, inspell, blank, lg, poss, regenerate;
short actiontaken[8];
short incombat, monsterturn, scribing, undead, didattack, nospell, infocombat, poisoned, canundo;
WindowPtr gshop, screen, bootywindow, mat, itemswindow, look;
Str255 theString;
char filename[256], gotword[40], registrationname[40];
Str255 buffer[9];
ControlHandle melee, leftarrowr, rightarrowr, shopitemsvert, charitemsvert, theControl, weapons, armor, helms, supplies, magic;
ControlHandle leftarrow, rightarrow, rightcharacter, leftcharacter, combatitem, shopbut;
ControlHandle moneybut, turn, charmainbut, tradebut, overviewbut, swapbut, movelook, campbut, itemsbut, goitems;
ControlHandle departshop, poolshop, shareshop, castspellsbut, viewspellsbut, rest, barbut;
ControlHandle torch, showitems, search, attacks, condition, monsterbut, showitembut, showconditionbut;
ControlHandle autoone[6];

char savedpostion[2], savedlandtype, savedlandlevel;
short sparesavedgamevariables[4];
DialogRef about, flashwindow, gCurrent, gGeneration, dummy, background = NIL;
DialogRef charstat, gswap, party, scroll, spellwindow, info3, info2, templewindow;
Handle sndhandle[4];
CCrsrHandle sword, ask, stop, mouse, nokeys, notecursor;
PicHandle battlepict, grey, gen, showpict, on, non, off, marker, mainpict;
Boolean forcesmall;
short delayspeed, oldspeed, musicvolume, volume;
char defaultspell, showcast, usehashmarks, numchannel, lastgame, horseicon, dropitemprotection, forgettreasure;
char fasttrade, autocash, autojoin, autoid, usedefaultfont, colormenus, showcaste, reducesound, showdescript, quickshow, hidedesktop, manualbandage, showbleedmessage, shownextroundmessage;
char auto256, iteminfo, autoweapswitch, nomusic, usenpc, castonfriends, allowfumble, allowunique;
short defaultfont;

CIconHandle iconhand, showmagic;

Boolean errorlevel, sorted, seemless, gotegg;
short width, depth, GlobalTop, GlobalLeft;
short menupos[251], oldvolume;

/*
struct battle
{
short battle [13][13];
char dist;
short messagebefore;
short messageafter;
short battlemacro;
}
*/
struct battle battle = {0};
long myrmagic_battle = MYRMAGIC;

short count, tempfield[90][90] = {0};
long myrmagic_tempfield = MYRMAGIC;

Boolean seenit, showreg;
FInfo fileinfo;
Rect bigspellrect;

/*
struct monster
{
unsigned char hd,bonus,dx,name,movementmax;
char ac,magres,dist;
char traiter,size,type[8],noofattacks,noofmagattacks,attacks[5][4];
char damplus,castpercent,runpercent,surrenderpercent,misslepercent,cansum;
char save[6],spellimmune[6];
short  money[3],spells[10],items[6],weapon,iconid,spellpoints,exp;
short  stamina,staminamax,underneath[2][2];
char target,guarding,notonmenu,beenattacked,movement,magtohit,condition[40],lr,up,attacknum,bonusattack;
short  todoondeath,maxspellpoints;
char monname[40];
}
*/
struct monster monster[100] = {0};
long myrmagic_monster = MYRMAGIC;

struct monster monpick;

struct monster holdover[20] = {0};
long myrmagic_holdover = MYRMAGIC;

struct monster blankmonster;

float percent, hardpercent;

/********* do not relocate block start *************************************************/
char deltax, deltay, scenarioname[256];
char killparty, charnum, head, currentshop, quest[128], commandkey, cl, cr, charselectnew, map[20];
Boolean inscroll, indung, view, editon, incamp, initems, inswap, inbooty, shopavail, campavail;
Boolean intemple, inshop, swapavail, templeavail, tradeavail, canshop, shopequip, lastcaste;
char lastspell[6][2], combatround, lastpix = -1;
/***************************************************************************************/
/***************************************************************************************/

/*
struct timeencounter
{
short day,increment,percent,door;
short reclevel,recrect,recx,recy,recitem,recquest;
short stuff[10];
}
*/
struct timeencounter dotime = {0};
long myrmagic_dotime = MYRMAGIC;

/*
struct storage
{
struct item items[30];
}
*/
struct storage storage[6] = {0};
long myrmagic_storage = MYRMAGIC;

/********* do not relocate block start *************************************************/
short currentscenario, howhard, fat, partycondition[10];
long x, y, wallx, wally, dunglevel, partyx, partyy, reclevel, maxlevel, landlevel, lookx, looky, fieldx, fieldy, floorx, floory;
long moneypool[3] = {0};
long myrmagic_moneypool = MYRMAGIC;

struct tm tyme;
char multiview, updatedir = 1;
char monsterset, bankavailable;
long bank[3];
short templecost;
char inboat, boatright, canencounter = TRUE;
char xydisplayflag = 0;
char blanksavegamevariables[14]; //**** only 18 are left,  used two for templecost move.

short cancamp, storeditems;
Boolean spellcasting, spellcharging, monstercasting, spareboolean;

short storageitems[180] = {0};
long myrmagic_storageitems = MYRMAGIC;
long wealthstore[3] = {0};
long myrmagic_wealthstore = MYRMAGIC;
/***************************************************************************************/
/***************************************************************************************/

short heldover, deduction, duration;
Boolean displaytag, hide;
short blankround, select[6];
long mouseuptime, templong;
Handle myMenuBar, copywright;
Boolean needupdate, shortupdateneed, putup;
short mapshiftx, mapshifty, divineref, itemrefnum, refnum, jewelsrefnum, portraitrefnum = -1;
short tacticalrefnum = -1;

/*
struct note
{
Str255 string;
long id;
Boolean isdung;
Boolean wasdark;
short x,y,level;
}
*/
struct note note;

long oldtime;
Rect tiny[24] = {0};
long myrmagic_tiny = MYRMAGIC;

/*
struct maps
{
short icon[10][3];
short startx,starty,level;
short pictid;
short iconsize;
short show,isdungeon;
short spare;
short rect[4];
Str255 note;
}
*/
struct maps themap = {0};
long myrmagic_themap = MYRMAGIC;

GDHandle curGDev;
Boolean showzero;
short laststring, doornum, extradoornum, extraid, soundnum, stringindex;

/*
struct thief
{
Boolean type[10];
char modifer[8],codes[8],codef[8];
short texts[8],textf[8],sounds[8],soundf[8];
short spell,lowdamage,highdamage,tumblers;
short prompt[3],sound[3];
}
*/
struct thief thief = {0};
long myrmagic_thief = MYRMAGIC;

Rect buttonrect;
short trace[3][maxloop];
short specdamage, numitems;
short flamestage, enctry;
long oldflametick;
short loop, font = 1602;

/*
struct que
{
Rect bounds;
short spellnum,icon;
char x,y,size,duration,powerlevel,rotate,phase,castlevel;
}
*/
struct que que[60] = {0};
long myrmagic_que = MYRMAGIC;

/*
struct randlevel
{
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
}
*/
struct randlevel randlevel = {0};
long myrmagic_randlevel = MYRMAGIC;

short extracode[5], holdcode[5]; // Fantasoft v7.1   Added holdcode[5]
short messageafter, key, scanCode, lastshown;
short rotate, attackfriend;

/*
struct treasure
{
short itemid[20];
short exp,gold,gems,jewelry;
}
*/
struct treasure treasure = {0};
long myrmagic_treasure = MYRMAGIC;

short oldup, channel = -1;
RGBColor greencolor, greycolor, cyancolor, goldcolor, lightgrey;
Rect sizestore, fieldstore;

short oldspellx, oldspelly;
long totalexp;

/*
struct encount2
{
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
char maxtimes,castesuccess,thiefsuccess,thieffail;
short prompt;
}
*/
struct encount2 enc2 = {0};
long myrmagic_enc2 = MYRMAGIC;

/*
struct encount
{
char code[4][8];
short id[4][8];
char choiceresult[4];
Boolean canbackout;
char maxtimes;
char castesuccess;
short prompt;
}
*/
struct encount enc = {0};
long myrmagic_enc = MYRMAGIC;

/*
struct scroll
{
char castcaste;
char castlevel;
char castnum;
char powerlevel;
}
*/
struct scroll blankscroll;

double currange;
FILE *fp, *op;
short t, tt, ttt;
short itemType, itemHit;
short tempid, temp, damage;
Rect itemRect;
char behind;
short enemy[12];
Handle itemHandle;
char track[maxloop], spellrange, numoftar, targetnum, aimindex, numenemy;
char castcaste, castlevel, castnum, powerlevel, size, charselectold, charup, attackloop, range[maxloop + 2];
SndChannelPtr cool[4];
short sampledSynth = 5;
long initMono = 0x0080;
long initNoDrop = 0x0008;
Rect spell, spellrect, spellrect2, infosmall, showspell, bodyrect;
char target[7][2], killmon;

/*
struct spell
{
char range1,range2,queicon,tohitbonus,savebonus,fixedtargetnum,canrotate,saveadjust,cannot,resistadjust,cost,damage1;
char damage2,powerdam1,powerdam2,duration1,duration2;
char powerdur1,powerdur2,spelllook1,spelllook2,sound1,sound2,targettype;
char size;
unsigned char special;
char damagetype;
unsigned char spellclass,incombat,incamp;
}
*/
struct spell spellinfo = {0};
long myrmagic_spellinfo = MYRMAGIC;

struct spell spelldata[5][7][15] = {0};
int monsternameid[100], tempmonsternameid; //*** used so critters over 256 route correctly and can be use in "if NPC present" checks.

long myrmagic_spelldata = MYRMAGIC;

double dist;
char monpos[100][2] = {0};
long myrmagic_monpos = MYRMAGIC;
char pos[6][2] = {0};
long myrmagic_pos = MYRMAGIC;
short curControlValue, minControlValue, maxControlValue;
char q[maxloop], up, monsterup, encountflag;
short itemused, incr;
char spellx, spelly;
short field[90][90] = {0};
long myrmagic_field = MYRMAGIC;
char tagnew, tagold;
short delta, thePart, shopselection;
char itemselectnew;
Rect charselectrect, box3, lookrect, box, info, textrect, greybox;
long tempvalue;
short value, gshopitem, selection, whichchar, theCode, tempindex, tempselection, reply, pick;
Point point;
Rect icon, chr, selectrect;
EventRecord gTheEvent;

/*
struct character
{
short version,verify1,tohit,dodge,missile,twohand,traiter,normattacks,beenattacked,guarding,target;
short numitems,weaponsound,underneath,face,attackbonus,magco,position,maglu,magst,magres;
short movebonus,ac,damage,race,caste,spellcastertype,gender,level,movement,movementmax,attacks,nspells[7];
short stamina,staminamax,pictid,iconid,spellpoints,spellpointsmax,nohands,weaponnum,missilenum,handtohand;
short condition[40],special[12],armor[20],spec[15],save[8],currentagegroup,verify2;
struct item items[30];
struct scroll scrollcase[5];
long age,exp;
unsigned short load,loadmax,money[3];
Boolean hasturned,canheal,canidentify,candetect,toggle,bleeding,inbattle;
char st,in,wi,de,co,lu;
char cspells[7][12];
char name[30];
short verify3;
long damagetaken,damagegiven,hitsgiven,hitstaken,imissed,umissed;
long kills,deaths,knockouts,spellscast,destroyed,turns,prestigepenelty;
short definespells[10][4],maxspellsattacks,spellsofar;
char spare[96];
}
*/
struct character characterl = {0};

long myrmagic_character1 = MYRMAGIC;

struct character characterr = {0};
long myrmagic_characterr = MYRMAGIC;

struct character c[6] = {0};
long myrmagic_c = MYRMAGIC;

/*
struct itemattr
{
short st,itemid,iconid,type,blunt,nohands,lu,movement,ac,magres,damage,spellpoints,sound;
short wieght,cost,charge,iscurse,ismagical;
long itemcat[2];
short racerestrictions,casterestrictions,specificrace,specificcaste,raceclassonly,casteclassonly,spare2[7];
short vssmall,vslarge,heat,cold,electric,vsundead,vsdd,vsevil,sp1,sp2,sp3,sp4,sp5,xcharge,drop;
}
*/
struct itemattr item = {0};
long myrmagic_item = MYRMAGIC;

struct itemattr allweapons[200] = {0};
long myrmagic_allweapons = MYRMAGIC;

struct itemattr allarmor[200] = {0};
long myrmagic_allarmor = MYRMAGIC;

struct itemattr allhelms[200] = {0};
long myrmagic_allhelms = MYRMAGIC;

struct itemattr allmagic[200] = {0};
long myrmagic_allmagic = MYRMAGIC;

struct itemattr allsupply[200] = {0};
long myrmagic_allsupply = MYRMAGIC;

long doorid;

/*
struct shop
{
short id[1000];
char num[1000];
short inflation;
}
*/
struct shop theshop = {0};
long myrmagic_theshop = MYRMAGIC;

/*
struct door
{
long doorid;
char landid;
char landx,landy;
char percent;
short code[8];
short id[8];
}
*/
struct door door[100] = {0};
long myrmagic_door = MYRMAGIC;

struct door extenddoor = {0};
long myrmagic_extenddoor = MYRMAGIC;
struct door infodoor = {0};
long myrmagic_infodoor = MYRMAGIC;
struct door stackdoor[20] = {0};
long myrmagic_stackdoor = MYRMAGIC;

short list[1000] = {0};
long myrmagic_list = MYRMAGIC;
short downshift, leftshift;
ControlHandle goshopinshop;
Str255 myString = {0};
long myrmagic_myString = MYRMAGIC;

/***************************************************************************************
                                                        MyrInitCheckMemory
Init the memory tracker
****************************************************************************************/
void MyrInitCheckMemory(void) {
#if MYR_CHECK > 0
  long i;

  for (i = 0; i < sizeof(myrmagictab) / sizeof(long); i++)
    myrmagictab[i] = 'AAAA';

#endif
}

/***************************************************************************************
                                                        MyrCheckMemory
Check if any copy, or array allocation reached a marker
****************************************************************************************/
void MyrCheckMemory(short mode) {
#if MYR_CHECK > 0
  register long i;
  char chaine[255];
  CGrafPtr g;

  if (mode > 1) {
    i = sizeof(myrmagictab) / sizeof(long);
    if (memcmp(&myrmagictab[0], &myrmagictab[i / 2], sizeof(myrmagictab) / 2) != 0)
      AcamErreur("Corrupted tab header  ");
  }
  if (myrmagic_race != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_caste != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_blank100 != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_mapstats != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_basetile != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_basescale != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_layout != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagicSolids != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_codename != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_racebasemove != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_codeseg1 != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_codeseg2 != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_collidecheck != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_notes != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_site != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_bufftrash2 != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_definespells != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_todoque != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_spellarea != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_fumque != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_gPlayer != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_battle != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_tempfield != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_monster != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_holdover != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_themap != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_tiny != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_wealthstore != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_thief != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_storage != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_storageitems != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_dotime != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_moneypool != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_que != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_randlevel != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_treasure != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_enc2 != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_enc != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_spellinfo != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_spelldata != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_monpos != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_pos != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_field != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_character1 != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_characterr != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_c != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_item != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_allweapons != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_allarmor != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_allhelms != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_allmagic != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_allsupply != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_theshop != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_door != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_extenddoor != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_infodoor != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_infodoor != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_stackdoor != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_list != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (myrmagic_myString != MYRMAGIC)
    AcamErreur("Data corrupted");
  if (mode > 0) {
    g = AcamCreatePortOffscreen(16, 16, 8);
    if (g == NULL)
      AcamErreur("MyrMagic:Can't create port");
    else
      AcamClosePortOffscreen(g);
    DrawMenuBar();
  }
#endif
}

/************************ SetSoundVol *******************/
pascal void _SetSoundVol(short volume) {
}

/************************ GetSoundVol *******************/
void _GetSoundVol(short* val) {
  *val = 0;
}

/************************ stringplus ********************/
void stringplus(int number) {
  NumToString(number, myString);
  if (number > 0)
    DrawString("\p+");
  DrawString(myString);
}

/************************ stringnozeronoplus ********************/
void stringnozeronoplus(int number) {
  NumToString(number, myString);
  if (number > 0)
    DrawString(myString);
}

/************************ stringnozero ********************/
void stringnozero(int number) {
  NumToString(number, myString);
  if (number > 0) {
    DrawString("\p+");
    DrawString(myString);
  }
}

/****************************** centercursor **********************/
void centercursor(void) {
  Point newMousePosition;

  newMousePosition.v = newMousePosition.h = 176;

  LocalToGlobal(&newMousePosition);
/*!Myriad 27/08/99 LMGet/Set calls*/
#ifdef PC
  LMSetMouseTemp(&newMousePosition);
  LMSetRawMouseLocation(&newMousePosition);
  LMSetCursorNew(0xFFFF);
#else
  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(fuzziqersoftware): This really is how you would change the mouse
   * location in Classic Mac OS; see LowMemoryGlobals.hh in resource_dasm for
   * what these addresses represent. We can't do this on modern systems, of
   * course, so we use SDL instead. */
  // *(Point*)0x82c = newMousePosition;
  // *(Point*)0x828 = newMousePosition;
  // *(short*)0x8ce = 0xffff;
  SetMouseLocation(&newMousePosition);
  /* *** END CHANGES *** */
#endif
}

/****************************** ToolBoxInit **********************/
void ToolBoxInit(void) {
  InitGraf(&qd.thePort);
  InitWindows();
#ifdef REALMZ_CLASSIC
  GetDateTime((unsigned long*)&qd.randSeed);
  InitFonts();
  TEInit();
  InitDialogs(NIL);
  MaxApplZone();
  InitCursor();
// MyrInitCheckMemory();
#elif defined(REALMZ_CARBON)
  // Set our working directory to the app bundle's directory.
  CFURLRef bundle_url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  CFURLRef wd_url = CFURLCreateCopyDeletingLastPathComponent(NULL, bundle_url);
  CFRelease(bundle_url);
  char wd[2048];
  CFURLGetFileSystemRepresentation(wd_url, true, (uint8_t*)wd, sizeof(wd));
  CFRelease(wd_url);
  chdir(wd);
#elif defined(REALMZ_COCOA)
  InitRealmzCocoa();
#endif

  defaultfont = 1601;
  Appl_Rsrc_Fork_Ref_Num = CurResFile();

  // Start by assuming we're doing both.
  nofade = nologo = FALSE;

#ifndef PC
  if (!IsColorGammaAvailable()) {
    nofade = nologo = TRUE;
  }
#endif

  GetVersStr(1, 0); /*** load in version of Realmz right away ***/

  jewelsrefnum = MyrOpenResFile((Ptr) "\p:Data Files:The Family Jewels");
  temp = MyrOpenResFile((Ptr) "\p:Data Files:Custom Dungeon Graphics");

  GetVersStr(9, jewelsrefnum); /*** load in version of family jewels right away ***/

  if (divine)
    temp = MyrOpenResFile((Ptr) "\p:Divine Right Module");
  if ((development) || (divine))
    divineref = MyrOpenResFile((Ptr) "\p:Divine Development X-tras");

  temp = MyrOpenResFile((Ptr) "\p:Data Files:Realmz Update Gem 1"); //***** fantasoft v7.1  This allows for resource updates without needing the wholde damn Bag of Holding file
  temp = MyrOpenResFile((Ptr) "\p:Data Files:Realmz Update Gem 2"); //***** fantasoft v7.1
  temp = MyrOpenResFile((Ptr) "\p:Data Files:Realmz Update Gem 3"); //***** fantasoft v7.1
  temp = MyrOpenResFile((Ptr) "\p:Data Files:Realmz Update Gem 4"); //***** fantasoft v7.1
  temp = MyrOpenResFile((Ptr) "\p:Data Files:Realmz Update Gem 5"); //***** fantasoft v7.1
  temp = MyrOpenResFile((Ptr) "\p:Data Files:Realmz Update Gem 6"); //***** fantasoft v7.1
  temp = MyrOpenResFile((Ptr) "\p:Data Files:Realmz Update Gem 7"); //***** fantasoft v7.1

  base = GetPixPat(131);
  light = GetPixPat(132);
  dark = GetPixPat(133);
  whitepat = GetPixPat(134);

  testdevice = GetMainDevice();

  width = (*testdevice)->gdRect.right - (*testdevice)->gdRect.left; /**** get size of screen for display offsets ***/
  depth = (*testdevice)->gdRect.bottom - (*testdevice)->gdRect.top;

  GlobalTop = 20 + (depth - 600) / 2;
  GlobalLeft = (width - 800) / 2;

  numchannel = -1;

  for (t = 0; t < 4; t++) /*************** setup sound channels **********/
  {
    if (!SndNewChannel(&cool[t], sampledSynth, initMono + initNoDrop, NIL)) {
      numchannel++;
      cool[t]->qLength = 128;
    }
  }

  if (numchannel < 3) {
    MyrParamText((Ptr) "Could not allocate all the requested sound channels.  Sounds may backlog or play out of sequence.  This is not fatal however.", (Ptr) "", (Ptr) "", (Ptr) "");
    background = GetNewDialog(151, NIL, (WindowPtr)-1L);
    SetPortDialogPort(background);
    ForeColor(yellowColor);
    SysBeep(20);
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
  }

  monsterset = 1;
  charnum = -1;

  getpref();

  FlushEvents(everyEvent, 0);
  SystemTask();
  t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif
  MyrCheckMemory(2);
  SystemTask(); //*** Have to do this twice before it reads the modifier keys?  Why?  Got me, just leave it.
  t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif

  if ((((**(**testdevice).gdPMap)).pixelSize < 8) && (!BitAnd(gTheEvent.modifiers, shiftKey))) {
    if (!auto256) {
      background = GetNewDialog(165, NIL, (WindowPtr)-1L);
      SysBeep(20);
      FlushEvents(everyEvent, 0);
      ModalDialog(0L, &itemHit);
      DisposeDialog(background);
    }

    if (itemHit == 4) {
      fullcolor = FALSE;
      goto keepmoving;
    }

    if ((itemHit == 1) || (auto256)) {
      currentDepth = ((**(**testdevice).gdPMap)).pixelSize;
      if (currentDepth < 8) {
        if (SetDepth(testdevice, 8, 4, ((**(**testdevice).gdPMap)).pixelSize))
          exit(0);
      }
    } else
      exit(0);
  }

keepmoving:

  if ((((**(**testdevice).gdPMap)).bounds.bottom - ((**(**testdevice).gdPMap)).bounds.top < 580) && (!BitAnd(gTheEvent.modifiers, shiftKey))) {
    MyrParamText((Ptr) "Sorry, this monitor is not displaying 800 X 600 pixels or greater.  This will not do.", (Ptr) "", (Ptr) "", (Ptr) "");
    background = GetNewDialog(151, NIL, (WindowPtr)-1L);
    SetPortDialogPort(background);
    ForeColor(yellowColor);
    SysBeep(20);
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    SetDepth(testdevice, currentDepth, 4, 8);
    exit(0);
  }

  /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
   * NOTE(fuzziqersoftware): Realmz checks if the Realmz Manual file exists at
   * startup time and exits if it's missing, but it appears the file isn't
   * actually used for anything. It could be that the file was needed in an
   * earlier version of Realmz and the dependency has since been removed, but
   * this check wasn't removed. Regardless of the reason, we've commented this
   * out since it seems the file is no longer required.

  if ((fp = MyrFopen(":Realmz Manual", "rb")) == NIL) {
    MyrParamText("Sorry, The file 'Realmz Manual' is a required file for Realmz to run.  Please make sure the file 'Realmz Manual' is in the same folder as Realmz.", (Ptr) "", (Ptr) "", (Ptr) "");
    background = GetNewDialog(151, NIL, (WindowPtr)-1L);
    SetPortDialogPort(background);
    ForeColor(yellowColor);
    SysBeep(20);
    FlushEvents(everyEvent, 0);
    ModalDialog(0L, &itemHit);
    SetDepth(testdevice, currentDepth, 4, 8);
    exit(0);
  } else
    fclose(fp);

   * *** END CHANGES *** */

  leftshift = screensize * 160;
  downshift = screensize * 96;

  GWorldInit(); /**** init offscreen buffers ***/

  GetSoundVol(&resetvolume);
  SetSoundVol(volume);

  /******** check to see if the logo or fading should be done ************/
  /*************************************************************************/

  t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif

  SystemTask(); //*** Have to do this twice before it reads the modifier keys?  Why?  Got me, just leave it.
  t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif

  if (gTheEvent.modifiers & optionKey)
    nofade = TRUE; /** option disables all fading routines **/

  if (BitAnd(gTheEvent.modifiers, shiftKey))
    tagger = TRUE; /** shift enambles code tracking **/

  if ((development) || (divine)) /*** development and Divine Right do not show Fantasoft logo ***/
  {
    nologo = nofade = TRUE;
  } else
    nologo = FALSE;

  if (!nologo)
    showlogo(130);

  /*************************************************************************/
  /*************************************************************************/

  sword = GetCCursor(128);
  HLock((Handle)sword);
  ask = GetCCursor(131);
  HLock((Handle)ask);
  mouse = GetCCursor(129);
  HLock((Handle)mouse);
  stop = GetCCursor(130);
  HLock((Handle)stop);
  notecursor = GetCCursor(133);
  HLock((Handle)sword);

  if (hidedesktop) {
    mat = GetNewWindow(132, 0L, (WindowPtr)-1L);
    SizeWindow(mat, width, depth, FALSE);
  }
  background = GetNewDialog(129 + 1000 * divine, NIL, (WindowPtr)-1L);
  MoveWindow(GetDialogWindow(background), GlobalLeft + 1 + (leftshift / 2), GlobalTop + 1 + (downshift / 2), FALSE);
  ShowWindow(GetDialogWindow(background));
  SetPortDialogPort(background);
  TextFont(defaultfont);
  gCurrent = background;
  DrawDialog(background);
  ForeColor(yellowColor);
  BackColor(blackColor);

  GetVersStr(1, Appl_Rsrc_Fork_Ref_Num); /*** load in version of Realmz ***/
  MyrPascalDiStr(3, theString);

  TextSize(32);
  TextFont(font);

#if !divine
  MyrCDiStr(2, (StringPtr) "Loading......Please wait.");
#endif

  if ((fp = MyrFopen(":Data Files:Data ID", "rb")) == NULL)
    scratch2(1);
  fread(&allweapons, sizeof allweapons, 1, fp);
  CvtTabItemAttrToPc(&allweapons, 200);
  fread(&allarmor, sizeof allarmor, 1, fp);
  CvtTabItemAttrToPc(&allarmor, 200);
  fread(&allhelms, sizeof allhelms, 1, fp);
  CvtTabItemAttrToPc(&allhelms, 200);
  fread(&allmagic, sizeof allmagic, 1, fp);
  CvtTabItemAttrToPc(&allmagic, 200);
  fclose(fp);

  if ((fp = MyrFopen(":Scenarios:City of Bywater:Data NI", "rb")) == NULL)
    scratch2(14);
  fread(&allsupply, sizeof allsupply, 1, fp);
  CvtTabItemAttrToPc(&allsupply, 200);

  fclose(fp); /************* load these in to start so New Character Torches/castes work ********/

  temp = MyrOpenResFile((Ptr) "\p:Data Files:Custom Names");
  usecustomnames = MyrOpenResFile((Ptr) "\p:Data Files:Scenario Names");
  portraitrefnum = MyrOpenResFile((Ptr) "\p:Data Files:Portraits");
  tacticalrefnum = MyrOpenResFile((Ptr) "\p:Data Files:Tacticals");

  refnum = MyrOpenResFile((Ptr) "\p:Scenarios:City of Bywater:Scenario");

  if (!reducesound)
    sound(20003);

noreg:

#if divine
  getdivineuser();
  if (!doreg())
    regdiv();
  showreg = TRUE;
  PtoCstr(Name_String);
  strcpy(myString, (StringPtr) "This copy is registered to                 ");
  strcat(myString, Name_String);
  CtoPstr((Ptr)Name_String);
  flashmessage(myString, 330, 200, -1, 0);
#else
  if (doreg()) {
    showreg = TRUE;
    PtoCstr(Name_String);
    if (strlen(Name_String) < 3) {
      serial = Rand(21987); /***** generate new serial number as it looks like a mistake ***/
      serial *= Rand(666);
      serial += Rand(32000);
      MyrBitSetLong(&serial, 8);
      MyrBitSetLong(&serial, 6 + divine);
      appnum = serial;

      PtoCstr(myString);

      if ((fp = MyrFopen((Ptr)myString, "r+b")) != NIL) {
        CvtLongToPc(&appnum);
        fwrite(&appnum, sizeof appnum, 1, fp);
        CvtLongToPc(&appnum);
        fclose(fp);
      }
      savepref();
    } else {
      strcpy(myString, (StringPtr) "This copy is registered to                 ");
      strcat(myString, Name_String);
      CtoPstr((Ptr)Name_String);
      flashmessage(myString, 15, 60, -1, 0);
    }
  }
#endif

  sound(20);
  SetCCursor(sword);

  if ((!nofade) && (!nologo))
    fadeinout(50, fadein); /******** fade to black ******/

  /******************** make sure caps lock is OFF ************************/
  /************************************************************************/
  FlushEvents(everyEvent, 0);
  SystemTask();
  t = GetNextEvent(everyEvent, &gTheEvent);
#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif
  if (gTheEvent.modifiers & alphaLock)
    warn(21);
  /************************************************************************/
  /************************************************************************/

  battlepict = GetPicture(160);
  HLock((Handle)battlepict);
  mainpict = GetPicture(176);
  // Myriad : detach the resource because they are used in dialog box
  DetachResource((Handle)mainpict);
  HLock((Handle)mainpict);
  grey = GetPicture(402);
  HLock((Handle)grey);
  marker = GetPicture(403);
  HLock((Handle)marker);
  on = GetPicture(147);
  HLock((Handle)on);
  off = GetPicture(150);
  HLock((Handle)off);
  non = GetPicture(146);
  HLock((Handle)non);
  gen = GetPicture(21);
  HLock((Handle)gen);

  curGDev = GetGDevice();

  MenuInit();

  if (nomusic) {
    /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
     * NOTE(danapplegate): SetMenuItemText expects a Pascal string. When compiled for the
     * classic Mac environment, string literals in C were assumed to be Pascal strings, but
     * here, we have to explicitly tell the compiler so with a '\p' character.
     */
    SetMenuItemText(musicmenu, 5, (StringPtr) "\pMusic Disabled In Preferences");
    nodispose = TRUE;
    for (t = 1; t < 28; t++)
      DisableItem(musicmenu, t);
    EnableItem(musicmenu, 2);
    EnableItem(musicmenu, 5);
    CheckItem(musicmenu, 1, FALSE);
  } else
    CheckItem(musicmenu, 1, TRUE);

  /**** set up rects for overhead view in dungeons ***************/
  for (tt = 0; tt < 6; tt++) {
    for (t = 0; t < 4; t++) {
      tiny[t + tt * 4].top = 320 + (16 * tt);
      tiny[t + tt * 4].left = 576 + 16 * t;
      tiny[t + tt * 4].right = tiny[t + tt * 4].left + 16;
      tiny[t + tt * 4].bottom = tiny[t + tt * 4].top + 16;
    }
  }
  /**************************************************************/

  SetMenuBar(myMenuBar);
  InsertMenu(gSound, -1);
  InsertMenu(gSpeed, -1);
  DrawMenuBar();

  gCurrent = background;
  blank = TRUE;
  currentshop = -1;

  showmagic = GetCIcon(2001);
  HLock((Handle)showmagic);

  strcpy(&fileinfo.fdCreator, (StringPtr) "RLMZ");

  greencolor.green = 65535;

  lightgrey.green = lightgrey.red = lightgrey.blue = 43690;

  goldcolor.red = 65535;
  goldcolor.blue = 8992;
  goldcolor.green = 50514;

  greycolor.green = greycolor.red = greycolor.blue = 30583;
  cyancolor.green = cyancolor.blue = 65535;

  greybox.top = 405;
  greybox.bottom = 406;
  greybox.left = 607;
  greybox.right = 608;

  bigspellrect.top = 0;
  bigspellrect.bottom = 224;
  bigspellrect.left = 0;
  bigspellrect.right = 224;

  buttons.top = 321 + downshift;
  buttons.left = 308 + leftshift;
  buttons.bottom = 410 + downshift;
  buttons.right = 640 + leftshift;

  info.top = 321 + downshift;
  info.left = 0;
  info.right = 308;
  info.bottom = 460 + downshift;

  sizestore.top = 32;
  sizestore.bottom = 96;
  sizestore.left = 576;
  sizestore.right = 640;

  lookrect.top = 0;
  lookrect.bottom = 320 + downshift;
  lookrect.left = 0;
  lookrect.right = 320 + leftshift;

  spellrect.top = 328 + downshift;
  spellrect.left = 570 + leftshift;
  spellrect.bottom = spellrect.top + 64;
  spellrect.right = spellrect.left + 64;

  infosmall.top = 410 + downshift;
  infosmall.left = 408 + leftshift;
  infosmall.right = 640 + leftshift;
  infosmall.bottom = 460 + downshift;

  if (screensize) //*** expand the cursor ranges for larger screen.
  {
    SetRect(&AHEAD_RECT, 80 + leftshift / 2, 0 + downshift / 2, 240 + leftshift / 2, 160 + downshift / 2);
    SetRect(&BACK_RECT, 80 + leftshift / 2, 160 + downshift / 2, 240 + leftshift / 2, 320 + downshift / 2);
    SetRect(&TURN_LEFT_RECT, 0 + leftshift / 2, 0 + downshift / 2, 80 + leftshift / 2, 320 + downshift / 2);
    SetRect(&TURN_RIGHT_RECT, 240 + leftshift / 2, 0 + downshift / 2, 320 + leftshift / 2, 320 + downshift / 2);
  }

  textrect.top = 333 + downshift;
  textrect.left = 12 + (3 * screensize);
  textrect.right = 296 + leftshift - (3 * screensize);
  textrect.bottom = 451 + downshift;

  spell.top = 335 + downshift;
  spell.left = 527 + leftshift;
  spell.right = spell.left + 32;
  spell.bottom = spell.top + 32;

  campavail = TRUE;
  canshop = shopavail = up = FALSE;

  if ((fp = MyrFopen(":Data Files:Data AD", "rb")) == NULL)
    scratch(197);
  fread(&spellarea, sizeof spellarea, 1, fp);
  fclose(fp);

  if ((fp = MyrFopen(":Data Files:Data S", "rb")) == NULL)
    scratch2(4); /********** load in standard spells **********/
  fread(&spelldata, sizeof spellinfo * 420, 1, fp);
  CvtTabSpellToPc(&spelldata, 420);
  fclose(fp);

  openrace();

  for (t = 0; t < 30; t++) {
    fread(&races, sizeof races, 1, fp);
    CvtRaceToPc(&races);
    racebasemove[t] = races.basemove; /***** load in race base movements ************/
  }
  fclose(fp);

  if ((fp = MyrFopen(":Data Files:Combat Data BD", "rb")) == NULL)
    scratch(412); /********* load in combat pixmap map stats ***/
  fread(&mapstats[200], (sizeof mapstats) / 2, 1, fp);
  CvtTabMapStatToPc(&mapstats[200], 402 / 2);
  fread(&basetile[2], sizeof basetile[2], 1, fp);
  CvtShortToPc(&basetile[2]);
  fread(&basescale[2], sizeof basescale[2], 1, fp);
  CvtShortToPc(&basescale[2]);
  fclose(fp);
}

/******************************** mainloop  ************************/
void MainLoop(void) {
  long oldtick;
  short flamestage2 = 0;

#if divine == 0
  Rect flamerect;

  flamerect.top = 122;
  flamerect.left = 474;
  flamerect.bottom = 154;
  flamerect.right = 506;

#endif

  oldtick = TickCount();
  SetMenuBar(myMenuBar);
  InsertMenu(gSound, -1);
  InsertMenu(gSpeed, -1);

  DrawMenuBar();
  FlushEvents(everyEvent, 0);
  for (;;) {
#if divine == 0
    if (TickCount() > oldtick + 20) {
      SetPortDialogPort(background);
      oldtick = TickCount();
      ploticon3(26178 + flamestage2, flamerect);

      flamestage2++;
      if (flamestage2 > 7)
        flamestage2 = 0;

      if (!reducesound) {
        t = Rand(100);
        if (t < 5)
          sound(615);
        if (t > 95)
          sound(645);
        if (twixt(t, 10, 15))
          sound(642);
        if (twixt(t, 30, 45))
          sound(10090);
      }
    }
#endif
    HandleEvent();
  }
}

/******************************** main  ************************/
int main(int argc, char* argvp[]) {
  ToolBoxInit();

  SetPortDialogPort(background);
  SetSoundVol(volume);
  BackColor(blackColor);

#if divine
  flashmessage((StringPtr) "", 330, 200, -1, 0);
  showreg = FALSE;
#else
  if (showreg) {
    flashmessage((StringPtr) "", 15, 60, -1, 0);
    showreg = FALSE;
  }
#endif

  SetPortDialogPort(background);
  ForeColor(blackColor);
  BackColor(blackColor);
  MyrCDiStr(2, (StringPtr) "");
  MyrCDiStr(3, (StringPtr) "");
  DrawDialog(background);
  MainLoop();

  return 0;
}

/************************** checkforerrors ********************************/
short checkforerrors(void) {
  short t, tt;
  //**  this is to correct problems with old format Mac version characters.
  //***********************************************************************

#if !winversion

  // if ((characterl.name[0] < 'A') && (characterl.version > -4))/*** correct chracter file if it is corrupt ***/
  if (characterl.name[0] < 'A') /*** correct chracter file if it is corrupt ***/
  {
    characterl.version = -4;
    for (t = 0; t < 7; t++) {
      for (tt = 0; tt < 12; tt++) {
        characterl.cspells[t][tt] = characterl.cspells[t][tt + 1];
      }
    }

    /*** now need to offset name by one byte too.  What a pain in the carcass! ***/
    for (t = 0; t < 30; t++)
      characterl.name[t] = characterl.name[t + 1];

    savecharacter(-1);
    return (TRUE);
  }

#endif

  return (FALSE);
}

#if divine
/*************************** about divineright **************************/
void aboutrealmz(void) {
  GrafPtr oldport;
  long noteid = 0;
  long regcode = 0;
  short randomcon[9] = {738, 752, 724, 741, 755, 790, 718, 713, 731};
  Str255 str; //  Fantasoft v7.1

changed:

  sound(10107);
  GetPort(&oldport);
  if (look)
    in();

  showserial = serial;

  MyrBitClrLong(&showserial, 7);
  MyrNumToString(showserial, myString);
  CtoPstr(myString);
  strcpy(str, (StringPtr) "Not Registered");
  CtoPstr(str);
  if (MyrBitTstLong(&serial, 7))
    ParamText(myString, str, theString, (StringPtr) "");
  else
    ParamText(myString, Name_String, theString, (StringPtr) "");

  about = GetNewDialog(166, 0L, (WindowPtr)-1L);
  SetPort(about);
  BackPixPat(base);
  TextFont(defaultfont);
  gCurrent = about;
  ForeColor(yellowColor);

  MoveWindow(about, GlobalLeft, GlobalTop, FALSE);
  ShowWindow(about);
  EraseRect(&qd.thePort->portRect);
  DrawDialog(about);

  GetDialogItem(about, 3, &itemType, &itemHandle, &itemRect);
  SetDialogItemText(itemHandle, (StringPtr) "");
  ploticon3(randomcon[Rand(9) - 1], itemRect);
  sound(30000);
  delay(20);
  sound(10141);
  BeginUpdate(about);
  EndUpdate(about);
  FlushEvents(everyEvent, 0);
  for (;;) {
    ModalDialog(0L, &itemHit);
    goto out;
  }
out:
  SetPort(oldport);
  if (!look) {
    SetPort(background);
    BackColor(blackColor);
    gCurrent = background;
    MyrCDiStr(2, (StringPtr) "");
    MyrCDiStr(3, (StringPtr) "");
    DisposeDialog(about);
    about = NIL;
    DrawDialog(background);
  } else {
    DisposeDialog(about);
    about = NIL;
    if (!incombat)
      updatemain(FALSE, -1);
  }
  FlushEvents(everyEvent, 0);
  return;
}
#endif

#if !winversion
/************************************ compactheap **********************/
void compactheap(void) {
  Size tempsize;
  long templong;

  UnlockPixels(edge); /****** Unlock GWorlds so the memory manager can get its house in order  ******/
  UnlockPixels(buff);
  UnlockPixels(buff2);
  UnlockPixels(thePixels);
  UnlockPixels(gBackPix);
  UnlockPixels(gFloorPix);
  UnlockPixels(gHWallsPix);
  UnlockPixels(gVWallsPix);
  UnlockPixels(gPillarsPix);
  UnlockPixels(maps);

  templong = MaxMem(&tempsize);

  edge = GetGWorldPixMap(gedge); /****** lock GWorld for cb functions ******/
  LockPixels(edge);

  buff = GetGWorldPixMap(gbuff); /****** lock GWorld for cb functions ******/
  LockPixels(buff);

  buff2 = GetGWorldPixMap(gbuff2); /****** lock GWorld for cb functions ******/
  LockPixels(buff2);

  thePixels = GetGWorldPixMap(gthePixels); /****** lock GWorld for cb functions ******/
  LockPixels(thePixels);

  gBackPix = GetGWorldPixMap(gBackWorld); /****** lock GWorld for cb functions ******/
  LockPixels(gBackPix);

  gFloorPix = GetGWorldPixMap(gFloorWorld); /****** lock GWorld for cb functions ******/
  LockPixels(gFloorPix);

  gHWallsPix = GetGWorldPixMap(gHWallsWorld); /****** lock GWorld for cb functions ******/
  LockPixels(gHWallsPix);

  gVWallsPix = GetGWorldPixMap(gVWallsWorld); /****** lock GWorld for cb functions ******/
  LockPixels(gVWallsPix);

  gPillarsPix = GetGWorldPixMap(gPillarsWorld); /****** lock GWorld for cb functions ******/
  LockPixels(gPillarsPix);

  maps = GetGWorldPixMap(gmaps); /****** lock GWorld for cb functions ******/
  LockPixels(maps);
}
#endif

/******************************** fadeinout  ************************/
void FadeOut(void);
void FadeIn(void);
void fadeinout(short steps, short fadedirection) {
  if ((nologo) || (nofade))
    return;

#ifdef PC

  if (fadedirection == fadeout)
    FadeOut();
  else
    FadeIn();

#else
  if (IsColorGammaAvailable()) {
    switch (fadedirection) {
      case fadeout:

        if (!fadestate) {
          StartFading(&initialState);
          FadeToBlack(steps, quadraticFade);
          fadestate = TRUE;
        }

        break;

      case fadein:

        if (fadestate) {
          FadeToGamma(initialState, 50, quadraticFade);
          StopFading(initialState, TRUE);
          fadestate = FALSE;
        }

        break;
    }
  }
#endif
}

/*************** regscen *******************************************/
short regscen(void) {
  long firsttime, secondtime;
  char tempword[40];
  short toolong = 180;
  Str255 tempnamestring;
  short primer1, primer2, primer3;
  long regcode, serialnumber;

  serialnumber = 0;
  if ((currentscenario == 13) || (currentscenario > 14)) {
    serialnumber = serial;
    serialnumber /= ((currentscenario - 5) * 666);
  }
#ifdef PC // Modif Myriad 15-9-99

  serialnumber = serial;
  serialnumber /= ((currentscenario - 5) * 666);

#endif

  flashmessage((StringPtr) "Enter your registration name for this scenario.", 30, 40, -1, 0);
  getword();
  flashmessage((StringPtr) "", 30, 40, -1, 0);
  strcpy(registrationname, (StringPtr)gotword);

  if (currentscenario > topfantasoftsceanrio) //*** v7.1  3rd party scenario
  {
    getfilename("Data CS"); /**** get codesegs from backup for use in demixing. ******/
    if ((fp = fopen(filename, "rb")) == NULL)
      scratch(700);
    fseek(fp, 5 * sizeof reclevel, SEEK_SET);
    fread(&codeseg3, 20, 1, fp);
    fread(&codeseg4, 20, 1, fp);
    fclose(fp);

    for (t = 0; t < 20; t++) /***** unmix codes segments using backup as the de-mixer ****/
    {
      codeseg2[t] -= codeseg3[t];
      codeseg1[t] -= codeseg2[t];
    }

    showserial = serial;
    BitClr(&showserial, 8); /*** may need to get the boot ***/
    serialnumber = regcode = showserial;

    temp = strlen(codeseg1);
    if (strlen(codeseg2) < temp)
      temp = strlen(codeseg2);

    for (t = 0; t < temp; t++) {
      regcode += codeseg1[t];
      regcode -= codeseg2[t];
    }

    StringToNum((StringPtr)registrationname, &tempvalue); /**** make sure its in pascal first ****/
    regcode += tempvalue; /***** regname *****/

    GetMenuItemText(gGame, currentscenario, myString);
    StringToNum(myString, &tempvalue);
    regcode += tempvalue; /***** title *****/

    firsttime = TickCount();

    MyrNumToString(serial, myString);

    CtoPstr((Ptr)codeseg1);
    CtoPstr((Ptr)codeseg2);

    primer1 = 32 - (registrationname[0] % 4);
    primer2 = 31 - (codeseg1[0] % 3);
    primer3 = 30 - (codeseg2[0] % 5);

    for (tt = 1; tt <= registrationname[0]; tt++)
      registrationname[tt] = tolower(registrationname[tt]); /**** convert to all lower case ******/

    /**** do bit flip for reg name ******/

    for (tt = 1; tt <= registrationname[0]; tt++) /**** reg name ******/
    {
      if (MyrBitTstLong(&regcode, registrationname[tt] % primer2)) {
        MyrBitClrLong(&regcode, registrationname[tt] % primer2 + 2);
      } else {
        MyrBitSetLong(&regcode, registrationname[tt] % primer2 + 1);
      }
    }

    /**** do bit flip for s/n ******/

    for (tt = 1; tt <= myString[0]; tt++) /***** do bit flip for s/n ****/
    {
      if (MyrBitTstLong(&regcode, myString[tt] % primer1 + 2)) {
        MyrBitClrLong(&regcode, myString[tt] % primer1 + 1);
      } else {
        MyrBitSetLong(&regcode, myString[tt] % primer1);
      }
    }

    /**** do bit flip for codeseg1 ******/

    for (tt = 1; tt <= codeseg1[0]; tt++) /***** do bit flip for codeseg1 ****/
    {
      if (MyrBitTstLong(&regcode, codeseg1[tt] % primer3 + 2)) {
        MyrBitClrLong(&regcode, codeseg1[tt] % primer3);
      } else {
        MyrBitSetLong(&regcode, codeseg1[tt] % primer3 + 1);
      }
    }

    /**** do bit flip for codeseg2 ******/

    for (tt = 1; tt <= codeseg2[0]; tt++) /***** do bit flip for codeseg2 ****/
    {
      if (MyrBitTstLong(&regcode, codeseg2[tt] % primer1)) {
        MyrBitClrLong(&regcode, codeseg2[tt] % primer2);
      } else {
        MyrBitSetLong(&regcode, codeseg2[tt] % primer3);
      }
    }

    /***** re-mix codes segsments using backup as the mixer ****/
    /***** this is in case they need to enter it a second time ****/
    GetMenuItemText(gGame, currentscenario, myString); /**** get codesegs ******/
    PtoCstr(myString);
    getfilename((Ptr)myString);
    if ((fp = MyrFopen(filename, "rb")) == NULL)
      scratch(700);
    fseek(fp, 5 * sizeof reclevel, SEEK_SET);
    fread(&codeseg1, 20, 1, fp);
    fread(&codeseg2, 20, 1, fp);
    fclose(fp);

    secondtime = TickCount();

    if (regcode < 0)
      regcode *= -1;

    flashmessage((StringPtr) "Now enter your registration number for this scenario.", 30, 40, -1, 0);
    getword();
    flashmessage((StringPtr) "", 30, 40, -1, 0);

    StringToNum((StringPtr)gotword, &tempvalue);
    strcpy(tempnamestring, Name_String);
    for (tt = 1; tt <= tempnamestring[0]; tt++)
      tempnamestring[tt] = tolower(tempnamestring[tt]); /**** convert to all lower case ******/

    PtoCstr(tempnamestring);
    PtoCstr((StringPtr)registrationname);

    tempvalue += strcmp(tempnamestring, registrationname); /***** user does not match code so offset tempvalue *****/

    if (tempvalue != regcode) {
      warn(78);

      if (FrontWindow() == look) {
        SetPort(GetWindowPort(screen));
        RGBBackColor(&greycolor);
      }
      return (FALSE);
    } else {
      FlushEvents(everyEvent, 0);
      WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);
#ifdef PC // Myriad
      DoCorrectBugMADRepeat();
#endif

      flashmessage((StringPtr) "Please keep your registration name and number in a safe place.  We cannot stress this enough.", 50, 150, 0, 1101);
      flashmessage((StringPtr) "If you begin a NEW ADVENTURE, you will need to re-enter these codes at this point in the future.", 50, 150, 0, 1100);
      flashmessage((StringPtr) "Do not distribute any saved game files beyond this point for any reason.  Thanks and have fun.", 50, 150, 0, 1102);
      flashmessage((StringPtr) "Now might be a good time to save the game.", 50, 150, 0, 1102);
      if (strcmp(tempnamestring, registrationname))
        exit(0); /***** user does not match code so offset tempvalue *****/
      setfree2(0);
      quest[127] = 66; /**** shows registered scenario ****/
      return (TRUE);
    }
  }

  // Fantasoft v7.0 BEGIN  I want the PC version to always use the bit switch for names

#ifdef PC
  if (currentscenario > 0) /**** Do bit switch by name for all scenarios on PC side *****/
  {
#else
  if (currentscenario > 14) /**** Do bit switch by name for WD or higher on mac side *****/
  {
#endif
    // Fantasoft v7.0 END

    strcpy(tempword, (StringPtr)gotword);
    PtoCstr((StringPtr)tempword);
    StringToNum((StringPtr)gotword, &regcode);

    for (tt = 0; tt < 26; tt++) {
      tempword[tt] = tolower(tempword[tt]);
      if (tempword[tt] == 0)
        goto pushon;
      if (tempword[tt] == 32)
        tempword[tt] = 128;

      temp = tempword[tt] - 97;

      if (MyrBitTstLong(&regcode, temp)) {
        MyrBitClrLong(&regcode, temp);
      } else {
        MyrBitSetLong(&regcode, temp);
      }
    }

  pushon:

    templong = serial;

    NumToString(templong, myString);

    for (tt = 1; tt <= myString[0]; tt++) /***** bitflip for s.n. ****/
    {
      if (BitTst(&regcode, myString[tt] + myString[tt] + tt - 95)) {
        BitClr(&regcode, myString[tt] + myString[tt] + tt - 95);
      } else {
        BitSet(&regcode, myString[tt] + myString[tt] + tt - 95);
      }
    }

    if (currentscenario == 21) //**** wrath of mind lords
    {
      if (serialnumber) {
        regcode = regcode % serialnumber;
        regcode *= myString[0];
      }
    } else {
      regcode = (((256 + 1024 * myString[0])) % (69 + myString[myString[0]])) + (((1024 + 256 * myString[0])) % (96 + myString[myString[0]])) * abs((regcode % serialnumber) * (tempword[0] * temp * abs(tempword[2] - tempword[1])));
    }
    serialnumber += regcode;
  }

  firsttime = TickCount();

  StringToNum((StringPtr)gotword, &tempvalue);

  GetMenuItemText(gGame, currentscenario, myString);
  PtoCstr(myString);
  getfilename((Ptr)myString);

  secondtime = TickCount();

  if ((fp = fopen(filename, "rb")) == NULL)
    scratch(122);
  fread(&reclevel, sizeof reclevel, 2, fp);
  fclose(fp);

  tempvalue /= reclevel;
  tempvalue += 24;
  tempvalue *= maxlevel;
  tempvalue -= 256;
  templong = tempvalue;

  if (templong < 0)
    templong *= -1;

  templong += 100;

  flashmessage((StringPtr) "Now enter your registration number as it appears on your registration form for this scenario.", 30, 40, -1, 0);

  getword();

  StringToNum((StringPtr)gotword, &tempvalue);

  if ((tempvalue < 999) && (tempvalue))
    exit(0);

  flashmessage((StringPtr) "", 30, 40, -1, 0);

  if (FrontWindow() == look) {
    SetPort(GetWindowPort(screen));
    RGBBackColor(&greycolor);
    EraseRect(&textrect);
  }

  if (tempvalue < 0)
    tempvalue *= -1;

    // Fantasoft v7.0 BEGIN
#ifdef PC
//
#else
  if ((templong + serialnumber - 100) == tempvalue) /********** check for old style scenario code on Macs only *********/
  {
    flashmessage((StringPtr) "If you have registered your copy with Fantasoft, contact us to get your new codes for this version.", 50, 150, 0, 1101);
    flashmessage((StringPtr) "Read the document 'Re-Registration Request' that is in your Realmz folder for details.", 50, 150, 0, 1100);
    goto wrong;
  }
#endif
  // Fantasoft v7.0 END

  if ((templong + serialnumber) != tempvalue) {
  wrong:
    warn(78);

    if (FrontWindow() == look) {
      SetPort(GetWindowPort(screen));
      RGBBackColor(&greycolor);
    }
    return (FALSE);
  }

  FlushEvents(everyEvent, 0);
  WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);

#ifdef PC // Myriad
  DoCorrectBugMADRepeat();
#endif

  flashmessage((StringPtr) "Please keep your registration name and numbers in a safe place.  We cannot stress this enough.", 50, 150, 0, 1101);
  flashmessage((StringPtr) "If you begin a NEW ADVENTURE, you will need to re-enter these codes at this point in the future.", 50, 150, 0, 1100);
  flashmessage((StringPtr) "Now might be a good time to save the game.", 50, 150, 0, 1102);
  centerpict();
  setfree2(0);
  quest[127] = 66; /**** shows registered scenario ****/
  return (TRUE);
}

/**************** specailabs **************/
short specailabs(short i) {
  if (i < 0)
    return (-i); /******** this HAS to be here to account for a bug in the old Think C abs() routine I used for the registraiton codes.  Yuck! ***/
  return (i);
}

/*************** regscen_pc *******************************************/
short regscen_pc(void) {
  long NameValue, SerialValue, RegistrationValue, RegistrationCode, UserCode, part1, part2;

  // This function calculates the number require to registered in the following way
  // Part 1) Get a registration name from the user
  //         Convert that name to a C string in all lower case letters.
  //         Use that name to calculate NameValue
  //
  // Part 2) Calculate the value of SerialValue using the serial number and scenario number
  //
  // Part 3) Use NameValue and SerialValue as calculated part 1 & 2 which gives us RegistrationValue
  //
  // Part 4) Use RegistrationValue along with variables from scenario to calculate RegistrationCode
  //
  // Part 5) Compare the RegistrationCode as calculated with that as entered by the user

  //============================================================================================================

  // Before we even start, lets send them to the module regscen_pc_custom() if it's a 3rd party scenairo instead of this module.

  if (currentscenario > 19) //*** handles the registration for custom scenarios ***/
  {
    reply = regscen_pc_custom();
    return (reply);
  } else {

    // Part 1: Get the value of NameValue
    //
    flashmessage((StringPtr) "Enter your registration name for this scenario.", 30, 40, -1, 0);
    getword();
    flashmessage((StringPtr) "", 30, 40, -1, 0);

    PtoCstr((StringPtr)gotword); // Convert it to a C String

    for (tt = 0; tt < 26; tt++) // Convert it to all lower case letters
    {
      gotword[tt] = tolower(gotword[tt]);
    }

    gotword[26] = 0; // Maximum of 26 letters in name

    NameValue = serial; // Initialize NameValue = Serial Number of copy of game.

    for (tt = 1; tt < strlen(gotword); tt++) // Use name to calculate NameValue
    {
      if (gotword[tt]) {
        NameValue += (tt * gotword[tt]);
        NameValue -= (gotword[tt] * gotword[tt - 1]);
      }
    }

    // Part 2: Calculate SerialValue
    //
    SerialValue = serial;
    SerialValue /= ((currentscenario - 5) * 666);

    // Part 3: Calculate RegistrationValue using SerialValue and NameValue
    //

    part1 = 256 * ((512 + SerialValue) % (128 * NameValue));
    part2 = 1024 + ((1024 + NameValue) % (512 * SerialValue));

    RegistrationValue = part1 + part2;

    // Part 4: Calculate RegistrationCode using RegistrationValue and data from scenario
    //

    GetMenuItemText(gGame, currentscenario, myString);
    PtoCstr(myString);
    getfilename((Ptr)myString);

    if ((fp = MyrFopen(filename, "rb")) == NULL)
      scratch(122);
    /* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
     * NOTE(fuzziqersoftware): On many modern systems, a long is 64 bits, but
     * on systems at the time Realmz was originally written, a long was
     * always 32 bits. To correct for this, we've changed sizeof(long) to
     * sizeof(int32_t) here.
     */
    fread(&reclevel, sizeof(int32_t), 1, fp);
    CvtLongToPc(&reclevel);
    fread(&maxlevel, sizeof(int32_t), 1, fp);
    CvtLongToPc(&maxlevel);
    fclose(fp);

    RegistrationCode = RegistrationValue;
    RegistrationCode *= maxlevel;
    RegistrationCode -= 512;
    RegistrationCode /= reclevel;
    RegistrationCode += 18;

    // Part 5: Compare RegistrationCode with that entered by user to see if it is valid.
    //         RegistrationCode should = what the user types in for valid registration

    flashmessage((StringPtr) "Now enter your registration number as it appears on your registration form for this scenario.", 30, 40, -1, 0);

    getword();

    StringToNum((StringPtr)gotword, &UserCode);

    if ((UserCode < 999) && (UserCode))
      exit(0);

    flashmessage((StringPtr) "", 30, 40, -1, 0);

    if (FrontWindow() == look) {
      SetPort(GetWindowPort(screen));
      RGBBackColor(&greycolor);
      EraseRect(&textrect);
    }

    if (RegistrationCode != UserCode) //**** does entered code match what is expected?
    {
      warn(78);

      if (FrontWindow() == look) {
        SetPort(GetWindowPort(screen));
        RGBBackColor(&greycolor);
      }
      return (FALSE);
    }

    FlushEvents(everyEvent, 0);
    WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);

#ifdef PC
    DoCorrectBugMADRepeat();
#endif

    flashmessage((StringPtr) "Please keep your registration name and numbers in a safe place.  We cannot stress this enough.", 50, 150, 0, 1101);
    flashmessage((StringPtr) "If you begin a NEW ADVENTURE, you will need to re-enter these codes at this point in the future.", 50, 150, 0, 1100);
    flashmessage((StringPtr) "Now might be a good time to save the game.", 50, 150, 0, 1102);
    centerpict();

    setfree2(0);
    quest[127] = 66; /**** shows registered scenario ****/
    return (TRUE);
  }
}

/*************** regscen_pc_custom *******************************************/
short regscen_pc_custom(void) {
  long NameValue, SerialValue, RegistrationValue, RegistrationCode, UserCode, part1, part2;

  // This function calculates the number require to registered in the following way
  // Part 1) Get a registration name from the user
  //         Convert that name to a C string in all lower case letters.
  //         Use that name to calculate NameValue
  //
  // Part 2) Calculate the value of SerialValue using the serial number
  //
  // Part 3) Use NameValue and SerialValue as calculated part 1 & 2 which gives us RegistrationValue
  //
  // Part 4) Use RegistrationValue along with secret  from scenario to calculate RegistrationCode
  //
  // Part 5) Compare the RegistrationCode as calculated with that as entered by the user

  //============================================================================================================

  // Part 1: Get the value of NameValue
  //
  flashmessage((StringPtr) "Enter your registration name for this scenario.", 30, 40, -1, 0);
  getword();
  flashmessage((StringPtr) "", 30, 40, -1, 0);

  PtoCstr((StringPtr)gotword); // Convert it to a C String

  for (tt = 0; tt < 26; tt++) // Convert it to all lower case letters
  {
    gotword[tt] = tolower(gotword[tt]);
  }

  gotword[26] = 0; // Maximum of 26 letters in name

  NameValue = serial; // Initialize NameValue = Serial Number of copy of game.

  for (tt = 1; tt < strlen(gotword); tt++) // Use name to calculate NameValue
  {
    if (gotword[tt]) {
      NameValue += (tt * gotword[tt]);
      NameValue -= (gotword[tt] * gotword[tt - 1]);
    }
  }

  // Part 2: Calculate SerialValue
  //
  SerialValue = serial;
  SerialValue /= 333;

  // Part 3: Calculate RegistrationValue using SerialValue and NameValue
  //

  part1 = 512 * ((450 + SerialValue) % (96 * NameValue));
  part2 = 999 + ((999 + NameValue) % (456 * SerialValue));

  RegistrationValue = part1 + part2;

  // Part 4: Calculate RegistrationCode using RegistrationValue and data from scenario
  //

  RegistrationCode = RegistrationValue;

  getfilename("Data CS"); /**** get codesegs from backup for use in demixing. ******/
  if ((fp = MyrFopen(filename, "rb")) == NULL)
    scratch(700);
  fseek(fp, 5 * sizeof reclevel, SEEK_SET);
  fread(&codeseg3, 20, 1, fp);
  fread(&codeseg4, 20, 1, fp);
  fclose(fp);

  for (t = 0; t < 20; t++) /***** unmix codes segments using backup as the de-mixer ****/
  {
    codeseg2[t] -= codeseg3[t];
    codeseg1[t] -= codeseg2[t];
  }

  for (t = 0; t < 20; t++) // Convert it to all lower case letters
  {
    codeseg1[t] = tolower(codeseg1[t]);
    codeseg2[t] = tolower(codeseg2[t]);
  }

  for (t = 0; t < strlen(codeseg1); t++) /***** add values of code segments to RegistrationCode ****/
  {
    RegistrationCode += (short)(1689 * (short)codeseg1[t]);
  }

  for (t = 0; t < strlen(codeseg2); t++) /***** add values of code segments to RegistrationCode ****/
  {
    RegistrationCode -= (short)(423 * (short)codeseg2[t]);
  }

  GetMenuItemText(gGame, currentscenario, myString);
  PtoCstr(myString);

  for (t = 0; t < 60; t++) // Convert it to all lower case letters
  {
    myString[t] = tolower(myString[t]);
  }

  for (t = 0; t < strlen(myString); t++) /***** add values of title to RegistrationCode ****/
  {
    RegistrationCode += 112233L * (long)myString[t];
  }

  // Part 5: Compare RegistrationCode with that entered by user to see if it is valid.
  //         RegistrationCode should = what the user types in for valid registration

  flashmessage((StringPtr) "Now enter your registration number as it appears on your registration form for this scenario.", 30, 40, -1, 0);

  getword();

  StringToNum((StringPtr)gotword, &UserCode);

  if ((UserCode < 999) && (UserCode))
    exit(0);

  flashmessage((StringPtr) "", 30, 40, -1, 0);

  if (FrontWindow() == look) {
    SetPort(GetWindowPort(screen));
    RGBBackColor(&greycolor);
    EraseRect(&textrect);
  }

  if (RegistrationCode != UserCode) //**** does entered code match what is expected?
  {
    warn(78);

    if (FrontWindow() == look) {
      SetPort(GetWindowPort(screen));
      RGBBackColor(&greycolor);
    }
    return (FALSE);
  }

  FlushEvents(everyEvent, 0);
  WaitNextEvent(everyEvent, &gTheEvent, 0L, NIL);

#ifdef PC
  DoCorrectBugMADRepeat();
#endif

  flashmessage((StringPtr) "Please keep your registration name and numbers in a safe place.  We cannot stress this enough.", 50, 150, 0, 1101);
  flashmessage((StringPtr) "If you begin a NEW ADVENTURE, you will need to re-enter these codes at this point in the future.", 50, 150, 0, 1100);
  flashmessage((StringPtr) "Now might be a good time to save the game.", 50, 150, 0, 1102);
  centerpict();

  setfree2(0);
  quest[127] = 66; /**** shows registered scenario ****/
  return (TRUE);
}

/***************** quickinfo *******************/
void quickinfo(int who, int itemnumber, int itemid, int where) {
  Rect pictrect, txtbox;
  int tempid;
  RGBColor fore, back;
  GrafPtr oldPort;
  Boolean showcurse = 0;
  Boolean ident = 0;

  if (!screensize)
    return;

  GetPort(&oldPort);
  GetForeColor(&fore);
  GetBackColor(&back);

  tempid = itemid;

  if ((itemnumber < 0) && (who > -1))
    return;

  switch (where) {
    case 1:
      SetPort(GetWindowPort(gshop));
      break;

    case 2:
      SetPort(GetWindowPort(bootywindow));
      break;

    default:
      SetPort(GetWindowPort(itemswindow));
      break;
  }

  BackPixPat(base);
  TextSize(16);
  ForeColor(yellowColor);
  TextMode(1);
  TextFont(font);
  txtbox.top = 468;
  txtbox.left = 7;
  txtbox.right = txtbox.left + 275;
  txtbox.bottom = txtbox.top + 80;

  pictrect.top = 460;
  pictrect.left = 0;
  pictrect.right = 800;
  pictrect.bottom = pictrect.top + downshift;

  switch (who) {
    case -1:
      loaditem(itemid);
      showcurse = 1;
      break;

    default:
      loaditem(c[who].items[itemnumber].id);
      if (c[who].items[itemnumber].equip)
        showcurse = 1;
      else
        showcurse = 0;
      break;
  }

  if (item.iscurse) {
    if (!showcurse)
      temp = getselection(item.iscurse);
    else
      getselection(item.itemid);
    inbooty = TRUE; //*** keeps the beep for text from happening
    pict(218, pictrect);
    textbox(temp + 2, item.iscurse - temp + 1, FALSE, TRUE, txtbox);
    inbooty = FALSE;
  } else {
    getselection(item.itemid);
    inbooty = TRUE; //*** keeps the beep for text from happening
    pict(218, pictrect);
    textbox(tempselection + 2, item.itemid - tempselection + 1, FALSE, TRUE, txtbox);
    inbooty = FALSE;
  }

  MoveTo(300, 484);
  stringnozeronoplus(item.vssmall);

  MoveTo(332, 484);
  stringnozeronoplus(item.vslarge);

  MoveTo(310, 513);
  temp = item.damage + item.heat + item.vsundead + item.cold + item.electric + item.vsdd + item.vsevil;
  stringnozero(temp);

  MoveTo(310, 539);
  temp = item.ac;
  stringnozero(temp);

  MoveTo(510, 484);
  temp = item.movement;
  stringnozero(temp);

  MoveTo(510, 513);
  temp = item.magres;
  stringnozero(temp);

  MoveTo(510, 539);
  temp = item.spellpoints;
  stringnozero(temp);

  SetPort(oldPort);
  BackPixPat(whitepat);
  RGBBackColor(&back);
  RGBForeColor(&fore);
}
