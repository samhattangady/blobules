#ifndef BOOMBOX_DEFINED
#define BOOMBOX

#include "cb_lib/cb_types.h"
#include <stdbool.h>

#include "miniaudio.h"

typedef enum {
    BEEP,
    MUSIC,
    SOUNDS_COUNT,
} sound_type;

typedef struct {
    sound_type sound;
    void* buffer;
    size_t size;
    ma_decoder_config decoder_config;
} sound_data;

typedef struct {
    bool is_busy;
    bool is_looping;
    ma_decoder decoder;
    ma_device device;
    ma_uint64 total_frames;
    ma_uint64 current_frames;
    float start_seconds;
} track_data;

typedef struct {
    sound_data* sounds;
    uint total_tracks;
    track_data* tracks;
} boombox;

int init_boombox(boombox* b);
int play_sound(boombox* b, sound_type sound, bool is_looping);

#endif
