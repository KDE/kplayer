/***************************************************************************
                          kplayersettings.h
                          -----------------
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

#ifndef KPLAYERSETTINGS_H
#define KPLAYERSETTINGS_H

#include <limits.h>
#include <math.h>
#include <qapplication.h>
#include <qobject.h>

#include "kplayerproperties.h"

class KURL;

/** The KPlayer settings.
  *@author kiriuja
  */
class KPlayerSettings : public QObject
{
  Q_OBJECT

public:
  KPlayerSettings (void);
  virtual ~KPlayerSettings();

  /** Returns the current KPlayerProperties object.
   */
  KPlayerProperties* properties (void)
    { return m_properties; }

  /** Loads the given URL settings and makes the necessary adjustments.
   */
  void load (const KURL&);

  void load (void);
  void save (void) const;
  void defaults (void);

  // Basic settings

  QString caption (void) const;

  int normalSeek (void) const;
  int fastSeek (void) const;

  KURL url (void) const
    { return m_properties ? m_properties -> url() : KURL(); }

  int volume (void) const;
  void setVolume (int);

  int actualVolume (void) const
    { return mute() ? 0 : volume(); }

  int volumeDefault (void) const
    { return m_volume_default; }
  void setVolumeDefault (int volume)
    { m_volume_default = limit (volume, volumeMinimum(), volumeMaximum()); }

  bool volumeOverride (void) const
    { return m_volume_override; }
  void setVolumeOverride (bool override)
    { m_volume_override = override; }

  bool mute (void) const
    { return m_mute; }
  void setMute (bool mute)
    { m_mute = mute; }

  float audioDelay (void) const;
  void setAudioDelay (float);

  float audioDelayDefault (void) const
    { return m_audio_delay_default; }
  void setAudioDelayDefault (float delay)
    { m_audio_delay_default = fabs (delay) < 0.0001 ? 0 : delay; }

  bool audioDelayOverride (void) const
    { return m_audio_delay_override; }
  void setAudioDelayOverride (bool override)
    { m_audio_delay_override = override; }

  int contrast (void) const;
  void setContrast (int);

  int contrastDefault (void) const
    { return m_contrast_default; }
  void setContrastDefault (int contrast)
    { m_contrast_default = limit (contrast, contrastMinimum(), contrastMaximum()); }

  bool contrastOverride (void) const
    { return m_contrast_override; }
  void setContrastOverride (bool override)
    { m_contrast_override = override; }

  int brightness (void) const;
  void setBrightness (int);

  int brightnessDefault (void) const
    { return m_brightness_default; }
  void setBrightnessDefault (int brightness)
    { m_brightness_default = limit (brightness, brightnessMinimum(), brightnessMaximum()); }

  bool brightnessOverride (void) const
    { return m_brightness_override; }
  void setBrightnessOverride (bool override)
    { m_brightness_override = override; }

  int hue (void) const;
  void setHue (int);

  int hueDefault (void) const
    { return m_hue_default; }
  void setHueDefault (int hue)
    { m_hue_default = limit (hue, hueMinimum(), hueMaximum()); }

  bool hueOverride (void) const
    { return m_hue_override; }
  void setHueOverride (bool override)
    { m_hue_override = override; }

  int saturation (void) const;
  void setSaturation (int);

  int saturationDefault (void) const
    { return m_saturation_default; }
  void setSaturationDefault (int saturation)
    { m_saturation_default = limit (saturation, saturationMinimum(), saturationMaximum()); }

  bool saturationOverride (void) const
    { return m_saturation_override; }
  void setSaturationOverride (bool override)
    { m_saturation_override = override; }

  bool playlist (void) const;

  QSize originalAspect (void) const;

  QSize aspect (void) const
    { return m_aspect; }
  void setAspect (QSize aspect);

  bool aspectOverride (void) const
    { return m_aspect_override; }
  void setAspectOverride (bool override)
    { m_aspect_override = override; }

  QSize displaySize (void) const
    { return m_display_size; }
  void setDisplaySize (QSize size);

  bool displaySizeOverride (void) const
    { return m_display_size_override; }
  void setDisplaySizeOverride (bool override)
    { m_display_size_override = override; }

  QSize adjustDisplaySize (bool, bool);

  //QSize displaySizeDefault (void);
  bool setInitialDisplaySize (void);

  bool isAspect (QSize);
  bool isZoomFactor (int, int = 1);

  bool fullScreen (void);
  void setFullScreen (bool);

  bool fullScreenDefault (void) const
    { return m_full_screen_default; }
  void setFullScreenDefault (bool full_screen)
    { m_full_screen_default = full_screen; }

  bool fullScreenOverride (void) const
    { return m_full_screen_override; }
  void setFullScreenOverride (bool override)
    { m_full_screen_override = override; }

  bool maximized (void) const;
  void setMaximized (bool);

  bool maximizedDefault (void) const
    { return m_maximized_default; }
  void setMaximizedDefault (bool maximized)
    { m_maximized_default = maximized; }

  bool maximizedOverride (void) const
    { return m_maximized_override; }
  void setMaximizedOverride (bool override)
    { m_maximized_override = override; }

  bool maintainAspect (void) const;
  void setMaintainAspect (bool, QSize);

  bool maintainAspectDefault (void) const
    { return m_maintain_aspect_default; }
  void setMaintainAspectDefault (bool maintain)
    { m_maintain_aspect_default = maintain; }

  bool maintainAspectOverride (void) const
    { return m_maintain_aspect_override; }
  void setMaintainAspectOverride (bool override)
    { m_maintain_aspect_override = override; }

  bool constrainedSize (void)
    { return fullScreen() || maximized() || ! resizeAutomatically(); }

  QSize constrainSize (QSize size) const;
  QSize adjustSize (QSize size, bool horizontally = false) const;

  QSize originalSize (void) const
    { return m_properties ? m_properties -> originalSize() : QSize(); }

  bool hasVideo (void) const
    { return ! originalSize().isEmpty() || m_properties && m_properties -> displaySizeOption() == 1
        && ! m_properties -> displaySizeValue().isEmpty(); }

  bool hasSubtitles (void) const
    { return hasVideo() && ! subtitleUrl().isEmpty(); }

  float length (void) const
    { return m_properties ? m_properties -> length() : 0; }
  QString lengthString (void) const
    { return m_properties ? m_properties -> lengthString() : QString(); }

  bool hasLength (void) const
    { return length() > 0; }

  bool shift (void) const
    { return m_shift; }
  void setShift (bool shift)
    { m_shift = shift; }

  // General settings

  bool resizeAutomatically (void) const
    { return m_resize_automatically; }
  void setResizeAutomatically (bool resize)
    { m_resize_automatically = resize; }

  int minimumInitialWidth (void) const
    { return m_minimum_initial_width; }
  void setMinimumInitialWidth (int width)
    { m_minimum_initial_width = limit (width, 200, QApplication::desktop() -> width()); }

  int recentFileListSize (void) const
    { return m_recent_file_list_size; }
  void setRecentFileListSize (int size)
    { m_recent_file_list_size = limit (size, 0); }

  bool showFilePath (void) const
    { return m_show_file_path; }
  void setShowFilePath (bool show)
    { m_show_file_path = show; }

  bool showOpenDialog (void) const
    { return m_show_open_dialog; }
  void setShowOpenDialog (bool show)
    { m_show_open_dialog = show; }

  bool disableScreenSaver (void) const
    { return m_disable_screen_saver; }
  void setDisableScreenSaver (bool disable)
    {  m_disable_screen_saver = disable; }

  // Playlist settings

  bool startPlayingImmediately (void) const
    { return m_start_playing_immediately; }
  void setStartPlayingImmediately (bool start)
    { m_start_playing_immediately = start; }

  bool startPlayingOnlyIfIdle (void) const
    { return m_start_playing_only_if_idle; }
  void setStartPlayingOnlyIfIdle (bool only)
    { m_start_playing_only_if_idle = only; }

  bool allowDuplicateEntries (void) const
    { return m_allow_duplicate_entries; }
  void setAllowDuplicateEntries (bool allow)
    { m_allow_duplicate_entries = allow; }

  int playlistSizeLimit (void) const
    { return m_playlist_size_limit; }
  void setPlaylistSizeLimit (int size)
    { m_playlist_size_limit = limit (size, 10, 1000); }

  int cacheSizeLimit (void) const
    { return m_cache_size_limit; }
  void setCacheSizeLimit (int size)
    { m_cache_size_limit = limit (size, 10, 10000); }

  // Control settings

  bool rememberWithShift (void) const
    { return m_remember_with_shift; }
  void setRememberWithShift (bool remember)
    { m_remember_with_shift = remember; }

  bool rememberSize (void) const
    { return m_remember_size; }
  void setRememberSize (bool remember)
    { m_remember_size = remember; }

  bool rememberAspect (void) const
    { return m_remember_aspect; }
  void setRememberAspect (bool remember)
    { m_remember_aspect = remember; }

  bool rememberFullScreen (void) const
    { return m_remember_full_screen; }
  void setRememberFullScreen (bool remember)
    { m_remember_full_screen = remember; }

  bool rememberMaximized (void) const
    { return m_remember_maximized; }
  void setRememberMaximized (bool remember)
    { m_remember_maximized = remember; }

  bool rememberMaintainAspect (void) const
    { return m_remember_maintain_aspect; }
  void setRememberMaintainAspect (bool remember)
    { m_remember_maintain_aspect = remember; }

  bool rememberVolume (void) const
    { return m_remember_volume; }
  void setRememberVolume (bool remember)
    { m_remember_volume = remember; }

  bool rememberAudioDelay (void) const
    { return m_remember_audio_delay; }
  void setRememberAudioDelay (bool remember)
    { m_remember_audio_delay = remember; }

  bool rememberFrameDrop (void) const
    { return m_remember_frame_drop; }
  void setRememberFrameDrop (bool remember)
    { m_remember_frame_drop = remember; }

  bool rememberContrast (void) const
    { return m_remember_contrast; }
  void setRememberContrast (bool remember)
    { m_remember_contrast = remember; }

  bool rememberBrightness (void) const
    { return m_remember_brightness; }
  void setRememberBrightness (bool remember)
    { m_remember_brightness = remember; }

  bool rememberHue (void) const
    { return m_remember_hue; }
  void setRememberHue (bool remember)
    { m_remember_hue = remember; }

  bool rememberSaturation (void) const
    { return m_remember_saturation; }
  void setRememberSaturation (bool remember)
    { m_remember_saturation = remember; }

  bool rememberSubtitleUrl (void) const
    { return m_remember_subtitle_url; }
  void setRememberSubtitleUrl (bool remember)
    { m_remember_subtitle_url = remember; }

  bool rememberSubtitleVisibility (void) const
    { return m_remember_subtitle_visibility; }
  void setRememberSubtitleVisibility (bool remember)
    { m_remember_subtitle_visibility = remember; }

  bool rememberSubtitlePosition (void) const
    { return m_remember_subtitle_position; }
  void setRememberSubtitlePosition (bool remember)
    { m_remember_subtitle_position = remember; }

  bool rememberSubtitleDelay (void) const
    { return m_remember_subtitle_delay; }
  void setRememberSubtitleDelay (bool remember)
    { m_remember_subtitle_delay = remember; }

  // Volume settings

  int volumeMinimum (void) const
    { return m_volume_minimum; }
  void setVolumeMinimum (int volume)
    { m_volume_minimum = limit (volume, 0, 100); }

  int volumeMaximum (void) const
    { return m_volume_maximum; }
  void setVolumeMaximum (int volume)
    { m_volume_maximum = limit (volume, 0, 100); }

  void setVolumeMinimumMaximum (int minimum, int maximum)
  {
    m_volume_minimum = minimum;
    m_volume_maximum = maximum;
    limit4 (m_volume_minimum, m_volume_maximum, 0, 100);
    setVolumeDefault (limit (volumeDefault(), volumeMinimum(), volumeMaximum()));
  }

  int volumeMarks (void) const
    { return m_volume_marks; }
  void setVolumeMarks (int volume)
    { m_volume_marks = limit (volume, 1, m_volume_maximum - m_volume_minimum); }

  int volumeStep (void) const
    { return m_volume_step; }
  void setVolumeStep (int volume)
    { m_volume_step = limit (volume, 1, m_volume_maximum - m_volume_minimum); }

  bool volumeReset (void) const
    { return m_volume_reset; }
  void setVolumeReset (bool volume)
    { m_volume_reset = volume; }

  int volumeEvery (void) const
    { return m_volume_every; }
  void setVolumeEvery (int volume)
    { m_volume_every = limit (volume, 0, 1); }

  int initialVolume (void) const
    { return m_initial_volume; }
  void setInitialVolume (int volume)
    { m_initial_volume = limit (volume, m_volume_minimum, m_volume_maximum); }

  // Contrast settings

  int contrastMinimum (void) const
    { return m_contrast_minimum; }
  void setContrastMinimum (int contrast)
    { m_contrast_minimum = limit (contrast, -100, 100); }

  int contrastMaximum (void) const
    { return m_contrast_maximum; }
  void setContrastMaximum (int contrast)
    { m_contrast_maximum = limit (contrast, -100, 100); }

  void setContrastMinimumMaximum (int minimum, int maximum)
  {
    m_contrast_minimum = minimum;
    m_contrast_maximum = maximum;
    limit4 (m_contrast_minimum, m_contrast_maximum, -100, 100);
    setContrastDefault (limit (contrastDefault(), contrastMinimum(), contrastMaximum()));
  }

  int contrastMarks (void) const
    { return m_contrast_marks; }
  void setContrastMarks (int contrast)
    { m_contrast_marks = limit (contrast, 1, m_contrast_maximum - m_contrast_minimum); }

  int contrastStep (void) const
    { return m_contrast_step; }
  void setContrastStep (int contrast)
    { m_contrast_step = limit (contrast, 1, m_contrast_maximum - m_contrast_minimum); }

  bool contrastReset (void) const
    { return m_contrast_reset; }
  void setContrastReset (bool contrast)
    { m_contrast_reset = contrast; }

  int contrastEvery (void) const
    { return m_contrast_every; }
  void setContrastEvery (int contrast)
    { m_contrast_every = limit (contrast, 0, 1); }

  int initialContrast (void) const
    { return m_initial_contrast; }
  void setInitialContrast (int contrast)
    { m_initial_contrast = limit (contrast, m_contrast_minimum, m_contrast_maximum); }

  // Brightness settings

  int brightnessMinimum (void) const
    { return m_brightness_minimum; }
  void setBrightnessMinimum (int brightness)
    { m_brightness_minimum = limit (brightness, -100, 100); }

  int brightnessMaximum (void) const
    { return m_brightness_maximum; }
  void setBrightnessMaximum (int brightness)
    { m_brightness_maximum = limit (brightness, -100, 100); }

  void setBrightnessMinimumMaximum (int minimum, int maximum)
  {
    m_brightness_minimum = minimum;
    m_brightness_maximum = maximum;
    limit4 (m_brightness_minimum, m_brightness_maximum, -100, 100);
    setBrightnessDefault (limit (brightnessDefault(), brightnessMinimum(), brightnessMaximum()));
  }

  int brightnessMarks (void) const
    { return m_brightness_marks; }
  void setBrightnessMarks (int brightness)
    { m_brightness_marks = limit (brightness, 1, m_brightness_maximum - m_brightness_minimum); }

  int brightnessStep (void) const
    { return m_brightness_step; }
  void setBrightnessStep (int brightness)
    { m_brightness_step = limit (brightness, 1, m_brightness_maximum - m_brightness_minimum); }

  bool brightnessReset (void) const
    { return m_brightness_reset; }
  void setBrightnessReset (bool brightness)
    { m_brightness_reset = brightness; }

  int brightnessEvery (void) const
    { return m_brightness_every; }
  void setBrightnessEvery (int brightness)
    { m_brightness_every = limit (brightness, 0, 1); }

  int initialBrightness (void) const
    { return m_initial_brightness; }
  void setInitialBrightness (int brightness)
    { m_initial_brightness = limit (brightness, m_brightness_minimum, m_brightness_maximum); }

  // Hue settings

  int hueMinimum (void) const
    { return m_hue_minimum; }
  void setHueMinimum (int hue)
    { m_hue_minimum = limit (hue, -100, 100); }

  int hueMaximum (void) const
    { return m_hue_maximum; }
  void setHueMaximum (int hue)
    { m_hue_maximum = limit (hue, -100, 100); }

  void setHueMinimumMaximum (int minimum, int maximum)
  {
    m_hue_minimum = minimum;
    m_hue_maximum = maximum;
    limit4 (m_hue_minimum, m_hue_maximum, -100, 100);
    setHueDefault (limit (hueDefault(), hueMinimum(), hueMaximum()));
  }

  int hueMarks (void) const
    { return m_hue_marks; }
  void setHueMarks (int hue)
    { m_hue_marks = limit (hue, 1, m_hue_maximum - m_hue_minimum); }

  int hueStep (void) const
    { return m_hue_step; }
  void setHueStep (int hue)
    { m_hue_step = limit (hue, 1, m_hue_maximum - m_hue_minimum); }

  bool hueReset (void) const
    { return m_hue_reset; }
  void setHueReset (bool hue)
    { m_hue_reset = hue; }

  int hueEvery (void) const
    { return m_hue_every; }
  void setHueEvery (int hue)
    { m_hue_every = limit (hue, 0, 1); }

  int initialHue (void) const
    { return m_initial_hue; }
  void setInitialHue (int hue)
    { m_initial_hue = limit (hue, m_hue_minimum, m_hue_maximum); }

  // Saturation settings

  int saturationMinimum (void) const
    { return m_saturation_minimum; }
  void setSaturationMinimum (int saturation)
    { m_saturation_minimum = limit (saturation, -100, 100); }

  int saturationMaximum (void) const
    { return m_saturation_maximum; }
  void setSaturationMaximum (int saturation)
    { m_saturation_maximum = limit (saturation, -100, 100); }

  void setSaturationMinimumMaximum (int minimum, int maximum)
  {
    m_saturation_minimum = minimum;
    m_saturation_maximum = maximum;
    limit4 (m_saturation_minimum, m_saturation_maximum, -100, 100);
    setSaturationDefault (limit (saturationDefault(), saturationMinimum(), saturationMaximum()));
  }

  int saturationMarks (void) const
    { return m_saturation_marks; }
  void setSaturationMarks (int saturation)
    { m_saturation_marks = limit (saturation, 1, m_saturation_maximum - m_saturation_minimum); }

  int saturationStep (void) const
    { return m_saturation_step; }
  void setSaturationStep (int saturation)
    { m_saturation_step = limit (saturation, 1, m_saturation_maximum - m_saturation_minimum); }

  bool saturationReset (void) const
    { return m_saturation_reset; }
  void setSaturationReset (bool saturation)
    { m_saturation_reset = saturation; }

  int saturationEvery (void) const
    { return m_saturation_every; }
  void setSaturationEvery (int saturation)
    { m_saturation_every = limit (saturation, 0, 1); }

  int initialSaturation (void) const
    { return m_initial_saturation; }
  void setInitialSaturation (int saturation)
    { m_initial_saturation = limit (saturation, m_saturation_minimum, m_saturation_maximum); }

  // Progress and seeking settings

  int progressNormalSeek (void) const
    { return m_progress_normal_seek; }
  void setProgressNormalSeek (int seek)
    { m_progress_normal_seek = limit (seek, 1,
        m_progress_fast_seek_units == m_progress_normal_seek_units ? m_progress_fast_seek :
        m_progress_normal_seek_units ? INT_MAX : 100); }

  int progressNormalSeekUnits (void) const
    { return m_progress_normal_seek_units; }
  void setProgressNormalSeekUnits (int units)
    { m_progress_normal_seek_units = limit (units, 0, 1); }

  int progressFastSeek (void) const
    { return m_progress_fast_seek; }
  void setProgressFastSeek (int seek)
    { m_progress_fast_seek = limit (seek,
        m_progress_fast_seek_units == m_progress_normal_seek_units ? m_progress_normal_seek : 1,
        m_progress_fast_seek_units ? INT_MAX : 100); }

  int progressFastSeekUnits (void) const
    { return m_progress_fast_seek_units; }
  void setProgressFastSeekUnits (int units)
    { m_progress_fast_seek_units = limit (units, 0, 1); }

  int progressMarks (void) const
    { return m_progress_marks; }
  void setProgressMarks (int marks)
    { m_progress_marks = limit (marks, 1, 100); }

  // Slider settings

  int preferredSliderLength (void) const
    { return m_preferred_slider_length; }
  void setPreferredSliderLength (int length)
    { m_preferred_slider_length = limit (length, 50, QApplication::desktop() -> width()); }

  int minimumSliderLength (void) const
    { return m_minimum_slider_length; }
  void setMinimumSliderLength (int length)
    { m_minimum_slider_length = limit (length, 50, QApplication::desktop() -> width()); }

  // Subtitle settings

  KURL subtitleUrl (void) const
    { return ! subtitleUrlOverride() && m_properties ? m_properties -> subtitleUrl() : subtitleUrlDefault(); }
  void setSubtitlePath (const QString& path)
  {
    setSubtitleUrlDefault (KURL::fromPathOrURL (path));
    setSubtitleUrlOverride (true);
  }
  void setSubtitleUrl (const KURL&);

  KURL subtitleUrlDefault (void) const
    { return m_subtitle_url_default; }
  void setSubtitleUrlDefault (const KURL& url)
    { m_subtitle_url_default = url; }

  bool subtitleUrlOverride (void) const
    { return m_subtitle_url_override; }
  void setSubtitleUrlOverride (bool override)
    { m_subtitle_url_override = override; }

  bool subtitleAutoload (void) const
  {
    return m_properties && m_properties -> subtitleAutoloadOption() >= 0 ?
      m_properties -> subtitleAutoloadOption() == 0 : subtitleAutoloadDefault();
  }

  bool subtitleAutoloadDefault (void) const
    { return m_subtitle_autoload_default; }
  void setSubtitleAutoloadDefault (bool autoload)
    { m_subtitle_autoload_default = autoload; }

  bool subtitleVisibility (void) const;
  void setSubtitleVisibility (bool);

  bool subtitleVisibilityDefault (void) const
    { return m_subtitle_visibility_default; }
  void setSubtitleVisibilityDefault (bool visibility)
    { m_subtitle_visibility_default = visibility; }

  bool subtitleVisibilityOverride (void) const
    { return m_subtitle_visibility_override; }
  void setSubtitleVisibilityOverride (bool override)
    { m_subtitle_visibility_override = override; }

  int subtitlePosition (void) const;
  void setSubtitlePosition (int);

  int subtitlePositionDefault (void) const
    { return m_subtitle_position_default; }
  void setSubtitlePositionDefault (int position)
    { m_subtitle_position_default = limit (position, 0, 100); }

  bool subtitlePositionOverride (void) const
    { return m_subtitle_position_override; }
  void setSubtitlePositionOverride (bool override)
    { m_subtitle_position_override = override; }

  int subtitlePositionStep (void) const
    { return m_subtitle_position_step; }
  void setSubtitlePositionStep (int step)
    { m_subtitle_position_step = limit (step, 1); }

  float subtitleDelay (void) const;
  void setSubtitleDelay (float);

  float subtitleDelayDefault (void) const
    { return m_subtitle_delay_default; }
  void setSubtitleDelayDefault (float delay)
    { m_subtitle_delay_default = fabs (delay) < 0.0001 ? 0 : delay; }

  bool subtitleDelayOverride (void) const
    { return m_subtitle_delay_override; }
  void setSubtitleDelayOverride (bool override)
    { m_subtitle_delay_override = override; }

  float subtitleDelayStep (void) const
    { return m_subtitle_delay_step; }
  void setSubtitleDelayStep (float step)
    { m_subtitle_delay_step = flimit (step, 0.01); }

  bool autoloadAqtSubtitles (void) const
    { return m_autoload_aqt_subtitles; }
  void setAutoloadAqtSubtitles (bool autoload)
    { m_autoload_aqt_subtitles = autoload; }

  bool autoloadJssSubtitles (void) const
    { return m_autoload_jss_subtitles; }
  void setAutoloadJssSubtitles (bool autoload)
    { m_autoload_jss_subtitles = autoload; }

  bool autoloadRtSubtitles (void) const
    { return m_autoload_rt_subtitles; }
  void setAutoloadRtSubtitles (bool autoload)
    { m_autoload_rt_subtitles = autoload; }

  bool autoloadSmiSubtitles (void) const
    { return m_autoload_smi_subtitles; }
  void setAutoloadSmiSubtitles (bool autoload)
    { m_autoload_smi_subtitles = autoload; }

  bool autoloadSrtSubtitles (void) const
    { return m_autoload_srt_subtitles; }
  void setAutoloadSrtSubtitles (bool autoload)
    { m_autoload_srt_subtitles = autoload; }

  bool autoloadSsaSubtitles (void) const
    { return m_autoload_ssa_subtitles; }
  void setAutoloadSsaSubtitles (bool autoload)
    { m_autoload_ssa_subtitles = autoload; }

  bool autoloadSubSubtitles (void) const
    { return m_autoload_sub_subtitles; }
  void setAutoloadSubSubtitles (bool autoload)
    { m_autoload_sub_subtitles = autoload; }

  bool autoloadTxtSubtitles (void) const
    { return m_autoload_txt_subtitles; }
  void setAutoloadTxtSubtitles (bool autoload)
    { m_autoload_txt_subtitles = autoload; }

  bool autoloadUtfSubtitles (void) const
    { return m_autoload_utf_subtitles; }
  void setAutoloadUtfSubtitles (bool autoload)
    { m_autoload_utf_subtitles = autoload; }

  bool autoloadOtherSubtitles (void) const
    { return m_autoload_other_subtitles; }
  void setAutoloadOtherSubtitles (bool autoload)
    { m_autoload_other_subtitles = autoload; }

  QString autoloadExtensionList (void) const
    { return m_autoload_extension_list; }
  void setAutoloadExtensionList (QString autoload)
    { m_autoload_extension_list = autoload; }

  // Message settings

  bool showMessagesOnError (void) const
    { return m_show_messages_on_error; }
  void setShowMessagesOnError (bool show)
    { m_show_messages_on_error = show; }

  bool showMessagesBeforePlaying (void) const
    { return m_show_messages_before_playing; }
  void setShowMessagesBeforePlaying (bool show)
    { m_show_messages_before_playing = show; }

  bool showMessagesAfterPlaying (void) const
    { return m_show_messages_after_playing; }
  void setShowMessagesAfterPlaying (bool show)
    { m_show_messages_after_playing = show; }

  bool clearMessagesBeforePlaying (void) const
    { return m_clear_messages_before_playing; }
  void setClearMessagesBeforePlaying (bool clear)
    { m_clear_messages_before_playing = clear; }

  // Audio settings

  float audioDelayStep (void) const
    { return m_audio_delay_step; }
  void setAudioDelayStep (float step)
    { m_audio_delay_step = flimit (step, 0.01); }

  QString audioDriverString (void) const;
  QString audioCodecString (void) const;

  QString audioDriver (void) const
  {
    return m_properties && ! m_properties -> audioDriverOption().isNull() ?
      m_properties -> audioDriverOption() : audioDriverDefault();
  }

  QString audioDriverDefault (void) const
    { return m_audio_driver_default; }
  void setAudioDriverDefault (QString driver)
    { m_audio_driver_default = driver; }

  bool audioDriverFallback (void) const
  {
    return m_properties && m_properties -> audioDriverFallbackOption() >= 0 ?
      m_properties -> audioDriverFallbackOption() == 0 : audioDriverFallbackDefault();
  }

  bool audioDriverFallbackDefault (void) const
    { return m_audio_driver_fallback_default; }
  void setAudioDriverFallbackDefault (bool fallback)
    { m_audio_driver_fallback_default = fallback; }

  QString audioDevice (void) const
  {
    return m_properties && m_properties -> audioDeviceOption() >= 0 ?
      m_properties -> audioDeviceValue() : audioDeviceDefault();
  }

  QString audioDeviceDefault (void) const
    { return m_audio_device_default; }
  void setAudioDeviceDefault (QString device)
    { m_audio_device_default = device; }

  QString audioCodec (void) const
  {
    return m_properties && ! m_properties -> audioCodecOption().isNull() ?
      m_properties -> audioCodecOption() : audioCodecDefault();
  }

  QString audioCodecDefault (void) const
    { return m_audio_codec_default; }
  void setAudioCodecDefault (QString codec)
    { m_audio_codec_default = codec; }

  bool audioCodecFallback (void) const
  {
    return m_properties && m_properties -> audioCodecFallbackOption() >= 0 ?
      m_properties -> audioCodecFallbackOption() == 0 : audioCodecFallbackDefault();
  }

  bool audioCodecFallbackDefault (void) const
    { return m_audio_codec_fallback_default; }
  void setAudioCodecFallbackDefault (bool fallback)
    { m_audio_codec_fallback_default = fallback; }

  // Video settings

  QString videoDriverString (void) const;
  QString videoCodecString (void) const;

  QString videoDriver (void) const
  {
    return m_properties && ! m_properties -> videoDriverOption().isNull() ?
      m_properties -> videoDriverOption() : videoDriverDefault();
  }

  QString videoDriverDefault (void) const
    { return m_video_driver_default; }
  void setVideoDriverDefault (QString driver)
    { m_video_driver_default = driver; }

  bool videoDriverFallback (void) const
  {
    return m_properties && m_properties -> videoDriverFallbackOption() >= 0 ?
      m_properties -> videoDriverFallbackOption() == 0 : videoDriverFallbackDefault();
  }

  bool videoDriverFallbackDefault (void) const
    { return m_video_driver_fallback_default; }
  void setVideoDriverFallbackDefault (bool fallback)
    { m_video_driver_fallback_default = fallback; }

  QString videoDevice (void) const
  {
    return m_properties && m_properties -> videoDeviceOption() >= 0 ?
      m_properties -> videoDeviceValue() : videoDeviceDefault();
  }

  QString videoDeviceDefault (void) const
    { return m_video_device_default; }
  void setVideoDeviceDefault (QString device)
    { m_video_device_default = device; }

  QString videoCodec (void) const
  {
    return m_properties && ! m_properties -> videoCodecOption().isNull() ?
      m_properties -> videoCodecOption() : videoCodecDefault();
  }

  QString videoCodecDefault (void) const
    { return m_video_codec_default; }
  void setVideoCodecDefault (QString codec)
    { m_video_codec_default = codec; }

  bool videoCodecFallback (void) const
  {
    return m_properties && m_properties -> videoCodecFallbackOption() >= 0 ?
      m_properties -> videoCodecFallbackOption() == 0 : videoCodecFallbackDefault();
  }

  bool videoCodecFallbackDefault (void) const
    { return m_video_codec_fallback_default; }
  void setVideoCodecFallbackDefault (bool fallback)
    { m_video_codec_fallback_default = fallback; }

  int videoScaler (void) const
  {
    return m_properties && m_properties -> videoScalerOption() >= 0 ?
      m_properties -> videoScalerOption() : videoScalerDefault();
  }

  int videoScalerDefault (void) const
    { return m_video_scaler_default; }
  void setVideoScalerDefault (int scaler)
    { m_video_scaler_default = limit (scaler, 0, 10); }

  bool videoDoubleBuffering (void) const
  {
    return m_properties && m_properties -> videoDoubleBufferingOption() >= 0 ?
      m_properties -> videoDoubleBufferingOption() == 0 : videoDoubleBufferingDefault();
  }

  bool videoDoubleBufferingDefault (void) const
    { return m_video_double_buffering_default; }
  void setVideoDoubleBufferingDefault (bool double_buffering)
    { m_video_double_buffering_default = double_buffering; }

  bool videoDirectRendering (void) const
  {
    return m_properties && m_properties -> videoDirectRenderingOption() >= 0 ?
      m_properties -> videoDirectRenderingOption() == 0 : videoDirectRenderingDefault();
  }

  bool videoDirectRenderingDefault (void) const
    { return m_video_direct_rendering_default; }
  void setVideoDirectRenderingDefault (bool direct_rendering)
    { m_video_direct_rendering_default = direct_rendering; }

  // Advanced settings

  QString executablePath (void) const
  {
    return m_properties && m_properties -> executablePathOption() >= 0 ?
      m_properties -> executablePathValue() : executablePathDefault();
  }

  QString executablePathDefault (void) const
    { return m_executable_path_default.isEmpty() ? "mplayer" : m_executable_path_default; }
  void setExecutablePathDefault (QString path)
    { m_executable_path_default = path; }

  QString commandLine (void) const
  {
    return m_properties && m_properties -> commandLineOption() == 0 ?
      m_properties -> commandLineValue() : m_properties && m_properties -> commandLineOption() == 1 ?
      commandLineDefault() + " " + m_properties -> commandLineValue() : commandLineDefault();
  }

  QString commandLineDefault (void) const
    { return m_command_line_default; }
  void setCommandLineDefault (QString commandline)
    { m_command_line_default = commandline; }

  int frameDrop (void) const;
  void setFrameDrop (int);

  int frameDropDefault (void) const
    { return m_frame_drop_default; }
  void setFrameDropDefault (int frame_drop)
    { m_frame_drop_default = limit (frame_drop, 0, 2); }

  bool frameDropOverride (void) const
    { return m_frame_drop_override; }
  void setFrameDropOverride (bool override)
    { m_frame_drop_override = override; }

  int cache (void) const
  {
    return m_properties && m_properties -> cacheOption() >= 0 ?
      m_properties -> cacheOption() : cacheDefault();
  }

  int cacheDefault (void) const
    { return m_cache_default; }
  void setCacheDefault (int use)
    { m_cache_default = limit (use, 0, 2); }

  int cacheSize (void) const
  {
    return m_properties && m_properties -> cacheOption() >= 0 ?
      m_properties -> cacheSizeValue() : cacheSizeDefault();
  }

  int cacheSizeDefault (void) const
    { return m_cache_size_default; }
  void setCacheSizeDefault (int size)
  {
    if ( size > 0 )
      m_cache_size_default = limit (size, 4);
    else
      setCacheDefault (1);
  }

  int buildNewIndex (void) const
  {
    return m_properties && m_properties -> buildNewIndexOption() >= 0 ?
      m_properties -> buildNewIndexOption() : buildNewIndexDefault();
  }

  int buildNewIndexDefault (void) const
    { return m_build_new_index_default; }
  void setBuildNewIndexDefault (int build)
    { m_build_new_index_default = limit (build, 0, 2); }

  int osdLevel (void) const
  {
    return m_properties && m_properties -> osdLevelOption() >= 0 ?
      m_properties -> osdLevelOption() : osdLevelDefault();
  }

  int osdLevelDefault (void) const
    { return m_osd_level_default; }
  void setOsdLevelDefault (int level)
    { m_osd_level_default = limit (level, 0, 3); }

  bool useTemporaryFile (void) const
  {
    return m_properties && m_properties -> useTemporaryFileOption() >= 0 ?
      m_properties -> useTemporaryFileOption() == 0 : useTemporaryFileDefault();
  }

  bool useTemporaryFileDefault (void) const
    { return m_use_temporary_file_default; }
  void setUseTemporaryFileDefault (bool use)
    { m_use_temporary_file_default = use; }

  bool useKioslaveForHttp (void) const
    { return m_use_kioslave_for_http; }
  void setUseKioslaveForHttp (bool use)
    { m_use_kioslave_for_http = use; }

  bool useKioslaveForFtp (void) const
    { return m_use_kioslave_for_ftp; }
  void setUseKioslaveForFtp (bool use)
    { m_use_kioslave_for_ftp = use; }

  bool useKioslaveForSmb (void) const
    { return m_use_kioslave_for_smb; }
  void setUseKioslaveForSmb (bool use)
    { m_use_kioslave_for_smb = use; }

  bool useKioslave (void) const;

protected:
  KPlayerProperties* m_properties;

  QSize m_display_size;
  QSize m_aspect;
  QSize m_original_aspect;
  bool m_display_size_override;
  bool m_aspect_override;
  bool m_full_screen_default;
  bool m_full_screen_override;
  bool m_last_full_screen;
  bool m_maximized_default;
  bool m_maximized_override;
  int m_volume_default;
  bool m_volume_override;
  bool m_mute;
  float m_audio_delay_default;
  bool m_audio_delay_override;
  int m_contrast_default;
  bool m_contrast_override;
  int m_brightness_default;
  bool m_brightness_override;
  int m_hue_default;
  bool m_hue_override;
  int m_saturation_default;
  bool m_saturation_override;
  bool m_maintain_aspect_default;
  bool m_maintain_aspect_override;
  bool m_resize_automatically;
  int m_minimum_initial_width;
  int m_recent_file_list_size;
  bool m_show_file_path;
  bool m_show_open_dialog;
  bool m_disable_screen_saver;
  bool m_start_playing_immediately;
  bool m_start_playing_only_if_idle;
  bool m_allow_duplicate_entries;
  int m_playlist_size_limit;
  int m_cache_size_limit;
  bool m_remember_with_shift;
  bool m_remember_size;
  bool m_remember_aspect;
  bool m_remember_full_screen;
  bool m_remember_maximized;
  bool m_remember_maintain_aspect;
  bool m_remember_volume;
  bool m_remember_audio_delay;
  bool m_remember_frame_drop;
  bool m_remember_contrast;
  bool m_remember_brightness;
  bool m_remember_hue;
  bool m_remember_saturation;
  bool m_remember_subtitle_url;
  bool m_remember_subtitle_visibility;
  bool m_remember_subtitle_position;
  bool m_remember_subtitle_delay;
  int m_preferred_slider_length;
  int m_minimum_slider_length;
  int m_volume_minimum;
  int m_volume_maximum;
  int m_volume_marks;
  int m_volume_step;
  bool m_volume_reset;
  int m_volume_every;
  int m_initial_volume;
  int m_contrast_minimum;
  int m_contrast_maximum;
  int m_contrast_marks;
  int m_contrast_step;
  bool m_contrast_reset;
  int m_contrast_every;
  int m_initial_contrast;
  int m_brightness_minimum;
  int m_brightness_maximum;
  int m_brightness_marks;
  int m_brightness_step;
  bool m_brightness_reset;
  int m_brightness_every;
  int m_initial_brightness;
  int m_hue_minimum;
  int m_hue_maximum;
  int m_hue_marks;
  int m_hue_step;
  bool m_hue_reset;
  int m_hue_every;
  int m_initial_hue;
  int m_saturation_minimum;
  int m_saturation_maximum;
  int m_saturation_marks;
  int m_saturation_step;
  bool m_saturation_reset;
  int m_saturation_every;
  int m_initial_saturation;
  int m_progress_normal_seek;
  int m_progress_normal_seek_units;
  int m_progress_fast_seek;
  int m_progress_fast_seek_units;
  int m_progress_marks;
  KURL m_subtitle_url_default;
  bool m_subtitle_url_override;
  bool m_subtitle_autoload_default;
  bool m_subtitle_autoload_override;
  bool m_subtitle_visibility_default;
  bool m_subtitle_visibility_override;
  int m_subtitle_position_default;
  bool m_subtitle_position_override;
  int m_subtitle_position_step;
  float m_subtitle_delay_default;
  bool m_subtitle_delay_override;
  float m_subtitle_delay_step;
  bool m_autoload_aqt_subtitles;
  bool m_autoload_jss_subtitles;
  bool m_autoload_rt_subtitles;
  bool m_autoload_smi_subtitles;
  bool m_autoload_srt_subtitles;
  bool m_autoload_ssa_subtitles;
  bool m_autoload_sub_subtitles;
  bool m_autoload_txt_subtitles;
  bool m_autoload_utf_subtitles;
  bool m_autoload_other_subtitles;
  QString m_autoload_extension_list;
  bool m_show_messages_on_error;
  bool m_show_messages_before_playing;
  bool m_show_messages_after_playing;
  bool m_clear_messages_before_playing;
  float m_audio_delay_step;
  QString m_audio_driver_default;
  bool m_audio_driver_fallback_default;
  QString m_audio_device_default;
  QString m_audio_codec_default;
  bool m_audio_codec_fallback_default;
  QString m_video_driver_default;
  bool m_video_driver_fallback_default;
  QString m_video_device_default;
  QString m_video_codec_default;
  bool m_video_codec_fallback_default;
  int m_video_scaler_default;
  bool m_video_double_buffering_default;
  bool m_video_direct_rendering_default;
  QString m_executable_path_default;
  QString m_command_line_default;
  int m_frame_drop_default;
  int m_frame_drop_override;
  int m_cache_default;
  int m_cache_size_default;
  int m_build_new_index_default;
  int m_osd_level_default;
  bool m_use_temporary_file_default;
  bool m_use_kioslave_for_http;
  bool m_use_kioslave_for_ftp;
  bool m_use_kioslave_for_smb;
  bool m_shift;

  /** Emits the refresh signal.
   */
  void emitRefresh (void)
    { emit refresh(); }

signals:
  /** Emitted when the properties have been saved.
   */
  void refresh (void);
};

#ifdef NDEBUG
#define kdDebugTime kndDebug
#else
kdbgstream kdDebugTime (void);
#endif

#endif
