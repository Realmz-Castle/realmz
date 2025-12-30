#ifndef SoundManager_h
#define SoundManager_h

#include <SDL3/SDL_audio.h>

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
  badChannel = -205,
};

enum {
  quietCmd = 3,
  flushCmd = 4,
};

typedef struct {
  uint16_t cmd;
  int16_t param1;
  int32_t param2;
} SndCommand;

typedef struct {
  uint16_t qLength;
  SDL_AudioStream* sdlAudioStream;
} SndChannel;
typedef SndChannel* SndChannelPtr;

OSErr SndNewChannel(SndChannelPtr* chan, uint16_t synth, int32_t init, void* userRoutine);
OSErr SndDoImmediate(SndChannelPtr chan, const SndCommand* cmd);
OSErr SndPlay(SndChannelPtr chan, Handle sndHdl, Boolean async);

OSErr SetDefaultOutputVolume(uint32_t level);
#define SetSoundVol SetDefaultOutputVolume

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // SoundManager_h
