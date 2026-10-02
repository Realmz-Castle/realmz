#ifndef MusicManager_h
#define MusicManager_h

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

Boolean RealmzMusicPlay(short playlist, const char* scenario_path);
void RealmzMusicStop(void);
void RealmzMusicSetVolume(short volume);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MusicManager_h
