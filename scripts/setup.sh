#!/bin/bash
# TizenRoblox First-Run Setup
# Sets up the Roblox credential file that sober reads via our libsecret stub.
#
# Background: sober uses GNOME libsecret to store the Roblox session token.
# Our stub replaces this with a plain file at: $HOME/.tizenroblox_creds
#
# Options:
#   1. Automatic: run this script on a Linux desktop with real Sober installed,
#      then copy the resulting credentials file to the TV.
#   2. Manual: enter your Roblox token directly.
#   3. Import: copy from an existing Sober installation.

set -e

CREDS_FILE="${HOME}/.tizenroblox_creds"
INSTALL_DIR="${TIZENROBLOX_DIR:-/opt/tizenroblox}"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " TizenRoblox — Credential Setup"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# ── Option 1: Import from real Sober (GNOME keyring) ──────────────────────
if command -v secret-tool >/dev/null 2>&1; then
    echo "[setup] Found GNOME secret-tool — trying to import Sober credentials..."
    # Sober stores credentials under various schema attributes
    for schema in "org.vinegarhq.Sober" "Sober" "roblox"; do
        TOKEN=$(secret-tool lookup application sober 2>/dev/null || \
                secret-tool lookup xdg:schema "org.vinegarhq.Sober" 2>/dev/null || \
                true)
        if [ -n "${TOKEN}" ]; then
            echo "${TOKEN}" > "${CREDS_FILE}"
            chmod 600 "${CREDS_FILE}"
            echo "[setup] ✅ Credentials imported from GNOME keyring"
            echo "[setup] Stored at: ${CREDS_FILE}"
            exit 0
        fi
    done
    echo "[setup] No Sober credentials found in GNOME keyring."
fi

# ── Option 2: Import from Sober data directory ─────────────────────────────
SOBER_DATA="${XDG_DATA_HOME:-$HOME/.local/share}/sober"
if [ -f "${SOBER_DATA}/credentials" ] || [ -f "${SOBER_DATA}/.credentials" ]; then
    CRED_FILE=$(ls "${SOBER_DATA}/"*.cred 2>/dev/null | head -1 || true)
    if [ -n "${CRED_FILE}" ]; then
        cp "${CRED_FILE}" "${CREDS_FILE}"
        chmod 600 "${CREDS_FILE}"
        echo "[setup] ✅ Credentials imported from Sober data dir"
        exit 0
    fi
fi

# ── Option 3: Manual token entry ──────────────────────────────────────────
echo ""
echo "No existing credentials found. You can enter your Roblox token manually."
echo ""
echo "HOW TO GET YOUR ROBLOX TOKEN:"
echo "  1. Log in to roblox.com in your browser"
echo "  2. Open DevTools → Application → Cookies → .ROBLOSECURITY"
echo "  3. Copy the cookie value (very long string starting with _|WARNING...)"
echo ""
echo "Your token will be stored securely at: ${CREDS_FILE}"
echo ""

read -r -p "Paste your .ROBLOSECURITY token (or press Enter to skip): " TOKEN

if [ -n "${TOKEN}" ]; then
    echo "${TOKEN}" > "${CREDS_FILE}"
    chmod 600 "${CREDS_FILE}"
    echo ""
    echo "[setup] ✅ Token saved to ${CREDS_FILE}"
else
    echo ""
    echo "[setup] Skipped. You can run this script again later."
    echo "[setup] Or manually create: ${CREDS_FILE} with your token"
    exit 0
fi

# ── Option 4: Copy to TV ──────────────────────────────────────────────────
echo ""
read -r -p "Copy credentials to TV now? Enter TV IP (or press Enter to skip): " TV_IP

if [ -n "${TV_IP}" ]; then
    TV_USER="${TV_USER:-root}"
    SSH_PORT="${SSH_PORT:-22}"
    TV_HOME="${INSTALL_DIR}/data"

    echo "[setup] Copying credentials to ${TV_USER}@${TV_IP}:${TV_HOME}/.tizenroblox_creds"
    ssh -p "${SSH_PORT}" -o StrictHostKeyChecking=no "${TV_USER}@${TV_IP}" \
        "mkdir -p ${TV_HOME}"
    scp -P "${SSH_PORT}" -o StrictHostKeyChecking=no \
        "${CREDS_FILE}" "${TV_USER}@${TV_IP}:${TV_HOME}/.tizenroblox_creds"

    echo "[setup] ✅ Credentials copied to TV"
    echo "[setup] HOME on TV is set to ${TV_HOME} in launch.sh"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " Setup complete! Launch TizenRoblox with:"
echo "   ssh ${TV_USER:-root}@<TV_IP> ${INSTALL_DIR}/bin/launch.sh"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
