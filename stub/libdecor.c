/*
 * libdecor stub for Tizen TV
 * libdecor은 Wayland 윈도우 데코레이션 라이브러리.
 * Tizen TV 전체화면에서는 데코레이션이 필요 없으므로 no-op 스텁으로 처리.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration;
struct wl_surface;
struct wl_display;

typedef struct libdecor libdecor_t;
typedef struct libdecor_frame libdecor_frame_t;

typedef struct libdecor_interface {
    void (*error)(libdecor_t *context, enum { LIBDECOR_ERROR_COMPOSITOR = 0 } error, const char *message);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
} libdecor_interface_t;

typedef struct libdecor_frame_interface {
    void (*configure)(libdecor_frame_t *frame, struct libdecor_configuration *config, void *user_data);
    void (*close)(libdecor_frame_t *frame, void *user_data);
    void (*commit)(libdecor_frame_t *frame, void *user_data);
    void (*dismiss_popup)(libdecor_frame_t *frame, const char *seat_name, void *user_data);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
    void (*reserved4)(void);
    void (*reserved5)(void);
} libdecor_frame_interface_t;

typedef enum {
    LIBDECOR_WINDOW_STATE_NONE       = 0,
    LIBDECOR_WINDOW_STATE_ACTIVE     = 1,
    LIBDECOR_WINDOW_STATE_MAXIMIZED  = 2,
    LIBDECOR_WINDOW_STATE_FULLSCREEN = 4,
    LIBDECOR_WINDOW_STATE_TILED_LEFT = 8,
    LIBDECOR_WINDOW_STATE_TILED_RIGHT = 16,
    LIBDECOR_WINDOW_STATE_TILED_TOP  = 32,
    LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 64,
} libdecor_window_state;

typedef enum {
    LIBDECOR_RESIZE_EDGE_NONE        = 0,
    LIBDECOR_RESIZE_EDGE_TOP         = 1,
    LIBDECOR_RESIZE_EDGE_BOTTOM      = 2,
    LIBDECOR_RESIZE_EDGE_LEFT        = 4,
    LIBDECOR_RESIZE_EDGE_TOP_LEFT    = 5,
    LIBDECOR_RESIZE_EDGE_BOTTOM_LEFT = 6,
    LIBDECOR_RESIZE_EDGE_RIGHT       = 8,
    LIBDECOR_RESIZE_EDGE_TOP_RIGHT   = 9,
    LIBDECOR_RESIZE_EDGE_BOTTOM_RIGHT = 10,
} libdecor_resize_edge;

typedef enum {
    LIBDECOR_CAPABILITIES_NONE           = 0,
    LIBDECOR_CAPABILITIES_ACTION_MOVE    = 1,
    LIBDECOR_CAPABILITIES_ACTION_RESIZE  = 2,
    LIBDECOR_CAPABILITIES_ACTION_MINIMIZE = 4,
    LIBDECOR_CAPABILITIES_ACTION_FULLSCREEN = 8,
    LIBDECOR_CAPABILITIES_ACTION_CLOSE   = 16,
} libdecor_capabilities;

/* Stub context - just a malloc'd blob */
struct libdecor {
    int fd;
};

struct libdecor_frame {
    int dummy;
    void *user_data;
};

struct libdecor_state {
    int width;
    int height;
};

/* Main API stubs */
libdecor_t* libdecor_new(struct wl_display *display, const libdecor_interface_t *iface) {
    (void)display; (void)iface;
    libdecor_t *ctx = calloc(1, sizeof(*ctx));
    ctx->fd = -1;
    return ctx;
}

void libdecor_unref(libdecor_t *context) {
    free(context);
}

int libdecor_get_fd(libdecor_t *context) {
    (void)context;
    return -1;
}

int libdecor_dispatch(libdecor_t *context, int timeout_ms) {
    (void)context; (void)timeout_ms;
    return 0;
}

libdecor_frame_t* libdecor_decorate(libdecor_t *context,
                                      struct wl_surface *surface,
                                      const libdecor_frame_interface_t *iface,
                                      void *user_data) {
    (void)context; (void)surface; (void)iface;
    libdecor_frame_t *frame = calloc(1, sizeof(*frame));
    frame->user_data = user_data;
    return frame;
}

void libdecor_frame_unref(libdecor_frame_t *frame) {
    free(frame);
}

void libdecor_frame_set_title(libdecor_frame_t *frame, const char *title) {
    (void)frame; (void)title;
}

void libdecor_frame_set_app_id(libdecor_frame_t *frame, const char *app_id) {
    (void)frame; (void)app_id;
}

void libdecor_frame_set_fullscreen(libdecor_frame_t *frame, void *output) {
    (void)frame; (void)output;
}

void libdecor_frame_unset_fullscreen(libdecor_frame_t *frame) {
    (void)frame;
}

int libdecor_frame_is_floating(libdecor_frame_t *frame) {
    (void)frame;
    return 0; /* always fullscreen on TV */
}

void libdecor_frame_set_capabilities(libdecor_frame_t *frame, libdecor_capabilities caps) {
    (void)frame; (void)caps;
}

void libdecor_frame_unset_capabilities(libdecor_frame_t *frame, libdecor_capabilities caps) {
    (void)frame; (void)caps;
}

void libdecor_frame_set_visibility(libdecor_frame_t *frame, int visible) {
    (void)frame; (void)visible;
}

int libdecor_frame_is_visible(libdecor_frame_t *frame) {
    (void)frame;
    return 1;
}

void libdecor_frame_commit(libdecor_frame_t *frame,
                            struct libdecor_state *state,
                            struct libdecor_configuration *config) {
    (void)frame; (void)state; (void)config;
}

struct libdecor_state* libdecor_state_new(int width, int height) {
    struct libdecor_state *s = calloc(1, sizeof(*s));
    s->width  = width;
    s->height = height;
    return s;
}

void libdecor_state_free(struct libdecor_state *state) {
    free(state);
}

int libdecor_configuration_get_content_size(struct libdecor_configuration *config,
                                              libdecor_frame_t *frame,
                                              int *width, int *height) {
    (void)config; (void)frame;
    /* Return native TV resolution */
    if (width)  *width  = 3840;
    if (height) *height = 2160;
    return 1;
}

int libdecor_configuration_get_window_state(struct libdecor_configuration *config,
                                              libdecor_window_state *window_state) {
    (void)config;
    if (window_state) *window_state = LIBDECOR_WINDOW_STATE_FULLSCREEN;
    return 1;
}

void libdecor_frame_set_min_content_size(libdecor_frame_t *frame, int w, int h) {
    (void)frame; (void)w; (void)h;
}

void libdecor_frame_set_max_content_size(libdecor_frame_t *frame, int w, int h) {
    (void)frame; (void)w; (void)h;
}

void libdecor_frame_popup_grab(libdecor_frame_t *frame, const char *seat_name) {
    (void)frame; (void)seat_name;
}

void libdecor_frame_popup_ungrab(libdecor_frame_t *frame, const char *seat_name) {
    (void)frame; (void)seat_name;
}

void libdecor_frame_translate_coordinate(libdecor_frame_t *frame,
                                          int content_x, int content_y,
                                          int *frame_x, int *frame_y) {
    (void)frame;
    if (frame_x) *frame_x = content_x;
    if (frame_y) *frame_y = content_y;
}

struct wl_surface* libdecor_frame_get_wl_surface(libdecor_frame_t *frame) {
    (void)frame;
    return NULL;
}

struct xdg_surface* libdecor_frame_get_xdg_surface(libdecor_frame_t *frame) {
    (void)frame;
    return NULL;
}

struct xdg_toplevel* libdecor_frame_get_xdg_toplevel(libdecor_frame_t *frame) {
    (void)frame;
    return NULL;
}

void libdecor_frame_show_window_menu(libdecor_frame_t *frame, void *seat, unsigned serial, int x, int y) {
    (void)frame; (void)seat; (void)serial; (void)x; (void)y;
}

void libdecor_frame_resize(libdecor_frame_t *frame, void *seat, unsigned serial, libdecor_resize_edge edge) {
    (void)frame; (void)seat; (void)serial; (void)edge;
}

void libdecor_frame_move(libdecor_frame_t *frame, void *seat, unsigned serial) {
    (void)frame; (void)seat; (void)serial;
}

const char* libdecor_frame_get_title(libdecor_frame_t *frame) {
    (void)frame;
    return "Roblox";
}
