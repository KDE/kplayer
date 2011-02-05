project(kplayer)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "7")
set(VERSION_PATCH "0")
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR})

set(KDE_MIN_VERSION "4.3.0")
find_package(KDE4 REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory(doc)
add_subdirectory(icons)
add_subdirectory(kplayer)

install(FILES COPYING DESTINATION ${DATA_INSTALL_DIR}/kplayer)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "KPlayer is a KDE multimedia player.")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${VERSION}")
set(CPACK_GENERATOR TBZ2)
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_IGNORE_FILES
"\\\\.bz2$"
"\\\\.cmake$"
"\\\\.cvsignore$"
"\\\\.files$"
"\\\\.gmo$"
"\\\\.moc$"
"\\\\.shell$"
"\\\\.vim$"
"_automoc\\\\.cpp$"
"/ui_.*\\\\.h$"
"/install_manifest\\\\.txt$"
"/CMakeCache\\\\.txt$"
"/CMakeFiles/"
"/CMakeTmp/"
"/Makefile$"
"^${PROJECT_SOURCE_DIR}.*/CVS/"
"^${PROJECT_SOURCE_DIR}/debian/"
"^${PROJECT_SOURCE_DIR}/lib/"
"^${PROJECT_SOURCE_DIR}/home/"
"^${PROJECT_SOURCE_DIR}/kplayer/kplayer$"
)
include(CPack)
