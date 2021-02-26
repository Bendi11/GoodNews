cmake_minimum_required(VERSION 3.10)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin) #Set output directory to bin folder for compiled binaries
project(GoodNews)

if( NOT DEFINED CMAKE_BUILD_TYPE ) #If no build type is specified, build release binaries
set( CMAKE_BUILD_TYPE "Release" )
endif()

set(BUILD_CPR_TESTS OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "") #Force building of all libraries as static to reduce number of .dll / .so needed to be shipped with program
set(USE_SYSTEM_CURL ON CACHE INTERNAL "") #Force use of pre installed libcurl, don't download and build it because it breaks

include(FetchContent) #FetchContent for cpr requests library
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/whoshuu/cpr.git GIT_TAG c8d33915dbd88ad6c92b258869b03aba06587ff9) # the commit hash for 1.5.0
FetchContent_MakeAvailable(cpr)

include_directories(
    "src/include"

    "third-party/pugixml/src"
    "third-party/stb"
)


set(SOURCES
    "src/main.cpp"
    "src/rss.cpp"
    "src/gui.cpp"

    "third-party/pugixml/src/pugixml.cpp"

    "res/res.rc"
)

if(WIN32)
#add_link_options("/subsystem:windows")
add_link_options("/NODEFAULTLIB:libcmt.lib")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
endif()

if(WIN32)
add_executable(${CMAKE_PROJECT_NAME} WIN32 ${SOURCES})
else()
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
endif()

set(wxBUILD_SHARED OFF CACHE INTERNAL "") #Build static wxWidgets
add_subdirectory(third-party/wxWidgets)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE wx::core wx::base wx::net wx::webView)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE cpr::cpr)

