/***************************************************************************
                         kplayerpropertiesdialog.cpp
                         ---------------------------
    begin                : Tue Mar 02 2004
    copyright            : (C) 2004-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kplayerpropertiesdialog.h"
#include "kplayerpropertiesdialog.moc"
#include "kplayerproperties.h"

#include <KLocale>
#include <KMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <cstdlib>

QString languageName (int id, QString language);

#ifdef DEBUG
#define DEBUG_KPLAYER_PROPERTIES_DIALOG
#endif

QString listEntry (QComboBox* combo, bool hasDefault)
{
  static QRegExp re_key_value ("^([^:]+): *(.*)$");
  if ( hasDefault && combo -> currentIndex() == 0 )
    return QString::null;
  if ( combo -> currentIndex() == 0 || ( hasDefault && combo -> currentIndex() == 1 ) )
    return "";
  if ( re_key_value.indexIn (combo -> currentText()) >= 0 )
    return re_key_value.cap(1);
  return QString::null;
}

KPlayerPropertiesDialog::KPlayerPropertiesDialog (void)
{
  setFaceType (KPageDialog::Tree);
  setCaption (i18n("File Properties"));
  setButtons (KDialog::Help | KDialog::Default | KDialog::Ok | KDialog::Apply | KDialog::Cancel);
  connect (this, SIGNAL (applyClicked()), SLOT (apply()));
  connect (this, SIGNAL (okClicked()), SLOT (apply()));
  connect (this, SIGNAL (defaultClicked()), SLOT (defaults()));
  connect (this, SIGNAL (currentPageChanged (KPageWidgetItem*, KPageWidgetItem*)),
    SLOT (pageAboutToShow (KPageWidgetItem*, KPageWidgetItem*)));
}

KPlayerPropertiesDialog::~KPlayerPropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPFP " << x() << "x" << y() << " " << width() << "x" << height() << " Hint " << sizeHint().width() << "x" << sizeHint().height() << "\n";
#endif
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
/*if ( size() == sizeHint() )
  {
    group.deleteEntry ("Properties Dialog Width");
    group.deleteEntry ("Properties Dialog Height");
  }
  else
  {*/
/*group.writeEntry ("Properties Dialog Left", x());
  group.writeEntry ("Properties Dialog Top", y());*/
  group.writeEntry ("Properties Dialog Width", width());
  group.writeEntry ("Properties Dialog Height", height());
  if ( m_page_names.contains (currentPage()) )
    group.writeEntry ("Properties Dialog Page", m_page_names [currentPage()]);
  else
    group.deleteEntry ("Properties Dialog Page");
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesDialog::setup (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDialog::setup\n";
#endif
  setupMedia (url);
  m_general = createGeneralPage();
  m_general -> setup (url);
  KPageWidgetItem* item = new KPageWidgetItem (m_general);
  item -> setName (i18n("General"));
  item -> setHeader (i18n("General Properties"));
  addPage (item);
  m_page_names.insert (item, "general");
  m_size = createSizePage();
  m_size -> setup (url);
  item = new KPageWidgetItem (m_size);
  item -> setName (i18n("Size"));
  item -> setHeader (i18n("Size Properties"));
  addPage (item);
  m_page_names.insert (item, "size");
  m_video = createVideoPage();
  m_video -> setup (url);
  item = new KPageWidgetItem (m_video);
  item -> setName (i18n("Video"));
  item -> setHeader (i18n("Video Properties"));
  addPage (item);
  m_page_names.insert (item, "video");
  m_audio = createAudioPage();
  m_audio -> setup (url);
  item = new KPageWidgetItem (m_audio);
  item -> setName (i18n("Audio"));
  item -> setHeader (i18n("Audio Properties"));
  addPage (item);
  m_page_names.insert (item, "audio");
  m_subtitles = createSubtitlesPage();
  m_subtitles -> setup (url);
  item = new KPageWidgetItem (m_subtitles);
  item -> setName (i18n("Subtitles"));
  item -> setHeader (i18n("Subtitle Properties"));
  addPage (item);
  m_page_names.insert (item, "subtitles");
  m_advanced = createAdvancedPage();
  m_advanced -> setup (url);
  item = new KPageWidgetItem (m_advanced);
  item -> setName (i18n("Advanced"));
  item -> setHeader (i18n("Advanced Properties"));
  addPage (item);
  m_page_names.insert (item, "advanced");
  setHelp ("properties");
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
  QString name (group.readEntry ("Properties Dialog Page"));
  for ( QHash<QObject*, QString>::ConstIterator it (m_page_names.constBegin()); it != m_page_names.constEnd(); ++ it )
    if ( it.value() == name )
      setCurrentPage ((KPageWidgetItem*) it.key());
/*int x = group.readEntry ("Properties Dialog Left", -1);
  int y = group.readEntry ("Properties Dialog Top", -1);*/
  int w = group.readEntry ("Properties Dialog Width", 0);
  int h = group.readEntry ("Properties Dialog Height", 0);
//QSize size (group.readEntry ("Properties Dialog Width", 0),
//  group.readEntry ("Properties Dialog Height", 0));
//QSize hint = minimumSizeHint();
//if ( size.width() < hint.width() || size.height() < hint.height() )
//  size = sizeHint();
//setGeometry (x, y, size.width(), size.height());
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
/*kdDebugTime() << "KPFP position " << x << "x" << y << " size " << w << "x" << h << "\n";
  if ( x >= 0 && y >= 0 )
    move (x, y);*/
#endif
  if ( w > 0 && h > 0 )
    resize (w, h);
}

void KPlayerPropertiesDialog::defaults (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDialog::defaults\n";
#endif
  if ( KMessageBox::warningYesNo (this, i18n("All file properties will be reset.\n\nAre you sure?"))
      != KMessageBox::Yes )
    return;
  properties() -> defaults();
  properties() -> commit();
  m_general -> load();
  m_size -> load();
  m_subtitles -> load();
  m_video -> load();
  m_audio -> load();
  m_advanced -> load();
  setButtonGuiItem (KDialog::Cancel, KStandardGuiItem::close());
}

void KPlayerPropertiesDialog::pageAboutToShow (KPageWidgetItem* current, KPageWidgetItem*)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDialog::pageAboutToShow\n";
#endif
  setHelp (m_page_names.contains (current) ? "properties-" + m_page_names [current] : "properties");
}

void KPlayerPropertiesDialog::apply (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDialog::apply\n";
#endif
  m_general -> save();
  m_size -> save();
  m_subtitles -> save();
  m_audio -> save();
  m_video -> save();
  m_advanced -> save();
  properties() -> commit();
  setButtonGuiItem (KDialog::Cancel, KStandardGuiItem::close());
}

KPlayerPropertiesDialog* KPlayerPropertiesDialog::createDialog (KPlayerTrackProperties* properties)
{
  if ( properties -> has ("Path") )
    return new KPlayerItemPropertiesDialog;
  else
  {
    const QString& type = ((KPlayerMediaProperties*) properties -> parent()) -> type();
    if ( type == "TV" || type == "DVB" )
      return new KPlayerChannelPropertiesDialog;
  }
  return new KPlayerDiskTrackPropertiesDialog;
}

KPlayerDevicePropertiesDialog::~KPlayerDevicePropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying device properties dialog\n";
#endif
}

void KPlayerDevicePropertiesDialog::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerDevicePropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

KPlayerPropertiesGeneral* KPlayerDevicePropertiesDialog::createGeneralPage (void)
{
  return new KPlayerPropertiesDeviceGeneral;
}

KPlayerPropertiesSize* KPlayerDevicePropertiesDialog::createSizePage (void)
{
  return new KPlayerPropertiesDeviceSize;
}

KPlayerPropertiesSubtitles* KPlayerDevicePropertiesDialog::createSubtitlesPage (void)
{
  return new KPlayerPropertiesDeviceSubtitles;
}

KPlayerPropertiesAudio* KPlayerDevicePropertiesDialog::createAudioPage (void)
{
  return new KPlayerPropertiesDeviceAudio;
}

KPlayerPropertiesVideo* KPlayerDevicePropertiesDialog::createVideoPage (void)
{
  return new KPlayerPropertiesDeviceVideo;
}

KPlayerPropertiesAdvanced* KPlayerDevicePropertiesDialog::createAdvancedPage (void)
{
  return new KPlayerPropertiesDeviceAdvanced;
}

KPlayerTVDevicePropertiesDialog::~KPlayerTVDevicePropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying TV device properties dialog\n";
#endif
}

void KPlayerTVDevicePropertiesDialog::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerTVDevicePropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::tvProperties (url);
}

KPlayerPropertiesGeneral* KPlayerTVDevicePropertiesDialog::createGeneralPage (void)
{
  return new KPlayerPropertiesTVDeviceGeneral;
}

KPlayerPropertiesAudio* KPlayerTVDevicePropertiesDialog::createAudioPage (void)
{
  return new KPlayerPropertiesTVDeviceAudio;
}

KPlayerPropertiesVideo* KPlayerTVDevicePropertiesDialog::createVideoPage (void)
{
  return new KPlayerPropertiesTVDeviceVideo;
}

KPlayerPropertiesAdvanced* KPlayerTVDevicePropertiesDialog::createAdvancedPage (void)
{
  return new KPlayerPropertiesTVDeviceAdvanced;
}

KPlayerDVBDevicePropertiesDialog::~KPlayerDVBDevicePropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying DVB device properties dialog\n";
#endif
}

void KPlayerDVBDevicePropertiesDialog::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerDVBDevicePropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::dvbProperties (url);
}

KPlayerPropertiesGeneral* KPlayerDVBDevicePropertiesDialog::createGeneralPage (void)
{
  return new KPlayerPropertiesDVBDeviceGeneral;
}

KPlayerPropertiesAudio* KPlayerDVBDevicePropertiesDialog::createAudioPage (void)
{
  return new KPlayerPropertiesDVBDeviceAudio;
}

KPlayerPropertiesVideo* KPlayerDVBDevicePropertiesDialog::createVideoPage (void)
{
  return new KPlayerPropertiesDVBDeviceVideo;
}

KPlayerDiskTrackPropertiesDialog::~KPlayerDiskTrackPropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying track properties dialog\n";
#endif
}

void KPlayerDiskTrackPropertiesDialog::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerDiskTrackPropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

KPlayerPropertiesGeneral* KPlayerDiskTrackPropertiesDialog::createGeneralPage (void)
{
  return new KPlayerPropertiesDiskTrackGeneral;
}

KPlayerPropertiesSize* KPlayerDiskTrackPropertiesDialog::createSizePage (void)
{
  return new KPlayerPropertiesTrackSize;
}

KPlayerPropertiesSubtitles* KPlayerDiskTrackPropertiesDialog::createSubtitlesPage (void)
{
  return new KPlayerPropertiesDiskTrackSubtitles;
}

KPlayerPropertiesAudio* KPlayerDiskTrackPropertiesDialog::createAudioPage (void)
{
  return new KPlayerPropertiesTrackAudio;
}

KPlayerPropertiesVideo* KPlayerDiskTrackPropertiesDialog::createVideoPage (void)
{
  return new KPlayerPropertiesTrackVideo;
}

KPlayerPropertiesAdvanced* KPlayerDiskTrackPropertiesDialog::createAdvancedPage (void)
{
  return new KPlayerPropertiesTrackAdvanced;
}

KPlayerChannelPropertiesDialog::~KPlayerChannelPropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying channel properties dialog\n";
#endif
}

KPlayerPropertiesGeneral* KPlayerChannelPropertiesDialog::createGeneralPage (void)
{
  return new KPlayerPropertiesChannelGeneral;
}

KPlayerPropertiesSubtitles* KPlayerChannelPropertiesDialog::createSubtitlesPage (void)
{
  return new KPlayerPropertiesChannelSubtitles;
}

KPlayerItemPropertiesDialog::~KPlayerItemPropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying item properties dialog\n";
#endif
}

KPlayerPropertiesGeneral* KPlayerItemPropertiesDialog::createGeneralPage (void)
{
  return new KPlayerPropertiesItemGeneral;
}

KPlayerPropertiesSubtitles* KPlayerItemPropertiesDialog::createSubtitlesPage (void)
{
  return new KPlayerPropertiesItemSubtitles;
}

KPlayerPropertiesAdvanced* KPlayerItemPropertiesDialog::createAdvancedPage (void)
{
  return new KPlayerPropertiesItemAdvanced;
}

/*void setupHistory (QComboBox* combo, const QStringList& history)
{
  for ( QStringList::ConstIterator it (history.begin()); it != history.end(); ++ it )
  {
    combo -> addItem (*it);
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
    kdDebugTime() << " History " << *it << "\n";
#endif
  }
}*/

KPlayerPropertiesGeneral::KPlayerPropertiesGeneral (QWidget* parent)
  : QFrame (parent)
{
}

KPlayerPropertiesGeneral::~KPlayerPropertiesGeneral()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesGeneral::setup (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::setup\n";
#endif
  setupMedia (url);
  setupUi (this);
  //setupHistory (c_name, properties() -> nameHistory());
  setupControls();
  load();
}

void KPlayerPropertiesGeneral::hideUrl (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::hideUrl\n";
#endif
  l_url -> hide();
  c_url -> hide();
}

void KPlayerPropertiesGeneral::hideFrequency (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::hideFrequency\n";
#endif
  l_frequency -> hide();
  c_frequency -> hide();
  l_mhz -> hide();
}

void KPlayerPropertiesGeneral::hideLength (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::hideLength\n";
#endif
  l_length -> hide();
  c_length -> hide();
}

void KPlayerPropertiesGeneral::hidePlaylist (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::hidePlaylist\n";
#endif
  l_playlist -> hide();
  c_playlist -> hide();
}

void KPlayerPropertiesGeneral::hideTV (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::hideTV\n";
#endif
  l_channels -> hide();
  c_channels -> hide();
  l_driver -> hide();
  c_driver -> hide();
}

void KPlayerPropertiesGeneral::hideDVB (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::hideDVB\n";
#endif
  l_channel_file -> hide();
  c_channel_file -> hide();
}

void KPlayerPropertiesGeneral::load (void)
{
  c_name -> setText (properties() -> name());
  c_name -> setCursorPosition (0);
  c_url -> setText (properties() -> pathString());
  c_url -> setCursorPosition (0);
}

void KPlayerPropertiesGeneral::save (void)
{
  properties() -> setName (c_name -> text());
}

KPlayerPropertiesDeviceGeneral::KPlayerPropertiesDeviceGeneral (QWidget* parent)
  : KPlayerPropertiesGeneral (parent)
{
}

void KPlayerPropertiesDeviceGeneral::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceGeneral::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

void KPlayerPropertiesDeviceGeneral::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceGeneral::setupControls\n";
#endif
  hideFrequency();
  hideLength();
  hidePlaylist();
  hideTV();
  hideDVB();
}

void KPlayerPropertiesDeviceGeneral::load (void)
{
  c_type -> setText (properties() -> typeString());
  KPlayerPropertiesGeneral::load();
}

KPlayerPropertiesTVDeviceGeneral::KPlayerPropertiesTVDeviceGeneral (QWidget* parent)
  : KPlayerPropertiesDeviceGeneral (parent)
{
}

void KPlayerPropertiesTVDeviceGeneral::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceGeneral::setupMedia\n";
#endif
  m_properties = KPlayerMedia::tvProperties (url);
}

void KPlayerPropertiesTVDeviceGeneral::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceGeneral::setupControls\n";
#endif
  hideFrequency();
  hideLength();
  hidePlaylist();
  hideDVB();
  for ( uint i = 0; i < channellistcount; i ++ )
    c_channels -> addItem (i18n(channellists[i].name));
}

void KPlayerPropertiesTVDeviceGeneral::load (void)
{
  const QString& list (properties() -> channelList());
  for ( uint i = 0; c_channels -> count(); i ++ )
    if ( channellists[i].id == list )
    {
      c_channels -> setCurrentIndex (i);
      break;
    }
  const QString& driver (properties() -> inputDriver());
  c_driver -> setCurrentIndex (driver == "bsdbt848" ? 0 : driver == "v4l" ? 1 : 2);
  KPlayerPropertiesDeviceGeneral::load();
}

void KPlayerPropertiesTVDeviceGeneral::save (void)
{
  properties() -> setChannelList (channellists[c_channels -> currentIndex()].id);
  int driver = c_driver -> currentIndex();
  properties() -> setInputDriver (driver == 0 ? "bsdbt848" : driver == 1 ? "v4l" : "v4l2");
  KPlayerPropertiesDeviceGeneral::save();
}

KPlayerPropertiesDVBDeviceGeneral::KPlayerPropertiesDVBDeviceGeneral (QWidget* parent)
  : KPlayerPropertiesDeviceGeneral (parent)
{
}

void KPlayerPropertiesDVBDeviceGeneral::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDVBDeviceGeneral::setupMedia\n";
#endif
  m_properties = KPlayerMedia::dvbProperties (url);
}

void KPlayerPropertiesDVBDeviceGeneral::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDVBDeviceGeneral::setupControls\n";
#endif
  hideFrequency();
  hideLength();
  hidePlaylist();
  hideTV();
}

void KPlayerPropertiesDVBDeviceGeneral::load (void)
{
  c_channel_file -> setText (properties() -> channelList());
  KPlayerPropertiesDeviceGeneral::load();
}

void KPlayerPropertiesDVBDeviceGeneral::save (void)
{
  if ( ! c_channel_file -> text().isEmpty() )
    properties() -> setChannelList (c_channel_file -> text());
  KPlayerPropertiesDeviceGeneral::save();
}

KPlayerPropertiesTrackGeneral::KPlayerPropertiesTrackGeneral (QWidget* parent)
  : KPlayerPropertiesGeneral (parent)
{
}

void KPlayerPropertiesTrackGeneral::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackGeneral::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

void KPlayerPropertiesTrackGeneral::load (void)
{
  c_length -> setText (properties() -> lengthString());
  KPlayerPropertiesGeneral::load();
}

KPlayerPropertiesDiskTrackGeneral::KPlayerPropertiesDiskTrackGeneral (QWidget* parent)
  : KPlayerPropertiesTrackGeneral (parent)
{
}

void KPlayerPropertiesDiskTrackGeneral::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDiskTrackGeneral::setupControls\n";
#endif
  hideUrl();
  hideFrequency();
  hidePlaylist();
  hideTV();
  hideDVB();
}

void KPlayerPropertiesDiskTrackGeneral::load (void)
{
  c_type -> setText (properties() -> parent() -> typeString());
  KPlayerPropertiesTrackGeneral::load();
}

KPlayerPropertiesChannelGeneral::KPlayerPropertiesChannelGeneral (QWidget* parent)
  : KPlayerPropertiesDiskTrackGeneral (parent)
{
}

void KPlayerPropertiesChannelGeneral::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesChannelGeneral::setupControls\n";
#endif
  hideUrl();
  hideLength();
  hidePlaylist();
  hideTV();
  hideDVB();
  c_frequency -> setReadOnly (! properties() -> canChangeFrequency());
}

void KPlayerPropertiesChannelGeneral::load (void)
{
  c_frequency -> setText (properties() -> frequencyString());
  KPlayerPropertiesDiskTrackGeneral::load();
}

void KPlayerPropertiesChannelGeneral::save (void)
{
  properties() -> setFrequency (int (fabs (c_frequency -> text().toFloat()) + 0.5));
  KPlayerPropertiesDiskTrackGeneral::save();
}

KPlayerPropertiesItemGeneral::KPlayerPropertiesItemGeneral (QWidget* parent)
  : KPlayerPropertiesTrackGeneral (parent)
{
}

void KPlayerPropertiesItemGeneral::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesItemGeneral::setupControls\n";
#endif
  hideFrequency();
  hideTV();
  hideDVB();
}

void KPlayerPropertiesItemGeneral::load (void)
{
  c_type -> setText (properties() -> typeString());
  c_playlist -> setCurrentIndex (properties() -> playlistOption());
  KPlayerPropertiesTrackGeneral::load();
}

void KPlayerPropertiesItemGeneral::save (void)
{
  properties() -> setPlaylistOption (c_playlist -> currentIndex());
  KPlayerPropertiesTrackGeneral::save();
}

KPlayerPropertiesSize::KPlayerPropertiesSize (QWidget* parent)
  : QFrame (parent)
{
}

KPlayerPropertiesSize::~KPlayerPropertiesSize()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesSize::setup (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSize::setup\n";
#endif
  setupMedia (url);
  setupUi (this);
  setupControls();
  load();
}

void KPlayerPropertiesSize::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSize::setupControls\n";
#endif
}

void KPlayerPropertiesSize::load (void)
{
  c_display_size -> setCurrentIndex (properties() -> displaySizeOption());
  displaySizeChanged (c_display_size -> currentIndex());
  c_full_screen -> setCurrentIndex (properties() -> fullScreenOption());
  c_maximized -> setCurrentIndex (properties() -> maximizedOption());
  c_maintain_aspect -> setCurrentIndex (properties() -> maintainAspectOption());
}

void KPlayerPropertiesSize::save (void)
{
  int width = labs (c_display_width -> text().toInt());
  int height = labs (c_display_height -> text().toInt());
  if ( width == 0 && c_display_size -> currentIndex() == 2 && c_display_width -> text().trimmed().toDouble() > 0 )
  {
    QRegExp re ("^\\s*(\\d*)[,.](\\d*)\\s*$");
    if ( re.indexIn (c_display_width -> text()) >= 0 )
    {
      width = (re.cap(1) + re.cap(2)).toInt();
      for ( int i = 0; i < re.cap(2).length(); i ++ )
        height *= 10;
    }
  }
  if ( c_display_size -> currentIndex() == 2 )
    for ( int i = 2; i <= height; i ++ )
      if ( width / i * i == width && height / i * i == height )
      {
        width /= i;
        height /= i --;
      }
  properties() -> setDisplaySize (QSize (width, height), c_display_size -> currentIndex());
  properties() -> setFullScreenOption (c_full_screen -> currentIndex());
  properties() -> setMaximizedOption (c_maximized -> currentIndex());
  properties() -> setMaintainAspectOption (c_maintain_aspect -> currentIndex());
}

void KPlayerPropertiesSize::displaySizeChanged (int option)
{
  bool enable = option != 0;
  c_display_width -> setEnabled (enable);
  l_display_by -> setEnabled (enable);
  c_display_height -> setEnabled (enable);
  c_display_width -> setText (enable ? properties() -> displayWidthString() : "");
  c_display_height -> setText (enable ? properties() -> displayHeightString() : "");
  if ( enable && sender() )
  {
    c_display_width -> setFocus();
    c_display_width -> selectAll();
  }
}

KPlayerPropertiesDeviceSize::KPlayerPropertiesDeviceSize (QWidget* parent)
  : KPlayerPropertiesSize (parent)
{
}

void KPlayerPropertiesDeviceSize::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceSize::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

void KPlayerPropertiesDeviceSize::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceSize::setupControls\n";
#endif
  l_resolution -> hide();
  c_resolution_width -> hide();
  l_resolution_by -> hide();
  c_resolution_height -> hide();
  l_original_size -> hide();
  c_original_width -> hide();
  l_original_by -> hide();
  c_original_height -> hide();
  l_current_size -> hide();
  c_current_width -> hide();
  l_current_by -> hide();
  c_current_height -> hide();
}

KPlayerPropertiesTrackSize::KPlayerPropertiesTrackSize (QWidget* parent)
  : KPlayerPropertiesSize (parent)
{
}

void KPlayerPropertiesTrackSize::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackSize::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

void KPlayerPropertiesTrackSize::load (void)
{
  c_resolution_width -> setText (properties() -> resolutionWidthString());
  c_resolution_height -> setText (properties() -> resolutionHeightString());
  c_original_width -> setText (properties() -> originalWidthString());
  c_original_height -> setText (properties() -> originalHeightString());
  c_current_width -> setText (properties() -> currentWidthString());
  c_current_height -> setText (properties() -> currentHeightString());
  KPlayerPropertiesSize::load();
}

KPlayerPropertiesSubtitles::KPlayerPropertiesSubtitles (QWidget* parent)
  : QFrame (parent)
{
}

KPlayerPropertiesSubtitles::~KPlayerPropertiesSubtitles()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesSubtitles::setup (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSubtitles::setup\n";
#endif
  setupMedia (url);
  setupUi (this);
  setupControls();
  load();
}

void KPlayerPropertiesSubtitles::hideTrack (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSubtitles::hideTrack\n";
#endif
  l_track -> hide();
  c_track_set -> hide();
  c_track -> hide();
}

void KPlayerPropertiesSubtitles::hideAutoload (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSubtitles::hideAutoload\n";
#endif
  l_autoload -> hide();
  c_autoload -> hide();
}

void KPlayerPropertiesSubtitles::hideUrl (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSubtitles::hideUrl\n";
#endif
  l_url -> hide();
  c_url -> hide();
  l_vobsub -> hide();
  c_vobsub -> hide();
  l_encoding -> hide();
  c_encoding -> hide();
  l_framerate -> hide();
  c_framerate -> hide();
}

void KPlayerPropertiesSubtitles::hideClosedCaption (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSubtitles::hideClosedCaption\n";
#endif
  l_closed_caption -> hide();
  c_closed_caption -> hide();
}

void KPlayerPropertiesSubtitles::load (void)
{
  c_position_set -> setCurrentIndex (properties() -> hasSubtitlePosition() ? 1 : 0);
  positionChanged (c_position_set -> currentIndex());
  c_delay_set -> setCurrentIndex (properties() -> hasSubtitleDelay() ? 1 : 0);
  delayChanged (c_delay_set -> currentIndex());
  c_closed_caption -> setCurrentIndex (properties() -> subtitleClosedCaptionOption());
}

void KPlayerPropertiesSubtitles::save (void)
{
  if ( c_position_set -> currentIndex() )
    properties() -> setSubtitlePositionValue (labs (c_position -> text().toInt()));
  else
    properties() -> resetSubtitlePosition();
  if ( c_delay_set -> currentIndex() )
    properties() -> setSubtitleDelayValue (c_delay -> text().toFloat());
  else
    properties() -> resetSubtitleDelay();
  properties() -> setSubtitleClosedCaptionOption (c_closed_caption -> currentIndex());
}

void KPlayerPropertiesSubtitles::positionChanged (int option)
{
  bool enable = option > 0;
  c_position -> setText (properties() -> subtitlePositionString());
  c_position -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_position -> setFocus();
    c_position -> selectAll();
  }
}

void KPlayerPropertiesSubtitles::delayChanged (int option)
{
  bool enable = option > 0;
  c_delay -> setText (enable ? properties() -> subtitleDelayString() : "");
  c_delay -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_delay -> setFocus();
    c_delay -> selectAll();
  }
}

KPlayerPropertiesDeviceSubtitles::KPlayerPropertiesDeviceSubtitles (QWidget* parent)
  : KPlayerPropertiesSubtitles (parent)
{
}

void KPlayerPropertiesDeviceSubtitles::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceSubtitles::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

void KPlayerPropertiesDeviceSubtitles::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceSubtitles::setupControls\n";
#endif
  hideTrack();
  hideAutoload();
  hideUrl();
}

KPlayerPropertiesTrackSubtitles::KPlayerPropertiesTrackSubtitles (QWidget* parent)
  : KPlayerPropertiesSubtitles (parent)
{
}

void KPlayerPropertiesTrackSubtitles::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackSubtitles::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

void KPlayerPropertiesTrackSubtitles::addTracks (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackSubtitles::addTracks\n";
#endif
  int i = 1;
  const QMap<int, QString>& sids (properties() -> subtitleIDs());
  QMap<int, QString>::ConstIterator iterator (sids.constBegin()), end (sids.constEnd());
  while ( iterator != end )
  {
    c_track_set -> addItem (languageName (iterator.key(), iterator.value()), i);
    ++ iterator;
    ++ i;
  }
  const QMap<int, QString>& vsids (properties() -> vobsubIDs());
  iterator = vsids.constBegin();
  end = vsids.constEnd();
  while ( iterator != end )
  {
    c_track_set -> addItem (languageName (iterator.key(), iterator.value()), i);
    ++ iterator;
    ++ i;
  }
}

void KPlayerPropertiesTrackSubtitles::load (void)
{
  int option = properties() -> subtitleOption();
  if ( option == c_track_set -> count() - 1 )
    option = 0;
  c_track_set -> setCurrentIndex (option);
  trackChanged (c_track_set -> currentIndex());
  KPlayerPropertiesSubtitles::load();
}

void KPlayerPropertiesTrackSubtitles::save (void)
{
  if ( c_track_set -> currentIndex() == c_track_set -> count() - 1 )
    properties() -> setSubtitleID (labs (c_track -> text().toInt()));
  else
    properties() -> setSubtitleOption (c_track_set -> currentIndex());
  KPlayerPropertiesSubtitles::save();
}

void KPlayerPropertiesTrackSubtitles::trackChanged (int option)
{
  bool enable = option == c_track_set -> count() - 1;
  c_track -> setText (enable ? properties() -> subtitleIDString() : "");
  c_track -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_track -> setFocus();
    c_track -> selectAll();
  }
}

KPlayerPropertiesChannelSubtitles::KPlayerPropertiesChannelSubtitles (QWidget* parent)
  : KPlayerPropertiesTrackSubtitles (parent)
{
}

void KPlayerPropertiesChannelSubtitles::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesChannelSubtitles::setupControls\n";
#endif
  c_track_set -> removeItem (1);
  addTracks();
  hideAutoload();
  hideUrl();
}

KPlayerPropertiesDiskTrackSubtitles::KPlayerPropertiesDiskTrackSubtitles (QWidget* parent)
  : KPlayerPropertiesTrackSubtitles (parent)
{
}

struct KPlayerSubtitleEncoding
{
  /** Encoding ID. */
  const char* id;
  /** Encoding name. */
  const char* name;
};

struct KPlayerSubtitleEncoding subtitleencodings[] = {
  { "UTF-8", I18N_NOOP("Unicode") },
  { "UTF-16", I18N_NOOP("Unicode") },
  { "UTF-16BE", I18N_NOOP("Unicode") },
  { "UTF-16LE", I18N_NOOP("Unicode") },
  { "UTF-32", I18N_NOOP("Unicode") },
  { "UTF-32BE", I18N_NOOP("Unicode") },
  { "UTF-32LE", I18N_NOOP("Unicode") },
  { "UCS-2", I18N_NOOP("Unicode") },
  { "UCS-2BE", I18N_NOOP("Unicode") },
  { "UCS-2LE", I18N_NOOP("Unicode") },
  { "UCS-4", I18N_NOOP("Unicode") },
  { "UCS-4BE", I18N_NOOP("Unicode") },
  { "UCS-4LE", I18N_NOOP("Unicode") },
  { "ISO-8859-1", I18N_NOOP("Western Europe") },
  { "ISO-8859-2", I18N_NOOP("Central and Eastern Europe") },
  { "ISO-8859-3", I18N_NOOP("Southern Europe") },
  { "ISO-8859-4", I18N_NOOP("Northern Europe") },
  { "ISO-8859-5", I18N_NOOP("Cyrillic") },
  { "ISO-8859-6", I18N_NOOP("Arabic") },
  { "ISO-8859-7", I18N_NOOP("Greek") },
  { "ISO-8859-8", I18N_NOOP("Hebrew") },
  { "ISO-8859-9", I18N_NOOP("Turkish") },
  { "ISO-8859-10", I18N_NOOP("Nordic") },
  { "ISO-8859-11", I18N_NOOP("Thai") },
  { "ISO-8859-13", I18N_NOOP("Baltic") },
  { "ISO-8859-14", I18N_NOOP("Celtic") },
  { "ISO-8859-15", I18N_NOOP("Western Europe") },
  { "ISO-8859-16", I18N_NOOP("South-Eastern Europe") },
  { "ARMSCII-8", I18N_NOOP("Armenian") },
  { "GEORGIAN-ACADEMY", I18N_NOOP("Georgian") },
  { "GEORGIAN-PS", I18N_NOOP("Georgian") },
  { "KOI8-R", I18N_NOOP("Russian") },
  { "KOI8-U", I18N_NOOP("Ukrainian") },
  { "KOI8-RU", I18N_NOOP("Cyrillic") },
  { "KOI8-T", I18N_NOOP("Tajik") },
  { "MULELAO-1", I18N_NOOP("Lao") },
  { "PT154", I18N_NOOP("Kazakh") },
  { "TIS-620", I18N_NOOP("Thai") },
  { "VISCII", I18N_NOOP("Vietnamese") },
  { "TCVN", I18N_NOOP("Vietnamese") },
  { "ISO646-JP", I18N_NOOP("Japanese") },
  { "JIS_X0201", I18N_NOOP("Japanese") },
  { "JIS_X0208", I18N_NOOP("Japanese") },
  { "JIS_X0212", I18N_NOOP("Japanese") },
  { "EUC-JP", I18N_NOOP("Japanese") },
  { "SHIFT_JIS", I18N_NOOP("Japanese") },
  { "ISO-2022-JP", I18N_NOOP("Japanese") },
  { "ISO-2022-JP-1", I18N_NOOP("Japanese") },
  { "ISO-2022-JP-2", I18N_NOOP("Japanese") },
  { "ISO646-CN", I18N_NOOP("Simplified Chinese") },
  { "GB_2312-80", I18N_NOOP("Simplified Chinese") },
  { "EUC-CN", I18N_NOOP("Simplified Chinese") },
  { "GBK", I18N_NOOP("Simplified Chinese") },
  { "GB18030", I18N_NOOP("Simplified Chinese") },
  { "ISO-2022-CN", I18N_NOOP("Simplified Chinese") },
  { "HZ", I18N_NOOP("Simplified Chinese") },
  { "EUC-TW", I18N_NOOP("Traditional Chinese") },
  { "BIG5", I18N_NOOP("Traditional Chinese") },
  { "BIG5-HKSCS", I18N_NOOP("Hong Kong") },
  { "ISO-2022-CN-EXT", I18N_NOOP("Hong Kong") },
  { "KSC_5601", I18N_NOOP("Korean") },
  { "EUC-KR", I18N_NOOP("Korean") },
  { "JOHAB", I18N_NOOP("Korean") },
  { "ISO-2022-KR", I18N_NOOP("Korean") },
  { "CP850", I18N_NOOP("Western Europe") },
  { "CP862", I18N_NOOP("Hebrew") },
  { "CP866", I18N_NOOP("Cyrillic") },
  { "CP874", I18N_NOOP("Thai") },
  { "CP932", I18N_NOOP("Japanese") },
  { "CP936", I18N_NOOP("Simplified Chinese") },
  { "CP949", I18N_NOOP("Korean") },
  { "CP950", I18N_NOOP("Traditional Chinese") },
  { "CP1133", I18N_NOOP("Lao") },
  { "CP1250", I18N_NOOP("Central and Eastern Europe") },
  { "CP1251", I18N_NOOP("Cyrillic") },
  { "CP1252", I18N_NOOP("Western Europe") },
  { "CP1253", I18N_NOOP("Greek") },
  { "CP1254", I18N_NOOP("Turkish") },
  { "CP1255", I18N_NOOP("Hebrew") },
  { "CP1256", I18N_NOOP("Arabic") },
  { "CP1257", I18N_NOOP("Baltic") },
  { "CP1258", I18N_NOOP("Vietnamese") }
};

const float framerates[] = {
  14.985,
  15,
  23.976,
  24,
  25,
  29.97,
  30,
  50,
  59.94,
  60
};

void fillEncodingCombobox (QComboBox* combobox)
{
  for ( uint i = 0; i < sizeof (subtitleencodings) / sizeof (struct KPlayerSubtitleEncoding); i ++ )
  {
    const struct KPlayerSubtitleEncoding& encoding = subtitleencodings[i];
    combobox -> addItem (QString (encoding.id) + ": " + i18n(encoding.name));
  }
}

void KPlayerPropertiesDiskTrackSubtitles::setupEncoding (void)
{
  fillEncodingCombobox (c_encoding);
  for ( uint i = 0; i < sizeof (framerates) / sizeof (float); ++ i )
    c_framerate -> addItem (QString::number (framerates[i]));
}

void KPlayerPropertiesDiskTrackSubtitles::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDiskTrackSubtitles::setupControls\n";
#endif
  addTracks();
  setupEncoding();
  hideAutoload();
}

void KPlayerPropertiesDiskTrackSubtitles::load (void)
{
  c_url -> setText (properties() -> subtitlePath());
  c_vobsub -> setCurrentIndex (properties() -> vobsubSubtitlesOption());
  if ( properties() -> hasSubtitleEncoding() )
  {
    QString encoding = properties() -> subtitleEncoding();
    if ( encoding.isEmpty() )
      c_encoding -> setCurrentIndex (1);
    else
    {
      c_encoding -> setEditText (encoding);
      encoding += ": ";
      for ( int i = 2; i < c_encoding -> count(); ++ i )
        if ( c_encoding -> itemText (i).startsWith (encoding) )
        {
          c_encoding -> setCurrentIndex (i);
          break;
        }
    }
  }
  else
    c_encoding -> setCurrentIndex (0);
  if ( properties() -> hasSubtitleFramerate() )
    c_framerate -> setEditText (properties() -> subtitleFramerateString());
  else
    c_framerate -> setCurrentIndex (0);
  KPlayerPropertiesTrackSubtitles::load();
}

void KPlayerPropertiesDiskTrackSubtitles::save (void)
{
  properties() -> setSubtitleUrl (c_url -> text());
  properties() -> setVobsubSubtitlesOption (c_vobsub -> currentIndex());
  if ( c_encoding -> currentIndex() == 0 )
    properties() -> resetSubtitleEncoding();
  else if ( c_encoding -> currentIndex() == 1 )
    properties() -> setSubtitleEncoding ("");
  else
    properties() -> setSubtitleEncoding (c_encoding -> currentText().section (':', 0, 0));
  if ( c_framerate -> currentIndex() )
    properties() -> setSubtitleFramerate (c_framerate -> currentText().toFloat());
  else
    properties() -> resetSubtitleFramerate();
  KPlayerPropertiesTrackSubtitles::save();
}

KPlayerPropertiesItemSubtitles::KPlayerPropertiesItemSubtitles (QWidget* parent)
  : KPlayerPropertiesDiskTrackSubtitles (parent)
{
}

void KPlayerPropertiesItemSubtitles::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesItemSubtitles::setupControls\n";
#endif
  addTracks();
  setupEncoding();
  if ( ! properties() -> url().isLocalFile() )
    hideAutoload();
  hideClosedCaption();
}

void KPlayerPropertiesItemSubtitles::load (void)
{
  if ( properties() -> url().isLocalFile() )
    c_autoload -> setCurrentIndex (properties() -> subtitleAutoloadOption());
  KPlayerPropertiesDiskTrackSubtitles::load();
}

void KPlayerPropertiesItemSubtitles::save (void)
{
  if ( properties() -> url().isLocalFile() )
    properties() -> setSubtitleAutoloadOption (c_autoload -> currentIndex());
  KPlayerPropertiesDiskTrackSubtitles::save();
}

KPlayerPropertiesAudio::KPlayerPropertiesAudio (QWidget* parent)
  : QFrame (parent)
{
}

KPlayerPropertiesAudio::~KPlayerPropertiesAudio()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesAudio::setup (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAudio::setup\n";
#endif
  setupMedia (url);
  setupUi (this);
  if ( engine() -> audioCodecCount() )
  {
    c_codec -> clear();
    const QString& codec = properties() -> audioCodecValue();
    if ( codec.isEmpty() )
      c_codec -> addItem (i18n("default"));
    else
      c_codec -> addItem (i18n("%1 (%2)", i18n("default"), codec));
    c_codec -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> audioCodecCount(); i ++ )
      c_codec -> addItem (engine() -> audioCodecName (i));
  }
  setupControls();
  load();
}

void KPlayerPropertiesAudio::hideTrack (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAudio::hideTrack\n";
#endif
  l_track -> hide();
  c_track_set -> hide();
  c_track -> hide();
}

void KPlayerPropertiesAudio::hideInput (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAudio::hideInput\n";
#endif
  l_input -> hide();
  c_input_set -> hide();
  c_input -> hide();
}

void KPlayerPropertiesAudio::hideTV (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAudio::hideTV\n";
#endif
  l_mode -> hide();
  c_mode -> hide();
  c_immediate -> hide();
  l_capture -> hide();
  c_capture -> hide();
  l_device -> hide();
  c_device -> hide();
}

void KPlayerPropertiesAudio::hideRates (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAudio::hideRates\n";
#endif
  l_bitrate -> hide();
  c_bitrate -> hide();
  l_kbps -> hide();
  l_samplerate -> hide();
  c_samplerate -> hide();
  l_hz -> hide();
}

void KPlayerPropertiesAudio::load (void)
{
  c_volume_set -> setCurrentIndex (properties() -> volumeOption());
  volumeChanged (c_volume_set -> currentIndex());
  c_delay_set -> setCurrentIndex (properties() -> hasAudioDelay() ? 1 : 0);
  delayChanged (c_delay_set -> currentIndex());
  const QString& codec (properties() -> audioCodecOption());
  c_codec -> setCurrentIndex (codec.isNull() ? 0 : engine() -> audioCodecIndex (codec) + 2);
}

void KPlayerPropertiesAudio::save (void)
{
  properties() -> setVolumeOption (labs (c_volume -> text().toInt()), c_volume_set -> currentIndex());
  if ( c_delay_set -> currentIndex() )
    properties() -> setAudioDelayValue (c_delay -> text().toFloat());
  else
    properties() -> resetAudioDelay();
  properties() -> setAudioCodecOption (listEntry (c_codec, true));
}

void KPlayerPropertiesAudio::volumeChanged (int option)
{
  bool enable = option > 0;
  c_volume -> setText (enable ? properties() -> volumeString() : "");
  c_volume -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_volume -> setFocus();
    c_volume -> selectAll();
  }
}

void KPlayerPropertiesAudio::delayChanged (int option)
{
  bool enable = option > 0;
  c_delay -> setText (enable ? properties() -> audioDelayString() : "");
  c_delay -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_delay -> setFocus();
    c_delay -> selectAll();
  }
}

KPlayerPropertiesDeviceAudio::KPlayerPropertiesDeviceAudio (QWidget* parent)
  : KPlayerPropertiesAudio (parent)
{
}

void KPlayerPropertiesDeviceAudio::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceAudio::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

void KPlayerPropertiesDeviceAudio::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceAudio::setupControls\n";
#endif
  hideTrack();
  hideRates();
  hideInput();
  hideTV();
}

KPlayerPropertiesTVDeviceAudio::KPlayerPropertiesTVDeviceAudio (QWidget* parent)
  : KPlayerPropertiesDVBDeviceAudio (parent)
{
}

void KPlayerPropertiesTVDeviceAudio::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceAudio::setupMedia\n";
#endif
  m_properties = KPlayerMedia::tvProperties (url);
}

void KPlayerPropertiesTVDeviceAudio::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceAudio::setupControls\n";
#endif
  hideTrack();
  hideRates();
}

void KPlayerPropertiesTVDeviceAudio::load (void)
{
  c_mode -> setCurrentIndex (properties() -> audioModeOption());
  c_immediate -> setChecked (properties() -> immediateMode());
  c_capture -> setCurrentIndex (properties() -> alsaCapture() ? 0 : 1);
  c_device -> setText (properties() -> captureDevice());
  KPlayerPropertiesDVBDeviceAudio::load();
}

void KPlayerPropertiesTVDeviceAudio::save (void)
{
  properties() -> setAudioModeOption (c_mode -> currentIndex());
  properties() -> setImmediateMode (c_immediate -> isChecked());
  properties() -> setAlsaCapture (c_capture -> currentIndex() == 0);
  properties() -> setCaptureDevice (c_device -> text());
  KPlayerPropertiesDVBDeviceAudio::save();
}

KPlayerPropertiesDVBDeviceAudio::KPlayerPropertiesDVBDeviceAudio (QWidget* parent)
  : KPlayerPropertiesDeviceAudio (parent)
{
}

void KPlayerPropertiesDVBDeviceAudio::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDVBDeviceAudio::setupMedia\n";
#endif
  m_properties = KPlayerMedia::dvbProperties (url);
}

void KPlayerPropertiesDVBDeviceAudio::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDVBDeviceAudio::setupControls\n";
#endif
  hideTrack();
  hideRates();
  hideTV();
}

void KPlayerPropertiesDVBDeviceAudio::load (void)
{
  c_input_set -> setCurrentIndex (properties() -> hasAudioInput() ? 1 : 0);
  inputChanged (c_input_set -> currentIndex());
  KPlayerPropertiesDeviceAudio::load();
}

void KPlayerPropertiesDVBDeviceAudio::save (void)
{
  if ( c_input_set -> currentIndex() )
    properties() -> setAudioInput (labs (c_input -> text().toInt()));
  else
    properties() -> resetAudioInput();
  KPlayerPropertiesDeviceAudio::save();
}

void KPlayerPropertiesDVBDeviceAudio::inputChanged (int option)
{
  bool enable = option > 0;
  c_input -> setText (! enable ? "" : properties() -> hasAudioInput() ? properties() -> audioInputString() : "0");
  c_input -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_input -> setFocus();
    c_input -> selectAll();
  }
}

KPlayerPropertiesTrackAudio::KPlayerPropertiesTrackAudio (QWidget* parent)
  : KPlayerPropertiesAudio (parent)
{
}

void KPlayerPropertiesTrackAudio::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackAudio::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

void KPlayerPropertiesTrackAudio::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackAudio::setupControls\n";
#endif
  const QMap<int, QString>& ids (properties() -> audioIDs());
  if ( ids.count() > 1 )
  {
    int i = 1;
    QMap<int, QString>::ConstIterator iterator (ids.constBegin()), end (ids.constEnd());
    while ( iterator != end )
    {
      c_track_set -> addItem (languageName (iterator.key(), iterator.value()), i);
      ++ iterator;
      ++ i;
    }
  }
  hideInput();
  hideTV();
}

void KPlayerPropertiesTrackAudio::load (void)
{
  c_track_set -> setCurrentIndex (properties() -> audioIDOption());
  trackChanged (c_track_set -> currentIndex());
  c_bitrate -> setText (properties() -> audioBitrateString());
  c_samplerate -> setText (properties() -> samplerateString());
  KPlayerPropertiesAudio::load();
}

void KPlayerPropertiesTrackAudio::save (void)
{
  if ( c_track_set -> currentIndex() == c_track_set -> count() - 1 )
    properties() -> setAudioID (labs (c_track -> text().toInt()));
  else
    properties() -> setAudioIDOption (c_track_set -> currentIndex());
  KPlayerPropertiesAudio::save();
}

void KPlayerPropertiesTrackAudio::trackChanged (int option)
{
  bool enable = option == c_track_set -> count() - 1;
  c_track -> setText (enable ? properties() -> audioIDString() : "");
  c_track -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_track -> setFocus();
    c_track -> selectAll();
  }
}

KPlayerPropertiesVideo::KPlayerPropertiesVideo (QWidget* parent)
  : QFrame (parent)
{
}

KPlayerPropertiesVideo::~KPlayerPropertiesVideo()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesVideo::setup (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesVideo::setup\n";
#endif
  setupMedia (url);
  setupUi (this);
  if ( engine() -> videoCodecCount() )
  {
    c_codec -> clear();
    const QString& codec = properties() -> videoCodecValue();
    if ( codec.isEmpty() )
      c_codec -> addItem (i18n("default"));
    else
      c_codec -> addItem (i18n("%1 (%2)", i18n("default"), codec));
    c_codec -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> videoCodecCount(); i ++ )
      c_codec -> addItem (engine() -> videoCodecName (i));
  }
  setupControls();
  load();
}

void KPlayerPropertiesVideo::hideTrack (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesVideo::hideTrack\n";
#endif
  l_track -> hide();
  c_track_set -> hide();
  c_track -> hide();
}

void KPlayerPropertiesVideo::hideRates (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesVideo::hideRates\n";
#endif
  l_bitrate -> hide();
  c_bitrate -> hide();
  l_kbps -> hide();
  l_framerate -> hide();
  c_framerate -> hide();
  l_fps -> hide();
}

void KPlayerPropertiesVideo::hideInput (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesVideo::hideInput\n";
#endif
  l_input -> hide();
  c_input_set -> hide();
  c_input -> hide();
}

void KPlayerPropertiesVideo::hideTV (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesVideo::hideTV\n";
#endif
  l_format -> hide();
  c_format -> hide();
  l_norm -> hide();
  c_norm -> hide();
  c_norm_id -> hide();
}

void KPlayerPropertiesVideo::load (void)
{
  c_contrast_set -> setCurrentIndex (properties() -> contrastOption());
  contrastChanged (c_contrast_set -> currentIndex());
  c_brightness_set -> setCurrentIndex (properties() -> brightnessOption());
  brightnessChanged (c_brightness_set -> currentIndex());
  c_hue_set -> setCurrentIndex (properties() -> hueOption());
  hueChanged (c_hue_set -> currentIndex());
  c_saturation_set -> setCurrentIndex (properties() -> saturationOption());
  saturationChanged (c_saturation_set -> currentIndex());
  const QString& codec (properties() -> videoCodecOption());
  c_codec -> setCurrentIndex (codec.isNull() ? 0 : engine() -> videoCodecIndex (codec) + 2);
}

void KPlayerPropertiesVideo::save (void)
{
  properties() -> setContrastOption (c_contrast -> text().toInt(), c_contrast_set -> currentIndex());
  properties() -> setBrightnessOption (c_brightness -> text().toInt(), c_brightness_set -> currentIndex());
  properties() -> setHueOption (c_hue -> text().toInt(), c_hue_set -> currentIndex());
  properties() -> setSaturationOption (c_saturation -> text().toInt(), c_saturation_set -> currentIndex());
  properties() -> setVideoCodecOption (listEntry (c_codec, true));
}

void KPlayerPropertiesVideo::contrastChanged (int option)
{
  bool enable = option > 0;
  c_contrast -> setText (enable ? properties() -> contrastString() : "");
  c_contrast -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_contrast -> setFocus();
    c_contrast -> selectAll();
  }
}

void KPlayerPropertiesVideo::brightnessChanged (int option)
{
  bool enable = option > 0;
  c_brightness -> setText (enable ? properties() -> brightnessString() : "");
  c_brightness -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_brightness -> setFocus();
    c_brightness -> selectAll();
  }
}

void KPlayerPropertiesVideo::hueChanged (int option)
{
  bool enable = option > 0;
  c_hue -> setText (enable ? properties() -> hueString() : "");
  c_hue -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_hue -> setFocus();
    c_hue -> selectAll();
  }
}

void KPlayerPropertiesVideo::saturationChanged (int option)
{
  bool enable = option > 0;
  c_saturation -> setText (enable ? properties() -> saturationString() : "");
  c_saturation -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_saturation -> setFocus();
    c_saturation -> selectAll();
  }
}

KPlayerPropertiesDeviceVideo::KPlayerPropertiesDeviceVideo (QWidget* parent)
  : KPlayerPropertiesVideo (parent)
{
}

void KPlayerPropertiesDeviceVideo::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceVideo::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

void KPlayerPropertiesDeviceVideo::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceVideo::setupControls\n";
#endif
  hideTrack();
  hideRates();
  hideInput();
  hideTV();
}

KPlayerPropertiesTVDeviceVideo::KPlayerPropertiesTVDeviceVideo (QWidget* parent)
  : KPlayerPropertiesDVBDeviceVideo (parent)
{
}

void KPlayerPropertiesTVDeviceVideo::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceVideo::setupMedia\n";
#endif
  m_properties = KPlayerMedia::tvProperties (url);
}

void KPlayerPropertiesTVDeviceVideo::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceVideo::setupControls\n";
#endif
  hideTrack();
  hideRates();
}

void KPlayerPropertiesTVDeviceVideo::load (void)
{
  const QString& format (properties() -> videoFormat());
  int i;
  for ( i = 1; i < c_format -> count(); i ++ )
    if ( c_format -> itemText (i) == format )
    {
      c_format -> setCurrentIndex (i);
      break;
    }
  if ( i == c_format -> count() )
    c_format -> setCurrentIndex (0);
  int norm = properties() -> videoNorm();
  c_norm -> setCurrentIndex (norm >= 0 ? c_norm -> count() - 1 : - norm - 1);
  normChanged (c_norm -> currentIndex());
  KPlayerPropertiesDVBDeviceVideo::load();
}

void KPlayerPropertiesTVDeviceVideo::save (void)
{
  properties() -> setVideoFormat (c_format -> currentIndex() ? c_format -> currentText() : "");
  properties() -> setVideoNorm (c_norm -> currentIndex() == c_norm -> count() - 1 ?
    labs (c_norm_id -> text().toInt()) : - c_norm -> currentIndex() - 1);
  KPlayerPropertiesDVBDeviceVideo::save();
}

void KPlayerPropertiesTVDeviceVideo::normChanged (int option)
{
  bool enable = option == c_norm -> count() - 1;
  c_norm_id -> setText (! enable ? "" : properties() -> videoNorm() >= 0 ? properties() -> videoNormString() : "0");
  c_norm_id -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_norm_id -> setFocus();
    c_norm_id -> selectAll();
  }
}

KPlayerPropertiesDVBDeviceVideo::KPlayerPropertiesDVBDeviceVideo (QWidget* parent)
  : KPlayerPropertiesDeviceVideo (parent)
{
}

void KPlayerPropertiesDVBDeviceVideo::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDVBDeviceVideo::setupMedia\n";
#endif
  m_properties = KPlayerMedia::dvbProperties (url);
}

void KPlayerPropertiesDVBDeviceVideo::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDVBDeviceVideo::setupControls\n";
#endif
  hideTrack();
  hideRates();
  hideTV();
}

void KPlayerPropertiesDVBDeviceVideo::load (void)
{
  c_input_set -> setCurrentIndex (properties() -> hasVideoInput() ? 1 : 0);
  inputChanged (c_input_set -> currentIndex());
  KPlayerPropertiesDeviceVideo::load();
}

void KPlayerPropertiesDVBDeviceVideo::save (void)
{
  if ( c_input_set -> currentIndex() )
    properties() -> setVideoInput (labs (c_input -> text().toInt()));
  else
    properties() -> resetVideoInput();
  KPlayerPropertiesDeviceVideo::save();
}

void KPlayerPropertiesDVBDeviceVideo::inputChanged (int option)
{
  bool enable = option > 0;
  c_input -> setText (! enable ? "" : properties() -> hasVideoInput() ? properties() -> videoInputString() : "0");
  c_input -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_input -> setFocus();
    c_input -> selectAll();
  }
}

KPlayerPropertiesTrackVideo::KPlayerPropertiesTrackVideo (QWidget* parent)
  : KPlayerPropertiesVideo (parent)
{
}

void KPlayerPropertiesTrackVideo::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackVideo::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

void KPlayerPropertiesTrackVideo::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackVideo::setupControls\n";
#endif
  const QMap<int, QString>& ids (properties() -> videoIDs());
  if ( ids.count() > 1 )
  {
    int i = 1;
    QMap<int, QString>::ConstIterator iterator (ids.constBegin()), end (ids.constEnd());
    while ( iterator != end )
    {
      c_track_set -> addItem (languageName (iterator.key(), iterator.value()), i);
      ++ iterator;
      ++ i;
    }
  }
  hideInput();
  hideTV();
}

void KPlayerPropertiesTrackVideo::load (void)
{
  c_track_set -> setCurrentIndex (properties() -> videoIDOption());
  trackChanged (c_track_set -> currentIndex());
  c_bitrate -> setText (properties() -> videoBitrateString());
  c_framerate -> setText (properties() -> framerateString());
  KPlayerPropertiesVideo::load();
}

void KPlayerPropertiesTrackVideo::save (void)
{
  if ( c_track_set -> currentIndex() == c_track_set -> count() - 1 )
    properties() -> setVideoID (labs (c_track -> text().toInt()));
  else
    properties() -> setVideoIDOption (c_track_set -> currentIndex());
  KPlayerPropertiesVideo::save();
}

void KPlayerPropertiesTrackVideo::trackChanged (int option)
{
  bool enable = option == c_track_set -> count() - 1;
  c_track -> setText (enable ? properties() -> videoIDString() : "");
  c_track -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_track -> setFocus();
    c_track -> selectAll();
  }
}

KPlayerPropertiesAdvanced::KPlayerPropertiesAdvanced (QWidget* parent)
  : QFrame (parent)
{
}

KPlayerPropertiesAdvanced::~KPlayerPropertiesAdvanced()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesAdvanced::setup (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAdvanced::setup\n";
#endif
  setupMedia (url);
  setupUi (this);
  if ( engine() -> demuxerCount() )
  {
    c_demuxer -> clear();
    const QString& demuxer = properties() -> demuxerValue();
    if ( demuxer.isEmpty() )
      c_demuxer -> addItem (i18n("default"));
    else
      c_demuxer -> addItem (i18n("%1 (%2)", i18n("default"), demuxer));
    c_demuxer -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> demuxerCount(); i ++ )
      c_demuxer -> addItem (engine() -> demuxerName (i));
  }
  //setupHistory (c_command_line, properties() -> commandLineHistory());
  setupControls();
  load();
}

void KPlayerPropertiesAdvanced::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAdvanced::setupControls\n";
#endif
  hideKioslave();
  hideCompression();
}

void KPlayerPropertiesAdvanced::hideKioslave (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAdvanced::hideKioslave\n";
#endif
  l_use_kioslave -> hide();
  c_use_kioslave -> hide();
  l_use_temporary_file -> hide();
  c_use_temporary_file -> hide();
}

void KPlayerPropertiesAdvanced::hideCompression (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAdvanced::hideCompression\n";
#endif
  c_compression -> hide();
  c_decimation -> hide();
  l_quality -> hide();
  c_quality -> hide();
  l_spacer -> hide();
}

void KPlayerPropertiesAdvanced::load (void)
{
  c_command_line_option -> setCurrentIndex (properties() -> commandLineOption());
  commandLineChanged (c_command_line_option -> currentIndex());
  const QString& demuxer (properties() -> demuxerOption());
  c_demuxer -> setCurrentIndex (demuxer.isNull() ? 0 : engine() -> demuxerIndex (demuxer) + 2);
  c_frame_drop -> setCurrentIndex (properties() -> frameDropOption());
  c_use_cache -> setCurrentIndex (properties() -> cacheOption());
  cacheChanged (c_use_cache -> currentIndex());
  c_build_index -> setCurrentIndex (properties() -> buildNewIndexOption());
}

void KPlayerPropertiesAdvanced::save (void)
{
  properties() -> setCommandLineOption (c_command_line -> text(), c_command_line_option -> currentIndex());
  properties() -> setDemuxerOption (listEntry (c_demuxer, true));
  properties() -> setFrameDropOption (c_frame_drop -> currentIndex());
  properties() -> setCacheOption (c_use_cache -> currentIndex(), labs (c_cache_size -> text().toInt()));
  properties() -> setBuildNewIndexOption (c_build_index -> currentIndex());
}

void KPlayerPropertiesAdvanced::commandLineChanged (int option)
{
  bool enable = option > 0;
  c_command_line -> setText (! enable ? ""
    : option == 2 ? properties() -> commandLineValue() : properties() -> commandLine());
  c_command_line -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_command_line -> setFocus();
    c_command_line -> selectAll();
  }
}

void KPlayerPropertiesAdvanced::cacheChanged (int cache)
{
  bool enable = cache == 3;
  c_cache_size -> setText (enable ? properties() -> cacheSizeString() : "");
  c_cache_size -> setEnabled (enable);
  l_cache_size_kb -> setEnabled (enable);
  if ( enable && sender() )
  {
    c_cache_size -> setFocus();
    c_cache_size -> selectAll();
  }
}

KPlayerPropertiesDeviceAdvanced::KPlayerPropertiesDeviceAdvanced (QWidget* parent)
  : KPlayerPropertiesAdvanced (parent)
{
}

void KPlayerPropertiesDeviceAdvanced::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceAdvanced::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

KPlayerPropertiesTVDeviceAdvanced::KPlayerPropertiesTVDeviceAdvanced (QWidget* parent)
  : KPlayerPropertiesDeviceAdvanced (parent)
{
}

void KPlayerPropertiesTVDeviceAdvanced::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceAdvanced::setupMedia\n";
#endif
  m_properties = KPlayerMedia::tvProperties (url);
}

void KPlayerPropertiesTVDeviceAdvanced::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTVDeviceAdvanced::setupControls\n";
#endif
  hideKioslave();
}

void KPlayerPropertiesTVDeviceAdvanced::load (void)
{
  c_compression -> setChecked (properties() -> hasMjpegDecimation());
  compressionChanged (c_compression -> isChecked());
  int decimation = properties() -> mjpegDecimation();
  c_decimation -> setCurrentIndex (decimation == 0 ? 1 : decimation == 4 ? 2 : decimation - 1);
  KPlayerPropertiesDeviceAdvanced::load();
}

void KPlayerPropertiesTVDeviceAdvanced::save (void)
{
  properties() -> setMjpegDecimation (! c_compression -> isChecked() ? 0
    : c_decimation -> currentIndex() == 2 ? 4 : c_decimation -> currentIndex() + 1);
  if ( c_compression -> isChecked() )
    properties() -> setMjpegQuality (labs (c_quality -> text().toInt()));
  KPlayerPropertiesDeviceAdvanced::save();
}

void KPlayerPropertiesTVDeviceAdvanced::compressionChanged (bool checked)
{
  c_quality -> setText (! checked ? "" : properties() -> hasMjpegQuality() ? properties() -> mjpegQualityString() : "90");
  c_decimation -> setEnabled (checked);
  l_quality -> setEnabled (checked);
  c_quality -> setEnabled (checked);
  if ( checked && sender() )
    c_decimation -> setFocus();
}

KPlayerPropertiesTrackAdvanced::KPlayerPropertiesTrackAdvanced (QWidget* parent)
  : KPlayerPropertiesAdvanced (parent)
{
}

void KPlayerPropertiesTrackAdvanced::setupMedia (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackAdvanced::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

KPlayerPropertiesItemAdvanced::KPlayerPropertiesItemAdvanced (QWidget* parent)
  : KPlayerPropertiesTrackAdvanced (parent)
{
}

void KPlayerPropertiesItemAdvanced::setupControls (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesItemAdvanced::setupControls\n";
#endif
  hideCompression();
}

void KPlayerPropertiesItemAdvanced::load (void)
{
  c_use_kioslave -> setCurrentIndex (properties() -> useKioslaveOption());
  c_use_temporary_file -> setCurrentIndex (properties() -> useTemporaryFileOption());
  KPlayerPropertiesTrackAdvanced::load();
}

void KPlayerPropertiesItemAdvanced::save (void)
{
  properties() -> setUseKioslaveOption (c_use_kioslave -> currentIndex());
  properties() -> setUseTemporaryFileOption (c_use_temporary_file -> currentIndex());
  KPlayerPropertiesTrackAdvanced::save();
}
