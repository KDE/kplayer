/***************************************************************************
                          kplayersettings.h
                          -----------------
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

#ifndef KPLAYERSETTINGS_H
#define KPLAYERSETTINGS_H

#include "kplayerproperties.h"

/** The KPlayer settings.
  *@author kiriuja
  */
class KPlayerSettings : public QObject
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerSettings (void);
  /** Destructor. */
  virtual ~KPlayerSettings();

  /** Returns the current properties. */
  KPlayerTrackProperties* properties (void) const
    { return m_properties; }
  /** Returns the configuration. */
  KPlayerConfiguration* configuration (void) const
    { return KPlayerEngine::engine() -> configuration(); }
  /** Returns the properties for the given key. */
  KPlayerProperties* properties (const QString& key) const
    { return configuration() -> override (key) ? (KPlayerProperties*) configuration()
      : (KPlayerProperties*) properties(); }

  void setOverride (const QString& key, bool override)
    { configuration() -> setOverride (key, override); }

  /** Loads the given URL settings and makes the necessary adjustments. */
  void load (const KUrl&);

  // Basic settings

  int volume (void) const
    { return properties ("Volume") -> volume(); }
  void setVolume (int);

  int actualVolume (void) const
    { return configuration() -> mute() ? 0 : volume(); }

  float audioDelay (void) const
    { return properties ("Audio Delay") -> audioDelay(); }
  void setAudioDelay (float);

  int contrast (void) const
    { return properties ("Contrast") -> contrast(); }
  void setContrast (int);

  int brightness (void) const
    { return properties ("Brightness") -> brightness(); }
  void setBrightness (int);

  int hue (void) const
    { return properties ("Hue") -> hue(); }
  void setHue (int);

  int saturation (void) const
    { return properties ("Saturation") -> saturation(); }
  void setSaturation (int);

  QSize aspect (void) const
    { return m_aspect; }
  void setAspect (QSize aspect);

  bool aspectOverride (void) const
    { return configuration() -> override ("Aspect"); }
  void setAspectOverride (bool override)
    { configuration() -> setOverride ("Aspect", override); }

  QSize displaySize (void) const
    { return m_display_size; }
  void setDisplaySize (QSize size);

  bool displaySizeOverride (void) const
    { return configuration() -> override ("Display Size"); }
  void setDisplaySizeOverride (bool override)
    { configuration() -> setOverride ("Display Size", override); }

  QSize adjustDisplaySize (bool, bool);

  bool setInitialDisplaySize (void);

  bool isAspect (QSize);
  bool isZoomFactor (int, int = 1);

  bool fullScreen (void);
  void setFullScreen (bool);

  bool maximized (void) const
    { return properties ("Maximized") -> maximized(); }
  void setMaximized (bool);

  bool maintainAspect (void) const
    { return properties ("Maintain Aspect") -> maintainAspect(); }
  void setMaintainAspect (bool, QSize);

  bool constrainedSize (void)
    { return fullScreen() || maximized() || ! configuration() -> resizeAutomatically()
      || KPlayerEngine::engine() -> light(); }

  QSize constrainSize (QSize size) const;
  QSize adjustSize (QSize size, bool horizontally = false) const;

  bool control (void) const
    { return m_control; }
  void setControl (bool control)
    { resetControl(); m_control = control; }

  bool shift (void) const
    { return m_shift; }
  void setShift (bool shift)
    { resetShift(); m_shift = shift; }

  bool anyControl (void) const
    { return ! m_no_control && (m_control || m_fake_control); }
  void setNoControl (void)
    { m_no_control = true; m_fake_control = false; }
  void setFakeControl (void)
    { m_fake_control = true; }
  void resetControl (void)
    { m_no_control = m_fake_control = false; }

  bool anyShift (void) const
    { return ! m_no_shift && (m_shift || m_fake_shift); }
  void setNoShift (void)
    { m_no_shift = true; m_fake_shift = false; }
  void setFakeShift (void)
    { m_fake_shift = true; }
  void resetShift (void)
    { m_no_shift = m_fake_shift = false; }

  // Subtitle settings

  bool hasSubtitles (void) const
    { return properties() -> hasSubtitleIDs() || properties() -> hasVobsubIDs()
      || hasExternalSubtitles() || hasVobsubSubtitles(); }
  bool showSubtitles (void) const
    { return properties() -> showInternalSubtitles()
      || (hasExternalSubtitles() || hasVobsubSubtitles()) && properties() -> showSubtitles(); }

  QString currentSubtitles (void) const;
  const QStringList& subtitles (void) const
    { return m_subtitles; }
  bool hasExternalSubtitles (void) const
    { return ! m_subtitles.isEmpty(); }

  const QString& vobsubSubtitles (void) const
    { return m_vobsub; }
  bool hasVobsubSubtitles (void) const
    { return ! m_vobsub.isEmpty(); }
  bool showVobsubSubtitles (void) const
    { return showSubtitles() && hasVobsubSubtitles() && currentSubtitles() != properties() -> subtitleUrlString()
      && ! properties() -> hasSubtitleID(); }

  QString currentSubtitlePath (void) const;

  void addSubtitlePath (const QString& path);
  void clearSubtitles (void)
    { m_subtitles.clear(); m_vobsub = QString::null; }

  int subtitlePosition (void) const
    { return properties ("Subtitle Position") -> subtitlePosition(); }
  void setSubtitlePosition (int);

  float subtitleDelay (void) const
    { return properties ("Subtitle Delay") -> subtitleDelay(); }
  void setSubtitleDelay (float);

  // Advanced settings

  int frameDrop (void) const
    { return properties ("Frame Dropping") -> frameDrop(); }
  void setFrameDrop (int);

protected:
  KPlayerTrackProperties* m_properties;
  bool m_last_full_screen;
  QSize m_display_size;
  QSize m_aspect;
  bool m_control;
  bool m_shift;
  bool m_no_control;
  bool m_no_shift;
  bool m_fake_control;
  bool m_fake_shift;
  QStringList m_subtitles;
  QString m_vobsub;
};

inline KPlayerTrackProperties* KPlayerEngine::properties (void) const
{
  return settings() -> properties();
}

#endif
