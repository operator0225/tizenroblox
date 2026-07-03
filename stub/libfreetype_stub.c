/*
 * FreeType stub/shim for Tizen TV
 *
 * Sober uses FreeType for glyph rasterization (text rendering).
 * This shim tries to load the system libfreetype.so.6.
 * Falls back to no-ops that return empty glyphs (text invisible but no crash).
 *
 * Symbols used by sober (16):
 *   FT_Activate_Size, FT_Add_Default_Modules, FT_Done_Face, FT_Done_Library,
 *   FT_Done_Size, FT_Get_Char_Index, FT_GlyphSlot_Embolden, FT_GlyphSlot_Oblique,
 *   FT_Library_Version, FT_Load_Glyph, FT_New_Library, FT_New_Memory_Face,
 *   FT_New_Size, FT_Render_Glyph, FT_Request_Size, FT_Select_Charmap
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Opaque FreeType types */
typedef void* FT_Library;
typedef void* FT_Face;
typedef void* FT_Size;
typedef unsigned long FT_ULong;
typedef unsigned int  FT_UInt;
typedef int           FT_Error;
typedef int           FT_Int32;
typedef void*         FT_GlyphSlot;
typedef void*         FT_Size_Request;

#define FT_ENCODING_UNICODE 0x756E6963

static void *ft_handle = NULL;

static FT_Error (*r_FT_New_Library)(void*, FT_Library*)  = NULL;
static FT_Error (*r_FT_Done_Library)(FT_Library)          = NULL;
static void     (*r_FT_Add_Default_Modules)(FT_Library)   = NULL;
static void     (*r_FT_Library_Version)(FT_Library, int*, int*, int*) = NULL;
static FT_Error (*r_FT_New_Memory_Face)(FT_Library, const unsigned char*, long, long, FT_Face*) = NULL;
static FT_Error (*r_FT_Done_Face)(FT_Face)                = NULL;
static FT_Error (*r_FT_New_Size)(FT_Face, FT_Size*)       = NULL;
static FT_Error (*r_FT_Done_Size)(FT_Size)                = NULL;
static FT_Error (*r_FT_Activate_Size)(FT_Size)            = NULL;
static FT_Error (*r_FT_Select_Charmap)(FT_Face, unsigned int) = NULL;
static FT_Error (*r_FT_Request_Size)(FT_Face, FT_Size_Request) = NULL;
static FT_UInt  (*r_FT_Get_Char_Index)(FT_Face, FT_ULong) = NULL;
static FT_Error (*r_FT_Load_Glyph)(FT_Face, FT_UInt, FT_Int32) = NULL;
static FT_Error (*r_FT_Render_Glyph)(FT_GlyphSlot, int)  = NULL;
static void     (*r_FT_GlyphSlot_Embolden)(FT_GlyphSlot)  = NULL;
static void     (*r_FT_GlyphSlot_Oblique)(FT_GlyphSlot)   = NULL;

__attribute__((constructor))
static void ft_stub_init(void) {
    ft_handle = dlopen("libfreetype.so.6", RTLD_NOW | RTLD_GLOBAL);
    if (!ft_handle) ft_handle = dlopen("libfreetype.so", RTLD_NOW | RTLD_GLOBAL);

    if (ft_handle) {
        fprintf(stderr, "[ft-stub] System FreeType loaded\n");
#define LOAD(sym) r_##sym = dlsym(ft_handle, #sym)
        LOAD(FT_New_Library); LOAD(FT_Done_Library); LOAD(FT_Add_Default_Modules);
        LOAD(FT_Library_Version);
        LOAD(FT_New_Memory_Face); LOAD(FT_Done_Face);
        LOAD(FT_New_Size); LOAD(FT_Done_Size); LOAD(FT_Activate_Size);
        LOAD(FT_Select_Charmap); LOAD(FT_Request_Size);
        LOAD(FT_Get_Char_Index); LOAD(FT_Load_Glyph); LOAD(FT_Render_Glyph);
        LOAD(FT_GlyphSlot_Embolden); LOAD(FT_GlyphSlot_Oblique);
#undef LOAD
    } else {
        fprintf(stderr, "[ft-stub] FreeType not found — text rendering disabled\n");
    }
}

/* Minimal allocator vtable stub for FT_New_Library when system not present */
static int ft_alloc_stub(void *memory, long size, void **block)
    { *block = malloc(size); return *block ? 0 : 6; }
static int ft_realloc_stub(void *memory, long cur, long new_sz, void **block)
    { *block = realloc(*block, new_sz); return *block ? 0 : 6; }
static void ft_free_stub(void *memory, void *block)
    { free(block); }

/* ── Exports ──────────────────────────────────────────────────────────────── */

FT_Error FT_New_Library(void *memory, FT_Library *alibrary) {
    if (r_FT_New_Library) return r_FT_New_Library(memory, alibrary);
    *alibrary = calloc(1, 256);
    return 0;
}
FT_Error FT_Done_Library(FT_Library library)
    { if (r_FT_Done_Library) return r_FT_Done_Library(library); free(library); return 0; }
void FT_Add_Default_Modules(FT_Library library)
    { if (r_FT_Add_Default_Modules) r_FT_Add_Default_Modules(library); }
void FT_Library_Version(FT_Library library, int *amajor, int *aminor, int *apatch) {
    if (r_FT_Library_Version) { r_FT_Library_Version(library, amajor, aminor, apatch); return; }
    if (amajor) *amajor = 2; if (aminor) *aminor = 13; if (apatch) *apatch = 0;
}
FT_Error FT_New_Memory_Face(FT_Library library, const unsigned char *file_base,
    long file_size, long face_index, FT_Face *aface) {
    if (r_FT_New_Memory_Face) return r_FT_New_Memory_Face(library, file_base, file_size, face_index, aface);
    *aface = calloc(1, 1024);
    return 0;
}
FT_Error FT_Done_Face(FT_Face face)
    { if (r_FT_Done_Face) return r_FT_Done_Face(face); free(face); return 0; }
FT_Error FT_New_Size(FT_Face face, FT_Size *asize)
    { if (r_FT_New_Size) return r_FT_New_Size(face, asize); *asize = calloc(1, 128); return 0; }
FT_Error FT_Done_Size(FT_Size size)
    { if (r_FT_Done_Size) return r_FT_Done_Size(size); free(size); return 0; }
FT_Error FT_Activate_Size(FT_Size size)
    { return r_FT_Activate_Size ? r_FT_Activate_Size(size) : 0; }
FT_Error FT_Select_Charmap(FT_Face face, unsigned int encoding)
    { return r_FT_Select_Charmap ? r_FT_Select_Charmap(face, encoding) : 0; }
FT_Error FT_Request_Size(FT_Face face, FT_Size_Request req)
    { return r_FT_Request_Size ? r_FT_Request_Size(face, req) : 0; }
FT_UInt FT_Get_Char_Index(FT_Face face, FT_ULong charcode)
    { return r_FT_Get_Char_Index ? r_FT_Get_Char_Index(face, charcode) : 0; }
FT_Error FT_Load_Glyph(FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags)
    { return r_FT_Load_Glyph ? r_FT_Load_Glyph(face, glyph_index, load_flags) : 0; }
FT_Error FT_Render_Glyph(FT_GlyphSlot slot, int render_mode)
    { return r_FT_Render_Glyph ? r_FT_Render_Glyph(slot, render_mode) : 0; }
void FT_GlyphSlot_Embolden(FT_GlyphSlot slot)
    { if (r_FT_GlyphSlot_Embolden) r_FT_GlyphSlot_Embolden(slot); }
void FT_GlyphSlot_Oblique(FT_GlyphSlot slot)
    { if (r_FT_GlyphSlot_Oblique) r_FT_GlyphSlot_Oblique(slot); }
