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

# Directory layout matching sober's RUNPATH ($ORIGIN/subprojects/mimalloc:$ORIGIN):
#   dist/bin/sober                         <- main binary
#   dist/bin/libloader.so                  <- found by $ORIGIN RUNPATH
#   dist/bin/libbadcpu.so                  <- found by $ORIGIN RUNPATH
#   dist/bin/subprojects/mimalloc/         <- found by $ORIGIN/subprojects/mimalloc
#     libmimalloc.so.3
#     libmimalloc.so -> libmimalloc.so.3
#   dist/lib/                              <- found by LD_LIBRARY_PATH (stubs)

mkdir -p "${DIST_DIR}/bin/subprojects/mimalloc"
mkdir -p "${DIST_DIR}/lib"
mkdir -p "${DIST_DIR}/data"
mkdir -p "${DIST_DIR}/logs"

# Input mapper and launcher
cp -f "${BUILD_DIR}/dist/bin/input_mapper" "${DIST_DIR}/bin/" 2>/dev/null || true
cp -f tizen/launch.sh "${DIST_DIR}/bin/"
chmod +x "${DIST_DIR}/bin/launch.sh"

# Stub shared libraries (LD_LIBRARY_PATH covers these)
cp -a "${BUILD_DIR}/dist/lib/"*.so* "${DIST_DIR}/lib/" 2>/dev/null || true

# Sober runtime: main binary
cp -f sober_bundle/bin/sober "${DIST_DIR}/bin/"
# sober_services excluded (GTK4 not available on Tizen)

# Sober's bundled libraries placed relative to sober binary (RUNPATH layout)
cp -f sober_bundle/libs/libloader.so     "${DIST_DIR}/bin/"
cp -f sober_bundle/libs/libbadcpu.so     "${DIST_DIR}/bin/"
cp -f sober_bundle/libs/libmimalloc.so.3 "${DIST_DIR}/bin/subprojects/mimalloc/"
(cd "${DIST_DIR}/bin/subprojects/mimalloc" && \
    ln -sf libmimalloc.so.3 libmimalloc.so 2>/dev/null || true)

# Also put them in lib/ as fallback (for LD_LIBRARY_PATH path)
cp -f sober_bundle/libs/libloader.so     "${DIST_DIR}/lib/"
cp -f sober_bundle/libs/libbadcpu.so     "${DIST_DIR}/lib/"
cp -f sober_bundle/libs/libmimalloc.so.3 "${DIST_DIR}/lib/"
ln -sf libmimalloc.so.3 "${DIST_DIR}/lib/libmimalloc.so" 2>/dev/null || true

echo "[build] Build complete!"
echo "[build] Distribution at: ${DIST_DIR}"
echo ""
echo "=== bin/ ==="
ls -lh "${DIST_DIR}/bin/"
echo ""
echo "=== lib/ (stubs) ==="
ls -lh "${DIST_DIR}/lib/"
