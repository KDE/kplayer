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

#include <kdebug.h>

#include "kplayersettings.h"
#include "kplayersettings.moc"
#include "kplayerengine.h"
#include "kplayerwidget.h"

#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#define HAVE_OSS_SUPPORT
#elif defined(HAVE_MACHINE_SOUNDCARD_H)
#include <machine/soundcard.h>
#define HAVE_OSS_SUPPORT
#elif defined(HAVE_SOUNDCARD_H)
#include <soundcard.h>
#define HAVE_OSS_SUPPORT
#else
#undef HAVE_OSS_SUPPORT
#endif

#define DEBUG_KPLAYER_SETTINGS

#ifndef NDEBUG
kdbgstream kdDebugTime (void)
{
  return kdbgstream (0, 0) << QTime::currentTime().toString ("hh:mm:ss:zzz ");
}
#endif

static QRegExp re_remote_url ("^[A-Za-z]+:/");
static QRegExp re_mplayer_url ("^(?:file|http_proxy|mms|mmst|mmsu|rtp|rtsp|sip|pnm|dvd|vcd|tv|dvb|mf|cdda|cddb|cue|sdp|mpst):/", false);
static QRegExp re_http_url ("^http:/", false);
static QRegExp re_ftp_url ("^ftp:/", false);
static QRegExp re_smb_url ("^smb:/", false);
static QRegExp re_playlist_url ("^(?:file|http|http_proxy|ftp|smb):/.*\\.(?:ram|smi|smil|rpm|asx|pls|m3u|strm)(?:\\?|$)", false);

KPlayerSettings::KPlayerSettings (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Creating settings\n";
#endif
  m_properties = 0;
  m_last_full_screen = false;
  defaults();
  setShift (false);
  setFullScreenDefault (false);
  setMaximizedDefault (false);
  setMaintainAspectDefault (true);
  setVolumeDefault (50);
  setMute (false);
  setAudioDelayDefault (0);
  setContrastDefault (0);
  setBrightnessDefault (0);
  setHueDefault (0);
  setSaturationDefault (0);
  setSubtitleVisibilityDefault (true);
  setSubtitlePositionDefault (100);
  setSubtitleDelayDefault (0);
  setVolumeOverride (false);
  setAudioDelayOverride (false);
  setContrastOverride (false);
  setBrightnessOverride (false);
  setHueOverride (false);
  setSaturationOverride (false);
  setAspectOverride (false);
  setDisplaySizeOverride (false);
  setFullScreenOverride (false);
  setMaximizedOverride (false);
  setMaintainAspectOverride (false);
  setSubtitleUrlOverride (false);
  setSubtitleVisibilityOverride (false);
  setSubtitlePositionOverride (false);
  setSubtitleDelayOverride (false);
  setFrameDropOverride (false);
  load();
}

KPlayerSettings::~KPlayerSettings()
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Destroying settings\n";
#endif
  if ( m_properties )
  {
    m_properties -> save();
    kPlayerEngine() -> dereference (m_properties);
  }
}

void KPlayerSettings::defaults (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::defaults\n";
#endif
  setResizeAutomatically (true);
  setMinimumInitialWidth (500);
  setRecentFileListSize (10);
  setShowFilePath (false);
  setShowOpenDialog (false);
  setDisableScreenSaver (true);
  setStartPlayingImmediately (true);
  setStartPlayingOnlyIfIdle (false);
  setAllowDuplicateEntries (false);
  setPlaylistSizeLimit (100);
  setCacheSizeLimit (1000);
  setRememberWithShift (true);
  setRememberSize (false);
  setRememberAspect (false);
  setRememberFullScreen (false);
  setRememberMaximized (false);
  setRememberMaintainAspect (false);
  setRememberVolume (false);
  setRememberAudioDelay (true);
  setRememberFrameDrop (false);
  setRememberContrast (false);
  setRememberBrightness (false);
  setRememberHue (false);
  setRememberSaturation (false);
  setRememberSubtitleUrl (true);
  setRememberSubtitleVisibility (false);
  setRememberSubtitlePosition (false);
  setRememberSubtitleDelay (true);
  setPreferredSliderLength (200);
  setMinimumSliderLength (100);
  setProgressNormalSeekUnits (0);
  setProgressFastSeekUnits (0);
  setProgressNormalSeek (1);
  setProgressFastSeek (10);
  setProgressMarks (10);
  setVolumeMinimum (0);
  setVolumeMaximum (100);
  setVolumeMarks (10);
  setVolumeStep (1);
  setInitialVolume (50);
  setVolumeReset (false);
  setVolumeEvery (0);
  setContrastMinimum (-100);
  setContrastMaximum (100);
  setContrastMarks (20);
  setContrastStep (1);
  setInitialContrast (0);
  setContrastReset (false);
  setContrastEvery (0);
  setBrightnessMinimum (-50);
  setBrightnessMaximum (50);
  setBrightnessMarks (10);
  setBrightnessStep (1);
  setInitialBrightness (0);
  setBrightnessReset (false);
  setBrightnessEvery (0);
  setHueMinimum (-100);
  setHueMaximum (100);
  setHueMarks (20);
  setHueStep (1);
  setInitialHue (0);
  setHueReset (false);
  setHueEvery (0);
  setSaturationMinimum (-100);
  setSaturationMaximum (100);
  setSaturationMarks (20);
  setSaturationStep (1);
  setInitialSaturation (0);
  setSaturationReset (false);
  setSaturationEvery (0);
  setSubtitlePositionStep (1);
  setSubtitleDelayStep (0.1);
  setSubtitleAutoloadDefault (true);
  setAutoloadAqtSubtitles (true);
  setAutoloadJssSubtitles (true);
  setAutoloadRtSubtitles (true);
  setAutoloadSmiSubtitles (true);
  setAutoloadSrtSubtitles (true);
  setAutoloadSsaSubtitles (true);
  setAutoloadSubSubtitles (true);
  setAutoloadTxtSubtitles (true);
  setAutoloadUtfSubtitles (true);
  setAutoloadOtherSubtitles (false);
  setAutoloadExtensionList ("");
  setShowMessagesOnError (false);
  setShowMessagesBeforePlaying (false);
  setShowMessagesAfterPlaying (false);
  setClearMessagesBeforePlaying (true);
  setVideoDriverDefault ("xv");
  setVideoDriverFallbackDefault (true);
  setVideoDeviceDefault ("");
  setVideoCodecDefault ("");
  setVideoCodecFallbackDefault (true);
  setVideoScalerDefault (0);
  setVideoDoubleBufferingDefault (true);
  setVideoDirectRenderingDefault (false);
  setAudioDelayStep (0.1);
  setAudioDriverDefault ("oss");
  setAudioDriverFallbackDefault (true);
  setAudioDeviceDefault ("");
  setAudioCodecDefault ("");
  setAudioCodecFallbackDefault (true);
  setExecutablePathDefault ("");
  setCommandLineDefault ("");
  setFrameDropDefault (0);
  setCacheDefault (2);
  setCacheSizeDefault (1024);
  setBuildNewIndexDefault (1);
  setUseTemporaryFileDefault (false);
  setUseKioslaveForHttp (false);
  setUseKioslaveForFtp (false);
  setUseKioslaveForSmb (false);
  setOsdLevelDefault (0);
}

void KPlayerSettings::load (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::load\n";
#endif
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Player Options");
  setFullScreenDefault (config -> readBoolEntry ("Full Screen", fullScreenDefault()));
  setMaximizedDefault (config -> readBoolEntry ("Maximized", maximizedDefault()));
  setMaintainAspectDefault (config -> readBoolEntry ("Maintain Aspect", maintainAspectDefault()));
  setResizeAutomatically (config -> readBoolEntry ("Resize Main Window Automatically", resizeAutomatically()));
  setMinimumInitialWidth (config -> readUnsignedNumEntry ("Minimum Initial Width", minimumInitialWidth()));
  setRecentFileListSize (config -> readUnsignedNumEntry ("Recent File List Size", recentFileListSize()));
  setShowFilePath (config -> readBoolEntry ("Show File Path", showFilePath()));
  setShowOpenDialog (config -> readBoolEntry ("Show Open Dialog", showOpenDialog()));
  setDisableScreenSaver (config -> readBoolEntry ("Disable Screen Saver", disableScreenSaver()));
  setStartPlayingImmediately (config -> readBoolEntry ("Start Playing Immediately", startPlayingImmediately()));
  setStartPlayingOnlyIfIdle (config -> readBoolEntry ("Start Playing Only If Idle", startPlayingOnlyIfIdle()));
  setAllowDuplicateEntries (config -> readBoolEntry ("Allow Duplicate Entries", allowDuplicateEntries()));
  setPlaylistSizeLimit (config -> readUnsignedNumEntry ("Playlist Size Limit", playlistSizeLimit()));
  setCacheSizeLimit (config -> readUnsignedNumEntry ("Cache Size Limit", cacheSizeLimit()));
  setRememberWithShift (config -> readBoolEntry ("Remember With Shift", rememberWithShift()));
  setRememberSize (config -> readBoolEntry ("Remember Size", rememberSize()));
  setRememberAspect (config -> readBoolEntry ("Remember Aspect", rememberAspect()));
  setRememberFullScreen (config -> readBoolEntry ("Remember Full Screen", rememberFullScreen()));
  setRememberMaximized (config -> readBoolEntry ("Remember Maximized", rememberMaximized()));
  setRememberMaintainAspect (config -> readBoolEntry ("Remember Maintain Aspect", rememberMaintainAspect()));
  setRememberVolume (config -> readBoolEntry ("Remember Volume", rememberVolume()));
  setRememberAudioDelay (config -> readBoolEntry ("Remember Audio Delay", rememberAudioDelay()));
  setRememberFrameDrop (config -> readBoolEntry ("Remember Frame Drop", rememberFrameDrop()));
  setRememberContrast (config -> readBoolEntry ("Remember Contrast", rememberContrast()));
  setRememberBrightness (config -> readBoolEntry ("Remember Brightness", rememberBrightness()));
  setRememberHue (config -> readBoolEntry ("Remember Hue", rememberHue()));
  setRememberSaturation (config -> readBoolEntry ("Remember Saturation", rememberSaturation()));
  setRememberSubtitleUrl (config -> readBoolEntry ("Remember Subtitle URL", rememberSubtitleUrl()));
  setRememberSubtitleVisibility (config -> readBoolEntry ("Remember Subtitle Visibility", rememberSubtitleVisibility()));
  setRememberSubtitlePosition (config -> readBoolEntry ("Remember Subtitle Position", rememberSubtitlePosition()));
  setRememberSubtitleDelay (config -> readBoolEntry ("Remember Subtitle Delay", rememberSubtitleDelay()));
  setPreferredSliderLength (config -> readUnsignedNumEntry ("Preferred Slider Length", preferredSliderLength()));
  setMinimumSliderLength (config -> readUnsignedNumEntry ("Minimum Slider Length", minimumSliderLength()));
  setProgressNormalSeekUnits (config -> readUnsignedNumEntry ("Normal Seek Units", progressNormalSeekUnits()));
  setProgressFastSeekUnits (config -> readUnsignedNumEntry ("Fast Seek Units", progressFastSeekUnits()));
  setProgressNormalSeek (config -> readUnsignedNumEntry ("Normal Seek", progressNormalSeek()));
  setProgressFastSeek (config -> readUnsignedNumEntry ("Fast Seek", progressFastSeek()));
  setProgressMarks (config -> readUnsignedNumEntry ("Progress Marks", progressMarks()));
  setVolumeMinimum (config -> readNumEntry ("Volume Minimum", volumeMinimum()));
  setVolumeMaximum (config -> readNumEntry ("Volume Maximum", volumeMaximum()));
  setVolumeMarks (config -> readNumEntry ("Volume Marks", volumeMarks()));
  setVolumeStep (config -> readNumEntry ("Volume Step", volumeStep()));
  setInitialVolume (config -> readNumEntry ("Volume Default", initialVolume()));
  setVolumeReset (config -> readBoolEntry ("Volume Reset", volumeReset()));
  setVolumeEvery (config -> readNumEntry ("Volume Every", volumeEvery()));
  if ( volumeReset() )
  {
    setVolumeDefault (initialVolume());
    setMute (false);
  }
  else
  {
    setVolumeDefault (config -> readUnsignedNumEntry ("Volume", volumeDefault()));
#ifdef HAVE_OSS_SUPPORT
    int vol, fd = open ("/dev/mixer", O_RDONLY);
    if ( fd < 0 )
      fd = open ("/dev/sound/mixer", O_RDONLY);
    if ( fd >= 0 )
    {
      if ( ioctl (fd, SOUND_MIXER_READ_PCM, &vol) != -1 )
      {
        setVolumeDefault (((vol & 0xFF) + ((vol & 0xFF00) >> 8)) / 2);
#ifdef DEBUG_KPLAYER_SETTINGS
        kdDebugTime() << "Read initial volume: " << volumeDefault() << "\n";
#endif
      }
      close (fd);
    }
#endif
    setMute (config -> readBoolEntry ("Mute", mute()));
  }
  setAudioDelayDefault (config -> readDoubleNumEntry ("Audio Delay", audioDelayDefault()));
  setContrastMinimum (config -> readNumEntry ("Contrast Minimum", contrastMinimum()));
  setContrastMaximum (config -> readNumEntry ("Contrast Maximum", contrastMaximum()));
  setContrastMarks (config -> readNumEntry ("Contrast Marks", contrastMarks()));
  setContrastStep (config -> readNumEntry ("Contrast Step", contrastStep()));
  setInitialContrast (config -> readNumEntry ("Contrast Default", initialContrast()));
  setContrastReset (config -> readBoolEntry ("Contrast Reset", contrastReset()));
  setContrastEvery (config -> readNumEntry ("Contrast Every", contrastEvery()));
  if ( contrastReset() )
    setContrastDefault (initialContrast());
  else
    setContrastDefault (config -> readNumEntry ("Contrast", contrastDefault()));
  setBrightnessMinimum (config -> readNumEntry ("Brightness Minimum", brightnessMinimum()));
  setBrightnessMaximum (config -> readNumEntry ("Brightness Maximum", brightnessMaximum()));
  setBrightnessMarks (config -> readNumEntry ("Brightness Marks", brightnessMarks()));
  setBrightnessStep (config -> readNumEntry ("Brightness Step", brightnessStep()));
  setInitialBrightness (config -> readNumEntry ("Brightness Default", initialBrightness()));
  setBrightnessReset (config -> readBoolEntry ("Brightness Reset", brightnessReset()));
  setBrightnessEvery (config -> readNumEntry ("Brightness Every", brightnessEvery()));
  if ( brightnessReset() )
    setBrightnessDefault (initialBrightness());
  else
    setBrightnessDefault (config -> readNumEntry ("Brightness", brightnessDefault()));
  setHueMinimum (config -> readNumEntry ("Hue Minimum", hueMinimum()));
  setHueMaximum (config -> readNumEntry ("Hue Maximum", hueMaximum()));
  setHueMarks (config -> readNumEntry ("Hue Marks", hueMarks()));
  setHueStep (config -> readNumEntry ("Hue Step", hueStep()));
  setInitialHue (config -> readNumEntry ("Hue Default", initialHue()));
  setHueReset (config -> readBoolEntry ("Hue Reset", hueReset()));
  setHueEvery (config -> readNumEntry ("Hue Every", hueEvery()));
  if ( hueReset() )
    setHueDefault (initialHue());
  else
    setHueDefault (config -> readNumEntry ("Hue", hueDefault()));
  setSaturationMinimum (config -> readNumEntry ("Saturation Minimum", saturationMinimum()));
  setSaturationMaximum (config -> readNumEntry ("Saturation Maximum", saturationMaximum()));
  setSaturationMarks (config -> readNumEntry ("Saturation Marks", saturationMarks()));
  setSaturationStep (config -> readNumEntry ("Saturation Step", saturationStep()));
  setInitialSaturation (config -> readNumEntry ("Saturation Default", initialSaturation()));
  setSaturationReset (config -> readBoolEntry ("Saturation Reset", saturationReset()));
  setSaturationEvery (config -> readNumEntry ("Saturation Every", saturationEvery()));
  if ( saturationReset() )
    setSaturationDefault (initialSaturation());
  else
    setSaturationDefault (config -> readNumEntry ("Saturation", saturationDefault()));
  setSubtitlePositionStep (config -> readNumEntry ("Subtitle Position Step", subtitlePositionStep()));
  setSubtitleDelayStep (config -> readDoubleNumEntry ("Subtitle Delay Step", subtitleDelayStep()));
  setAutoloadAqtSubtitles (config -> readBoolEntry ("Autoload Aqt Subtitles", autoloadAqtSubtitles()));
  setAutoloadJssSubtitles (config -> readBoolEntry ("Autoload Jss Subtitles", autoloadJssSubtitles()));
  setAutoloadRtSubtitles (config -> readBoolEntry ("Autoload Rt Subtitles", autoloadRtSubtitles()));
  setAutoloadSmiSubtitles (config -> readBoolEntry ("Autoload Smi Subtitles", autoloadSmiSubtitles()));
  setAutoloadSrtSubtitles (config -> readBoolEntry ("Autoload Srt Subtitles", autoloadSrtSubtitles()));
  setAutoloadSsaSubtitles (config -> readBoolEntry ("Autoload Ssa Subtitles", autoloadSsaSubtitles()));
  setAutoloadSubSubtitles (config -> readBoolEntry ("Autoload Sub Subtitles", autoloadSubSubtitles()));
  setAutoloadTxtSubtitles (config -> readBoolEntry ("Autoload Txt Subtitles", autoloadTxtSubtitles()));
  setAutoloadUtfSubtitles (config -> readBoolEntry ("Autoload Utf Subtitles", autoloadUtfSubtitles()));
  setAutoloadOtherSubtitles (config -> readBoolEntry ("Autoload Other Subtitles", autoloadOtherSubtitles()));
  setAutoloadExtensionList (config -> readEntry ("Autoload Extension List", autoloadExtensionList()));
  setSubtitleAutoloadDefault (config -> readBoolEntry ("Autoload Subtitles", subtitleAutoloadDefault()));
  setSubtitleVisibilityDefault (config -> readBoolEntry ("Show Subtitles", subtitleVisibilityDefault()));
  setSubtitlePositionDefault (config -> readUnsignedNumEntry ("Subtitle Position", subtitlePositionDefault()));
  setSubtitleDelayDefault (config -> readDoubleNumEntry ("Subtitle Delay", subtitleDelayDefault()));
  setShowMessagesOnError (config -> readBoolEntry ("Show Messages On Error", showMessagesOnError()));
  setShowMessagesBeforePlaying (config -> readBoolEntry ("Show Messages Before Playing", showMessagesBeforePlaying()));
  setShowMessagesAfterPlaying (config -> readBoolEntry ("Show Messages After Playing", showMessagesAfterPlaying()));
  setClearMessagesBeforePlaying (config -> readBoolEntry ("Clear Messages Before Playing", clearMessagesBeforePlaying()));
  setVideoDriverDefault (config -> readEntry ("Video Driver", videoDriverDefault()));
  setVideoDriverFallbackDefault (config -> readBoolEntry ("Video Driver Fallback", videoDriverFallbackDefault()));
  setVideoDeviceDefault (config -> readEntry ("Video Device", videoDeviceDefault()));
  setVideoCodecDefault (config -> readEntry ("Video Codec", videoCodecDefault()));
  setVideoCodecFallbackDefault (config -> readBoolEntry ("Video Codec Fallback", videoCodecFallbackDefault()));
  setVideoScalerDefault (config -> readUnsignedNumEntry ("Video Scaler", videoScalerDefault()));
  setVideoDoubleBufferingDefault (config -> readBoolEntry ("Enable Doublebuffering", videoDoubleBufferingDefault()));
  config -> deleteEntry ("Enable Doublebuffering");
  setVideoDoubleBufferingDefault (config -> readBoolEntry ("Double Buffering", videoDoubleBufferingDefault()));
  setVideoDirectRenderingDefault (config -> readBoolEntry ("Enable Direct Rendering", videoDirectRenderingDefault()));
  config -> deleteEntry ("Enable Direct Rendering");
  setVideoDirectRenderingDefault (config -> readBoolEntry ("Direct Rendering", videoDirectRenderingDefault()));
  setAudioDelayStep (config -> readDoubleNumEntry ("Audio Delay Step", audioDelayStep()));
  setAudioDriverDefault (config -> readEntry ("Audio Driver", audioDriverDefault()));
  setAudioDriverFallbackDefault (config -> readBoolEntry ("Audio Driver Fallback", audioDriverFallbackDefault()));
  setAudioDeviceDefault (config -> readEntry ("Audio Device", audioDeviceDefault()));
  setAudioCodecDefault (config -> readEntry ("Audio Codec", audioCodecDefault()));
  setAudioCodecFallbackDefault (config -> readBoolEntry ("Audio Codec Fallback", audioCodecFallbackDefault()));
  setExecutablePathDefault (config -> readEntry ("Executable Path", executablePathDefault()));
  setCommandLineDefault (config -> readEntry ("Command Line", commandLineDefault()));
  setFrameDropDefault (config -> readUnsignedNumEntry ("Frame Dropping", frameDropDefault()));
  setCacheDefault (config -> readBoolEntry ("Use Cache", cacheDefault() != 0) ? 2 : 0);
  config -> deleteEntry ("Use Cache");
  setCacheDefault (config -> readUnsignedNumEntry ("Cache", cacheDefault()));
  setCacheSizeDefault (config -> readUnsignedNumEntry ("Cache Size", cacheSizeDefault()));
  setBuildNewIndexDefault (config -> readBoolEntry ("Build Index", buildNewIndexDefault() != 1) ? 0 : 1);
  config -> deleteEntry ("Build Index");
  setBuildNewIndexDefault (config -> readUnsignedNumEntry ("Build New Index", buildNewIndexDefault()));
  setUseTemporaryFileDefault (config -> readBoolEntry ("Use Temporary File For KIOSlave", useTemporaryFileDefault()));
  setUseKioslaveForHttp (config -> readBoolEntry ("Use KIOSlave For HTTP", useKioslaveForHttp()));
  setUseKioslaveForFtp (config -> readBoolEntry ("Use KIOSlave For FTP", useKioslaveForFtp()));
  setUseKioslaveForSmb (config -> readBoolEntry ("Use KIOSlave For SMB", useKioslaveForSmb()));
  setOsdLevelDefault (config -> readUnsignedNumEntry ("OSD Level", osdLevelDefault()));
}

void KPlayerSettings::load (const KURL& url)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::load (" << url.prettyURL() << ")\n";
#endif
  if ( url == this -> url() )
    return;
  if ( m_properties )
  {
    m_properties -> save();
    kPlayerEngine() -> dereference (m_properties);
  }
  m_properties = kPlayerEngine() -> reference (url);
  m_properties -> load();
  setAspect (originalAspect());
  if ( m_properties -> displaySizeOption() == 1 )
    setDisplaySize (m_properties -> displaySizeValue());
  if ( volumeReset() && volumeEvery() == 0 )
  {
    setVolumeDefault (initialVolume());
    setMute (false);
  }
  if ( contrastReset() && contrastEvery() == 0 )
    setContrastDefault (initialContrast());
  if ( brightnessReset() && brightnessEvery() == 0 )
    setBrightnessDefault (initialBrightness());
  if ( hueReset() && hueEvery() == 0 )
    setHueDefault (initialHue());
  if ( saturationReset() && saturationEvery() == 0 )
    setSaturationDefault (initialSaturation());
  setSubtitleUrlDefault (KURL());
  setSubtitleVisibilityDefault (true);
  setSubtitleDelayDefault (0);
  setAudioDelayDefault (0);
  setVolumeOverride (false);
  setAudioDelayOverride (false);
  setContrastOverride (false);
  setBrightnessOverride (false);
  setHueOverride (false);
  setSaturationOverride (false);
  setAspectOverride (false);
  setDisplaySizeOverride (false);
  setFullScreenOverride (false);
  setMaximizedOverride (false);
  setMaintainAspectOverride (false);
  setSubtitleUrlOverride (false);
  setSubtitleVisibilityOverride (false);
  setSubtitlePositionOverride (false);
  setSubtitleDelayOverride (false);
  setFrameDropOverride (false);
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Display size " << m_properties -> displaySizeOption() << " " << m_properties -> displaySizeValue().width() << "x" << m_properties -> displaySizeValue().height() << " " << m_properties -> originalSize().width() << "x" << m_properties -> originalSize().height() << " -> " << displaySize().width() << "x" << displaySize().height() << "\n";
  kdDebug() << "             Maintain aspect " << m_properties -> maintainAspectOption() << " " << maintainAspectDefault() << " -> " << maintainAspect() << " " << aspect().width() << "x" << aspect().height() << "\n";
  kdDebug() << "             Volume " << m_properties -> volumeValue() << " " << m_properties -> volumeOption() << " " << volumeDefault() << " " << mute() << " -> " << actualVolume() << "\n";
  kdDebug() << "             Audio delay " << m_properties -> audioDelayValue() << " " << m_properties -> audioDelayOption() << " " << audioDelayDefault() << " -> " << audioDelay() << "\n";
  kdDebug() << "             Contrast " << m_properties -> contrastValue() << " " << m_properties -> contrastOption() << " " << contrastDefault() << " -> " << contrast() << "\n";
  kdDebug() << "             Brightness " << m_properties -> brightnessValue() << " " << m_properties -> brightnessOption() << " " << brightnessDefault() << " -> " << brightness() << "\n";
  kdDebug() << "             Hue " << m_properties -> hueValue() << " " << m_properties -> hueOption() << " " << hueDefault() << " -> " << hue() << "\n";
  kdDebug() << "             Saturation " << m_properties -> saturationValue() << " " << m_properties -> saturationOption() << " " << saturationDefault() << " -> " << saturation() << "\n";
  kdDebug() << "             Subtitle autoload " << m_properties -> subtitleAutoloadOption() << " " << subtitleAutoloadDefault() << " -> " << subtitleAutoload() << "\n";
  kdDebug() << "             Subtitle visibility " << m_properties -> subtitleVisibilityOption() << " " << subtitleVisibilityDefault() << " -> " << subtitleVisibility() << "\n";
  kdDebug() << "             Subtitle position " << m_properties -> subtitlePositionValue() << " " << m_properties -> subtitlePositionOption() << " " << subtitlePositionDefault() << " -> " << subtitlePosition() << "\n";
  kdDebug() << "             Subtitle delay " << m_properties -> subtitleDelayValue() << " " << m_properties -> subtitleDelayOption() << " " << subtitleDelayDefault() << " -> " << subtitleDelay() << "\n";
  kdDebug() << "             Audio driver '" << m_properties -> audioDriverValue() << "' '" << m_properties -> audioDriverOption() << "' '" << audioDriverDefault() << "' " << m_properties -> audioDriverFallbackOption() << " " << audioDriverFallbackDefault() << " '" << m_properties -> audioDeviceValue() << "' " << m_properties -> audioDeviceOption() << " '" << audioDeviceDefault() << "' -> '" << audioDriver() << "'\n";
  kdDebug() << "             Audio codec '" << m_properties -> audioCodecValue() << "' '" << m_properties -> audioCodecOption() << "' '" << audioCodecDefault() << "' " << m_properties -> audioCodecFallbackOption() << " " << audioCodecFallbackDefault() << " -> '" << audioCodec() << "'\n";
  kdDebug() << "             Video driver '" << m_properties -> videoDriverValue() << "' '" << m_properties -> videoDriverOption() << "' '" << videoDriverDefault() << "' " << m_properties -> videoDriverFallbackOption() << " " << videoDriverFallbackDefault() << " '" << m_properties -> videoDeviceValue() << "' " << m_properties -> videoDeviceOption() << " '" << videoDeviceDefault() << "' -> '" << videoDriver() << "'\n";
  kdDebug() << "             Video codec '" << m_properties -> videoCodecValue() << "' '" << m_properties -> videoCodecOption() << "' '" << videoCodecDefault() << "' " << m_properties -> videoCodecFallbackOption() << " " << videoCodecFallbackDefault() << " -> '" << videoCodec() << "'\n";
  kdDebug() << "             Video double buffering " << m_properties -> videoDoubleBufferingOption() << " " << videoDoubleBufferingDefault() << " -> " << videoDoubleBuffering() << "\n";
  kdDebug() << "             Video direct rendering " << m_properties -> videoDirectRenderingOption() << " " << videoDirectRenderingDefault() << " -> " << videoDirectRendering() << "\n";
  kdDebug() << "             Executable path '" << m_properties -> executablePathValue() << "' " << m_properties -> executablePathOption() << " '" << executablePathDefault() << "' -> '" << executablePath() << "'\n";
  kdDebug() << "             Additional command line '" << m_properties -> commandLineValue() << "' " << m_properties -> commandLineOption() << " '" << commandLineDefault() << "' -> '" << commandLine() << "'\n";
  kdDebug() << "             Frame drop " << m_properties -> frameDropOption() << " " << frameDropDefault() << " -> " << frameDrop() << "\n";
  kdDebug() << "             Cache " << m_properties -> cacheOption() << " " << cacheDefault() << " -> " << cache() << "\n";
  kdDebug() << "             Cache size " << m_properties -> cacheSizeValue() << " " << cacheSizeDefault() << " -> " << cacheSize() << "\n";
  kdDebug() << "             Build index " << m_properties -> buildNewIndexOption() << " " << buildNewIndexDefault() << " -> " << buildNewIndex() << "\n";
  kdDebug() << "             OSD Level " << m_properties -> osdLevelOption() << " " << osdLevelDefault() << " -> " << osdLevel() << "\n";
  kdDebug() << "             Temporary file " << m_properties -> useTemporaryFileOption() << " " << useTemporaryFileDefault() << " -> " << useTemporaryFile() << "\n";
  kdDebug() << "             KIOSlave " << m_properties -> useKioslaveOption() << " " << useKioslaveForHttp() << " " << useKioslaveForFtp() << " " << useKioslaveForSmb() << " -> " << useKioslave() << "\n";
#endif
}

void KPlayerSettings::save (void) const
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::save\n";
#endif
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Player Options");
  if ( fullScreenDefault() )
    config -> writeEntry ("Full Screen", fullScreenDefault());
  else
    config -> deleteEntry ("Full Screen");
  if ( maximizedDefault() )
    config -> writeEntry ("Maximized", maximizedDefault());
  else
    config -> deleteEntry ("Maximized");
  if ( maintainAspectDefault() )
    config -> deleteEntry ("Maintain Aspect");
  else
    config -> writeEntry ("Maintain Aspect", maintainAspectDefault());
  if ( ! kPlayerEngine() -> light() )
  {
    if ( resizeAutomatically() )
      config -> deleteEntry ("Resize Main Window Automatically");
    else
      config -> writeEntry ("Resize Main Window Automatically", resizeAutomatically());
  }
  if ( minimumInitialWidth() == 500 )
    config -> deleteEntry ("Minimum Initial Width");
  else
    config -> writeEntry ("Minimum Initial Width", minimumInitialWidth());
  if ( recentFileListSize() == 10 )
    config -> deleteEntry ("Recent File List Size");
  else
    config -> writeEntry ("Recent File List Size", recentFileListSize());
  if ( showFilePath() )
    config -> writeEntry ("Show File Path", showFilePath());
  else
    config -> deleteEntry ("Show File Path");
  if ( startPlayingImmediately() )
    config -> deleteEntry ("Start Playing Immediately");
  else
    config -> writeEntry ("Start Playing Immediately", startPlayingImmediately());
  if ( startPlayingOnlyIfIdle() )
    config -> writeEntry ("Start Playing Only If Idle", startPlayingOnlyIfIdle());
  else
    config -> deleteEntry ("Start Playing Only If Idle");
  if ( showOpenDialog() )
    config -> writeEntry ("Show Open Dialog", showOpenDialog());
  else
    config -> deleteEntry ("Show Open Dialog");
  if ( disableScreenSaver() )
    config -> deleteEntry ("Disable Screen Saver");
  else
    config -> writeEntry ("Disable Screen Saver", disableScreenSaver());
  if ( allowDuplicateEntries() )
    config -> writeEntry ("Allow Duplicate Entries", allowDuplicateEntries());
  else
    config -> deleteEntry ("Allow Duplicate Entries");
  if ( playlistSizeLimit() == 100 )
    config -> deleteEntry ("Playlist Size Limit");
  else
    config -> writeEntry ("Playlist Size Limit", playlistSizeLimit());
  if ( cacheSizeLimit() == 1000 )
    config -> deleteEntry ("Cache Size Limit");
  else
    config -> writeEntry ("Cache Size Limit", cacheSizeLimit());
  if ( rememberWithShift() )
    config -> deleteEntry ("Remember With Shift");
  else
    config -> writeEntry ("Remember With Shift", rememberWithShift());
  if ( rememberSize() )
    config -> writeEntry ("Remember Size", rememberSize());
  else
    config -> deleteEntry ("Remember Size");
  if ( rememberAspect() )
    config -> writeEntry ("Remember Aspect", rememberAspect());
  else
    config -> deleteEntry ("Remember Aspect");
  if ( rememberFullScreen() )
    config -> writeEntry ("Remember Full Screen", rememberFullScreen());
  else
    config -> deleteEntry ("Remember Full Screen");
  if ( rememberMaximized() )
    config -> writeEntry ("Remember Maximized", rememberMaximized());
  else
    config -> deleteEntry ("Remember Maximized");
  if ( rememberMaintainAspect() )
    config -> writeEntry ("Remember Maintain Aspect", rememberMaintainAspect());
  else
    config -> deleteEntry ("Remember Maintain Aspect");
  if ( rememberVolume() )
    config -> writeEntry ("Remember Volume", rememberVolume());
  else
    config -> deleteEntry ("Remember Volume");
  if ( rememberAudioDelay() )
    config -> deleteEntry ("Remember Audio Delay");
  else
    config -> writeEntry ("Remember Audio Delay", rememberAudioDelay());
  if ( rememberFrameDrop() )
    config -> writeEntry ("Remember Frame Drop", rememberFrameDrop());
  else
    config -> deleteEntry ("Remember Frame Drop");
  if ( rememberContrast() )
    config -> writeEntry ("Remember Contrast", rememberContrast());
  else
    config -> deleteEntry ("Remember Contrast");
  if ( rememberBrightness() )
    config -> writeEntry ("Remember Brightness", rememberBrightness());
  else
    config -> deleteEntry ("Remember Brightness");
  if ( rememberHue() )
    config -> writeEntry ("Remember Hue", rememberHue());
  else
    config -> deleteEntry ("Remember Hue");
  if ( rememberSaturation() )
    config -> writeEntry ("Remember Saturation", rememberSaturation());
  else
    config -> deleteEntry ("Remember Saturation");
  if ( rememberSubtitleUrl() )
    config -> deleteEntry ("Remember Subtitle URL");
  else
    config -> writeEntry ("Remember Subtitle URL", rememberSubtitleUrl());
  if ( rememberSubtitleVisibility() )
    config -> writeEntry ("Remember Subtitle Visibility", rememberSubtitleVisibility());
  else
    config -> deleteEntry ("Remember Subtitle Visibility");
  if ( rememberSubtitlePosition() )
    config -> writeEntry ("Remember Subtitle Position", rememberSubtitlePosition());
  else
    config -> deleteEntry ("Remember Subtitle Position");
  if ( rememberSubtitleDelay() )
    config -> deleteEntry ("Remember Subtitle Delay");
  else
    config -> writeEntry ("Remember Subtitle Delay", rememberSubtitleDelay());
  if ( preferredSliderLength() == 200 )
    config -> deleteEntry ("Preferred Slider Length");
  else
    config -> writeEntry ("Preferred Slider Length", preferredSliderLength());
  if ( minimumSliderLength() == 100 )
    config -> deleteEntry ("Minimum Slider Length");
  else
    config -> writeEntry ("Minimum Slider Length", minimumSliderLength());
  if ( progressNormalSeek() == 1 )
    config -> deleteEntry ("Normal Seek");
  else
    config -> writeEntry ("Normal Seek", progressNormalSeek());
  if ( progressNormalSeekUnits() == 0 )
    config -> deleteEntry ("Normal Seek Units");
  else
    config -> writeEntry ("Normal Seek Units", progressNormalSeekUnits());
  if ( progressFastSeek() == 10 )
    config -> deleteEntry ("Fast Seek");
  else
    config -> writeEntry ("Fast Seek", progressFastSeek());
  if ( progressFastSeekUnits() == 0 )
    config -> deleteEntry ("Fast Seek Units");
  else
    config -> writeEntry ("Fast Seek Units", progressFastSeekUnits());
  if ( progressMarks() == 10 )
    config -> deleteEntry ("Progress Marks");
  else
    config -> writeEntry ("Progress Marks", progressMarks());
  if ( volumeMinimum() == 0 )
    config -> deleteEntry ("Volume Minimum");
  else
    config -> writeEntry ("Volume Minimum", volumeMinimum());
  if ( volumeMaximum() == 100 )
    config -> deleteEntry ("Volume Maximum");
  else
    config -> writeEntry ("Volume Maximum", volumeMaximum());
  if ( volumeMarks() == 10 )
    config -> deleteEntry ("Volume Marks");
  else
    config -> writeEntry ("Volume Marks", volumeMarks());
  if ( volumeStep() == 1 )
    config -> deleteEntry ("Volume Step");
  else
    config -> writeEntry ("Volume Step", volumeStep());
  if ( initialVolume() == 50 )
    config -> deleteEntry ("Volume Default");
  else
    config -> writeEntry ("Volume Default", initialVolume());
  if ( volumeReset() )
    config -> writeEntry ("Volume Reset", volumeReset());
  else
    config -> deleteEntry ("Volume Reset");
  if ( volumeEvery() == 0 )
    config -> deleteEntry ("Volume Every");
  else
    config -> writeEntry ("Volume Every", volumeEvery());
  if ( volumeReset() || volumeDefault() == 50 )
    config -> deleteEntry ("Volume");
  else
    config -> writeEntry ("Volume", volumeDefault());
  if ( volumeReset() || ! mute() )
    config -> deleteEntry ("Mute");
  else
    config -> writeEntry ("Mute", mute());
  if ( audioDelayDefault() == 0 )
    config -> deleteEntry ("Audio Delay");
  else
    config -> writeEntry ("Audio Delay", audioDelayDefault());
  if ( contrastMinimum() == -100 )
    config -> deleteEntry ("Contrast Minimum");
  else
    config -> writeEntry ("Contrast Minimum", contrastMinimum());
  if ( contrastMaximum() == 100 )
    config -> deleteEntry ("Contrast Maximum");
  else
    config -> writeEntry ("Contrast Maximum", contrastMaximum());
  if ( contrastMarks() == 20 )
    config -> deleteEntry ("Contrast Marks");
  else
    config -> writeEntry ("Contrast Marks", contrastMarks());
  if ( contrastStep() == 1 )
    config -> deleteEntry ("Contrast Step");
  else
    config -> writeEntry ("Contrast Step", contrastStep());
  if ( initialContrast() == 0 )
    config -> deleteEntry ("Contrast Default");
  else
    config -> writeEntry ("Contrast Default", initialContrast());
  if ( contrastReset() )
    config -> writeEntry ("Contrast Reset", contrastReset());
  else
    config -> deleteEntry ("Contrast Reset");
  if ( contrastEvery() == 0 )
    config -> deleteEntry ("Contrast Every");
  else
    config -> writeEntry ("Contrast Every", contrastEvery());
  if ( contrastReset() || contrastDefault() == 0 )
    config -> deleteEntry ("Contrast");
  else
    config -> writeEntry ("Contrast", contrastDefault());
  if ( brightnessMinimum() == -50 )
    config -> deleteEntry ("Brightness Minimum");
  else
    config -> writeEntry ("Brightness Minimum", brightnessMinimum());
  if ( brightnessMaximum() == 50 )
    config -> deleteEntry ("Brightness Maximum");
  else
    config -> writeEntry ("Brightness Maximum", brightnessMaximum());
  if ( brightnessMarks() == 10 )
    config -> deleteEntry ("Brightness Marks");
  else
    config -> writeEntry ("Brightness Marks", brightnessMarks());
  if ( brightnessStep() == 1 )
    config -> deleteEntry ("Brightness Step");
  else
    config -> writeEntry ("Brightness Step", brightnessStep());
  if ( initialBrightness() == 0 )
    config -> deleteEntry ("Brightness Default");
  else
    config -> writeEntry ("Brightness Default", initialBrightness());
  if ( brightnessReset() )
    config -> writeEntry ("Brightness Reset", brightnessReset());
  else
    config -> deleteEntry ("Brightness Reset");
  if ( brightnessEvery() == 0 )
    config -> deleteEntry ("Brightness Every");
  else
    config -> writeEntry ("Brightness Every", brightnessEvery());
  if ( brightnessReset() || brightnessDefault() == 0 )
    config -> deleteEntry ("Brightness");
  else
    config -> writeEntry ("Brightness", brightnessDefault());
  if ( hueMinimum() == -100 )
    config -> deleteEntry ("Hue Minimum");
  else
    config -> writeEntry ("Hue Minimum", hueMinimum());
  if ( hueMaximum() == 100 )
    config -> deleteEntry ("Hue Maximum");
  else
    config -> writeEntry ("Hue Maximum", hueMaximum());
  if ( hueMarks() == 20 )
    config -> deleteEntry ("Hue Marks");
  else
    config -> writeEntry ("Hue Marks", hueMarks());
  if ( hueStep() == 1 )
    config -> deleteEntry ("Hue Step");
  else
    config -> writeEntry ("Hue Step", hueStep());
  if ( initialHue() == 0 )
    config -> deleteEntry ("Hue Default");
  else
    config -> writeEntry ("Hue Default", initialHue());
  if ( hueReset() )
    config -> writeEntry ("Hue Reset", hueReset());
  else
    config -> deleteEntry ("Hue Reset");
  if ( hueEvery() == 0 )
    config -> deleteEntry ("Hue Every");
  else
    config -> writeEntry ("Hue Every", hueEvery());
  if ( hueReset() || hueDefault() == 0 )
    config -> deleteEntry ("Hue");
  else
    config -> writeEntry ("Hue", hueDefault());
  if ( saturationMinimum() == -100 )
    config -> deleteEntry ("Saturation Minimum");
  else
    config -> writeEntry ("Saturation Minimum", saturationMinimum());
  if ( saturationMaximum() == 100 )
    config -> deleteEntry ("Saturation Maximum");
  else
    config -> writeEntry ("Saturation Maximum", saturationMaximum());
  if ( saturationMarks() == 20 )
    config -> deleteEntry ("Saturation Marks");
  else
    config -> writeEntry ("Saturation Marks", saturationMarks());
  if ( saturationStep() == 1 )
    config -> deleteEntry ("Saturation Step");
  else
    config -> writeEntry ("Saturation Step", saturationStep());
  if ( initialSaturation() == 0 )
    config -> deleteEntry ("Saturation Default");
  else
    config -> writeEntry ("Saturation Default", initialSaturation());
  if ( saturationReset() )
    config -> writeEntry ("Saturation Reset", saturationReset());
  else
    config -> deleteEntry ("Saturation Reset");
  if ( saturationEvery() == 0 )
    config -> deleteEntry ("Saturation Every");
  else
    config -> writeEntry ("Saturation Every", saturationEvery());
  if ( saturationReset() || saturationDefault() == 0 )
    config -> deleteEntry ("Saturation");
  else
    config -> writeEntry ("Saturation", saturationDefault());
  if ( subtitlePositionStep() == 1 )
    config -> deleteEntry ("Subtitle Position Step");
  else
    config -> writeEntry ("Subtitle Position Step", subtitlePositionStep());
  if ( subtitleDelayStep() == 0.1 )
    config -> deleteEntry ("Subtitle Delay Step");
  else
    config -> writeEntry ("Subtitle Delay Step", subtitleDelayStep());
  if ( autoloadAqtSubtitles() )
    config -> deleteEntry ("Autoload Aqt Subtitles");
  else
    config -> writeEntry ("Autoload Aqt Subtitles", autoloadAqtSubtitles());
  if ( autoloadJssSubtitles() )
    config -> deleteEntry ("Autoload Jss Subtitles");
  else
    config -> writeEntry ("Autoload Jss Subtitles", autoloadJssSubtitles());
  if ( autoloadRtSubtitles() )
    config -> deleteEntry ("Autoload Rt Subtitles");
  else
    config -> writeEntry ("Autoload Rt Subtitles", autoloadRtSubtitles());
  if ( autoloadSmiSubtitles() )
    config -> deleteEntry ("Autoload Smi Subtitles");
  else
    config -> writeEntry ("Autoload Smi Subtitles", autoloadSmiSubtitles());
  if ( autoloadSrtSubtitles() )
    config -> deleteEntry ("Autoload Srt Subtitles");
  else
    config -> writeEntry ("Autoload Srt Subtitles", autoloadSrtSubtitles());
  if ( autoloadSsaSubtitles() )
    config -> deleteEntry ("Autoload Ssa Subtitles");
  else
    config -> writeEntry ("Autoload Ssa Subtitles", autoloadSsaSubtitles());
  if ( autoloadSubSubtitles() )
    config -> deleteEntry ("Autoload Sub Subtitles");
  else
    config -> writeEntry ("Autoload Sub Subtitles", autoloadSubSubtitles());
  if ( autoloadTxtSubtitles() )
    config -> deleteEntry ("Autoload Txt Subtitles");
  else
    config -> writeEntry ("Autoload Txt Subtitles", autoloadTxtSubtitles());
  if ( autoloadUtfSubtitles() )
    config -> deleteEntry ("Autoload Utf Subtitles");
  else
    config -> writeEntry ("Autoload Utf Subtitles", autoloadUtfSubtitles());
  if ( autoloadOtherSubtitles() )
    config -> writeEntry ("Autoload Other Subtitles", autoloadOtherSubtitles());
  else
    config -> deleteEntry ("Autoload Other Subtitles");
  if ( autoloadExtensionList().isEmpty() )
    config -> deleteEntry ("Autoload Extension List");
  else
    config -> writeEntry ("Autoload Extension List", autoloadExtensionList());
  if ( subtitleAutoloadDefault() )
    config -> deleteEntry ("Autoload Subtitles");
  else
    config -> writeEntry ("Autoload Subtitles", subtitleAutoloadDefault());
  if ( subtitleVisibilityDefault() )
    config -> deleteEntry ("Show Subtitles");
  else
    config -> writeEntry ("Show Subtitles", subtitleVisibilityDefault());
  if ( subtitlePositionDefault() == 100 )
    config -> deleteEntry ("Subtitle Position");
  else
    config -> writeEntry ("Subtitle Position", subtitlePositionDefault());
  if ( subtitleDelayDefault() == 0 )
    config -> deleteEntry ("Subtitle Delay");
  else
    config -> writeEntry ("Subtitle Delay", subtitleDelayDefault());
  if ( showMessagesOnError() )
    config -> writeEntry ("Show Messages On Error", showMessagesOnError());
  else
    config -> deleteEntry ("Show Messages On Error");
  if ( showMessagesBeforePlaying() )
    config -> writeEntry ("Show Messages Before Playing", showMessagesBeforePlaying());
  else
    config -> deleteEntry ("Show Messages Before Playing");
  if ( showMessagesAfterPlaying() )
    config -> writeEntry ("Show Messages After Playing", showMessagesAfterPlaying());
  else
    config -> deleteEntry ("Show Messages After Playing");
  if ( clearMessagesBeforePlaying() )
    config -> deleteEntry ("Clear Messages Before Playing");
  else
    config -> writeEntry ("Clear Messages Before Playing", clearMessagesBeforePlaying());
  if ( videoDriverDefault() == "xv" )
    config -> deleteEntry ("Video Driver");
  else
    config -> writeEntry ("Video Driver", videoDriverDefault());
  if ( videoDriverFallbackDefault() )
    config -> deleteEntry ("Video Driver Fallback");
  else
    config -> writeEntry ("Video Driver Fallback", videoDriverFallbackDefault());
  if ( videoDeviceDefault().isEmpty() )
    config -> deleteEntry ("Video Device");
  else
    config -> writeEntry ("Video Device", videoDeviceDefault());
  if ( videoCodecDefault().isEmpty() )
    config -> deleteEntry ("Video Codec");
  else
    config -> writeEntry ("Video Codec", videoCodecDefault());
  if ( videoCodecFallbackDefault() )
    config -> deleteEntry ("Video Codec Fallback");
  else
    config -> writeEntry ("Video Codec Fallback", videoCodecFallbackDefault());
  if ( videoScalerDefault() == 0 )
    config -> deleteEntry ("Video Scaler");
  else
    config -> writeEntry ("Video Scaler", videoScalerDefault());
  if ( videoDoubleBufferingDefault() )
    config -> deleteEntry ("Double Buffering");
  else
    config -> writeEntry ("Double Buffering", videoDoubleBufferingDefault());
  if ( videoDirectRenderingDefault() )
    config -> writeEntry ("Direct Rendering", videoDirectRenderingDefault());
  else
    config -> deleteEntry ("Direct Rendering");
  if ( audioDelayStep() == 0.1 )
    config -> deleteEntry ("Audio Delay Step");
  else
    config -> writeEntry ("Audio Delay Step", audioDelayStep());
  if ( audioDriverDefault() == "oss" )
    config -> deleteEntry ("Audio Driver");
  else
    config -> writeEntry ("Audio Driver", audioDriverDefault());
  if ( audioDriverFallbackDefault() )
    config -> deleteEntry ("Audio Driver Fallback");
  else
    config -> writeEntry ("Audio Driver Fallback", audioDriverFallbackDefault());
  if ( audioDeviceDefault().isEmpty() )
    config -> deleteEntry ("Audio Device");
  else
    config -> writeEntry ("Audio Device", audioDeviceDefault());
  if ( audioCodecDefault().isEmpty() )
    config -> deleteEntry ("Audio Codec");
  else
    config -> writeEntry ("Audio Codec", audioCodecDefault());
  if ( audioCodecFallbackDefault() )
    config -> deleteEntry ("Audio Codec Fallback");
  else
    config -> writeEntry ("Audio Codec Fallback", audioCodecFallbackDefault());
  if ( executablePathDefault() == "mplayer" )
    config -> deleteEntry ("Executable Path");
  else
    config -> writeEntry ("Executable Path", executablePathDefault());
  if ( commandLineDefault().isEmpty() )
    config -> deleteEntry ("Command Line");
  else
    config -> writeEntry ("Command Line", commandLineDefault());
  if ( frameDropDefault() == 0 )
    config -> deleteEntry ("Frame Dropping");
  else
    config -> writeEntry ("Frame Dropping", frameDropDefault());
  if ( cacheDefault() == 2 )
    config -> deleteEntry ("Cache");
  else
    config -> writeEntry ("Cache", cacheDefault());
  if ( cacheSizeDefault() == 1024 )
    config -> deleteEntry ("Cache Size");
  else
    config -> writeEntry ("Cache Size", cacheSizeDefault());
  if ( buildNewIndexDefault() == 1 )
    config -> deleteEntry ("Build New Index");
  else
    config -> writeEntry ("Build New Index", buildNewIndexDefault());
  if ( useTemporaryFileDefault() )
    config -> writeEntry ("Use Temporary File For KIOSlave", useTemporaryFileDefault());
  else
    config -> deleteEntry ("Use Temporary File For KIOSlave");
  if ( useKioslaveForHttp() )
    config -> writeEntry ("Use KIOSlave For HTTP", useKioslaveForHttp());
  else
    config -> deleteEntry ("Use KIOSlave For HTTP");
  if ( useKioslaveForFtp() )
    config -> writeEntry ("Use KIOSlave For FTP", useKioslaveForFtp());
  else
    config -> deleteEntry ("Use KIOSlave For FTP");
  if ( useKioslaveForSmb() )
    config -> writeEntry ("Use KIOSlave For SMB", useKioslaveForSmb());
  else
    config -> deleteEntry ("Use KIOSlave For SMB");
  if ( osdLevelDefault() == 0 )
    config -> deleteEntry ("OSD Level");
  else
    config -> writeEntry ("OSD Level", osdLevelDefault());
  config -> sync();
  ((KPlayerSettings*) this) -> emitRefresh();
}

QString KPlayerSettings::audioDriverString (void) const
{
  QString driver (audioDriver());
  if ( ! driver.isEmpty() )
  {
    QString device (audioDevice());
    if ( ! device.isEmpty() )
      driver += ":" + device;
    if ( audioDriverFallback() )
      driver += ",";
  }
  return driver;
}

QString KPlayerSettings::audioCodecString (void) const
{
  QString codec (audioCodec());
  if ( ! codec.isEmpty() && audioCodecFallback() )
      codec += ",";
  return codec;
}

QString KPlayerSettings::videoDriverString (void) const
{
  QString driver (videoDriver());
  if ( ! driver.isEmpty() )
  {
    QString device (videoDevice());
    if ( ! device.isEmpty() )
      driver += ":" + device;
    if ( videoDriverFallback() )
      driver += ",";
  }
  return driver;
}

QString KPlayerSettings::videoCodecString (void) const
{
  QString codec (videoCodec());
  if ( ! codec.isEmpty() && videoCodecFallback() )
      codec += ",";
  return codec;
}

bool KPlayerSettings::fullScreen (void)
{
  if ( kPlayerEngine() -> stopped() )
    return false;
#ifdef DEBUG_KPLAYER_SETTINGS
  bool last_full_screen = m_last_full_screen;
#endif
  m_last_full_screen = ! fullScreenOverride() && m_properties && m_properties -> fullScreenOption() >= 0 ?
    m_properties -> fullScreenOption() == 1 : originalSize().isValid() ?
    fullScreenDefault() && hasVideo() : m_last_full_screen;
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::fullScreen " << originalSize().isValid() << " ? " << fullScreenDefault()
    << " && " << hasVideo() << " : " << last_full_screen << " = " << m_last_full_screen << "\n";
#endif
  return m_last_full_screen;
}

void KPlayerSettings::setFullScreen (bool full_screen)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setFullScreen (" << full_screen << ")\n";
#endif
  if ( (rememberFullScreen() || rememberWithShift() && shift()) && m_properties )
  {
    setFullScreenOverride (false);
    m_properties -> setFullScreenOption (full_screen == fullScreenDefault() ? -1 : full_screen ? 1 : 0);
  }
  else
  {
    setFullScreenDefault (full_screen);
    setFullScreenOverride (true);
  }
}

bool KPlayerSettings::maximized (void) const
{
  return ! maximizedOverride() && m_properties && m_properties -> fullScreenOption() >= 0 ?
    m_properties -> fullScreenOption() == 2 : maximizedDefault();
}

void KPlayerSettings::setMaximized (bool maximized)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setMaximized (" << maximized << ")\n";
#endif
  if ( (rememberMaximized() || rememberWithShift() && shift()) && m_properties )
  {
    setMaximizedOverride (false);
    m_properties -> setFullScreenOption (maximized == maximizedDefault() ? -1 : maximized ? 2 : 0);
  }
  else
  {
    setMaximizedDefault (maximized);
    setMaximizedOverride (true);
  }
}

bool KPlayerSettings::maintainAspect (void) const
{
  return ! maintainAspectOverride() && m_properties && m_properties -> maintainAspectOption() >= 0 ?
    m_properties -> maintainAspectOption() == 0 : maintainAspectDefault();
}

void KPlayerSettings::setMaintainAspect (bool maintain, QSize aspect)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setMaintainAspect (" << maintain << ", " << aspect.width() << "x" << aspect.height() << ")\n";
#endif
  if ( (rememberMaintainAspect() || rememberWithShift() && shift()) && m_properties )
  {
    setMaintainAspectOverride (false);
    m_properties -> setMaintainAspectOption (maintain == maintainAspectDefault() ? -1 : maintain ? 0 : 1);
  }
  else
  {
    setMaintainAspectDefault (maintain);
    setMaintainAspectOverride (true);
  }
  if ( ! maintain )
    return;
  setAspect (aspect);
  if ( (rememberAspect() || rememberWithShift() && shift()) && m_properties )
  {
    if ( ! aspect.isEmpty() && ! originalSize().isEmpty()
        && aspect.width() * originalSize().height() == aspect.height() * originalSize().width() )
      m_properties -> setDisplaySizeOption (0);
    else
    {
      m_properties -> setDisplaySizeOption (2);
      m_properties -> setDisplaySizeValue (aspect);
    }
    setAspectOverride (false);
  }
}

QSize KPlayerSettings::originalAspect (void) const
{
  return m_properties && m_properties -> displaySizeOption() ? m_properties -> displaySizeValue() : originalSize();
}

void KPlayerSettings::setAspect (QSize aspect)
{
  if ( aspect.width() * m_aspect.height() == aspect.height() * m_aspect.width() || aspect.isEmpty() && m_aspect.isEmpty() )
    return;
  m_aspect = aspect;
  aspect = originalAspect();
  setAspectOverride (! aspect.isEmpty() && ! m_aspect.isEmpty() && aspect.width() * m_aspect.height() != aspect.height() * m_aspect.width());
}

void KPlayerSettings::setDisplaySize (QSize size)
{
  if ( size == m_display_size )
    return;
  setDisplaySizeOverride (true);
  m_display_size = size;
}

int KPlayerSettings::volume (void) const
{
  return limit (volumeOverride() || ! m_properties ||
    m_properties -> volumeOption() == -1 ? volumeDefault() :
    m_properties -> volumeOption() == 1 ? volumeDefault() + m_properties -> volumeValue() :
    m_properties -> volumeOption() == 2 ? volumeDefault() - m_properties -> volumeValue() :
    m_properties -> volumeValue(), volumeMinimum(), volumeMaximum());
}

void KPlayerSettings::setVolume (int volume)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setVolume (" << volume << ")\n";
#endif
  if ( (rememberVolume() || rememberWithShift() && shift()) && m_properties )
  {
    setVolumeOverride (false);
    if ( volume > volumeDefault() )
    {
      m_properties -> setVolumeOption (1);
      m_properties -> setVolumeValue (volume - volumeDefault());
    }
    else if ( volume < volumeDefault() )
    {
      m_properties -> setVolumeOption (2);
      m_properties -> setVolumeValue (volumeDefault() - volume);
    }
    else
      m_properties -> setVolumeOption (-1);
  }
  else
  {
    if ( m_properties && m_properties -> volumeOption() == 1 && volume - m_properties -> volumeValue() >= volumeMinimum() )
    {
      setVolumeDefault (volume - m_properties -> volumeValue());
      setVolumeOverride (false);
    }
    else if ( m_properties && m_properties -> volumeOption() == 2 && volume + m_properties -> volumeValue() <= volumeMaximum() )
    {
      setVolumeDefault (volume + m_properties -> volumeValue());
      setVolumeOverride (false);
    }
    else
    {
      setVolumeDefault (volume);
      setVolumeOverride (true);
    }
  }
}

float KPlayerSettings::audioDelay (void) const
{
  return ! audioDelayOverride() && m_properties && m_properties -> audioDelayOption() >= 0 ?
    m_properties -> audioDelayValue() : audioDelayDefault();
}

void KPlayerSettings::setAudioDelay (float delay)
{
  if ( fabs (delay) < 0.0001 )
    delay = 0;
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setAudioDelay (" << delay << ")\n";
#endif
  if ( (rememberAudioDelay() || rememberWithShift() && shift()) && m_properties )
  {
    setAudioDelayOverride (false);
    if ( delay == audioDelayDefault() )
      m_properties -> setAudioDelayOption (-1);
    else
    {
      m_properties -> setAudioDelayOption (0);
      m_properties -> setAudioDelayValue (delay);
    }
  }
  else
  {
    setAudioDelayDefault (delay);
    setAudioDelayOverride (true);
  }
}

int KPlayerSettings::frameDrop (void) const
{
  return ! frameDropOverride() && m_properties && m_properties -> frameDropOption() >= 0 ?
    m_properties -> frameDropOption() : frameDropDefault();
}

void KPlayerSettings::setFrameDrop (int frame_drop)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setFrameDrop (" << frame_drop << ")\n";
#endif
  if ( (rememberFrameDrop() || rememberWithShift() && shift()) && m_properties )
  {
    setFrameDropOverride (false);
    m_properties -> setFrameDropOption (frame_drop == frameDropDefault() ? -1 : frame_drop);
  }
  else
  {
    setFrameDropDefault (frame_drop);
    setFrameDropOverride (true);
  }
}

int KPlayerSettings::contrast (void) const
{
  return limit (contrastOverride() || ! m_properties ||
    m_properties -> contrastOption() == -1 ? contrastDefault() :
    m_properties -> contrastOption() == 1 ? contrastDefault() + m_properties -> contrastValue() :
    m_properties -> contrastOption() == 2 ? contrastDefault() - m_properties -> contrastValue() :
    m_properties -> contrastValue(), contrastMinimum(), contrastMaximum());
}

void KPlayerSettings::setContrast (int contrast)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setContrast (" << contrast << ")\n";
#endif
  if ( (rememberContrast() || rememberWithShift() && shift()) && m_properties )
  {
    setContrastOverride (false);
    if ( contrast > contrastDefault() )
    {
      m_properties -> setContrastOption (1);
      m_properties -> setContrastValue (contrast - contrastDefault());
    }
    else if ( contrast < contrastDefault() )
    {
      m_properties -> setContrastOption (2);
      m_properties -> setContrastValue (contrastDefault() - contrast);
    }
    else
      m_properties -> setContrastOption (-1);
  }
  else
  {
    if ( m_properties && m_properties -> contrastOption() == 1 && contrast - m_properties -> contrastValue() >= contrastMinimum() )
    {
      setContrastDefault (contrast - m_properties -> contrastValue());
      setContrastOverride (false);
    }
    else if ( m_properties && m_properties -> contrastOption() == 2 && contrast + m_properties -> contrastValue() <= contrastMaximum() )
    {
      setContrastDefault (contrast + m_properties -> contrastValue());
      setContrastOverride (false);
    }
    else
    {
      setContrastDefault (contrast);
      setContrastOverride (true);
    }
  }
}

int KPlayerSettings::brightness (void) const
{
  return limit (brightnessOverride() || ! m_properties ||
    m_properties -> brightnessOption() == -1 ? brightnessDefault() :
    m_properties -> brightnessOption() == 1 ? brightnessDefault() + m_properties -> brightnessValue() :
    m_properties -> brightnessOption() == 2 ? brightnessDefault() - m_properties -> brightnessValue() :
    m_properties -> brightnessValue(), brightnessMinimum(), brightnessMaximum());
}

void KPlayerSettings::setBrightness (int brightness)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setBrightness (" << brightness << ")\n";
#endif
  if ( (rememberBrightness() || rememberWithShift() && shift()) && m_properties )
  {
    setBrightnessOverride (false);
    if ( brightness > brightnessDefault() )
    {
      m_properties -> setBrightnessOption (1);
      m_properties -> setBrightnessValue (brightness - brightnessDefault());
    }
    else if ( brightness < brightnessDefault() )
    {
      m_properties -> setBrightnessOption (2);
      m_properties -> setBrightnessValue (brightnessDefault() - brightness);
    }
    else
      m_properties -> setBrightnessOption (-1);
  }
  else
  {
    if ( m_properties && m_properties -> brightnessOption() == 1 && brightness - m_properties -> brightnessValue() >= brightnessMinimum() )
    {
      setBrightnessDefault (brightness - m_properties -> brightnessValue());
      setBrightnessOverride (false);
    }
    else if ( m_properties && m_properties -> brightnessOption() == 2 && brightness + m_properties -> brightnessValue() <= brightnessMaximum() )
    {
      setBrightnessDefault (brightness + m_properties -> brightnessValue());
      setBrightnessOverride (false);
    }
    else
    {
      setBrightnessDefault (brightness);
      setBrightnessOverride (true);
    }
  }
}

int KPlayerSettings::hue (void) const
{
  return limit (hueOverride() || ! m_properties ||
    m_properties -> hueOption() == -1 ? hueDefault() :
    m_properties -> hueOption() == 1 ? hueDefault() + m_properties -> hueValue() :
    m_properties -> hueOption() == 2 ? hueDefault() - m_properties -> hueValue() :
    m_properties -> hueValue(), hueMinimum(), hueMaximum());
}

void KPlayerSettings::setHue (int hue)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setHue (" << hue << ")\n";
#endif
  if ( (rememberHue() || rememberWithShift() && shift()) && m_properties )
  {
    setHueOverride (false);
    if ( hue > hueDefault() )
    {
      m_properties -> setHueOption (1);
      m_properties -> setHueValue (hue - hueDefault());
    }
    else if ( hue < hueDefault() )
    {
      m_properties -> setHueOption (2);
      m_properties -> setHueValue (hueDefault() - hue);
    }
    else
      m_properties -> setHueOption (-1);
  }
  else
  {
    if ( m_properties && m_properties -> hueOption() == 1 && hue - m_properties -> hueValue() >= hueMinimum() )
    {
      setHueDefault (hue - m_properties -> hueValue());
      setHueOverride (false);
    }
    else if ( m_properties && m_properties -> hueOption() == 2 && hue + m_properties -> hueValue() <= hueMaximum() )
    {
      setHueDefault (hue + m_properties -> hueValue());
      setHueOverride (false);
    }
    else
    {
      setHueDefault (hue);
      setHueOverride (true);
    }
  }
}

int KPlayerSettings::saturation (void) const
{
  return limit (saturationOverride() || ! m_properties ||
    m_properties -> saturationOption() == -1 ? saturationDefault() :
    m_properties -> saturationOption() == 1 ? saturationDefault() + m_properties -> saturationValue() :
    m_properties -> saturationOption() == 2 ? saturationDefault() - m_properties -> saturationValue() :
    m_properties -> saturationValue(), saturationMinimum(), saturationMaximum());
}

void KPlayerSettings::setSaturation (int saturation)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSaturation (" << saturation << ")\n";
#endif
  if ( (rememberSaturation() || rememberWithShift() && shift()) && m_properties )
  {
    setSaturationOverride (false);
    if ( saturation > saturationDefault() )
    {
      m_properties -> setSaturationOption (1);
      m_properties -> setSaturationValue (saturation - saturationDefault());
    }
    else if ( saturation < saturationDefault() )
    {
      m_properties -> setSaturationOption (2);
      m_properties -> setSaturationValue (saturationDefault() - saturation);
    }
    else
      m_properties -> setSaturationOption (-1);
  }
  else
  {
    if ( m_properties && m_properties -> saturationOption() == 1 && saturation - m_properties -> saturationValue() >= saturationMinimum() )
    {
      setSaturationDefault (saturation - m_properties -> saturationValue());
      setSaturationOverride (false);
    }
    else if ( m_properties && m_properties -> saturationOption() == 2 && saturation + m_properties -> saturationValue() <= saturationMaximum() )
    {
      setSaturationDefault (saturation + m_properties -> saturationValue());
      setSaturationOverride (false);
    }
    else
    {
      setSaturationDefault (saturation);
      setSaturationOverride (true);
    }
  }
}

void KPlayerSettings::setSubtitleUrl (const KURL& url)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSubtitleUrl (" << url.url() << ")\n";
#endif
  if ( (rememberSubtitleUrl() || rememberWithShift() && shift()) && m_properties )
  {
    setSubtitleUrlOverride (false);
    m_properties -> setSubtitleAutoloadOption (1);
    m_properties -> setSubtitleUrl (url);
  }
  else
  {
    setSubtitleUrlOverride (true);
    setSubtitleUrlDefault (url);
  }
}

bool KPlayerSettings::subtitleVisibility (void) const
{
  return ! subtitleVisibilityOverride() && m_properties && m_properties -> subtitleVisibilityOption() >= 0 ?
    m_properties -> subtitleVisibilityOption() == 0 : subtitleVisibilityDefault();
}

void KPlayerSettings::setSubtitleVisibility (bool visibility)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSubtitleVisibility (" << visibility << ")\n";
#endif
  if ( (rememberSubtitleVisibility() || rememberWithShift() && shift()) && m_properties )
  {
    setSubtitleVisibilityOverride (false);
    m_properties -> setSubtitleVisibilityOption (visibility == subtitleVisibilityDefault() ? -1 : visibility ? 0 : 1);
  }
  else
  {
    setSubtitleVisibilityDefault (visibility);
    setSubtitleVisibilityOverride (true);
  }
}

int KPlayerSettings::subtitlePosition (void) const
{
  return ! subtitlePositionOverride() && m_properties && m_properties -> subtitlePositionOption() >= 0 ?
    m_properties -> subtitlePositionValue() : subtitlePositionDefault();
}

void KPlayerSettings::setSubtitlePosition (int position)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSubtitlePosition (" << position << ")\n";
#endif
  if ( (rememberSubtitlePosition() || rememberWithShift() && shift()) && m_properties )
  {
    setSubtitlePositionOverride (false);
    if ( position == subtitlePositionDefault() )
      m_properties -> setSubtitlePositionOption (-1);
    else
    {
      m_properties -> setSubtitlePositionOption (0);
      m_properties -> setSubtitlePositionValue (position);
    }
  }
  else
  {
    setSubtitlePositionDefault (position);
    setSubtitlePositionOverride (true);
  }
}

float KPlayerSettings::subtitleDelay (void) const
{
  return ! subtitleDelayOverride() && m_properties && m_properties -> subtitleDelayOption() >= 0 ?
    m_properties -> subtitleDelayValue() : subtitleDelayDefault();
}

void KPlayerSettings::setSubtitleDelay (float delay)
{
  if ( fabs (delay) < 0.0001 )
    delay = 0;
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setSubtitleDelay (" << delay << ")\n";
#endif
  if ( (rememberSubtitleDelay() || rememberWithShift() && shift()) && m_properties )
  {
    setSubtitleDelayOverride (false);
    if ( delay == subtitleDelayDefault() )
      m_properties -> setSubtitleDelayOption (-1);
    else
    {
      m_properties -> setSubtitleDelayOption (0);
      m_properties -> setSubtitleDelayValue (delay);
    }
  }
  else
  {
    setSubtitleDelayDefault (delay);
    setSubtitleDelayOverride (true);
  }
}

QString KPlayerSettings::caption (void) const
{
  if ( ! showFilePath() && m_properties && ! m_properties -> name().isEmpty() )
    return m_properties -> name();
  //if ( ! showFilePath() && ! url().fileName().isEmpty() )
  //  return url().fileName();
  if ( url().isLocalFile() && ! url().path().isEmpty() )
    return url().path();
  return url().prettyURL (0, KURL::StripFileProtocol);
}

int KPlayerSettings::normalSeek (void) const
{
  int seek = progressNormalSeekUnits() ? progressNormalSeek()
    : hasLength() ? int (m_properties -> length() * progressNormalSeek() / 100 + 0.5) : 10;
  if ( seek == 0 )
    seek = 1;
  return seek;
}

int KPlayerSettings::fastSeek (void) const
{
  int seek = progressFastSeekUnits() ? progressFastSeek()
    : hasLength() ? int (m_properties -> length() * progressFastSeek() / 100 + 0.5) : 60;
  if ( seek == 0 )
    seek = 2;
  return seek;
}

bool KPlayerSettings::playlist (void) const
{
  if ( ! m_properties )
    return false;
  if ( m_properties -> playlistOption() != 0 )
    return m_properties -> playlistOption() == 1;
  return re_playlist_url.search (url().url()) >= 0;
}

bool KPlayerSettings::useKioslave (void) const
{
  if ( ! m_properties )
    return false;
  if ( m_properties -> useKioslaveOption() != 0 )
    return m_properties -> useKioslaveOption() == 1;
  if ( re_http_url.search (url().url()) >= 0 )
    return useKioslaveForHttp();
  if ( re_ftp_url.search (url().url()) >= 0 )
    return useKioslaveForFtp();
  if ( re_smb_url.search (url().url()) >= 0 )
    return useKioslaveForSmb();
  return re_remote_url.search (url().url()) >= 0 && re_mplayer_url.search (url().url()) < 0;
}

bool KPlayerSettings::isAspect (QSize size)
{
  return ! size.isEmpty() && ! aspect().isEmpty() && size.width() * aspect().height() == size.height() * aspect().width();
}

bool KPlayerSettings::isZoomFactor (int m, int d)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::isZoomFactor (" << m << ", " << d << ") " << originalSize().width() << "x" << originalSize().height()
    << " " << displaySize().width() << "x" << displaySize().height()
    << " " << aspect().width() << "x" << aspect().height() << "\n";
#endif
  if ( fullScreen() || maximized() || originalSize().isEmpty() )
    return false;
  QSize size (originalSize() * m / d);
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
    if ( (user_zoom || user_resize) && m_properties )
    {
      if ( rememberSize() || rememberWithShift() && shift() )
      {
        m_properties -> setDisplaySizeOption (1);
        m_properties -> setDisplaySizeValue (size);
        setDisplaySizeOverride (false);
        setAspectOverride (false);
      }
      else if ( rememberAspect() )
      {
        m_properties -> setDisplaySizeOption (2);
        m_properties -> setDisplaySizeValue (size);
        setAspectOverride (false);
      }
    }
  }
  return size;
}

/*QSize KPlayerSettings::displaySizeDefault (void)
{
  if ( m_properties -> displaySizeOption() == 1 )
    return m_properties -> displaySizeValue();
  if ( m_properties -> originalSize().isEmpty() )
    return QSize (minimumInitialWidth(), 0);
  QSize size (m_properties -> originalSize());
  if ( ! aspect().isEmpty() )
    size.setHeight (size.width() * aspect().height() / aspect().width());
  return size;
}*/

bool KPlayerSettings::setInitialDisplaySize (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings::setInitialDisplaySize\n";
#endif
  if ( ! originalSize().isValid() )
    return false;
  if ( ! aspectOverride() )
  {
    setAspect (originalAspect());
    setAspectOverride (false);
#ifdef DEBUG_KPLAYER_SETTINGS
    kdDebugTime() << "Settings: Initial aspect: " << aspect().width() << "x" << aspect().height() << " " << maintainAspect() << "\n";
#endif
  }
  if ( displaySizeOverride() )
    return ! aspectOverride();
  QSize size;
  if ( m_properties -> displaySizeOption() == 1 )
    size = m_properties -> displaySizeValue();
  else if ( originalSize().isEmpty() )
    size = QSize (minimumInitialWidth(), 0);
  else
  {
    int d = 1, n = (minimumInitialWidth() - 1) / originalSize().width();
    if ( n > 0 && originalSize().width() * n + originalSize().width() / 2 >= minimumInitialWidth() )
      n *= d = 2;
#ifdef DEBUG_KPLAYER_SETTINGS
    kdDebugTime() << "Initial Zoom Factor " << (n + 1) << " / " << d << "\n";
#endif
    size = adjustSize (originalSize() * (n + 1) / d);
    //if ( m_properties -> displaySizeOption() == 2 && m_properties -> displaySizeValue().width() > 0 )
    //  size.setHeight (size.width() * m_properties -> displaySizeValue().height() / m_properties -> displaySizeValue().width());
  }
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Settings: Initial size: " << size.width() << "x" << size.height() << "\n";
#endif
  //if ( fullScreen() && size.isEmpty() )
  //  setFullScreenDefault (false);
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
