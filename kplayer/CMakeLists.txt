set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUGFULL "-DDEBUG ${CMAKE_CXX_FLAGS_DEBUGFULL}")
set(CMAKE_CXX_FLAGS_PROFILE "-DDEBUG ${CMAKE_CXX_FLAGS_PROFILE}")

########### next target ###############

set(kplayer_common_SRCS x11.cpp)

kde4_add_ui_files(kplayer_common_SRCS 
   kplayerpropertiesdevice.ui 
   kplayerpropertiesgeneral.ui
   kplayerpropertiessize.ui
   kplayerpropertiessubtitles.ui
   kplayerpropertiesaudio.ui
   kplayerpropertiesvideo.ui
   kplayerpropertiesadvanced.ui)

set(kplayer_common_SRCS ${kplayer_common_SRCS}
   kplayerwidget.cpp
   kplayerslideraction.cpp
   kplayerprocess.cpp
   kplayersettings.cpp
   kplayerproperties.cpp
   kplayeractionlist.cpp
   kplayerengine.cpp
   kplayerpropertiesdialog.cpp)

set(kplayer_SRCS ${kplayer_common_SRCS})

kde4_add_ui_files(kplayer_SRCS 
   kplayersettingssubtitles.ui 
   kplayersettingsbrightness.ui 
   kplayersettingsvolume.ui 
   kplayersettingssaturation.ui 
   kplayersettingsprogress.ui 
   kplayersettingshue.ui 
   kplayersettingscontrols.ui 
   kplayersettingssliders.ui 
   kplayersettingscontrast.ui 
   kplayersettingsvideo.ui 
   kplayersettingsaudio.ui 
   kplayersettingsadvanced.ui 
   kplayersettingsgeneral.ui)

set(kplayer_SRCS ${kplayer_SRCS}
   kplayersource.cpp
   kplayernode.cpp
   kplayernodeaction.cpp 
   kplayernodeview.cpp 
   kplayerplaylist.cpp 
   kplayersettingsdialog.cpp 
   kplayerlogwindow.cpp 
   kplayer.cpp 
   main.cpp)

kde4_add_executable(kplayer ${kplayer_SRCS})

target_link_libraries(kplayer ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDE3SUPPORT_LIBRARY})

install(TARGETS kplayer DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(kplayerpart_PART_SRCS ${kplayer_common_SRCS} kplayerpart.cpp)

kde4_add_plugin(kplayerpart WITH_PREFIX ${kplayerpart_PART_SRCS})

target_link_libraries(kplayerpart ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QT3SUPPORT_LIBRARY} ${KDE4_KDE3SUPPORT_LIBRARY})

install(TARGETS kplayerpart DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES  kplayer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES  kplayerpartui.rc DESTINATION ${DATA_INSTALL_DIR}/kplayer )
install( FILES  input.conf DESTINATION ${DATA_INSTALL_DIR}/kplayer )
install( FILES  kplayerpart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES  kplayerui.rc DESTINATION ${DATA_INSTALL_DIR}/kplayer )
install( FILES  kplayer-play-queue.desktop kplayer-next.desktop
kplayer-directory.desktop kplayer-actions.desktop DESTINATION
${SERVICES_INSTALL_DIR}/ServiceMenus )

kde4_install_icons( ${ICON_INSTALL_DIR} )
