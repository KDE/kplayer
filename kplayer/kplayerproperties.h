/***************************************************************************
                          kplayerproperties.h
                          -------------------
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

#ifndef KPLAYERPROPERTIES_H
#define KPLAYERPROPERTIES_H

#include <kurl.h>
#include <limits.h>
#include <math.h>
#include <qobject.h>

/** Returns the given time length as string.
  */
QString timeString (float length, bool zero_ok = false);

inline float flimit (float value, float minValue)
{
  return value < minValue ? minValue : value;
}

inline int limit (int value, int minValue, int maxValue = INT_MAX)
{
  return value < minValue ? minValue : value > maxValue ? maxValue : value;
}

inline void limit4 (int& lowerValue, int& higherValue, int minValue, int maxValue)
{
  if ( lowerValue > higherValue )
  {
    int i = lowerValue;
    lowerValue = higherValue;
    higherValue = i;
  }
  lowerValue = limit (lowerValue, minValue, maxValue);
  higherValue = limit (higherValue, minValue, maxValue);
}

/** The KPlayer file properties.
  *@author kiriuja
  */
class KPlayerProperties : public QObject
{
  Q_OBJECT

public:
  /** Default constructor, to be used by KPlayerSettings. Initializes properties to the defaults.
   */
  KPlayerProperties (void);
  /** Standalone constructor. Initializes properties to the defaults.
   */
  KPlayerProperties (const KURL&);
  /** The destructor. Does nothing but make noise.
   */
  virtual ~KPlayerProperties();

  void reference (void)
    { m_references ++; }
  bool dereference (void)
    { return -- m_references == 0; }

  void load (void);
  void save (void) const;
  void reset (void);
  void defaults (void);

  // Basic properties

  KURL url (void) const
    { return m_url; }
  void setUrl (const KURL& url)
    { m_url = url; }

  QString defaultName (void) const;

  QString name (void) const
    { return m_name.isEmpty() ? defaultName() : m_name; }
  void setName (QString name)
    { m_name = name; }

  QString type (void) const;

  float length (void) const
    { return m_length; }
  void setLength (float length)
    { m_length = length; }

  /** Time length as string.
    */
  QString lengthString (void) const
    { return timeString (length()); }

  QSize originalSize (void) const
    { return m_original_size; }
  void setOriginalSize (QSize size)
    { m_original_size = size; }

  QString audioCodecValue (void) const
    { return m_audio_codec; }
  void setAudioCodecValue (QString codec)
    { m_audio_codec = codec; }

  int audioBitrate (void) const
    { return m_audio_bitrate; }
  void setAudioBitrate (int bitrate)
    { m_audio_bitrate = limit (bitrate, 0); }

  QString videoCodecValue (void) const
    { return m_video_codec; }
  void setVideoCodecValue (QString codec)
    { m_video_codec = codec; }

  int videoBitrate (void) const
    { return m_video_bitrate; }
  void setVideoBitrate (int bitrate)
    { m_video_bitrate = limit (bitrate, 0); }

  float framerate (void) const
    { return m_framerate; }
  void setFramerate (float rate)
    { m_framerate = flimit (rate, 0); }

  // General properties

  int playlistOption (void) const
    { return m_playlist; }
  void setPlaylistOption (int playlist)
    { m_playlist = limit (playlist, 0, 2); }

  int displaySizeOption (void) const
    { return m_display_size_option; }
  void setDisplaySizeOption (int option)
    { m_display_size_option = limit (option, 0, 2); }

  QSize displaySizeValue (void) const
    { return m_display_size; }
  void setDisplaySizeValue (QSize size)
    { m_display_size = size; }

  int fullScreenOption (void) const
    { return m_full_screen; }
  void setFullScreenOption (int full_screen)
    { m_full_screen = limit (full_screen, -1, 2); }

  int maintainAspectOption (void) const
    { return m_maintain_aspect; }
  void setMaintainAspectOption (int maintain)
    { m_maintain_aspect = limit (maintain, -1, 1); }

  // Subtitle properties

  int subtitleAutoloadOption (void) const
    { return m_subtitle_autoload; }
  void setSubtitleAutoloadOption (int option)
    { m_subtitle_autoload = limit (option, -1, 1); }

  KURL subtitleUrl (void) const
    { return m_subtitle_url; }
  void setSubtitleUrl (const KURL& url)
    { m_subtitle_url = url; }
  void setSubtitleUrl (const QString& url)
    { m_subtitle_url = KURL::fromPathOrURL (url); }

  int subtitleVisibilityOption (void) const
    { return m_subtitle_visibility; }
  void setSubtitleVisibilityOption (int option)
    { m_subtitle_visibility = limit (option, -1, 1); }

  int subtitlePositionOption (void) const
    { return m_subtitle_position_option; }
  void setSubtitlePositionOption (int option)
    { m_subtitle_position_option = limit (option, -1, 0); }

  int subtitlePositionValue (void) const
    { return m_subtitle_position; }
  void setSubtitlePositionValue (int position)
    { m_subtitle_position = limit (position, 0, 100); }

  int subtitleDelayOption (void) const
    { return m_subtitle_delay_option; }
  void setSubtitleDelayOption (int option)
    { m_subtitle_delay_option = limit (option, -1, 0); }

  float subtitleDelayValue (void) const
    { return m_subtitle_delay; }
  void setSubtitleDelayValue (float delay)
    { m_subtitle_delay = fabs (delay) < 0.0001 ? 0 : delay; }

  // Audio properties

  int volumeOption (void) const
    { return m_volume_option; }
  void setVolumeOption (int option)
    { m_volume_option = limit (option, -1, 2); }

  int volumeValue (void) const
    { return m_volume; }
  void setVolumeValue (int volume)
    { m_volume = limit (volume, volumeOption() > 0 ? -100 : 0, 100); }

  int audioDelayOption (void) const
    { return m_audio_delay_option; }
  void setAudioDelayOption (int option)
    { m_audio_delay_option = limit (option, -1, 0); }

  float audioDelayValue (void) const
    { return m_audio_delay; }
  void setAudioDelayValue (float delay)
    { m_audio_delay = fabs (delay) < 0.0001 ? 0 : delay; }

  QString audioDriverValue (void) const
    { return m_audio_driver; }
  void setAudioDriverValue (QString driver)
    { m_audio_driver = driver; }

  QString audioDriverOption (void) const
    { return m_audio_driver_option; }
  void setAudioDriverOption (QString driver)
    { m_audio_driver_option = driver; }

  int audioDriverFallbackOption (void) const
    { return m_audio_driver_fallback; }
  void setAudioDriverFallbackOption (int fallback)
    { m_audio_driver_fallback = limit (fallback, -1, 1); }

  QString audioDeviceValue (void) const
    { return m_audio_device; }
  void setAudioDeviceValue (QString device)
    { m_audio_device = device; }

  int audioDeviceOption (void) const
    { return m_audio_device_option; }
  void setAudioDeviceOption (int option)
    { m_audio_device_option = limit (option, -1, 0); }

  QString audioCodecOption (void) const
    { return m_audio_codec_option; }
  void setAudioCodecOption (QString codec)
    { m_audio_codec_option = codec; }

  int audioCodecFallbackOption (void) const
    { return m_audio_codec_fallback; }
  void setAudioCodecFallbackOption (int fallback)
    { m_audio_codec_fallback = limit (fallback, -1, 1); }

  // Video properties

  int contrastOption (void) const
    { return m_contrast_option; }
  void setContrastOption (int option)
    { m_contrast_option = limit (option, -1, 2); }

  int contrastValue (void) const
    { return m_contrast; }
  void setContrastValue (int contrast)
    { m_contrast = limit (contrast, contrastOption() > 0 ? -200 : -100, contrastOption() > 0 ? 200 : 100); }

  int brightnessOption (void) const
    { return m_brightness_option; }
  void setBrightnessOption (int option)
    { m_brightness_option = limit (option, -1, 2); }

  int brightnessValue (void) const
    { return m_brightness; }
  void setBrightnessValue (int brightness)
    { m_brightness = limit (brightness, brightnessOption() > 0 ? -200 : -100, brightnessOption() > 0 ? 200 : 100); }

  int hueOption (void) const
    { return m_hue_option; }
  void setHueOption (int option)
    { m_hue_option = limit (option, -1, 2); }

  int hueValue (void) const
    { return m_hue; }
  void setHueValue (int hue)
    { m_hue = limit (hue, hueOption() > 0 ? -200 : -100, hueOption() > 0 ? 200 : 100); }

  int saturationOption (void) const
    { return m_saturation_option; }
  void setSaturationOption (int option)
    { m_saturation_option = limit (option, -1, 2); }

  int saturationValue (void) const
    { return m_saturation; }
  void setSaturationValue (int saturation)
    { m_saturation = limit (saturation, saturationOption() > 0 ? -200 : -100, saturationOption() > 0 ? 200 : 100); }

  QString videoDriverValue (void) const
    { return m_video_driver; }
  void setVideoDriverValue (QString driver)
    { m_video_driver = driver; }

  QString videoDriverOption (void) const
    { return m_video_driver_option; }
  void setVideoDriverOption (QString driver)
    { m_video_driver_option = driver; }

  int videoDriverFallbackOption (void) const
    { return m_video_driver_fallback; }
  void setVideoDriverFallbackOption (int fallback)
    { m_video_driver_fallback = limit (fallback, -1, 1); }

  QString videoDeviceValue (void) const
    { return m_video_device; }
  void setVideoDeviceValue (QString device)
    { m_video_device = device; }

  int videoDeviceOption (void) const
    { return m_video_device_option; }
  void setVideoDeviceOption (int option)
    { m_video_device_option = limit (option, -1, 0); }

  QString videoCodecOption (void) const
    { return m_video_codec_option; }
  void setVideoCodecOption (QString codec)
    { m_video_codec_option = codec; }

  int videoCodecFallbackOption (void) const
    { return m_video_codec_fallback; }
  void setVideoCodecFallbackOption (int fallback)
    { m_video_codec_fallback = limit (fallback, -1, 1); }

  int videoScalerOption (void) const
    { return m_video_scaler; }
  void setVideoScalerOption (int scaler)
    { m_video_scaler = limit (scaler, -1, 10); }

  int videoDoubleBufferingOption (void) const
    { return m_video_double_buffering; }
  void setVideoDoubleBufferingOption (int double_buffering)
    { m_video_double_buffering = limit (double_buffering, -1, 1); }

  int videoDirectRenderingOption (void) const
    { return m_video_direct_rendering; }
  void setVideoDirectRenderingOption (int direct_rendering)
    { m_video_direct_rendering = limit (direct_rendering, -1, 1); }

  // Advanced properties

  QString executablePathValue (void) const
    { return m_executable_path.isEmpty() ? "mplayer" : m_executable_path; }
  void setExecutablePathValue (QString path)
    { m_executable_path = path; }

  int executablePathOption (void) const
    { return m_executable_path_option; }
  void setExecutablePathOption (int option)
    { m_executable_path_option = limit (option, -1, 0); }

  QString commandLineValue (void) const
    { return m_command_line; }
  void setCommandLineValue (QString command_line)
    { m_command_line = command_line; }

  int commandLineOption (void) const
    { return m_command_line_option; }
  void setCommandLineOption (int option)
    { m_command_line_option = limit (option, -1, 1); }

  int frameDropOption (void) const
    { return m_frame_drop; }
  void setFrameDropOption (int frame_drop)
    { m_frame_drop = limit (frame_drop, -1, 2); }

  int cacheOption (void) const
    { return m_cache_option; }
  void setCacheOption (int option)
    { m_cache_option = limit (option, -1, 2); }

  int cacheSizeValue (void) const
    { return m_cache_size; }
  void setCacheSizeValue (int size)
  {
    if ( size > 0 )
      m_cache_size = limit (size, 4);
    else
      setCacheOption (1);
  }

  int buildNewIndexOption (void) const
    { return m_build_new_index; }
  void setBuildNewIndexOption (int build)
    { m_build_new_index = limit (build, -1, 2); }

  int useKioslaveOption (void) const
    { return m_use_kioslave; }
  void setUseKioslaveOption (int option)
    { m_use_kioslave = limit (option, 0, 2); }

  int useTemporaryFileOption (void) const
    { return m_use_temporary; }
  void setUseTemporaryFileOption (int option)
    { m_use_temporary = limit (option, -1, 1); }

  int osdLevelOption (void) const
    { return m_osd_level; }
  void setOsdLevelOption (int level)
    { m_osd_level = limit (level, -1, 3); }

protected:
  KURL m_url;
  QString m_name;
  float m_length;
  QSize m_original_size;
  QString m_audio_codec;
  int m_audio_bitrate;
  QString m_video_codec;
  int m_video_bitrate;
  float m_framerate;
  int m_playlist;
  int m_display_size_option;
  QSize m_display_size;
  int m_full_screen;
  int m_maintain_aspect;
  int m_subtitle_autoload;
  KURL m_subtitle_url;
  int m_subtitle_visibility;
  int m_subtitle_position_option;
  int m_subtitle_position;
  int m_subtitle_delay_option;
  float m_subtitle_delay;
  int m_volume_option;
  int m_volume;
  int m_contrast_option;
  int m_contrast;
  int m_brightness_option;
  int m_brightness;
  int m_hue_option;
  int m_hue;
  int m_saturation_option;
  int m_saturation;
  int m_audio_delay_option;
  float m_audio_delay;
  QString m_audio_driver;
  QString m_audio_driver_option;
  int m_audio_driver_fallback;
  QString m_audio_device;
  int m_audio_device_option;
  QString m_audio_codec_option;
  int m_audio_codec_fallback;
  QString m_video_driver;
  QString m_video_driver_option;
  int m_video_driver_fallback;
  QString m_video_device;
  int m_video_device_option;
  QString m_video_codec_option;
  int m_video_codec_fallback;
  int m_video_scaler;
  int m_video_double_buffering;
  int m_video_direct_rendering;
  QString m_executable_path;
  int m_executable_path_option;
  QString m_command_line;
  int m_command_line_option;
  int m_frame_drop;
  int m_cache_option;
  int m_cache_size;
  int m_build_new_index;
  int m_use_kioslave;
  int m_use_temporary;
  int m_osd_level;

  int m_references;

  /** Emits the refresh signal.
   */
  void emitRefresh (void)
    { emit refresh(); }

signals:
  /** Emitted when the properties have been saved.
   */
  void refresh (void);
};

#endif
