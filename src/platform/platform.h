#pragma once
#include "defines.h"

typedef struct PlatformState {
    void* display;
    void* surface;
    uint32_t width;
    uint32_t height;
    bool isWindowClosed;
    bool isWindowResized;
    bool resizeSupportEnabled;
} PlatformState;

typedef enum KeyboardInputMap {
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_a, KEY_b, KEY_c, KEY_d, KEY_e, KEY_f, KEY_g, KEY_h, KEY_i, KEY_j, KEY_k, KEY_l, KEY_m, KEY_n, KEY_o, KEY_p, KEY_q, KEY_r, KEY_s, KEY_t, KEY_u, KEY_v, KEY_w, KEY_x, KEY_y, KEY_z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

    KEY_ESC,

    // KEY_ARROW_UP,
    // KEY_ARROW_DOWN,
    // KEY_ARROW_LEFT,
    // KEY_ARROW_RIGHT,

    // KEY_ENTER,
    // KEY_SPACE,
    // KEY_TAB,
    // KEY_SHIFT,

    KEY_INPUT_MAX
} KeyboardInputMap;

typedef union PointerInput {
    int ele[2];
    struct {
        int x, y;
    };
} PointerInput;

void platformInitialize(const char* windowTitle, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void platformPullEvent();
void platformShutdown();

PlatformState* platformGetPlatformState();

void platformConsoleWrite(const char* message, uint8_t color);
double platformGetTime();
void platformSleep(double time);

bool platformInputIsKeyDown(KeyboardInputMap key);
PointerInput platformInputPointerCurr();
PointerInput platformInputPointerRelative();

void platformPointerLock();
void platformPointerUnlock();
bool platformPointerIsLocked();
void platformPointerHide();
void platformPointerUnhide();

void platformWindowSetResizeSupport(bool enable);
void platformWindowSetFullScreen(bool fullscreen);
bool platformWindowIsFocused();

uint64_t platformThreadCreate(void*(*fun)(void*), void* arg);