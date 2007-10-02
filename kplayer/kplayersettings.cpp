/***************************************************************************
                          kplayersettings.cpp
                          -------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kconfig.h>
#include <qfileinfo.h>
#include <qregexp.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_SETTINGS
#endif

#include "kplayersettings.h"
#include "kplayersettings.moc"
#include "kplayerengine.h"
#include "kplayerwidget.h"

KPlayerSettings::KPlayerSettings (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Creating settings\n";
#endif
  m_last_full_screen = false;
  setControl (false);
  setShift (false);
  m_properties = KPlayerMedia::trackProperties (KUrl());
}

KPlayerSettings::~KPlayerSettings()
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Destroying settings\n";
#endif
  if ( properties() -> url().isValid() )
    properties() -> commit();
  KPlayerMedia::release (properties());
}

void KPlayerSettings::load (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::load (" << url.prettyUrl() << ")\n";
#endif
  if ( url == properties() -> url() )
    return;
  bool hadAspect = properties() -> originalAspect().isValid() || ! properties() -> url().isValid();
  if ( properties() -> url().isValid() )
    properties() -> commit();
  KPlayerMedia::release (m_properties);
  m_properties = KPlayerMedia::trackProperties (url);
  setAspect (properties() -> currentAspect());
  if ( hadAspect || aspect().isValid() )
    setDisplaySizeOverride (false);
  if ( properties() -> displaySizeOption() == 1 )
    setDisplaySize (properties() -> displaySize());
  configuration() -> itemReset();
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Display size " << properties() -> displaySizeOption() << " " << properties() -> displaySize().width() << "x" << properties() -> displaySize().height() << " " << properties() -> currentSize().width() << "x" << properties() -> currentSize().height() << " -> " << displaySize().width() << "x" << displaySize().height() << "\n";
  kDebug() << "             Maintain aspect " << properties() -> maintainAspect() << " " << configuration() -> maintainAspect() << " -> " << maintainAspect() << " " << aspect().width() << "x" << aspect().height() << "\n";
  kDebug() << "             Volume " << properties() -> volume() << " " << properties() -> volumeOption() << " " << configuration() -> volume() << " " << configuration() -> mute() << " -> " << actualVolume() << "\n";
  kDebug() << "             Audio delay " << properties() -> audioDelay() << " " << configuration() -> audioDelay() << " -> " << audioDelay() << "\n";
  kDebug() << "             Contrast " << properties() -> contrast() << " " << properties() -> contrastOption() << " " << configuration() -> contrast() << " -> " << contrast() << "\n";
  kDebug() << "             Brightness " << properties() -> brightness() << " " << properties() -> brightnessOption() << " " << configuration() -> brightness() << " -> " << brightness() << "\n";
  kDebug() << "             Hue " << properties() -> hue() << " " << properties() -> hueOption() << " " << configuration() -> hue() << " -> " << hue() << "\n";
  kDebug() << "             Saturation " << properties() -> saturation() << " " << properties() -> saturationOption() << " " << configuration() -> saturation() << " -> " << saturation() << "\n";
  kDebug() << "             Subtitle autoload " << properties() -> subtitleAutoload() << " " << configuration() -> subtitleAutoload() << "\n";
  kDebug() << "             Subtitle position " << properties() -> subtitlePosition() << " " << configuration() -> subtitlePosition() << " -> " << subtitlePosition() << "\n";
  kDebug() << "             Subtitle delay " << properties() -> subtitleDelay() << " " << configuration() -> subtitleDelay() << " -> " << subtitleDelay() << "\n";
  kDebug() << "             Audio driver '" << configuration() -> audioDriver() << "' '" << configuration() -> audioDevice() << "' -> '" << properties() -> audioDriverString() << "'\n";
  kDebug() << "             Audio mixer '" << configuration() -> mixerDevice() << "' -> '" << properties() -> mixerDevice() << "' channel '" << configuration() -> mixerChannel() << "' '" << properties() -> mixerChannel() << "' -> '" << properties() -> mixerChannelString() << "'\n";
  kDebug() << "             Audio codec '" << properties() -> audioCodecOption() << "' '" << configuration() -> audioCodec() << "' '" << properties() -> audioCodecString() << "'\n";
  kDebug() << "             Video driver '" << configuration() -> videoDriver() << "' '" << configuration() -> videoDevice() << "' -> '" << properties() -> videoDriverString() << "'\n";
  kDebug() << "             Video codec '" << properties() -> videoCodecOption() << "' '" << configuration() -> videoCodec() << "' '" << properties() -> videoCodecString() << "'\n";
  kDebug() << "             Video double buffering " << properties() -> videoDoubleBuffering() << " " << configuration() -> videoDoubleBuffering() << "\n";
  kDebug() << "             Video direct rendering " << properties() -> videoDirectRendering() << " " << configuration() -> videoDirectRendering() << "\n";
  kDebug() << "             Executable path '" << properties() -> executablePath() << "' '" << configuration() -> executablePath() << "'\n";
  kDebug() << "             Additional command line '" << properties() -> commandLine() << "' " << properties() -> commandLineOption() << " '" << configuration() -> commandLine() << "'\n";
  kDebug() << "             Demuxer '" << properties() -> demuxerOption() << "' '" << configuration() -> demuxer() << "' '" << properties() -> demuxerString() << "'\n";
  kDebug() << "             Frame drop " << properties() -> frameDrop() << " " << configuration() -> frameDrop() << " -> " << frameDrop() << "\n";
  kDebug() << "             Cache " << properties() -> cache() << " " << configuration() -> cache() << "\n";
  kDebug() << "             Cache size " << properties() -> cacheSize() << " " << configuration() -> cacheSize() << "\n";
  kDebug() << "             Build index " << properties() -> buildNewIndex() << " " << configuration() -> buildNewIndex() << "\n";
  kDebug() << "             OSD Level " << properties() -> osdLevel() << " " << configuration() -> osdLevel() << "\n";
  kDebug() << "             Temporary file " << properties() -> useTemporaryFile() << " " << configuration() -> useTemporaryFile() << "\n";
  kDebug() << "             KIOSlave " << properties() -> useKioslaveOption() << " " << configuration() -> useKioslaveForHttp() << " " << configuration() -> useKioslaveForFtp() << " " << configuration() -> useKioslaveForSmb() << "\n";
#endif
}

bool KPlayerSettings::fullScreen (void)
{
  if ( kPlayerEngine() -> stopped() )
    return false;
#ifdef DEBUG_KPLAYER_SETTINGS
  bool last_full_screen = m_last_full_screen;
#endif
  bool full_screen_default = properties() -> hasVideo() ? configuration() -> fullScreen()
    : properties() -> hasNoVideo() ? false : m_last_full_screen;
  m_last_full_screen = ! configuration() -> override ("Full Screen") && ! properties() -> hasNoVideo() 
    && properties() -> hasFullScreen() ? properties() -> fullScreen() : full_screen_default;
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::fullScreen " << properties() -> hasVideo() << " " << properties() -> hasNoVideo()
    << " " << configuration() -> fullScreen() << " " << last_full_screen << " = " << m_last_full_screen << "\n";
#endif
  return m_last_full_screen;
}

void KPlayerSettings::setFullScreen (bool full_screen)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setFullScreen (" << full_screen << ")\n";
#endif
  setOverride ("Full Screen", ! configuration() -> rememberFullScreen (shift()));
  properties ("Full Screen") -> setFullScreen (full_screen);
}

void KPlayerSettings::setMaximized (bool maximized)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setMaximized (" << maximized << ")\n";
#endif
  setOverride ("Maximized", ! configuration() -> rememberMaximized (shift()));
  properties ("Maximized") -> setMaximized (maximized);
}

void KPlayerSettings::setMaintainAspect (bool maintain, QSize aspect)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setMaintainAspect (" << maintain << ", " << aspect.width() << "x" << aspect.height() << ")\n";
#endif
  setOverride ("Maintain Aspect", ! configuration() -> rememberMaintainAspect (shift()));
  properties ("Maintain Aspect") -> setMaintainAspect (maintain);
  if ( ! maintain )
    return;
  setAspect (aspect);
  if ( configuration() -> rememberAspect (shift()) )
  {
    properties() -> setDisplaySize (aspect, 2);
    setAspectOverride (false);
  }
}

void KPlayerSettings::setAspect (QSize aspect)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setAspect " << m_aspect.width() << "x" << m_aspect.height() << " => " << aspect.width() << "x" << aspect.height() << "\n";
#endif
  if ( ! aspect.isEmpty() && ! m_aspect.isEmpty() && aspect.width() * m_aspect.height()
      == aspect.height() * m_aspect.width() || aspect.isEmpty() && m_aspect.isEmpty() )
    return;
  m_aspect = aspect;
  aspect = properties() -> currentAspect();
  setAspectOverride (! aspect.isEmpty() && ! m_aspect.isEmpty() && aspect.width() * m_aspect.height() != aspect.height() * m_aspect.width());
}

void KPlayerSettings::setDisplaySize (QSize size)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setDisplaySize (" << size.width() << "x" << size.height() << ")\n";
#endif
  m_display_size = size;
}

void KPlayerSettings::setVolume (int volume)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setVolume (" << volume << ")\n";
#endif
  setOverride ("Volume", false);
  if ( configuration() -> rememberVolume (shift()) )
    properties() -> setVolume (volume);
  else
    properties() -> adjustVolume (volume);
}

void KPlayerSettings::setAudioDelay (float delay)
{
  if ( fabs (delay) < 0.0001 )
    delay = 0;
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setAudioDelay (" << delay << ")\n";
#endif
  setOverride ("Audio Delay", ! configuration() -> rememberAudioDelay (shift()));
  properties ("Audio Delay") -> setAudioDelay (delay);
}

void KPlayerSettings::setFrameDrop (int frame_drop)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setFrameDrop (" << frame_drop << ")\n";
#endif
  setOverride ("Frame Dropping", ! configuration() -> rememberFrameDrop (shift()));
  properties ("Frame Dropping") -> setFrameDrop (frame_drop);
}

void KPlayerSettings::setContrast (int contrast)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setContrast (" << contrast << ")\n";
#endif
  setOverride ("Contrast", false);
  if ( configuration() -> rememberContrast (shift()) )
    properties() -> setContrast (contrast);
  else
    properties() -> adjustContrast (contrast);
}

void KPlayerSettings::setBrightness (int brightness)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setBrightness (" << brightness << ")\n";
#endif
  setOverride ("Brightness", false);
  if ( configuration() -> rememberBrightness (shift()) )
    properties() -> setBrightness (brightness);
  else
    properties() -> adjustBrightness (brightness);
}

void KPlayerSettings::setHue (int hue)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setHue (" << hue << ")\n";
#endif
  setOverride ("Hue", false);
  if ( configuration() -> rememberHue (shift()) )
    properties() -> setHue (hue);
  else
    properties() -> adjustHue (hue);
}

void KPlayerSettings::setSaturation (int saturation)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSaturation (" << saturation << ")\n";
#endif
  setOverride ("Saturation", false);
  if ( configuration() -> rememberSaturation (shift()) )
    properties() -> setSaturation (saturation);
  else
    properties() -> adjustSaturation (saturation);
}

QString KPlayerSettings::currentSubtitles (void) const
{
  if ( properties() -> hasSubtitleUrl() )
  {
    QString subtitle (properties() -> subtitleUrlString());
    if ( subtitles().indexOf (subtitle) != -1 )
      return subtitle;
  }
  return subtitles().isEmpty() ? QString() : subtitles().first();
}

QString KPlayerSettings::currentSubtitlePath (void) const
{
  QString current (currentSubtitles());
  return hasVobsubSubtitles() && current != properties() -> subtitleUrlString() ? vobsubSubtitles() : current;
}

static bool vobsubExtension (const QString& path)
{
  return path.endsWith (".idx", Qt::CaseInsensitive) || path.endsWith (".ifo", Qt::CaseInsensitive)
    || path.endsWith (".sub", Qt::CaseInsensitive);
}

bool vobsub (const QString& path)
{
  if ( ! vobsubExtension (path) )
    return false;
  if ( ! path.endsWith (".sub", Qt::CaseInsensitive) )
    return true;
  QFile file (path);
  if ( ! file.open (QIODevice::ReadOnly) )
    return false;
  char data [4];
  int length = file.read (data, sizeof (data));
  file.close();
  return length == sizeof (data) && memcmp (data, "\0\0\1\272", sizeof (data)) == 0;
}

void KPlayerSettings::addSubtitlePath (const QString& path)
{
  if ( path == properties() -> subtitleUrlString() ? properties() -> vobsubSubtitles() : vobsub (path) )
  {
    if ( path == properties() -> subtitleUrlString() || m_vobsub.isEmpty() )
      m_vobsub = vobsubExtension (path) ? path.left (path.length() - 4) : path;
  }
  else if ( subtitles().indexOf (path) == -1 )
    m_subtitles.append (path);
}

void KPlayerSettings::setSubtitlePosition (int position)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSubtitlePosition (" << position << ")\n";
#endif
  setOverride ("Subtitle Position", ! configuration() -> rememberSubtitlePosition (shift()));
  properties ("Subtitle Position") -> setSubtitlePosition (position);
}

void KPlayerSettings::setSubtitleDelay (float delay)
{
  if ( fabs (delay) < 0.0001 )
    delay = 0;
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSubtitleDelay (" << delay << ")\n";
#endif
  setOverride ("Subtitle Delay", ! configuration() -> rememberSubtitleDelay (shift()));
  properties ("Subtitle Delay") -> setSubtitleDelay (delay);
}

bool KPlayerSettings::isAspect (QSize size)
{
  return ! size.isEmpty() && ! aspect().isEmpty() && size.width() * aspect().height() == size.height() * aspect().width();
}

bool KPlayerSettings::isZoomFactor (int m, int d)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::isZoomFactor (" << m << ", " << d << ") "
    << properties() -> currentSize().width() << "x" << properties() -> currentSize().height()
    << " " << displaySize().width() << "x" << displaySize().height()
    << " " << aspect().width() << "x" << aspect().height() << "\n";
#endif
  if ( fullScreen() || maximized() || ! properties() -> hasOriginalSize() )
    return false;
  QSize size (properties() -> currentSize() * m / d);
  if ( ! aspect().isEmpty() )
    size.setHeight (size.width() * aspect().height() / aspect().width());
  bool result = size == displaySize();
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::isZoomFactor " << size.width() << "x" << size.height() << " " << result << "\n";
#endif
  return result;
}

QSize KPlayerSettings::adjustDisplaySize (bool user_zoom, bool user_resize)
{
  QSize size (! user_zoom && constrainedSize() ? constrainSize (kPlayerWorkspace() -> size())
    : adjustSize (displaySize()));
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::adjustDisplaySize (" << user_zoom << ", " << user_resize << ") " << size.width() << "x" << size.height() << "\n";
#endif
  if ( ! fullScreen() && ! maximized() )
  {
    setDisplaySize (size);
    if ( user_zoom || user_resize )
    {
      setDisplaySizeOverride (true);
      if ( configuration() -> rememberSize (shift()) )
      {
        properties() -> setDisplaySize (size, 1);
        setDisplaySizeOverride (false);
        setAspectOverride (false);
      }
      else if ( user_resize && ! maintainAspect() && configuration() -> rememberAspect() )
      {
        properties() -> setDisplaySize (size, 2);
        setAspectOverride (false);
      }
    }
  }
  return size;
}

bool KPlayerSettings::setInitialDisplaySize (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setInitialDisplaySize\n";
#endif
  if ( ! properties() -> hasVideo() && ! properties() -> hasNoVideo() )
    return false;
  if ( ! aspectOverride() )
  {
    setAspect (properties() -> currentAspect());
    setAspectOverride (false);
#ifdef DEBUG_KPLAYER_SETTINGS
    kdDebugTime() << "Settings: Initial aspect: " << aspect().width() << "x" << aspect().height() << " " << maintainAspect() << "\n";
#endif
  }
  if ( displaySizeOverride() )
    return ! aspectOverride();
  QSize size;
  if ( properties() -> displaySizeOption() == 1 )
    size = properties() -> displaySize();
  else if ( ! properties() -> hasOriginalSize() )
    size = QSize (configuration() -> minimumInitialWidth(), 0);
  else
  {
    int d = 1, n = (configuration() -> minimumInitialWidth() - 1) / properties() -> currentSize().width();
    if ( n > 0 && properties() -> currentSize().width() * n
        + properties() -> currentSize().width() / 2 >= configuration() -> minimumInitialWidth() )
      n *= d = 2;
#ifdef DEBUG_KPLAYER_SETTINGS
    kdDebugTime() << "Initial Zoom Factor " << (n + 1) << " / " << d << "\n";
#endif
    size = adjustSize (properties() -> currentSize() * (n + 1) / d);
  }
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings: Initial size: " << size.width() << "x" << size.height() << "\n";
#endif
  setDisplaySize (size);
  return true;
}

QSize KPlayerSettings::constrainSize (QSize size) const
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::constrainSize " << size.width() << "x" << size.height() << "\n";
#endif
  return adjustSize (size, size.width() * aspect().height() > size.height() * aspect().width());
}

QSize KPlayerSettings::adjustSize (QSize size, bool horizontally) const
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::adjustSize " << size.width() << "x" << size.height() << " " << horizontally << "\n";
#endif
  if ( maintainAspect() && ! aspect().isEmpty() && ! size.isEmpty() )
  {
    if ( horizontally )
      size.setWidth (size.height() * aspect().width() / aspect().height());
    else
      size.setHeight (size.width() * aspect().height() / aspect().width());
  }
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::adjustSize " << size.width() << "x" << size.height() << "\n";
#endif
  return size;
}

void KPlayerSetControlShiftState (bool control, bool shift)
{
  KPlayerEngine::engine() -> settings() -> setControl (control);
  KPlayerEngine::engine() -> settings() -> setShift (shift);
}
