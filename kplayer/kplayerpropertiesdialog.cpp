/***************************************************************************
                         kplayerpropertiesdialog.cpp
                         ---------------------------
    begin                : Tue Mar 02 2004
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
#include <stdlib.h>

#include <kdebug.h>

#include "kplayerpropertiesdialog.h"
#include "kplayerpropertiesdialog.moc"
#include "kplayerengine.h"
#include "kplayerproperties.h"
#include "kplayersettings.h"

static QString s_default_entry (i18n("%1 (%2)"));
static QRegExp re_key_value ("^([^:]+): *(.*)$");

QString listEntry (QComboBox* combo, bool hasDefault = false)
{
  if ( hasDefault && combo -> currentItem() == 0 )
    return QString::null;
  else if ( combo -> currentItem() == 0 || hasDefault && combo -> currentItem() == 1 )
    return "";
  else if ( re_key_value.search (combo -> currentText()) >= 0 )
    return re_key_value.cap(1);
  return QString::null;
}

KPlayerPropertiesDialog::KPlayerPropertiesDialog (KPlayerProperties* properties)
  : KDialogBase (TreeList, i18n("File Properties"), Help | Default | Ok | Apply | Cancel, Ok)
{
  m_properties = properties;
  QApplication::connect (this, SIGNAL (aboutToShowPage (QWidget*)), this, SLOT (pageAboutToShow(QWidget*)));
  QFrame* frame = addPage (i18n("General"), i18n("General Properties"));
  QVBoxLayout* layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_general = new KPlayerPropertiesGeneral (m_properties, frame, "general");
  frame = addPage (i18n("Video"), i18n("Video Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_video = new KPlayerPropertiesVideo (m_properties, frame, "video");
  frame = addPage (i18n("Audio"), i18n("Audio Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_audio = new KPlayerPropertiesAudio (m_properties, frame, "audio");
  frame = addPage (i18n("Subtitles"), i18n("Subtitle Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_subtitles = new KPlayerPropertiesSubtitles (m_properties, frame, "subtitles");
  frame = addPage (i18n("Advanced"), i18n("Advanced Properties"));
  layout = new QVBoxLayout (frame, 0, 0);
  layout -> setAutoAdd (true);
  m_advanced = new KPlayerPropertiesAdvanced (m_properties, frame, "advanced");
  setHelp ("properties");
  KListView* view = (KListView*) child (0, "KListView");
  if ( view )
    view -> setMinimumSize (view -> sizeHint());
  KConfig* config = kPlayerConfig();
  config -> setGroup ("General Options");
  QString name (config -> readEntry ("Properties Dialog Page"));
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
/*kdDebugTime() << "KPFP position " << x << "x" << y << " size " << w << "x" << h << "\n";
  if ( x >= 0 && y >= 0 )
    move (x, y);*/
  if ( w > 0 && h > 0 )
    resize (w, h);
}

KPlayerPropertiesDialog::~KPlayerPropertiesDialog (void)
{
  KConfig* config = kPlayerConfig();
  config -> setGroup ("General Options");
  kdDebugTime() << "KPFP " << x() << "x" << y() << " " << width() << "x" << height() << " Hint " << sizeHint().width() << "x" << sizeHint().height() << "\n";
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
}

void KPlayerPropertiesDialog::slotDefault (void)
{
  if ( KMessageBox::warningYesNo (this, i18n("All file properties will be reset.\n\nAre you sure?"))
      != KMessageBox::Yes )
    return;
  m_properties -> defaults();
  m_properties -> save();
  m_general -> load();
  m_subtitles -> load();
  m_video -> load();
  m_audio -> load();
  m_advanced -> load();
  setButtonCancelText (i18n("&Close"));
  KDialogBase::slotDefault();
}

void KPlayerPropertiesDialog::pageAboutToShow (QWidget* page)
{
  /*setHelp (page == m_general -> parent() ? "properties-general"
    : page == m_subtitles -> parent() ? "properties-subtitles"
    : page == m_audio -> parent() ? "properties-audio"
    : page == m_video -> parent() ? "properties-video"
    : page == m_advanced -> parent() ? "properties-advanced" : "properties");*/
  QObject* object = page -> child (0, "QFrame");
  KConfig* config = kPlayerConfig();
  config -> setGroup ("General Options");
  QString name;
  if ( object )
    name = object -> name ("");
  if ( name.isEmpty() )
    config -> deleteEntry ("Properties Dialog Page");
  else
    config -> writeEntry ("Properties Dialog Page", name);
  setHelp (name.isEmpty() ? "properties" : "properties-" + name);
}

void KPlayerPropertiesDialog::slotOk (void)
{
  slotApply();
  KDialogBase::slotOk();
}

void KPlayerPropertiesDialog::slotApply (void)
{
  m_general -> save();
  m_subtitles -> save();
  m_audio -> save();
  m_video -> save();
  m_advanced -> save();
  m_properties -> save();
  setButtonCancelText (i18n("&Close"));
  KDialogBase::slotApply();
}

KPlayerPropertiesGeneral::KPlayerPropertiesGeneral (KPlayerProperties* properties, QWidget* parent, const char* name)
  : KPlayerPropertiesGeneralPage (parent, name),
    m_properties (properties)
{
  c_url -> setReadOnly (true);
  c_length -> setReadOnly (true);
  c_original_width -> setReadOnly (true);
  c_original_height -> setReadOnly (true);
  load();
}

void KPlayerPropertiesGeneral::load (void)
{
  c_url -> setText (m_properties -> url().isLocalFile() ? m_properties -> url().path() : m_properties -> url().prettyURL());
  c_name -> setText (m_properties -> name());
  c_playlist -> setCurrentItem (m_properties -> playlistOption());
  c_length -> setText (timeString (m_properties -> length()));
  c_original_width -> setText (m_properties -> originalSize().isEmpty() ? "" : QString::number (m_properties -> originalSize().width()));
  c_original_height -> setText (m_properties -> originalSize().isEmpty() ? "" : QString::number (m_properties -> originalSize().height()));
  c_display_size -> setCurrentItem (m_properties -> displaySizeOption());
  displaySizeChanged (c_display_size -> currentItem());
  c_maintain_aspect -> setCurrentItem (m_properties -> maintainAspectOption() + 1);
}

void KPlayerPropertiesGeneral::save (void)
{
  m_properties -> setName (c_name -> text());
  m_properties -> setPlaylistOption (c_playlist -> currentItem());
  m_properties -> setDisplaySizeOption (c_display_size -> currentItem());
  if ( m_properties -> displaySizeOption() == 1 || m_properties -> displaySizeOption() == 2 )
    m_properties -> setDisplaySizeValue (QSize (labs (c_display_width -> text().toLong()), labs (c_display_height -> text().toLong())));
  m_properties -> setMaintainAspectOption (c_maintain_aspect -> currentItem() - 1);
}

void KPlayerPropertiesGeneral::displaySizeChanged (int option)
{
  if ( option == 0 || ! m_properties -> displaySizeValue().isValid() )
  {
    c_display_width -> setText ("");
    c_display_height -> setText ("");
  }
  else
  {
    c_display_width -> setText (QString::number (m_properties -> displaySizeValue().width()));
    c_display_height -> setText (QString::number (m_properties -> displaySizeValue().height()));
  }
  c_display_width -> setEnabled (option == 1 || option == 2);
  l_display_by -> setEnabled (option == 1 || option == 2);
  c_display_height -> setEnabled (option == 1 || option == 2);
  if ( (option == 1 || option == 2) && sender() )
  {
    c_display_width -> setFocus();
    c_display_width -> selectAll();
  }
}

KPlayerPropertiesSubtitles::KPlayerPropertiesSubtitles (KPlayerProperties* properties, QWidget* parent, const char* name)
  : KPlayerPropertiesSubtitlesPage (parent, name),
    m_properties (properties)
{
  load();
  if ( ! m_properties -> url().isLocalFile() )
    c_autoload -> setEnabled (false);
}

void KPlayerPropertiesSubtitles::load (void)
{
  c_autoload -> setCurrentItem (m_properties -> subtitleAutoloadOption() + 1);
  autoloadChanged (c_autoload -> currentItem());
  c_visibility -> setCurrentItem (m_properties -> subtitleVisibilityOption() + 1);
  c_position_set -> setCurrentItem (m_properties -> subtitlePositionOption() + 1);
  positionChanged (c_position_set -> currentItem());
  c_delay_set -> setCurrentItem (m_properties -> subtitleDelayOption() + 1);
  delayChanged (c_delay_set -> currentItem());
}

void KPlayerPropertiesSubtitles::save (void)
{
  m_properties -> setSubtitleAutoloadOption (c_autoload -> currentItem() - 1);
  if ( m_properties -> subtitleAutoloadOption() == 1 )
    m_properties -> setSubtitleUrl (c_url -> text());
  m_properties -> setSubtitleVisibilityOption (c_visibility -> currentItem() - 1);
  m_properties -> setSubtitlePositionOption (c_position_set -> currentItem() - 1);
  if ( m_properties -> subtitlePositionOption() != -1 )
    m_properties -> setSubtitlePositionValue (labs (c_position -> text().toLong()));
  m_properties -> setSubtitleDelayOption (c_delay_set -> currentItem() - 1);
  if ( m_properties -> subtitleDelayOption() != -1 )
    m_properties -> setSubtitleDelayValue (c_delay -> text().toFloat());
}

void KPlayerPropertiesSubtitles::autoloadChanged (int option)
{
  c_url -> setText (option < 2 || m_properties -> subtitleUrl().isEmpty() ? ""
    : m_properties -> subtitleUrl().isLocalFile() ? m_properties -> subtitleUrl().path()
    : m_properties -> subtitleUrl().url());
  c_url -> setEnabled (option == 2);
}

void KPlayerPropertiesSubtitles::positionChanged (int option)
{
  c_position -> setText (option > 0 ? QString::number (m_properties -> subtitlePositionValue()) : "");
  c_position -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_position -> setFocus();
    c_position -> selectAll();
  }
}

void KPlayerPropertiesSubtitles::delayChanged (int option)
{
  c_delay -> setText (option > 0 ? QString::number (m_properties -> subtitleDelayValue()) : "");
  c_delay -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_delay -> setFocus();
    c_delay -> selectAll();
  }
}

KPlayerPropertiesAudio::KPlayerPropertiesAudio (KPlayerProperties* properties, QWidget* parent, const char* name)
  : KPlayerPropertiesAudioPage (parent, name),
    m_properties (properties)
{
  if ( m_properties -> audioCodecOption().isNull() )
    m_default_codec = m_properties -> audioCodecValue();
  loadLists();
  load();
  c_bitrate -> setReadOnly (true);
}

void KPlayerPropertiesAudio::load (void)
{
  c_volume_set -> setCurrentItem (m_properties -> volumeOption() + 1);
  volumeChanged (c_volume_set -> currentItem());
  c_delay_set -> setCurrentItem (m_properties -> audioDelayOption() + 1);
  delayChanged (c_delay_set -> currentItem());
  c_codec -> setCurrentItem (m_properties -> audioCodecOption().isNull() ? 0
    : kPlayerEngine() -> audioCodecIndex (m_properties -> audioCodecOption()) + 2);
  codecChanged (c_codec -> currentItem());
  if ( m_properties -> audioBitrate() > 0 )
    c_bitrate -> setText (QString::number (m_properties -> audioBitrate()));
}

void KPlayerPropertiesAudio::save (void)
{
  m_properties -> setVolumeOption (c_volume_set -> currentItem() - 1);
  if ( m_properties -> volumeOption() != -1 )
    m_properties -> setVolumeValue (labs (c_volume -> text().toLong()));
  m_properties -> setAudioDelayOption (c_delay_set -> currentItem() - 1);
  if ( m_properties -> audioDelayOption() != -1 )
    m_properties -> setAudioDelayValue (c_delay -> text().toFloat());
  m_properties -> setAudioCodecOption (listEntry (c_codec, true));
  if ( c_codec -> currentItem() != 1 )
    m_properties -> setAudioCodecFallbackOption (c_fallback -> currentItem() - 1);
}

void KPlayerPropertiesAudio::loadLists (void)
{
  if ( kPlayerEngine() -> audioCodecCount() )
  {
    c_codec -> clear();
    if ( m_default_codec.isEmpty() )
      c_codec -> insertItem (i18n("default"));
    else
      c_codec -> insertItem (s_default_entry.arg (i18n("default")).arg (m_default_codec));
    c_codec -> insertItem (i18n("auto"));
    for ( int i = 0; i < kPlayerEngine() -> audioCodecCount(); i ++ )
      c_codec -> insertItem (kPlayerEngine() -> audioCodecName (i));
  }
}

void KPlayerPropertiesAudio::volumeChanged (int option)
{
  c_volume -> setText (option > 0 ? QString::number (m_properties -> volumeValue()) : "");
  c_volume -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_volume -> setFocus();
    c_volume -> selectAll();
  }
}

void KPlayerPropertiesAudio::delayChanged (int option)
{
  c_delay -> setText (option > 0 ? QString::number (m_properties -> audioDelayValue()) : "");
  c_delay -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_delay -> setFocus();
    c_delay -> selectAll();
  }
}

void KPlayerPropertiesAudio::codecChanged (int index)
{
  c_fallback -> setCurrentItem (index == 1 ? 2 : m_properties -> audioCodecFallbackOption() + 1);
  c_fallback -> setEnabled (index != 1);
}

KPlayerPropertiesVideo::KPlayerPropertiesVideo (KPlayerProperties* properties, QWidget* parent, const char* name)
  : KPlayerPropertiesVideoPage (parent, name),
    m_properties (properties)
{
  if ( m_properties -> videoCodecOption().isNull() )
    m_default_codec = m_properties -> videoCodecValue();
  loadLists();
  load();
  c_bitrate -> setReadOnly (true);
  c_framerate -> setReadOnly (true);
}

void KPlayerPropertiesVideo::load (void)
{
  c_contrast_set -> setCurrentItem (m_properties -> contrastOption() + 1);
  contrastChanged (c_contrast_set -> currentItem());
  c_brightness_set -> setCurrentItem (m_properties -> brightnessOption() + 1);
  brightnessChanged (c_brightness_set -> currentItem());
  c_hue_set -> setCurrentItem (m_properties -> hueOption() + 1);
  hueChanged (c_hue_set -> currentItem());
  c_saturation_set -> setCurrentItem (m_properties -> saturationOption() + 1);
  saturationChanged (c_saturation_set -> currentItem());
  c_codec -> setCurrentItem (m_properties -> videoCodecOption().isNull() ? 0
    : kPlayerEngine() -> videoCodecIndex (m_properties -> videoCodecOption()) + 2);
  codecChanged (c_codec -> currentItem());
  if ( m_properties -> videoBitrate() > 0 )
    c_bitrate -> setText (QString::number (m_properties -> videoBitrate()));
  if ( m_properties -> framerate() > 0 )
    c_framerate -> setText (QString::number (m_properties -> framerate()));
}

void KPlayerPropertiesVideo::save (void)
{
  m_properties -> setContrastOption (c_contrast_set -> currentItem() - 1);
  if ( m_properties -> contrastOption() != -1 )
    m_properties -> setContrastValue (c_contrast -> text().toLong());
  m_properties -> setBrightnessOption (c_brightness_set -> currentItem() - 1);
  if ( m_properties -> brightnessOption() != -1 )
    m_properties -> setBrightnessValue (c_brightness -> text().toLong());
  m_properties -> setHueOption (c_hue_set -> currentItem() - 1);
  if ( m_properties -> hueOption() != -1 )
    m_properties -> setHueValue (c_hue -> text().toLong());
  m_properties -> setSaturationOption (c_saturation_set -> currentItem() - 1);
  if ( m_properties -> saturationOption() != -1 )
    m_properties -> setSaturationValue (c_saturation -> text().toLong());
  m_properties -> setVideoCodecOption (listEntry (c_codec, true));
  if ( c_codec -> currentItem() != 1 )
    m_properties -> setVideoCodecFallbackOption (c_fallback -> currentItem() - 1);
}

void KPlayerPropertiesVideo::loadLists (void)
{
  if ( kPlayerEngine() -> videoCodecCount() )
  {
    c_codec -> clear();
    if ( m_default_codec.isEmpty() )
      c_codec -> insertItem (i18n("default"));
    else
      c_codec -> insertItem (s_default_entry.arg (i18n("default")).arg (m_default_codec));
    c_codec -> insertItem (i18n("auto"));
    for ( int i = 0; i < kPlayerEngine() -> videoCodecCount(); i ++ )
      c_codec -> insertItem (kPlayerEngine() -> videoCodecName (i));
  }
}

void KPlayerPropertiesVideo::contrastChanged (int option)
{
  c_contrast -> setText (option > 0 ? QString::number (m_properties -> contrastValue()) : "");
  c_contrast -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_contrast -> setFocus();
    c_contrast -> selectAll();
  }
}

void KPlayerPropertiesVideo::brightnessChanged (int option)
{
  c_brightness -> setText (option > 0 ? QString::number (m_properties -> brightnessValue()) : "");
  c_brightness -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_brightness -> setFocus();
    c_brightness -> selectAll();
  }
}

void KPlayerPropertiesVideo::hueChanged (int option)
{
  c_hue -> setText (option > 0 ? QString::number (m_properties -> hueValue()) : "");
  c_hue -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_hue -> setFocus();
    c_hue -> selectAll();
  }
}

void KPlayerPropertiesVideo::saturationChanged (int option)
{
  c_saturation -> setText (option > 0 ? QString::number (m_properties -> saturationValue()) : "");
  c_saturation -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_saturation -> setFocus();
    c_saturation -> selectAll();
  }
}

void KPlayerPropertiesVideo::codecChanged (int index)
{
  c_fallback -> setCurrentItem (index == 1 ? 2 : m_properties -> videoCodecFallbackOption() + 1);
  c_fallback -> setEnabled (index != 1);
}

KPlayerPropertiesAdvanced::KPlayerPropertiesAdvanced (KPlayerProperties* properties, QWidget* parent, const char* name)
  : KPlayerPropertiesAdvancedPage (parent, name),
    m_properties (properties)
{
  load();
}

void KPlayerPropertiesAdvanced::load (void)
{
  c_command_line_option -> setCurrentItem (m_properties -> commandLineOption() + 1);
  commandLineChanged (c_command_line_option -> currentItem());
  c_frame_drop -> setCurrentItem (m_properties -> frameDropOption() + 1);
  c_use_cache -> setCurrentItem (m_properties -> cacheOption() + 1);
  cacheChanged (c_use_cache -> currentItem());
  c_build_index -> setCurrentItem (m_properties -> buildNewIndexOption() + 1);
  c_use_kioslave -> setCurrentItem (m_properties -> useKioslaveOption());
  c_use_temporary_file -> setCurrentItem (m_properties -> useTemporaryFileOption() + 1);
}

void KPlayerPropertiesAdvanced::save (void)
{
  m_properties -> setCommandLineOption (c_command_line_option -> currentItem() - 1);
  if ( m_properties -> commandLineOption() != -1 )
    m_properties -> setCommandLineValue (c_command_line -> text());
  m_properties -> setFrameDropOption (c_frame_drop -> currentItem() - 1);
  m_properties -> setCacheOption (c_use_cache -> currentItem() - 1);
  if ( c_use_cache -> currentItem() == 3 )
    m_properties -> setCacheSizeValue (c_cache_size -> text().toLong());
  m_properties -> setBuildNewIndexOption (c_build_index -> currentItem() - 1);
  m_properties -> setUseKioslaveOption (c_use_kioslave -> currentItem());
  m_properties -> setUseTemporaryFileOption (c_use_temporary_file -> currentItem() - 1);
}

void KPlayerPropertiesAdvanced::commandLineChanged (int option)
{
  c_command_line -> setText (option > 0 ? m_properties -> commandLineValue() : "");
  c_command_line -> setEnabled (option > 0);
  if ( option > 0 && sender() )
  {
    c_command_line -> setFocus();
    c_command_line -> selectAll();
  }
}

void KPlayerPropertiesAdvanced::cacheChanged (int cache)
{
  if ( cache == 3 )
    c_cache_size -> setText (QString::number (m_properties -> cacheSizeValue()));
  else
    c_cache_size -> setText ("");
  c_cache_size -> setEnabled (cache == 3);
  l_cache_size_kb -> setEnabled (cache == 3);
  if ( cache == 3 && sender() )
  {
    c_cache_size -> setFocus();
    c_cache_size -> selectAll();
  }
}
