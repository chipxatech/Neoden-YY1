cmake_minimum_required(VERSION 3.15)
project(neoden_converter_cpp VERSION 2.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(neoden_converter_gui WIN32
    gui_main.cpp
    app.rc
)
target_link_libraries(neoden_converter_gui PRIVATE comctl32 gdiplus shell32)
