#ifndef COMMON_TYPES_DEFINED
#define COMMON_TYPES_DEFINED

typedef struct {
    bool l_pressed;
    bool l_released; // set for just the one tick where it was released
    bool r_pressed;
    bool r_released; // set for just the one tick where it was released
    float l_down_x;
    float l_down_y;
    float r_down_x;
    float r_down_y;
    float current_x;
    float current_y;
} mouse_data;

#endif
