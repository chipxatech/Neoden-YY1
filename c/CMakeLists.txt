cmake_minimum_required(VERSION 3.15)
project(neoden_converter_c VERSION 2.0 LANGUAGES C RC)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(neoden_converter_c_gui WIN32
    gui_main.c
    converter.c
    converter.h
    app.rc
)
target_link_libraries(neoden_converter_c_gui PRIVATE comctl32 shell32 gdi32 gdiplus user32)
