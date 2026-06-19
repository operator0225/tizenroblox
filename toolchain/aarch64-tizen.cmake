set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CROSS_PREFIX aarch64-linux-gnu)

set(CMAKE_C_COMPILER   ${CROSS_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_PREFIX}-g++)
set(CMAKE_AR           ${CROSS_PREFIX}-ar)
set(CMAKE_STRIP        ${CROSS_PREFIX}-strip)

set(CMAKE_FIND_ROOT_PATH /usr/${CROSS_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Tizen 9.0 target flags
set(TIZEN_FLAGS "-march=armv8-a -mfpu=neon-fp-armv8 -ftree-vectorize")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${TIZEN_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TIZEN_FLAGS}")

# Link flags: RPATH so shims are found at runtime
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,/opt/tizenroblox/lib -Wl,--dynamic-linker=/lib/ld-linux-aarch64.so.1")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,/opt/tizenroblox/lib")
