#!/bin/bash
# TizenRoblox Build Script
# Cross-compiles for ARM64 (aarch64) targeting Tizen 9.0 TV
# Run from the project root on an x86_64 Linux host

set -e
cd "$(dirname "$0")/.."

TOOLCHAIN="$(pwd)/toolchain/aarch64-tizen.cmake"
BUILD_DIR="build"
DIST_DIR="$(pwd)/dist"

# ── Install cross-compiler if missing ────────────────────────────────────────
if ! command -v aarch64-linux-gnu-gcc >/dev/null 2>&1; then
    echo "[build] Installing aarch64 cross-compiler..."
    apt-get update -qq
    apt-get install -y -qq gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
        binutils-aarch64-linux-gnu pkg-config
fi

# ── Configure ────────────────────────────────────────────────────────────────
echo "[build] Configuring CMake for aarch64-tizen..."
mkdir -p "${BUILD_DIR}"
cmake -B "${BUILD_DIR}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="/opt/tizenroblox" \
    "$@"

# ── Build ─────────────────────────────────────────────────────────────────────
echo "[build] Building ($(nproc) jobs)..."
cmake --build "${BUILD_DIR}" --parallel "$(nproc)"

# ── Assemble dist/ ────────────────────────────────────────────────────────────
echo "[build] Assembling distribution package..."
mkdir -p "${DIST_DIR}/bin"
mkdir -p "${DIST_DIR}/lib"
mkdir -p "${DIST_DIR}/data"
mkdir -p "${DIST_DIR}/logs"

# Copy compiled binaries + stubs
cp -f "${BUILD_DIR}/dist/bin/input_mapper"    "${DIST_DIR}/bin/" 2>/dev/null || true
cp -f "${BUILD_DIR}/dist/bin/launch.sh"        "${DIST_DIR}/bin/" 2>/dev/null || \
    cp -f tizen/launch.sh                       "${DIST_DIR}/bin/"
chmod +x "${DIST_DIR}/bin/launch.sh"

# Copy stub shared libraries (SONAME-versioned, preserve symlinks)
cp -a "${BUILD_DIR}/dist/lib/"*.so* "${DIST_DIR}/lib/" 2>/dev/null || true

# Sober runtime binaries (real Flatpak binaries)
cp -f sober_bundle/bin/sober              "${DIST_DIR}/bin/"
# sober_services intentionally excluded (GTK4 not available on Tizen)

# Sober's bundled libraries
cp -f sober_bundle/libs/libloader.so      "${DIST_DIR}/lib/"
cp -f sober_bundle/libs/libbadcpu.so      "${DIST_DIR}/lib/"
cp -f sober_bundle/libs/libmimalloc.so.3  "${DIST_DIR}/lib/"
# Create SONAME symlink for mimalloc
ln -sf libmimalloc.so.3 "${DIST_DIR}/lib/libmimalloc.so" 2>/dev/null || true

# ── Create SONAME symlinks for stubs ─────────────────────────────────────────
(cd "${DIST_DIR}/lib" && {
    [ -f libsecret-1.so.0 ]  || ln -sf libsecret-1.so.0.0.0 libsecret-1.so.0  2>/dev/null || true
    [ -f libdecor-0.so.0 ]   || ln -sf libdecor-0.so.0.0.0  libdecor-0.so.0   2>/dev/null || true
    [ -f libxml2.so.16 ]     || ln -sf libxml2.so.16.0.0     libxml2.so.16     2>/dev/null || true
})

echo "[build] Build complete!"
echo "[build] Distribution at: ${DIST_DIR}"
ls -lh "${DIST_DIR}/bin/" "${DIST_DIR}/lib/"
