/*
 * libdbus-1.so.3 delegating shim for Tizen TV
 *
 * Sober uses D-Bus for session management and IPC (22 symbols).
 * Tizen 9.0 should have D-Bus, but it may be in a non-standard path.
 *
 * This shim tries to load the real libdbus-1.so.3 from absolute paths.
 * Falls back to minimal no-ops that prevent hard crashes (but D-Bus
 * communication won't work — sober will still start, IPC features disabled).
 *
 * Symbols used by sober (22):
 *   dbus_bus_add_match, dbus_bus_get,
 *   dbus_connection_flush, dbus_connection_pop_message,
 *   dbus_connection_read_write, dbus_connection_send_with_reply_and_block,
 *   dbus_connection_unref,
 *   dbus_error_free, dbus_error_init, dbus_error_is_set,
 *   dbus_message_get_path, dbus_message_is_signal,
 *   dbus_message_iter_append_basic, dbus_message_iter_close_container,
 *   dbus_message_iter_get_arg_type, dbus_message_iter_get_basic,
 *   dbus_message_iter_init, dbus_message_iter_init_append,
 *   dbus_message_iter_next, dbus_message_iter_open_container,
 *   dbus_message_iter_recurse,
 *   dbus_message_new_method_call, dbus_message_unref
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque D-Bus types */
typedef void DBusConnection;
typedef void DBusMessage;
typedef void DBusPendingCall;
typedef int  dbus_bool_t;
typedef unsigned int dbus_uint32_t;
typedef int  dbus_int32_t;

typedef struct {
    const char *name;
    const char *message;
    dbus_uint32_t dummy1;
    dbus_uint32_t dummy2;
    dbus_uint32_t dummy3;
    dbus_uint32_t dummy4;
    dbus_uint32_t dummy5;
    void *padding1;
} DBusError;

typedef struct {
    void *dummy1, *dummy2, *dummy3, *dummy4, *dummy5, *dummy6;
} DBusMessageIter;

#define DBUS_BUS_SESSION 0
#define DBUS_BUS_SYSTEM  1

static void *dbus_handle = NULL;

/* --- Function pointers --- */
static DBusConnection* (*r_dbus_bus_get)(int, DBusError*) = NULL;
static void     (*r_dbus_bus_add_match)(DBusConnection*, const char*, DBusError*) = NULL;
static void     (*r_dbus_connection_flush)(DBusConnection*) = NULL;
static DBusMessage* (*r_dbus_connection_pop_message)(DBusConnection*) = NULL;
static dbus_bool_t  (*r_dbus_connection_read_write)(DBusConnection*, int) = NULL;
static DBusMessage* (*r_dbus_connection_send_with_reply_and_block)(
    DBusConnection*, DBusMessage*, int, DBusError*) = NULL;
static void     (*r_dbus_connection_unref)(DBusConnection*) = NULL;
static void     (*r_dbus_error_free)(DBusError*) = NULL;
static void     (*r_dbus_error_init)(DBusError*) = NULL;
static dbus_bool_t  (*r_dbus_error_is_set)(const DBusError*) = NULL;
static const char*  (*r_dbus_message_get_path)(DBusMessage*) = NULL;
static dbus_bool_t  (*r_dbus_message_is_signal)(DBusMessage*, const char*, const char*) = NULL;
static dbus_bool_t  (*r_dbus_message_iter_append_basic)(DBusMessageIter*, int, const void*) = NULL;
static dbus_bool_t  (*r_dbus_message_iter_close_container)(DBusMessageIter*, DBusMessageIter*) = NULL;
static int      (*r_dbus_message_iter_get_arg_type)(DBusMessageIter*) = NULL;
static void     (*r_dbus_message_iter_get_basic)(DBusMessageIter*, void*) = NULL;
static dbus_bool_t  (*r_dbus_message_iter_init)(DBusMessage*, DBusMessageIter*) = NULL;
static void     (*r_dbus_message_iter_init_append)(DBusMessage*, DBusMessageIter*) = NULL;
static dbus_bool_t  (*r_dbus_message_iter_next)(DBusMessageIter*) = NULL;
static dbus_bool_t  (*r_dbus_message_iter_open_container)(DBusMessageIter*, int,
                                                           const char*, DBusMessageIter*) = NULL;
static void     (*r_dbus_message_iter_recurse)(DBusMessageIter*, DBusMessageIter*) = NULL;
static DBusMessage* (*r_dbus_message_new_method_call)(const char*, const char*,
                                                       const char*, const char*) = NULL;
static void     (*r_dbus_message_unref)(DBusMessage*) = NULL;

__attribute__((constructor))
static void dbus_stub_init(void) {
    static const char * const paths[] = {
        "/usr/lib/aarch64-linux-gnu/libdbus-1.so.3",
        "/usr/lib64/libdbus-1.so.3",
        "/usr/lib/libdbus-1.so.3",
        "/lib/aarch64-linux-gnu/libdbus-1.so.3",
        "/lib64/libdbus-1.so.3",
        "/usr/lib/tizen/libdbus-1.so.3",
        NULL
    };
    for (int i = 0; paths[i] && !dbus_handle; i++)
        dbus_handle = dlopen(paths[i], RTLD_NOW | RTLD_GLOBAL);

    if (dbus_handle) {
        fprintf(stderr, "[dbus-stub] System D-Bus loaded\n");
#define LOAD(sym) r_##sym = dlsym(dbus_handle, #sym)
        LOAD(dbus_bus_get); LOAD(dbus_bus_add_match);
        LOAD(dbus_connection_flush); LOAD(dbus_connection_pop_message);
        LOAD(dbus_connection_read_write); LOAD(dbus_connection_send_with_reply_and_block);
        LOAD(dbus_connection_unref);
        LOAD(dbus_error_free); LOAD(dbus_error_init); LOAD(dbus_error_is_set);
        LOAD(dbus_message_get_path); LOAD(dbus_message_is_signal);
        LOAD(dbus_message_iter_append_basic); LOAD(dbus_message_iter_close_container);
        LOAD(dbus_message_iter_get_arg_type); LOAD(dbus_message_iter_get_basic);
        LOAD(dbus_message_iter_init); LOAD(dbus_message_iter_init_append);
        LOAD(dbus_message_iter_next); LOAD(dbus_message_iter_open_container);
        LOAD(dbus_message_iter_recurse);
        LOAD(dbus_message_new_method_call); LOAD(dbus_message_unref);
#undef LOAD
    } else {
        fprintf(stderr, "[dbus-stub] libdbus-1.so.3 not found — D-Bus IPC disabled\n");
    }
}

/* ── D-Bus connection ────────────────────────────────────────────────────── */

DBusConnection* dbus_bus_get(int type, DBusError *error) {
    if (r_dbus_bus_get) return r_dbus_bus_get(type, error);
    return NULL;
}

void dbus_bus_add_match(DBusConnection *conn, const char *rule, DBusError *error) {
    if (r_dbus_bus_add_match) r_dbus_bus_add_match(conn, rule, error);
    (void)conn; (void)rule; (void)error;
}

void dbus_connection_flush(DBusConnection *conn) {
    if (r_dbus_connection_flush) r_dbus_connection_flush(conn);
}

DBusMessage* dbus_connection_pop_message(DBusConnection *conn) {
    if (r_dbus_connection_pop_message) return r_dbus_connection_pop_message(conn);
    return NULL;
}

dbus_bool_t dbus_connection_read_write(DBusConnection *conn, int timeout_ms) {
    if (r_dbus_connection_read_write) return r_dbus_connection_read_write(conn, timeout_ms);
    return 0;
}

DBusMessage* dbus_connection_send_with_reply_and_block(DBusConnection *conn,
    DBusMessage *msg, int timeout_ms, DBusError *error) {
    if (r_dbus_connection_send_with_reply_and_block)
        return r_dbus_connection_send_with_reply_and_block(conn, msg, timeout_ms, error);
    return NULL;
}

void dbus_connection_unref(DBusConnection *conn) {
    if (r_dbus_connection_unref) r_dbus_connection_unref(conn);
}

/* ── D-Bus errors ────────────────────────────────────────────────────────── */

void dbus_error_free(DBusError *error) {
    if (r_dbus_error_free) { r_dbus_error_free(error); return; }
    if (error) { error->name = NULL; error->message = NULL; }
}

void dbus_error_init(DBusError *error) {
    if (r_dbus_error_init) { r_dbus_error_init(error); return; }
    if (error) memset(error, 0, sizeof(*error));
}

dbus_bool_t dbus_error_is_set(const DBusError *error) {
    if (r_dbus_error_is_set) return r_dbus_error_is_set(error);
    return error && error->name != NULL;
}

/* ── D-Bus messages ──────────────────────────────────────────────────────── */

const char* dbus_message_get_path(DBusMessage *msg) {
    if (r_dbus_message_get_path) return r_dbus_message_get_path(msg);
    return NULL;
}

dbus_bool_t dbus_message_is_signal(DBusMessage *msg, const char *iface, const char *name) {
    if (r_dbus_message_is_signal) return r_dbus_message_is_signal(msg, iface, name);
    (void)msg; (void)iface; (void)name;
    return 0;
}

DBusMessage* dbus_message_new_method_call(const char *dest, const char *path,
    const char *iface, const char *method) {
    if (r_dbus_message_new_method_call)
        return r_dbus_message_new_method_call(dest, path, iface, method);
    return NULL;
}

void dbus_message_unref(DBusMessage *msg) {
    if (r_dbus_message_unref) r_dbus_message_unref(msg);
}

/* ── D-Bus message iterators ─────────────────────────────────────────────── */

dbus_bool_t dbus_message_iter_append_basic(DBusMessageIter *iter, int type, const void *val) {
    if (r_dbus_message_iter_append_basic) return r_dbus_message_iter_append_basic(iter, type, val);
    (void)iter; (void)type; (void)val;
    return 0;
}

dbus_bool_t dbus_message_iter_close_container(DBusMessageIter *iter, DBusMessageIter *sub) {
    if (r_dbus_message_iter_close_container) return r_dbus_message_iter_close_container(iter, sub);
    (void)iter; (void)sub;
    return 0;
}

int dbus_message_iter_get_arg_type(DBusMessageIter *iter) {
    if (r_dbus_message_iter_get_arg_type) return r_dbus_message_iter_get_arg_type(iter);
    return 0; /* DBUS_TYPE_INVALID */
}

void dbus_message_iter_get_basic(DBusMessageIter *iter, void *value) {
    if (r_dbus_message_iter_get_basic) r_dbus_message_iter_get_basic(iter, value);
    else if (value) memset(value, 0, sizeof(void*));
}

dbus_bool_t dbus_message_iter_init(DBusMessage *msg, DBusMessageIter *iter) {
    if (r_dbus_message_iter_init) return r_dbus_message_iter_init(msg, iter);
    (void)msg;
    if (iter) memset(iter, 0, sizeof(*iter));
    return 0;
}

void dbus_message_iter_init_append(DBusMessage *msg, DBusMessageIter *iter) {
    if (r_dbus_message_iter_init_append) r_dbus_message_iter_init_append(msg, iter);
    else if (iter) memset(iter, 0, sizeof(*iter));
}

dbus_bool_t dbus_message_iter_next(DBusMessageIter *iter) {
    if (r_dbus_message_iter_next) return r_dbus_message_iter_next(iter);
    return 0;
}

dbus_bool_t dbus_message_iter_open_container(DBusMessageIter *iter, int type,
    const char *contained_sig, DBusMessageIter *sub) {
    if (r_dbus_message_iter_open_container)
        return r_dbus_message_iter_open_container(iter, type, contained_sig, sub);
    (void)iter; (void)type; (void)contained_sig;
    if (sub) memset(sub, 0, sizeof(*sub));
    return 0;
}

void dbus_message_iter_recurse(DBusMessageIter *iter, DBusMessageIter *sub) {
    if (r_dbus_message_iter_recurse) { r_dbus_message_iter_recurse(iter, sub); return; }
    (void)iter;
    if (sub) memset(sub, 0, sizeof(*sub));
}
