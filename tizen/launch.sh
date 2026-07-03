#!/bin/bash
# TizenRoblox Launcher
# Launches Sober (Roblox ARM64 runtime) on Tizen 9.0 TV
# Bypasses sober_services (GTK4/WebKitGTK not available on Tizen)

set -e

INSTALL_DIR="${TIZENROBLOX_DIR:-/opt/tizenroblox}"
SOBER_BIN="${INSTALL_DIR}/bin/sober"
INPUT_MAPPER="${INSTALL_DIR}/bin/input_mapper"
LIB_DIR="${INSTALL_DIR}/lib"
LOG_FILE="${INSTALL_DIR}/logs/tizenroblox.log"
PID_FILE="/tmp/tizenroblox.pid"
INPUT_PID_FILE="/tmp/tizenroblox_input.pid"

# ── Logging ──────────────────────────────────────────────────────────────────
mkdir -p "${INSTALL_DIR}/logs"
exec >> "${LOG_FILE}" 2>&1
echo "[$(date '+%Y-%m-%d %H:%M:%S')] TizenRoblox starting..."

# ── Sanity checks ─────────────────────────────────────────────────────────────
if [ ! -f "${SOBER_BIN}" ]; then
    echo "ERROR: sober binary not found at ${SOBER_BIN}"
    exit 1
fi

if [ -f "${PID_FILE}" ]; then
    OLD_PID=$(cat "${PID_FILE}")
    if kill -0 "${OLD_PID}" 2>/dev/null; then
        echo "TizenRoblox already running (PID ${OLD_PID})"
        exit 0
    fi
    rm -f "${PID_FILE}"
fi

# ── Wayland environment ───────────────────────────────────────────────────────
# Tizen Enlightenment Wayland compositor
if [ -z "${WAYLAND_DISPLAY}" ]; then
    # Tizen 9: /run/display/wayland-0 or /tmp/.RTE/wayland-0
    for socket in "wayland-0" "wayland-1"; do
        if [ -S "/run/display/${socket}" ]; then
            export WAYLAND_DISPLAY="${socket}"
            export XDG_RUNTIME_DIR="/run/display"
            break
        elif [ -S "/tmp/.RTE/${socket}" ]; then
            export WAYLAND_DISPLAY="${socket}"
            export XDG_RUNTIME_DIR="/tmp/.RTE"
            break
        elif [ -S "${XDG_RUNTIME_DIR:-/run/user/5000}/${socket}" ]; then
            export WAYLAND_DISPLAY="${socket}"
            break
        fi
    done
fi

if [ -z "${WAYLAND_DISPLAY}" ]; then
    echo "WARNING: No Wayland display socket found — trying wayland-0 anyway"
    export WAYLAND_DISPLAY="wayland-0"
fi

export XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-/run/user/5000}"
echo "[launch] WAYLAND_DISPLAY=${WAYLAND_DISPLAY}"
echo "[launch] XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR}"

# ── Home / data directories ───────────────────────────────────────────────────
# Tizen app data path (writable by the app user)
SOBER_HOME="${INSTALL_DIR}/data"
mkdir -p "${SOBER_HOME}/.local/share/sober"
mkdir -p "${SOBER_HOME}/.config/sober"
mkdir -p "${SOBER_HOME}/.cache/sober"

export HOME="${SOBER_HOME}"
export XDG_DATA_HOME="${SOBER_HOME}/.local/share"
export XDG_CONFIG_HOME="${SOBER_HOME}/.config"
export XDG_CACHE_HOME="${SOBER_HOME}/.cache"

# ── Library path ──────────────────────────────────────────────────────────────
# Order: our stubs first, then sober's bundled libs, then system libs
SYSTEM_LIB_PATHS="/usr/lib/aarch64-linux-gnu:/usr/lib64:/usr/lib:/lib/aarch64-linux-gnu:/lib"

# Build LD_LIBRARY_PATH:
#   1. Our stubs (lib/) - take priority for SONAME-shimmed libs
#   2. sober's bundled libs (bin/) - libloader.so, libbadcpu.so
#   3. mimalloc subdir - matches sober RUNPATH $ORIGIN/subprojects/mimalloc
#   4. System libs
export LD_LIBRARY_PATH="${LIB_DIR}:${INSTALL_DIR}/bin:${INSTALL_DIR}/bin/subprojects/mimalloc:${SYSTEM_LIB_PATHS}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "[launch] LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"

# ── libxml2.so.16 compatibility ───────────────────────────────────────────────
# Sober requires libxml2.so.16 (GNOME Platform 50 / libxml2 3.x SONAME)
# Tizen may have libxml2.so.2 (2.x) — our shim bridges the gap
if [ ! -f "${LIB_DIR}/libxml2.so.16" ]; then
    # Try to find system libxml2 and create symlink
    for f in /usr/lib/aarch64-linux-gnu/libxml2.so.2 \
              /usr/lib64/libxml2.so.2 \
              /usr/lib/libxml2.so.2; do
        if [ -f "$f" ]; then
            echo "[launch] Creating libxml2.so.16 symlink → $f"
            ln -sf "$f" "${LIB_DIR}/libxml2.so.16" 2>/dev/null || true
            break
        fi
    done
fi

# ── GLib/GObject stubs ────────────────────────────────────────────────────────
# Check if Tizen has GLib; if not, sober won't launch
for lib in libglib-2.0.so.0 libgobject-2.0.so.0; do
    if ! /sbin/ldconfig -p 2>/dev/null | grep -q "${lib}" && \
       [ ! -f "${LIB_DIR}/${lib}" ]; then
        echo "WARNING: ${lib} not found — Sober may fail"
    fi
done

# ── GPU / EGL environment ─────────────────────────────────────────────────────
# Force hardware EGL (no software fallback)
export EGL_PLATFORM="wayland"
export MESA_GL_VERSION_OVERRIDE=""

# Samsung NQ4 AI Gen3 GPU (Mali or custom)
# Disable GPU debugging overhead
export LIBGL_DEBUG=""
export EGL_LOG_LEVEL="fatal"

# ── Sober-specific environment ────────────────────────────────────────────────
# Tell sober this is a TV environment
export SOBER_DISPLAY_MODE="fullscreen"
export SOBER_DISABLE_SERVICES="1"    # Skip GTK4 sober_services launcher
export SOBER_SKIP_UPDATE_CHECK="1"   # No auto-update on TV

# Roblox data directory
export SOBER_DATA_DIR="${SOBER_HOME}/.local/share/sober"

# ── D-Bus ────────────────────────────────────────────────────────────────────
# Start a D-Bus session if not present (sober needs it for libsecret)
if [ -z "${DBUS_SESSION_BUS_ADDRESS}" ]; then
    if command -v dbus-launch >/dev/null 2>&1; then
        eval $(dbus-launch --sh-syntax 2>/dev/null) || true
    fi
fi

# ── Input mapper ──────────────────────────────────────────────────────────────
cleanup() {
    echo "[launch] Shutting down TizenRoblox..."
    # Kill input mapper
    if [ -f "${INPUT_PID_FILE}" ]; then
        INPUT_PID=$(cat "${INPUT_PID_FILE}")
        kill "${INPUT_PID}" 2>/dev/null || true
        rm -f "${INPUT_PID_FILE}"
    fi
    rm -f "${PID_FILE}"
    if [ -n "${DBUS_SESSION_BUS_PID}" ]; then
        kill "${DBUS_SESSION_BUS_PID}" 2>/dev/null || true
    fi
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] TizenRoblox stopped."
}
trap cleanup EXIT INT TERM

if [ -f "${INPUT_MAPPER}" ]; then
    echo "[launch] Starting Samsung TV input mapper..."
    "${INPUT_MAPPER}" &
    INPUT_MAPPER_PID=$!
    echo "${INPUT_MAPPER_PID}" > "${INPUT_PID_FILE}"
    # Give uinput device time to register
    sleep 0.5
    echo "[launch] Input mapper PID: ${INPUT_MAPPER_PID}"
else
    echo "WARNING: input_mapper not found at ${INPUT_MAPPER} — gamepad won't work"
fi

# ── Launch Sober ─────────────────────────────────────────────────────────────
echo "$$" > "${PID_FILE}"
echo "[launch] Launching Sober runtime..."
echo "[launch] Binary: ${SOBER_BIN}"

# Pass Roblox place ID or join args from environment/command line
SOBER_ARGS="${@}"

exec "${SOBER_BIN}" ${SOBER_ARGS}
