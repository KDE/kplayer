project(kplayer)

find_package(KDE4 REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

add_subdirectory(icons)
add_subdirectory(kplayer)




#original Makefile.am contents follow:

#SUBDIRS = $(TOPSUBDIRS)
#
#AUTOMAKE_OPTIONS = dist-bzip2 foreign 
#
#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 
#
#appdatadir = $(kde_datadir)/kplayer
#appdata_DATA = COPYING
#
#package-messages:
#	$(MAKE) -f admin/Makefile.common package-messages
#	$(MAKE) -C po merge
