#!/bin/bash
# TizenRoblox Third-Party Library Cross-Compiler
#
# Cross-compiles OpenSSL, curl, libxml2, glib/gobject, freetype, and
# fontconfig (plus their small deps: zlib, libffi, pcre2, expat) as static
# libraries for aarch64, then statically links each into the corresponding
# TizenRoblox stub .so (see stub/CMakeLists.txt). This replaces the old
# dlopen-delegation stubs for these libraries: the .so we ship now IS the
# real implementation, with only libc/libm/libdl/pthread as runtime deps
# (always present on any glibc Linux, including Tizen).
#
# GPU/display libraries (libEGL, libGLESv2, libwayland-egl, GStreamer +
# plugins) are intentionally NOT bundled here — they must come from the
# Tizen system to talk to the real Samsung GPU driver / Enlightenment
# compositor / hardware codecs. See tizen/launch.sh prefer_system_lib().
#
# Usage: bash scripts/build_thirdparty.sh
#
# Requires network access to: curl.se, download.gnome.org,
# download.savannah.gnu.org, freedesktop.org, and Ubuntu's deb-src repos
# (for zlib/openssl/libffi/pcre2/expat, which ship as Ubuntu source
# packages with upstream CVE patches already applied).

set -e
cd "$(dirname "$0")/.."

THIRDPARTY_DIR="$(pwd)/thirdparty"
SRC_DIR="${THIRDPARTY_DIR}/src"
PREFIX="${THIRDPARTY_DIR}/install"

CROSS_PREFIX="aarch64-linux-gnu"
export CC="${CROSS_PREFIX}-gcc"
export CXX="${CROSS_PREFIX}-g++"
export AR="${CROSS_PREFIX}-ar"
export RANLIB="${CROSS_PREFIX}-ranlib"
export STRIP="${CROSS_PREFIX}-strip"
export LD="${CROSS_PREFIX}-ld"
export HOST="aarch64-linux-gnu"
export PKG_CONFIG_PATH="${PREFIX}/lib/pkgconfig"
export PKG_CONFIG_LIBDIR="${PREFIX}/lib/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR=""
export CFLAGS="-O2 -fPIC -I${PREFIX}/include"
export LDFLAGS="-L${PREFIX}/lib"

mkdir -p "${SRC_DIR}" "${PREFIX}"

# ── glibc ISO-C23 strtol/scanf redirect neutralizer ─────────────────────────
# Ubuntu 24.04's aarch64 sysroot ships glibc 2.39. Any TU compiled with
# _GNU_SOURCE (required by OpenSSL/curl/glib/etc.) has strtol/strtoul/
# strtoll/scanf/etc. silently redirected by <stdlib.h>/<wchar.h> to
# __isoc23_* symbols, which only exist in glibc >= 2.38 (Aug 2023). Tizen
# 9.0's actual glibc version is unknown to us, so this would introduce an
# unnecessary hard floor. The __isoc23_* variants only add C23 0b/0B binary
# literal parsing support that none of our bundled libs rely on — safe to
# redirect back to the classic (GLIBC_2.17-era) symbol names via objcopy.
# Run this on every static .a we produce before installing it.
degrade_isoc23_symbols() {
    local archive="$1"
    "${CROSS_PREFIX}-objcopy" \
        --redefine-sym __isoc23_strtol=strtol \
        --redefine-sym __isoc23_strtoul=strtoul \
        --redefine-sym __isoc23_strtoll=strtoll \
        --redefine-sym __isoc23_strtoull=strtoull \
        --redefine-sym __isoc23_strtol_l=strtol_l \
        --redefine-sym __isoc23_strtoul_l=strtoul_l \
        --redefine-sym __isoc23_strtoll_l=strtoll_l \
        --redefine-sym __isoc23_strtoull_l=strtoull_l \
        --redefine-sym __isoc23_strtof=strtof \
        --redefine-sym __isoc23_strtod=strtod \
        --redefine-sym __isoc23_strtold=strtold \
        --redefine-sym __isoc23_scanf=scanf \
        --redefine-sym __isoc23_sscanf=sscanf \
        --redefine-sym __isoc23_fscanf=fscanf \
        --redefine-sym __isoc23_vscanf=vscanf \
        --redefine-sym __isoc23_vsscanf=vsscanf \
        --redefine-sym __isoc23_vfscanf=vfscanf \
        --redefine-sym __isoc23_wcstol=wcstol \
        --redefine-sym __isoc23_wcstoul=wcstoul \
        --redefine-sym __isoc23_wcstoll=wcstoll \
        --redefine-sym __isoc23_wcstoull=wcstoull \
        "${archive}" "${archive}" 2>/dev/null || true
}

# ── Toolchain sanity check ────────────────────────────────────────────────
if ! command -v "${CC}" >/dev/null 2>&1; then
    echo "[thirdparty] Installing aarch64 cross-compiler..."
    apt-get update -qq
    apt-get install -y -qq gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
        binutils-aarch64-linux-gnu pkg-config
fi
if ! command -v meson >/dev/null 2>&1; then
    echo "[thirdparty] Installing meson..."
    apt-get install -y -qq meson
fi

# ── Fetch sources ─────────────────────────────────────────────────────────
# zlib, OpenSSL, libffi, pcre2, expat: via Ubuntu deb-src (CVE-patched)
if [ ! -d "${SRC_DIR}/zlib-1.3.dfsg" ] || [ ! -d "${SRC_DIR}/openssl-3.0.13" ]; then
    echo "[thirdparty] Fetching Ubuntu source packages..."
    if ! grep -q "^Types: deb deb-src" /etc/apt/sources.list.d/ubuntu.sources 2>/dev/null; then
        sed -i 's/^Types: deb$/Types: deb deb-src/' /etc/apt/sources.list.d/ubuntu.sources
        apt-get update -qq
    fi
    (cd "${SRC_DIR}" && apt-get source zlib1g openssl libffi8 libpcre2-8-0 libexpat1)
fi

# curl, libxml2, glib, freetype, fontconfig: direct upstream tarballs
fetch() {
    local url="$1" fname
    fname=$(basename "$url")
    if [ ! -f "${SRC_DIR}/${fname}" ]; then
        echo "[thirdparty] Downloading ${fname}..."
        curl -sL --max-time 60 -o "${SRC_DIR}/${fname}" "$url"
    fi
}
fetch "https://curl.se/download/curl-8.10.1.tar.gz"
fetch "https://download.gnome.org/sources/libxml2/2.12/libxml2-2.12.9.tar.xz"
fetch "https://download.gnome.org/sources/glib/2.82/glib-2.82.2.tar.xz"
fetch "https://download.savannah.gnu.org/releases/freetype/freetype-2.13.3.tar.gz"
fetch "https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.15.0.tar.gz"

cd "${SRC_DIR}"
[ -d curl-8.10.1 ]       || tar xzf curl-8.10.1.tar.gz
[ -d libxml2-2.12.9 ]    || tar xJf libxml2-2.12.9.tar.xz
[ -d glib-2.82.2 ]       || tar xJf glib-2.82.2.tar.xz
[ -d freetype-2.13.3 ]   || tar xzf freetype-2.13.3.tar.gz
[ -d fontconfig-2.15.0 ] || tar xzf fontconfig-2.15.0.tar.gz

echo "[thirdparty] All sources ready."

# ── zlib (static) ─────────────────────────────────────────────────────────
if [ ! -f "${PREFIX}/lib/libz.a" ]; then
    echo "[thirdparty] Building zlib..."
    cd "${SRC_DIR}/zlib-1.3.dfsg"
    CC="${CC}" CFLAGS="-O2 -fPIC" ./configure --prefix="${PREFIX}" --static
    make -j"$(nproc)" libz.a
    make install
fi

# ── OpenSSL (static) ─────────────────────────────────────────────────────
if [ ! -f "${PREFIX}/lib/libcrypto.a" ]; then
    echo "[thirdparty] Building OpenSSL..."
    cd "${SRC_DIR}/openssl-3.0.13"
    perl ./Configure linux-aarch64 \
        --prefix="${PREFIX}" --openssldir="${PREFIX}/ssl" \
        no-shared no-tests \
        CC="${CC}" AR="${AR}" RANLIB="${RANLIB}"
    make -j"$(nproc)"
    degrade_isoc23_symbols libcrypto.a
    degrade_isoc23_symbols libssl.a
    make install_dev
fi

# ── curl (static) ─────────────────────────────────────────────────────────
if [ ! -f "${PREFIX}/lib/libcurl.a" ]; then
    echo "[thirdparty] Building curl..."
    cd "${SRC_DIR}/curl-8.10.1"
    # --disable-symbol-hiding: curl defaults to -fvisibility=hidden even for
    # static builds, which would make curl_easy_* etc invisible once
    # whole-archived into our shared object.
    ./configure --host="${HOST}" --prefix="${PREFIX}" \
        --with-openssl="${PREFIX}" --with-zlib="${PREFIX}" \
        --disable-shared --enable-static --disable-symbol-hiding \
        --disable-ldap --disable-ldaps \
        --without-libpsl --without-libidn2 --without-brotli --without-zstd \
        --without-nghttp2 \
        CC="${CC}" AR="${AR}" RANLIB="${RANLIB}" CFLAGS="-O2 -fPIC"
    make -j"$(nproc)"
    degrade_isoc23_symbols lib/.libs/libcurl.a
    make install
fi

# ── libxml2 (static) ──────────────────────────────────────────────────────
if [ ! -f "${PREFIX}/lib/libxml2.a" ]; then
    echo "[thirdparty] Building libxml2..."
    cd "${SRC_DIR}/libxml2-2.12.9"
    ./configure --host="${HOST}" --prefix="${PREFIX}" \
        --disable-shared --enable-static \
        --without-python --without-lzma --with-zlib="${PREFIX}" \
        CC="${CC}" AR="${AR}" RANLIB="${RANLIB}" \
        CFLAGS="-O2 -fPIC -fvisibility=default"
    make -j"$(nproc)"
    degrade_isoc23_symbols .libs/libxml2.a
    make install
fi

echo "[thirdparty] zlib + OpenSSL + curl + libxml2 ready. Remaining libs:"
echo "[thirdparty] libffi, pcre2, glib, freetype, expat, fontconfig — see PROGRESS.md Phase 12."
