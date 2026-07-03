/*
 * libcrypto.so.3 compatibility shim for Tizen TV
 *
 * Sober is linked against OpenSSL 3.x (libcrypto.so.3).
 * Tizen TV may only have OpenSSL 1.1 (libcrypto.so.1.1).
 * All symbols used by sober exist in both versions with compatible ABI.
 *
 * Tries libcrypto.so.3 first, then falls back to libcrypto.so.1.1.
 *
 * Complete symbol list used by sober:
 *   ASN1_INTEGER_set, ASN1_OBJECT_free, ASN1_OCTET_STRING_{free,new,set}
 *   ERR_error_string_n, ERR_get_error
 *   EVP_Digest{Final_ex,Init_ex,Update}, EVP_MD_CTX_{free,new,reset}
 *   EVP_PKEY_CTX_{free,new_id,set_ec_paramgen_curve_nid}
 *   EVP_PKEY_{free,keygen,keygen_init}, EVP_sha256
 *   OBJ_create, OBJ_txt2obj
 *   X509_{EXTENSION_create_by_OBJ,EXTENSION_free,NAME_add_entry_by_txt}
 *   X509_{add_ext,free,get_serialNumber,get_subject_name}
 *   X509_{getm_notAfter,getm_notBefore,gmtime_adj,new}
 *   X509_{set_issuer_name,set_pubkey,set_version,sign}, i2d_X509
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Opaque type aliases */
typedef void EVP_MD_CTX;
typedef void EVP_MD;
typedef void EVP_PKEY;
typedef void EVP_PKEY_CTX;
typedef void X509;
typedef void X509_NAME;
typedef void X509_EXTENSION;
typedef void ASN1_INTEGER;
typedef void ASN1_TIME;
typedef void ASN1_OBJECT;
typedef void ASN1_OCTET_STRING;

static void *crypto_handle = NULL;

/* ── Function pointers ────────────────────────────────────────────────────── */

/* ASN1 */
static int              (*r_ASN1_INTEGER_set)(ASN1_INTEGER*, long)             = NULL;
static void             (*r_ASN1_OBJECT_free)(ASN1_OBJECT*)                   = NULL;
static void             (*r_ASN1_OCTET_STRING_free)(ASN1_OCTET_STRING*)       = NULL;
static ASN1_OCTET_STRING* (*r_ASN1_OCTET_STRING_new)(void)                    = NULL;
static int              (*r_ASN1_OCTET_STRING_set)(ASN1_OCTET_STRING*,
                             const unsigned char*, int)                        = NULL;

/* ERR */
static char*   (*r_ERR_error_string_n)(unsigned long, char*, size_t)          = NULL;
static unsigned long (*r_ERR_get_error)(void)                                  = NULL;

/* EVP digest */
static EVP_MD_CTX*   (*r_EVP_MD_CTX_new)(void)                                = NULL;
static void          (*r_EVP_MD_CTX_free)(EVP_MD_CTX*)                        = NULL;
static int           (*r_EVP_MD_CTX_reset)(EVP_MD_CTX*)                       = NULL;
static int           (*r_EVP_DigestInit_ex)(EVP_MD_CTX*, const EVP_MD*, void*) = NULL;
static int           (*r_EVP_DigestUpdate)(EVP_MD_CTX*, const void*, size_t)  = NULL;
static int           (*r_EVP_DigestFinal_ex)(EVP_MD_CTX*, unsigned char*, unsigned int*) = NULL;
static const EVP_MD* (*r_EVP_sha256)(void)                                    = NULL;

/* EVP PKEY */
static EVP_PKEY_CTX* (*r_EVP_PKEY_CTX_new_id)(int, void*)                    = NULL;
static void          (*r_EVP_PKEY_CTX_free)(EVP_PKEY_CTX*)                   = NULL;
static int           (*r_EVP_PKEY_CTX_set_ec_paramgen_curve_nid)(EVP_PKEY_CTX*, int) = NULL;
static int           (*r_EVP_PKEY_keygen_init)(EVP_PKEY_CTX*)                 = NULL;
static int           (*r_EVP_PKEY_keygen)(EVP_PKEY_CTX*, EVP_PKEY**)         = NULL;
static void          (*r_EVP_PKEY_free)(EVP_PKEY*)                            = NULL;

/* OBJ */
static int            (*r_OBJ_create)(const char*, const char*, const char*)  = NULL;
static ASN1_OBJECT*   (*r_OBJ_txt2obj)(const char*, int)                      = NULL;

/* X509 */
static X509*          (*r_X509_new)(void)                                     = NULL;
static void           (*r_X509_free)(X509*)                                   = NULL;
static int            (*r_X509_set_version)(X509*, long)                      = NULL;
static ASN1_INTEGER*  (*r_X509_get_serialNumber)(X509*)                       = NULL;
static ASN1_TIME*     (*r_X509_getm_notBefore)(const X509*)                  = NULL;
static ASN1_TIME*     (*r_X509_getm_notAfter)(const X509*)                   = NULL;
static ASN1_TIME*     (*r_X509_gmtime_adj)(ASN1_TIME*, long)                 = NULL;
static X509_NAME*     (*r_X509_get_subject_name)(const X509*)                = NULL;
static int            (*r_X509_NAME_add_entry_by_txt)(X509_NAME*, const char*,
                          int, const unsigned char*, int, int, int)           = NULL;
static int            (*r_X509_set_issuer_name)(X509*, X509_NAME*)           = NULL;
static int            (*r_X509_set_pubkey)(X509*, EVP_PKEY*)                 = NULL;
static int            (*r_X509_sign)(X509*, EVP_PKEY*, const EVP_MD*)        = NULL;
static int            (*r_X509_add_ext)(X509*, X509_EXTENSION*, int)         = NULL;
static void           (*r_X509_EXTENSION_free)(X509_EXTENSION*)              = NULL;
static X509_EXTENSION* (*r_X509_EXTENSION_create_by_OBJ)(X509_EXTENSION**,
                          const ASN1_OBJECT*, int, const void*)               = NULL;
static int            (*r_i2d_X509)(X509*, unsigned char**)                  = NULL;

__attribute__((constructor))
static void crypto3_compat_init(void) {
    static const char *candidates[] = {
        "libcrypto.so.3",
        "libcrypto.so",
        "/usr/lib/aarch64-linux-gnu/libcrypto.so.3",
        "/usr/lib64/libcrypto.so.3",
        "libcrypto.so.1.1",
        "/usr/lib/aarch64-linux-gnu/libcrypto.so.1.1",
        "/usr/lib64/libcrypto.so.1.1",
        NULL
    };

    for (int i = 0; candidates[i]; i++) {
        crypto_handle = dlopen(candidates[i], RTLD_NOW | RTLD_GLOBAL);
        if (crypto_handle) {
            fprintf(stderr, "[libcrypto3-compat] Loaded: %s\n", candidates[i]);
            break;
        }
    }

    if (!crypto_handle) {
        fprintf(stderr, "[libcrypto3-compat] WARNING: No libcrypto found: %s\n", dlerror());
        return;
    }

#define LOAD(sym) r_##sym = dlsym(crypto_handle, #sym)
    /* ASN1 */
    LOAD(ASN1_INTEGER_set); LOAD(ASN1_OBJECT_free);
    LOAD(ASN1_OCTET_STRING_free); LOAD(ASN1_OCTET_STRING_new); LOAD(ASN1_OCTET_STRING_set);
    /* ERR */
    LOAD(ERR_error_string_n); LOAD(ERR_get_error);
    /* EVP digest */
    LOAD(EVP_MD_CTX_new); LOAD(EVP_MD_CTX_free); LOAD(EVP_MD_CTX_reset);
    LOAD(EVP_DigestInit_ex); LOAD(EVP_DigestUpdate); LOAD(EVP_DigestFinal_ex);
    LOAD(EVP_sha256);
    /* EVP PKEY */
    LOAD(EVP_PKEY_CTX_new_id); LOAD(EVP_PKEY_CTX_free);
    LOAD(EVP_PKEY_CTX_set_ec_paramgen_curve_nid);
    LOAD(EVP_PKEY_keygen_init); LOAD(EVP_PKEY_keygen); LOAD(EVP_PKEY_free);
    /* OBJ */
    LOAD(OBJ_create); LOAD(OBJ_txt2obj);
    /* X509 */
    LOAD(X509_new); LOAD(X509_free); LOAD(X509_set_version);
    LOAD(X509_get_serialNumber); LOAD(X509_getm_notBefore); LOAD(X509_getm_notAfter);
    LOAD(X509_gmtime_adj); LOAD(X509_get_subject_name);
    LOAD(X509_NAME_add_entry_by_txt); LOAD(X509_set_issuer_name);
    LOAD(X509_set_pubkey); LOAD(X509_sign); LOAD(X509_add_ext);
    LOAD(X509_EXTENSION_free); LOAD(X509_EXTENSION_create_by_OBJ);
    LOAD(i2d_X509);
#undef LOAD
}

/* ── ASN1 exports ─────────────────────────────────────────────────────────── */
int ASN1_INTEGER_set(ASN1_INTEGER *a, long v)
    { return r_ASN1_INTEGER_set ? r_ASN1_INTEGER_set(a, v) : 0; }
void ASN1_OBJECT_free(ASN1_OBJECT *a)
    { if (r_ASN1_OBJECT_free) r_ASN1_OBJECT_free(a); else free(a); }
void ASN1_OCTET_STRING_free(ASN1_OCTET_STRING *a)
    { if (r_ASN1_OCTET_STRING_free) r_ASN1_OCTET_STRING_free(a); else free(a); }
ASN1_OCTET_STRING* ASN1_OCTET_STRING_new(void)
    { return r_ASN1_OCTET_STRING_new ? r_ASN1_OCTET_STRING_new() : calloc(1, 64); }
int ASN1_OCTET_STRING_set(ASN1_OCTET_STRING *str, const unsigned char *data, int len)
    { return r_ASN1_OCTET_STRING_set ? r_ASN1_OCTET_STRING_set(str, data, len) : 0; }

/* ── ERR exports ──────────────────────────────────────────────────────────── */
char* ERR_error_string_n(unsigned long e, char *buf, size_t len) {
    if (r_ERR_error_string_n) return r_ERR_error_string_n(e, buf, len);
    if (buf && len > 0) { snprintf(buf, len, "error:%08lx", e); return buf; }
    return NULL;
}
unsigned long ERR_get_error(void)
    { return r_ERR_get_error ? r_ERR_get_error() : 0; }

/* ── EVP digest exports ───────────────────────────────────────────────────── */
EVP_MD_CTX* EVP_MD_CTX_new(void)
    { return r_EVP_MD_CTX_new ? r_EVP_MD_CTX_new() : NULL; }
void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
    { if (r_EVP_MD_CTX_free) r_EVP_MD_CTX_free(ctx); }
int EVP_MD_CTX_reset(EVP_MD_CTX *ctx)
    { return r_EVP_MD_CTX_reset ? r_EVP_MD_CTX_reset(ctx) : 0; }
int EVP_DigestInit_ex(EVP_MD_CTX *ctx, const EVP_MD *type, void *impl)
    { return r_EVP_DigestInit_ex ? r_EVP_DigestInit_ex(ctx, type, impl) : 0; }
int EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *d, size_t cnt)
    { return r_EVP_DigestUpdate ? r_EVP_DigestUpdate(ctx, d, cnt) : 0; }
int EVP_DigestFinal_ex(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *s)
    { return r_EVP_DigestFinal_ex ? r_EVP_DigestFinal_ex(ctx, md, s) : 0; }
const EVP_MD* EVP_sha256(void)
    { return r_EVP_sha256 ? r_EVP_sha256() : NULL; }

/* ── EVP PKEY exports ─────────────────────────────────────────────────────── */
EVP_PKEY_CTX* EVP_PKEY_CTX_new_id(int id, void *e)
    { return r_EVP_PKEY_CTX_new_id ? r_EVP_PKEY_CTX_new_id(id, e) : NULL; }
void EVP_PKEY_CTX_free(EVP_PKEY_CTX *ctx)
    { if (r_EVP_PKEY_CTX_free) r_EVP_PKEY_CTX_free(ctx); }
int EVP_PKEY_CTX_set_ec_paramgen_curve_nid(EVP_PKEY_CTX *ctx, int nid)
    { return r_EVP_PKEY_CTX_set_ec_paramgen_curve_nid ?
        r_EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, nid) : -1; }
int EVP_PKEY_keygen_init(EVP_PKEY_CTX *ctx)
    { return r_EVP_PKEY_keygen_init ? r_EVP_PKEY_keygen_init(ctx) : -1; }
int EVP_PKEY_keygen(EVP_PKEY_CTX *ctx, EVP_PKEY **ppkey)
    { return r_EVP_PKEY_keygen ? r_EVP_PKEY_keygen(ctx, ppkey) : -1; }
void EVP_PKEY_free(EVP_PKEY *pkey)
    { if (r_EVP_PKEY_free) r_EVP_PKEY_free(pkey); }

/* ── OBJ exports ──────────────────────────────────────────────────────────── */
int OBJ_create(const char *oid, const char *sn, const char *ln)
    { return r_OBJ_create ? r_OBJ_create(oid, sn, ln) : 0; }
ASN1_OBJECT* OBJ_txt2obj(const char *s, int no_name)
    { return r_OBJ_txt2obj ? r_OBJ_txt2obj(s, no_name) : NULL; }

/* ── X509 exports ─────────────────────────────────────────────────────────── */
X509* X509_new(void)
    { return r_X509_new ? r_X509_new() : NULL; }
void X509_free(X509 *a)
    { if (r_X509_free) r_X509_free(a); }
int X509_set_version(X509 *x, long version)
    { return r_X509_set_version ? r_X509_set_version(x, version) : 0; }
ASN1_INTEGER* X509_get_serialNumber(X509 *a)
    { return r_X509_get_serialNumber ? r_X509_get_serialNumber(a) : NULL; }
ASN1_TIME* X509_getm_notBefore(const X509 *x)
    { return r_X509_getm_notBefore ? r_X509_getm_notBefore(x) : NULL; }
ASN1_TIME* X509_getm_notAfter(const X509 *x)
    { return r_X509_getm_notAfter ? r_X509_getm_notAfter(x) : NULL; }
ASN1_TIME* X509_gmtime_adj(ASN1_TIME *s, long adj)
    { return r_X509_gmtime_adj ? r_X509_gmtime_adj(s, adj) : NULL; }
X509_NAME* X509_get_subject_name(const X509 *a)
    { return r_X509_get_subject_name ? r_X509_get_subject_name(a) : NULL; }
int X509_NAME_add_entry_by_txt(X509_NAME *name, const char *field, int type,
    const unsigned char *bytes, int len, int loc, int set) {
    return r_X509_NAME_add_entry_by_txt ?
        r_X509_NAME_add_entry_by_txt(name, field, type, bytes, len, loc, set) : 0;
}
int X509_set_issuer_name(X509 *x, X509_NAME *name)
    { return r_X509_set_issuer_name ? r_X509_set_issuer_name(x, name) : 0; }
int X509_set_pubkey(X509 *x, EVP_PKEY *pkey)
    { return r_X509_set_pubkey ? r_X509_set_pubkey(x, pkey) : 0; }
int X509_sign(X509 *x, EVP_PKEY *pkey, const EVP_MD *md)
    { return r_X509_sign ? r_X509_sign(x, pkey, md) : 0; }
int X509_add_ext(X509 *x, X509_EXTENSION *ex, int loc)
    { return r_X509_add_ext ? r_X509_add_ext(x, ex, loc) : 0; }
void X509_EXTENSION_free(X509_EXTENSION *a)
    { if (r_X509_EXTENSION_free) r_X509_EXTENSION_free(a); }
X509_EXTENSION* X509_EXTENSION_create_by_OBJ(X509_EXTENSION **ex,
    const ASN1_OBJECT *obj, int crit, const void *data) {
    return r_X509_EXTENSION_create_by_OBJ ?
        r_X509_EXTENSION_create_by_OBJ(ex, obj, crit, data) : NULL;
}
int i2d_X509(X509 *a, unsigned char **out)
    { return r_i2d_X509 ? r_i2d_X509(a, out) : -1; }
