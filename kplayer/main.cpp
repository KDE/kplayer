/***************************************************************************
                          main.cpp
                          --------
    begin                : Sat Nov 16 10:12:50 EST 2002
    copyright            : (C) 2002-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kaboutdata.h>
//#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <kdebug.h>

#include "kplayer.h"

static const char* description = I18N_NOOP("KPlayer, a KDE media player based on MPlayer");

static KCmdLineOptions options[] =
{
  { "+[files]", I18N_NOOP("File or list of files to play (optional)"), 0 },
  { 0, 0, 0 }
};

int main (int argc, char *argv[])
{
  KAboutData aboutData ("kplayer", "KPlayer",
    VERSION, description, KAboutData::License_GPL,
    "(C) 2002-2004, kiriuja", 0, 0, "kplayer dash developer at en dash directo dot net");
  aboutData.addAuthor ("kiriuja", 0, "kplayer dash developer at en dash directo dot net");
  KCmdLineArgs::init (argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions (options);  // Add our own options.
  KPlayerApplication::addCmdLineOptions();
  return KPlayerApplication().exec();
}
