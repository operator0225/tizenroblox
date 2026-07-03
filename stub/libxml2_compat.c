/*
 * libxml2.so.16 for Tizen TV — real libxml2 2.12.9, statically bundled.
 *
 * Sober is linked against GNOME Platform 50's libxml2 3.x (SONAME .16).
 * Statically links libxml2's real libxml2.a (cross-compiled by
 * scripts/build_thirdparty.sh) into a shared object named libxml2.so.16.
 * Every real libxml2 symbol is exported directly — no dlopen, no
 * dependency on whatever libxml2 version Tizen ships (typically 2.x/.so.2).
 */

#include <stdio.h>

__attribute__((constructor))
static void xml2_bundled_init(void) {
    fprintf(stderr, "[libxml2] Using statically bundled libxml2 2.12.9\n");
}
