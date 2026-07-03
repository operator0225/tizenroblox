/*
 * libfreetype.so.6 for Tizen TV — real FreeType 2.13.3, statically bundled.
 *
 * Sober uses FreeType for glyph rasterization (text rendering). Statically
 * links FreeType's real libfreetype.a (cross-compiled by
 * scripts/build_thirdparty.sh, built against our bundled zlib, no
 * harfbuzz/png/bzip2/brotli) into a shared object named libfreetype.so.6.
 * Every real FreeType symbol is exported directly — no dlopen.
 */

#include <stdio.h>

__attribute__((constructor))
static void freetype_bundled_init(void) {
    fprintf(stderr, "[freetype] Using statically bundled FreeType 2.13.3\n");
}
