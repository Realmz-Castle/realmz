#include "convert.h"
#include "structs.h"

void CvtItemToPc(struct item* x) {
  CvtShortToPc(&x->id);
  CvtShortToPc(&x->charge);
}

void CvtRaceToPc(struct race* x) {
  CvtTabShortToPc(x->plusminustohit, 8);
  CvtTabShortToPc(x->specialability, 14);
  CvtTabShortToPc(x->drvbonus, 8);
  CvtTabShortToPc(x->attbonus, 6);
  CvtTabShortToPc(x->minmax, 12);
  CvtTabShortToPc(x->spare, 8);
  CvtTabShortToPc(x->conditions, 40);
  CvtShortToPc(&x->maxage);
  CvtShortToPc(&x->doesnotdie);
  CvtShortToPc(&x->basemove);
  CvtShortToPc(&x->magres);
  CvtShortToPc(&x->twohand);
  CvtShortToPc(&x->missile);
  CvtTabShortToPc(x->numofattacks, 2);
  CvtTabShortToPc((short*)x->agerange, 5 * 2);
  CvtTabLongToPc(x->itemtypes, 2);
  CvtShortToPc(&x->defaulticonset);
  CvtShortToPc(&x->descriptors);
  CvtTabShortToPc(x->spacer, 31);
}

void CvtCasteToPc(struct caste* x) {
  CvtTabShortToPc((short*)x->specialability, 2 * 14);
  CvtTabShortToPc(x->drvbonus, 8);
  CvtTabShortToPc(x->attbonus, 6);
  CvtTabShortToPc((short*)x->spellcasters, 4 * 3);
  CvtTabShortToPc(x->minmax, 12);
  CvtTabShortToPc(x->conditions, 40);
  CvtShortToPc(&x->canusemissile);
  CvtShortToPc(&x->getsmissilebonus);
  CvtTabShortToPc(x->stamina, 2);
  CvtTabShortToPc(x->strength, 2);
  CvtTabShortToPc(x->dodge, 2);
  CvtTabShortToPc(x->tohit, 2);
  CvtTabShortToPc(x->missile, 2);
  CvtTabShortToPc(x->hand2hand, 2);
  CvtTabShortToPc(x->spare1, 2);
  CvtTabShortToPc(x->spare2, 2);
  CvtShortToPc(&x->casteclass);
  CvtShortToPc(&x->minimumagegroup);
  CvtShortToPc(&x->movebonus);
  CvtShortToPc(&x->magres);
  CvtShortToPc(&x->twohand);
  CvtShortToPc(&x->maxstaminabonus);
  CvtShortToPc(&x->bonusattacks);
  CvtShortToPc(&x->maxattacks);
  CvtTabLongToPc(x->victory, 30);
  CvtShortToPc(&x->startmoney);
  CvtTabShortToPc(x->startitems, 20);
  CvtTabLongToPc(x->itemtypes, 2);
  CvtShortToPc(&x->defaulticon);
  CvtShortToPc(&x->maxspellsattacks);
  CvtShortToPc(&x->spellssofar);
  CvtTabShortToPc(x->spacer, 63);
}

void CvtCharacterToPc(struct character* x) {
  CvtShortToPc(&x->version);
  CvtShortToPc(&x->verify1);
  CvtShortToPc(&x->tohit);
  CvtShortToPc(&x->dodge);
  CvtShortToPc(&x->missile);
  CvtShortToPc(&x->twohand);
  CvtShortToPc(&x->traiter);
  CvtShortToPc(&x->normattacks);
  CvtShortToPc(&x->beenattacked);
  CvtShortToPc(&x->guarding);
  CvtShortToPc(&x->target);
  CvtShortToPc(&x->numitems);
  CvtShortToPc(&x->weaponsound);
  CvtShortToPc(&x->underneath);
  CvtShortToPc(&x->face);
  CvtShortToPc(&x->attackbonus);
  CvtShortToPc(&x->magco);
  CvtShortToPc(&x->position);
  CvtShortToPc(&x->maglu);
  CvtShortToPc(&x->magst);
  CvtShortToPc(&x->magres);
  CvtShortToPc(&x->movebonus);
  CvtShortToPc(&x->ac);
  CvtShortToPc(&x->damage);
  CvtShortToPc(&x->race);
  CvtShortToPc(&x->caste);
  CvtShortToPc(&x->spellcastertype);
  CvtShortToPc(&x->gender);
  CvtShortToPc(&x->level);
  CvtShortToPc(&x->movement);
  CvtShortToPc(&x->movementmax);
  CvtShortToPc(&x->attacks);
  CvtTabShortToPc(x->nspells, 7);
  CvtShortToPc(&x->stamina);
  CvtShortToPc(&x->staminamax);
  CvtShortToPc(&x->pictid);
  CvtShortToPc(&x->iconid);
  CvtShortToPc(&x->spellpoints);
  CvtShortToPc(&x->spellpointsmax);
  CvtShortToPc(&x->nohands);
  CvtShortToPc(&x->weaponnum);
  CvtShortToPc(&x->missilenum);
  CvtShortToPc(&x->handtohand);
  CvtTabShortToPc(x->condition, 40);
  CvtTabShortToPc(x->special, 12);
  CvtTabShortToPc(x->armor, 20);
  CvtTabShortToPc(x->spec, 15);
  CvtTabShortToPc(x->save, 8);
  CvtShortToPc(&x->currentagegroup);
  CvtShortToPc(&x->verify2);
  CvtTabItemToPc((struct item*)&x->items, 1);
  // scroll needs no conversion; all byte values
  CvtLongToPc(&x->age);
  CvtLongToPc(&x->exp);
  CvtShortToPc((short*)&x->load);
  CvtShortToPc((short*)&x->loadmax);
  CvtTabShortToPc((short*)x->money, 3);
  CvtBoolToPc(&x->hasturned);
  CvtBoolToPc(&x->canheal);
  CvtBoolToPc(&x->canidentify);
  CvtBoolToPc(&x->candetect);
  CvtBoolToPc(&x->toggle);
  CvtBoolToPc(&x->bleeding);
  CvtBoolToPc(&x->inbattle);
  CvtShortToPc(&x->verify3);
  CvtLongToPc(&x->damagetaken);
  CvtLongToPc(&x->damagegiven);
  CvtLongToPc(&x->hitsgiven);
  CvtLongToPc(&x->hitstaken);
  CvtLongToPc(&x->imissed);
  CvtLongToPc(&x->umissed);
  CvtLongToPc(&x->kills);
  CvtLongToPc(&x->deaths);
  CvtLongToPc(&x->knockouts);
  CvtLongToPc(&x->spellscast);
  CvtLongToPc(&x->destroyed);
  CvtLongToPc(&x->turns);
  CvtLongToPc(&x->prestigepenelty);

  CvtTabShortToPc((short*)x->definespells, 10 * 4);
  CvtShortToPc(&x->maxspellsattacks);
  CvtShortToPc(&x->spellsofar);
}

void CvtItemAttrToPc(struct itemattr* x) {
  CvtShortToPc(&x->st);
  CvtShortToPc(&x->itemid);
  CvtShortToPc(&x->iconid);
  CvtShortToPc(&x->type);
  CvtShortToPc(&x->blunt);
  CvtShortToPc(&x->nohands);
  CvtShortToPc(&x->lu);
  CvtShortToPc(&x->movement);
  CvtShortToPc(&x->ac);
  CvtShortToPc(&x->magres);
  CvtShortToPc(&x->damage);
  CvtShortToPc(&x->spellpoints);
  CvtShortToPc(&x->sound);
  CvtShortToPc(&x->wieght);
  CvtShortToPc(&x->cost);
  CvtShortToPc(&x->charge);
  CvtShortToPc(&x->iscurse);
  CvtShortToPc(&x->ismagical);
  CvtTabLongToPc(x->itemcat, 2);
  CvtShortToPc(&x->racerestrictions);
  CvtShortToPc(&x->casterestrictions);
  CvtShortToPc(&x->specificrace);
  CvtShortToPc(&x->specificcaste);
  CvtShortToPc(&x->raceclassonly);
  CvtShortToPc(&x->casteclassonly);
  CvtTabShortToPc(x->spare2, 7);
  CvtShortToPc(&x->vssmall);
  CvtShortToPc(&x->vslarge);
  CvtShortToPc(&x->heat);
  CvtShortToPc(&x->cold);
  CvtShortToPc(&x->electric);
  CvtShortToPc(&x->vsundead);
  CvtShortToPc(&x->vsdd);
  CvtShortToPc(&x->vsevil);
  CvtShortToPc(&x->sp1);
  CvtShortToPc(&x->sp2);
  CvtShortToPc(&x->sp3);
  CvtShortToPc(&x->sp4);
  CvtShortToPc(&x->sp5);
  CvtShortToPc(&x->xcharge);
  CvtShortToPc(&x->drop);
}

void CvtDoorToPc(struct door* x) {
  CvtLongToPc(&x->doorid);
  CvtTabShortToPc(x->code, 8);
  CvtTabShortToPc(x->id, 8);
}

void CvtMapsToPc(struct maps* x) {
  // This is all shorts except for the big Str255 at the end.
  CvtTabShortToPc(x, (sizeof(*x) - 256) / 2);
}

void CvtThiefToPc(struct thief* x) {
  CvtTabBoolToPc(x->type, 10);
  CvtTabShortToPc(x->modifer, 8);
  CvtTabShortToPc(x->codes, 8);
  CvtTabShortToPc(x->codef, 8);
  CvtTabShortToPc(x->texts, 8);
  CvtTabShortToPc(x->textf, 8);
  CvtTabShortToPc(x->sounds, 8);
  CvtTabShortToPc(x->soundf, 8);
  CvtShortToPc(&x->spell);
  CvtShortToPc(&x->lowdamage);
  CvtShortToPc(&x->highdamage);
  CvtShortToPc(&x->tumblers);
  CvtTabShortToPc(x->prompt, 3);
  CvtTabShortToPc(x->sound, 3);
}

void CvtRandLevelToPc(struct randlevel* x) {
  CvtTabRectToPc(x->randrect, 20);
  CvtTabShortToPc(x->percent, 20);
  CvtTabShortToPc((short*)x->battlerange, 20 * 2);
  CvtTabShortToPc((short*)x->randdoor, 20 * 3);
  CvtTabShortToPc((short*)x->randdoorpercent, 20 * 3);
  CvtBoolToPc(&x->isdark);
  CvtBoolToPc(&x->uselos);
  CvtTabBoolToPc(x->only, 20);
  CvtTabShortToPc(x->sound, 20);
  CvtTabShortToPc(x->text, 20);
}

void CvtMonsterToPc(struct monster* x) {
  CvtTabShortToPc(x->money, 3);
  CvtTabShortToPc(x->spells, 10);
  CvtTabShortToPc(x->items, 6);
  CvtShortToPc(&x->weapon);
  CvtShortToPc(&x->iconid);
  CvtShortToPc(&x->spellpoints);
  CvtShortToPc(&x->exp);
  CvtShortToPc(&x->stamina);
  CvtShortToPc(&x->staminamax);
  CvtTabShortToPc((short*)x->underneath, 2 * 2);
  CvtShortToPc(&x->todoondeath);
  CvtShortToPc(&x->maxspellpoints);
}

void CvtNoteToPc(struct note* x) {
  CvtLongToPc(&x->id);
  CvtBoolToPc(&x->isdung);
  CvtBoolToPc(&x->wasdark);
  CvtShortToPc(&x->x);
  CvtShortToPc(&x->y);
  CvtShortToPc(&x->level);
}

void CvtEncount2ToPc(struct encount2* x) {
  CvtTabShortToPc((short*)x->id, 4 * 8);
  CvtTabShortToPc(x->spellid, 10);
  CvtTabShortToPc(x->itemid, 5);
  CvtBoolToPc(&x->canbackout);
  CvtBoolToPc(&x->thief);
  CvtShortToPc(&x->prompt);
}

void CvtEncountToPc(struct encount* x) {
  CvtTabShortToPc((short*)x->id, 4 * 8);
  CvtBoolToPc(&x->canbackout);
  CvtShortToPc(&x->prompt);
}

void CvtBattleToPc(struct battle* x) {
  CvtTabShortToPc((short*)x->battle, 13 * 13);
  CvtShortToPc(&x->messagebefore);
  CvtShortToPc(&x->messageafter);
  CvtShortToPc(&x->battlemacro);
}

void CvtShopToPc(struct shop* x) {
  CvtTabShortToPc(x->id, 1000);
  CvtShortToPc(&x->inflation);
}

void CvtRestrictionInfoToPc(struct restrictinfo* x) {
  CvtShortToPc(&x->maxpc);
  CvtShortToPc(&x->maxlevel);
}

void CvtPrefsToPc(PrefRecord* x) {
  CvtShortToPc(&x->delayspeed);
  CvtShortToPc(&x->oldspeed);
  CvtShortToPc(&x->oldvolume);
  CvtShortToPc(&x->volume);
  CvtShortToPc(&x->defaultfont);
  CvtLongToPc(&x->serial);
  CvtShortToPc(&x->journalindex2);
  CvtShortToPc(&x->blank5);
  CvtShortToPc(&x->blank6);
  CvtShortToPc(&x->blank7);
  CvtShortToPc(&x->blank8);
  CvtShortToPc(&x->blank9);
  CvtShortToPc(&x->blank10);
}