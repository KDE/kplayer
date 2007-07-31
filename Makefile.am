#SUBDIRS = $(TOPSUBDIRS)

AUTOMAKE_OPTIONS = dist-bzip2 foreign 

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

appdatadir = $(kde_datadir)/kplayer
appdata_DATA = COPYING

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge
