/***************************************************************************
                         kplayerpropertiesdialog.h
                         -------------------------
    begin                : Tue Mar 02 2004
    copyright            : (C) 2004-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERPROPERTIESDIALOG_H
#define KPLAYERPROPERTIESDIALOG_H

#include <kpagedialog.h>

#include "ui_kplayerpropertiesgeneral.h"
#include "ui_kplayerpropertiessize.h"
#include "ui_kplayerpropertiessubtitles.h"
#include "ui_kplayerpropertiesaudio.h"
#include "ui_kplayerpropertiesvideo.h"
#include "ui_kplayerpropertiesadvanced.h"
#include "kplayerengine.h"

class KPlayerMediaProperties;
class KPlayerDeviceProperties;
class KPlayerTVProperties;
class KPlayerDVBProperties;
class KPlayerTrackProperties;
class KPlayerDiskTrackProperties;
class KPlayerChannelProperties;
class KPlayerItemProperties;

class KPlayerPropertiesGeneral : public QFrame, protected Ui_KPlayerPropertiesGeneralPage
{
  Q_OBJECT

public:
  KPlayerPropertiesGeneral (QWidget* parent = 0);
  virtual ~KPlayerPropertiesGeneral();

  void setup (const KUrl&);
  virtual void setupMedia (const KUrl&) = 0;
  virtual void setupControls (void) = 0;
  void hideUrl (void);
  void hideFrequency (void);
  void hideLength (void);
  void hidePlaylist (void);
  void hideTV (void);
  void hideDVB (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerEngine* engine (void) const
    { return KPlayerEngine::engine(); }
  KPlayerMediaProperties* properties (void) const
    { return m_properties; }

protected:
  KPlayerMediaProperties* m_properties;
};

class KPlayerPropertiesDeviceGeneral : public KPlayerPropertiesGeneral
{
  Q_OBJECT

public:
  KPlayerPropertiesDeviceGeneral (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);

  KPlayerDeviceProperties* properties (void) const
    { return (KPlayerDeviceProperties*) KPlayerPropertiesGeneral::properties(); }
};

class KPlayerPropertiesTVDeviceGeneral : public KPlayerPropertiesDeviceGeneral
{
  Q_OBJECT

public:
  KPlayerPropertiesTVDeviceGeneral (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerTVProperties* properties (void) const
    { return (KPlayerTVProperties*) KPlayerPropertiesGeneral::properties(); }
};

class KPlayerPropertiesDVBDeviceGeneral : public KPlayerPropertiesDeviceGeneral
{
  Q_OBJECT

public:
  KPlayerPropertiesDVBDeviceGeneral (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerDVBProperties* properties (void) const
    { return (KPlayerDVBProperties*) KPlayerPropertiesGeneral::properties(); }
};

class KPlayerPropertiesTrackGeneral : public KPlayerPropertiesGeneral
{
  Q_OBJECT

public:
  KPlayerPropertiesTrackGeneral (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void load (void);

  KPlayerTrackProperties* properties (void) const
    { return (KPlayerTrackProperties*) KPlayerPropertiesGeneral::properties(); }
};

class KPlayerPropertiesDiskTrackGeneral : public KPlayerPropertiesTrackGeneral
{
  Q_OBJECT

public:
  KPlayerPropertiesDiskTrackGeneral (QWidget* parent = 0);

  virtual void setupControls (void);
  virtual void load (void);

  KPlayerDiskTrackProperties* properties (void) const
    { return (KPlayerDiskTrackProperties*) KPlayerPropertiesGeneral::properties(); }
};

class KPlayerPropertiesChannelGeneral : public KPlayerPropertiesDiskTrackGeneral
{
  Q_OBJECT

public:
  KPlayerPropertiesChannelGeneral (QWidget* parent = 0);

  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerChannelProperties* properties (void) const
    { return (KPlayerChannelProperties*) KPlayerPropertiesGeneral::properties(); }
};

class KPlayerPropertiesItemGeneral : public KPlayerPropertiesTrackGeneral
{
  Q_OBJECT

public:
  KPlayerPropertiesItemGeneral (QWidget* parent = 0);

  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerItemProperties* properties (void) const
    { return (KPlayerItemProperties*) KPlayerPropertiesGeneral::properties(); }
};

class KPlayerPropertiesSize : public QFrame, protected Ui_KPlayerPropertiesSizePage
{
  Q_OBJECT

public:
  KPlayerPropertiesSize (QWidget* parent = 0);
  virtual ~KPlayerPropertiesSize();

  void setup (const KUrl&);
  virtual void setupMedia (const KUrl&) = 0;
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerMediaProperties* properties (void) const
    { return m_properties; }

protected slots:
  void displaySizeChanged (int);

protected:
  KPlayerMediaProperties* m_properties;
};

class KPlayerPropertiesDeviceSize : public KPlayerPropertiesSize
{
  Q_OBJECT

public:
  KPlayerPropertiesDeviceSize (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);

  KPlayerDeviceProperties* properties (void) const
    { return (KPlayerDeviceProperties*) KPlayerPropertiesSize::properties(); }
};

class KPlayerPropertiesTrackSize : public KPlayerPropertiesSize
{
  Q_OBJECT

public:
  KPlayerPropertiesTrackSize (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void load (void);

  KPlayerTrackProperties* properties (void) const
    { return (KPlayerTrackProperties*) KPlayerPropertiesSize::properties(); }
};

class KPlayerPropertiesSubtitles : public QFrame, protected Ui_KPlayerPropertiesSubtitlesPage
{
  Q_OBJECT

public:
  KPlayerPropertiesSubtitles (QWidget* parent = 0);
  virtual ~KPlayerPropertiesSubtitles();

  void setup (const KUrl&);
  virtual void setupMedia (const KUrl&) = 0;
  virtual void setupControls (void) = 0;
  void hideTrack (void);
  void hideAutoload (void);
  void hideClosedCaption (void);
  void hideUrl (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerMediaProperties* properties (void) const
    { return m_properties; }

protected slots:
  void positionChanged (int);
  void delayChanged (int);

protected:
  KPlayerMediaProperties* m_properties;
};

class KPlayerPropertiesDeviceSubtitles : public KPlayerPropertiesSubtitles
{
  Q_OBJECT

public:
  KPlayerPropertiesDeviceSubtitles (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);

  KPlayerDeviceProperties* properties (void) const
    { return (KPlayerDeviceProperties*) KPlayerPropertiesSubtitles::properties(); }
};

class KPlayerPropertiesTrackSubtitles : public KPlayerPropertiesSubtitles
{
  Q_OBJECT

public:
  KPlayerPropertiesTrackSubtitles (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  void addTracks (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerTrackProperties* properties (void) const
    { return (KPlayerTrackProperties*) KPlayerPropertiesSubtitles::properties(); }

protected slots:
  void trackChanged (int);
};

class KPlayerPropertiesChannelSubtitles : public KPlayerPropertiesTrackSubtitles
{
  Q_OBJECT

public:
  KPlayerPropertiesChannelSubtitles (QWidget* parent = 0);

  virtual void setupControls (void);

  KPlayerChannelProperties* properties (void) const
    { return (KPlayerChannelProperties*) KPlayerPropertiesSubtitles::properties(); }
};

class KPlayerPropertiesDiskTrackSubtitles : public KPlayerPropertiesTrackSubtitles
{
  Q_OBJECT

public:
  KPlayerPropertiesDiskTrackSubtitles (QWidget* parent = 0);

  virtual void setupControls (void);
  void setupEncoding (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerDiskTrackProperties* properties (void) const
    { return (KPlayerDiskTrackProperties*) KPlayerPropertiesSubtitles::properties(); }
};

class KPlayerPropertiesItemSubtitles : public KPlayerPropertiesDiskTrackSubtitles
{
  Q_OBJECT

public:
  KPlayerPropertiesItemSubtitles (QWidget* parent = 0);

  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerItemProperties* properties (void) const
    { return (KPlayerItemProperties*) KPlayerPropertiesSubtitles::properties(); }
};

class KPlayerPropertiesAudio : public QFrame, protected Ui_KPlayerPropertiesAudioPage
{
  Q_OBJECT

public:
  KPlayerPropertiesAudio (QWidget* parent = 0);
  virtual ~KPlayerPropertiesAudio();

  void setup (const KUrl&);
  virtual void setupMedia (const KUrl&) = 0;
  virtual void setupControls (void) = 0;
  void hideTrack (void);
  void hideRates (void);
  void hideInput (void);
  void hideTV (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerEngine* engine (void) const
    { return KPlayerEngine::engine(); }
  KPlayerMediaProperties* properties (void) const
    { return m_properties; }

protected slots:
  void volumeChanged (int);
  void delayChanged (int);

protected:
  KPlayerMediaProperties* m_properties;
};

class KPlayerPropertiesDeviceAudio : public KPlayerPropertiesAudio
{
  Q_OBJECT

public:
  KPlayerPropertiesDeviceAudio (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);

  KPlayerDeviceProperties* properties (void) const
    { return (KPlayerDeviceProperties*) KPlayerPropertiesAudio::properties(); }
};

class KPlayerPropertiesDVBDeviceAudio : public KPlayerPropertiesDeviceAudio
{
  Q_OBJECT

public:
  KPlayerPropertiesDVBDeviceAudio (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerDVBProperties* properties (void) const
    { return (KPlayerDVBProperties*) KPlayerPropertiesAudio::properties(); }

protected slots:
  void inputChanged (int);
};

class KPlayerPropertiesTVDeviceAudio : public KPlayerPropertiesDVBDeviceAudio
{
  Q_OBJECT

public:
  KPlayerPropertiesTVDeviceAudio (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerTVProperties* properties (void) const
    { return (KPlayerTVProperties*) KPlayerPropertiesAudio::properties(); }
};

class KPlayerPropertiesTrackAudio : public KPlayerPropertiesAudio
{
  Q_OBJECT

public:
  KPlayerPropertiesTrackAudio (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerTrackProperties* properties (void) const
    { return (KPlayerTrackProperties*) KPlayerPropertiesAudio::properties(); }

protected slots:
  void trackChanged (int);
};

class KPlayerPropertiesVideo : public QFrame, protected Ui_KPlayerPropertiesVideoPage
{
  Q_OBJECT

public:
  KPlayerPropertiesVideo (QWidget* parent = 0);
  virtual ~KPlayerPropertiesVideo();

  void setup (const KUrl&);
  virtual void setupMedia (const KUrl&) = 0;
  virtual void setupControls (void) = 0;
  void hideTrack (void);
  void hideRates (void);
  void hideInput (void);
  void hideTV (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerEngine* engine (void) const
    { return KPlayerEngine::engine(); }
  KPlayerMediaProperties* properties (void) const
    { return m_properties; }

protected slots:
  void contrastChanged (int);
  void brightnessChanged (int);
  void hueChanged (int);
  void saturationChanged (int);

protected:
  void loadLists (void);

  KPlayerMediaProperties* m_properties;
};

class KPlayerPropertiesDeviceVideo : public KPlayerPropertiesVideo
{
  Q_OBJECT

public:
  KPlayerPropertiesDeviceVideo (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);

  KPlayerDeviceProperties* properties (void) const
    { return (KPlayerDeviceProperties*) KPlayerPropertiesVideo::properties(); }
};

class KPlayerPropertiesDVBDeviceVideo : public KPlayerPropertiesDeviceVideo
{
  Q_OBJECT

public:
  KPlayerPropertiesDVBDeviceVideo (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerDVBProperties* properties (void) const
    { return (KPlayerDVBProperties*) KPlayerPropertiesVideo::properties(); }

protected slots:
  void inputChanged (int);
};

class KPlayerPropertiesTVDeviceVideo : public KPlayerPropertiesDVBDeviceVideo
{
  Q_OBJECT

public:
  KPlayerPropertiesTVDeviceVideo (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerTVProperties* properties (void) const
    { return (KPlayerTVProperties*) KPlayerPropertiesVideo::properties(); }

protected slots:
  void normChanged (int);
};

class KPlayerPropertiesTrackVideo : public KPlayerPropertiesVideo
{
  Q_OBJECT

public:
  KPlayerPropertiesTrackVideo (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerTrackProperties* properties (void) const
    { return (KPlayerTrackProperties*) KPlayerPropertiesVideo::properties(); }

protected slots:
  void trackChanged (int);
};

class KPlayerPropertiesAdvanced : public QFrame, protected Ui_KPlayerPropertiesAdvancedPage
{
  Q_OBJECT

public:
  KPlayerPropertiesAdvanced (QWidget* parent = 0);
  virtual ~KPlayerPropertiesAdvanced();

  void setup (const KUrl&);
  virtual void setupMedia (const KUrl&) = 0;
  virtual void setupControls (void);
  void hideKioslave (void);
  void hideCompression (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerEngine* engine (void) const
    { return KPlayerEngine::engine(); }
  KPlayerMediaProperties* properties (void) const
    { return m_properties; }

protected slots:
  void commandLineChanged (int);
  void cacheChanged (int);

protected:
  KPlayerMediaProperties* m_properties;
};

class KPlayerPropertiesDeviceAdvanced : public KPlayerPropertiesAdvanced
{
  Q_OBJECT

public:
  KPlayerPropertiesDeviceAdvanced (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);

  KPlayerDeviceProperties* properties (void) const
    { return (KPlayerDeviceProperties*) KPlayerPropertiesAdvanced::properties(); }
};

class KPlayerPropertiesTVDeviceAdvanced : public KPlayerPropertiesDeviceAdvanced
{
  Q_OBJECT

public:
  KPlayerPropertiesTVDeviceAdvanced (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);
  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerTVProperties* properties (void) const
    { return (KPlayerTVProperties*) KPlayerPropertiesAdvanced::properties(); }

protected slots:
  void compressionChanged (bool);
};

class KPlayerPropertiesTrackAdvanced : public KPlayerPropertiesAdvanced
{
  Q_OBJECT

public:
  KPlayerPropertiesTrackAdvanced (QWidget* parent = 0);

  virtual void setupMedia (const KUrl&);

  KPlayerTrackProperties* properties (void) const
    { return (KPlayerTrackProperties*) KPlayerPropertiesAdvanced::properties(); }
};

class KPlayerPropertiesItemAdvanced : public KPlayerPropertiesTrackAdvanced
{
  Q_OBJECT

public:
  KPlayerPropertiesItemAdvanced (QWidget* parent = 0);

  virtual void setupControls (void);
  virtual void load (void);
  virtual void save (void);

  KPlayerItemProperties* properties (void) const
    { return (KPlayerItemProperties*) KPlayerPropertiesAdvanced::properties(); }
};

/** The KPlayer properties dialog.
  *@author kiriuja
  */
class KPlayerPropertiesDialog : public KPageDialog
{
  Q_OBJECT

public:
  KPlayerPropertiesDialog (void);
  virtual ~KPlayerPropertiesDialog();

  void setup (const KUrl&);
  virtual void setupMedia (const KUrl&) = 0;

  KPlayerMediaProperties* properties (void) const
    { return m_properties; }

  static KPlayerPropertiesDialog* createDialog (KPlayerTrackProperties* properties);

protected:
  virtual KPlayerPropertiesGeneral* createGeneralPage (void) = 0;
  virtual KPlayerPropertiesSize* createSizePage (void) = 0;
  virtual KPlayerPropertiesSubtitles* createSubtitlesPage (void) = 0;
  virtual KPlayerPropertiesAudio* createAudioPage (void) = 0;
  virtual KPlayerPropertiesVideo* createVideoPage (void) = 0;
  virtual KPlayerPropertiesAdvanced* createAdvancedPage (void) = 0;

  KPlayerPropertiesGeneral* m_general;
  KPlayerPropertiesSize* m_size;
  KPlayerPropertiesSubtitles* m_subtitles;
  KPlayerPropertiesAudio* m_audio;
  KPlayerPropertiesVideo* m_video;
  KPlayerPropertiesAdvanced* m_advanced;

  KPlayerMediaProperties* m_properties;

  QHash<QObject*, QString> m_page_names;

protected slots:
  void apply (void);
  void defaults (void);
  void pageAboutToShow (KPageWidgetItem* current, KPageWidgetItem* before);
};

/** The KPlayer device properties dialog.
  *@author kiriuja
  */
class KPlayerDevicePropertiesDialog : public KPlayerPropertiesDialog
{
  Q_OBJECT

public:
  KPlayerDevicePropertiesDialog (void) { }
  virtual ~KPlayerDevicePropertiesDialog();

  virtual void setupMedia (const KUrl&);

  virtual KPlayerPropertiesGeneral* createGeneralPage (void);
  virtual KPlayerPropertiesSize* createSizePage (void);
  virtual KPlayerPropertiesSubtitles* createSubtitlesPage (void);
  virtual KPlayerPropertiesAudio* createAudioPage (void);
  virtual KPlayerPropertiesVideo* createVideoPage (void);
  virtual KPlayerPropertiesAdvanced* createAdvancedPage (void);

  KPlayerDeviceProperties* properties (void) const
    { return (KPlayerDeviceProperties*) KPlayerPropertiesDialog::properties(); }
};

/** The KPlayer TV device properties dialog.
  *@author kiriuja
  */
class KPlayerTVDevicePropertiesDialog : public KPlayerDevicePropertiesDialog
{
  Q_OBJECT

public:
  KPlayerTVDevicePropertiesDialog (void) { }
  virtual ~KPlayerTVDevicePropertiesDialog();

  virtual void setupMedia (const KUrl&);

  virtual KPlayerPropertiesGeneral* createGeneralPage (void);
  virtual KPlayerPropertiesAudio* createAudioPage (void);
  virtual KPlayerPropertiesVideo* createVideoPage (void);
  virtual KPlayerPropertiesAdvanced* createAdvancedPage (void);

  KPlayerTVProperties* properties (void) const
    { return (KPlayerTVProperties*) KPlayerPropertiesDialog::properties(); }
};

/** The KPlayer DVB device properties dialog.
  *@author kiriuja
  */
class KPlayerDVBDevicePropertiesDialog : public KPlayerDevicePropertiesDialog
{
  Q_OBJECT

public:
  KPlayerDVBDevicePropertiesDialog (void) { }
  virtual ~KPlayerDVBDevicePropertiesDialog();

  virtual void setupMedia (const KUrl&);

  virtual KPlayerPropertiesGeneral* createGeneralPage (void);
  virtual KPlayerPropertiesAudio* createAudioPage (void);
  virtual KPlayerPropertiesVideo* createVideoPage (void);

  KPlayerDVBProperties* properties (void) const
    { return (KPlayerDVBProperties*) KPlayerPropertiesDialog::properties(); }
};

/** The KPlayer track properties dialog.
  *@author kiriuja
  */
class KPlayerDiskTrackPropertiesDialog : public KPlayerPropertiesDialog
{
  Q_OBJECT

public:
  KPlayerDiskTrackPropertiesDialog (void) { }
  virtual ~KPlayerDiskTrackPropertiesDialog();

  virtual void setupMedia (const KUrl&);

  virtual KPlayerPropertiesGeneral* createGeneralPage (void);
  virtual KPlayerPropertiesSize* createSizePage (void);
  virtual KPlayerPropertiesSubtitles* createSubtitlesPage (void);
  virtual KPlayerPropertiesAudio* createAudioPage (void);
  virtual KPlayerPropertiesVideo* createVideoPage (void);
  virtual KPlayerPropertiesAdvanced* createAdvancedPage (void);

  KPlayerDiskTrackProperties* properties (void) const
    { return (KPlayerDiskTrackProperties*) KPlayerPropertiesDialog::properties(); }
};

/** The KPlayer channel properties dialog.
  *@author kiriuja
  */
class KPlayerChannelPropertiesDialog : public KPlayerDiskTrackPropertiesDialog
{
  Q_OBJECT

public:
  KPlayerChannelPropertiesDialog (void) { }
  virtual ~KPlayerChannelPropertiesDialog();

  virtual KPlayerPropertiesGeneral* createGeneralPage (void);
  virtual KPlayerPropertiesSubtitles* createSubtitlesPage (void);

  KPlayerChannelProperties* properties (void) const
    { return (KPlayerChannelProperties*) KPlayerPropertiesDialog::properties(); }
};

/** The KPlayer item properties dialog.
  *@author kiriuja
  */
class KPlayerItemPropertiesDialog : public KPlayerDiskTrackPropertiesDialog
{
  Q_OBJECT

public:
  KPlayerItemPropertiesDialog (void) { }
  virtual ~KPlayerItemPropertiesDialog();

  virtual KPlayerPropertiesGeneral* createGeneralPage (void);
  virtual KPlayerPropertiesSubtitles* createSubtitlesPage (void);
  virtual KPlayerPropertiesAdvanced* createAdvancedPage (void);

  KPlayerItemProperties* properties (void) const
    { return (KPlayerItemProperties*) KPlayerPropertiesDialog::properties(); }
};

#endif
