/***************************************************************************
                           kplayerkfileplugin.cpp
                           ----------------------
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

#include <kconfig.h>
#include <kgenericfactory.h>
#include <kurl.h>

#include "kplayerkfileplugin.h"
#include "kplayerkfileplugin.moc"

typedef KGenericFactory<KPlayerKFilePlugin> KPlayerKFilePluginFactory;
K_EXPORT_COMPONENT_FACTORY (kfile_kplayer, KPlayerKFilePluginFactory ("kfile_kplayer"));

KPlayerKFilePlugin::KPlayerKFilePlugin (QObject* parent, const char* name, const QStringList& args)
  : KFilePlugin (parent, name, args)
{
  m_config = new KConfig ("kplayerplaylistrc");
  setupMimeTypeInfo ("audio/mpeg");
  setupMimeTypeInfo ("audio/mp4");
  setupMimeTypeInfo ("video/mpeg");
  setupMimeTypeInfo ("video/x-mpeg");
  setupMimeTypeInfo ("video/x-mpeg2");
  setupMimeTypeInfo ("video/mp4");
  setupMimeTypeInfo ("video/x-msmpeg");
  setupMimeTypeInfo ("video/vnd.mpegurl");
  setupMimeTypeInfo ("video/x-mpegurl");
  setupMimeTypeInfo ("video/x-msvideo");
  setupMimeTypeInfo ("video/quicktime");
  setupMimeTypeInfo ("audio/x-ms-asf");
  setupMimeTypeInfo ("audio/x-ms-wma");
  setupMimeTypeInfo ("audio/x-ms-wax");
  setupMimeTypeInfo ("audio/x-ms-wmp");
  setupMimeTypeInfo ("video/x-ms-asf");
  setupMimeTypeInfo ("video/x-ms-afs");
  setupMimeTypeInfo ("video/x-ms-wmp");
  setupMimeTypeInfo ("video/x-ms-wmv");
  setupMimeTypeInfo ("video/x-ms-wma");
  setupMimeTypeInfo ("video/x-ms-wvx");
  setupMimeTypeInfo ("video/x-ms-wmx");
  setupMimeTypeInfo ("application/x-ms-asf");
  setupMimeTypeInfo ("application/vnd.ms-asf");
  setupMimeTypeInfo ("audio/x-scpls");
  setupMimeTypeInfo ("audio/x-pls");
  setupMimeTypeInfo ("audio/vnd.mpegurl");
  setupMimeTypeInfo ("audio/x-mpegurl");
  setupMimeTypeInfo ("audio/x-realaudio");
  setupMimeTypeInfo ("audio/x-pn-realaudio");
  setupMimeTypeInfo ("audio/x-pn-realaudio-plugin");
  setupMimeTypeInfo ("application/x-pn-realaudio");
  setupMimeTypeInfo ("audio/vnd.rn-realaudio");
  setupMimeTypeInfo ("video/x-realvideo");
  setupMimeTypeInfo ("video/x-pn-realvideo");
  setupMimeTypeInfo ("video/vnd.rn-realvideo");
  setupMimeTypeInfo ("video/x-pn-realvideo-plugin");
  setupMimeTypeInfo ("application/vnd.rn-realmedia");
  setupMimeTypeInfo ("application/vnd.rn-realplayer");
  setupMimeTypeInfo ("application/smil");
  setupMimeTypeInfo ("video/nsv");
  setupMimeTypeInfo ("audio/x-mp3");
  setupMimeTypeInfo ("video/x-ogm");
  setupMimeTypeInfo ("video/x-theora");
  setupMimeTypeInfo ("audio/vorbis");
  setupMimeTypeInfo ("audio/x-vorbis");
  setupMimeTypeInfo ("audio/x-oggflac");
  setupMimeTypeInfo ("application/x-ogg");
  setupMimeTypeInfo ("application/ogg");
  setupMimeTypeInfo ("audio/x-mp2");
  setupMimeTypeInfo ("audio/x-wav");
  setupMimeTypeInfo ("video/x-avi");
  setupMimeTypeInfo ("video/x-divx");
  setupMimeTypeInfo ("video/x-matroska");
  setupMimeTypeInfo ("audio/x-matroska");
  setupMimeTypeInfo ("application/x-mplayer2");
  setupMimeTypeInfo ("application/x-cda");
  setupMimeTypeInfo ("audio/aac");
  setupMimeTypeInfo ("audio/ac3");
  setupMimeTypeInfo ("audio/basic");
  setupMimeTypeInfo ("audio/x-adpcm");
}

void KPlayerKFilePlugin::setupMimeTypeInfo (QString mimeType)
{
  KFileMimeTypeInfo* info = addMimeTypeInfo (mimeType);
  KFileMimeTypeInfo::GroupInfo* group = addGroupInfo (info, "General", i18n("General"));
  KFileMimeTypeInfo::ItemInfo* item = addItemInfo (group, "Name", i18n("Name"), QVariant::String);
  setAttributes (item, KFileMimeTypeInfo::Modifiable);
  setHint (item, KFileMimeTypeInfo::Name);
  item = addItemInfo (group, "Length", i18n("Length"), QVariant::Int);
  setAttributes (item,  KFileMimeTypeInfo::Cummulative);
  setUnit (item, KFileMimeTypeInfo::Seconds);
  setHint (item, KFileMimeTypeInfo::Length);
  item = addItemInfo (group, "Resolution", i18n("Resolution"), QVariant::Size);
  setUnit (item, KFileMimeTypeInfo::Pixels);
  setHint (item, KFileMimeTypeInfo::Size);
  group = addGroupInfo (info, "Technical", i18n("Technical Details"));
  item = addItemInfo (group, "Frame Rate", i18n("Frame Rate"), QVariant::Double);
  setAttributes (item, KFileMimeTypeInfo::Averaged);
  setUnit (item, KFileMimeTypeInfo::FramesPerSecond);
  //setSuffix (item, i18n("fps"));
  item = addItemInfo (group, "Video Codec", i18n("Video Codec"), QVariant::String);
  item = addItemInfo (group, "Video Bitrate", i18n("Video Bitrate"), QVariant::Int);
  setAttributes (item, KFileMimeTypeInfo::Averaged);
  setHint (item, KFileMimeTypeInfo::Bitrate);
  setUnit (item, KFileMimeTypeInfo::BitsPerSecond);
  //setSuffix (item, i18n("kbps"));
  item = addItemInfo (group, "Audio Codec", i18n("Audio Codec"), QVariant::String);
  item = addItemInfo (group, "Audio Bitrate", i18n("Audio Bitrate"), QVariant::Int);
  setAttributes (item, KFileMimeTypeInfo::Averaged);
  setHint (item, KFileMimeTypeInfo::Bitrate);
  setUnit (item, KFileMimeTypeInfo::BitsPerSecond);
  //setSuffix (item, i18n("kbps"));
}

KPlayerKFilePlugin::~KPlayerKFilePlugin()
{
}

bool KPlayerKFilePlugin::readInfo (KFileMetaInfo& info, uint)
{
  QString configGroup (KURL::fromPathOrURL (info.path()).url());
  bool has = m_config -> hasGroup (configGroup);
  QString s;
  if ( has )
  {
    m_config -> setGroup (configGroup);
    s = m_config -> readEntry ("Name");
  }
  KFileMetaInfoGroup group = appendGroup (info, "General");
  appendItem (group, "Name", s);
  if ( ! has )
    return true;
  int i = int (m_config -> readDoubleNumEntry ("Length") + 0.5);
  if ( i > 0 )
    appendItem (group, "Length", i);
  QSize size (m_config -> readSizeEntry ("Video Size"));
  if ( ! size.isEmpty() )
    appendItem (group, "Resolution", size);
  group = appendGroup (info, "Technical");
  double d = m_config -> readDoubleNumEntry ("Framerate");
  if ( d > 0 )
    appendItem (group, "Frame Rate", d);
  s = m_config -> readEntry ("Video Codec");
  if ( ! s.isEmpty() )
    appendItem (group, "Video Codec", s);
  i = m_config -> readNumEntry ("Video Bitrate");
  if ( i > 0 )
    appendItem (group, "Video Bitrate", i * 1000);
  s = m_config -> readEntry ("Audio Codec");
  if ( ! s.isEmpty() )
    appendItem (group, "Audio Codec", s);
  i = m_config -> readNumEntry ("Audio Bitrate");
  if ( i > 0 )
    appendItem (group, "Audio Bitrate", i * 1000);
  return true;
}

bool KPlayerKFilePlugin::writeInfo (const KFileMetaInfo& info) const
{
  QString configGroup (KURL::fromPathOrURL(info.path()).url());
  m_config -> setGroup (configGroup);
  QString s (info.group ("General").item ("Name").value().toString());
  if ( s.isEmpty() )
    m_config -> deleteEntry ("Name");
  else
    m_config -> writeEntry ("Name", s);
  m_config -> sync();
  return true;
}

/*QValidator* KPlayerKFilePlugin::createValidator (const QString&, const QString&, const QString& key, QObject*, const char*) const
{
  return 0;
}*/
