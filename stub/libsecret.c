/*
 * libsecret stub for Tizen TV
 * Tizen TV에는 GNOME keyring(libsecret)이 없음.
 * Sober는 Roblox 자격증명 저장에 사용 → TV에선 파일 기반으로 대체.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

typedef void GError;
typedef void GCancellable;
typedef void GHashTable;
typedef void GMainLoop;
typedef int gboolean;
typedef char gchar;
typedef void* gpointer;
typedef unsigned int guint32;

/* SecretSchema stub */
typedef struct {
    const char *name;
    int flags;
    struct { const char *name; int type; } attributes[32];
} SecretSchema;

typedef struct SecretService SecretService;
typedef struct SecretItem SecretItem;
typedef struct SecretCollection SecretCollection;
typedef struct SecretValue SecretValue;

/* SecretValue - wraps a string credential */
struct SecretValue {
    char *secret;
    size_t length;
    char *content_type;
};

/* Credential storage path on Tizen */
static const char* cred_path(void) {
    static char path[512];
    const char *home = getenv("HOME");
    if (!home) home = "/opt/tizenroblox";
    snprintf(path, sizeof(path), "%s/.tizenroblox_creds", home);
    return path;
}

/* secret_password_store_sync */
gboolean secret_password_store_sync(const SecretSchema *schema,
                                     const char *collection,
                                     const char *label,
                                     const char *password,
                                     GCancellable *cancellable,
                                     GError **error, ...) {
    (void)schema; (void)collection; (void)label; (void)cancellable; (void)error;
    FILE *f = fopen(cred_path(), "w");
    if (!f) return 0;
    fprintf(f, "%s", password ? password : "");
    fclose(f);
    return 1;
}

/* secret_password_lookup_sync */
gchar* secret_password_lookup_sync(const SecretSchema *schema,
                                    GCancellable *cancellable,
                                    GError **error, ...) {
    (void)schema; (void)cancellable; (void)error;
    FILE *f = fopen(cred_path(), "r");
    if (!f) return NULL;
    char buf[4096] = {0};
    size_t n = fread(buf, 1, sizeof(buf)-1, f);
    fclose(f);
    if (n == 0 || buf[0] == '\0') return NULL;
    return strdup(buf);
}

/* secret_password_clear_sync */
gboolean secret_password_clear_sync(const SecretSchema *schema,
                                     GCancellable *cancellable,
                                     GError **error, ...) {
    (void)schema; (void)cancellable; (void)error;
    remove(cred_path());
    return 1;
}

/* secret_password_free */
void secret_password_free(gchar *password) {
    free(password);
}

/* secret_password_wipe */
void secret_password_wipe(gchar *password) {
    if (password) {
        size_t len = strlen(password);
        memset(password, 0, len);
        free(password);
    }
}

/* SecretService stubs */
SecretService* secret_service_get_sync(int flags, GCancellable *cancel, GError **err) {
    (void)flags; (void)cancel; (void)err;
    return NULL;
}

void secret_service_disconnect(void) {}

/* SecretValue stubs */
SecretValue* secret_value_new(const char *secret, ssize_t length, const char *content_type) {
    SecretValue *v = malloc(sizeof(SecretValue));
    if (!v) return NULL;
    v->secret = length >= 0 ? strndup(secret, length) : strdup(secret);
    v->length = length >= 0 ? (size_t)length : strlen(secret);
    v->content_type = strdup(content_type ? content_type : "text/plain");
    return v;
}

const char* secret_value_get(SecretValue *value, size_t *length) {
    if (!value) return NULL;
    if (length) *length = value->length;
    return value->secret;
}

const char* secret_value_get_text(SecretValue *value) {
    if (!value) return NULL;
    return value->secret;
}

void secret_value_unref(SecretValue *value) {
    if (!value) return;
    free(value->secret);
    free(value->content_type);
    free(value);
}

/* GObject stub needed by libsecret */
void g_error_free(GError *err) { (void)err; }
GError* g_error_copy(const GError *err) { (void)err; return NULL; }
