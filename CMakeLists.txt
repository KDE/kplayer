project(kplayer)
set(KDE_MIN_VERSION "4.3.0")

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory(doc)
add_subdirectory(icons)
add_subdirectory(kplayer)

install(FILES COPYING DESTINATION ${DATA_INSTALL_DIR}/kplayer)
