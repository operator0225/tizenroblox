cmake_minimum_required(VERSION 3.16)

# ── Third-party static libraries ────────────────────────────────────────────
# Cross-compiled by scripts/build_thirdparty.sh (OpenSSL, curl, libxml2,
# glib/gobject, freetype, fontconfig + small deps). Statically linked (whole
# archive) into the corresponding stub .so below, so the shipped library IS
# the real implementation — no dlopen, no system-path guessing, no version
# mismatch risk. GPU/display libs are NOT here; see tizen/launch.sh instead.
set(THIRDPARTY_PREFIX ${CMAKE_SOURCE_DIR}/thirdparty/install)

# ── Helper macro ──────────────────────────────────────────────────────────────
# add_stub(TARGET OUTPUT_NAME SOVERSION VERSION SOURCES...)
macro(add_stub target outname soversion version)
    add_library(${target} SHARED ${ARGN})
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME  "${outname}"
        SOVERSION    ${soversion}
        VERSION      ${version}
        PREFIX       "lib"
    )
    target_compile_options(${target} PRIVATE -fvisibility=default -Wall -Wextra)
    target_link_libraries(${target} PRIVATE dl)
endmacro()

# ── libsecret-1.so.0 ─────────────────────────────────────────────────────────
# Replaces GNOME keyring with file-based credential storage
add_stub(secret-1 "secret-1" 0 "0.0.0" libsecret.c)
target_link_libraries(secret-1 PRIVATE)  # no dl needed for this one

# ── libdecor-0.so.0 ──────────────────────────────────────────────────────────
# Always-fullscreen Wayland decoration no-op for Tizen TV
add_stub(decor-0 "decor-0" 0 "0.0.0" libdecor.c)
target_link_libraries(decor-0 PRIVATE)   # no dl needed

# ── libxml2.so.16 ────────────────────────────────────────────────────────────
# Real libxml2 2.12.9, statically bundled (SONAME renamed to match sober's
# expected GNOME Platform 50 libxml2 3.x .16 versioning; Tizen only has .2)
add_stub(xml2-compat "xml2" 16 "16.0.0" libxml2_compat.c)
target_link_options(xml2-compat PRIVATE
    "-Wl,--whole-archive" "${THIRDPARTY_PREFIX}/lib/libxml2.a" "-Wl,--no-whole-archive")
target_link_libraries(xml2-compat PRIVATE "${THIRDPARTY_PREFIX}/lib/libz.a" pthread)

# ── libcrypto.so.3 ───────────────────────────────────────────────────────────
# Real OpenSSL 3.0.13, statically bundled (see thirdparty/ + build_thirdparty.sh)
add_stub(crypto3-compat "crypto" 3 "3.0.0" libcrypto3_compat.c)
target_link_options(crypto3-compat PRIVATE
    "-Wl,--whole-archive" "${THIRDPARTY_PREFIX}/lib/libcrypto.a" "-Wl,--no-whole-archive")
target_link_libraries(crypto3-compat PRIVATE pthread)

# ── libgstreamer-1.0.so.0 + libgstapp + libgstvideo ─────────────────────────
# GStreamer shim for Roblox video playback (covers all three libs in one .so)
add_stub(gstreamer-stub "gstreamer-1.0" 0 "0.0.0" libgstreamer_stub.c)
add_stub(gstapp-stub    "gstapp-1.0"    0 "0.0.0" libgstreamer_stub.c)
add_stub(gstvideo-stub  "gstvideo-1.0"  0 "0.0.0" libgstreamer_stub.c)

# ── libglib-2.0.so.0 + libgobject-2.0.so.0 ───────────────────────────────────
# Real GLib 2.82.2, statically bundled (sober only calls 4 symbols, but we
# ship the real thing rather than hand-reimplementing a subset)
set(GLIB_DEPS
    "${THIRDPARTY_PREFIX}/lib/libffi.a"
    "${THIRDPARTY_PREFIX}/lib/libpcre2-8.a"
    "${THIRDPARTY_PREFIX}/lib/libz.a")

add_stub(glib-stub    "glib-2.0"    0 "0.0.0" libglib_stub.c)
target_link_options(glib-stub PRIVATE
    "-Wl,--whole-archive" "${THIRDPARTY_PREFIX}/lib/libglib-2.0.a" "-Wl,--no-whole-archive")
target_link_libraries(glib-stub PRIVATE ${GLIB_DEPS} pthread)

add_stub(gobject-stub "gobject-2.0" 0 "0.0.0" libglib_stub.c)
target_link_options(gobject-stub PRIVATE
    "-Wl,--whole-archive"
    "${THIRDPARTY_PREFIX}/lib/libgobject-2.0.a" "${THIRDPARTY_PREFIX}/lib/libglib-2.0.a"
    "-Wl,--no-whole-archive")
target_link_libraries(gobject-stub PRIVATE ${GLIB_DEPS} pthread)

# ── libfontconfig.so.1 ───────────────────────────────────────────────────────
# Real fontconfig 2.15.0, statically bundled against our freetype + expat
add_stub(fontconfig-stub "fontconfig" 1 "1.0.0" libfontconfig_stub.c)
target_link_options(fontconfig-stub PRIVATE
    "-Wl,--whole-archive" "${THIRDPARTY_PREFIX}/lib/libfontconfig.a" "-Wl,--no-whole-archive")
target_link_libraries(fontconfig-stub PRIVATE
    "${THIRDPARTY_PREFIX}/lib/libfreetype.a" "${THIRDPARTY_PREFIX}/lib/libexpat.a"
    "${THIRDPARTY_PREFIX}/lib/libz.a" pthread)

# ── libfreetype.so.6 ─────────────────────────────────────────────────────────
# Real FreeType 2.13.3, statically bundled
add_stub(freetype-stub "freetype" 6 "6.0.0" libfreetype_stub.c)
target_link_options(freetype-stub PRIVATE
    "-Wl,--whole-archive" "${THIRDPARTY_PREFIX}/lib/libfreetype.a" "-Wl,--no-whole-archive")
target_link_libraries(freetype-stub PRIVATE "${THIRDPARTY_PREFIX}/lib/libz.a" pthread)

# ── libcurl.so.4 ─────────────────────────────────────────────────────────────
# Real curl 8.10.1, statically bundled against our OpenSSL+zlib
add_stub(curl-stub "curl" 4 "4.0.0" libcurl_stub.c)
target_link_options(curl-stub PRIVATE
    "-Wl,--whole-archive" "${THIRDPARTY_PREFIX}/lib/libcurl.a" "-Wl,--no-whole-archive")
target_link_libraries(curl-stub PRIVATE
    "${THIRDPARTY_PREFIX}/lib/libssl.a" "${THIRDPARTY_PREFIX}/lib/libcrypto.a"
    "${THIRDPARTY_PREFIX}/lib/libz.a" pthread)
# ── libdbus-1.so.3 ──────────────────────────────────────────────────────────────
# D-Bus shim for session IPC — delegates to system libdbus-1.so.3
add_stub(dbus-stub "dbus-1" 3 "3.0.0" libdbus_stub.c)


# ── libwayland-client.so.0 / libwayland-egl.so.1 / libwayland-cursor.so.0 ───
# Last-resort fallback stubs; normally the launcher creates symlinks to the
# real Tizen system libraries so SDL2 can connect to the Wayland compositor.
# These stubs prevent hard crashes on systems where symlinks couldn't be made.
option(WAYLAND_STUB "Build Wayland passthrough stubs" ON)
if(WAYLAND_STUB)
    add_stub(wayland-client-stub  "wayland-client"  0 "0.0.0" libwayland_stub.c)
    add_stub(wayland-egl-stub     "wayland-egl"     1 "1.0.0" libwayland_stub.c)
    add_stub(wayland-cursor-stub  "wayland-cursor"  0 "0.0.0" libwayland_stub.c)
    set(WAYLAND_STUB_TARGETS wayland-client-stub wayland-egl-stub wayland-cursor-stub)
else()
    set(WAYLAND_STUB_TARGETS)
endif()

install(TARGETS
    secret-1 decor-0 xml2-compat crypto3-compat
    gstreamer-stub gstapp-stub gstvideo-stub
    glib-stub gobject-stub
    fontconfig-stub freetype-stub curl-stub dbus-stub
    ${WAYLAND_STUB_TARGETS}
    LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/dist/lib
)
