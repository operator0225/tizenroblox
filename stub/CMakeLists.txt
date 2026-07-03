cmake_minimum_required(VERSION 3.16)

# libsecret stub (replaces GNOME keyring with file-based storage)
add_library(secret-1 SHARED libsecret.c)
set_target_properties(secret-1 PROPERTIES
    OUTPUT_NAME "secret-1"
    SOVERSION 0
    VERSION 0.0.0
    PREFIX "lib"
)
target_compile_options(secret-1 PRIVATE -fvisibility=default -Wall -Wextra)

# libdecor stub (always-fullscreen no-op for Tizen TV)
add_library(decor-0 SHARED libdecor.c)
set_target_properties(decor-0 PROPERTIES
    OUTPUT_NAME "decor-0"
    SOVERSION 0
    VERSION 0.0.0
    PREFIX "lib"
)
target_compile_options(decor-0 PRIVATE -fvisibility=default -Wall -Wextra)

# libxml2.so.16 compatibility shim
# Sober requires libxml2 3.x SONAME (libxml2.so.16); Tizen has libxml2 2.x
# This shim dynamically loads the system libxml2.so.2 and re-exports symbols
add_library(xml2-compat SHARED libxml2_compat.c)
set_target_properties(xml2-compat PROPERTIES
    OUTPUT_NAME "xml2"
    SOVERSION 16
    VERSION 16.0.0
    PREFIX "lib"
)
target_compile_options(xml2-compat PRIVATE -fvisibility=default -Wall -Wextra)
target_link_libraries(xml2-compat PRIVATE dl)

install(TARGETS secret-1 decor-0 xml2-compat
    LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/dist/lib
)
