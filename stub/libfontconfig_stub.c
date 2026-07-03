/*
 * fontconfig stub/shim for Tizen TV
 *
 * Sober uses fontconfig to locate system fonts for text rendering.
 * This shim tries to use the system fontconfig, falling back to a
 * minimal stub that returns a fake font match so Roblox can start.
 *
 * Symbols used by sober:
 *   FcConfigSubstitute, FcDefaultSubstitute, FcFontSetDestroy, FcFontSort,
 *   FcLangSetAdd, FcLangSetCreate, FcLangSetDestroy, FcLangSetHasLang,
 *   FcPatternAddLangSet, FcPatternAddString, FcPatternCreate, FcPatternDestroy,
 *   FcPatternGetLangSet, FcPatternGetString
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void FcConfig;
typedef void FcPattern;
typedef void FcFontSet;
typedef void FcLangSet;
typedef void FcObjectSet;
typedef unsigned int FcBool;
typedef unsigned char FcChar8;
typedef int FcMatchKind;
typedef int FcResult;

static void *fc_handle = NULL;

static FcBool     (*r_FcConfigSubstitute)(FcConfig*, FcPattern*, FcMatchKind) = NULL;
static void       (*r_FcDefaultSubstitute)(FcPattern*) = NULL;
static void       (*r_FcFontSetDestroy)(FcFontSet*) = NULL;
static FcFontSet* (*r_FcFontSort)(FcConfig*, FcPattern*, FcBool, void*, FcResult*) = NULL;
static FcLangSet* (*r_FcLangSetCreate)(void) = NULL;
static void       (*r_FcLangSetDestroy)(FcLangSet*) = NULL;
static FcBool     (*r_FcLangSetAdd)(FcLangSet*, const FcChar8*) = NULL;
static int        (*r_FcLangSetHasLang)(const FcLangSet*, const FcChar8*) = NULL;
static FcBool     (*r_FcPatternAddLangSet)(FcPattern*, const char*, const FcLangSet*) = NULL;
static FcBool     (*r_FcPatternAddString)(FcPattern*, const char*, const FcChar8*) = NULL;
static FcPattern* (*r_FcPatternCreate)(void) = NULL;
static void       (*r_FcPatternDestroy)(FcPattern*) = NULL;
static FcResult   (*r_FcPatternGetLangSet)(const FcPattern*, const char*, int, FcLangSet**) = NULL;
static FcResult   (*r_FcPatternGetString)(const FcPattern*, const char*, int, FcChar8**) = NULL;

__attribute__((constructor))
static void fc_stub_init(void) {
    fc_handle = dlopen("/usr/lib/aarch64-linux-gnu/libfontconfig.so.1", RTLD_NOW | RTLD_GLOBAL);
    if (!fc_handle) fc_handle = dlopen("/usr/lib64/libfontconfig.so.1", RTLD_NOW | RTLD_GLOBAL);
    if (!fc_handle) fc_handle = dlopen("/usr/lib/libfontconfig.so.1", RTLD_NOW | RTLD_GLOBAL);

    if (fc_handle) {
        fprintf(stderr, "[fc-stub] System fontconfig loaded\n");
#define LOAD(sym) r_##sym = dlsym(fc_handle, #sym)
        LOAD(FcConfigSubstitute); LOAD(FcDefaultSubstitute);
        LOAD(FcFontSetDestroy); LOAD(FcFontSort);
        LOAD(FcLangSetCreate); LOAD(FcLangSetDestroy);
        LOAD(FcLangSetAdd); LOAD(FcLangSetHasLang);
        LOAD(FcPatternAddLangSet); LOAD(FcPatternAddString);
        LOAD(FcPatternCreate); LOAD(FcPatternDestroy);
        LOAD(FcPatternGetLangSet); LOAD(FcPatternGetString);
#undef LOAD
    } else {
        fprintf(stderr, "[fc-stub] fontconfig not found — using minimal stubs\n");
    }
}

/* ── Exports ──────────────────────────────────────────────────────────────── */

FcBool FcConfigSubstitute(FcConfig *config, FcPattern *p, FcMatchKind kind)
    { return r_FcConfigSubstitute ? r_FcConfigSubstitute(config, p, kind) : 1; }
void FcDefaultSubstitute(FcPattern *pattern)
    { if (r_FcDefaultSubstitute) r_FcDefaultSubstitute(pattern); }
void FcFontSetDestroy(FcFontSet *s)
    { if (r_FcFontSetDestroy) r_FcFontSetDestroy(s); else free(s); }
FcFontSet* FcFontSort(FcConfig *config, FcPattern *p, FcBool trim,
    void *csp, FcResult *result) {
    if (r_FcFontSort) return r_FcFontSort(config, p, trim, csp, result);
    /* Return empty font set */
    FcFontSet *fs = calloc(1, 32);
    if (result) *result = 0;
    return fs;
}
FcLangSet* FcLangSetCreate(void)
    { return r_FcLangSetCreate ? r_FcLangSetCreate() : calloc(1, 64); }
void FcLangSetDestroy(FcLangSet *ls)
    { if (r_FcLangSetDestroy) r_FcLangSetDestroy(ls); else free(ls); }
FcBool FcLangSetAdd(FcLangSet *ls, const FcChar8 *lang)
    { return r_FcLangSetAdd ? r_FcLangSetAdd(ls, lang) : 1; }
int FcLangSetHasLang(const FcLangSet *ls, const FcChar8 *lang)
    { return r_FcLangSetHasLang ? r_FcLangSetHasLang(ls, lang) : 0; }
FcBool FcPatternAddLangSet(FcPattern *p, const char *object, const FcLangSet *ls)
    { return r_FcPatternAddLangSet ? r_FcPatternAddLangSet(p, object, ls) : 1; }
FcBool FcPatternAddString(FcPattern *p, const char *object, const FcChar8 *s)
    { return r_FcPatternAddString ? r_FcPatternAddString(p, object, s) : 1; }
FcPattern* FcPatternCreate(void)
    { return r_FcPatternCreate ? r_FcPatternCreate() : calloc(1, 64); }
void FcPatternDestroy(FcPattern *p)
    { if (r_FcPatternDestroy) r_FcPatternDestroy(p); else free(p); }
FcResult FcPatternGetLangSet(const FcPattern *p, const char *object, int n, FcLangSet **ls)
    { return r_FcPatternGetLangSet ? r_FcPatternGetLangSet(p, object, n, ls) : 1; }
FcResult FcPatternGetString(const FcPattern *p, const char *object, int n, FcChar8 **s)
    { return r_FcPatternGetString ? r_FcPatternGetString(p, object, n, s) : 1; }
