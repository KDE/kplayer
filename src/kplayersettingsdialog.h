/***************************************************************************
                          kplayersettingsdialog.h
                          -----------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERSETTINGSDIALOG_H
#define KPLAYERSETTINGSDIALOG_H

#include <KPageDialog>

#include "ui_kplayersettingsadvanced.h"
#include "ui_kplayersettingsaudio.h"
#include "ui_kplayersettingscontrols.h"
#include "ui_kplayersettingsgeneral.h"
#include "ui_kplayersettingssliders.h"
#include "ui_kplayersettingssubtitles.h"
#include "ui_kplayersettingsvideo.h"
#include "ui_kplayersettingsprogress.h"
#include "ui_kplayersettingsvolume.h"
#include "ui_kplayersettingscontrast.h"
#include "ui_kplayersettingsbrightness.h"
#include "ui_kplayersettingshue.h"
#include "ui_kplayersettingssaturation.h"
#include "kplayerproperties.h"

QString listEntry (QComboBox* combo, bool hasDefault = false);

class KPlayerSettingsAdvanced : public QFrame, protected Ui_KPlayerSettingsAdvancedPage
{
  Q_OBJECT

public:
  KPlayerSettingsAdvanced (QWidget* parent = 0);

  KPlayerEngine* engine (void)
    { return KPlayerEngine::engine(); }
  KPlayerConfiguration* configuration (void)
    { return engine() -> configuration(); }

  void load (void);
  void save (void);
  void refreshLists (void);

public slots:
  void refresh (void);

protected slots:
  void cacheChanged (int);

protected:
  void loadLists (void);
};

class KPlayerSettingsAudio : public QFrame, protected Ui_KPlayerSettingsAudioPage
{
  Q_OBJECT

public:
  KPlayerSettingsAudio (QWidget* parent = 0);

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
  void amixerOutput (KPlayerLineOutputProcess*, char*);
  /** Finishes refreshing ALSA mixer channel list. */
  void amixerFinished (KPlayerLineOutputProcess*);

  void driverChanged (int);
  void deviceChanged (const QString&);
  void mixerChanged (const QString&);
  void softvolChanged (bool);

protected:
  void loadLists (void);
  void runAmixer (void);
  void defaultAlsaChannels (void);

  /** Last selected driver. */
  QString m_driver;
  /** Last entered device. */
  QString m_device;
  /** Last software volume setting. */
  bool m_softvol;
  /** Amixer tracking flags. */
  bool m_amixer_running, m_rerun_amixer;
  /** Device listed indicator. */
  bool m_devices_listed;
};

class KPlayerSettingsControls : public QFrame, protected Ui_KPlayerSettingsControlsPage
{
  Q_OBJECT

public:
  KPlayerSettingsControls (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected slots:
  void rememberSizeChanged (bool);
};

class KPlayerSettingsSliders : public QFrame, protected Ui_KPlayerSettingsSlidersPage
{
  Q_OBJECT

public:
  KPlayerSettingsSliders (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected slots:
  void showMarksChanged (bool);
  void minimumSliderLengthChanged (int);
};

class KPlayerSettingsGeneral : public QFrame, protected Ui_KPlayerSettingsGeneralPage
{
  Q_OBJECT

public:
  KPlayerSettingsGeneral (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);
};

class KPlayerSettingsSubtitles : public QFrame, protected Ui_KPlayerSettingsSubtitlesPage
{
  Q_OBJECT

public:
  KPlayerSettingsSubtitles (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void loadLists (void);
  void load (void);
  void save (void);

protected slots:
  void widthSliderChanged (int);
  void widthEditChanged (const int);
  void outlineSliderChanged (int);
  void outlineEditChanged (const double);
  void autoexpandChanged (bool);
  void autoloadSubtitlesChanged (bool);

protected:
  /** Recursion prevention. */
  bool m_recursion;
  /** Leave focus alone. */
  bool m_initialized;
};

class KPlayerSettingsVideo : public QFrame, protected Ui_KPlayerSettingsVideoPage
{
  Q_OBJECT

public:
  KPlayerSettingsVideo (QWidget* parent = 0);

  KPlayerEngine* engine (void)
    { return KPlayerEngine::engine(); }
  KPlayerConfiguration* configuration (void)
    { return engine() -> configuration(); }

  void load (void);
  void save (void);

public slots:
  void refresh (void);

protected slots:
  void driverChanged (int);

protected:
  void loadLists (void);
};

class KPlayerSettingsProgress : public QFrame, protected Ui_KPlayerSettingsProgressPage
{
  Q_OBJECT

public:
  KPlayerSettingsProgress (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected:
  enum IntervalUnits {percent=0, seconds};
  
protected slots:
  void normalSeekAmountChanged(int);
  void normalSeekUnitsChanged();
  void fastSeekAmountChanged(int);
  void fastSeekUnitsChanged();
};

class KPlayerSettingsVolume : public QFrame, protected Ui_KPlayerSettingsVolumePage
{
  Q_OBJECT

public:
  KPlayerSettingsVolume (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected slots:
  void resetChanged (bool);
  void minimumVolumeValueChanged(int);
  void maximumVolumeValueChanged(int);
};

class KPlayerSettingsContrast : public QFrame, protected Ui_KPlayerSettingsContrastPage
{
  Q_OBJECT

public:
  KPlayerSettingsContrast (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected slots:
  void resetChanged (bool);
  void minimumContrastValueChanged(int);
  void maximumContrastValueChanged(int);
};

class KPlayerSettingsBrightness : public QFrame, protected Ui_KPlayerSettingsBrightnessPage
{
  Q_OBJECT

public:
  KPlayerSettingsBrightness (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected slots:
  void resetChanged (bool);
  void minimumBrightnessValueChanged(int);
  void maximumBrightnessValueChanged(int);
};

class KPlayerSettingsHue : public QFrame, protected Ui_KPlayerSettingsHuePage
{
  Q_OBJECT

public:
  KPlayerSettingsHue (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected slots:
  void resetChanged (bool);
  void minimumHueValueChanged(int);
  void maximumHueValueChanged(int);
};

class KPlayerSettingsSaturation : public QFrame, protected Ui_KPlayerSettingsSaturationPage
{
  Q_OBJECT

public:
  KPlayerSettingsSaturation (QWidget* parent = 0);

  KPlayerConfiguration* configuration (void)
    { return KPlayerEngine::engine() -> configuration(); }

  void load (void);
  void save (void);

protected slots:
  void resetChanged (bool);
  void minimumSaturationValueChanged(int);
  void maximumSaturationValueChanged(int);
};

/** The KPlayer settings dialog.
  *@author kiriuja
  */
class KPlayerSettingsDialog : public KPageDialog
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

  QHash<QObject*, QString> m_page_names;

protected slots:
  void apply (void);
  void defaults (void);
  void pageAboutToShow (KPageWidgetItem* current, KPageWidgetItem* before);
};

#endif
