/*
 * libcrypto.so.3 for Tizen TV — real OpenSSL 3.0.13, statically bundled.
 *
 * Sober is linked against OpenSSL 3.x (libcrypto.so.3). Rather than
 * dlopen-delegating to whatever OpenSSL Tizen ships (version/path unknown,
 * and previously caused a circular-load bug), this target statically links
 * OpenSSL's real libcrypto.a (cross-compiled by scripts/build_thirdparty.sh)
 * into a shared object named libcrypto.so.3. Every real OpenSSL symbol is
 * exported directly — no forwarding wrappers, no dlopen, no version guessing.
 *
 * Runtime deps: only libc/libm/libdl/pthread (guaranteed present on any
 * glibc Linux, including Tizen).
 */

#include <stdio.h>

__attribute__((constructor))
static void crypto3_bundled_init(void) {
    fprintf(stderr, "[libcrypto3] Using statically bundled OpenSSL 3.0.13\n");
}
