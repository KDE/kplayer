/***************************************************************************
                         kplayerpropertiesdialog.h
                         -------------------------
    begin                : Tue Mar 02 2004
    copyright            : (C) 2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERPROPERTIESDIALOG_H
#define KPLAYERPROPERTIESDIALOG_H

#include <kdialogbase.h>

#include "kplayerpropertiesgeneral.h"
#include "kplayerpropertiessubtitles.h"
#include "kplayerpropertiesaudio.h"
#include "kplayerpropertiesvideo.h"
#include "kplayerpropertiesadvanced.h"

class KPlayerProperties;

class KPlayerPropertiesGeneral : public KPlayerPropertiesGeneralPage
{
  Q_OBJECT

public:
  KPlayerPropertiesGeneral (KPlayerProperties*, QWidget* parent = 0, const char* name = 0);

  void load (void);
  void save (void);

protected:
  virtual void displaySizeChanged (int);

  KPlayerProperties* m_properties;
};

class KPlayerPropertiesSubtitles : public KPlayerPropertiesSubtitlesPage
{
  Q_OBJECT

public:
  KPlayerPropertiesSubtitles (KPlayerProperties*, QWidget* parent = 0, const char* name = 0);

  void load (void);
  void save (void);

protected:
  virtual void autoloadChanged (int);
  virtual void positionChanged (int);
  virtual void delayChanged (int);

  KPlayerProperties* m_properties;
};

class KPlayerPropertiesAudio : public KPlayerPropertiesAudioPage
{
  Q_OBJECT

public:
  KPlayerPropertiesAudio (KPlayerProperties*, QWidget* parent = 0, const char* name = 0);

  void load (void);
  void save (void);

protected:
  void loadLists (void);
  virtual void volumeChanged (int);
  virtual void delayChanged (int);
  virtual void codecChanged (int);

  KPlayerProperties* m_properties;
  QString m_default_codec;
};

class KPlayerPropertiesVideo : public KPlayerPropertiesVideoPage
{
  Q_OBJECT

public:
  KPlayerPropertiesVideo (KPlayerProperties*, QWidget* parent = 0, const char* name = 0);

  void load (void);
  void save (void);

protected:
  void loadLists (void);
  virtual void contrastChanged (int);
  virtual void brightnessChanged (int);
  virtual void hueChanged (int);
  virtual void saturationChanged (int);
  virtual void codecChanged (int);

  KPlayerProperties* m_properties;
  QString m_default_codec;
};

class KPlayerPropertiesAdvanced : public KPlayerPropertiesAdvancedPage
{
  Q_OBJECT

public:
  KPlayerPropertiesAdvanced (KPlayerProperties*, QWidget* parent = 0, const char* name = 0);

  void load (void);
  void save (void);

protected:
  virtual void commandLineChanged (int);
  virtual void cacheChanged (int);

  KPlayerProperties* m_properties;
};

/** The KPlayer file properties dialog.
  *@author kiriuja
  */
class KPlayerPropertiesDialog : public KDialogBase
{
  Q_OBJECT
public:
  KPlayerPropertiesDialog (KPlayerProperties*);
  virtual ~KPlayerPropertiesDialog();

protected:
  KPlayerPropertiesGeneral* m_general;
  KPlayerPropertiesSubtitles* m_subtitles;
  KPlayerPropertiesAudio* m_audio;
  KPlayerPropertiesVideo* m_video;
  KPlayerPropertiesAdvanced* m_advanced;

  KPlayerProperties* m_properties;

  virtual void slotOk (void);
  virtual void slotApply (void);
  virtual void slotDefault (void);

protected slots:
  void pageAboutToShow (QWidget*);
};

#endif
