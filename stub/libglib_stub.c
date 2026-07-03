/*
 * libglib-2.0.so.0 / libgobject-2.0.so.0 for Tizen TV — real GLib 2.82.2,
 * statically bundled.
 *
 * Sober only calls 4 GLib/GObject symbols (g_clear_error, g_str_has_prefix,
 * g_object_set, g_signal_connect_data), but rather than hand-reimplement
 * them we statically link the real libglib-2.0.a / libgobject-2.0.a
 * (cross-compiled by scripts/build_thirdparty.sh, built against our
 * bundled libffi + pcre2 + zlib) into the two shared objects. Every real
 * GLib/GObject symbol is exported directly — no dlopen.
 */

#include <stdio.h>

__attribute__((constructor))
static void glib_bundled_init(void) {
    fprintf(stderr, "[glib] Using statically bundled GLib 2.82.2\n");
}
