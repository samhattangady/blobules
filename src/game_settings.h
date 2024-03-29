#ifndef GAME_SETTINGS_DEFINED
#define GAME_SETTINGS_DEFINED

//#define WINDOW_WIDTH 1536
//#define WINDOW_HEIGHT 864
// #define WINDOW_WIDTH 1344
// #define WINDOW_HEIGHT 756
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define BLOCK_WIDTH 1.0/8.0
#define BLOCK_HEIGHT BLOCK_WIDTH * 200.0/300.0
#define LEVEL_SPRITE_SIZE 1/48.0

#define UI_WIDTH 400
#define TEXT_HEIGHT 18
#define UI_PADDING 8
#define X_PADDING -0.30
#define Y_PADDING -0.90
#define MAX_WORLD_ENTITIES 512
#define HISTORY_STEPS 128
#define ANIMATION_SINGLE_STEP_TIME 0.15
#define TOTAL_NUM_LEVELS 128
#define TOTAL_NUM_TRACKS 16
#define NUM_ANIMATION_FRAMES 32
#define ENTITY_NUM_ANIMATIONS 8
#define ANIMATION_FRAMES_PER_SECOND 40
#define ANIMATION_CLIP_LENGTH ANIMATION_FRAMES_PER_SECOND*ANIMATION_SINGLE_STEP_TIME
#define MAX_QUEUED_ANIMATIONS 8
#define SOUND_QUEUE_LENGTH 32
#define DEBUG_BUILD true
#define LEVEL_LINE_WIDTH 20.0/1920.0
#define LEVEL_LINE_HEIGHT LEVEL_LINE_WIDTH/3.0
#define LEVEL_LINE_SPACING 0.2
#define LEVEL_LINE_DRAW_TIME 2
#define INPUT_QUEUE_LENGTH 8
#define INPUT_DOWN_REPEAT 0.27
#define LEVEL_COMPLETION_ANIMATION_TIME 1.0
#define CONTROLS_POPUP_TIMER 5.0

#endif
