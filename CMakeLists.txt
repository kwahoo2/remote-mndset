cmake_minimum_required(VERSION 3.16)

project(remote-mndset LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)

set(IMGUI_SOURCES
    3rdparty/imgui/imgui.cpp
    3rdparty/imgui/imgui_draw.cpp
    3rdparty/imgui/imgui_tables.cpp
    3rdparty/imgui/imgui_widgets.cpp
    3rdparty/imgui/backends/imgui_impl_sdl3.cpp
    3rdparty/imgui/backends/imgui_impl_sdlgpu3.cpp
)

add_executable(remote-mndset main.cpp
    structs.h
    data_sender.h
    data_sender.cpp
    movement.h
    movement.cpp
    ${IMGUI_SOURCES}
    math_helper.h
    math_helper.cpp
    main_window.h
    main_window.cpp
    settings.cpp
    settings.h
)

target_include_directories(remote-mndset PRIVATE 3rdparty/imgui;3rdparty/imgui/backends)

target_link_libraries(remote-mndset PRIVATE SDL3::SDL3-shared)

include(GNUInstallDirs)
install(TARGETS remote-mndset
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
