/***************************************************************************
                          kplayerproperties.cpp
                          ---------------------
    begin                : Tue Feb 10 2004
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
#include <qfileinfo.h>
#include <qregexp.h>

#include <kdebug.h>

#include "kplayerproperties.h"
#include "kplayerproperties.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

#define DEBUG_KPLAYER_PROPERTIES

static QRegExp re_dvd_vcd ("^(?:vcd|dvd|cdda|cddb)://", false);
static QRegExp re_dvb ("^dvb://([^/]+)$", false);

KPlayerProperties::KPlayerProperties (const KURL& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating properties\n";
#endif
  m_references = 1;
  setUrl (url);
  reset();
  defaults();
  load();
}

KPlayerProperties::~KPlayerProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying properties\n";
#endif
}

void KPlayerProperties::reset (void)
{
  setLength (0);
  setOriginalSize (QSize());
  setAudioCodecValue (QString::null);
  setAudioBitrate (0);
  setVideoCodecValue (QString::null);
  setVideoBitrate (0);
  setFramerate (0);
}

void KPlayerProperties::defaults (void)
{
  setName (QString::null);
  setPlaylistOption (0);
  setDisplaySizeOption (0);
  setDisplaySizeValue (QSize());
  setFullScreenOption (-1);
  setMaintainAspectOption (-1);
  setSubtitleAutoloadOption (url().isLocalFile() ? -1 : 1);
  setSubtitleUrl (KURL());
  setSubtitleVisibilityOption (-1);
  setSubtitlePositionOption (-1);
  setSubtitlePositionValue (100);
  setSubtitleDelayOption (-1);
  setSubtitleDelayValue (0);
  setVolumeOption (-1);
  setVolumeValue (50);
  setAudioDelayOption (-1);
  setAudioDelayValue (0);
  setAudioDriverOption (QString::null);
  setAudioDriverFallbackOption (-1);
  setAudioDeviceValue (QString::null);
  setAudioDeviceOption (-1);
  setAudioCodecOption (QString::null);
  setAudioCodecFallbackOption (-1);
  setContrastOption (-1);
  setContrastValue (0);
  setBrightnessOption (-1);
  setBrightnessValue (0);
  setHueOption (-1);
  setHueValue (0);
  setSaturationOption (-1);
  setSaturationValue (0);
  setVideoDriverOption (QString::null);
  setVideoDriverFallbackOption (-1);
  setVideoDeviceValue (QString::null);
  setVideoDeviceOption (-1);
  setVideoCodecOption (QString::null);
  setVideoCodecFallbackOption (-1);
  setVideoScalerOption (-1);
  setVideoDoubleBufferingOption (-1);
  setVideoDirectRenderingOption (-1);
  setExecutablePathValue (QString::null);
  setExecutablePathOption (-1);
  setCommandLineValue ("");
  setCommandLineOption (-1);
  setFrameDropOption (-1);
  setCacheOption (-1);
  setCacheSizeValue (1024);
  setBuildNewIndexOption (-1);
  setUseKioslaveOption (0);
  setUseTemporaryFileOption (-1);
  setOsdLevelOption (-1);
}

void KPlayerProperties::load (void)
{
  KConfig* config = kPlayerPlaylistConfig();
  config -> setGroup (url().url());
  setName (config -> readEntry ("Name", m_name));
  if ( re_dvd_vcd.search (url().url()) < 0 )
  {
    setLength (config -> readDoubleNumEntry ("Length", length()));
    setOriginalSize (config -> readSizeEntry ("Video Size", &m_original_size));
    setVideoCodecValue (config -> readEntry ("Video Codec", videoCodecValue()));
    setVideoBitrate (config -> readNumEntry ("Video Bitrate", videoBitrate()));
    setAudioCodecValue (config -> readEntry ("Audio Codec", audioCodecValue()));
    setAudioBitrate (config -> readNumEntry ("Audio Bitrate", audioBitrate()));
    setFramerate (config -> readDoubleNumEntry ("Framerate", framerate()));
  }
  else
    reset();
  setPlaylistOption (config -> readNumEntry ("Playlist", playlistOption()));
  setDisplaySizeValue (config -> readSizeEntry ("Display Size"));
  setDisplaySizeOption (displaySizeValue().isValid() ? config -> readNumEntry ("Display Size Option", displaySizeOption()) : 0);
  setFullScreenOption (config -> readNumEntry ("Full Screen", fullScreenOption()));
  setMaintainAspectOption (config -> readNumEntry ("Maintain Aspect", maintainAspectOption()));
  setSubtitleUrl (config -> readEntry ("Subtitle URL", subtitleUrl().url()));
  if ( ! subtitleUrl().isEmpty() || ! url().isLocalFile() )
    setSubtitleAutoloadOption (1);
  if ( url().isLocalFile() )
    setSubtitleAutoloadOption (config -> readNumEntry ("Autoload Subtitles", subtitleAutoloadOption()));
  setSubtitleVisibilityOption (config -> readNumEntry ("Subtitle Visibility", subtitleVisibilityOption()));
  int numentry = config -> readNumEntry ("Subtitle Position", INT_MIN);
  setSubtitlePositionOption (numentry == INT_MIN ? -1 : 0);
  if ( numentry != INT_MIN )
    setSubtitlePositionValue (numentry);
  float fnumentry = config -> readDoubleNumEntry ("Subtitle Delay", INT_MIN);
  setSubtitleDelayOption (fnumentry < INT_MIN / 2 ? -1 : 0);
  if ( fnumentry > INT_MIN / 2 )
    setSubtitleDelayValue (fnumentry);
  numentry = config -> readNumEntry ("Volume", INT_MIN);
  setVolumeOption (numentry == INT_MIN ? -1 : config -> readNumEntry ("Volume Option", volumeOption()));
  if ( numentry != INT_MIN )
    setVolumeValue (numentry);
  fnumentry = config -> readDoubleNumEntry ("Audio Delay", INT_MIN);
  setAudioDelayOption (fnumentry < INT_MIN / 2 ? -1 : 0);
  if ( fnumentry > INT_MIN / 2 )
    setAudioDelayValue (fnumentry);
  numentry = config -> readNumEntry ("Contrast", INT_MIN);
  setContrastOption (numentry == INT_MIN ? -1 : config -> readNumEntry ("Contrast Option", contrastOption()));
  if ( numentry != INT_MIN )
    setContrastValue (numentry);
  numentry = config -> readNumEntry ("Brightness", INT_MIN);
  setBrightnessOption (numentry == INT_MIN ? -1 : config -> readNumEntry ("Brightness Option", brightnessOption()));
  if ( numentry != INT_MIN )
    setBrightnessValue (numentry);
  numentry = config -> readNumEntry ("Hue", INT_MIN);
  setHueOption (numentry == INT_MIN ? -1 : config -> readNumEntry ("Hue Option", hueOption()));
  if ( numentry != INT_MIN )
    setHueValue (numentry);
  numentry = config -> readNumEntry ("Saturation", INT_MIN);
  setSaturationOption (numentry == INT_MIN ? -1 : config -> readNumEntry ("Saturation Option", saturationOption()));
  if ( numentry != INT_MIN )
    setSaturationValue (numentry);
  setVideoDriverValue (config -> readEntry ("Video Driver", videoDriverValue()));
  setVideoDriverOption (config -> readEntry ("Video Driver Option", videoDriverOption()));
  setVideoDriverFallbackOption (config -> readNumEntry ("Video Driver Fallback", videoDriverFallbackOption()));
  setVideoDeviceValue (config -> readEntry ("Video Device", videoDeviceValue()));
  setVideoDeviceOption (videoDeviceValue().isNull() ? -1 : 0);
  setVideoCodecOption (config -> readEntry ("Video Codec Option", videoCodecOption()));
  setVideoCodecFallbackOption (config -> readNumEntry ("Video Codec Fallback", videoCodecFallbackOption()));
  setVideoScalerOption (config -> readNumEntry ("Video Scaler", videoScalerOption()));
  setVideoDoubleBufferingOption (config -> readNumEntry ("Double Buffering", videoDoubleBufferingOption()));
  setVideoDirectRenderingOption (config -> readNumEntry ("Direct Rendering", videoDirectRenderingOption()));
  setAudioDriverValue (config -> readEntry ("Audio Driver", audioDriverValue()));
  setAudioDriverOption (config -> readEntry ("Audio Driver Option", audioDriverOption()));
  setAudioDriverFallbackOption (config -> readNumEntry ("Audio Driver Fallback", audioDriverFallbackOption()));
  setAudioDeviceValue (config -> readEntry ("Audio Device", audioDeviceValue()));
  setAudioDeviceOption (audioDeviceValue().isNull() ? -1 : 0);
  setAudioCodecOption (config -> readEntry ("Audio Codec Option", audioCodecOption()));
  setAudioCodecFallbackOption (config -> readNumEntry ("Audio Codec Fallback", audioCodecFallbackOption()));
  setExecutablePathValue (config -> readEntry ("Executable Path", m_executable_path));
  setExecutablePathOption (m_executable_path.isNull() ? -1 : 0);
  setCommandLineValue (config -> readEntry ("Command Line", commandLineValue()));
  setCommandLineOption (config -> readNumEntry ("Command Line Option", commandLineOption()));
  setFrameDropOption (config -> readNumEntry ("Frame Dropping", frameDropOption()));
  setCacheOption (config -> readNumEntry ("Cache", cacheOption()));
  setCacheSizeValue (config -> readUnsignedNumEntry ("Cache Size", cacheSizeValue()));
  setBuildNewIndexOption (config -> readNumEntry ("Build New Index", buildNewIndexOption()));
  setUseKioslaveOption (config -> readNumEntry ("Use KIOSlave", useKioslaveOption()));
  setUseTemporaryFileOption (config -> readNumEntry ("Use Temporary File For KIOSlave", useTemporaryFileOption()));
  setOsdLevelOption (config -> readNumEntry ("OSD Level", osdLevelOption()));
}

void KPlayerProperties::save (void) const
{
  KConfig* config = kPlayerPlaylistConfig();
  config -> setGroup (url().url());
  if ( name() == defaultName() )
    config -> deleteEntry ("Name");
  else
    config -> writeEntry ("Name", name());
  config -> writeEntry ("Date", QDateTime::currentDateTime());
  if ( re_dvd_vcd.search (url().url()) < 0 )
  {
    if ( length() == 0 )
      config -> deleteEntry ("Length");
    else
      config -> writeEntry ("Length", length());
    if ( ! originalSize().isValid() )
      config -> deleteEntry ("Video Size");
    else
      config -> writeEntry ("Video Size", originalSize());
    if ( videoDriverValue().isEmpty() )
      config -> deleteEntry ("Video Driver");
    else
      config -> writeEntry ("Video Driver", videoDriverValue());
    if ( videoCodecValue().isEmpty() )
      config -> deleteEntry ("Video Codec");
    else
      config -> writeEntry ("Video Codec", videoCodecValue());
    if ( videoBitrate() == 0 )
      config -> deleteEntry ("Video Bitrate");
    else
      config -> writeEntry ("Video Bitrate", videoBitrate());
    if ( audioDriverValue().isEmpty() )
      config -> deleteEntry ("Audio Driver");
    else
      config -> writeEntry ("Audio Driver", audioDriverValue());
    if ( audioCodecValue().isEmpty() )
      config -> deleteEntry ("Audio Codec");
    else
      config -> writeEntry ("Audio Codec", audioCodecValue());
    if ( audioBitrate() == 0 )
      config -> deleteEntry ("Audio Bitrate");
    else
      config -> writeEntry ("Audio Bitrate", audioBitrate());
    if ( framerate() == 0 )
      config -> deleteEntry ("Framerate");
    else
      config -> writeEntry ("Framerate", framerate());
    if ( subtitleUrl().isEmpty() || subtitleAutoloadOption() != 1 )
      config -> deleteEntry ("Subtitle URL");
    else
      config -> writeEntry ("Subtitle URL", subtitleUrl().url());
    if ( subtitleDelayOption() == -1 )
      config -> deleteEntry ("Subtitle Delay");
    else
      config -> writeEntry ("Subtitle Delay", subtitleDelayValue());
    if ( audioDelayOption() == -1 )
      config -> deleteEntry ("Audio Delay");
    else
      config -> writeEntry ("Audio Delay", audioDelayValue());
  }
  if ( playlistOption() == 0 )
    config -> deleteEntry ("Playlist");
  else
    config -> writeEntry ("Playlist", playlistOption());
  if ( displaySizeOption() == 0 || ! displaySizeValue().isValid() )
    config -> deleteEntry ("Display Size Option");
  else
    config -> writeEntry ("Display Size Option", displaySizeOption());
  if ( displaySizeOption() == 0 || ! displaySizeValue().isValid() )
    config -> deleteEntry ("Display Size");
  else
    config -> writeEntry ("Display Size", displaySizeValue());
  if ( fullScreenOption() == -1 )
    config -> deleteEntry ("Full Screen");
  else
    config -> writeEntry ("Full Screen", fullScreenOption());
  if ( maintainAspectOption() == -1 )
    config -> deleteEntry ("Maintain Aspect");
  else
    config -> writeEntry ("Maintain Aspect", maintainAspectOption());
  if ( url().isLocalFile() && (subtitleUrl().isEmpty() || subtitleAutoloadOption() != 1) && subtitleAutoloadOption() != -1 )
    config -> writeEntry ("Autoload Subtitles", subtitleAutoloadOption());
  else
    config -> deleteEntry ("Autoload Subtitles");
  if ( subtitleVisibilityOption() == -1 )
    config -> deleteEntry ("Subtitle Visibility");
  else
    config -> writeEntry ("Subtitle Visibility", subtitleVisibilityOption());
  if ( subtitlePositionOption() == -1 )
    config -> deleteEntry ("Subtitle Position");
  else
    config -> writeEntry ("Subtitle Position", subtitlePositionValue());
  if ( volumeOption() == -1 )
    config -> deleteEntry ("Volume");
  else
    config -> writeEntry ("Volume", volumeValue());
  if ( volumeOption() == -1 )
    config -> deleteEntry ("Volume Option");
  else
    config -> writeEntry ("Volume Option", volumeOption());
  if ( contrastOption() == -1 )
    config -> deleteEntry ("Contrast");
  else
    config -> writeEntry ("Contrast", contrastValue());
  if ( contrastOption() == -1 )
    config -> deleteEntry ("Contrast Option");
  else
    config -> writeEntry ("Contrast Option", contrastOption());
  if ( brightnessOption() == -1 )
    config -> deleteEntry ("Brightness");
  else
    config -> writeEntry ("Brightness", brightnessValue());
  if ( brightnessOption() == -1 )
    config -> deleteEntry ("Brightness Option");
  else
    config -> writeEntry ("Brightness Option", brightnessOption());
  if ( hueOption() == -1 )
    config -> deleteEntry ("Hue");
  else
    config -> writeEntry ("Hue", hueValue());
  if ( hueOption() == -1 )
    config -> deleteEntry ("Hue Option");
  else
    config -> writeEntry ("Hue Option", hueOption());
  if ( saturationOption() == -1 )
    config -> deleteEntry ("Saturation");
  else
    config -> writeEntry ("Saturation", saturationValue());
  if ( saturationOption() == -1 )
    config -> deleteEntry ("Saturation Option");
  else
    config -> writeEntry ("Saturation Option", saturationOption());
  if ( videoDriverOption().isNull() )
    config -> deleteEntry ("Video Driver Option");
  else
    config -> writeEntry ("Video Driver Option", videoDriverOption());
  if ( videoDriverFallbackOption() == -1 )
    config -> deleteEntry ("Video Driver Fallback");
  else
    config -> writeEntry ("Video Driver Fallback", videoDriverFallbackOption());
  if ( videoDeviceOption() == -1 )
    config -> deleteEntry ("Video Device");
  else
    config -> writeEntry ("Video Device", videoDeviceValue());
  if ( videoCodecOption().isNull() )
    config -> deleteEntry ("Video Codec Option");
  else
    config -> writeEntry ("Video Codec Option", videoCodecOption());
  if ( videoCodecFallbackOption() == -1 )
    config -> deleteEntry ("Video Codec Fallback");
  else
    config -> writeEntry ("Video Codec Fallback", videoCodecFallbackOption());
  if ( videoScalerOption() == -1 )
    config -> deleteEntry ("Video Scaler");
  else
    config -> writeEntry ("Video Scaler", videoScalerOption());
  if ( videoDoubleBufferingOption() == -1 )
    config -> deleteEntry ("Double Buffering");
  else
    config -> writeEntry ("Double Buffering", videoDoubleBufferingOption());
  if ( videoDirectRenderingOption() == -1 )
    config -> deleteEntry ("Direct Rendering");
  else
    config -> writeEntry ("Direct Rendering", videoDirectRenderingOption());
  if ( audioDriverOption().isNull() )
    config -> deleteEntry ("Audio Driver Option");
  else
    config -> writeEntry ("Audio Driver Option", audioDriverOption());
  if ( audioDriverFallbackOption() == -1 )
    config -> deleteEntry ("Audio Driver Fallback");
  else
    config -> writeEntry ("Audio Driver Fallback", audioDriverFallbackOption());
  if ( audioDeviceOption() == -1 )
    config -> deleteEntry ("Audio Device");
  else
    config -> writeEntry ("Audio Device", audioDeviceValue());
  if ( audioCodecOption().isNull() )
    config -> deleteEntry ("Audio Codec Option");
  else
    config -> writeEntry ("Audio Codec Option", audioCodecOption());
  if ( audioCodecFallbackOption() == -1 )
    config -> deleteEntry ("Audio Codec Fallback");
  else
    config -> writeEntry ("Audio Codec Fallback", audioCodecFallbackOption());
  if ( executablePathOption() == -1 )
    config -> deleteEntry ("Executable Path");
  else
    config -> writeEntry ("Executable Path", executablePathValue());
  if ( commandLineOption() == -1 || commandLineValue().isEmpty() )
    config -> deleteEntry ("Command Line");
  else
    config -> writeEntry ("Command Line", commandLineValue());
  if ( commandLineOption() == -1 || commandLineOption() == 1 && commandLineValue().isEmpty() )
    config -> deleteEntry ("Command Line Option");
  else
    config -> writeEntry ("Command Line Option", commandLineOption());
  if ( frameDropOption() == -1 )
    config -> deleteEntry ("Frame Dropping");
  else
    config -> writeEntry ("Frame Dropping", frameDropOption());
  if ( cacheOption() == -1 )
    config -> deleteEntry ("Cache");
  else
    config -> writeEntry ("Cache", cacheOption());
  if ( cacheSizeValue() == 1024 )
    config -> deleteEntry ("Cache Size");
  else
    config -> writeEntry ("Cache Size", cacheSizeValue());
  if ( buildNewIndexOption() == -1 )
    config -> deleteEntry ("Build New Index");
  else
    config -> writeEntry ("Build New Index", buildNewIndexOption());
  if ( useKioslaveOption() == 0 )
    config -> deleteEntry ("Use KIOSlave");
  else
    config -> writeEntry ("Use KIOSlave", useKioslaveOption());
  if ( useTemporaryFileOption() == -1 )
    config -> deleteEntry ("Use Temporary File For KIOSlave");
  else
    config -> writeEntry ("Use Temporary File For KIOSlave", useTemporaryFileOption());
  if ( osdLevelOption() == -1 )
    config -> deleteEntry ("OSD Level");
  else
    config -> writeEntry ("OSD Level", osdLevelOption());
  config -> deleteEntry ("Selected");
  config -> sync();
  ((KPlayerProperties*) this) -> emitRefresh();
}

QString KPlayerProperties::defaultName (void) const
{
  if ( url().fileName().isEmpty() )
  {
    QString name (url().prettyURL());
    if ( re_dvb.search (name) >= 0 )
      return re_dvb.cap (1);
    return name;
  }
  QString s;
  s = QFileInfo (url().fileName()).baseName (true);
  return s.isEmpty() ? url().fileName() : s;
}

QString KPlayerProperties::type (void) const
{
  return QFileInfo (url().fileName()).extension(false).lower();
}

QString timeString (float l, bool zero_ok)
{
  l += 0.02;
  if ( ! zero_ok && l < 0.05 )
    return QString ("");
  int lHour = int (l) / 3600;
  if ( lHour > 0 )
    l -= lHour * 3600;
  int lMinute = int (l) / 60;
  if ( lMinute > 0 )
    l -= lMinute * 60;
  QString s;
  if ( lHour > 0 )
    s.sprintf ("%u:%02u:%04.1f", lHour, lMinute, l);
  else if ( lMinute > 0 )
    s.sprintf ("%u:%04.1f", lMinute, l);
  else
    s.sprintf ("%03.1f", l);
  return s;
}
