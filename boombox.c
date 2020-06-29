#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "boombox.h"
#include "game_settings.h"

uint read_and_mix_pcm_frames(boombox* b, ma_decoder* decoder, float* output, uint frame_count) {
    uint channels = b->device.playback.channels;
    float buffer[4096];
    uint buffer_capacity = 4096 / channels;
    uint total_frames_read = 0;
    while (total_frames_read < frame_count) {
        uint frames_read;
        uint total_frames_remaining = frame_count - total_frames_read;
        uint frames_to_read = buffer_capacity;
        if (frames_to_read > total_frames_remaining)
            frames_to_read = total_frames_remaining;
        frames_read = (uint) ma_decoder_read_pcm_frames(decoder, buffer, frames_to_read);
        if (frames_read == 0)
            break;
        for (int i=0; i<frames_read*channels; i++) {
            output[total_frames_read*channels + i] += buffer[i] * 2.0 * (1.0+(float)(i%2));
        }
        total_frames_read += frames_read;
        if (frames_read < frames_to_read)
            break;
    }
    return total_frames_read;
}

void boombox_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count) {
    float* mix_output = (float*) output;
    boombox* b = (boombox*) device->pUserData;
    int track_index = -1;
    for (int i=0; i<b->total_tracks; i++) {
        if (b->tracks[i].is_playing) {
            uint frames_read = read_and_mix_pcm_frames(b, b->tracks[i].decoder, mix_output, frame_count);
            if (frames_read < frame_count) {
                b->tracks[i].is_busy = false;                
                b->tracks[i].is_playing = false;                
            }
        }
    }
    (void) input;
}

int init_boombox(boombox* b) {
    ma_result result;
    ma_device_config device_config;
    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format   = ma_format_f32;
    device_config.playback.channels = 0;
    device_config.sampleRate        = 0;
    device_config.dataCallback      = boombox_callback;
    device_config.pUserData         = b;
    result = ma_device_init(NULL, &device_config, &b->device);
    if (result != MA_SUCCESS) {
        printf("could not init device...\n");
        return -2;
    }
    char* sound_files[SOUNDS_COUNT] = {"static/sound1.wav", "static/toto.wav"};
    sound_data* sounds = (sound_data*) malloc(sizeof(sound_data) * SOUNDS_COUNT);
    memset(sounds, 0, sizeof(sound_data)*SOUNDS_COUNT);
    for (int i=0; i<SOUNDS_COUNT; i++) {
        size_t file_size;
        ma_decoder_config decoder_config;
        char* buffer = NULL;
        FILE* handler = fopen(sound_files[i], "r");
        fseek(handler, 0, SEEK_END);
        file_size = ftell(handler);
        rewind(handler);
        buffer = (char*) malloc(sizeof(char) * (file_size) );
        memset(buffer, 0, sizeof(char)*(file_size));
        fread(buffer, sizeof(char), file_size, handler);
        fclose(handler);
        decoder_config = ma_decoder_config_init(b->device.playback.format, b->device.playback.channels,
                                                b->device.sampleRate);
        sound_data s = {i, buffer, file_size, decoder_config};
        sounds[i] = s;
    }
    track_data* tracks = (track_data*) malloc(sizeof(track_data)*TOTAL_NUM_TRACKS);
    for (int i=0; i<TOTAL_NUM_TRACKS; i++) {
        tracks[i].is_busy = false;
    }
    // result = ma_device_start(&b->device);
    // if (result != MA_SUCCESS) {
    //     printf("could not start device...\n");
    //     return -2;
    // }
    b->is_playing = false;
    b->sounds = sounds;
    b->total_tracks = TOTAL_NUM_TRACKS;
    b->tracks = tracks;
    return 0;
}

int play_sound(boombox* b, sound_type sound, bool is_looping, float start_seconds) {
    ma_result result;
    int track_index = -1;
    for (int i=0; i<b->total_tracks; i++) {
        if (!b->tracks[i].is_busy) {
            track_index = i;
            break;
        }
    }
    if (track_index < 0) {
        // TODO (27 Jun 2020 sam): Implement malloc of more tracks here.
        printf("run out of tracks.\n");
        return -1;
    }
    printf("playing track: %i\n", track_index);
    sound_data s = b->sounds[sound];
    ma_decoder* decoder = (ma_decoder*) malloc(sizeof(ma_decoder));
    b->tracks[track_index].sound = sound;
    b->tracks[track_index].is_busy = true;
    b->tracks[track_index].is_playing = false;
    b->tracks[track_index].is_looping = is_looping;
    b->tracks[track_index].decoder = decoder;
    b->tracks[track_index].start_seconds = start_seconds;
    result = ma_decoder_init_memory(s.buffer, s.size, &s.decoder_config, b->tracks[track_index].decoder);
    if (result != MA_SUCCESS) {
        printf("could not init from memory...\n");
        return -2;
    }
    return 0;
}

int update_boombox(boombox* b, float seconds) {
    bool should_be_playing = false;    
    for (int i=0; i<b->total_tracks; i++) {
        if (b->tracks[i].is_busy) {
            if (seconds > b->tracks[i].start_seconds) {
                b->tracks[i].is_playing = true;
                should_be_playing = true;
            }
        }
    }
    // printf("updating boombox? should=%i is=%i\n", should_be_playing, b->is_playing);
    if (should_be_playing && !b->is_playing) {
        printf("starting device\n");
        ma_result result = ma_device_start(&b->device);
        if (result != MA_SUCCESS) {
            printf("could not start device\n");
            return -1;
        }
        b->is_playing = true;
    } else if (!should_be_playing && b->is_playing) {
        printf("stopping device\n");
        ma_result result = ma_device_stop(&b->device);
        if (result != MA_SUCCESS) {
            printf("could not stop device\n");
            return -1;
        }
        b->is_playing = false;
    }
    return 0;
}
