/***************************************************************************
                          kplayersettingsdialog.cpp
                          -------------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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
#include <math.h>
#include <stdlib.h>

#include <kdebug.h>

#include "kplayersettingsdialog.h"
#include "kplayersettingsdialog.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

#define DEBUG_KPLAYER_SETTINGS_DIALOG

static QString s_default_entry (i18n("%1 (%2)"));
static QRegExp re_key_value ("^([^:]+): *(.*)$");

KPlayerSettingsDialog::KPlayerSettingsDialog (QWidget* parent)
  : KDialogBase (TreeList, i18n("KPlayer Preferences"), Help | Default | Ok | Apply | Cancel, Ok, parent)
{
//m_initial_move = false;
  QApplication::connect (this, SIGNAL (aboutToShowPage (QWidget*)), this, SLOT (pageAboutToShow(QWidget*)));
  QFrame* frame = addPage (i18n("General"), i18n("General Settings"));
  QVBoxLayout* layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_general = new KPlayerSettingsGeneral (frame, "general");
  frame = addPage (i18n("Playlist"), i18n("Playlist Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_playlist = new KPlayerSettingsPlaylist (frame, "playlist");
  frame = addPage (i18n("Controls"), i18n("Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_controls = new KPlayerSettingsControls (frame, "controls");
  frame = addPage (i18n("Video"), i18n("Video Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_video = new KPlayerSettingsVideo (frame, "video");
  frame = addPage (i18n("Audio"), i18n("Audio Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_audio = new KPlayerSettingsAudio (frame, "audio");
  frame = addPage (i18n("Subtitles"), i18n("Subtitle Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_subtitles = new KPlayerSettingsSubtitles (frame, "subtitles");
  frame = addPage (i18n("Messages"), i18n("Message Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_messages = new KPlayerSettingsMessages (frame, "messages");
  frame = addPage (i18n("Advanced"), i18n("Advanced Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_advanced = new KPlayerSettingsAdvanced (frame, "advanced");
  frame = addPage (QStringList() << i18n("Controls") << i18n("Progress"), i18n("Progress Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_progress = new KPlayerSettingsProgress (frame, "progress");
  frame = addPage (QStringList() << i18n("Controls") << i18n("Volume"), i18n("Volume Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_volume = new KPlayerSettingsVolume (frame, "volume");
  frame = addPage (QStringList() << i18n("Controls") << i18n("Contrast"), i18n("Contrast Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_contrast = new KPlayerSettingsContrast (frame, "contrast");
  frame = addPage (QStringList() << i18n("Controls") << i18n("Brightness"), i18n("Brightness Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_brightness = new KPlayerSettingsBrightness (frame, "brightness");
  frame = addPage (QStringList() << i18n("Controls") << i18n("Hue"), i18n("Hue Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_hue = new KPlayerSettingsHue (frame, "hue");
  frame = addPage (QStringList() << i18n("Controls") << i18n("Saturation"), i18n("Saturation Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_saturation = new KPlayerSettingsSaturation (frame, "saturation");
  frame = addPage (QStringList() << i18n("Controls") << i18n("Sliders"), i18n("Slider Control Settings"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_sliders = new KPlayerSettingsSliders (frame, "sliders");
  setHelp ("settings");
  KListView* view = (KListView*) child (0, "KListView");
  if ( view )
  {
    for ( QListViewItem* item = view -> firstChild(); item; item = item -> nextSibling() )
      item -> setOpen (true);
    view -> setMinimumSize (view -> sizeHint());
  }
  KConfig* config = kPlayerConfig();
  config -> setGroup ("General Options");
  QString name (config -> readEntry ("Settings Dialog Page"));
  if ( ! name.isEmpty() )
  {
    frame = (QFrame*) child (name.latin1());
    if ( frame )
    {
      frame = (QFrame*) frame -> parent();
      if ( frame )
        showPage (pageIndex (frame));
    }
  }
/*int x = config -> readNumEntry ("Settings Dialog Left", -1);
  int y = config -> readNumEntry ("Settings Dialog Top", -1);*/
  int w = config -> readNumEntry ("Settings Dialog Width");
  int h = config -> readNumEntry ("Settings Dialog Height");
//QSize size (config -> readNumEntry ("Settings Dialog Width"),
//  config -> readNumEntry ("Settings Dialog Height"));
//QSize hint = minimumSizeHint();
//if ( size.width() < hint.width() || size.height() < hint.height() )
//  size = sizeHint();
//setGeometry (x, y, size.width(), size.height());
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
//kdDebugTime() << "KPlayerSettingsDialog position " << x << "x" << y << " size " << w << "x" << h << "\n";
#endif
/*if ( x >= 0 && y >= 0 )
  {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << "KPlayerSettingsDialog moving to " << x << "x" << y << "\n";
#endif
    move (x, y);
  }*/
  if ( w > 0 && h > 0 )
  {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << "KPlayerSettingsDialog resizing to " << w << "x" << h << "\n";
#endif
    resize (w, h);
  }
}

KPlayerSettingsDialog::~KPlayerSettingsDialog (void)
{
  kPlayerEngine() -> getDriversCodecs();
  KConfig* config = kPlayerConfig();
  config -> setGroup ("General Options");
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog " << x() << "x" << y() << " " << width() << "x" << height() << " Hint " << sizeHint().width() << "x" << sizeHint().height() << "\n";
#endif
/*if ( size() == sizeHint() )
  {
    config -> deleteEntry ("Settings Dialog Width");
    config -> deleteEntry ("Settings Dialog Height");
  }
  else
  {*/
/*config -> writeEntry ("Settings Dialog Left", frameGeometry().x());
  config -> writeEntry ("Settings Dialog Top", frameGeometry().y());*/
  config -> writeEntry ("Settings Dialog Width", width());
  config -> writeEntry ("Settings Dialog Height", height());
}

/*void KPlayerSettingsDialog::moveEvent (QMoveEvent* event)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog move " << event -> oldPos().x() << "x" << event -> oldPos().y()
    << " => " << event -> pos().x() << "x" << event -> pos().y() << "\n";
#endif
  KDialogBase::moveEvent (event);
  if ( m_initial_move )
    return;
  m_initial_move = true;
  KConfig* config = kPlayerConfig();
  config -> setGroup ("General Options");
  int x = config -> readNumEntry ("Settings Dialog Left", -1);
  int y = config -> readNumEntry ("Settings Dialog Top", -1);
  if ( x >= 0 && y >= 0 )
  {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << "KPlayerSettingsDialog moving to " << x << "x" << y << "\n";
#endif
    move (x + frameGeometry().x() - geometry().x(), y + frameGeometry().y() - geometry().y());
  }
  m_initial_move = false;
}

void KPlayerSettingsDialog::resizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog resize " << event -> oldSize(). width() << "x" << event -> oldSize(). height()
    << " => " << event -> size(). width() << "x" << event -> size(). height() << "\n";
#endif
  KDialogBase::resizeEvent (event);
}

void KPlayerSettingsDialog::showEvent (QShowEvent* event)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog::showEvent\n";
#endif
  KDialogBase::showEvent (event);
}

void KPlayerSettingsDialog::windowActivationChange (bool old)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog::windowActivationChange " << old << "\n";
#endif
  m_initial_move = true;
  KDialogBase::windowActivationChange (old);
}*/

void KPlayerSettingsDialog::slotDefault (void)
{
  if ( KMessageBox::warningYesNo (this, i18n("All configuration settings will be reset.\n\nAre you sure?"))
      != KMessageBox::Yes )
    return;
  kPlayerSettings() -> defaults();
  kPlayerSettings() -> save();
  m_general -> load();
  m_playlist -> load();
  m_controls -> load();
  m_video -> load();
  m_audio -> load();
  m_sliders -> load();
  m_subtitles -> load();
  m_messages -> load();
  m_advanced -> load();
  m_progress -> load();
  m_volume -> load();
  m_contrast -> load();
  m_brightness -> load();
  m_hue -> load();
  m_saturation -> load();
  setButtonCancel(KStdGuiItem::close());
  KDialogBase::slotDefault();
}

void KPlayerSettingsDialog::pageAboutToShow (QWidget* page)
{
  m_advanced -> refresh();
  /*setHelp (page == m_general -> parent() ? "settings-general" : page == m_playlist -> parent() ? "settings-playlist"
    : page == m_controls -> parent() ? "settings-controls" : page == m_progress -> parent() ? "settings-progress"
    : page == m_volume -> parent() ? "settings-volume" : page == m_contrast -> parent() ? "settings-contrast"
    : page == m_brightness -> parent() ? "settings-brightness" : page == m_hue -> parent() ? "settings-hue"
    : page == m_saturation -> parent() ? "settings-saturation" : page == m_sliders -> parent() ? "settings-sliders"
    : page == m_messages -> parent() ? "settings-messages" : page == m_subtitles -> parent() ? "settings-subtitles"
    : page == m_audio -> parent() ? "settings-audio" : page == m_video -> parent() ? "settings-video"
    : page == m_advanced -> parent() ? "settings-advanced" : "settings");*/
  QObject* object = page -> child (0, "QFrame");
  KConfig* config = kPlayerConfig();
  config -> setGroup ("General Options");
  QString name;
  if ( object )
    name = object -> name ("");
  if ( name.isEmpty() )
    config -> deleteEntry ("Settings Dialog Page");
  else
    config -> writeEntry ("Settings Dialog Page", name);
  setHelp (name.isEmpty() ? "settings" : "settings-" + name);
}

void KPlayerSettingsDialog::slotOk (void)
{
  slotApply();
  KDialogBase::slotOk();
}

void KPlayerSettingsDialog::slotApply (void)
{
  m_advanced -> save();
  m_audio -> save();
  m_controls -> save();
  m_general -> save();
  m_playlist -> save();
  m_messages -> save();
  m_sliders -> save();
  m_subtitles -> save();
  m_video -> save();
  m_progress -> save();
  m_volume -> save();
  m_contrast -> save();
  m_brightness -> save();
  m_hue -> save();
  m_saturation -> save();
  kPlayerSettings() -> save();
  setButtonCancel(KStdGuiItem::close());
  KDialogBase::slotApply();
}

KPlayerSettingsAdvanced::KPlayerSettingsAdvanced (QWidget* parent, const char* name)
  : KPlayerSettingsAdvancedPage (parent, name)
{
  load();
}

void KPlayerSettingsAdvanced::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_executable_path -> setText (settings -> executablePathDefault());
  c_command_line -> setText (settings -> commandLineDefault());
  c_frame_drop -> setCurrentItem (settings -> frameDropDefault());
  c_use_cache -> setCurrentItem (settings -> cacheDefault());
  cacheChanged (c_use_cache -> currentItem());
  c_build_index -> setCurrentItem (settings -> buildNewIndexDefault());
  c_osd_level -> setCurrentItem (settings -> osdLevelDefault());
  c_use_temporary_file -> setChecked (settings -> useTemporaryFileDefault());
  c_use_kioslave_for_http -> setChecked (settings -> useKioslaveForHttp());
  c_use_kioslave_for_ftp -> setChecked (settings -> useKioslaveForFtp());
  c_use_kioslave_for_smb -> setChecked (settings -> useKioslaveForSmb());
}

void KPlayerSettingsAdvanced::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setExecutablePathDefault (c_executable_path -> text());
  settings -> setCommandLineDefault (c_command_line -> text());
  settings -> setFrameDropDefault (c_frame_drop -> currentItem());
  settings -> setCacheDefault (c_use_cache -> currentItem());
  if ( c_use_cache -> currentItem() == 2 )
    settings -> setCacheSizeDefault (c_cache_size -> text().toLong());
  settings -> setBuildNewIndexDefault (c_build_index -> currentItem());
  settings -> setOsdLevelDefault (c_osd_level -> currentItem());
  settings -> setUseTemporaryFileDefault (c_use_temporary_file -> isChecked());
  settings -> setUseKioslaveForHttp (c_use_kioslave_for_http -> isChecked());
  settings -> setUseKioslaveForFtp (c_use_kioslave_for_ftp -> isChecked());
  settings -> setUseKioslaveForSmb (c_use_kioslave_for_smb -> isChecked());
}

void KPlayerSettingsAdvanced::refresh (void)
{
  kPlayerEngine() -> getDriversCodecs (c_executable_path -> text());
}

void KPlayerSettingsAdvanced::cacheChanged (int cache)
{
  if ( cache == 2 )
    c_cache_size -> setText (QString::number (kPlayerSettings() -> cacheSizeDefault()));
  else
    c_cache_size -> setText ("");
  c_cache_size -> setEnabled (cache == 2);
  l_cache_size_kb -> setEnabled (cache == 2);
  if ( cache == 2 && sender() )
  {
    c_cache_size -> setFocus();
    c_cache_size -> selectAll();
  }
}

KPlayerSettingsAudio::KPlayerSettingsAudio (QWidget* parent, const char* name)
  : KPlayerSettingsAudioPage (parent, name)
{
  loadLists();
  load();
  QApplication::connect (kPlayerEngine(), SIGNAL (refresh()), this, SLOT (refresh()));
}

void KPlayerSettingsAudio::refresh (void)
{
  int dindex = c_driver -> currentItem();
  if ( dindex > 0 )
    dindex = kPlayerEngine() -> audioDriverIndex (listEntry (c_driver)) + 1;
  int cindex = c_codec -> currentItem();
  if ( cindex > 0 )
    cindex = kPlayerEngine() -> audioCodecIndex (listEntry (c_codec)) + 1;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "Audio Driver: " << c_driver -> currentText() << " => " << dindex << " / " << kPlayerEngine() -> audioDriverCount() << "\n";
  kdDebugTime() << "Audio Codec: " << c_codec -> currentText() << " => " << cindex << " / " << kPlayerEngine() -> audioCodecCount() << "\n";
#endif
  loadLists();
  c_driver -> setCurrentItem (dindex);
  driverChanged (dindex);
  c_codec -> setCurrentItem (cindex);
  codecChanged (cindex);
}

void KPlayerSettingsAudio::loadLists (void)
{
  if ( kPlayerEngine() -> audioCodecCount() )
  {
    c_codec -> clear();
    c_codec -> insertItem (i18n("auto"));
    for ( int i = 0; i < kPlayerEngine() -> audioCodecCount(); i ++ )
      c_codec -> insertItem (kPlayerEngine() -> audioCodecName (i));
  }
  if ( kPlayerEngine() -> audioDriverCount() )
  {
    c_driver -> clear();
    c_driver -> insertItem (i18n("auto"));
    for ( int i = 0; i < kPlayerEngine() -> audioDriverCount(); i ++ )
      c_driver -> insertItem (kPlayerEngine() -> audioDriverName (i));
  }
}

void KPlayerSettingsAudio::load (void)
{
  c_driver -> setCurrentItem (kPlayerEngine() -> audioDriverIndex (kPlayerSettings() -> audioDriverDefault()) + 1);
  driverChanged (c_driver -> currentItem());
  c_codec -> setCurrentItem (kPlayerEngine() -> audioCodecIndex (kPlayerSettings() -> audioCodecDefault()) + 1);
  codecChanged (c_codec -> currentItem());
  c_delay_step -> setText (QString::number (kPlayerSettings() -> audioDelayStep()));
}

void KPlayerSettingsAudio::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setAudioDriverDefault (listEntry (c_driver));
  if ( c_driver -> currentItem() > 0 )
  {
    settings -> setAudioDriverFallbackDefault (c_driver_fallback -> isChecked());
    settings -> setAudioDeviceDefault (c_device -> text());
  }
  settings -> setAudioCodecDefault (listEntry (c_codec));
  if ( c_codec -> currentItem() > 0 )
    settings -> setAudioCodecFallbackDefault (c_codec_fallback -> isChecked());
  settings -> setAudioDelayStep (fabs (c_delay_step -> text().toFloat()));
}

void KPlayerSettingsAudio::driverChanged (int index)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_driver_fallback -> setChecked (index > 0 && settings -> audioDriverFallbackDefault());
  c_driver_fallback -> setEnabled (index > 0);
  c_device -> setText (index > 0 ? settings -> audioDeviceDefault() : "");
  c_device -> setEnabled (index > 0);
}

void KPlayerSettingsAudio::codecChanged (int index)
{
  c_codec_fallback -> setChecked (index > 0 && kPlayerSettings() -> audioCodecFallbackDefault());
  c_codec_fallback -> setEnabled (index > 0);
}

KPlayerSettingsControls::KPlayerSettingsControls (QWidget* parent, const char* name)
  : KPlayerSettingsControlsPage (parent, name)
{
  load();
}

void KPlayerSettingsControls::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_shift -> setChecked (settings -> rememberWithShift());
  c_size -> setChecked (settings -> rememberSize());
  rememberSizeChanged (settings -> rememberSize());
  c_full_screen -> setChecked (settings -> rememberFullScreen());
  c_maximized -> setChecked (settings -> rememberMaximized());
  c_maintain_aspect -> setChecked (settings -> rememberMaintainAspect());
  c_volume -> setChecked (settings -> rememberVolume());
  c_audio_delay -> setChecked (settings -> rememberAudioDelay());
  c_frame_drop -> setChecked (settings -> rememberFrameDrop());
  c_contrast -> setChecked (settings -> rememberContrast());
  c_brightness -> setChecked (settings -> rememberBrightness());
  c_hue -> setChecked (settings -> rememberHue());
  c_saturation -> setChecked (settings -> rememberSaturation());
  c_subtitle_url -> setChecked (settings -> rememberSubtitleUrl());
  c_subtitle_visibility -> setChecked (settings -> rememberSubtitleVisibility());
  c_subtitle_position -> setChecked (settings -> rememberSubtitlePosition());
  c_subtitle_delay -> setChecked (settings -> rememberSubtitleDelay());
}

void KPlayerSettingsControls::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setRememberWithShift (c_shift -> isChecked());
  settings -> setRememberSize (c_size -> isChecked());
  if ( ! settings -> rememberSize() )
    settings -> setRememberAspect (c_aspect -> isChecked());
  settings -> setRememberFullScreen (c_full_screen -> isChecked());
  settings -> setRememberMaximized (c_maximized -> isChecked());
  settings -> setRememberMaintainAspect (c_maintain_aspect -> isChecked());
  settings -> setRememberVolume (c_volume -> isChecked());
  settings -> setRememberAudioDelay (c_audio_delay -> isChecked());
  settings -> setRememberFrameDrop (c_frame_drop -> isChecked());
  settings -> setRememberContrast (c_contrast -> isChecked());
  settings -> setRememberBrightness (c_brightness -> isChecked());
  settings -> setRememberHue (c_hue -> isChecked());
  settings -> setRememberSaturation (c_saturation -> isChecked());
  settings -> setRememberSubtitleUrl (c_subtitle_url -> isChecked());
  settings -> setRememberSubtitleVisibility (c_subtitle_visibility -> isChecked());
  settings -> setRememberSubtitlePosition (c_subtitle_position -> isChecked());
  settings -> setRememberSubtitleDelay (c_subtitle_delay -> isChecked());
}

void KPlayerSettingsControls::rememberSizeChanged (bool checked)
{
  c_aspect -> setChecked (checked || kPlayerSettings() -> rememberAspect());
  c_aspect -> setEnabled (! checked);
}

KPlayerSettingsGeneral::KPlayerSettingsGeneral (QWidget* parent, const char* name)
  : KPlayerSettingsGeneralPage (parent, name)
{
  load();
}

void KPlayerSettingsGeneral::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_resize_automatically -> setChecked (settings -> resizeAutomatically());
  resizeAutomaticallyChanged (settings -> resizeAutomatically());
  c_recent_file_list_size -> setText (QString::number (settings -> recentFileListSize()));
  c_show_file_path -> setChecked (settings -> showFilePath());
  c_show_open_dialog -> setChecked (settings -> showOpenDialog());
  c_disable_screen_saver -> setChecked (settings -> disableScreenSaver());
}

void KPlayerSettingsGeneral::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setResizeAutomatically (c_resize_automatically -> isChecked());
  if ( settings -> resizeAutomatically() )
    settings -> setMinimumInitialWidth (labs (c_minimum_initial_width -> text().toLong()));
  settings -> setRecentFileListSize (labs (c_recent_file_list_size -> text().toLong()));
  settings -> setShowFilePath (c_show_file_path -> isChecked());
  settings -> setShowOpenDialog (c_show_open_dialog -> isChecked());
  settings -> setDisableScreenSaver (c_disable_screen_saver -> isChecked());
}

void KPlayerSettingsGeneral::resizeAutomaticallyChanged (bool resizeAutomaticallyChecked)
{
  KPlayerSettings* settings = kPlayerSettings();
  if ( resizeAutomaticallyChecked )
    c_minimum_initial_width -> setText (QString::number (settings -> minimumInitialWidth()));
  else
    c_minimum_initial_width -> setText ("");
  c_minimum_initial_width -> setEnabled (resizeAutomaticallyChecked);
  l_minimum_initial_width -> setEnabled (resizeAutomaticallyChecked);
}

KPlayerSettingsPlaylist::KPlayerSettingsPlaylist (QWidget* parent, const char* name)
  : KPlayerSettingsPlaylistPage (parent, name)
{
  load();
}

void KPlayerSettingsPlaylist::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_start_playing -> setChecked (settings -> startPlayingImmediately());
  startPlayingChanged (settings -> startPlayingImmediately());
  c_allow_duplicate_entries -> setChecked (settings -> allowDuplicateEntries());
  c_playlist_size_limit -> setText (QString::number (settings -> playlistSizeLimit()));
  c_cache_size_limit -> setText (QString::number (settings -> cacheSizeLimit()));
}

void KPlayerSettingsPlaylist::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setStartPlayingImmediately (c_start_playing -> isChecked());
  if ( settings -> startPlayingImmediately() )
    settings -> setStartPlayingOnlyIfIdle (c_only_if_idle -> isChecked());
  settings -> setAllowDuplicateEntries (c_allow_duplicate_entries -> isChecked());
  settings -> setPlaylistSizeLimit (labs (c_playlist_size_limit -> text().toLong()));
  settings -> setCacheSizeLimit (labs (c_cache_size_limit -> text().toLong()));
}

void KPlayerSettingsPlaylist::startPlayingChanged (bool checked)
{
  c_only_if_idle -> setChecked (checked && kPlayerSettings() -> startPlayingOnlyIfIdle());
  c_only_if_idle -> setEnabled (checked);
}

KPlayerSettingsMessages::KPlayerSettingsMessages (QWidget* parent, const char* name)
  : KPlayerSettingsMessagesPage (parent, name)
{
  load();
}

void KPlayerSettingsMessages::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_show_on_error -> setChecked (settings -> showMessagesOnError());
  c_show_before -> setChecked (settings -> showMessagesBeforePlaying());
  c_show_after -> setChecked (settings -> showMessagesAfterPlaying());
  c_clear_before -> setChecked (settings -> clearMessagesBeforePlaying());
}

void KPlayerSettingsMessages::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setShowMessagesOnError (c_show_on_error -> isChecked());
  settings -> setShowMessagesBeforePlaying (c_show_before -> isChecked());
  settings -> setShowMessagesAfterPlaying (c_show_after -> isChecked());
  settings -> setClearMessagesBeforePlaying (c_clear_before -> isChecked());
}

KPlayerSettingsSliders::KPlayerSettingsSliders (QWidget* parent, const char* name)
  : KPlayerSettingsSlidersPage (parent, name)
{
  load();
}

void KPlayerSettingsSliders::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_preferred_slider_length -> setText (QString::number (settings -> preferredSliderLength()));
  c_minimum_slider_length -> setText (QString::number (settings -> minimumSliderLength()));
}

void KPlayerSettingsSliders::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setPreferredSliderLength (labs (c_preferred_slider_length -> text().toLong()));
  settings -> setMinimumSliderLength (labs (c_minimum_slider_length -> text().toLong()));
}

KPlayerSettingsSubtitles::KPlayerSettingsSubtitles (QWidget* parent, const char* name)
  : KPlayerSettingsSubtitlesPage (parent, name)
{
  load();
}

void KPlayerSettingsSubtitles::load (void)
{
  c_position_step -> setText (QString::number (kPlayerSettings() -> subtitlePositionStep()));
  c_delay_step -> setText (QString::number (kPlayerSettings() -> subtitleDelayStep()));
  c_subtitles_autoload -> setChecked (kPlayerSettings() -> subtitleAutoloadDefault());
  autoloadSubtitlesChanged (kPlayerSettings() -> subtitleAutoloadDefault());
}

void KPlayerSettingsSubtitles::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setSubtitlePositionStep (labs (c_position_step -> text().toLong()));
  settings -> setSubtitleDelayStep (fabs (c_delay_step -> text().toFloat()));
  settings -> setSubtitleAutoloadDefault (c_subtitles_autoload -> isChecked());
  if ( settings -> subtitleAutoloadDefault() )
  {
    settings -> setAutoloadAqtSubtitles (c_subtitles_aqt -> isChecked());
    settings -> setAutoloadJssSubtitles (c_subtitles_jss -> isChecked());
    settings -> setAutoloadRtSubtitles (c_subtitles_rt -> isChecked());
    settings -> setAutoloadSmiSubtitles (c_subtitles_smi -> isChecked());
    settings -> setAutoloadSrtSubtitles (c_subtitles_srt -> isChecked());
    settings -> setAutoloadSsaSubtitles (c_subtitles_ssa -> isChecked());
    settings -> setAutoloadSubSubtitles (c_subtitles_sub -> isChecked());
    settings -> setAutoloadTxtSubtitles (c_subtitles_txt -> isChecked());
    settings -> setAutoloadUtfSubtitles (c_subtitles_utf -> isChecked());
    settings -> setAutoloadOtherSubtitles (c_subtitles_other -> isChecked());
    if ( settings -> autoloadOtherSubtitles() )
      settings -> setAutoloadExtensionList (c_subtitles_list -> text());
  }
}

void KPlayerSettingsSubtitles::otherSubtitlesChanged (bool otherSubtitlesChecked)
{
  KPlayerSettings* settings = kPlayerSettings();
  if ( otherSubtitlesChecked )
    c_subtitles_list -> setText (settings -> autoloadExtensionList());
  else
    c_subtitles_list -> setText ("");
  c_subtitles_list -> setEnabled (otherSubtitlesChecked);
  if ( otherSubtitlesChecked && sender() )
  {
    c_subtitles_list -> setFocus();
    c_subtitles_list -> selectAll();
  }
}

void KPlayerSettingsSubtitles::autoloadSubtitlesChanged (bool autoloadSubtitlesChecked)
{
  KPlayerSettings* settings = kPlayerSettings();
  if ( autoloadSubtitlesChecked )
  {
    c_subtitles_aqt -> setChecked (settings -> autoloadAqtSubtitles());
    c_subtitles_jss -> setChecked (settings -> autoloadJssSubtitles());
    c_subtitles_rt -> setChecked (settings -> autoloadRtSubtitles());
    c_subtitles_smi -> setChecked (settings -> autoloadSmiSubtitles());
    c_subtitles_srt -> setChecked (settings -> autoloadSrtSubtitles());
    c_subtitles_ssa -> setChecked (settings -> autoloadSsaSubtitles());
    c_subtitles_sub -> setChecked (settings -> autoloadSubSubtitles());
    c_subtitles_txt -> setChecked (settings -> autoloadTxtSubtitles());
    c_subtitles_utf -> setChecked (settings -> autoloadUtfSubtitles());
    c_subtitles_other -> setChecked (settings -> autoloadOtherSubtitles());
  }
  else
  {
    c_subtitles_aqt -> setChecked (false);
    c_subtitles_jss -> setChecked (false);
    c_subtitles_rt -> setChecked (false);
    c_subtitles_smi -> setChecked (false);
    c_subtitles_srt -> setChecked (false);
    c_subtitles_ssa -> setChecked (false);
    c_subtitles_sub -> setChecked (false);
    c_subtitles_txt -> setChecked (false);
    c_subtitles_utf -> setChecked (false);
    c_subtitles_other -> setChecked (false);
  }
  c_subtitles_aqt -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_jss -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_rt -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_smi -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_srt -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_ssa -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_sub -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_txt -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_utf -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_other -> setEnabled (autoloadSubtitlesChecked);
  otherSubtitlesChanged (c_subtitles_other -> isChecked());
}

KPlayerSettingsVideo::KPlayerSettingsVideo (QWidget* parent, const char* name)
  : KPlayerSettingsVideoPage (parent, name)
{
  loadLists();
  load();
  QApplication::connect (kPlayerEngine(), SIGNAL (refresh()), this, SLOT (refresh()));
}

void KPlayerSettingsVideo::refresh (void)
{
  int dindex = c_driver -> currentItem();
  if ( dindex > 0 )
    dindex = kPlayerEngine() -> videoDriverIndex (listEntry (c_driver)) + 1;
  int cindex = c_codec -> currentItem();
  if ( cindex > 0 )
    cindex = kPlayerEngine() -> videoCodecIndex (listEntry (c_codec)) + 1;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "Video Driver: " << c_driver -> currentText() << " => " << dindex << " / " << kPlayerEngine() -> videoDriverCount() << "\n";
  kdDebugTime() << "Video Codec: " << c_codec -> currentText() << " => " << cindex << " / " << kPlayerEngine() -> videoCodecCount() << "\n";
#endif
  loadLists();
  c_driver -> setCurrentItem (dindex);
  driverChanged (dindex);
  c_codec -> setCurrentItem (cindex);
  codecChanged (cindex);
}

void KPlayerSettingsVideo::loadLists (void)
{
  if ( kPlayerEngine() -> videoCodecCount() )
  {
    c_codec -> clear();
    c_codec -> insertItem (i18n("auto"));
    for ( int i = 0; i < kPlayerEngine() -> videoCodecCount(); i ++ )
      c_codec -> insertItem (kPlayerEngine() -> videoCodecName (i));
  }
  if ( kPlayerEngine() -> videoDriverCount() )
  {
    c_driver -> clear();
    c_driver -> insertItem (i18n("auto"));
    for ( int i = 0; i < kPlayerEngine() -> videoDriverCount(); i ++ )
      c_driver -> insertItem (kPlayerEngine() -> videoDriverName (i));
  }
}

void KPlayerSettingsVideo::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_driver -> setCurrentItem (kPlayerEngine() -> videoDriverIndex (settings -> videoDriverDefault()) + 1);
  driverChanged (c_driver -> currentItem());
  c_codec -> setCurrentItem (kPlayerEngine() -> videoCodecIndex (settings -> videoCodecDefault()) + 1);
  codecChanged (c_codec -> currentItem());
  c_scaler -> setCurrentItem (settings -> videoScalerDefault());
  c_doublebuffering -> setChecked (settings -> videoDoubleBufferingDefault());
  c_direct_rendering -> setChecked (settings -> videoDirectRenderingDefault());
}

void KPlayerSettingsVideo::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setVideoDriverDefault (listEntry (c_driver));
  if ( c_driver -> currentItem() > 0 )
  {
    settings -> setVideoDriverFallbackDefault (c_driver_fallback -> isChecked());
    settings -> setVideoDeviceDefault (c_device -> text());
  }
  settings -> setVideoCodecDefault (listEntry (c_codec));
  if ( c_codec -> currentItem() > 0 )
    settings -> setVideoCodecFallbackDefault (c_codec_fallback -> isChecked());
  settings -> setVideoScalerDefault (c_scaler -> currentItem());
  settings -> setVideoDoubleBufferingDefault (c_doublebuffering -> isChecked());
  settings -> setVideoDirectRenderingDefault (c_direct_rendering -> isChecked());
}

void KPlayerSettingsVideo::driverChanged (int index)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_driver_fallback -> setChecked (index > 0 && settings -> videoDriverFallbackDefault());
  c_driver_fallback -> setEnabled (index > 0);
  c_device -> setText (index > 0 ? settings -> videoDeviceDefault() : "");
  c_device -> setEnabled (index > 0);
}

void KPlayerSettingsVideo::codecChanged (int index)
{
  c_codec_fallback -> setChecked (index > 0 && kPlayerSettings() -> videoCodecFallbackDefault());
  c_codec_fallback -> setEnabled (index > 0);
}

KPlayerSettingsProgress::KPlayerSettingsProgress (QWidget* parent, const char* name)
  : KPlayerSettingsProgressPage (parent, name)
{
  load();
}

void KPlayerSettingsProgress::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_progress_seek -> setText (QString::number (settings -> progressNormalSeek()));
  c_progress_seek_units -> setCurrentItem (settings -> progressNormalSeekUnits());
  c_progress_fast -> setText (QString::number (settings -> progressFastSeek()));
  c_progress_fast_units -> setCurrentItem (settings -> progressFastSeekUnits());
  c_progress_marks -> setText (QString::number (settings -> progressMarks()));
}

void KPlayerSettingsProgress::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setProgressNormalSeekUnits (c_progress_seek_units -> currentItem());
  settings -> setProgressFastSeekUnits (c_progress_fast_units -> currentItem());
  settings -> setProgressNormalSeek (labs (c_progress_seek -> text().toLong()));
  settings -> setProgressFastSeek (labs (c_progress_fast -> text().toLong()));
  settings -> setProgressMarks (labs (c_progress_marks -> text().toLong()));
}

KPlayerSettingsVolume::KPlayerSettingsVolume (QWidget* parent, const char* name)
  : KPlayerSettingsVolumePage (parent, name)
{
  load();
}

void KPlayerSettingsVolume::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_volume_minimum -> setText (QString::number (settings -> volumeMinimum()));
  c_volume_maximum -> setText (QString::number (settings -> volumeMaximum()));
  c_volume_marks -> setText (QString::number (settings -> volumeMarks()));
  c_volume_step -> setText (QString::number (settings -> volumeStep()));
  c_volume_reset -> setChecked (settings -> volumeReset());
  resetChanged (settings -> volumeReset());
}

void KPlayerSettingsVolume::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setVolumeMinimumMaximum (c_volume_minimum -> text().toLong(), labs (c_volume_maximum -> text().toLong()));
  settings -> setVolumeMarks (labs (c_volume_marks -> text().toLong()));
  settings -> setVolumeStep (labs (c_volume_step -> text().toLong()));
  settings -> setVolumeReset (c_volume_reset -> isChecked());
  if ( settings -> volumeReset() )
  {
    settings -> setInitialVolume (labs (c_volume_default -> text().toLong()));
    settings -> setVolumeEvery (c_volume_every -> currentItem());
  }
}

void KPlayerSettingsVolume::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    KPlayerSettings* settings = kPlayerSettings();
    c_volume_default -> setText (QString::number (settings -> initialVolume()));
    c_volume_every -> setCurrentItem (0);
    c_volume_every -> setCurrentText (i18n("file"));
    c_volume_every -> setCurrentItem (settings -> volumeEvery());
  }
  else
  {
    c_volume_default -> setText ("");
    c_volume_every -> setCurrentItem (0);
    c_volume_every -> setCurrentText ("");
  }
  c_volume_default -> setEnabled (resetChecked);
  c_volume_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_volume_default -> setFocus();
    c_volume_default -> selectAll();
  }
}

KPlayerSettingsContrast::KPlayerSettingsContrast (QWidget* parent, const char* name)
  : KPlayerSettingsContrastPage (parent, name)
{
  load();
}

void KPlayerSettingsContrast::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_contrast_minimum -> setText (QString::number (settings -> contrastMinimum()));
  c_contrast_maximum -> setText (QString::number (settings -> contrastMaximum()));
  c_contrast_marks -> setText (QString::number (settings -> contrastMarks()));
  c_contrast_step -> setText (QString::number (settings -> contrastStep()));
  c_contrast_reset -> setChecked (settings -> contrastReset());
  resetChanged (settings -> contrastReset());
}

void KPlayerSettingsContrast::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setContrastMinimumMaximum (c_contrast_minimum -> text().toLong(), c_contrast_maximum -> text().toLong());
  settings -> setContrastMarks (labs (c_contrast_marks -> text().toLong()));
  settings -> setContrastStep (labs (c_contrast_step -> text().toLong()));
  settings -> setContrastReset (c_contrast_reset -> isChecked());
  if ( settings -> contrastReset() )
  {
    settings -> setInitialContrast (c_contrast_default -> text().toLong());
    settings -> setContrastEvery (c_contrast_every -> currentItem());
  }
}

void KPlayerSettingsContrast::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    KPlayerSettings* settings = kPlayerSettings();
    c_contrast_default -> setText (QString::number (settings -> initialContrast()));
    c_contrast_every -> setCurrentItem (0);
    c_contrast_every -> setCurrentText (i18n("file"));
    c_contrast_every -> setCurrentItem (settings -> contrastEvery());
  }
  else
  {
    c_contrast_default -> setText ("");
    c_contrast_every -> setCurrentItem (0);
    c_contrast_every -> setCurrentText ("");
  }
  c_contrast_default -> setEnabled (resetChecked);
  c_contrast_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_contrast_default -> setFocus();
    c_contrast_default -> selectAll();
  }
}

KPlayerSettingsBrightness::KPlayerSettingsBrightness (QWidget* parent, const char* name)
  : KPlayerSettingsBrightnessPage (parent, name)
{
  load();
}

void KPlayerSettingsBrightness::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_brightness_minimum -> setText (QString::number (settings -> brightnessMinimum()));
  c_brightness_maximum -> setText (QString::number (settings -> brightnessMaximum()));
  c_brightness_marks -> setText (QString::number (settings -> brightnessMarks()));
  c_brightness_step -> setText (QString::number (settings -> brightnessStep()));
  c_brightness_reset -> setChecked (settings -> brightnessReset());
  resetChanged (settings -> brightnessReset());
}

void KPlayerSettingsBrightness::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setBrightnessMinimumMaximum (c_brightness_minimum -> text().toLong(), c_brightness_maximum -> text().toLong());
  settings -> setBrightnessMarks (labs (c_brightness_marks -> text().toLong()));
  settings -> setBrightnessStep (labs (c_brightness_step -> text().toLong()));
  settings -> setBrightnessReset (c_brightness_reset -> isChecked());
  if ( settings -> brightnessReset() )
  {
    settings -> setInitialBrightness (c_brightness_default -> text().toLong());
    settings -> setBrightnessEvery (c_brightness_every -> currentItem());
  }
}

void KPlayerSettingsBrightness::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    KPlayerSettings* settings = kPlayerSettings();
    c_brightness_default -> setText (QString::number (settings -> initialBrightness()));
    c_brightness_every -> setCurrentItem (0);
    c_brightness_every -> setCurrentText (i18n("file"));
    c_brightness_every -> setCurrentItem (settings -> brightnessEvery());
  }
  else
  {
    c_brightness_default -> setText ("");
    c_brightness_every -> setCurrentItem (0);
    c_brightness_every -> setCurrentText ("");
  }
  c_brightness_default -> setEnabled (resetChecked);
  c_brightness_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_brightness_default -> setFocus();
    c_brightness_default -> selectAll();
  }
}

KPlayerSettingsHue::KPlayerSettingsHue (QWidget* parent, const char* name)
  : KPlayerSettingsHuePage (parent, name)
{
  load();
}

void KPlayerSettingsHue::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_hue_minimum -> setText (QString::number (settings -> hueMinimum()));
  c_hue_maximum -> setText (QString::number (settings -> hueMaximum()));
  c_hue_marks -> setText (QString::number (settings -> hueMarks()));
  c_hue_step -> setText (QString::number (settings -> hueStep()));
  c_hue_reset -> setChecked (settings -> hueReset());
  resetChanged (settings -> hueReset());
}

void KPlayerSettingsHue::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setHueMinimumMaximum (c_hue_minimum -> text().toLong(), c_hue_maximum -> text().toLong());
  settings -> setHueMarks (labs (c_hue_marks -> text().toLong()));
  settings -> setHueStep (labs (c_hue_step -> text().toLong()));
  settings -> setHueReset (c_hue_reset -> isChecked());
  if ( settings -> hueReset() )
  {
    settings -> setInitialHue (c_hue_default -> text().toLong());
    settings -> setHueEvery (c_hue_every -> currentItem());
  }
}

void KPlayerSettingsHue::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    KPlayerSettings* settings = kPlayerSettings();
    c_hue_default -> setText (QString::number (settings -> initialHue()));
    c_hue_every -> setCurrentItem (0);
    c_hue_every -> setCurrentText (i18n("file"));
    c_hue_every -> setCurrentItem (settings -> hueEvery());
  }
  else
  {
    c_hue_default -> setText ("");
    c_hue_every -> setCurrentItem (0);
    c_hue_every -> setCurrentText ("");
  }
  c_hue_default -> setEnabled (resetChecked);
  c_hue_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_hue_default -> setFocus();
    c_hue_default -> selectAll();
  }
}

KPlayerSettingsSaturation::KPlayerSettingsSaturation (QWidget* parent, const char* name)
  : KPlayerSettingsSaturationPage (parent, name)
{
  load();
}

void KPlayerSettingsSaturation::load (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  c_saturation_minimum -> setText (QString::number (settings -> saturationMinimum()));
  c_saturation_maximum -> setText (QString::number (settings -> saturationMaximum()));
  c_saturation_marks -> setText (QString::number (settings -> saturationMarks()));
  c_saturation_step -> setText (QString::number (settings -> saturationStep()));
  c_saturation_reset -> setChecked (settings -> saturationReset());
  resetChanged (settings -> saturationReset());
}

void KPlayerSettingsSaturation::save (void)
{
  KPlayerSettings* settings = kPlayerSettings();
  settings -> setSaturationMinimumMaximum (c_saturation_minimum -> text().toLong(), c_saturation_maximum -> text().toLong());
  settings -> setSaturationMarks (labs (c_saturation_marks -> text().toLong()));
  settings -> setSaturationStep (labs (c_saturation_step -> text().toLong()));
  settings -> setSaturationReset (c_saturation_reset -> isChecked());
  if ( settings -> saturationReset() )
  {
    settings -> setInitialSaturation (c_saturation_default -> text().toLong());
    settings -> setSaturationEvery (c_saturation_every -> currentItem());
  }
}

void KPlayerSettingsSaturation::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    KPlayerSettings* settings = kPlayerSettings();
    c_saturation_default -> setText (QString::number (settings -> initialSaturation()));
    c_saturation_every -> setCurrentItem (0);
    c_saturation_every -> setCurrentText (i18n("file"));
    c_saturation_every -> setCurrentItem (settings -> saturationEvery());
  }
  else
  {
    c_saturation_default -> setText ("");
    c_saturation_every -> setCurrentItem (0);
    c_saturation_every -> setCurrentText ("");
  }
  c_saturation_default -> setEnabled (resetChecked);
  c_saturation_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_saturation_default -> setFocus();
    c_saturation_default -> selectAll();
  }
}
