/***************************************************************************
                           kplayerkfileplugin.h
                           --------------------
    begin                : Mon Jan 12 2004
    copyright            : (C) 2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERKFILEPLUGIN_H
#define KPLAYERKFILEPLUGIN_H

#include <kfilemetainfo.h>

class KConfig;

/**The KPlayer KFilePlugin class.
  *@author kiriuja
  */
class KPlayerKFilePlugin : public KFilePlugin
{
  Q_OBJECT

public:
  KPlayerKFilePlugin (QObject* parent, const char* name, const QStringList& args);
  virtual ~KPlayerKFilePlugin();

  virtual bool readInfo (KFileMetaInfo& info, uint what = KFileMetaInfo::Fastest);
  virtual bool writeInfo (const KFileMetaInfo&) const;
  //virtual QValidator* createValidator (const QString&, const QString&, const QString&, QObject*, const char*) const;

protected:
  void setupMimeTypeInfo (QString mimeType);

  KConfig* m_config;
};

#endif
