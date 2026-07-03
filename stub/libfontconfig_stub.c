/*
 * libfontconfig.so.1 for Tizen TV — real fontconfig 2.15.0, statically
 * bundled.
 *
 * Sober uses fontconfig to locate system fonts for text rendering.
 * Statically links fontconfig's real libfontconfig.a (cross-compiled by
 * scripts/build_thirdparty.sh, built against our bundled freetype + expat)
 * into a shared object named libfontconfig.so.1. Every real fontconfig
 * symbol is exported directly — no dlopen.
 *
 * Font matching still requires an actual fonts.cache and font files on
 * the TV (fontconfig itself doesn't bundle fonts); Tizen's own
 * /usr/share/fonts and fontconfig cache are used at runtime as normal.
 */

#include <stdio.h>

__attribute__((constructor))
static void fontconfig_bundled_init(void) {
    fprintf(stderr, "[fontconfig] Using statically bundled fontconfig 2.15.0\n");
}
