#ifndef SoundManager_h
#define SoundManager_h

#include <SDL3/SDL_audio.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

SDL_AudioStream* SoundManager_new_audio_stream(void);
bool SoundManager_is_sound_registered(int16_t id);
void SoundManager_register_sound(int16_t id, uint32_t freq, void* data, uint32_t len);
void SoundManager_play_sound(SDL_AudioStream* sdlAudioStream, int16_t id);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // SoundManager_h
