#include "emath.h"
#include "platform.h"
#include <pthread.h>

#ifdef USE_GLFW

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <time.h>

PlatformState globalStatePlatform;
struct {
    GLFWwindow* window;
    vec2 cursorLastPos;
    vec2 cursorCurrentPos;
} internalStatePlatform;

const uint32_t keyboard_input_glfw_map[KEY_INPUT_MAX] = {
    GLFW_KEY_A, GLFW_KEY_B, GLFW_KEY_C, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_F, GLFW_KEY_G, GLFW_KEY_H, GLFW_KEY_I, GLFW_KEY_J, GLFW_KEY_K, GLFW_KEY_L, GLFW_KEY_M, GLFW_KEY_N, GLFW_KEY_O, GLFW_KEY_P, GLFW_KEY_Q, GLFW_KEY_R, GLFW_KEY_S, GLFW_KEY_T, GLFW_KEY_U, GLFW_KEY_V, GLFW_KEY_W, GLFW_KEY_X, GLFW_KEY_Y, GLFW_KEY_Z,
    GLFW_KEY_A, GLFW_KEY_B, GLFW_KEY_C, GLFW_KEY_D, GLFW_KEY_E, GLFW_KEY_F, GLFW_KEY_G, GLFW_KEY_H, GLFW_KEY_I, GLFW_KEY_J, GLFW_KEY_K, GLFW_KEY_L, GLFW_KEY_M, GLFW_KEY_N, GLFW_KEY_O, GLFW_KEY_P, GLFW_KEY_Q, GLFW_KEY_R, GLFW_KEY_S, GLFW_KEY_T, GLFW_KEY_U, GLFW_KEY_V, GLFW_KEY_W, GLFW_KEY_X, GLFW_KEY_Y, GLFW_KEY_Z,
    GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,
    GLFW_KEY_ESCAPE
};

void glfw_window_size(GLFWwindow* window, int width, int height) {
    globalStatePlatform.width = width;
    globalStatePlatform.height = height;
    globalStatePlatform.isWindowResized = true;
}
void glfw_cursor_pos(GLFWwindow* window, double xpos, double ypos) {
    internalStatePlatform.cursorLastPos = internalStatePlatform.cursorCurrentPos;
    internalStatePlatform.cursorCurrentPos = (vec2){{xpos, ypos}};
}

void platformInitialize(const char* windowTitle, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (!glfwInit()) FATAL("Failed to initialize GLFW");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if ((internalStatePlatform.window = glfwCreateWindow(width, height, windowTitle, NULL, NULL)) == NULL) FATAL("Failed to create glfw window");
    globalStatePlatform.width = width;
    globalStatePlatform.height = height;
    globalStatePlatform.isWindowResizable = true;

    glfwSetWindowSizeCallback(internalStatePlatform.window, &glfw_window_size);
    glfwSetCursorPosCallback(internalStatePlatform.window, &glfw_cursor_pos);
}
VkSurfaceKHR platformCreateSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, internalStatePlatform.window, NULL, &surface) != VK_SUCCESS) FATAL("Failed to create window surface");
    return surface;
}
void platformPullEvent() {
    if (glfwWindowShouldClose(internalStatePlatform.window)) globalStatePlatform.isWindowClosed = true;
    glfwPollEvents();
}
void platformShutdown() {
    glfwDestroyWindow(internalStatePlatform.window);
    glfwTerminate();
}

PlatformState* platformGetPlatformState() {
    return &globalStatePlatform;
}

void platformConsoleWrite(const char* message, uint8_t color);
double platformGetTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}
void platformSleep(double time) {
    struct timespec ts;
    ts.tv_sec = (time_t)time;
    ts.tv_nsec = (time - ts.tv_sec) * 1e9;
    nanosleep(&ts, NULL);
}

bool platformInputIsKeyDown(KeyboardInputMap key) {
    return glfwGetKey(internalStatePlatform.window, keyboard_input_glfw_map[key]) == GLFW_PRESS;
}
vec2 platformInputPointerCurr() {
    double xpos, ypos;
    glfwGetCursorPos(internalStatePlatform.window, &xpos, &ypos);
    return (vec2){{xpos, ypos}};
}
vec2 platformInputPointerRelative() {
    vec2 ret = vec2_sub(internalStatePlatform.cursorCurrentPos, internalStatePlatform.cursorLastPos);
    internalStatePlatform.cursorLastPos = internalStatePlatform.cursorCurrentPos;
    return ret;
}

void platformPointerLock() {
    glfwSetInputMode(internalStatePlatform.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
void platformPointerUnlock() {
    glfwSetInputMode(internalStatePlatform.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
bool platformPointerIsLocked() {
    return glfwGetInputMode(internalStatePlatform.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}
void platformPointerHide() {
    glfwSetInputMode(internalStatePlatform.window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
}
void platformPointerUnhide() {
    glfwSetInputMode(internalStatePlatform.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void platformWindowSetResizable(bool enable) {
    glfwSetWindowAttrib(internalStatePlatform.window, GLFW_RESIZABLE, enable ? GLFW_TRUE : GLFW_FALSE);
    globalStatePlatform.isWindowResizable = enable;
}
void platformWindowSetFullScreen(bool fullscreen) {
    glfwSetWindowMonitor(internalStatePlatform.window, fullscreen ? glfwGetPrimaryMonitor() : NULL, 0, 0, globalStatePlatform.width, globalStatePlatform.height, GLFW_DONT_CARE);
}
bool platformWindowIsFocused() {
    return glfwGetWindowAttrib(internalStatePlatform.window, GLFW_FOCUSED) == GLFW_TRUE;
}

uint64_t platformThreadCreate(void*(*fun)(void*), void* arg) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, fun, arg) != 0) {
        FATAL("Failed to create thread");
    };
    return thread;
}

#endif