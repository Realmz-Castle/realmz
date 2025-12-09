#ifndef CONVERT_H
#define CONVERT_H

#include <SDL3/SDL_endian.h>
#include <stdint.h>

#include "structs.h"

// We need slow swap routines for misaligned structs.
#ifdef __BIG_ENDIAN__
#define SLOWSWAP_BIG16(x)
#define SLOWSWAP_BIG32(x)
#else

#define SWAP_BIG16(x) (SDL_Swap16BE(x))
#define SWAP_BIG32(x) (SDL_Swap32BE(x))

static inline void SLOWSWAP_BIG16(int16_t* x) {
  uint8_t* p = (uint8_t*)x;
  uint8_t a = p[0], b = p[1];
  p[0] = b;
  p[1] = a;
}
static inline void SLOWSWAP_BIG32(int32_t* x) {
  uint8_t* p = (uint8_t*)x;
  uint8_t a = p[0], b = p[1], c = p[2], d = p[3];
  p[0] = d;
  p[1] = c;
  p[2] = b;
  p[3] = a;
}
#endif

static inline void rintel2moto(Rect* r) {
  r->top = SWAP_BIG16(r->top);
  r->left = SWAP_BIG16(r->left);
  r->bottom = SWAP_BIG16(r->bottom);
  r->right = SWAP_BIG16(r->right);
}

static inline void CvtShortToPc(int16_t* x) { SLOWSWAP_BIG16(x); }
static inline void CvtLongToPc(int32_t* x) { SLOWSWAP_BIG32(x); }
static inline void CvtFloatToPc(float* x) { SLOWSWAP_BIG32((int32_t*)x); }

static inline void CvtBoolToPc(Boolean* x) {
  if (sizeof(Boolean) == 2)
    SLOWSWAP_BIG16((int16_t*)x);
  else if (sizeof(Boolean) == 4)
    SLOWSWAP_BIG32((int32_t*)x);
}

static inline void CvtRectToPc(Rect* x) {
  rintel2moto(x);
}

static inline void CvtTabLongToPc(int32_t* x, unsigned int count) {
  while (count--)
    CvtLongToPc(x++);
}

static inline void CvtTabShortToPc(int16_t* x, unsigned int count) {
  while (count--)
    CvtShortToPc(x++);
}

static inline void CvtTabBoolToPc(Boolean* x, unsigned int count) {
  while (count--)
    CvtBoolToPc(x++);
}

static inline void CvtTabRectToPc(Rect* x, unsigned int count) {
  while (count--)
    rintel2moto(x++);
}

// These structs are all shorts, and can be treated as an array.
#define CVT_ALL_SHORTS(x) CvtTabShortToPc((short*)(x), (sizeof(*(x)) / 2))

static inline void CvtMapStatToPc(struct mapstats* x) {
  CVT_ALL_SHORTS(x);
}
static inline void CvtTimeEncounterToPc(struct timeencounter* x) {
  CVT_ALL_SHORTS(x);
}
static inline void CvtTreasureToPc(struct treasure* x) {
  CVT_ALL_SHORTS(x);
}

void CvtItemAttrToPc(struct itemattr* x);
void CvtItemToPc(struct item* x);
void CvtDoorToPc(struct door* x);
void CvtMapsToPc(struct maps* x);
void CvtRaceToPc(struct race* x);
void CvtCasteToPc(struct caste* x);
void CvtThiefToPc(struct thief* x);
void CvtRandLevelToPc(struct randlevel* x);
void CvtCharacterToPc(struct character* x);
void CvtMonsterToPc(struct monster* x);
void CvtNoteToPc(struct note* x);
void CvtEncountToPc(struct encount* x);
void CvtEncount2ToPc(struct encount2* x);
void CvtBattleToPc(struct battle* x);
void CvtShopToPc(struct shop* x);
void CvtRestrictionInfoToPc(struct restrictinfo* x);
void CvtPrefsToPc(PrefRecord* x);

static inline void CvtTabItemAttrToPc(struct itemattr* x, unsigned int count) {
  while (count--)
    CvtItemAttrToPc(x++);
}

static inline void CvtTabItemToPc(struct item* x, unsigned int count) {
  // Inexplicably, this means a count of 30 items.
  count *= 30;

  while (count--)
    CvtItemToPc(x++);
}

static inline void CvtTabDoorToPc(struct door* x, unsigned int count) {
  while (count--)
    CvtDoorToPc(x++);
}

static inline void CvtTabMonsterToPc(struct monster* x, unsigned int count) {
  while (count--)
    CvtMonsterToPc(x++);
}

static inline void CvtTabCharacterToPc(struct character* x, unsigned int count) {
  while (count--)
    CvtCharacterToPc(x++);
}

static inline void CvtTabMapStatToPc(struct mapstats* x, unsigned int count) {
  while (count--)
    CvtMapStatToPc(x++);
}

/* *** CHANGED FROM ORIGINAL IMPLEMENTATION ***
 * NOTE(jpetrie): CvtTabSpellToPc and CvtContactToPc were originally just #define'd to nothing as they are no longer
 * neccessary. That generates warnings because they're used as macros in the code, which leaves the expressions within
 * the macro behind. They have been changed to take and ignore parameters, which has the same effect without warnings.
 */

// Spells are already composed solely of bytes.
#define CvtTabSpellToPc(spelldata, spellid)

// contactdata is a bunch of static strings.
#define CvtContactToPc(contactdata)

// These are just arrays of shorts.
#define CvtFieldToPc(x) CvtTabShortToPc(x, 90 * 90)
#define CvtLayoutToPc(x) CvtTabShortToPc(x, 8 * 16)

#endif // CONVERT_H
