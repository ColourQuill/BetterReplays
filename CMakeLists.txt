cmake_minimum_required(VERSION 3.10)
project(BetterReplays VERSION 0.1.0 LANGUAGES C CXX)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# *********************************************************************************************
#                                      REQUIRED PACKAGES
# *********************************************************************************************

find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3 libportal glib-2.0 gobject-2.0)
pkg_check_modules(FFMPEG REQUIRED libavcodec libavutil libswscale libavformat)

# *********************************************************************************************
#                                         DEFINITIONS
# *********************************************************************************************



# *********************************************************************************************
#                                         CMAKE OUTPUT
# *********************************************************************************************

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/build/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_DIR}/build/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_DIR}/build/release)
file(MAKE_DIRECTORY ${PROJECT_DIR}/build/debug/resources)

# *********************************************************************************************
#                                      LOCATION DEFINITIONS
# *********************************************************************************************

set(CAPTURE_DIR ${PROJECT_DIR}/capture)
set(ENCODER_DIR ${PROJECT_DIR}/encoder)
set(BUFFER_DIR ${PROJECT_DIR}/buffer)
set(MUXER_DIR ${PROJECT_DIR}/muxer)
set(HOTKEY_DIR ${PROJECT_DIR}/hotkey)
set(PIPEWIRE_DIR ${PROJECT_DIR}/pipewire)
set(STRUCTS_DIR ${PROJECT_DIR}/structs)
set(UTILS_DIR ${PROJECT_DIR}/utils)
set(AUDIO_DIR ${PROJECT_DIR}/audio)
set(APP_DIR ${PROJECT_DIR}/app)

# *********************************************************************************************
#                                      SOURCE DEFINITIONS
# *********************************************************************************************

set(CAPTURE_SRC
    ${CAPTURE_DIR}/screen_capture.cpp
    ${CAPTURE_DIR}/linux_screen_capture.cpp
)
set(ENCODER_SRC
    ${ENCODER_DIR}/encoder.cpp
    ${ENCODER_DIR}/ffmpeg_encoder.cpp
)
set(BUFFER_SRC
    ${BUFFER_DIR}/buffer.cpp
)
set(MUXER_SRC
    ${MUXER_DIR}/muxer.cpp
    ${MUXER_DIR}/ffmpeg_muxer.cpp
)
set(HOTKEY_SRC
    ${HOTKEY_DIR}/hotkey.cpp
    ${HOTKEY_DIR}/mac_hotkey.cpp
    ${HOTKEY_DIR}/linux_hotkey.cpp
    ${HOTKEY_DIR}/windows_hotkey.cpp
)
set(PIPEWIRE_SRC

)
set(STRUCTS_SRC

)
set(UTILS_SRC
    ${UTILS_DIR}/logger.cpp
    ${UTILS_DIR}/portal.cpp
    ${UTILS_DIR}/timer.cpp
    ${UTILS_DIR}/time.cpp
    ${UTILS_DIR}/config.cpp
    ${UTILS_DIR}/file.cpp
)
set(AUDIO_SRC
    ${AUDIO_DIR}/audio.cpp
)
set(APP_SRC 
    ${APP_DIR}/better_replays.cpp
    ${APP_DIR}/settings.cpp
)

set(SOURCES
    ${CAPTURE_SRC}
    ${ENCODER_SRC}
    ${BUFFER_SRC}
    ${MUXER_SRC}
    ${HOTKEY_SRC}
    ${PIPEWIRE_SRC}
    ${STRUCTS_SRC}
    ${UTILS_SRC}
    ${AUDIO_SRC}
    ${APP_SRC}
)

# *********************************************************************************************
#                                          EXECUTABLE
# *********************************************************************************************

add_executable(BetterReplays main.cpp ${SOURCES})

# *********************************************************************************************
#                                           LINKING
# *********************************************************************************************

target_include_directories(BetterReplays PRIVATE
    ${CAPTURE_DIR}
    ${ENCODER_DIR}
    ${BUFFER_DIR}
    ${MUXER_DIR}
    ${HOTKEY_DIR}
    ${PIPEWIRE_DIR}
    ${STRUCTS_DIR}
    ${UTILS_DIR}
    ${AUDIO_DIR}
    ${APP_DIR}

    ${PIPEWIRE_INCLUDE_DIRS}
    ${PORTAL_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(BetterReplays PRIVATE
    ${PIPEWIRE_LIBRARIES}
    ${PORTAL_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${FFMPEG_LIBRARIES}
    SDL2::SDL2
)

target_compile_options(BetterReplays PRIVATE
    ${PIPEWIRE_CFLAGS_OTHER}
    ${FFMPEG_CFLAGS_OTHER}
)