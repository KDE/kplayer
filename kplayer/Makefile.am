bin_PROGRAMS = kplayer

kplayer_SOURCES = kplayerpropertiesdevice.ui kplayernodeaction.cpp kplayernodeview.cpp kplayerplaylist.cpp kplayersettingsdialog.cpp kplayersettingssubtitles.ui kplayersettingsbrightness.ui kplayersettingsvolume.ui kplayersettingssaturation.ui kplayersettingsprogress.ui kplayersettingshue.ui kplayersettingscontrols.ui kplayersettingssliders.ui kplayersettingscontrast.ui kplayerlogwindow.cpp kplayersettingsvideo.ui kplayersettingsaudio.ui kplayersettingsadvanced.ui kplayersettingsgeneral.ui kplayer.cpp main.cpp 

kplayer_LDADD   = libkplayernoinst.la $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

#EXTRA_DIST = main.cpp kplayer.cpp kplayer.h kplayerpropertiesgeneral.ui kplayerpropertiessize.ui kplayerpropertiessubtitles.ui kplayerpropertiesaudio.ui kplayerpropertiesvideo.ui kplayerpropertiesadvanced.ui kplayerpropertiesdevice.ui kplayersettingsgeneral.ui kplayersettingsadvanced.ui kplayersettingsaudio.ui kplayersettingsvideo.ui kplayerlogwindow.cpp kplayerlogwindow.h kplayersettingscontrast.ui kplayersettingscontrols.ui kplayersettingssliders.ui kplayersettingshue.ui kplayersettingsprogress.ui kplayersettingssaturation.ui kplayersettingsvolume.ui kplayersettingsbrightness.ui kplayersettingssubtitles.ui kplayersettingsdialog.h kplayersettingsdialog.cpp kplayerpropertiesdialog.h kplayerpropertiesdialog.cpp kplayersource.h kplayersource.cpp kplayernode.h kplayernode.cpp kplayernodeaction.h kplayernodeaction.cpp kplayernodeview.h kplayernodeview.cpp kplayerplaylist.h kplayerplaylist.cpp kplayerwidget.h kplayerwidget.cpp kplayerprocess.h kplayerprocess.cpp kplayerproperties.h kplayerproperties.cpp kplayeractionlist.h kplayeractionlist.cpp kplayerengine.h kplayerengine.cpp kplayersettings.h kplayersettings.cpp kplayerslideraction.h kplayerslideraction.cpp kplayerpart.h kplayerpart.cpp x11.cpp kplayerui.rc kplayerpartui.rc input.conf kplayer.desktop kplayerpart.desktop hi16-app-kplayer.png hi22-app-kplayer.png hi32-app-kplayer.png hi48-app-kplayer.png hi64-app-kplayer.png lo16-app-kplayer.png lo22-app-kplayer.png lo32-app-kplayer.png rtsp.protocol pnm.protocol mms.protocol mmst.protocol mmsu.protocol

# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

# METASOURCES = AUTO
kplayer_METASOURCES = kplayer.moc kplayerlogwindow.moc kplayersettingsdialog.moc kplayerplaylist.moc kplayernodeaction.moc kplayernodeview.moc
# kplayersettingssubtitles.moc kplayersettingsbrightness.moc kplayersettingsvolume.moc kplayersettingssaturation.moc kplayersettingsprogress.moc kplayersettingshue.moc kplayersettingscontrols.moc kplayersettingssliders.moc kplayersettingscontrast.moc kplayersettingsvideo.moc kplayersettingsaudio.moc kplayersettingsadvanced.moc kplayersettingsgeneral.moc kplayerpropertiesgeneral.moc kplayerpropertiessize.moc kplayerpropertiessubtitles.moc kplayerpropertiesaudio.moc kplayerpropertiesvideo.moc kplayerpropertiesadvanced.moc

KDE_ICON = kplayer

#DIST_SOURCES =

# the library search path. 
kplayer_LDFLAGS = $(all_libraries) $(KDE_RPATH)

noinst_LTLIBRARIES = libkplayernoinst.la
libkplayernoinst_la_LDFLAGS = $(all_libraries) -no-undefined
libkplayernoinst_la_SOURCES = x11.cpp kplayerwidget.cpp kplayerslideraction.cpp kplayerprocess.cpp kplayersettings.cpp kplayerproperties.cpp kplayeractionlist.cpp kplayerengine.cpp kplayersource.cpp kplayernode.cpp kplayerpropertiesdialog.cpp kplayerpropertiesgeneral.ui kplayerpropertiessize.ui kplayerpropertiessubtitles.ui kplayerpropertiesaudio.ui kplayerpropertiesvideo.ui kplayerpropertiesadvanced.ui
libkplayernoinst_la_LIBADD = $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT)
libkplayernoinst_la_METASOURCES = kplayerwidget.moc kplayerslideraction.moc kplayerprocess.moc kplayersettings.moc kplayerproperties.moc kplayeractionlist.moc kplayerengine.moc kplayerpropertiesdialog.moc kplayersource.moc kplayernode.moc

kde_module_LTLIBRARIES = libkplayerpart.la
libkplayerpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module -no-undefined
libkplayerpart_la_SOURCES = kplayerpart.cpp
libkplayerpart_la_LIBADD = $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) libkplayernoinst.la
libkplayerpart_la_METASOURCES = kplayerpart.moc

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDEs XML GUI builing
rcdir = $(kde_datadir)/kplayer
rc_DATA = kplayerui.rc

partdir = $(kde_datadir)/kplayer
part_DATA = kplayerpartui.rc

appdatadir = $(kde_datadir)/kplayer
appdata_DATA = input.conf

xdg_apps_DATA = kplayer.desktop

kde_services_DATA = kplayerpart.desktop

servicemenudir = $(kde_datadir)/konqueror/servicemenus
servicemenu_DATA = kplayer-play-queue.desktop kplayer-next.desktop kplayer-directory.desktop kplayer-actions.desktop

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kplayer.pot; \
	fi
