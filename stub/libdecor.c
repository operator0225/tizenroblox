/*
 * libdecor stub for Tizen TV
 *
 * libdecor is a Wayland client-side decoration library. SDL2 (embedded in
 * sober) probes for it and uses it to manage window decorations on Wayland.
 *
 * For Tizen TV (always fullscreen, no decorations), we stub the entire library
 * so SDL2 believes libdecor is present and the window is fullscreen from the
 * start. The critical correctness requirement is that libdecor_decorate()
 * IMMEDIATELY invokes the frame->configure callback, otherwise SDL2 blocks
 * forever waiting for the compositor to send a configure event.
 *
 * Since our libdecor stub does NOT connect to the Wayland compositor's
 * xdg_shell protocol, actual surface display depends on the real
 * libwayland-client handling xdg_wm_base. SDL2 falls back to direct
 * xdg_shell if libdecor's xdg_toplevel is NULL (which we return).
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Forward declare Wayland types without including wayland headers */
struct wl_surface;
struct wl_display;
struct wl_output;
struct xdg_surface;
struct xdg_toplevel;

/* ── Enums and structs ────────────────────────────────────────────────────── */

typedef enum {
    LIBDECOR_WINDOW_STATE_NONE         = 0,
    LIBDECOR_WINDOW_STATE_ACTIVE       = 1,
    LIBDECOR_WINDOW_STATE_MAXIMIZED    = 2,
    LIBDECOR_WINDOW_STATE_FULLSCREEN   = 4,
    LIBDECOR_WINDOW_STATE_TILED_LEFT   = 8,
    LIBDECOR_WINDOW_STATE_TILED_RIGHT  = 16,
    LIBDECOR_WINDOW_STATE_TILED_TOP    = 32,
    LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 64,
} libdecor_window_state;

typedef enum {
    LIBDECOR_RESIZE_EDGE_NONE         = 0,
    LIBDECOR_RESIZE_EDGE_TOP          = 1,
    LIBDECOR_RESIZE_EDGE_BOTTOM       = 2,
    LIBDECOR_RESIZE_EDGE_LEFT         = 4,
    LIBDECOR_RESIZE_EDGE_TOP_LEFT     = 5,
    LIBDECOR_RESIZE_EDGE_BOTTOM_LEFT  = 6,
    LIBDECOR_RESIZE_EDGE_RIGHT        = 8,
    LIBDECOR_RESIZE_EDGE_TOP_RIGHT    = 9,
    LIBDECOR_RESIZE_EDGE_BOTTOM_RIGHT = 10,
} libdecor_resize_edge;

typedef enum {
    LIBDECOR_CAPABILITIES_NONE            = 0,
    LIBDECOR_CAPABILITIES_ACTION_MOVE     = 1,
    LIBDECOR_CAPABILITIES_ACTION_RESIZE   = 2,
    LIBDECOR_CAPABILITIES_ACTION_MINIMIZE = 4,
    LIBDECOR_CAPABILITIES_ACTION_FULLSCREEN = 8,
    LIBDECOR_CAPABILITIES_ACTION_CLOSE    = 16,
} libdecor_capabilities;

typedef enum {
    LIBDECOR_ERROR_COMPOSITOR = 0,
} libdecor_error;

/* Opaque handles (must be defined, not just declared, to allow sizeof) */
struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration { int _dummy; };

typedef struct libdecor             libdecor_t;
typedef struct libdecor_frame       libdecor_frame_t;
typedef struct libdecor_configuration libdecor_configuration_t;

typedef struct {
    void (*error)(libdecor_t*, libdecor_error, const char*);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
} libdecor_interface_t;

typedef struct {
    void (*configure)(libdecor_frame_t*, libdecor_configuration_t*, void*);
    void (*close)(libdecor_frame_t*, void*);
    void (*commit)(libdecor_frame_t*, void*);
    void (*dismiss_popup)(libdecor_frame_t*, const char*, void*);
    void (*reserved1)(void);
    void (*reserved2)(void);
    void (*reserved3)(void);
    void (*reserved4)(void);
    void (*reserved5)(void);
} libdecor_frame_interface_t;

/* ── Internal structs ─────────────────────────────────────────────────────── */

struct libdecor {
    int  fd;          /* always -1; no real fd needed */
};

struct libdecor_frame {
    const libdecor_frame_interface_t *iface;
    void *user_data;
};

struct libdecor_state {
    int width;
    int height;
};

/* Singleton configuration returned during the synthetic configure event */
static struct libdecor_configuration g_config_singleton;

/* ── libdecor context ─────────────────────────────────────────────────────── */

libdecor_t* libdecor_new(struct wl_display *display,
                          const libdecor_interface_t *iface)
{
    (void)display; (void)iface;
    libdecor_t *ctx = calloc(1, sizeof(*ctx));
    ctx->fd = -1;
    return ctx;
}

void libdecor_unref(libdecor_t *context) { free(context); }

int libdecor_get_fd(libdecor_t *context)
{
    (void)context;
    return -1;
}

int libdecor_dispatch(libdecor_t *context, int timeout_ms)
{
    (void)context; (void)timeout_ms;
    return 0;
}

/* ── libdecor frame ───────────────────────────────────────────────────────── */

libdecor_frame_t* libdecor_decorate(libdecor_t *context,
                                     struct wl_surface *surface,
                                     const libdecor_frame_interface_t *iface,
                                     void *user_data)
{
    (void)context; (void)surface;

    libdecor_frame_t *frame = calloc(1, sizeof(*frame));
    frame->iface     = iface;
    frame->user_data = user_data;

    /*
     * SDL2 blocks after libdecor_decorate(), waiting for the compositor to
     * send a configure event (via libdecor_dispatch → iface->configure).
     * Since we have no real compositor connection, fire the configure callback
     * immediately with a "fullscreen, 3840×2160" synthetic event.
     *
     * iface may be NULL if caller doesn't set a listener.
     */
    if (iface && iface->configure) {
        iface->configure(frame, &g_config_singleton, user_data);
    }

    return frame;
}

void libdecor_frame_unref(libdecor_frame_t *frame) { free(frame); }

/* Window properties — no-ops for a TV (always fullscreen) */
void libdecor_frame_set_title(libdecor_frame_t *f, const char *t)
    { (void)f; (void)t; }
void libdecor_frame_set_app_id(libdecor_frame_t *f, const char *id)
    { (void)f; (void)id; }
void libdecor_frame_set_fullscreen(libdecor_frame_t *f, struct wl_output *o)
    { (void)f; (void)o; }
void libdecor_frame_unset_fullscreen(libdecor_frame_t *f) { (void)f; }
void libdecor_frame_set_minimized(libdecor_frame_t *f) { (void)f; }
void libdecor_frame_set_maximized(libdecor_frame_t *f) { (void)f; }
void libdecor_frame_unset_maximized(libdecor_frame_t *f) { (void)f; }
void libdecor_frame_set_capabilities(libdecor_frame_t *f, libdecor_capabilities c)
    { (void)f; (void)c; }
void libdecor_frame_unset_capabilities(libdecor_frame_t *f, libdecor_capabilities c)
    { (void)f; (void)c; }
void libdecor_frame_set_visibility(libdecor_frame_t *f, int v) { (void)f; (void)v; }
void libdecor_frame_set_min_content_size(libdecor_frame_t *f, int w, int h)
    { (void)f; (void)w; (void)h; }
void libdecor_frame_set_max_content_size(libdecor_frame_t *f, int w, int h)
    { (void)f; (void)w; (void)h; }

int libdecor_frame_is_floating(libdecor_frame_t *f) { (void)f; return 0; }
int libdecor_frame_is_visible(libdecor_frame_t *f)  { (void)f; return 1; }

void libdecor_frame_commit(libdecor_frame_t *f,
                            struct libdecor_state *s,
                            libdecor_configuration_t *c)
    { (void)f; (void)s; (void)c; }

void libdecor_frame_popup_grab(libdecor_frame_t *f, const char *seat)
    { (void)f; (void)seat; }
void libdecor_frame_popup_ungrab(libdecor_frame_t *f, const char *seat)
    { (void)f; (void)seat; }
void libdecor_frame_translate_coordinate(libdecor_frame_t *f,
                                          int cx, int cy, int *fx, int *fy)
{
    (void)f;
    if (fx) *fx = cx;
    if (fy) *fy = cy;
}

void libdecor_frame_show_window_menu(libdecor_frame_t *f, void *seat,
                                      unsigned serial, int x, int y)
    { (void)f; (void)seat; (void)serial; (void)x; (void)y; }
void libdecor_frame_resize(libdecor_frame_t *f, void *seat,
                             unsigned serial, libdecor_resize_edge edge)
    { (void)f; (void)seat; (void)serial; (void)edge; }
void libdecor_frame_move(libdecor_frame_t *f, void *seat, unsigned serial)
    { (void)f; (void)seat; (void)serial; }

const char* libdecor_frame_get_title(libdecor_frame_t *f)
    { (void)f; return "Roblox"; }

/*
 * SDL2 calls these to get the underlying xdg_surface/xdg_toplevel so it can
 * call xdg_toplevel_set_fullscreen(). We return NULL because we don't
 * implement the xdg_shell protocol. SDL2 falls back to a no-op for fullscreen
 * in this case, but rendering still works via the wl_surface + EGL path.
 */
struct xdg_surface* libdecor_frame_get_xdg_surface(libdecor_frame_t *f)
    { (void)f; return NULL; }
struct xdg_toplevel* libdecor_frame_get_xdg_toplevel(libdecor_frame_t *f)
    { (void)f; return NULL; }
struct wl_surface* libdecor_frame_get_wl_surface(libdecor_frame_t *f)
    { (void)f; return NULL; }

/* ── libdecor_state ───────────────────────────────────────────────────────── */

struct libdecor_state* libdecor_state_new(int width, int height)
{
    struct libdecor_state *s = calloc(1, sizeof(*s));
    s->width  = width;
    s->height = height;
    return s;
}

void libdecor_state_free(struct libdecor_state *s) { free(s); }

/* ── libdecor_configuration ──────────────────────────────────────────────── */

/*
 * Called by SDL2 inside the configure callback to learn the window size and
 * state. We report 3840×2160 fullscreen — the native resolution of
 * KQ83SF95AEXKR (83" OLED, 4K).
 */
int libdecor_configuration_get_content_size(libdecor_configuration_t *config,
                                              libdecor_frame_t *frame,
                                              int *width, int *height)
{
    (void)config; (void)frame;
    if (width)  *width  = 3840;
    if (height) *height = 2160;
    return 1; /* success */
}

int libdecor_configuration_get_window_state(libdecor_configuration_t *config,
                                              libdecor_window_state *window_state)
{
    (void)config;
    if (window_state) *window_state = LIBDECOR_WINDOW_STATE_FULLSCREEN;
    return 1; /* success */
}
