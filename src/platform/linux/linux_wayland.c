#include "platform.h"

#ifdef __linux__

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-protocol.h"
#include <xkbcommon/xkbcommon.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

PlatformState platformState;

struct {
    struct wl_display* wl_display;
    struct wl_compositor* compositor;
    struct wl_surface* wl_surface;
    struct wl_shm* wl_shm;
    struct wl_shm_pool* wl_shm_pool;
    struct xdg_wm_base* xdg_wm_base;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct zxdg_decoration_manager_v1* zxdg_decoration_manager_v1;
    struct zxdg_toplevel_decoration_v1* zxdg_toplevel_decoration_v1;
    struct wl_seat* wl_seat;
    struct wl_keyboard* wl_keyboard;
    struct xkb_state* xkb_state;
    struct xkb_context* xkb_context;
    struct xkb_keymap* xkb_keymap;
} internalStatePlatform;

static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    //TRACE("interface: '%s', version: %d, name: %d", interface, version, name);
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        internalStatePlatform.compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        internalStatePlatform.wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, 2);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        internalStatePlatform.xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        internalStatePlatform.zxdg_decoration_manager_v1 = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
    else if (strcmp(interface, wl_seat_interface.name) == 0) {
        internalStatePlatform.wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
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
void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    uint32_t* key;
    wl_array_for_each(key, keys) {
        char buf[128];
        xkb_keysym_t keysym = xkb_state_key_get_one_sym(internalStatePlatform.xkb_state, *key + 8);
        xkb_keysym_get_name(keysym, buf, sizeof(buf));
        DEBUG("keyboard enter sym: %s (%d)", buf, keysym);
    }
}
void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {

}
void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    char buf[128];
    xkb_keysym_t keysym = xkb_state_key_get_one_sym(internalStatePlatform.xkb_state, key + 8);
    xkb_keysym_get_name(keysym, buf, sizeof(buf));
    DEBUG("sym: %s (%d)", buf, keysym);
}
void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    xkb_state_update_mask(internalStatePlatform.xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, mods_locked);
}
void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
    // get system wise keyboard info
}
const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info
};
void wl_seat_name(void* data, struct wl_seat* wl_seat, const char* name) {
}
void wl_seat_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
    bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;
    if (have_keyboard && internalStatePlatform.wl_keyboard == NULL) {
        internalStatePlatform.wl_keyboard = wl_seat_get_keyboard(internalStatePlatform.wl_seat);
        wl_keyboard_add_listener(internalStatePlatform.wl_keyboard, &wl_keyboard_listener, NULL);
    } else if (!have_keyboard && internalStatePlatform.wl_keyboard != NULL) {
        wl_keyboard_release(internalStatePlatform.wl_keyboard);
        internalStatePlatform.wl_keyboard = NULL;
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

void platformInitialize(const char *applicationName, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
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

    internalStatePlatform.wl_surface = wl_compositor_create_surface(internalStatePlatform.compositor);

    // xdg related
    xdg_wm_base_add_listener(internalStatePlatform.xdg_wm_base, &xdg_wm_base_listener, NULL);

    internalStatePlatform.xdg_surface = xdg_wm_base_get_xdg_surface(internalStatePlatform.xdg_wm_base, internalStatePlatform.wl_surface);
    xdg_surface_add_listener(internalStatePlatform.xdg_surface, &xdg_surface_listener, NULL);

    internalStatePlatform.xdg_toplevel = xdg_surface_get_toplevel(internalStatePlatform.xdg_surface);
    xdg_toplevel_add_listener(internalStatePlatform.xdg_toplevel, &xdg_toplevel_listener, NULL);
    xdg_toplevel_set_title(internalStatePlatform.xdg_toplevel, "yanGameEngine - Triangle");

    // zxdg decoration related
    internalStatePlatform.zxdg_toplevel_decoration_v1 = zxdg_decoration_manager_v1_get_toplevel_decoration(internalStatePlatform.zxdg_decoration_manager_v1, internalStatePlatform.xdg_toplevel);

    // wl_seat related
    wl_seat_add_listener(internalStatePlatform.wl_seat, &wl_seat_listener, NULL);

    wl_surface_commit(internalStatePlatform.wl_surface);

    platformState.display = internalStatePlatform.wl_display;
    platformState.surface = internalStatePlatform.wl_surface;
    platformState.width = width;
    platformState.height = height;
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

uint64_t platformThreadCreate(void*(*fun)(void*), void* arg) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, fun, arg) != 0) {
        FATAL("Failed to create thread");
    };
    return thread;
}

#endif