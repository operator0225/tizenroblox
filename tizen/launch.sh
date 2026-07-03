#!/bin/bash
# TizenRoblox Launcher
# Launches Sober (Roblox ARM64 runtime) on Tizen 9.0 TV
# Bypasses sober_services (GTK4/WebKitGTK not available on Tizen)

set -e

# Auto-detect install location: env override → Tizen .tpk sandbox → manual install
if [ -n "${TIZENROBLOX_DIR}" ]; then
    INSTALL_DIR="${TIZENROBLOX_DIR}"
elif [ -d "/opt/usr/apps/org.tizenroblox.app" ]; then
    INSTALL_DIR="/opt/usr/apps/org.tizenroblox.app"
else
    INSTALL_DIR="/opt/tizenroblox"
fi
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

# ── ELF interpreter check ────────────────────────────────────────────────────
# Sober is built for /lib/ld-linux-aarch64.so.1. If Tizen has it elsewhere,
# create a symlink so the kernel finds it.
if [ ! -f "/lib/ld-linux-aarch64.so.1" ] && [ ! -L "/lib/ld-linux-aarch64.so.1" ]; then
    for _interp in /lib/aarch64-linux-gnu/ld-linux-aarch64.so.1 \
                   /usr/lib/ld-linux-aarch64.so.1 \
                   /usr/lib64/ld-linux-aarch64.so.1; do
        if [ -f "${_interp}" ]; then
            echo "[launch] Symlinking ELF interpreter /lib/ld-linux-aarch64.so.1 → ${_interp}"
            mkdir -p /lib
            ln -sf "${_interp}" /lib/ld-linux-aarch64.so.1 2>/dev/null || \
                echo "[launch] WARNING: could not create ELF interpreter symlink (need root)"
            break
        fi
    done
fi

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

# ── Library symlink helpers ───────────────────────────────────────────────────

# try_symlink_lib SONAME path1 [path2...]
# Creates a symlink in lib/ only if it doesn't already exist there.
# Used for libraries Tizen has but not in the linker's default search path.
try_symlink_lib() {
    local SONAME="$1"
    shift
    if [ -f "${LIB_DIR}/${SONAME}" ] || [ -L "${LIB_DIR}/${SONAME}" ]; then
        return 0  # already present (our stub or a previously-created symlink)
    fi
    for candidate in "$@"; do
        if [ -f "${candidate}" ]; then
            echo "[launch] Symlinking ${SONAME} → ${candidate}"
            ln -sf "${candidate}" "${LIB_DIR}/${SONAME}" 2>/dev/null || true
            return 0
        fi
    done
    echo "[launch] WARNING: ${SONAME} not found on this TV"
    return 1
}

# prefer_system_lib SONAME path1 [path2...]
# ALWAYS replaces whatever is in lib/ with a symlink to the real system lib.
# Used for Wayland client libraries where using the real compositor-specific
# library is critical — our fallback stubs cannot connect to the compositor.
prefer_system_lib() {
    local SONAME="$1"
    shift
    for candidate in "$@"; do
        if [ -f "${candidate}" ]; then
            echo "[launch] Using system ${SONAME} → ${candidate}"
            ln -sf "${candidate}" "${LIB_DIR}/${SONAME}" 2>/dev/null || true
            return 0
        fi
    done
    echo "[launch] WARNING: ${SONAME} not found — using built-in stub (display may not work)"
    return 1
}

# ── Wayland client libraries ──────────────────────────────────────────────────
# CRITICAL: SDL2 needs the REAL libwayland-client from Tizen's compositor stack
# to connect to Enlightenment. Our fallback stub can't do this. prefer_system_lib
# always replaces our stub symlink with the real system library when found.
prefer_system_lib libwayland-client.so.0 \
    /usr/lib/aarch64-linux-gnu/libwayland-client.so.0 \
    /usr/lib64/libwayland-client.so.0 \
    /usr/lib/libwayland-client.so.0 \
    /lib/aarch64-linux-gnu/libwayland-client.so.0 \
    /lib64/libwayland-client.so.0

prefer_system_lib libwayland-egl.so.1 \
    /usr/lib/aarch64-linux-gnu/libwayland-egl.so.1 \
    /usr/lib64/libwayland-egl.so.1 \
    /usr/lib/libwayland-egl.so.1 \
    /lib/aarch64-linux-gnu/libwayland-egl.so.1 \
    /usr/lib/aarch64-linux-gnu/mesa-egl/libwayland-egl.so.1

prefer_system_lib libwayland-cursor.so.0 \
    /usr/lib/aarch64-linux-gnu/libwayland-cursor.so.0 \
    /usr/lib64/libwayland-cursor.so.0 \
    /usr/lib/libwayland-cursor.so.0

try_symlink_lib libxkbcommon.so.0 \
    /usr/lib/aarch64-linux-gnu/libxkbcommon.so.0 \
    /usr/lib64/libxkbcommon.so.0 \
    /usr/lib/libxkbcommon.so.0

# ── Audio libraries ───────────────────────────────────────────────────────────
# ALSA (libasound): Samsung TVs typically use ALSA internally.
# PulseAudio: Tizen may have its own audio system; try anyway.
try_symlink_lib libasound.so.2 \
    /usr/lib/aarch64-linux-gnu/libasound.so.2 \
    /usr/lib64/libasound.so.2 \
    /usr/lib/libasound.so.2

try_symlink_lib libpulse.so.0 \
    /usr/lib/aarch64-linux-gnu/libpulse.so.0 \
    /usr/lib64/libpulse.so.0 \
    /usr/lib/libpulse.so.0

# ── udev for input device hotplug ────────────────────────────────────────────
try_symlink_lib libudev.so.1 \
    /usr/lib/aarch64-linux-gnu/libudev.so.1 \
    /usr/lib64/libudev.so.1 \
    /usr/lib/libudev.so.1 \
    /lib/aarch64-linux-gnu/libudev.so.1

# ── libxml2.so.16 compatibility ───────────────────────────────────────────────
# Sober requires libxml2.so.16 (GNOME Platform 50 / libxml2 3.x SONAME)
# Tizen may have libxml2.so.2 (2.x) — our shim bridges the gap
if [ ! -f "${LIB_DIR}/libxml2.so.16" ]; then
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

# ── D-Bus ─────────────────────────────────────────────────────────────────────
# Our libdbus_stub.so handles loading but pre-symlinking speeds up cold start
try_symlink_lib libdbus-1.so.3 \
    /usr/lib/aarch64-linux-gnu/libdbus-1.so.3 \
    /usr/lib64/libdbus-1.so.3 \
    /usr/lib/libdbus-1.so.3 \
    /lib/aarch64-linux-gnu/libdbus-1.so.3 \
    /usr/lib/tizen/libdbus-1.so.3

# ── GLib/GObject stubs ────────────────────────────────────────────────────────
for lib in libglib-2.0.so.0 libgobject-2.0.so.0; do
    if ! /sbin/ldconfig -p 2>/dev/null | grep -q "${lib}" && \
       [ ! -f "${LIB_DIR}/${lib}" ]; then
        echo "WARNING: ${lib} not found — Sober may fail"
    fi
done

# ── Library path ──────────────────────────────────────────────────────────────
# Order: our stubs first, then sober's bundled libs, then system libs
SYSTEM_LIB_PATHS="/usr/lib/aarch64-linux-gnu:/usr/lib64:/usr/lib:/lib/aarch64-linux-gnu:/lib"

# Build LD_LIBRARY_PATH:
#   1. Our stubs/shims (lib/) — highest priority
#   2. sober's bundled libs (bin/) — libloader.so, libbadcpu.so
#   3. mimalloc subdir — matches sober RUNPATH $ORIGIN/subprojects/mimalloc
#   4. System libs
export LD_LIBRARY_PATH="${LIB_DIR}:${INSTALL_DIR}/bin:${INSTALL_DIR}/bin/subprojects/mimalloc:${SYSTEM_LIB_PATHS}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

echo "[launch] LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"

# ── GPU / EGL libraries ───────────────────────────────────────────────────────
# Samsung Tizen TVs may put EGL/GLES in vendor-specific paths.
# Prefer_system_lib for EGL so we always get the real HW driver.
prefer_system_lib libEGL.so.1 \
    /usr/lib/aarch64-linux-gnu/libEGL.so.1 \
    /usr/lib64/libEGL.so.1 \
    /usr/lib/libEGL.so.1 \
    /usr/lib/aarch64-linux-gnu/mesa-egl/libEGL.so.1 \
    /usr/lib/tizen/libEGL.so.1

prefer_system_lib libGLESv2.so.2 \
    /usr/lib/aarch64-linux-gnu/libGLESv2.so.2 \
    /usr/lib64/libGLESv2.so.2 \
    /usr/lib/libGLESv2.so.2 \
    /usr/lib/tizen/libGLESv2.so.2

# ── GPU / EGL environment ─────────────────────────────────────────────────────
export EGL_PLATFORM="wayland"

# Samsung NQ4 AI Gen3 GPU — disable debug overhead, use hardware path
export LIBGL_DEBUG=""
export EGL_LOG_LEVEL="fatal"

# Shader cache: enable for faster subsequent launches
export MESA_SHADER_CACHE_DISABLE="false"
export __GL_SHADER_DISK_CACHE=1
export __GL_SHADER_DISK_CACHE_PATH="${SOBER_HOME}/.cache/sober/gl_shaders"
mkdir -p "${__GL_SHADER_DISK_CACHE_PATH}"

# Wayland vsync: avoid tearing on 120Hz OLED panel
export SDL_VIDEODRIVER="wayland"
export SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-alsa}"

# ── CPU performance mode ──────────────────────────────────────────────────────
# Request performance governor if available (Tizen may require root)
for cpu_gov in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    echo "performance" > "${cpu_gov}" 2>/dev/null || true
done

# ── NQ4 AI Gen3 SoC performance tuning ───────────────────────────────────────
# ARM big.LITTLE — keep Roblox on the big cores (A78-class)
# Tizen may use cpuset cgroups; try both interfaces
if [ -d /dev/cpuset/foreground ]; then
    echo $$ > /dev/cpuset/foreground/tasks 2>/dev/null || true
fi

# mimalloc: reduce lock contention on 8-thread SoC
export MIMALLOC_LARGE_OS_PAGES=1
export MIMALLOC_PAGE_RESET=0

# SDL2: single GL context swap interval = 1 for 60fps cap (OLED handles it)
export SDL_HINT_RENDER_VSYNC=1
# SDL2 video thread priority
export SDL_HINT_THREAD_PRIORITY_POLICY=2

# ── Sober-specific environment ────────────────────────────────────────────────
export SOBER_DISPLAY_MODE="fullscreen"
export SOBER_DISABLE_SERVICES="1"
export SOBER_SKIP_UPDATE_CHECK="1"
export SOBER_DATA_DIR="${SOBER_HOME}/.local/share/sober"

# ── D-Bus ────────────────────────────────────────────────────────────────────
if [ -z "${DBUS_SESSION_BUS_ADDRESS}" ]; then
    if command -v dbus-launch >/dev/null 2>&1; then
        eval $(dbus-launch --sh-syntax 2>/dev/null) || true
    fi
fi

# ── Input mapper ──────────────────────────────────────────────────────────────
cleanup() {
    echo "[launch] Shutting down TizenRoblox..."
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
    sleep 0.5
    echo "[launch] Input mapper PID: ${INPUT_MAPPER_PID}"
else
    echo "WARNING: input_mapper not found at ${INPUT_MAPPER} — gamepad won't work"
fi

# ── Launch Sober (with crash recovery) ───────────────────────────────────────
echo "$$" > "${PID_FILE}"
echo "[launch] Launching Sober runtime..."
echo "[launch] Binary: ${SOBER_BIN}"

SOBER_ARGS="${@}"

# Auto-restart on crash, up to MAX_RESTARTS times within RESTART_WINDOW seconds.
# Deliberate exit (code 0) or user signal breaks the loop immediately.
MAX_RESTARTS=3
RESTART_WINDOW=60
_restart_count=0
_window_start=$(date +%s)
_STOP_RESTART=0

_sober_stopped() {
    _STOP_RESTART=1
}
trap '_sober_stopped' USR1

while [ "${_STOP_RESTART}" -eq 0 ]; do
    "${SOBER_BIN}" ${SOBER_ARGS}
    EXIT_CODE=$?

    # Clean exit or SIGTERM/SIGINT — don't restart
    if [ "${EXIT_CODE}" -eq 0 ] || [ "${EXIT_CODE}" -eq 130 ] || [ "${EXIT_CODE}" -eq 143 ]; then
        echo "[launch] Sober exited cleanly (code ${EXIT_CODE})"
        break
    fi

    echo "[launch] Sober crashed (code ${EXIT_CODE})"

    # Reset restart counter if outside window
    _now=$(date +%s)
    if [ $((_now - _window_start)) -gt "${RESTART_WINDOW}" ]; then
        _restart_count=0
        _window_start="${_now}"
    fi

    _restart_count=$((_restart_count + 1))
    if [ "${_restart_count}" -gt "${MAX_RESTARTS}" ]; then
        echo "[launch] Too many crashes (${MAX_RESTARTS} in ${RESTART_WINDOW}s) — giving up"
        break
    fi

    echo "[launch] Restarting (attempt ${_restart_count}/${MAX_RESTARTS})..."
    sleep 2
done
