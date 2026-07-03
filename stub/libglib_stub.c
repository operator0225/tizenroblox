/*
 * GLib / GObject minimal stub for Tizen TV
 *
 * Sober uses only 4 GLib symbols:
 *   GLib:    g_clear_error, g_str_has_prefix
 *   GObject: g_object_set, g_signal_connect_data
 *
 * Tries to delegate to the REAL system GLib via absolute paths (not SONAME)
 * to avoid circular self-loading via LD_LIBRARY_PATH shadowing.
 * Falls back to no-op/minimal implementations if the real library isn't found.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef void GError;
typedef void GObject;
typedef unsigned long gulong;
typedef int gboolean;

static void *glib_handle    = NULL;
static void *gobject_handle = NULL;

static void     (*r_g_clear_error)(GError**)                              = NULL;
static gboolean (*r_g_str_has_prefix)(const char*, const char*)           = NULL;
static void     (*r_g_object_set)(GObject*, const char*, ...)             = NULL;
static gulong   (*r_g_signal_connect_data)(GObject*, const char*, void*,
                                            void*, void*, unsigned)       = NULL;

static void* try_open_real(const char *name) {
    /* Use absolute paths to avoid re-loading ourselves via LD_LIBRARY_PATH */
    char path[256];
    static const char * const dirs[] = {
        "/usr/lib/aarch64-linux-gnu",
        "/usr/lib64",
        "/usr/lib",
        "/lib/aarch64-linux-gnu",
        "/lib",
        NULL
    };
    for (int i = 0; dirs[i]; i++) {
        snprintf(path, sizeof(path), "%s/%s", dirs[i], name);
        void *h = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
        if (h) return h;
    }
    return NULL;
}

__attribute__((constructor))
static void glib_stub_init(void) {
    glib_handle    = try_open_real("libglib-2.0.so.0");
    gobject_handle = try_open_real("libgobject-2.0.so.0");

    if (glib_handle) {
        r_g_clear_error    = dlsym(glib_handle, "g_clear_error");
        r_g_str_has_prefix = dlsym(glib_handle, "g_str_has_prefix");
    } else {
        fprintf(stderr, "[glib-stub] System GLib not found — using built-in no-ops\n");
    }

    if (gobject_handle) {
        r_g_object_set          = dlsym(gobject_handle, "g_object_set");
        r_g_signal_connect_data = dlsym(gobject_handle, "g_signal_connect_data");
    }
}

/* ── GLib exports ─────────────────────────────────────────────────────────── */

void g_clear_error(GError **err) {
    if (r_g_clear_error) { r_g_clear_error(err); return; }
    if (err) *err = NULL;
}

gboolean g_str_has_prefix(const char *str, const char *prefix) {
    if (r_g_str_has_prefix) return r_g_str_has_prefix(str, prefix);
    if (!str || !prefix) return 0;
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

/* ── GObject exports ──────────────────────────────────────────────────────── */

void g_object_set(GObject *object, const char *first_property_name, ...) {
    if (!r_g_object_set || !object) return;
    /* Forwarding variadic args is not possible directly; use va_list trick */
    va_list args;
    va_start(args, first_property_name);
    /* g_object_set_valist exists in GLib for exactly this purpose */
    typedef void (*g_object_set_valist_t)(GObject*, const char*, va_list);
    g_object_set_valist_t g_object_set_valist_fn =
        dlsym(gobject_handle, "g_object_set_valist");
    if (g_object_set_valist_fn) {
        g_object_set_valist_fn(object, first_property_name, args);
    }
    va_end(args);
}

gulong g_signal_connect_data(GObject *instance, const char *detailed_signal,
    void *c_handler, void *data, void *destroy_data, unsigned connect_flags) {
    if (r_g_signal_connect_data)
        return r_g_signal_connect_data(instance, detailed_signal,
                                        c_handler, data, destroy_data, connect_flags);
    return 0;
}
