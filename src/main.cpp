/***************************************************************************
                          main.cpp
                          --------
    begin                : Sat Nov 16 10:12:50 EST 2002
    copyright            : (C) 2002-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KStandardDirs>
#include <QFile>

#ifdef DEBUG
#include <KDebug>
kdbgstream kdDebugTime (void);
#define DEBUG_KPLAYER_MAIN
#endif

#include "kplayer.h"

static const KLocalizedString description = ki18n("KPlayer, a KDE media player");
KAboutData* about;

int main (int argc, char *argv[])
{
#ifdef DEBUG_KPLAYER_MAIN
  kdDebugTime() << "Welcome to KPlayer!\n";
#endif
  about = new KAboutData ("kplayer", 0, ki18n("KPlayer"), "0.7.2", description, KAboutData::License_GPL_V3,
    ki18n("(C) 2002-2008, Kirill Bulygin"), KLocalizedString());
  about -> addAuthor (ki18n("Kirill Bulygin"), KLocalizedString(), "http://kplayer.sourceforge.net/email.html");
  KComponentData* data = new KComponentData (about);
  QString localdir (KGlobal::dirs() -> localkdedir());
#ifdef DEBUG_KPLAYER_MAIN
  kdDebugTime() << " Local directory " << localdir << "\n";
#endif
  QString rcpath (KStandardDirs::locateLocal ("config", "kplayerrc"));
  if ( localdir.endsWith ("/.kde4/") )
  {
    QString plpath (KStandardDirs::locateLocal ("config", "kplayerplaylistrc"));
    QString mlpath (KStandardDirs::locateLocal ("config", "kplayerlibraryrc"));
#ifdef DEBUG_KPLAYER_MAIN
    kdDebugTime() << " Config path " << rcpath << "\n";
    kdDebugTime() << " Cache path " << plpath << "\n";
    kdDebugTime() << " Library path " << mlpath << "\n";
#endif
    if ( ! QFile::exists (rcpath) && ! QFile::exists (plpath) && ! QFile::exists (mlpath) )
    {
      QString path (rcpath.left (localdir.length() - 2) + rcpath.mid (localdir.length() - 1));
      if ( rcpath.startsWith (localdir) && QFile::exists (path) )
      {
#ifdef DEBUG_KPLAYER_MAIN
        kdDebugTime() << " Copying old config\n";
#endif
        QFile::copy (path, rcpath);
      }
      path = plpath.left (localdir.length() - 2) + plpath.mid (localdir.length() - 1);
      if ( plpath.startsWith (localdir) && QFile::exists (path) )
      {
#ifdef DEBUG_KPLAYER_MAIN
        kdDebugTime() << " Copying old cache\n";
#endif
        QFile::copy (path, plpath);
      }
      path = mlpath.left (localdir.length() - 2) + mlpath.mid (localdir.length() - 1);
      if ( mlpath.startsWith (localdir) && QFile::exists (path) )
      {
#ifdef DEBUG_KPLAYER_MAIN
        kdDebugTime() << " Copying old library\n";
#endif
        QFile::copy (path, mlpath);
      }
    }
  }
  delete data;
#ifdef DEBUG_KPLAYER_MAIN
  kdDebugTime() << "Processing command line\n";
#endif
  KCmdLineArgs::init (argc, argv, about);
  KCmdLineOptions options;
  options.add ("play", ki18n("Play the files immediately (default)"));
  options.add ("queue", ki18n("Queue the files for playing"));
  options.add ("play-next", ki18n("Play the files after the currently playing file has finished"));
  options.add ("queue-next", ki18n("Queue the files for playing after the currently playing file"));
  options.add ("add-to-new-playlist", ki18n("Add the files to a new playlist"));
  options.add ("add-to-playlists", ki18n("Add the files to the list of playlists"));
  options.add ("add-to-collection", ki18n("Add the files to the multimedia collection"));
  options.add ("+[files]", ki18n("File(s), directory/ies, or URL(s)"));
  KCmdLineArgs::addCmdLineOptions (options);  // Add our own options.
  KPlayerApplication::addCmdLineOptions();
  return KPlayerApplication().exec();
}
