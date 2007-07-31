/***************************************************************************
                          main.cpp
                          --------
    begin                : Sat Nov 16 10:12:50 EST 2002
    copyright            : (C) 2002-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "kplayer.h"

static const char* description = I18N_NOOP("KPlayer, a KDE media player");
static const char* license = I18N_NOOP("This program is distributed under the terms of the GPL version 3 or later.");
KAboutData* about;

static KCmdLineOptions options[] =
{
  { "play", I18N_NOOP("Play the files immediately (default)"), 0 },
  { "queue", I18N_NOOP("Queue the files for playing"), 0 },
  { "play-next", I18N_NOOP("Play the files after the currently played file finishes"), 0 },
  { "queue-next", I18N_NOOP("Queue the files for playing after the currently played file"), 0 },
  { "add-to-new-playlist", I18N_NOOP("Add the files to a new playlist"), 0 },
  { "add-to-playlists", I18N_NOOP("Add the files to the list of playlists"), 0 },
  { "add-to-collection", I18N_NOOP("Add the files to the multimedia collection"), 0 },
  { "+[files]", I18N_NOOP("File(s), directory/ies, or URL(s)"), 0 },
  { 0, 0, 0 }
};

int main (int argc, char *argv[])
{
  about = new KAboutData ("kplayer", "KPlayer", VERSION, description, KAboutData::License_File,
    "(C) 2002-2007, kiriuja", license, "http://kplayer.sourceforge.net/", "http://kplayer.sourceforge.net/email.html");
  about -> setLicenseText (license);
  about -> addAuthor ("kiriuja", 0, "http://kplayer.sourceforge.net/email.html");
  KCmdLineArgs::init (argc, argv, about);
  KCmdLineArgs::addCmdLineOptions (options);  // Add our own options.
  KPlayerApplication::addCmdLineOptions();
  return KPlayerApplication().exec();
}
