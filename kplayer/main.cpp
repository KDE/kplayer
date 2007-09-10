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

static const KLocalizedString description = ki18n("KPlayer, a KDE media player");
static const KLocalizedString license = ki18n("This program is distributed under the terms of the GPL version 3 or later.");
KAboutData* about;

int main (int argc, char *argv[])
{
  about = new KAboutData ("kplayer", 0, ki18n("KPlayer"), "0.7", description, KAboutData::License_File,
    ki18n("(C) 2002-2007, kiriuja"), license, "http://kplayer.sourceforge.net/", "http://kplayer.sourceforge.net/email.html");
  about -> setLicenseText (license);
  about -> addAuthor (ki18n("kiriuja"), KLocalizedString(), "http://kplayer.sourceforge.net/email.html");
  KCmdLineArgs::init (argc, argv, about);
  KCmdLineOptions options;
  options.add ("play", ki18n("Play the files immediately (default)"));
  options.add ("queue", ki18n("Queue the files for playing"));
  options.add ("play-next", ki18n("Play the files after the currently played file finishes"));
  options.add ("queue-next", ki18n("Queue the files for playing after the currently played file"));
  options.add ("add-to-new-playlist", ki18n("Add the files to a new playlist"));
  options.add ("add-to-playlists", ki18n("Add the files to the list of playlists"));
  options.add ("add-to-collection", ki18n("Add the files to the multimedia collection"));
  options.add ("+[files]", ki18n("File(s), directory/ies, or URL(s)"));
  KCmdLineArgs::addCmdLineOptions (options);  // Add our own options.
  KPlayerApplication::addCmdLineOptions();
  return KPlayerApplication().exec();
}
