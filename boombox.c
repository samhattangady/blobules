#include "boombox.h"
#include "game_settings.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void boombox_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    track_data* track = (track_data*) pDevice->pUserData;
    ma_decoder* pDecoder = &track->decoder;
    if (pDecoder == NULL) {
        return;
    }
    ma_uint64 frames = ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);
    track->current_frames += frames;
    if (frames < frameCount) {
        if (track->is_looping)
            ma_decoder_seek_to_pcm_frame(pDecoder, 0);
        else
            track->is_busy = false;

    }
    (void)pInput;
}

int init_boombox(boombox* b) {
    char* sound_files[SOUNDS_COUNT] = {"static/sound1.wav", "static/toto.wav"};
    sound_data* sounds = (sound_data*) malloc(sizeof(sound_data) * SOUNDS_COUNT);
    for (int i=0; i<SOUNDS_COUNT; i++) {
        size_t file_size;
        ma_decoder_config decoder_config;
        ma_decoder decoder;
        // TODO (27 Jun 2020 sam): We have to read the file twice here. Once in the init_file
        // phase, and once to get the actual data. See whether it is possible to read just once.
        // I don't know how to get the decoder config if we just read the memory...
        char* buffer = NULL;
        FILE* handler = fopen(sound_files[i], "r");
        fseek(handler, 0, SEEK_END);
        file_size = ftell(handler);
        rewind(handler);
        buffer = (char*) malloc(sizeof(char) * (file_size) );
        memset(buffer, 0, sizeof(char)*(file_size));
        fread(buffer, sizeof(char), file_size, handler);
        fclose(handler);
        ma_decoder_init_file(sound_files[i], NULL, &decoder);
        decoder_config = ma_decoder_config_init(decoder.outputFormat, decoder.outputChannels,
                                                decoder.outputSampleRate);
        sound_data s = {i, buffer, file_size, decoder_config};
        sounds[i] = s;
    }
    track_data* tracks = (track_data*) malloc(sizeof(track_data)*TOTAL_NUM_TRACKS);
    for (int i=0; i<TOTAL_NUM_TRACKS; i++) {
        tracks[i].is_busy = false;
    }
    b->sounds = sounds;
    b->total_tracks = TOTAL_NUM_TRACKS;
    b->tracks = tracks;
}

int play_sound(boombox* b, sound_type sound, bool is_looping) {
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
    sound_data s = b->sounds[sound];
    ma_decoder decoder;
    ma_device_config device_config;
    ma_device device;
    track_data track;
    ma_decoder_init_memory(s.buffer, s.size, &s.decoder_config, &decoder);
    device_config.playback.format   = decoder.outputFormat;
    device_config.playback.channels = decoder.outputChannels;
    device_config.sampleRate        = decoder.outputSampleRate;
    device_config.dataCallback      = boombox_callback;
    device_config.pUserData         = &track;
    ma_device_init(NULL, &device_config, &device);
    track.is_busy = true;
    track.is_looping = is_looping;
    track.decoder = decoder;
    track.device = device;
    track.total_frames = ma_decoder_get_length_in_pcm_frames(&decoder);
    track.current_frames = 0;
    track.start_seconds = 0.0;
    ma_device_start(&device);
    return 0;
}
