cmake_minimum_required(VERSION 3.16)

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
# GNOME Platform 50 uses libxml2 3.x (SONAME .16); Tizen has 2.x (.2)
add_stub(xml2-compat "xml2" 16 "16.0.0" libxml2_compat.c)

# ── libcrypto.so.3 ───────────────────────────────────────────────────────────
# OpenSSL 3.x shim — forwards to system libcrypto.so.3 or libcrypto.so.1.1
add_stub(crypto3-compat "crypto" 3 "3.0.0" libcrypto3_compat.c)

# ── libgstreamer-1.0.so.0 + libgstapp + libgstvideo ─────────────────────────
# GStreamer shim for Roblox video playback (covers all three libs in one .so)
add_stub(gstreamer-stub "gstreamer-1.0" 0 "0.0.0" libgstreamer_stub.c)
add_stub(gstapp-stub    "gstapp-1.0"    0 "0.0.0" libgstreamer_stub.c)
add_stub(gstvideo-stub  "gstvideo-1.0"  0 "0.0.0" libgstreamer_stub.c)

# ── libglib-2.0.so.0 + libgobject-2.0.so.0 ───────────────────────────────────
# GLib/GObject shim — only 4 symbols used by sober
add_stub(glib-stub    "glib-2.0"    0 "0.0.0" libglib_stub.c)
add_stub(gobject-stub "gobject-2.0" 0 "0.0.0" libglib_stub.c)

# ── libfontconfig.so.1 ───────────────────────────────────────────────────────
# fontconfig shim for font discovery
add_stub(fontconfig-stub "fontconfig" 1 "1.0.0" libfontconfig_stub.c)

# ── libfreetype.so.6 ─────────────────────────────────────────────────────────
# FreeType shim for glyph rasterization
add_stub(freetype-stub "freetype" 6 "6.0.0" libfreetype_stub.c)

# ── libcurl.so.4 ─────────────────────────────────────────────────────────────
# libcurl shim for HTTP (Roblox API / asset downloads)
add_stub(curl-stub "curl" 4 "4.0.0" libcurl_stub.c)

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
    fontconfig-stub freetype-stub curl-stub
    ${WAYLAND_STUB_TARGETS}
    LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/dist/lib
)
