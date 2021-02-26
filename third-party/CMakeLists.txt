cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH .)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE) #If vcpkg.cmake is used, don't use findSDL2.cmake
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}) # For FindSDL2.cmake
endif()
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(imgui 
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h

    imgui/backends/imgui_impl_opengl3.cpp 
    imgui/backends/imgui_impl_sdl.cpp
    imgui/misc/cpp/imgui_stdlib.cpp

    glad/src/glad.c
)

target_link_libraries(imgui SDL2::SDL2main SDL2::SDL2 ${OPENGL_gl_LIBRARY})

target_include_directories(imgui PUBLIC
    imgui
    imgui/backends/
    glad/include
    imgui/misc/cpp
    ${SDL2_INCLUDE_DIRS}
)
