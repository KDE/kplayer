include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

########### install files ###############

install( FILES  DESTINATION ${DATA_INSTALL_DIR}/kplayer/icons )

kde4_install_icons(${DATA_INSTALL_DIR}/kplayer/icons)

#original Makefile.am contents follow:

## KDE_ICON = AUTO
#kplayericonsdir = $(kde_datadir)/kplayer/icons
#kplayericons_ICON = AUTO
