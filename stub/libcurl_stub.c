/*
 * libcurl stub/shim for Tizen TV
 *
 * Sober uses libcurl for HTTP requests (Roblox API, asset downloads).
 * This shim forwards to the system libcurl.so.4.
 * Falls back to error-returning stubs if curl is unavailable.
 *
 * Symbols used by sober (8):
 *   curl_easy_cleanup, curl_easy_getinfo, curl_easy_init, curl_easy_perform,
 *   curl_easy_setopt, curl_easy_strerror, curl_slist_append, curl_slist_free_all
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef void CURL;
typedef void curl_slist;
typedef unsigned int CURLcode;
typedef unsigned int CURLINFO;
typedef unsigned int CURLoption;

#define CURLE_FAILED_INIT 2
#define CURLE_URL_MALFORMAT 3

static void *curl_handle = NULL;

static CURL*       (*r_curl_easy_init)(void) = NULL;
static void        (*r_curl_easy_cleanup)(CURL*) = NULL;
static CURLcode    (*r_curl_easy_perform)(CURL*) = NULL;
static CURLcode    (*r_curl_easy_setopt)(CURL*, CURLoption, ...) = NULL;
static CURLcode    (*r_curl_easy_getinfo)(CURL*, CURLINFO, ...) = NULL;
static const char* (*r_curl_easy_strerror)(CURLcode) = NULL;
static curl_slist* (*r_curl_slist_append)(curl_slist*, const char*) = NULL;
static void        (*r_curl_slist_free_all)(curl_slist*) = NULL;

__attribute__((constructor))
static void curl_stub_init(void) {
    curl_handle = dlopen("/usr/lib/aarch64-linux-gnu/libcurl.so.4", RTLD_NOW | RTLD_GLOBAL);
    if (!curl_handle) curl_handle = dlopen("/usr/lib64/libcurl.so.4", RTLD_NOW | RTLD_GLOBAL);
    if (!curl_handle) curl_handle = dlopen("/usr/lib/libcurl.so.4", RTLD_NOW | RTLD_GLOBAL);

    if (curl_handle) {
        fprintf(stderr, "[curl-stub] System libcurl loaded\n");
#define LOAD(sym) r_##sym = dlsym(curl_handle, #sym)
        LOAD(curl_easy_init); LOAD(curl_easy_cleanup); LOAD(curl_easy_perform);
        LOAD(curl_easy_setopt); LOAD(curl_easy_getinfo); LOAD(curl_easy_strerror);
        LOAD(curl_slist_append); LOAD(curl_slist_free_all);
#undef LOAD
    } else {
        fprintf(stderr, "[curl-stub] libcurl not found — network requests disabled\n");
    }
}

/* ── Exports ──────────────────────────────────────────────────────────────── */

CURL* curl_easy_init(void)
    { return r_curl_easy_init ? r_curl_easy_init() : NULL; }
void curl_easy_cleanup(CURL *curl)
    { if (r_curl_easy_cleanup) r_curl_easy_cleanup(curl); }
CURLcode curl_easy_perform(CURL *curl)
    { return r_curl_easy_perform ? r_curl_easy_perform(curl) : CURLE_FAILED_INIT; }

CURLcode curl_easy_setopt(CURL *curl, CURLoption option, ...) {
    if (!r_curl_easy_setopt) return 0;
    /* Forward varargs through raw pointer */
    va_list ap;
    va_start(ap, option);
    void *arg = va_arg(ap, void*);
    va_end(ap);
    return r_curl_easy_setopt(curl, option, arg);
}

CURLcode curl_easy_getinfo(CURL *curl, CURLINFO info, ...) {
    if (!r_curl_easy_getinfo) return CURLE_FAILED_INIT;
    va_list ap;
    va_start(ap, info);
    void *arg = va_arg(ap, void*);
    va_end(ap);
    return r_curl_easy_getinfo(curl, info, arg);
}

const char* curl_easy_strerror(CURLcode errornum) {
    if (r_curl_easy_strerror) return r_curl_easy_strerror(errornum);
    return "curl unavailable";
}
curl_slist* curl_slist_append(curl_slist *list, const char *string)
    { return r_curl_slist_append ? r_curl_slist_append(list, string) : NULL; }
void curl_slist_free_all(curl_slist *list)
    { if (r_curl_slist_free_all) r_curl_slist_free_all(list); }
