#!/bin/bash
# TizenRoblox TV Deployment via SDB (Smart Development Bridge)
#
# SDB is Tizen's equivalent of ADB for Samsung TV development.
# Some Samsung TVs use SDB instead of SSH.
#
# Prerequisites:
#   - Enable Developer Mode on TV: Settings → Support → Developer Mode
#   - Enter your PC's IP address, restart TV
#   - Download Tizen Studio (includes sdb): https://developer.tizen.org/development/tizen-studio
#   - Or install sdb separately from Tizen SDK
#
# Usage:
#   ./scripts/deploy_sdb.sh [TV_IP]

set -e
cd "$(dirname "$0")/.."

TV_IP="${1:-}"
INSTALL_DIR="/opt/tizenroblox"
DIST_DIR="$(pwd)/dist"

if [ -z "${TV_IP}" ]; then
    echo "Usage: $0 <TV_IP>"
    echo ""
    echo "Example: $0 192.168.1.100"
    echo ""
    echo "If you don't know your TV's IP:"
    echo "  Settings → Network → Network Status → IP Settings"
    exit 1
fi

# ── Check sdb is installed ────────────────────────────────────────────────────
if ! command -v sdb >/dev/null 2>&1; then
    echo "ERROR: sdb not found."
    echo "Install Tizen Studio from: https://developer.tizen.org/development/tizen-studio"
    echo "Or use deploy.sh (SSH-based) instead."
    exit 1
fi

# ── Connect ───────────────────────────────────────────────────────────────────
echo "[sdb] Connecting to ${TV_IP}:26101..."
sdb connect "${TV_IP}:26101"

# Wait for connection
sleep 1

if ! sdb devices | grep -q "${TV_IP}"; then
    echo "ERROR: Could not connect to TV at ${TV_IP}"
    echo "Ensure Developer Mode is enabled and PC IP is configured"
    exit 1
fi

echo "[sdb] Connected!"

# ── Build if needed ───────────────────────────────────────────────────────────
if [ ! -d "${DIST_DIR}" ] || [ ! -f "${DIST_DIR}/bin/sober" ]; then
    echo "[sdb] Building first..."
    bash scripts/build.sh
fi

# ── Create directories on TV ─────────────────────────────────────────────────
echo "[sdb] Creating install directory..."
sdb shell "mkdir -p ${INSTALL_DIR}/bin ${INSTALL_DIR}/lib ${INSTALL_DIR}/data/.local/share/sober ${INSTALL_DIR}/logs"

# ── Push files ────────────────────────────────────────────────────────────────
echo "[sdb] Pushing binaries..."
sdb push "${DIST_DIR}/bin/sober"        "${INSTALL_DIR}/bin/sober"
sdb push "${DIST_DIR}/bin/input_mapper" "${INSTALL_DIR}/bin/input_mapper" 2>/dev/null || true
sdb push "${DIST_DIR}/bin/launch.sh"    "${INSTALL_DIR}/bin/launch.sh"

echo "[sdb] Pushing libraries..."
for lib in "${DIST_DIR}/lib/"*; do
    # Skip symlinks - recreate them on device
    [ -L "$lib" ] && continue
    sdb push "$lib" "${INSTALL_DIR}/lib/"
done

# ── Set permissions ───────────────────────────────────────────────────────────
echo "[sdb] Setting permissions..."
sdb shell "
    chmod +x ${INSTALL_DIR}/bin/sober
    chmod +x ${INSTALL_DIR}/bin/launch.sh
    chmod +x ${INSTALL_DIR}/bin/input_mapper 2>/dev/null || true

    # Recreate versioned symlinks
    cd ${INSTALL_DIR}/lib
    for f in *.so.*.*.*; do
        soname=\$(echo \$f | sed 's/\\.so\\.[0-9]*\\.[0-9]*\\.[0-9]*//' | sed 's/\$//')
        # e.g. libsecret-1.so.0.0.0 -> soname is libsecret-1.so.0
        base=\$(echo \$f | sed 's/\\.so\\.[0-9.]*$//')
        soname2=\$(echo \$f | sed 's/\\.[0-9]*\\.[0-9]*$//')
        ln -sf \$f \$soname2 2>/dev/null || true
        ln -sf \$soname2 \${base}.so 2>/dev/null || true
    done

    # uinput permission
    [ -c /dev/uinput ] && chmod 0660 /dev/uinput 2>/dev/null || true
"

# ── Verify ────────────────────────────────────────────────────────────────────
echo "[sdb] Verifying installation..."
sdb shell "ls -lh ${INSTALL_DIR}/bin/ ${INSTALL_DIR}/lib/"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " Deployment complete via SDB!"
echo " To launch:"
echo "   sdb shell ${INSTALL_DIR}/bin/launch.sh"
echo " To view logs:"
echo "   sdb shell cat ${INSTALL_DIR}/logs/tizenroblox.log"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
