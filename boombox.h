#ifndef BOOMBOX_DEFINED
#define BOOMBOX_DEFINED

#include <stdbool.h>
#include "miniaudio.h"
#include "cb_lib/cb_types.h"

typedef enum {
    BEEP,
    MUSIC,
    SOUNDS_COUNT,
} sound_type;

typedef struct {
    sound_type sound;
    void* buffer;
    size_t size;
} sound_data;

typedef struct {
    sound_type sound;
    bool is_busy;
    bool is_looping;
    bool is_playing;
    ma_decoder decoder;
    float start_seconds;
} track_data;

typedef struct {
    ma_device device;
    ma_decoder_config decoder_config;
    bool is_playing;
    sound_data* sounds;
    uint total_tracks;
    track_data* tracks;
} boombox;

int init_boombox(boombox* b);
int play_sound(boombox* b, sound_type sound, bool is_looping, float start_seconds);
int update_boombox(boombox* b, float seconds);

#endif
