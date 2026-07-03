cmake_minimum_required(VERSION 3.16)

# Samsung TV input mapper (IR remote → uinput virtual gamepad)
add_executable(input_mapper input_mapper.c)

target_compile_options(input_mapper PRIVATE
    -Wall -Wextra -O2 -D_GNU_SOURCE
)

# uinput is a kernel interface, no extra link flags needed
target_link_libraries(input_mapper PRIVATE pthread)

install(TARGETS input_mapper
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/dist/bin
)

install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/launch.sh
    DESTINATION ${CMAKE_BINARY_DIR}/dist/bin
    RENAME launch.sh
)
