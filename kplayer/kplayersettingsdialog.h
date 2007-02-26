/***************************************************************************
                          kplayersettingsdialog.h
                          -----------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERSETTINGSDIALOG_H
#define KPLAYERSETTINGSDIALOG_H

#include <kdialogbase.h>

#include "kplayersettingsadvanced.h"
#include "kplayersettingsaudio.h"
#include "kplayersettingscontrols.h"
#include "kplayersettingsgeneral.h"
#include "kplayersettingssliders.h"
#include "kplayersettingssubtitles.h"
#include "kplayersettingsvideo.h"
#include "kplayersettingsprogress.h"
#include "kplayersettingsvolume.h"
#include "kplayersettingscontrast.h"
#include "kplayersettingsbrightness.h"
#include "kplayersettingshue.h"
#include "kplayersettingssaturation.h"
#include "kplayerproperties.h"

QString listEntry (QComboBox* combo, bool hasDefault = false);

class KPlayerSettingsAdvanced : public KPlayerSettingsAdvancedPage
{
  Q_OBJECT

public:
  KPlayerSettingsAdvanced (QWidget* parent = 0, const char* name = 0);

  KPlayerEngine* engine (void)
    { return KPlayerEngine::engine(); }
  KPlayerConfiguration* configuration (void)
    { return engine() -> configuration(); }

  void load (void);
  void save (void);
  void refreshLists (void);

public slots:
  void refresh (void);

protected:
  void loadLists (void);
  virtual void cacheChanged (int);
};

class KPlayerSettingsAudio : public KPlayerSettingsAudioPage
{
  Q_OBJECT

public:
  KPlayerSettingsAudio (QWidget* parent = 0, const char* name = 0);

  KPlayerEngine* engine (void)
    { return KPlayerEngine::engine(); }
  KPlayerConfiguration* configuration (void)
    { return engine() -> configuration(); }

  void load (void);
  void save (void);

public slots:
  void refresh (void);

protected slots:
  /** Processes an amixer output line. */
  void amixerOutput (KPlayerLineOutputProcess*, char*, int);
  /** Finishes refreshing ALSA mixer channel list. */
  void amixerExited (KProcess*);

protected:
  void loadLists (void);
  void runAmixer (void);
  void defaultAlsaChannels (void);
  virtual void driverChanged (int);
  virtual void deviceChanged (const QString&);
  virtual void mixerChanged (const QString&);
  virtual void softvolChanged (bool);

  /** Last selected driver. */
  QString m_driver;
  /** Last entered device. */
  QString m_device;
  /** Last software volume setting. */
  bool m_softvol;
  /** Amixer tracking flags. */
  bool m_amixer_running, m_rerun_amixer;
};

class KPlayerSettingsControls : public KPlayerSettingsControlsPage
{
  Q_OBJECT

public:
  KPlayerSettingsControls (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  virtual void rememberSizeChanged (bool);
};

class KPlayerSettingsSliders : public KPlayerSettingsSlidersPage
{
  Q_OBJECT

public:
  KPlayerSettingsSliders (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);
};

class KPlayerSettingsGeneral : public KPlayerSettingsGeneralPage
{
  Q_OBJECT

public:
  KPlayerSettingsGeneral (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  void resizeAutomaticallyChanged (bool);
};

class KPlayerSettingsSubtitles : public KPlayerSettingsSubtitlesPage
{
  Q_OBJECT

public:
  KPlayerSettingsSubtitles (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  virtual void autoloadSubtitlesChanged (bool);
  virtual void otherSubtitlesChanged (bool);
};

class KPlayerSettingsVideo : public KPlayerSettingsVideoPage
{
  Q_OBJECT

public:
  KPlayerSettingsVideo (QWidget* parent = 0, const char* name = 0);

  KPlayerEngine* engine (void)
    { return KPlayerEngine::engine(); }
  KPlayerConfiguration* configuration (void)
    { return engine() -> configuration(); }

  void load (void);
  void save (void);

public slots:
  void refresh (void);

protected:
  void loadLists (void);
  virtual void driverChanged (int);
};

class KPlayerSettingsProgress : public KPlayerSettingsProgressPage
{
  Q_OBJECT

public:
  KPlayerSettingsProgress (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);
};

class KPlayerSettingsVolume : public KPlayerSettingsVolumePage
{
  Q_OBJECT

public:
  KPlayerSettingsVolume (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  virtual void resetChanged (bool);
};

class KPlayerSettingsContrast : public KPlayerSettingsContrastPage
{
  Q_OBJECT

public:
  KPlayerSettingsContrast (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  virtual void resetChanged (bool);
};

class KPlayerSettingsBrightness : public KPlayerSettingsBrightnessPage
{
  Q_OBJECT

public:
  KPlayerSettingsBrightness (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  virtual void resetChanged (bool);
};

class KPlayerSettingsHue : public KPlayerSettingsHuePage
{
  Q_OBJECT

public:
  KPlayerSettingsHue (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  virtual void resetChanged (bool);
};

class KPlayerSettingsSaturation : public KPlayerSettingsSaturationPage
{
  Q_OBJECT

public:
  KPlayerSettingsSaturation (QWidget* parent = 0, const char* name = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  virtual void resetChanged (bool);
};

/** The KPlayer settings dialog.
  *@author kiriuja
  */
class KPlayerSettingsDialog : public KDialogBase
{
  Q_OBJECT

public:
  KPlayerSettingsDialog (QWidget*);
  virtual ~KPlayerSettingsDialog();

  /** Configuration. */
  KPlayerConfiguration* configuration (void) const
    { return KPlayerEngine::engine() -> configuration(); }

protected:
/*virtual void moveEvent (QMoveEvent*);
  virtual void resizeEvent (QResizeEvent*);
  virtual void showEvent (QShowEvent*);
  virtual void windowActivationChange (bool);

  bool m_initial_move;*/

  KPlayerSettingsAdvanced* m_advanced;
  KPlayerSettingsAudio* m_audio;
  KPlayerSettingsControls* m_controls;
  KPlayerSettingsGeneral* m_general;
  KPlayerSettingsSliders* m_sliders;
  KPlayerSettingsSubtitles* m_subtitles;
  KPlayerSettingsVideo* m_video;
  KPlayerSettingsProgress* m_progress;
  KPlayerSettingsVolume* m_volume;
  KPlayerSettingsContrast* m_contrast;
  KPlayerSettingsBrightness* m_brightness;
  KPlayerSettingsHue* m_hue;
  KPlayerSettingsSaturation* m_saturation;

  virtual void slotOk (void);
  virtual void slotApply (void);
  virtual void slotDefault (void);

protected slots:
  void pageAboutToShow (QWidget*);
};

#endif
