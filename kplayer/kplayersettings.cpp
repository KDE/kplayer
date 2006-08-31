/***************************************************************************
                          kplayersettings.cpp
                          -------------------
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
  m_properties = KPlayerMedia::trackProperties (KURL());
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

void KPlayerSettings::load (const KURL& url)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::load (" << url.prettyURL() << ")\n";
#endif
  if ( url == properties() -> url() )
    return;
  if ( properties() -> url().isValid() )
    properties() -> commit();
  KPlayerMedia::release (m_properties);
  m_properties = KPlayerMedia::trackProperties (url);
  setAspect (properties() -> originalAspect());
  if ( properties() -> displaySizeOption() == 1 )
    setDisplaySize (properties() -> displaySize());
  configuration() -> itemReset();
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Display size " << properties() -> displaySizeOption() << " " << properties() -> displaySize().width() << "x" << properties() -> displaySize().height() << " " << properties() -> originalSize().width() << "x" << properties() -> originalSize().height() << " -> " << displaySize().width() << "x" << displaySize().height() << "\n";
  kdDebug() << "             Maintain aspect " << properties() -> maintainAspect() << " " << configuration() -> maintainAspect() << " -> " << maintainAspect() << " " << aspect().width() << "x" << aspect().height() << "\n";
  kdDebug() << "             Volume " << properties() -> volume() << " " << properties() -> volumeOption() << " " << configuration() -> volume() << " " << configuration() -> mute() << " -> " << actualVolume() << "\n";
  kdDebug() << "             Audio delay " << properties() -> audioDelay() << " " << configuration() -> audioDelay() << " -> " << audioDelay() << "\n";
  kdDebug() << "             Contrast " << properties() -> contrast() << " " << properties() -> contrastOption() << " " << configuration() -> contrast() << " -> " << contrast() << "\n";
  kdDebug() << "             Brightness " << properties() -> brightness() << " " << properties() -> brightnessOption() << " " << configuration() -> brightness() << " -> " << brightness() << "\n";
  kdDebug() << "             Hue " << properties() -> hue() << " " << properties() -> hueOption() << " " << configuration() -> hue() << " -> " << hue() << "\n";
  kdDebug() << "             Saturation " << properties() -> saturation() << " " << properties() -> saturationOption() << " " << configuration() -> saturation() << " -> " << saturation() << "\n";
  kdDebug() << "             Subtitle autoload " << properties() -> subtitleAutoload() << " " << configuration() -> subtitleAutoload() << "\n";
  kdDebug() << "             Subtitle position " << properties() -> subtitlePosition() << " " << configuration() -> subtitlePosition() << " -> " << subtitlePosition() << "\n";
  kdDebug() << "             Subtitle delay " << properties() -> subtitleDelay() << " " << configuration() -> subtitleDelay() << " -> " << subtitleDelay() << "\n";
  kdDebug() << "             Audio driver '" << properties() -> audioDriver() << "' '" << configuration() -> audioDriver() << "' '" << properties() -> audioDevice() << "' '" << configuration() -> audioDevice() << "'\n";
  kdDebug() << "             Audio codec '" << properties() -> audioCodec() << "' '" << properties() -> audioCodecOption() << "' '" << configuration() -> audioCodecOption() << "'\n";
  kdDebug() << "             Video driver '" << properties() -> videoDriver() << "' '" << configuration() -> videoDriver() << "' '" << properties() -> videoDevice() << "' '" << configuration() -> videoDevice() << "'\n";
  kdDebug() << "             Video codec '" << properties() -> videoCodec() << "' '" << properties() -> videoCodecOption() << "' '" << configuration() -> videoCodecOption() << "'\n";
  kdDebug() << "             Video double buffering " << properties() -> videoDoubleBuffering() << " " << configuration() -> videoDoubleBuffering() << "\n";
  kdDebug() << "             Video direct rendering " << properties() -> videoDirectRendering() << " " << configuration() -> videoDirectRendering() << "\n";
  kdDebug() << "             Executable path '" << properties() -> executablePath() << "' '" << configuration() -> executablePath() << "'\n";
  kdDebug() << "             Additional command line '" << properties() -> commandLine() << "' " << properties() -> commandLineOption() << " '" << configuration() -> commandLine() << "'\n";
  kdDebug() << "             Frame drop " << properties() -> frameDrop() << " " << configuration() -> frameDrop() << " -> " << frameDrop() << "\n";
  kdDebug() << "             Cache " << properties() -> cache() << " " << configuration() -> cache() << "\n";
  kdDebug() << "             Cache size " << properties() -> cacheSize() << " " << configuration() -> cacheSize() << "\n";
  kdDebug() << "             Build index " << properties() -> buildNewIndex() << " " << configuration() -> buildNewIndex() << "\n";
  kdDebug() << "             OSD Level " << properties() -> osdLevel() << " " << configuration() -> osdLevel() << "\n";
  kdDebug() << "             Temporary file " << properties() -> useTemporaryFile() << " " << configuration() -> useTemporaryFile() << "\n";
  kdDebug() << "             KIOSlave " << properties() -> useKioslaveOption() << " " << configuration() -> useKioslaveForHttp() << " " << configuration() -> useKioslaveForFtp() << " " << configuration() -> useKioslaveForSmb() << "\n";
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
    if ( ! aspect.isEmpty() && properties() -> hasOriginalSize()
        && aspect.width() * properties() -> originalSize().height()
        == aspect.height() * properties() -> originalSize().width() )
      properties() -> resetDisplaySize();
    else
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
  aspect = properties() -> originalAspect();
  setAspectOverride (! aspect.isEmpty() && ! m_aspect.isEmpty() && aspect.width() * m_aspect.height() != aspect.height() * m_aspect.width());
}

void KPlayerSettings::setDisplaySize (QSize size)
{
  setDisplaySizeOverride (true);
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
    << properties() -> originalSize().width() << "x" << properties() -> originalSize().height()
    << " " << displaySize().width() << "x" << displaySize().height()
    << " " << aspect().width() << "x" << aspect().height() << "\n";
#endif
  if ( fullScreen() || maximized() || ! properties() -> hasOriginalSize() )
    return false;
  QSize size (properties() -> originalSize() * m / d);
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
  QSize size;
  if ( ! user_zoom && constrainedSize() )
    size = constrainSize (kPlayerWorkspace() -> size());
  else
    size = adjustSize (displaySize());
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::adjustDisplaySize (" << user_zoom << ", " << user_resize << ") " << size.width() << "x" << size.height() << "\n";
#endif
  if ( ! fullScreen() && ! maximized() )
  {
    setDisplaySize (size);
    if ( user_zoom || user_resize )
    {
      if ( configuration() -> rememberSize (shift()) )
      {
        properties() -> setDisplaySize (size, 1);
        setDisplaySizeOverride (false);
        setAspectOverride (false);
      }
      else if ( configuration() -> rememberAspect() )
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
    setAspect (properties() -> originalAspect());
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
    int d = 1, n = (configuration() -> minimumInitialWidth() - 1) / properties() -> originalSize().width();
    if ( n > 0 && properties() -> originalSize().width() * n
        + properties() -> originalSize().width() / 2 >= configuration() -> minimumInitialWidth() )
      n *= d = 2;
#ifdef DEBUG_KPLAYER_SETTINGS
    kdDebugTime() << "Initial Zoom Factor " << (n + 1) << " / " << d << "\n";
#endif
    size = adjustSize (properties() -> originalSize() * (n + 1) / d);
  }
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings: Initial size: " << size.width() << "x" << size.height() << "\n";
#endif
  setDisplaySize (size);
  setDisplaySizeOverride (false);
  return true;
}

QSize KPlayerSettings::constrainSize (QSize size) const
{
  return adjustSize (size, size.width() * aspect().height() > size.height() * aspect().width());
}

QSize KPlayerSettings::adjustSize (QSize size, bool horizontally) const
{
  if ( maintainAspect() && ! aspect().isEmpty() && ! size.isEmpty() )
  {
    if ( horizontally )
      size.setWidth (size.height() * aspect().width() / aspect().height());
    else
      size.setHeight (size.width() * aspect().height() / aspect().width());
  }
  return size;
}

void KPlayerSetControlShiftState (bool control, bool shift)
{
  KPlayerEngine::engine() -> settings() -> setControl (control);
  KPlayerEngine::engine() -> settings() -> setShift (shift);
}
