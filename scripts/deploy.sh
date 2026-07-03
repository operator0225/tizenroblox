#!/bin/bash
# TizenRoblox TV Deployment Script
# Copies the built distribution to a Samsung Tizen TV via SSH
# Usage: ./scripts/deploy.sh <TV_IP> [SSH_PORT]

set -e
cd "$(dirname "$0")/.."

TV_IP="${1:-}"
SSH_PORT="${2:-22}"
TV_USER="${TV_USER:-root}"
INSTALL_DIR="/opt/tizenroblox"
DIST_DIR="$(pwd)/dist"

if [ -z "${TV_IP}" ]; then
    echo "Usage: $0 <TV_IP> [SSH_PORT]"
    echo "Example: $0 192.168.1.100"
    echo "         TV_USER=owner $0 192.168.1.100 2222"
    exit 1
fi

SSH_OPTS="-p ${SSH_PORT} -o StrictHostKeyChecking=no -o ConnectTimeout=10"
SCP_OPTS="-P ${SSH_PORT} -o StrictHostKeyChecking=no"

echo "[deploy] Target: ${TV_USER}@${TV_IP}:${INSTALL_DIR}"

# ── Verify dist/ exists ───────────────────────────────────────────────────────
if [ ! -d "${DIST_DIR}" ]; then
    echo "[deploy] dist/ not found — running build first..."
    bash scripts/build.sh
fi

if [ ! -f "${DIST_DIR}/bin/sober" ]; then
    echo "ERROR: ${DIST_DIR}/bin/sober not found. Run build.sh first."
    exit 1
fi

# ── Check TV connectivity ─────────────────────────────────────────────────────
echo "[deploy] Testing SSH connection to TV..."
ssh ${SSH_OPTS} "${TV_USER}@${TV_IP}" "echo connected" || {
    echo "ERROR: Cannot SSH to TV."
    echo "  - Enable Developer Mode on TV: Settings → Support → Developer Mode → ON"
    echo "  - Enter the host PC IP, restart TV, then try again"
    exit 1
}

# ── Create install directory on TV ────────────────────────────────────────────
echo "[deploy] Creating ${INSTALL_DIR} on TV..."
ssh ${SSH_OPTS} "${TV_USER}@${TV_IP}" "
    mkdir -p ${INSTALL_DIR}/bin
    mkdir -p ${INSTALL_DIR}/lib
    mkdir -p ${INSTALL_DIR}/data/.local/share/sober
    mkdir -p ${INSTALL_DIR}/logs
"

# ── Copy distribution ─────────────────────────────────────────────────────────
echo "[deploy] Copying binaries..."
scp ${SCP_OPTS} "${DIST_DIR}/bin/sober"        "${TV_USER}@${TV_IP}:${INSTALL_DIR}/bin/"
scp ${SCP_OPTS} "${DIST_DIR}/bin/input_mapper" "${TV_USER}@${TV_IP}:${INSTALL_DIR}/bin/" 2>/dev/null || true
scp ${SCP_OPTS} "${DIST_DIR}/bin/launch.sh"    "${TV_USER}@${TV_IP}:${INSTALL_DIR}/bin/"

# Sober bundled libs (RUNPATH: $ORIGIN)
for f in libloader.so libbadcpu.so; do
    [ -f "${DIST_DIR}/bin/${f}" ] && \
        scp ${SCP_OPTS} "${DIST_DIR}/bin/${f}" "${TV_USER}@${TV_IP}:${INSTALL_DIR}/bin/"
done
# mimalloc (RUNPATH: $ORIGIN/subprojects/mimalloc)
if [ -f "${DIST_DIR}/bin/subprojects/mimalloc/libmimalloc.so.3" ]; then
    ssh ${SSH_OPTS} "${TV_USER}@${TV_IP}" \
        "mkdir -p ${INSTALL_DIR}/bin/subprojects/mimalloc"
    scp ${SCP_OPTS} \
        "${DIST_DIR}/bin/subprojects/mimalloc/libmimalloc.so.3" \
        "${TV_USER}@${TV_IP}:${INSTALL_DIR}/bin/subprojects/mimalloc/"
fi

echo "[deploy] Copying scripts..."
ssh ${SSH_OPTS} "${TV_USER}@${TV_IP}" "mkdir -p ${INSTALL_DIR}/scripts"
scp ${SCP_OPTS} scripts/diagnose.sh "${TV_USER}@${TV_IP}:${INSTALL_DIR}/scripts/"

echo "[deploy] Copying libraries..."
scp ${SCP_OPTS} "${DIST_DIR}/lib/"*.so*  "${TV_USER}@${TV_IP}:${INSTALL_DIR}/lib/" 2>/dev/null || true

# ── Set permissions ───────────────────────────────────────────────────────────
echo "[deploy] Setting permissions..."
ssh ${SSH_OPTS} "${TV_USER}@${TV_IP}" "
    chmod +x ${INSTALL_DIR}/bin/sober
    chmod +x ${INSTALL_DIR}/bin/launch.sh
    chmod +x ${INSTALL_DIR}/bin/input_mapper 2>/dev/null || true

    # input_mapper needs uinput access
    # On Tizen, /dev/uinput may need chmod or cap_net_admin
    if [ -c /dev/uinput ]; then
        chmod 0660 /dev/uinput 2>/dev/null || true
    fi

    # Recreate SONAME symlinks on TV
    cd ${INSTALL_DIR}/lib
    for f in *.so.*; do
        base=\$(echo \$f | sed 's/\\.so\\.[0-9].*//')
        soname=\$(echo \$f | sed 's/\\(.*\\.so\\.[0-9]*\\).*/\\1/')
        [ ! -L \"\${soname}\" ] && ln -sf \"\$f\" \"\${soname}\" 2>/dev/null || true
    done
"

# ── Verify on TV ──────────────────────────────────────────────────────────────
echo "[deploy] Verifying installation..."
ssh ${SSH_OPTS} "${TV_USER}@${TV_IP}" "
    echo '=== Installed files ==='
    ls -lh ${INSTALL_DIR}/bin/ ${INSTALL_DIR}/lib/
    echo
    echo '=== Library check ==='
    LD_LIBRARY_PATH=${INSTALL_DIR}/lib ldd ${INSTALL_DIR}/bin/sober 2>&1 | head -30
"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " Deployment complete!"
echo ""
echo " 1. Run diagnostics:"
echo "    ssh ${TV_USER}@${TV_IP} bash ${INSTALL_DIR}/scripts/diagnose.sh"
echo ""
echo " 2. Launch (if diagnostics pass):"
echo "    ssh ${TV_USER}@${TV_IP} ${INSTALL_DIR}/bin/launch.sh"
echo ""
echo " 3. View logs:"
echo "    ssh ${TV_USER}@${TV_IP} tail -f ${INSTALL_DIR}/logs/tizenroblox.log"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
