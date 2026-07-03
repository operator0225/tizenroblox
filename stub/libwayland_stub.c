/*
 * libwayland-client.so.0 / libwayland-egl.so.1 / libwayland-cursor.so.0
 * passthrough shims for Tizen TV
 *
 * SDL2 (embedded in sober) probes these via dlopen. If the system has them
 * under a path not in the default search path, the launcher symlinks them
 * into our lib/ dir (see launch.sh: try_symlink_lib). This shim is only a
 * last-resort fallback to prevent crashes on systems without real Wayland
 * client libraries — it cannot provide actual Wayland connectivity.
 *
 * The actual rendering path requires the REAL libwayland-client.so.0 from
 * Tizen's Wayland (Enlightenment) installation.
 *
 * NOTE: This file is built as libwayland_stub.so and is NOT installed by
 * default. It only gets placed in lib/ if the launcher's try_symlink_lib
 * fails to find the real library. See CMakeLists.txt WAYLAND_STUB option.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Wayland opaque types */
typedef void wl_display;
typedef void wl_registry;
typedef void wl_compositor;
typedef void wl_surface;
typedef void wl_shell;
typedef void wl_shell_surface;
typedef void wl_output;
typedef void wl_seat;
typedef void wl_egl_window;

typedef unsigned int wl_fixed_t;
typedef void (*wl_dispatcher_func_t)(void*, void*, unsigned int, void*);

/* ── wl_display ──────────────────────────────────────────────────────────── */
wl_display* wl_display_connect(const char *name)
{
    /* Try absolute paths to avoid loading ourselves via SONAME */
    static const char * const paths[] = {
        "/usr/lib/aarch64-linux-gnu/libwayland-client.so.0",
        "/usr/lib64/libwayland-client.so.0",
        "/usr/lib/libwayland-client.so.0",
        "/lib/aarch64-linux-gnu/libwayland-client.so.0",
        NULL
    };
    void *h = NULL;
    for (int i = 0; paths[i] && !h; i++)
        h = dlopen(paths[i], RTLD_LAZY | RTLD_LOCAL);

    if (!h) {
        fprintf(stderr, "[wayland-stub] Real libwayland-client not found at "
                "any system path — Wayland display unavailable\n");
        return NULL;
    }
    typedef wl_display* (*fn_t)(const char*);
    fn_t real_fn = (fn_t)dlsym(h, "wl_display_connect");
    if (real_fn) return real_fn(name);
    return NULL;
}

void wl_display_disconnect(wl_display *display)       { (void)display; }
int  wl_display_get_fd(wl_display *display)           { (void)display; return -1; }
int  wl_display_dispatch(wl_display *display)         { (void)display; return -1; }
int  wl_display_dispatch_pending(wl_display *display) { (void)display; return -1; }
int  wl_display_flush(wl_display *display)            { (void)display; return -1; }
int  wl_display_roundtrip(wl_display *display)        { (void)display; return -1; }

wl_registry* wl_display_get_registry(wl_display *display) { (void)display; return NULL; }

/* ── wl_registry ────────────────────────────────────────────────────────── */
void  wl_registry_destroy(wl_registry *r) { (void)r; }
void* wl_registry_bind(wl_registry *r, unsigned int name,
                       const void *interface, unsigned int version)
{ (void)r; (void)name; (void)interface; (void)version; return NULL; }

int wl_registry_add_listener(wl_registry *r, const void *listener, void *data)
{ (void)r; (void)listener; (void)data; return -1; }

/* ── wl_compositor / wl_surface ─────────────────────────────────────────── */
wl_surface* wl_compositor_create_surface(wl_compositor *c) { (void)c; return NULL; }
void wl_compositor_destroy(wl_compositor *c) { (void)c; }
void wl_surface_destroy(wl_surface *s) { (void)s; }
void wl_surface_commit(wl_surface *s) { (void)s; }
void wl_surface_attach(wl_surface *s, void *buf, int x, int y)
    { (void)s; (void)buf; (void)x; (void)y; }
void wl_surface_damage(wl_surface *s, int x, int y, int w, int h)
    { (void)s; (void)x; (void)y; (void)w; (void)h; }
int wl_surface_add_listener(wl_surface *s, const void *listener, void *data)
    { (void)s; (void)listener; (void)data; return -1; }

/* ── wl_output ──────────────────────────────────────────────────────────── */
int wl_output_add_listener(wl_output *o, const void *listener, void *data)
    { (void)o; (void)listener; (void)data; return -1; }
void wl_output_destroy(wl_output *o) { (void)o; }

/* ── wl_seat ────────────────────────────────────────────────────────────── */
int wl_seat_add_listener(wl_seat *s, const void *listener, void *data)
    { (void)s; (void)listener; (void)data; return -1; }

/* ── wl_proxy (generic object) ───────────────────────────────────────────── */
void  wl_proxy_destroy(void *proxy) { (void)proxy; }
void* wl_proxy_marshal_flags(void *proxy, unsigned int opcode,
                              const void *interface, unsigned int version,
                              unsigned int flags, ...)
    { (void)proxy; (void)opcode; (void)interface; (void)version; (void)flags; return NULL; }
void  wl_proxy_marshal(void *proxy, unsigned int opcode, ...)
    { (void)proxy; (void)opcode; }
int   wl_proxy_add_listener(void *proxy, void (**implementation)(void),
                             void *data)
    { (void)proxy; (void)implementation; (void)data; return -1; }
int   wl_proxy_add_dispatcher(void *proxy, wl_dispatcher_func_t dispatcher,
                               const void *implementation, void *data)
    { (void)proxy; (void)dispatcher; (void)implementation; (void)data; return -1; }
unsigned int wl_proxy_get_version(void *proxy) { (void)proxy; return 0; }
unsigned int wl_proxy_get_id(void *proxy)      { (void)proxy; return 0; }
const char*  wl_proxy_get_class(void *proxy)   { (void)proxy; return ""; }
void         wl_proxy_set_user_data(void *proxy, void *data)
    { (void)proxy; (void)data; }
void*        wl_proxy_get_user_data(void *proxy) { (void)proxy; return NULL; }
void*        wl_proxy_create(void *factory, const void *interface)
    { (void)factory; (void)interface; return NULL; }

/* ── wl_egl_window (from libwayland-egl) ────────────────────────────────── */
wl_egl_window* wl_egl_window_create(wl_surface *surface, int width, int height)
    { (void)surface; (void)width; (void)height; return NULL; }
void wl_egl_window_destroy(wl_egl_window *w) { (void)w; }
void wl_egl_window_resize(wl_egl_window *w, int width, int height, int dx, int dy)
    { (void)w; (void)width; (void)height; (void)dx; (void)dy; }
void wl_egl_window_get_attached_size(wl_egl_window *w, int *width, int *height)
    { (void)w; if (width) *width = 3840; if (height) *height = 2160; }

/* ── wl_cursor (from libwayland-cursor) ─────────────────────────────────── */
typedef struct { unsigned int count; void **images; } wl_cursor_theme;
typedef struct { int width, height, hotspot_x, hotspot_y; } wl_cursor_image;
typedef struct { char *name; unsigned int image_count; wl_cursor_image **images; } wl_cursor;

wl_cursor_theme* wl_cursor_theme_load(const char *name, int size, void *shm)
    { (void)name; (void)size; (void)shm; return NULL; }
void wl_cursor_theme_destroy(wl_cursor_theme *t) { (void)t; }
wl_cursor* wl_cursor_theme_get_cursor(wl_cursor_theme *t, const char *name)
    { (void)t; (void)name; return NULL; }
void* wl_cursor_image_get_buffer(wl_cursor_image *image) { (void)image; return NULL; }
int   wl_cursor_frame(wl_cursor *cursor, unsigned int time)
    { (void)cursor; (void)time; return 0; }

/* ── Interface objects (required by SDL2 registry binding) ───────────────── */
/* These are extern data symbols SDL2 passes to wl_registry_bind */
typedef struct { const char *name; const void *implementation; unsigned int version; } wl_interface;

/* Minimal interface stubs — name string must match protocol */
const wl_interface wl_compositor_interface   = { "wl_compositor",   NULL, 4 };
const wl_interface wl_output_interface       = { "wl_output",       NULL, 3 };
const wl_interface wl_seat_interface         = { "wl_seat",         NULL, 5 };
const wl_interface wl_shell_interface        = { "wl_shell",        NULL, 1 };
const wl_interface wl_shell_surface_interface= { "wl_shell_surface",NULL, 1 };
const wl_interface wl_surface_interface      = { "wl_surface",      NULL, 4 };
const wl_interface wl_subcompositor_interface= { "wl_subcompositor",NULL, 1 };
const wl_interface wl_data_device_manager_interface = { "wl_data_device_manager", NULL, 3 };
const wl_interface xdg_wm_base_interface     = { "xdg_wm_base",     NULL, 2 };
const wl_interface xdg_surface_interface     = { "xdg_surface",     NULL, 2 };
const wl_interface xdg_toplevel_interface    = { "xdg_toplevel",    NULL, 2 };
