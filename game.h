#ifndef BLOBULES_GAME_DEFINED
#define BLOBULES_GAME_DEFINED

typedef struct {
    float position[3];
    float target_position[3];
    float rotation[3];
    float seconds;
} game_state;

#endif
