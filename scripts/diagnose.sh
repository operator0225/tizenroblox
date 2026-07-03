#!/bin/bash
# TizenRoblox Runtime Diagnostics
#
# Run this on the Tizen TV before launching to verify the environment.
# Reports missing libraries, misconfigured paths, and potential issues.
#
# Usage:
#   ssh root@<TV_IP> bash /opt/tizenroblox/scripts/diagnose.sh

INSTALL_DIR="${TIZENROBLOX_DIR:-/opt/tizenroblox}"
PASS=0
WARN=0
FAIL=0

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

ok()   { echo -e "${GREEN}[OK]${NC}   $*";  PASS=$((PASS+1)); }
warn() { echo -e "${YELLOW}[WARN]${NC} $*"; WARN=$((WARN+1)); }
fail() { echo -e "${RED}[FAIL]${NC} $*";   FAIL=$((FAIL+1)); }

find_lib() {
    local SONAME="$1"
    # Check our install dir first, then system
    for dir in "${INSTALL_DIR}/lib" \
               /usr/lib/aarch64-linux-gnu \
               /usr/lib64 /usr/lib \
               /lib/aarch64-linux-gnu /lib; do
        if [ -f "${dir}/${SONAME}" ] || [ -L "${dir}/${SONAME}" ]; then
            echo "${dir}/${SONAME}"
            return 0
        fi
    done
    # Also try ldconfig cache
    /sbin/ldconfig -p 2>/dev/null | grep " ${SONAME}" | awk '{print $NF}' | head -1
    return 1
}

echo "=== TizenRoblox Runtime Diagnostics ==="
echo "Install dir: ${INSTALL_DIR}"
echo "Date: $(date)"
echo ""

# ── Architecture check ────────────────────────────────────────────────────────
echo "--- Architecture ---"
ARCH=$(uname -m)
if [ "${ARCH}" = "aarch64" ]; then
    ok "Architecture: aarch64 (ARM64) ✓"
else
    fail "Architecture: ${ARCH} (expected aarch64)"
fi

# ── Kernel version ────────────────────────────────────────────────────────────
echo ""
echo "--- Kernel ---"
KVER=$(uname -r)
ok "Kernel: ${KVER}"

# Check uinput for virtual gamepad
if [ -c /dev/uinput ]; then
    ok "/dev/uinput present"
    if [ -w /dev/uinput ]; then
        ok "/dev/uinput writable (virtual gamepad will work)"
    else
        warn "/dev/uinput not writable — run as root or add to 'input' group"
    fi
else
    fail "/dev/uinput not found — virtual gamepad will not work"
    echo "     Try: modprobe uinput"
fi

# ── ELF interpreter ──────────────────────────────────────────────────────────
echo ""
echo "--- ELF Interpreter ---"
INTERP="/lib/ld-linux-aarch64.so.1"
if [ -f "${INTERP}" ] || [ -L "${INTERP}" ]; then
    ok "ELF interpreter: ${INTERP}"
else
    ALT_INTERP=""
    for p in /lib/aarch64-linux-gnu/ld-linux-aarch64.so.1 \
              /usr/lib/ld-linux-aarch64.so.1 \
              /usr/lib64/ld-linux-aarch64.so.1; do
        if [ -f "${p}" ]; then ALT_INTERP="${p}"; break; fi
    done
    if [ -n "${ALT_INTERP}" ]; then
        warn "ELF interpreter at ${ALT_INTERP} (not /lib/) — launcher will symlink"
    else
        fail "ELF interpreter ld-linux-aarch64.so.1 not found — sober will not start"
        echo "     Try: ln -sf \$(find / -name ld-linux-aarch64.so.1 2>/dev/null | head -1) /lib/"
    fi
fi

# ── Sober binary ─────────────────────────────────────────────────────────────
echo ""
echo "--- Sober Binary ---"
SOBER="${INSTALL_DIR}/bin/sober"
if [ -x "${SOBER}" ]; then
    SIZE=$(du -h "${SOBER}" | cut -f1)
    ok "sober binary: ${SOBER} (${SIZE})"
else
    fail "sober binary not found at ${SOBER}"
    echo "     Run: bash scripts/extract_sober.sh <your.flatpak>"
fi

for f in libloader.so libbadcpu.so "subprojects/mimalloc/libmimalloc.so.3"; do
    if [ -f "${INSTALL_DIR}/bin/${f}" ]; then
        ok "Sober bundled lib: ${INSTALL_DIR}/bin/${f}"
    else
        fail "Missing bundled lib: ${INSTALL_DIR}/bin/${f}"
    fi
done

# ── Display (Wayland) ─────────────────────────────────────────────────────────
echo ""
echo "--- Wayland Display ---"
WAYLAND_FOUND=0
for socket in "wayland-0" "wayland-1"; do
    for dir in "/run/display" "/tmp/.RTE" "/run/user/5000"; do
        if [ -S "${dir}/${socket}" ]; then
            ok "Wayland socket: ${dir}/${socket}"
            WAYLAND_FOUND=1
            break 2
        fi
    done
done
[ "${WAYLAND_FOUND}" -eq 0 ] && fail "No Wayland socket found (tried /run/display, /tmp/.RTE, /run/user/5000)"

for lib in libwayland-client.so.0 libwayland-egl.so.1 libwayland-cursor.so.0; do
    path=$(find_lib "${lib}")
    if [ -n "${path}" ]; then
        ok "${lib}: ${path}"
    elif [ "${lib}" = "libwayland-cursor.so.0" ]; then
        warn "${lib} not found (cursor shapes disabled, OK for TV)"
    else
        fail "${lib} not found — Wayland display will NOT work"
    fi
done

path=$(find_lib libxkbcommon.so.0)
if [ -n "${path}" ]; then
    ok "libxkbcommon.so.0: ${path}"
else
    warn "libxkbcommon.so.0 not found (keyboard input disabled, OK for TV)"
fi

# ── GPU / EGL ────────────────────────────────────────────────────────────────
echo ""
echo "--- GPU / EGL ---"
for lib in libEGL.so.1 libGLESv2.so.2; do
    path=$(find_lib "${lib}")
    if [ -n "${path}" ]; then
        ok "${lib}: ${path}"
    else
        fail "${lib} not found — 3D rendering will not work"
    fi
done

# Check EGL vendor
if command -v eglinfo >/dev/null 2>&1; then
    VENDOR=$(eglinfo 2>/dev/null | grep "EGL vendor" | head -1)
    ok "EGL info: ${VENDOR:-unknown}"
fi

# GPU device nodes
if ls /dev/dri/card* >/dev/null 2>&1; then
    ok "GPU device: $(ls /dev/dri/card* | head -1)"
elif ls /dev/mali* >/dev/null 2>&1; then
    ok "Mali GPU device: $(ls /dev/mali* | head -1)"
else
    warn "/dev/dri/card* and /dev/mali* not found — GPU access may fail"
fi

# ── Audio ─────────────────────────────────────────────────────────────────────
echo ""
echo "--- Audio ---"
path=$(find_lib libasound.so.2)
if [ -n "${path}" ]; then
    ok "ALSA (libasound.so.2): ${path}"
else
    warn "libasound.so.2 not found — no ALSA audio"
fi

path=$(find_lib libpulse.so.0)
if [ -n "${path}" ]; then
    ok "PulseAudio (libpulse.so.0): ${path}"
else
    warn "libpulse.so.0 not found — no PulseAudio (ALSA fallback if available)"
fi

# ── Required stub/shim libraries ─────────────────────────────────────────────
echo ""
echo "--- TizenRoblox Stubs/Shims ---"
STUB_LIBS="
libsecret-1.so.0
libdecor-0.so.0
libxml2.so.16
libcrypto.so.3
libgstreamer-1.0.so.0
libgstapp-1.0.so.0
libgstvideo-1.0.so.0
libglib-2.0.so.0
libgobject-2.0.so.0
libfontconfig.so.1
libfreetype.so.6
libcurl.so.4
libdbus-1.so.3
"
for lib in ${STUB_LIBS}; do
    if [ -f "${INSTALL_DIR}/lib/${lib}" ] || [ -L "${INSTALL_DIR}/lib/${lib}" ]; then
        ok "Stub: ${lib}"
    else
        fail "Missing stub: ${INSTALL_DIR}/lib/${lib}"
        echo "     Run: bash scripts/build.sh"
    fi
done

# ── D-Bus ─────────────────────────────────────────────────────────────────────
echo ""
echo "--- D-Bus ---"
path=$(find_lib libdbus-1.so.3)
if [ -n "${path}" ]; then
    ok "libdbus-1.so.3: ${path}"
else
    warn "libdbus-1.so.3 not found at standard paths — stub will attempt runtime load"
fi

if [ -n "${DBUS_SESSION_BUS_ADDRESS}" ]; then
    ok "D-Bus session: ${DBUS_SESSION_BUS_ADDRESS}"
elif command -v dbus-launch >/dev/null 2>&1; then
    ok "dbus-launch available (will be started by launcher)"
else
    warn "No D-Bus session and no dbus-launch — libsecret may not work"
fi

# ── Input ─────────────────────────────────────────────────────────────────────
echo ""
echo "--- Input Devices ---"
INPUT_MAPPER="${INSTALL_DIR}/bin/input_mapper"
if [ -x "${INPUT_MAPPER}" ]; then
    ok "input_mapper binary present"
else
    warn "input_mapper not found — Samsung remote won't act as gamepad"
fi

REMOTE_COUNT=0
GAMEPAD_COUNT=0
for dev in /dev/input/event*; do
    [ -c "${dev}" ] || continue
    name=""
    name=$(cat /sys/class/input/$(basename ${dev})/device/name 2>/dev/null || true)
    case "${name}" in
        *Samsung*|*TV*Remote*|*RC*|*CEC*)
            ok "Samsung remote: ${dev} (${name})"
            REMOTE_COUNT=$((REMOTE_COUNT+1))
            ;;
        *Xbox*|*PlayStation*|*DualShock*|*Gamepad*|*Controller*)
            ok "Gamepad: ${dev} (${name})"
            GAMEPAD_COUNT=$((GAMEPAD_COUNT+1))
            ;;
    esac
done
[ "${REMOTE_COUNT}" -eq 0 ] && warn "No Samsung TV remote detected (may appear after boot)"
[ "${GAMEPAD_COUNT}" -eq 0 ] && warn "No physical gamepad detected (optional)"

# ── System libraries (Tizen standard) ────────────────────────────────────────
echo ""
echo "--- System Libraries ---"
for lib in libc.so.6 libm.so.6 libz.so.1 libgcc_s.so.1; do
    path=$(find_lib "${lib}")
    if [ -n "${path}" ]; then
        ok "System: ${lib}"
    else
        fail "Missing system lib: ${lib} — Tizen is broken"
    fi
done

# ── Credentials ──────────────────────────────────────────────────────────────
echo ""
echo "--- Credentials ---"
CREDS_FILE="${INSTALL_DIR}/data/.tizenroblox_creds"
if [ -f "${CREDS_FILE}" ]; then
    ok "Credentials file exists: ${CREDS_FILE}"
else
    warn "No credentials file — run: bash scripts/setup.sh"
    echo "     Without credentials, Roblox will show login screen"
fi

# ── Summary ───────────────────────────────────────────────────────────────────
echo ""
echo "=== Summary ==="
echo -e "${GREEN}PASS${NC}: ${PASS}  ${YELLOW}WARN${NC}: ${WARN}  ${RED}FAIL${NC}: ${FAIL}"
echo ""

if [ "${FAIL}" -gt 0 ]; then
    echo -e "${RED}System is NOT ready. Fix the FAIL items above before launching.${NC}"
    exit 1
elif [ "${WARN}" -gt 0 ]; then
    echo -e "${YELLOW}System is mostly ready. Review WARN items for best experience.${NC}"
    exit 0
else
    echo -e "${GREEN}System is ready. Launch with: ${INSTALL_DIR}/bin/launch.sh${NC}"
    exit 0
fi
