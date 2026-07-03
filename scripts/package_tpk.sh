#!/bin/bash
# TizenRoblox .tpk Packager
#
# Creates a Tizen TV application package (.tpk) for installation
# via the Tizen Store or sideloading with SDB.
#
# Requires:
#   - Tizen Studio CLI (tizen command) installed in PATH
#     OR manual zip-based packaging (see below)
#   - Development certificate (created in Tizen Studio Certificate Manager)
#
# Usage:
#   ./scripts/package_tpk.sh [cert_profile_name]
#
# Without Tizen Studio, this creates an unsigned package for dev sideloading.

set -e
cd "$(dirname "$0")/.."

CERT_PROFILE="${1:-}"
DIST_DIR="dist"
PKG_DIR="pkg_work"
OUTPUT_TPK="tizenroblox.tpk"
APP_ID="org.tizenroblox.app"
APP_DIR="${PKG_DIR}/${APP_ID}"

# ── Build first if needed ─────────────────────────────────────────────────────
if [ ! -f "${DIST_DIR}/bin/sober" ]; then
    echo "[pkg] Building..."
    bash scripts/build.sh
fi

# ── Create package structure ──────────────────────────────────────────────────
echo "[pkg] Creating package structure..."
rm -rf "${PKG_DIR}"
mkdir -p "${APP_DIR}/bin"
mkdir -p "${APP_DIR}/lib"
mkdir -p "${APP_DIR}/data"
mkdir -p "${APP_DIR}/res"

# Copy binaries
cp "${DIST_DIR}/bin/sober"         "${APP_DIR}/bin/"
cp "${DIST_DIR}/bin/input_mapper"  "${APP_DIR}/bin/" 2>/dev/null || true
cp "${DIST_DIR}/bin/launch.sh"     "${APP_DIR}/bin/"
chmod +x "${APP_DIR}/bin/"*

# Sober bundled libs — must be beside the sober binary (RUNPATH: $ORIGIN)
for f in libloader.so libbadcpu.so; do
    [ -f "${DIST_DIR}/bin/${f}" ] && cp "${DIST_DIR}/bin/${f}" "${APP_DIR}/bin/" || true
    [ -f "sober_bundle/libs/${f}" ] && cp "sober_bundle/libs/${f}" "${APP_DIR}/bin/" || true
done

# mimalloc lives in RUNPATH subprojects/mimalloc/
mkdir -p "${APP_DIR}/bin/subprojects/mimalloc"
MIMALLOC_SRC=""
[ -f "${DIST_DIR}/bin/subprojects/mimalloc/libmimalloc.so.3" ] && \
    MIMALLOC_SRC="${DIST_DIR}/bin/subprojects/mimalloc/libmimalloc.so.3"
[ -z "${MIMALLOC_SRC}" ] && [ -f "sober_bundle/libs/libmimalloc.so.3" ] && \
    MIMALLOC_SRC="sober_bundle/libs/libmimalloc.so.3"
if [ -n "${MIMALLOC_SRC}" ]; then
    cp "${MIMALLOC_SRC}" "${APP_DIR}/bin/subprojects/mimalloc/"
    (cd "${APP_DIR}/bin/subprojects/mimalloc" && \
        ln -sf libmimalloc.so.3 libmimalloc.so 2>/dev/null || true)
fi

# Copy stub/shim libraries (preserve symlinks)
cp -a "${DIST_DIR}/lib/"*.so* "${APP_DIR}/lib/" 2>/dev/null || true

# Diagnostic script
mkdir -p "${APP_DIR}/scripts"
cp scripts/diagnose.sh "${APP_DIR}/scripts/" 2>/dev/null || true

# Copy manifest
cp "tizen/pkg/tizen-manifest.xml"  "${APP_DIR}/"

# ── Update manifest paths to match Tizen app sandbox ─────────────────────────
# In a .tpk, the app's root is /opt/usr/apps/<appid>/
sed -i "s|/opt/tizenroblox/bin|/opt/usr/apps/${APP_ID}/bin|g" \
    "${APP_DIR}/tizen-manifest.xml" 2>/dev/null || true

# ── Placeholder icon ─────────────────────────────────────────────────────────
# Create a minimal 1x1 PNG placeholder icon (replace with real icon)
python3 -c "
import base64, sys
# Minimal 1x1 red PNG
png = base64.b64decode(
    'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVQI12NgYGBg'
    'AAAFAgQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA' + '=' * 8)
open('${APP_DIR}/res/tizenroblox-86x86.png', 'wb').write(png)
open('${APP_DIR}/res/tizenroblox.png', 'wb').write(png)
" 2>/dev/null || \
    touch "${APP_DIR}/res/tizenroblox.png" "${APP_DIR}/res/tizenroblox-86x86.png"

# ── Package ───────────────────────────────────────────────────────────────────
if command -v tizen >/dev/null 2>&1 && [ -n "${CERT_PROFILE}" ]; then
    # Use Tizen Studio CLI for signing
    echo "[pkg] Signing with profile: ${CERT_PROFILE}..."
    tizen package \
        -t tpk \
        -s "${CERT_PROFILE}" \
        -- "${APP_DIR}"
    mv "${APP_DIR}/"*.tpk "./${OUTPUT_TPK}"
    echo "[pkg] ✅ Signed package: ./${OUTPUT_TPK}"
else
    # Unsigned package (dev sideloading)
    # -y: store symlinks as symlinks instead of dereferencing them.
    # Without this, each libfoo.so -> libfoo.so.N -> libfoo.so.N.M.P SONAME
    # symlink chain gets copied as 3 full duplicate files (observed: 77MB
    # uncompressed instead of the true ~30MB, once per bundled library).
    echo "[pkg] Creating unsigned .tpk (ZIP format)..."
    (cd "${APP_DIR}" && zip -ry "../../${OUTPUT_TPK}" .)
    echo "[pkg] ✅ Unsigned package: ./${OUTPUT_TPK}"
    echo ""
    echo "NOTE: Unsigned packages require developer mode on TV."
    echo "      Install with: sdb install ${OUTPUT_TPK}"
    echo "      Or sign with: tizen package -t tpk -s <profile> -- ${APP_DIR}"
fi

# ── Cleanup ───────────────────────────────────────────────────────────────────
rm -rf "${PKG_DIR}"

echo ""
echo "Package size: $(du -sh ${OUTPUT_TPK} | cut -f1)"
echo ""
echo "Install commands:"
echo "  sdb connect <TV_IP>:26101"
echo "  sdb install ${OUTPUT_TPK}"
