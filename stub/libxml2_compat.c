/*
 * libxml2.so.16 compatibility shim for Tizen TV
 *
 * Sober is linked against GNOME Platform 50 which bundles libxml2 3.x
 * (SONAME: libxml2.so.16). Tizen TV ships libxml2 2.x (SONAME: libxml2.so.2).
 *
 * This shim builds a shared library named libxml2.so.16 that dynamically
 * loads the system's libxml2.so.2 and forwards the symbols sober uses.
 *
 * Symbols needed by sober (from strings analysis):
 *   xmlParseFile, xmlDocGetRootElement, xmlFreeDoc, xmlStrcmp,
 *   xmlGetProp, xmlNodeSetContent, xmlSaveFormatFileEnc,
 *   xmlCleanupParser, xmlFree
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward types without including libxml2 headers */
typedef void xmlDoc;
typedef void xmlNode;
typedef unsigned char xmlChar;

static void *libxml2_handle = NULL;

/* Function pointer table */
static xmlDoc*   (*real_xmlParseFile)(const char *filename)            = NULL;
static xmlNode*  (*real_xmlDocGetRootElement)(xmlDoc *doc)             = NULL;
static void      (*real_xmlFreeDoc)(xmlDoc *doc)                       = NULL;
static int       (*real_xmlStrcmp)(const xmlChar *str1, const xmlChar *str2) = NULL;
static xmlChar*  (*real_xmlGetProp)(xmlNode *node, const xmlChar *name) = NULL;
static void      (*real_xmlNodeSetContent)(xmlNode *node, const xmlChar *content) = NULL;
static int       (*real_xmlSaveFormatFileEnc)(const char *filename, xmlDoc *doc,
                                               const char *encoding, int format) = NULL;
static void      (*real_xmlCleanupParser)(void)                        = NULL;
static void      (*real_xmlFree)(void *mem)                            = NULL;

__attribute__((constructor))
static void xml2_compat_init(void) {
    /* Try libxml2.so.2 first, then libxml2.so */
    static const char *candidates[] = {
        "libxml2.so.2",
        "libxml2.so",
        "/usr/lib/aarch64-linux-gnu/libxml2.so.2",
        "/usr/lib64/libxml2.so.2",
        "/usr/lib/libxml2.so.2",
        NULL
    };

    for (int i = 0; candidates[i]; i++) {
        libxml2_handle = dlopen(candidates[i], RTLD_NOW | RTLD_GLOBAL);
        if (libxml2_handle) break;
    }

    if (!libxml2_handle) {
        fprintf(stderr, "[libxml2-compat] WARNING: system libxml2 not found: %s\n", dlerror());
        return;
    }

#define LOAD(sym) real_##sym = dlsym(libxml2_handle, #sym)
    LOAD(xmlParseFile);
    LOAD(xmlDocGetRootElement);
    LOAD(xmlFreeDoc);
    LOAD(xmlStrcmp);
    LOAD(xmlGetProp);
    LOAD(xmlNodeSetContent);
    LOAD(xmlSaveFormatFileEnc);
    LOAD(xmlCleanupParser);
    LOAD(xmlFree);
#undef LOAD
}

__attribute__((destructor))
static void xml2_compat_fini(void) {
    if (libxml2_handle) {
        dlclose(libxml2_handle);
        libxml2_handle = NULL;
    }
}

/* ── Exported symbols ──────────────────────────────────────────────────────── */

xmlDoc* xmlParseFile(const char *filename) {
    if (real_xmlParseFile) return real_xmlParseFile(filename);
    return NULL;
}

xmlNode* xmlDocGetRootElement(xmlDoc *doc) {
    if (real_xmlDocGetRootElement) return real_xmlDocGetRootElement(doc);
    return NULL;
}

void xmlFreeDoc(xmlDoc *doc) {
    if (real_xmlFreeDoc) real_xmlFreeDoc(doc);
}

int xmlStrcmp(const xmlChar *str1, const xmlChar *str2) {
    if (real_xmlStrcmp) return real_xmlStrcmp(str1, str2);
    return strcmp((const char*)str1, (const char*)str2);
}

xmlChar* xmlGetProp(xmlNode *node, const xmlChar *name) {
    if (real_xmlGetProp) return real_xmlGetProp(node, name);
    return NULL;
}

void xmlNodeSetContent(xmlNode *node, const xmlChar *content) {
    if (real_xmlNodeSetContent) real_xmlNodeSetContent(node, content);
}

int xmlSaveFormatFileEnc(const char *filename, xmlDoc *doc,
                          const char *encoding, int format) {
    if (real_xmlSaveFormatFileEnc)
        return real_xmlSaveFormatFileEnc(filename, doc, encoding, format);
    return -1;
}

void xmlCleanupParser(void) {
    if (real_xmlCleanupParser) real_xmlCleanupParser();
}

void xmlFree(void *mem) {
    if (real_xmlFree) real_xmlFree(mem);
    else free(mem);
}
