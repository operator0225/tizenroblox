/*
 * libcurl.so.4 for Tizen TV — real curl 8.10.1, statically bundled.
 *
 * Sober uses libcurl for HTTP requests (Roblox API, asset downloads).
 * Statically links curl's real libcurl.a (built against our bundled
 * OpenSSL 3.0.13 + zlib, cross-compiled by scripts/build_thirdparty.sh)
 * into a shared object named libcurl.so.4. Every real libcurl symbol is
 * exported directly — no dlopen, no dependency on Tizen's own curl/SSL.
 */

#include <stdio.h>

__attribute__((constructor))
static void curl_bundled_init(void) {
    fprintf(stderr, "[libcurl] Using statically bundled curl 8.10.1\n");
}
