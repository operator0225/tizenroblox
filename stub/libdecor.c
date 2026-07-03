/*
 * libdecor-0.so.0 passthrough/fallback stub for Tizen TV
 *
 * Strategy (two-level):
 *   1. At startup, try to load the REAL libdecor from a system path.
 *      If found, ALL calls are forwarded to the real library — this gives
 *      SDL2 genuine xdg_surface/xdg_toplevel/decorations.
 *   2. If the real library is not installed on this Tizen TV, fall back to
 *      a minimal stub that fires a synthetic "fullscreen, 3840×2160" configure
 *      event immediately so SDL2 doesn't block indefinitely.
 *
 * Why we need this:
 *   Our lib/ dir is in LD_LIBRARY_PATH before the system paths, so any
 *   library we build shadows the system version. For libdecor specifically
 *   we WANT the system version when it exists (so SDL2 gets real xdg_shell
 *   protocol support). This wrapper achieves that while remaining a safe
 *   fallback for TVs without libdecor.
 *
 * Note: wayland-client.so.0 is also in our lib/ (stub); we intentionally
 *   try absolute paths for libdecor to avoid recursion.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Forward declarations ─────────────────────────────────────────────────── */

struct wl_display;
struct wl_surface;
struct wl_output;
struct xdg_surface;
struct xdg_toplevel;

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

typedef enum { LIBDECOR_ERROR_COMPOSITOR = 0 } libdecor_error;

struct libdecor;
struct libdecor_frame;
struct libdecor_state;
struct libdecor_configuration { int _dummy; };

typedef struct libdecor              libdecor_t;
typedef struct libdecor_frame        libdecor_frame_t;
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
    void (*reserved1)(void); void (*reserved2)(void);
    void (*reserved3)(void); void (*reserved4)(void);
    void (*reserved5)(void);
} libdecor_frame_interface_t;

/* ── Real libdecor delegation ─────────────────────────────────────────────── */

static void *g_real = NULL;   /* handle to real libdecor, or NULL */

/* Function pointers to real library */
static libdecor_t*       (*r_new)(struct wl_display*, const libdecor_interface_t*) = NULL;
static void              (*r_unref)(libdecor_t*)                                   = NULL;
static int               (*r_get_fd)(libdecor_t*)                                  = NULL;
static int               (*r_dispatch)(libdecor_t*, int)                           = NULL;
static libdecor_frame_t* (*r_decorate)(libdecor_t*, struct wl_surface*,
                                        const libdecor_frame_interface_t*,void*)   = NULL;
static void              (*r_frame_unref)(libdecor_frame_t*)                       = NULL;
static void              (*r_frame_set_title)(libdecor_frame_t*, const char*)      = NULL;
static void              (*r_frame_set_app_id)(libdecor_frame_t*, const char*)     = NULL;
static void              (*r_frame_set_fullscreen)(libdecor_frame_t*, struct wl_output*) = NULL;
static void              (*r_frame_unset_fullscreen)(libdecor_frame_t*)            = NULL;
static void              (*r_frame_set_maximized)(libdecor_frame_t*)               = NULL;
static void              (*r_frame_unset_maximized)(libdecor_frame_t*)             = NULL;
static void              (*r_frame_set_minimized)(libdecor_frame_t*)               = NULL;
static void              (*r_frame_set_capabilities)(libdecor_frame_t*, libdecor_capabilities) = NULL;
static void              (*r_frame_unset_capabilities)(libdecor_frame_t*, libdecor_capabilities) = NULL;
static void              (*r_frame_set_visibility)(libdecor_frame_t*, int)         = NULL;
static void              (*r_frame_set_min_content_size)(libdecor_frame_t*, int, int) = NULL;
static void              (*r_frame_set_max_content_size)(libdecor_frame_t*, int, int) = NULL;
static int               (*r_frame_is_floating)(libdecor_frame_t*)                 = NULL;
static int               (*r_frame_is_visible)(libdecor_frame_t*)                  = NULL;
static void              (*r_frame_commit)(libdecor_frame_t*,
                                           struct libdecor_state*,
                                           libdecor_configuration_t*)              = NULL;
static void              (*r_frame_popup_grab)(libdecor_frame_t*, const char*)     = NULL;
static void              (*r_frame_popup_ungrab)(libdecor_frame_t*, const char*)   = NULL;
static void              (*r_frame_translate)(libdecor_frame_t*, int, int, int*, int*) = NULL;
static void              (*r_frame_menu)(libdecor_frame_t*, void*, unsigned, int, int) = NULL;
static void              (*r_frame_resize)(libdecor_frame_t*, void*, unsigned, libdecor_resize_edge) = NULL;
static void              (*r_frame_move)(libdecor_frame_t*, void*, unsigned)       = NULL;
static const char*       (*r_frame_get_title)(libdecor_frame_t*)                   = NULL;
static struct xdg_surface*  (*r_frame_get_xdg_surface)(libdecor_frame_t*)          = NULL;
static struct xdg_toplevel* (*r_frame_get_xdg_toplevel)(libdecor_frame_t*)         = NULL;
static struct wl_surface*   (*r_frame_get_wl_surface)(libdecor_frame_t*)           = NULL;
static struct libdecor_state* (*r_state_new)(int, int)                             = NULL;
static void                   (*r_state_free)(struct libdecor_state*)              = NULL;
static int  (*r_cfg_get_size)(libdecor_configuration_t*, libdecor_frame_t*, int*, int*) = NULL;
static int  (*r_cfg_get_state)(libdecor_configuration_t*, libdecor_window_state*)  = NULL;

#define LOAD_SYM(h, name) r_##name = dlsym(h, "libdecor_" #name)
#define LOAD_SYM2(h, alias, sym) r_##alias = dlsym(h, sym)

__attribute__((constructor))
static void libdecor_stub_init(void)
{
    /* Allow forcing the fallback stub for Wayland compositors that don't
     * support xdg_shell (e.g. older Tizen Enlightenment builds). */
    if (getenv("TIZENROBLOX_FORCE_LIBDECOR_STUB")) {
        fprintf(stderr, "[libdecor-stub] TIZENROBLOX_FORCE_LIBDECOR_STUB set, "
                        "using built-in fullscreen fallback\n");
        return;
    }

    /* Try common Tizen / Linux system paths for the real libdecor */
    static const char * const paths[] = {
        "/usr/lib/aarch64-linux-gnu/libdecor-0.so.0",
        "/usr/lib64/libdecor-0.so.0",
        "/usr/lib/libdecor-0.so.0",
        "/lib/aarch64-linux-gnu/libdecor-0.so.0",
        "/usr/lib/tizen/libdecor-0.so.0",
        "/usr/share/tizen/libdecor-0.so.0",
        NULL
    };

    void *h = NULL;
    for (int i = 0; paths[i] && !h; i++)
        h = dlopen(paths[i], RTLD_LAZY | RTLD_LOCAL);

    if (!h) {
        fprintf(stderr, "[libdecor-stub] real libdecor not found, "
                        "using built-in fullscreen fallback\n");
        return;
    }

    fprintf(stderr, "[libdecor-stub] delegating to real libdecor\n");
    g_real = h;

    r_new           = dlsym(h, "libdecor_new");
    r_unref         = dlsym(h, "libdecor_unref");
    r_get_fd        = dlsym(h, "libdecor_get_fd");
    r_dispatch      = dlsym(h, "libdecor_dispatch");
    r_decorate      = dlsym(h, "libdecor_decorate");
    r_frame_unref   = dlsym(h, "libdecor_frame_unref");
    r_frame_set_title   = dlsym(h, "libdecor_frame_set_title");
    r_frame_set_app_id  = dlsym(h, "libdecor_frame_set_app_id");
    r_frame_set_fullscreen  = dlsym(h, "libdecor_frame_set_fullscreen");
    r_frame_unset_fullscreen= dlsym(h, "libdecor_frame_unset_fullscreen");
    r_frame_set_maximized   = dlsym(h, "libdecor_frame_set_maximized");
    r_frame_unset_maximized = dlsym(h, "libdecor_frame_unset_maximized");
    r_frame_set_minimized   = dlsym(h, "libdecor_frame_set_minimized");
    r_frame_set_capabilities   = dlsym(h, "libdecor_frame_set_capabilities");
    r_frame_unset_capabilities = dlsym(h, "libdecor_frame_unset_capabilities");
    r_frame_set_visibility  = dlsym(h, "libdecor_frame_set_visibility");
    r_frame_set_min_content_size = dlsym(h, "libdecor_frame_set_min_content_size");
    r_frame_set_max_content_size = dlsym(h, "libdecor_frame_set_max_content_size");
    r_frame_is_floating = dlsym(h, "libdecor_frame_is_floating");
    r_frame_is_visible  = dlsym(h, "libdecor_frame_is_visible");
    r_frame_commit      = dlsym(h, "libdecor_frame_commit");
    r_frame_popup_grab  = dlsym(h, "libdecor_frame_popup_grab");
    r_frame_popup_ungrab= dlsym(h, "libdecor_frame_popup_ungrab");
    r_frame_translate   = dlsym(h, "libdecor_frame_translate_coordinate");
    r_frame_menu        = dlsym(h, "libdecor_frame_show_window_menu");
    r_frame_resize      = dlsym(h, "libdecor_frame_resize");
    r_frame_move        = dlsym(h, "libdecor_frame_move");
    r_frame_get_title       = dlsym(h, "libdecor_frame_get_title");
    r_frame_get_xdg_surface = dlsym(h, "libdecor_frame_get_xdg_surface");
    r_frame_get_xdg_toplevel= dlsym(h, "libdecor_frame_get_xdg_toplevel");
    r_frame_get_wl_surface  = dlsym(h, "libdecor_frame_get_wl_surface");
    r_state_new  = dlsym(h, "libdecor_state_new");
    r_state_free = dlsym(h, "libdecor_state_free");
    r_cfg_get_size  = dlsym(h, "libdecor_configuration_get_content_size");
    r_cfg_get_state = dlsym(h, "libdecor_configuration_get_window_state");
}

/* ── Stub fallback structs ─────────────────────────────────────────────────── */

struct libdecor         { int fd; };
struct libdecor_frame   { const libdecor_frame_interface_t *iface; void *user_data; };
struct libdecor_state   { int width; int height; };

static struct libdecor_configuration g_config_singleton;

/* ── Public API ────────────────────────────────────────────────────────────── */

libdecor_t* libdecor_new(struct wl_display *display,
                          const libdecor_interface_t *iface)
{
    if (r_new) return r_new(display, iface);
    libdecor_t *ctx = calloc(1, sizeof(*ctx));
    ctx->fd = -1;
    return ctx;
}

void libdecor_unref(libdecor_t *ctx)
{
    if (r_unref) { r_unref(ctx); return; }
    free(ctx);
}

int libdecor_get_fd(libdecor_t *ctx)
{
    if (r_get_fd) return r_get_fd(ctx);
    return -1;
}

int libdecor_dispatch(libdecor_t *ctx, int timeout_ms)
{
    if (r_dispatch) return r_dispatch(ctx, timeout_ms);
    return 0;
}

libdecor_frame_t* libdecor_decorate(libdecor_t *ctx,
                                     struct wl_surface *surface,
                                     const libdecor_frame_interface_t *iface,
                                     void *user_data)
{
    if (r_decorate) return r_decorate(ctx, surface, iface, user_data);

    libdecor_frame_t *frame = calloc(1, sizeof(*frame));
    frame->iface     = iface;
    frame->user_data = user_data;

    /* Fire synthetic fullscreen configure immediately so SDL2 doesn't hang. */
    if (iface && iface->configure)
        iface->configure(frame, &g_config_singleton, user_data);

    return frame;
}

void libdecor_frame_unref(libdecor_frame_t *f)
{
    if (r_frame_unref) { r_frame_unref(f); return; }
    free(f);
}

void libdecor_frame_set_title(libdecor_frame_t *f, const char *t)
    { if (r_frame_set_title) r_frame_set_title(f, t); }
void libdecor_frame_set_app_id(libdecor_frame_t *f, const char *id)
    { if (r_frame_set_app_id) r_frame_set_app_id(f, id); }
void libdecor_frame_set_fullscreen(libdecor_frame_t *f, struct wl_output *o)
    { if (r_frame_set_fullscreen) r_frame_set_fullscreen(f, o); }
void libdecor_frame_unset_fullscreen(libdecor_frame_t *f)
    { if (r_frame_unset_fullscreen) r_frame_unset_fullscreen(f); }
void libdecor_frame_set_maximized(libdecor_frame_t *f)
    { if (r_frame_set_maximized) r_frame_set_maximized(f); }
void libdecor_frame_unset_maximized(libdecor_frame_t *f)
    { if (r_frame_unset_maximized) r_frame_unset_maximized(f); }
void libdecor_frame_set_minimized(libdecor_frame_t *f)
    { if (r_frame_set_minimized) r_frame_set_minimized(f); }
void libdecor_frame_set_capabilities(libdecor_frame_t *f, libdecor_capabilities c)
    { if (r_frame_set_capabilities) r_frame_set_capabilities(f, c); }
void libdecor_frame_unset_capabilities(libdecor_frame_t *f, libdecor_capabilities c)
    { if (r_frame_unset_capabilities) r_frame_unset_capabilities(f, c); }
void libdecor_frame_set_visibility(libdecor_frame_t *f, int v)
    { if (r_frame_set_visibility) r_frame_set_visibility(f, v); }
void libdecor_frame_set_min_content_size(libdecor_frame_t *f, int w, int h)
    { if (r_frame_set_min_content_size) r_frame_set_min_content_size(f, w, h); }
void libdecor_frame_set_max_content_size(libdecor_frame_t *f, int w, int h)
    { if (r_frame_set_max_content_size) r_frame_set_max_content_size(f, w, h); }
int libdecor_frame_is_floating(libdecor_frame_t *f)
    { return r_frame_is_floating ? r_frame_is_floating(f) : 0; }
int libdecor_frame_is_visible(libdecor_frame_t *f)
    { return r_frame_is_visible ? r_frame_is_visible(f) : 1; }
void libdecor_frame_commit(libdecor_frame_t *f, struct libdecor_state *s,
                            libdecor_configuration_t *c)
    { if (r_frame_commit) r_frame_commit(f, s, c); }
void libdecor_frame_popup_grab(libdecor_frame_t *f, const char *seat)
    { if (r_frame_popup_grab) r_frame_popup_grab(f, seat); }
void libdecor_frame_popup_ungrab(libdecor_frame_t *f, const char *seat)
    { if (r_frame_popup_ungrab) r_frame_popup_ungrab(f, seat); }
void libdecor_frame_translate_coordinate(libdecor_frame_t *f, int cx, int cy,
                                          int *fx, int *fy)
{
    if (r_frame_translate) { r_frame_translate(f, cx, cy, fx, fy); return; }
    if (fx) *fx = cx;
    if (fy) *fy = cy;
}
void libdecor_frame_show_window_menu(libdecor_frame_t *f, void *seat,
                                      unsigned serial, int x, int y)
    { if (r_frame_menu) r_frame_menu(f, seat, serial, x, y); }
void libdecor_frame_resize(libdecor_frame_t *f, void *seat, unsigned serial,
                             libdecor_resize_edge edge)
    { if (r_frame_resize) r_frame_resize(f, seat, serial, edge); }
void libdecor_frame_move(libdecor_frame_t *f, void *seat, unsigned serial)
    { if (r_frame_move) r_frame_move(f, seat, serial); }
const char* libdecor_frame_get_title(libdecor_frame_t *f)
    { return r_frame_get_title ? r_frame_get_title(f) : "Roblox"; }
struct xdg_surface* libdecor_frame_get_xdg_surface(libdecor_frame_t *f)
    { return r_frame_get_xdg_surface ? r_frame_get_xdg_surface(f) : NULL; }
struct xdg_toplevel* libdecor_frame_get_xdg_toplevel(libdecor_frame_t *f)
    { return r_frame_get_xdg_toplevel ? r_frame_get_xdg_toplevel(f) : NULL; }
struct wl_surface* libdecor_frame_get_wl_surface(libdecor_frame_t *f)
    { return r_frame_get_wl_surface ? r_frame_get_wl_surface(f) : NULL; }

/* ── libdecor_state ───────────────────────────────────────────────────────── */

struct libdecor_state* libdecor_state_new(int width, int height)
{
    if (r_state_new) return r_state_new(width, height);
    struct libdecor_state *s = calloc(1, sizeof(*s));
    s->width = width; s->height = height;
    return s;
}

void libdecor_state_free(struct libdecor_state *s)
{
    if (r_state_free) { r_state_free(s); return; }
    free(s);
}

/* ── libdecor_configuration ──────────────────────────────────────────────── */

int libdecor_configuration_get_content_size(libdecor_configuration_t *cfg,
                                              libdecor_frame_t *frame,
                                              int *width, int *height)
{
    if (r_cfg_get_size) return r_cfg_get_size(cfg, frame, width, height);
    /* Fallback: native 4K TV resolution */
    if (width)  *width  = 3840;
    if (height) *height = 2160;
    return 1;
}

int libdecor_configuration_get_window_state(libdecor_configuration_t *cfg,
                                              libdecor_window_state *state)
{
    if (r_cfg_get_state) return r_cfg_get_state(cfg, state);
    if (state) *state = LIBDECOR_WINDOW_STATE_FULLSCREEN;
    return 1;
}
