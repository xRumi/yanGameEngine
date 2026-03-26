#include "platform.h"

#ifdef __linux__

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-protocol.h"
#include "relative-pointer-unstable-v1.h"
#include "pointer-constraints-unstable-v1.h"
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-keysyms.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

PlatformState platformState;

struct {
    struct wl_display* wl_display;
    struct wl_compositor* wl_compositor;
    struct wl_surface* wl_surface;
    struct wl_shm* wl_shm;
    struct wl_shm_pool* wl_shm_pool;
    struct xdg_wm_base* xdg_wm_base;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct zxdg_decoration_manager_v1* zxdg_decoration_manager_v1;
    struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1;
    struct zwp_relative_pointer_manager_v1* zwp_relative_pointer_manager_v1;
    struct zwp_pointer_constraints_v1* zwp_pointer_constraints_v1;
    struct wl_seat* wl_seat;
    struct wl_keyboard* wl_keyboard;
    struct wl_pointer* wl_pointer;
    struct zwp_relative_pointer_v1* zwp_relative_pointer_v1;
    struct zwp_locked_pointer_v1* zwp_locked_pointer_v1;
    struct xkb_state* xkb_state;
    struct xkb_context* xkb_context;
    struct xkb_keymap* xkb_keymap;

    bool window_focused;
    bool keyboard_input_xkb[256];
    bool wl_pointer_inside_surface;
    bool hide_cursor;
    uint32_t wl_pointer_serial;
    PointerInput wl_pointer_last_input;
    PointerInput wl_relative_pointer_last_input;

} internalStatePlatform;

const uint32_t keyboard_input_xkb_map[KEY_INPUT_MAX] = {
    XKB_KEY_A, XKB_KEY_B, XKB_KEY_C, XKB_KEY_D, XKB_KEY_E, XKB_KEY_F, XKB_KEY_G, XKB_KEY_H, XKB_KEY_I, XKB_KEY_J, XKB_KEY_K, XKB_KEY_L, XKB_KEY_M, XKB_KEY_N, XKB_KEY_O, XKB_KEY_P, XKB_KEY_Q, XKB_KEY_R, XKB_KEY_S, XKB_KEY_T, XKB_KEY_U, XKB_KEY_V, XKB_KEY_W, XKB_KEY_X, XKB_KEY_Y, XKB_KEY_Z,
    XKB_KEY_a, XKB_KEY_b, XKB_KEY_c, XKB_KEY_d, XKB_KEY_e, XKB_KEY_f, XKB_KEY_g, XKB_KEY_h, XKB_KEY_i, XKB_KEY_j, XKB_KEY_k, XKB_KEY_l, XKB_KEY_m, XKB_KEY_n, XKB_KEY_o, XKB_KEY_p, XKB_KEY_q, XKB_KEY_r, XKB_KEY_s, XKB_KEY_t, XKB_KEY_u, XKB_KEY_v, XKB_KEY_w, XKB_KEY_x, XKB_KEY_y, XKB_KEY_z,
    XKB_KEY_0, XKB_KEY_1, XKB_KEY_2, XKB_KEY_3, XKB_KEY_4, XKB_KEY_5, XKB_KEY_6, XKB_KEY_7, XKB_KEY_8, XKB_KEY_9
};

static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        internalStatePlatform.wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        internalStatePlatform.wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 2);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        internalStatePlatform.xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        internalStatePlatform.zxdg_decoration_manager_v1 = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        internalStatePlatform.wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
    } else if (strcmp(interface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
        internalStatePlatform.zwp_relative_pointer_manager_v1 = wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
    } else if (strcmp(interface, zwp_pointer_constraints_v1_interface.name) == 0) {
        internalStatePlatform.zwp_pointer_constraints_v1 = wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);
    }
}
const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}
const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surface, serial);
    zxdg_toplevel_decoration_v1_set_mode(internalStatePlatform.zxdg_toplevel_decoration_v1, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}
const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure
};


void xdg_toplevel_configure(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height, struct wl_array* states) {

}
void xdg_toplevel_close(void* data, struct xdg_toplevel* xdg_toplevel) {
    platformState.platformWindowClosed = 1;
    TRACE("Window closed");
}
void xdg_toplevel_configure_bounds(void* data, struct xdg_toplevel* xdg_toplevel, int32_t width, int32_t height) {

}
void xdg_toplevel_wm_capabilities(void* data, struct xdg_toplevel* xdg_toplevel, struct wl_array* states) {

}
const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
    .configure_bounds = xdg_toplevel_configure_bounds,
    .wm_capabilities = xdg_toplevel_wm_capabilities
};

void wl_keyboard_keymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    if (format != XKB_KEYMAP_FORMAT_TEXT_V1) FATAL("XKB keymap only format v1 supported");
    const char* keymap_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (keymap_shm == MAP_FAILED) FATAL("XKB keymap shared memory map failed: %s", strerror(errno));
    xkb_state_unref(internalStatePlatform.xkb_state);
    xkb_keymap_unref(internalStatePlatform.xkb_keymap);
    internalStatePlatform.xkb_keymap = xkb_keymap_new_from_string(internalStatePlatform.xkb_context, keymap_shm, format, XKB_KEYMAP_COMPILE_NO_FLAGS);
    internalStatePlatform.xkb_state = xkb_state_new(internalStatePlatform.xkb_keymap);
}

void handle_wl_keyboard_input(uint32_t key, uint32_t state) {
    char buf[128];
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(internalStatePlatform.xkb_state, key + 8);
    xkb_keysym_get_name(keysym, buf, sizeof(buf));
    if (keysym >= CARRAY_SIZE(internalStatePlatform.keyboard_input_xkb)) {
        WARN("Unknown key (%s) %s", buf, state == WL_KEYBOARD_KEY_STATE_PRESSED ? "pressed" : "released");
        return;
    }
    internalStatePlatform.keyboard_input_xkb[keysym] = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
}

void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    internalStatePlatform.window_focused = true;
    uint32_t* key;
    wl_array_for_each(key, keys) {
        handle_wl_keyboard_input(*key, WL_KEYBOARD_KEY_STATE_PRESSED);
    }
    if (internalStatePlatform.zwp_locked_pointer_v1) platformPointerLock();
}
void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
    internalStatePlatform.window_focused = false;
    int keymapSize = CARRAY_SIZE(internalStatePlatform.keyboard_input_xkb);
    for (int i = 0; i < keymapSize; i++) internalStatePlatform.keyboard_input_xkb[i] = false;
}
void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    handle_wl_keyboard_input(key, state);
}
void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    xkb_state_update_mask(internalStatePlatform.xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, mods_locked);
}
void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
    // get key repeat info from system
}
const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};
void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    internalStatePlatform.wl_pointer_last_input = (PointerInput){{wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y)}};
    internalStatePlatform.wl_pointer_inside_surface = true;
    internalStatePlatform.wl_pointer_serial = serial;
    if (internalStatePlatform.hide_cursor) wl_pointer_set_cursor(internalStatePlatform.wl_pointer, internalStatePlatform.wl_pointer_serial, NULL, 0, 0);
    if (internalStatePlatform.zwp_locked_pointer_v1) platformPointerLock();
}
void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
    internalStatePlatform.wl_pointer_inside_surface = true;
}
void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    internalStatePlatform.wl_pointer_last_input = (PointerInput){{wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y)}};
}
void wl_pointer_button(void *data,
		       struct wl_pointer *wl_pointer,
		       uint32_t serial,
		       uint32_t time,
		       uint32_t button,
		       uint32_t state){}
void wl_pointer_frame(void *data,
		      struct wl_pointer *wl_pointer){}
void wl_pointer_axis(void *data,
		     struct wl_pointer *wl_pointer,
		     uint32_t time,
		     uint32_t axis,
		     wl_fixed_t value){}
void wl_pointer_axis_source(void *data,
			    struct wl_pointer *wl_pointer,
			    uint32_t axis_source){}
void wl_pointer_axis_stop(void *data,
			  struct wl_pointer *wl_pointer,
			  uint32_t time,
			  uint32_t axis){}
void wl_pointer_axis_discrete(void *data,
			      struct wl_pointer *wl_pointer,
			      uint32_t axis,
			      int32_t discrete){}
void wl_pointer_axis_value120(void *data,
			      struct wl_pointer *wl_pointer,
			      uint32_t axis,
			      int32_t value120){}
void wl_pointer_axis_relative_direction(void *data,
					struct wl_pointer *wl_pointer,
					uint32_t axis,
					uint32_t direction){}

const struct wl_pointer_listener wl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .frame = wl_pointer_frame,
    .axis_source = wl_pointer_axis_source,
    .axis_stop = wl_pointer_axis_stop,
    .axis_discrete = wl_pointer_axis_discrete,
    .axis_value120 = wl_pointer_axis_value120,
    .axis_relative_direction = wl_pointer_axis_relative_direction
};
void zwp_relative_pointer_v1_relative_motion(void *data, struct zwp_relative_pointer_v1 *zwp_relative_pointer_v1, uint32_t utime_hi, uint32_t utime_lo, wl_fixed_t dx, wl_fixed_t dy, wl_fixed_t dx_unaccel, wl_fixed_t dy_unaccel) {
    internalStatePlatform.wl_relative_pointer_last_input = (PointerInput){{wl_fixed_to_int(dx), wl_fixed_to_int(dy)}};
}
const struct zwp_relative_pointer_v1_listener zwp_relative_pointer_v1_listener = {
    .relative_motion = zwp_relative_pointer_v1_relative_motion
};
void wl_seat_name(void* data, struct wl_seat* wl_seat, const char* name) {
}
void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        internalStatePlatform.wl_keyboard = wl_seat_get_keyboard(internalStatePlatform.wl_seat);
        wl_keyboard_add_listener(internalStatePlatform.wl_keyboard, &wl_keyboard_listener, NULL);
    }
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        internalStatePlatform.wl_pointer = wl_seat_get_pointer(wl_seat);
        wl_pointer_add_listener(internalStatePlatform.wl_pointer, &wl_pointer_listener, NULL);
        internalStatePlatform.zwp_relative_pointer_v1 = zwp_relative_pointer_manager_v1_get_relative_pointer(internalStatePlatform.zwp_relative_pointer_manager_v1, internalStatePlatform.wl_pointer);
        zwp_relative_pointer_v1_add_listener(internalStatePlatform.zwp_relative_pointer_v1, &zwp_relative_pointer_v1_listener, NULL);
    }
}
const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name
};

int allocate_shm_file(uint64_t size) {
    char randomName[] = "/wl_shm-XXXXXXXXX";
    for (int i = 8; randomName[i]; i++)
        randomName[i] = 'A' + (rand() % 26);
    int fd = shm_open(randomName, O_CREAT | O_RDWR | O_EXCL, 0600);
    if (fd < 0) FATAL("shm_open: %s", strerror(errno));
    if (shm_unlink(randomName) < 0) FATAL("shm_unlink: %s", strerror(errno));
    if (ftruncate(fd, size) < 0) {
        close(fd);
        FATAL("ftruncate: %s", strerror(errno));
    }
    return fd;
}

void platformInitialize(const char *windowTitle, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (strcmp("wayland", getenv("XDG_SESSION_TYPE")) != 0) {
        FATAL("Session type wayland required");
    }
    if ((internalStatePlatform.wl_display = wl_display_connect(NULL)) == NULL) {
        FATAL("Failed to connect to wayland wl_display");
    }
    TRACE("Wayland connection established");

    struct wl_registry* registry = wl_display_get_registry(internalStatePlatform.wl_display);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    internalStatePlatform.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wl_display_roundtrip(internalStatePlatform.wl_display);

    internalStatePlatform.wl_surface = wl_compositor_create_surface(internalStatePlatform.wl_compositor);

    // xdg related
    xdg_wm_base_add_listener(internalStatePlatform.xdg_wm_base, &xdg_wm_base_listener, NULL);

    internalStatePlatform.xdg_surface = xdg_wm_base_get_xdg_surface(internalStatePlatform.xdg_wm_base, internalStatePlatform.wl_surface);
    xdg_surface_add_listener(internalStatePlatform.xdg_surface, &xdg_surface_listener, NULL);

    internalStatePlatform.xdg_toplevel = xdg_surface_get_toplevel(internalStatePlatform.xdg_surface);
    xdg_toplevel_add_listener(internalStatePlatform.xdg_toplevel, &xdg_toplevel_listener, NULL);
    xdg_toplevel_set_title(internalStatePlatform.xdg_toplevel, windowTitle);

    // zxdg decoration related
    internalStatePlatform.zxdg_toplevel_decoration_v1 = zxdg_decoration_manager_v1_get_toplevel_decoration(internalStatePlatform.zxdg_decoration_manager_v1, internalStatePlatform.xdg_toplevel);

    // wl_seat related
    wl_seat_add_listener(internalStatePlatform.wl_seat, &wl_seat_listener, NULL);

    wl_surface_commit(internalStatePlatform.wl_surface);

    platformState.display = internalStatePlatform.wl_display;
    platformState.surface = internalStatePlatform.wl_surface;
    platformState.width = width;
    platformState.height = height;

    platformPullEvent();
}

void platformPullEvent() {
    wl_display_dispatch_pending(internalStatePlatform.wl_display);
    wl_display_flush(internalStatePlatform.wl_display);
}

PlatformState* platformGetPlatformState() {
    return &platformState;
}

void platformShutdown() {
    wl_display_disconnect(internalStatePlatform.wl_display);
};

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
    return internalStatePlatform.keyboard_input_xkb[keyboard_input_xkb_map[key]];
}

PointerInput platformInputPointerCurr() {
    return internalStatePlatform.wl_pointer_last_input;
}
PointerInput platformInputPointerRelative() {
    PointerInput ret = internalStatePlatform.wl_pointer_inside_surface ? internalStatePlatform.wl_relative_pointer_last_input : (PointerInput){{}};
    internalStatePlatform.wl_relative_pointer_last_input = (PointerInput){{}};
    return ret;
}

void platformPointerLock() {
    if (!internalStatePlatform.zwp_locked_pointer_v1) internalStatePlatform.zwp_locked_pointer_v1 = zwp_pointer_constraints_v1_lock_pointer(internalStatePlatform.zwp_pointer_constraints_v1, internalStatePlatform.wl_surface, internalStatePlatform.wl_pointer, NULL, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);
}
void platformPointerUnlock() {
    if (internalStatePlatform.zwp_locked_pointer_v1) zwp_locked_pointer_v1_destroy(internalStatePlatform.zwp_locked_pointer_v1);
    internalStatePlatform.zwp_locked_pointer_v1 = NULL;
}
bool platformPointerIsLocked() {
    return internalStatePlatform.zwp_locked_pointer_v1 != NULL;
}

void platformPointerHide() {
    internalStatePlatform.hide_cursor = true;
    wl_pointer_set_cursor(internalStatePlatform.wl_pointer, internalStatePlatform.wl_pointer_serial, NULL, 0, 0);
}
void platformPointerUnhide() {
    internalStatePlatform.hide_cursor = false;
    // TODO: show cursor properly
}

bool platformWindowIsFocused() {
    return internalStatePlatform.window_focused;
}

uint64_t platformThreadCreate(void*(*fun)(void*), void* arg) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, fun, arg) != 0) {
        FATAL("Failed to create thread");
    };
    return thread;
}

#endif