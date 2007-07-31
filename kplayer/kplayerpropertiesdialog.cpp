/***************************************************************************
                         kplayerpropertiesdialog.cpp
                         ---------------------------
    begin                : Tue Mar 02 2004
    copyright            : (C) 2004-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <stdlib.h>

QString languageName (int id, QString language);

#ifdef DEBUG
#define DEBUG_KPLAYER_PROPERTIES_DIALOG
#endif

#include "kplayerpropertiesdialog.h"
#include "kplayerpropertiesdialog.moc"
#include "kplayerproperties.h"

static QString s_default_entry (i18n("%1 (%2)"));
static QRegExp re_key_value ("^([^:]+): *(.*)$");

QString listEntry (QComboBox* combo, bool hasDefault = false)
{
  if ( hasDefault && combo -> currentItem() == 0 )
    return QString::null;
  if ( combo -> currentItem() == 0 || hasDefault && combo -> currentItem() == 1 )
    return "";
  if ( re_key_value.search (combo -> currentText()) >= 0 )
    return re_key_value.cap(1);
  return QString::null;
}

KPlayerPropertiesDialog::KPlayerPropertiesDialog (void)
  : KDialogBase (TreeList, i18n("File Properties"), Help | Default | Ok | Apply | Cancel, Ok)
{
  QApplication::connect (this, SIGNAL (aboutToShowPage (QWidget*)), this, SLOT (pageAboutToShow(QWidget*)));
}

KPlayerPropertiesDialog::~KPlayerPropertiesDialog (void)
{
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Dialog Options");
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPFP " << x() << "x" << y() << " " << width() << "x" << height() << " Hint " << sizeHint().width() << "x" << sizeHint().height() << "\n";
#endif
/*if ( size() == sizeHint() )
  {
    config -> deleteEntry ("Properties Dialog Width");
    config -> deleteEntry ("Properties Dialog Height");
  }
  else
  {*/
/*config -> writeEntry ("Properties Dialog Left", x());
  config -> writeEntry ("Properties Dialog Top", y());*/
  config -> writeEntry ("Properties Dialog Width", width());
  config -> writeEntry ("Properties Dialog Height", height());
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesDialog::setup (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDialog::setup\n";
#endif
  setupMedia (url);
  QFrame* frame = addPage (i18n("General"), i18n("General Properties"));
  QBoxLayout* layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_general = createGeneralPage (frame, "general");
  m_general -> setup (url);
  frame = addPage (i18n("Size"), i18n("Size Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_size = createSizePage (frame, "size");
  m_size -> setup (url);
  frame = addPage (i18n("Video"), i18n("Video Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_video = createVideoPage (frame, "video");
  m_video -> setup (url);
  frame = addPage (i18n("Audio"), i18n("Audio Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_audio = createAudioPage (frame, "audio");
  m_audio -> setup (url);
  frame = addPage (i18n("Subtitles"), i18n("Subtitle Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_subtitles = createSubtitlesPage (frame, "subtitles");
  m_subtitles -> setup (url);
  frame = addPage (i18n("Advanced"), i18n("Advanced Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_advanced = createAdvancedPage (frame, "advanced");
  m_advanced -> setup (url);
  setHelp ("properties");
  KListView* view = (KListView*) child (0, "KListView");
  if ( view )
    view -> setMinimumSize (view -> sizeHint());
  layout = (QBoxLayout*) child (0, "QHBoxLayout");
  if ( layout )
    layout -> insertSpacing (0, 6);
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Dialog Options");
  QString name (config -> readEntry ("Properties Dialog Page"));
  if ( ! name.isEmpty() )
  {
    QFrame* frame = (QFrame*) child (name.latin1());
    if ( frame )
    {
      frame = (QFrame*) frame -> parent();
      if ( frame )
        showPage (pageIndex (frame));
    }
  }
/*int x = config -> readNumEntry ("Properties Dialog Left", -1);
  int y = config -> readNumEntry ("Properties Dialog Top", -1);*/
  int w = config -> readNumEntry ("Properties Dialog Width");
  int h = config -> readNumEntry ("Properties Dialog Height");
//QSize size (config -> readNumEntry ("Properties Dialog Width"),
//  config -> readNumEntry ("Properties Dialog Height"));
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

void KPlayerPropertiesDialog::slotDefault (void)
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
  setButtonCancel (KStdGuiItem::close());
  KDialogBase::slotDefault();
}

void KPlayerPropertiesDialog::pageAboutToShow (QWidget* page)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDialog::pageAboutToShow\n";
#endif
  QObject* object = page -> child (0, "QFrame");
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Dialog Options");
  QString name;
  if ( object )
  {
    name = object -> name ("");
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
    kdDebugTime() << " Page   " << name << "\n";
#endif
  }
  if ( name.isEmpty() )
    config -> deleteEntry ("Properties Dialog Page");
  else
    config -> writeEntry ("Properties Dialog Page", name);
  setHelp (name.isEmpty() ? "properties" : "properties-" + name);
}

void KPlayerPropertiesDialog::slotOk (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDialog::OK\n";
#endif
  slotApply();
  KDialogBase::slotOk();
}

void KPlayerPropertiesDialog::slotApply (void)
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
  setButtonCancel (KStdGuiItem::close());
  KDialogBase::slotApply();
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

void KPlayerDevicePropertiesDialog::setupMedia (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerDevicePropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

KPlayerPropertiesGeneral* KPlayerDevicePropertiesDialog::createGeneralPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDeviceGeneral (frame, name.utf8());
}

KPlayerPropertiesSize* KPlayerDevicePropertiesDialog::createSizePage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDeviceSize (frame, name.utf8());
}

KPlayerPropertiesSubtitles* KPlayerDevicePropertiesDialog::createSubtitlesPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDeviceSubtitles (frame, name.utf8());
}

KPlayerPropertiesAudio* KPlayerDevicePropertiesDialog::createAudioPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDeviceAudio (frame, name.utf8());
}

KPlayerPropertiesVideo* KPlayerDevicePropertiesDialog::createVideoPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDeviceVideo (frame, name.utf8());
}

KPlayerPropertiesAdvanced* KPlayerDevicePropertiesDialog::createAdvancedPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDeviceAdvanced (frame, name.utf8());
}

KPlayerTVDevicePropertiesDialog::~KPlayerTVDevicePropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying TV device properties dialog\n";
#endif
}

void KPlayerTVDevicePropertiesDialog::setupMedia (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerTVDevicePropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::tvProperties (url);
}

KPlayerPropertiesGeneral* KPlayerTVDevicePropertiesDialog::createGeneralPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTVDeviceGeneral (frame, name.utf8());
}

KPlayerPropertiesAudio* KPlayerTVDevicePropertiesDialog::createAudioPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTVDeviceAudio (frame, name.utf8());
}

KPlayerPropertiesVideo* KPlayerTVDevicePropertiesDialog::createVideoPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTVDeviceVideo (frame, name.utf8());
}

KPlayerPropertiesAdvanced* KPlayerTVDevicePropertiesDialog::createAdvancedPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTVDeviceAdvanced (frame, name.utf8());
}

KPlayerDVBDevicePropertiesDialog::~KPlayerDVBDevicePropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying DVB device properties dialog\n";
#endif
}

void KPlayerDVBDevicePropertiesDialog::setupMedia (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerDVBDevicePropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::dvbProperties (url);
}

KPlayerPropertiesGeneral* KPlayerDVBDevicePropertiesDialog::createGeneralPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDVBDeviceGeneral (frame, name.utf8());
}

KPlayerPropertiesAudio* KPlayerDVBDevicePropertiesDialog::createAudioPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDVBDeviceAudio (frame, name.utf8());
}

KPlayerPropertiesVideo* KPlayerDVBDevicePropertiesDialog::createVideoPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDVBDeviceVideo (frame, name.utf8());
}

KPlayerDiskTrackPropertiesDialog::~KPlayerDiskTrackPropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying track properties dialog\n";
#endif
}

void KPlayerDiskTrackPropertiesDialog::setupMedia (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerDiskTrackPropertiesDialog::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

KPlayerPropertiesGeneral* KPlayerDiskTrackPropertiesDialog::createGeneralPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDiskTrackGeneral (frame, name.utf8());
}

KPlayerPropertiesSize* KPlayerDiskTrackPropertiesDialog::createSizePage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTrackSize (frame, name.utf8());
}

KPlayerPropertiesSubtitles* KPlayerDiskTrackPropertiesDialog::createSubtitlesPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesDiskTrackSubtitles (frame, name.utf8());
}

KPlayerPropertiesAudio* KPlayerDiskTrackPropertiesDialog::createAudioPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTrackAudio (frame, name.utf8());
}

KPlayerPropertiesVideo* KPlayerDiskTrackPropertiesDialog::createVideoPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTrackVideo (frame, name.utf8());
}

KPlayerPropertiesAdvanced* KPlayerDiskTrackPropertiesDialog::createAdvancedPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesTrackAdvanced (frame, name.utf8());
}

KPlayerChannelPropertiesDialog::~KPlayerChannelPropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying channel properties dialog\n";
#endif
}

KPlayerPropertiesGeneral* KPlayerChannelPropertiesDialog::createGeneralPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesChannelGeneral (frame, name.utf8());
}

KPlayerPropertiesSubtitles* KPlayerChannelPropertiesDialog::createSubtitlesPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesChannelSubtitles (frame, name.utf8());
}

KPlayerItemPropertiesDialog::~KPlayerItemPropertiesDialog (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "Destroying item properties dialog\n";
#endif
}

KPlayerPropertiesGeneral* KPlayerItemPropertiesDialog::createGeneralPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesItemGeneral (frame, name.utf8());
}

KPlayerPropertiesSubtitles* KPlayerItemPropertiesDialog::createSubtitlesPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesItemSubtitles (frame, name.utf8());
}

KPlayerPropertiesAdvanced* KPlayerItemPropertiesDialog::createAdvancedPage (QFrame* frame, const QString& name)
{
  return new KPlayerPropertiesItemAdvanced (frame, name.utf8());
}

/*void setupHistory (QComboBox* combo, const QStringList& history)
{
  for ( QStringList::ConstIterator it (history.begin()); it != history.end(); ++ it )
  {
    combo -> insertItem (*it);
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
    kdDebugTime() << " History " << *it << "\n";
#endif
  }
}*/

KPlayerPropertiesGeneral::KPlayerPropertiesGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesGeneralPage (parent, name)
{
}

KPlayerPropertiesGeneral::~KPlayerPropertiesGeneral()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesGeneral::setup (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesGeneral::setup\n";
#endif
  setupMedia (url);
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

KPlayerPropertiesDeviceGeneral::KPlayerPropertiesDeviceGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesGeneral (parent, name)
{
}

void KPlayerPropertiesDeviceGeneral::setupMedia (const KURL& url)
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

KPlayerPropertiesTVDeviceGeneral::KPlayerPropertiesTVDeviceGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesDeviceGeneral (parent, name)
{
}

void KPlayerPropertiesTVDeviceGeneral::setupMedia (const KURL& url)
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
    c_channels -> insertItem (i18n(channellists[i].name));
}

void KPlayerPropertiesTVDeviceGeneral::load (void)
{
  const QString& list (properties() -> channelList());
  for ( uint i = 0; c_channels -> count(); i ++ )
    if ( channellists[i].id == list )
    {
      c_channels -> setCurrentItem (i);
      break;
    }
  const QString& driver (properties() -> inputDriver());
  c_driver -> setCurrentItem (driver == "bsdbt848" ? 0 : driver == "v4l" ? 1 : 2);
  KPlayerPropertiesDeviceGeneral::load();
}

void KPlayerPropertiesTVDeviceGeneral::save (void)
{
  properties() -> setChannelList (channellists[c_channels -> currentItem()].id);
  int driver = c_driver -> currentItem();
  properties() -> setInputDriver (driver == 0 ? "bsdbt848" : driver == 1 ? "v4l" : "v4l2");
  KPlayerPropertiesDeviceGeneral::save();
}

KPlayerPropertiesDVBDeviceGeneral::KPlayerPropertiesDVBDeviceGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesDeviceGeneral (parent, name)
{
}

void KPlayerPropertiesDVBDeviceGeneral::setupMedia (const KURL& url)
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

KPlayerPropertiesTrackGeneral::KPlayerPropertiesTrackGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesGeneral (parent, name)
{
}

void KPlayerPropertiesTrackGeneral::setupMedia (const KURL& url)
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

KPlayerPropertiesDiskTrackGeneral::KPlayerPropertiesDiskTrackGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesTrackGeneral (parent, name)
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

KPlayerPropertiesChannelGeneral::KPlayerPropertiesChannelGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesDiskTrackGeneral (parent, name)
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

KPlayerPropertiesItemGeneral::KPlayerPropertiesItemGeneral (QWidget* parent, const char* name)
  : KPlayerPropertiesTrackGeneral (parent, name)
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
  c_playlist -> setCurrentItem (properties() -> playlistOption());
  KPlayerPropertiesTrackGeneral::load();
}

void KPlayerPropertiesItemGeneral::save (void)
{
  properties() -> setPlaylistOption (c_playlist -> currentItem());
  KPlayerPropertiesTrackGeneral::save();
}

KPlayerPropertiesSize::KPlayerPropertiesSize (QWidget* parent, const char* name)
  : KPlayerPropertiesSizePage (parent, name)
{
}

KPlayerPropertiesSize::~KPlayerPropertiesSize()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesSize::setup (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSize::setup\n";
#endif
  setupMedia (url);
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
  c_display_size -> setCurrentItem (properties() -> displaySizeOption());
  displaySizeChanged (c_display_size -> currentItem());
  c_full_screen -> setCurrentItem (properties() -> fullScreenOption());
  c_maximized -> setCurrentItem (properties() -> maximizedOption());
  c_maintain_aspect -> setCurrentItem (properties() -> maintainAspectOption());
}

void KPlayerPropertiesSize::save (void)
{
  int width = labs (c_display_width -> text().toInt());
  int height = labs (c_display_height -> text().toInt());
  if ( width == 0 && c_display_size -> currentItem() == 2 && c_display_width -> text().stripWhiteSpace().toDouble() > 0 )
  {
    QRegExp re ("^\\s*(\\d*)[,.](\\d*)\\s*$");
    if ( re.search (c_display_width -> text()) >= 0 )
    {
      width = (re.cap(1) + re.cap(2)).toInt();
      for ( uint i = 0; i < re.cap(2).length(); i ++ )
        height *= 10;
    }
  }
  if ( c_display_size -> currentItem() == 2 )
    for ( int i = 2; i <= height; i ++ )
      if ( width / i * i == width && height / i * i == height )
      {
        width /= i;
        height /= i --;
      }
  properties() -> setDisplaySize (QSize (width, height), c_display_size -> currentItem());
  properties() -> setFullScreenOption (c_full_screen -> currentItem());
  properties() -> setMaximizedOption (c_maximized -> currentItem());
  properties() -> setMaintainAspectOption (c_maintain_aspect -> currentItem());
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

KPlayerPropertiesDeviceSize::KPlayerPropertiesDeviceSize (QWidget* parent, const char* name)
  : KPlayerPropertiesSize (parent, name)
{
}

void KPlayerPropertiesDeviceSize::setupMedia (const KURL& url)
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

KPlayerPropertiesTrackSize::KPlayerPropertiesTrackSize (QWidget* parent, const char* name)
  : KPlayerPropertiesSize (parent, name)
{
}

void KPlayerPropertiesTrackSize::setupMedia (const KURL& url)
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

KPlayerPropertiesSubtitles::KPlayerPropertiesSubtitles (QWidget* parent, const char* name)
  : KPlayerPropertiesSubtitlesPage (parent, name)
{
}

KPlayerPropertiesSubtitles::~KPlayerPropertiesSubtitles()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesSubtitles::setup (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesSubtitles::setup\n";
#endif
  setupMedia (url);
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
  c_position_set -> setCurrentItem (properties() -> hasSubtitlePosition() ? 1 : 0);
  positionChanged (c_position_set -> currentItem());
  c_delay_set -> setCurrentItem (properties() -> hasSubtitleDelay() ? 1 : 0);
  delayChanged (c_delay_set -> currentItem());
  c_closed_caption -> setCurrentItem (properties() -> subtitleClosedCaptionOption());
}

void KPlayerPropertiesSubtitles::save (void)
{
  if ( c_position_set -> currentItem() )
    properties() -> setSubtitlePositionValue (labs (c_position -> text().toInt()));
  else
    properties() -> resetSubtitlePosition();
  if ( c_delay_set -> currentItem() )
    properties() -> setSubtitleDelayValue (c_delay -> text().toFloat());
  else
    properties() -> resetSubtitleDelay();
  properties() -> setSubtitleClosedCaptionOption (c_closed_caption -> currentItem());
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

KPlayerPropertiesDeviceSubtitles::KPlayerPropertiesDeviceSubtitles (QWidget* parent, const char* name)
  : KPlayerPropertiesSubtitles (parent, name)
{
}

void KPlayerPropertiesDeviceSubtitles::setupMedia (const KURL& url)
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

KPlayerPropertiesTrackSubtitles::KPlayerPropertiesTrackSubtitles (QWidget* parent, const char* name)
  : KPlayerPropertiesSubtitles (parent, name)
{
}

void KPlayerPropertiesTrackSubtitles::setupMedia (const KURL& url)
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
    c_track_set -> insertItem (languageName (iterator.key(), iterator.data()), i);
    ++ iterator;
    ++ i;
  }
  const QMap<int, QString>& vsids (properties() -> vobsubIDs());
  iterator = vsids.constBegin();
  end = vsids.constEnd();
  while ( iterator != end )
  {
    c_track_set -> insertItem (languageName (iterator.key(), iterator.data()), i);
    ++ iterator;
    ++ i;
  }
}

void KPlayerPropertiesTrackSubtitles::load (void)
{
  int option = properties() -> subtitleOption();
  if ( option == c_track_set -> count() - 1 )
    option = 0;
  c_track_set -> setCurrentItem (option);
  trackChanged (c_track_set -> currentItem());
  KPlayerPropertiesSubtitles::load();
}

void KPlayerPropertiesTrackSubtitles::save (void)
{
  if ( c_track_set -> currentItem() == c_track_set -> count() - 1 )
    properties() -> setSubtitleID (labs (c_track -> text().toInt()));
  else
    properties() -> setSubtitleOption (c_track_set -> currentItem());
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

KPlayerPropertiesChannelSubtitles::KPlayerPropertiesChannelSubtitles (QWidget* parent, const char* name)
  : KPlayerPropertiesTrackSubtitles (parent, name)
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

KPlayerPropertiesDiskTrackSubtitles::KPlayerPropertiesDiskTrackSubtitles (QWidget* parent, const char* name)
  : KPlayerPropertiesTrackSubtitles (parent, name)
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
    combobox -> insertItem (QString (encoding.id) + ": " + i18n(encoding.name));
  }
}

void KPlayerPropertiesDiskTrackSubtitles::setupEncoding (void)
{
  fillEncodingCombobox (c_encoding);
  for ( uint i = 0; i < sizeof (framerates) / sizeof (float); ++ i )
    c_framerate -> insertItem (QString::number (framerates[i]));
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
  c_vobsub -> setCurrentItem (properties() -> vobsubSubtitlesOption());
  if ( properties() -> hasSubtitleEncoding() )
  {
    QString encoding = properties() -> subtitleEncoding();
    if ( encoding.isEmpty() )
      c_encoding -> setCurrentItem (1);
    else
    {
      c_encoding -> setEditText (encoding);
      encoding += ": ";
      for ( int i = 2; i < c_encoding -> count(); ++ i )
        if ( c_encoding -> text (i).startsWith (encoding) )
        {
          c_encoding -> setCurrentItem (i);
          break;
        }
    }
  }
  else
    c_encoding -> setCurrentItem (0);
  if ( properties() -> hasSubtitleFramerate() )
    c_framerate -> setEditText (properties() -> subtitleFramerateString());
  else
    c_framerate -> setCurrentItem (0);
  KPlayerPropertiesTrackSubtitles::load();
}

void KPlayerPropertiesDiskTrackSubtitles::save (void)
{
  properties() -> setSubtitleUrl (c_url -> text());
  properties() -> setVobsubSubtitlesOption (c_vobsub -> currentItem());
  if ( c_encoding -> currentItem() == 0 )
    properties() -> resetSubtitleEncoding();
  else if ( c_encoding -> currentItem() == 1 )
    properties() -> setSubtitleEncoding ("");
  else
    properties() -> setSubtitleEncoding (c_encoding -> currentText().section (':', 0, 0));
  if ( c_framerate -> currentItem() )
    properties() -> setSubtitleFramerate (c_framerate -> currentText().toFloat());
  else
    properties() -> resetSubtitleFramerate();
  KPlayerPropertiesTrackSubtitles::save();
}

KPlayerPropertiesItemSubtitles::KPlayerPropertiesItemSubtitles (QWidget* parent, const char* name)
  : KPlayerPropertiesDiskTrackSubtitles (parent, name)
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
    c_autoload -> setCurrentItem (properties() -> subtitleAutoloadOption());
  KPlayerPropertiesDiskTrackSubtitles::load();
}

void KPlayerPropertiesItemSubtitles::save (void)
{
  if ( properties() -> url().isLocalFile() )
    properties() -> setSubtitleAutoloadOption (c_autoload -> currentItem());
  KPlayerPropertiesDiskTrackSubtitles::save();
}

KPlayerPropertiesAudio::KPlayerPropertiesAudio (QWidget* parent, const char* name)
  : KPlayerPropertiesAudioPage (parent, name)
{
}

KPlayerPropertiesAudio::~KPlayerPropertiesAudio()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesAudio::setup (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAudio::setup\n";
#endif
  setupMedia (url);
  if ( engine() -> audioCodecCount() )
  {
    c_codec -> clear();
    const QString& codec = properties() -> audioCodecValue();
    if ( codec.isEmpty() )
      c_codec -> insertItem (i18n("default"));
    else
      c_codec -> insertItem (s_default_entry.arg (i18n("default")).arg (codec));
    c_codec -> insertItem (i18n("auto"));
    for ( int i = 0; i < engine() -> audioCodecCount(); i ++ )
      c_codec -> insertItem (engine() -> audioCodecName (i));
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
  c_volume_set -> setCurrentItem (properties() -> volumeOption());
  volumeChanged (c_volume_set -> currentItem());
  c_delay_set -> setCurrentItem (properties() -> hasAudioDelay() ? 1 : 0);
  delayChanged (c_delay_set -> currentItem());
  const QString& codec (properties() -> audioCodecOption());
  c_codec -> setCurrentItem (codec.isNull() ? 0 : engine() -> audioCodecIndex (codec) + 2);
}

void KPlayerPropertiesAudio::save (void)
{
  properties() -> setVolumeOption (labs (c_volume -> text().toInt()), c_volume_set -> currentItem());
  if ( c_delay_set -> currentItem() )
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

KPlayerPropertiesDeviceAudio::KPlayerPropertiesDeviceAudio (QWidget* parent, const char* name)
  : KPlayerPropertiesAudio (parent, name)
{
}

void KPlayerPropertiesDeviceAudio::setupMedia (const KURL& url)
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

KPlayerPropertiesTVDeviceAudio::KPlayerPropertiesTVDeviceAudio (QWidget* parent, const char* name)
  : KPlayerPropertiesDVBDeviceAudio (parent, name)
{
}

void KPlayerPropertiesTVDeviceAudio::setupMedia (const KURL& url)
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
  c_mode -> setCurrentItem (properties() -> audioModeOption());
  c_immediate -> setChecked (properties() -> immediateMode());
  c_capture -> setCurrentItem (properties() -> alsaCapture() ? 0 : 1);
  c_device -> setText (properties() -> captureDevice());
  KPlayerPropertiesDVBDeviceAudio::load();
}

void KPlayerPropertiesTVDeviceAudio::save (void)
{
  properties() -> setAudioModeOption (c_mode -> currentItem());
  properties() -> setImmediateMode (c_immediate -> isChecked());
  properties() -> setAlsaCapture (c_capture -> currentItem() == 0);
  properties() -> setCaptureDevice (c_device -> text());
  KPlayerPropertiesDVBDeviceAudio::save();
}

KPlayerPropertiesDVBDeviceAudio::KPlayerPropertiesDVBDeviceAudio (QWidget* parent, const char* name)
  : KPlayerPropertiesDeviceAudio (parent, name)
{
}

void KPlayerPropertiesDVBDeviceAudio::setupMedia (const KURL& url)
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
  c_input_set -> setCurrentItem (properties() -> hasAudioInput() ? 1 : 0);
  inputChanged (c_input_set -> currentItem());
  KPlayerPropertiesDeviceAudio::load();
}

void KPlayerPropertiesDVBDeviceAudio::save (void)
{
  if ( c_input_set -> currentItem() )
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

KPlayerPropertiesTrackAudio::KPlayerPropertiesTrackAudio (QWidget* parent, const char* name)
  : KPlayerPropertiesAudio (parent, name)
{
}

void KPlayerPropertiesTrackAudio::setupMedia (const KURL& url)
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
      c_track_set -> insertItem (languageName (iterator.key(), iterator.data()), i);
      ++ iterator;
      ++ i;
    }
  }
  hideInput();
  hideTV();
}

void KPlayerPropertiesTrackAudio::load (void)
{
  c_track_set -> setCurrentItem (properties() -> audioIDOption());
  trackChanged (c_track_set -> currentItem());
  c_bitrate -> setText (properties() -> audioBitrateString());
  c_samplerate -> setText (properties() -> samplerateString());
  KPlayerPropertiesAudio::load();
}

void KPlayerPropertiesTrackAudio::save (void)
{
  if ( c_track_set -> currentItem() == c_track_set -> count() - 1 )
    properties() -> setAudioID (labs (c_track -> text().toInt()));
  else
    properties() -> setAudioIDOption (c_track_set -> currentItem());
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

KPlayerPropertiesVideo::KPlayerPropertiesVideo (QWidget* parent, const char* name)
  : KPlayerPropertiesVideoPage (parent, name)
{
}

KPlayerPropertiesVideo::~KPlayerPropertiesVideo()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesVideo::setup (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesVideo::setup\n";
#endif
  setupMedia (url);
  if ( engine() -> videoCodecCount() )
  {
    c_codec -> clear();
    const QString& codec = properties() -> videoCodecValue();
    if ( codec.isEmpty() )
      c_codec -> insertItem (i18n("default"));
    else
      c_codec -> insertItem (s_default_entry.arg (i18n("default")).arg (codec));
    c_codec -> insertItem (i18n("auto"));
    for ( int i = 0; i < engine() -> videoCodecCount(); i ++ )
      c_codec -> insertItem (engine() -> videoCodecName (i));
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
  c_contrast_set -> setCurrentItem (properties() -> contrastOption());
  contrastChanged (c_contrast_set -> currentItem());
  c_brightness_set -> setCurrentItem (properties() -> brightnessOption());
  brightnessChanged (c_brightness_set -> currentItem());
  c_hue_set -> setCurrentItem (properties() -> hueOption());
  hueChanged (c_hue_set -> currentItem());
  c_saturation_set -> setCurrentItem (properties() -> saturationOption());
  saturationChanged (c_saturation_set -> currentItem());
  const QString& codec (properties() -> videoCodecOption());
  c_codec -> setCurrentItem (codec.isNull() ? 0 : engine() -> videoCodecIndex (codec) + 2);
}

void KPlayerPropertiesVideo::save (void)
{
  properties() -> setContrastOption (c_contrast -> text().toInt(), c_contrast_set -> currentItem());
  properties() -> setBrightnessOption (c_brightness -> text().toInt(), c_brightness_set -> currentItem());
  properties() -> setHueOption (c_hue -> text().toInt(), c_hue_set -> currentItem());
  properties() -> setSaturationOption (c_saturation -> text().toInt(), c_saturation_set -> currentItem());
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

KPlayerPropertiesDeviceVideo::KPlayerPropertiesDeviceVideo (QWidget* parent, const char* name)
  : KPlayerPropertiesVideo (parent, name)
{
}

void KPlayerPropertiesDeviceVideo::setupMedia (const KURL& url)
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

KPlayerPropertiesTVDeviceVideo::KPlayerPropertiesTVDeviceVideo (QWidget* parent, const char* name)
  : KPlayerPropertiesDVBDeviceVideo (parent, name)
{
}

void KPlayerPropertiesTVDeviceVideo::setupMedia (const KURL& url)
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
    if ( c_format -> text (i) == format )
    {
      c_format -> setCurrentItem (i);
      break;
    }
  if ( i == c_format -> count() )
    c_format -> setCurrentItem (0);
  int norm = properties() -> videoNorm();
  c_norm -> setCurrentItem (norm >= 0 ? c_norm -> count() - 1 : - norm - 1);
  normChanged (c_norm -> currentItem());
  KPlayerPropertiesDVBDeviceVideo::load();
}

void KPlayerPropertiesTVDeviceVideo::save (void)
{
  properties() -> setVideoFormat (c_format -> currentItem() ? c_format -> currentText() : "");
  properties() -> setVideoNorm (c_norm -> currentItem() == c_norm -> count() - 1 ?
    labs (c_norm_id -> text().toInt()) : - c_norm -> currentItem() - 1);
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

KPlayerPropertiesDVBDeviceVideo::KPlayerPropertiesDVBDeviceVideo (QWidget* parent, const char* name)
  : KPlayerPropertiesDeviceVideo (parent, name)
{
}

void KPlayerPropertiesDVBDeviceVideo::setupMedia (const KURL& url)
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
  c_input_set -> setCurrentItem (properties() -> hasVideoInput() ? 1 : 0);
  inputChanged (c_input_set -> currentItem());
  KPlayerPropertiesDeviceVideo::load();
}

void KPlayerPropertiesDVBDeviceVideo::save (void)
{
  if ( c_input_set -> currentItem() )
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

KPlayerPropertiesTrackVideo::KPlayerPropertiesTrackVideo (QWidget* parent, const char* name)
  : KPlayerPropertiesVideo (parent, name)
{
}

void KPlayerPropertiesTrackVideo::setupMedia (const KURL& url)
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
      c_track_set -> insertItem (languageName (iterator.key(), iterator.data()), i);
      ++ iterator;
      ++ i;
    }
  }
  hideInput();
  hideTV();
}

void KPlayerPropertiesTrackVideo::load (void)
{
  c_track_set -> setCurrentItem (properties() -> videoIDOption());
  trackChanged (c_track_set -> currentItem());
  c_bitrate -> setText (properties() -> videoBitrateString());
  c_framerate -> setText (properties() -> framerateString());
  KPlayerPropertiesVideo::load();
}

void KPlayerPropertiesTrackVideo::save (void)
{
  if ( c_track_set -> currentItem() == c_track_set -> count() - 1 )
    properties() -> setVideoID (labs (c_track -> text().toInt()));
  else
    properties() -> setVideoIDOption (c_track_set -> currentItem());
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

KPlayerPropertiesAdvanced::KPlayerPropertiesAdvanced (QWidget* parent, const char* name)
  : KPlayerPropertiesAdvancedPage (parent, name)
{
}

KPlayerPropertiesAdvanced::~KPlayerPropertiesAdvanced()
{
  KPlayerMedia::release (properties());
}

void KPlayerPropertiesAdvanced::setup (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesAdvanced::setup\n";
#endif
  setupMedia (url);
  if ( engine() -> demuxerCount() )
  {
    c_demuxer -> clear();
    const QString& demuxer = properties() -> demuxerValue();
    if ( demuxer.isEmpty() )
      c_demuxer -> insertItem (i18n("default"));
    else
      c_demuxer -> insertItem (s_default_entry.arg (i18n("default")).arg (demuxer));
    c_demuxer -> insertItem (i18n("auto"));
    for ( int i = 0; i < engine() -> demuxerCount(); i ++ )
      c_demuxer -> insertItem (engine() -> demuxerName (i));
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
  c_command_line_option -> setCurrentItem (properties() -> commandLineOption());
  commandLineChanged (c_command_line_option -> currentItem());
  const QString& demuxer (properties() -> demuxerOption());
  c_demuxer -> setCurrentItem (demuxer.isNull() ? 0 : engine() -> demuxerIndex (demuxer) + 2);
  c_frame_drop -> setCurrentItem (properties() -> frameDropOption());
  c_use_cache -> setCurrentItem (properties() -> cacheOption());
  cacheChanged (c_use_cache -> currentItem());
  c_build_index -> setCurrentItem (properties() -> buildNewIndexOption());
}

void KPlayerPropertiesAdvanced::save (void)
{
  properties() -> setCommandLineOption (c_command_line -> text(), c_command_line_option -> currentItem());
  properties() -> setDemuxerOption (listEntry (c_demuxer, true));
  properties() -> setFrameDropOption (c_frame_drop -> currentItem());
  properties() -> setCacheOption (c_use_cache -> currentItem(), labs (c_cache_size -> text().toInt()));
  properties() -> setBuildNewIndexOption (c_build_index -> currentItem());
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

KPlayerPropertiesDeviceAdvanced::KPlayerPropertiesDeviceAdvanced (QWidget* parent, const char* name)
  : KPlayerPropertiesAdvanced (parent, name)
{
}

void KPlayerPropertiesDeviceAdvanced::setupMedia (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesDeviceAdvanced::setupMedia\n";
#endif
  m_properties = KPlayerMedia::deviceProperties (url);
}

KPlayerPropertiesTVDeviceAdvanced::KPlayerPropertiesTVDeviceAdvanced (QWidget* parent, const char* name)
  : KPlayerPropertiesDeviceAdvanced (parent, name)
{
}

void KPlayerPropertiesTVDeviceAdvanced::setupMedia (const KURL& url)
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
  c_decimation -> setCurrentItem (decimation == 0 ? 1 : decimation == 4 ? 2 : decimation - 1);
  KPlayerPropertiesDeviceAdvanced::load();
}

void KPlayerPropertiesTVDeviceAdvanced::save (void)
{
  properties() -> setMjpegDecimation (! c_compression -> isChecked() ? 0
    : c_decimation -> currentItem() == 2 ? 4 : c_decimation -> currentItem() + 1);
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

KPlayerPropertiesTrackAdvanced::KPlayerPropertiesTrackAdvanced (QWidget* parent, const char* name)
  : KPlayerPropertiesAdvanced (parent, name)
{
}

void KPlayerPropertiesTrackAdvanced::setupMedia (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES_DIALOG
  kdDebugTime() << "KPlayerPropertiesTrackAdvanced::setupMedia\n";
#endif
  m_properties = KPlayerMedia::trackProperties (url);
}

KPlayerPropertiesItemAdvanced::KPlayerPropertiesItemAdvanced (QWidget* parent, const char* name)
  : KPlayerPropertiesTrackAdvanced (parent, name)
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
  c_use_kioslave -> setCurrentItem (properties() -> useKioslaveOption());
  c_use_temporary_file -> setCurrentItem (properties() -> useTemporaryFileOption());
  KPlayerPropertiesTrackAdvanced::load();
}

void KPlayerPropertiesItemAdvanced::save (void)
{
  properties() -> setUseKioslaveOption (c_use_kioslave -> currentItem());
  properties() -> setUseTemporaryFileOption (c_use_temporary_file -> currentItem());
  KPlayerPropertiesTrackAdvanced::save();
}
