/***************************************************************************
                          kplayerengine.h
                          ---------------
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

#ifndef KPLAYERENGINE_H
#define KPLAYERENGINE_H

#include <kaction.h>
#include <kdiroperator.h>
#include <kfiledialog.h>

#include "kplayerprocess.h"

class KConfig;
class KProcess;
class KPlayerLineOutputProcess;
class KPlayerProcess;
class KPlayerProperties;
class KPlayerSettings;
class KPlayerWidget;
class KPlayerWorkspace;
class KPlayerSliderAction;
class KPlayerPopupSliderAction;
class KURL;
class KURL::List;

int listIndex (const QStringList&, const QString&);

/** The KPlayer file dialog.
  *@author kiriuja
  */
class KPlayerFileDialog : public KFileDialog
{
   Q_OBJECT

public: 
  /** The dialog constructor.
      Parameters are passed to the KFileDialog constructor. */
  KPlayerFileDialog (const QString& dir, const QString& filter, QWidget* parent, const char* name);

  /** Returns the last used directory.
    */
  QString directory (void)
  {
    return ops -> url().path (-1);
  }
};

/** The KPlayer engine.
  *@author kiriuja
  */
class KPlayerEngine : public QObject
{
  Q_OBJECT

public:
  KPlayerEngine (KActionCollection* ac, QWidget* parent, const char* name, KConfig*);
  virtual ~KPlayerEngine();

  /** Returns whether the engine is running in the KPart mode.
   */
  bool light (void)
    { return m_light; }

  /** Returns whether playback is being stopped at user request.
   */
  bool stopped (void)
    { return m_stop; }

  /** Starts loading of all mplayer drivers and codecs. The refresh signal is emitted when loading finishes.
   */
  void getDriversCodecs (QString = QString::null);

  /** Creates actions and connects signals to slots.
   */
  void setupActions (void);
  /** Enables or disables player actions.
    */
  void enablePlayerActions (void);
  /** Enables or disables video actions.
    */
  void enableVideoActions (void);
  /** Enables or disables subtitle actions.
    */
  void enableSubtitleActions (void);
  /** Enables or disables, checks or unchecks zoom actions.
    */
  void enableZoomActions (void);

  /** Loads the given URL and optionally starts playback.
    */
  void load (KURL);
  /** Autoloads subtitles as specified in the settings.
   */
  void autoloadSubtitles (void);
  /** Loads the given subtitle URL and restarts playback if needed.
   */
  void loadSubtitle (KURL);

  /** Displays the open file dialog and lets the user choose a file or files.
      Returns the list of chosen URLs or an empty list if the user did not choose any. */
  KURL::List openFiles (QWidget* = 0);
  /** Displays the URL requester dialog and lets the user enter a URL.
      Returns the entered URL or an empty URL if the user did not enter any. */
  KURL openUrl (QWidget* = 0);
  /** Displays the open file dialog and lets the user choose a subtitle file.
      Returns the chosen URL or an empty URL if the user did not choose any. */
  KURL openSubtitle (QWidget* = 0);
  /** Displays the URL requester dialog and lets the user enter a subtitle URL.
      Returns the entered URL or an empty URL if the user did not enter any. */
  KURL openSubtitleUrl (QWidget* = 0);

  /** Retrieves an action from the actionCollection by name.
    */
  KAction* action (const char* name) const
    { return m_ac -> action (name); }
  /** Retrieves a toggle action from the actionCollection by name.
    */
  KToggleAction* toggleAction (const char* name) const
    { return (KToggleAction*) action (name); }
  /** Retrieves a slider action from the actionCollection by name.
    */
  KPlayerSliderAction* sliderAction (const char* name) const
    { return (KPlayerSliderAction*) action (name); }
  /** Retrieves a slider action from the actionCollection by name.
    */
  KPlayerPopupSliderAction* popupAction (const char* name) const
    { return (KPlayerPopupSliderAction*) action (name); }

  /** Sets the sound volume.
    */
  void setVolume (void);
  /** Sets the video contrast.
    */
  void setContrast (void);
  /** Sets the video brightness.
    */
  void setBrightness (void);
  /** Sets the video hue.
    */
  void setHue (void);
  /** Sets the video saturation.
    */
  void setSaturation (void);

  /** Initializes the engine.
   */
  static void initialize (KActionCollection* ac, QWidget* parent, const char* name = 0, KConfig* = 0);
  /** Terminates the engine.
   */
  static void terminate (void);

  /** Returns the static engine object.
   */
  static KPlayerEngine* kPlayerEngine (void)
    { return m_engine; }

  /** Returns the settings object.
   */
  KPlayerSettings* kPlayerSettings (void) const
    { return m_settings; }

  /** Returns the widget object.
   */
  KPlayerWidget* kPlayerWidget (void) const
    { return m_widget; }

  /** Returns the widget object.
   */
  KPlayerWorkspace* kPlayerWorkspace (void) const
    { return m_workspace; }

  /** Returns the process object.
   */
  KPlayerProcess* kPlayerProcess (void) const
    { return m_process; }

  /** Returns the config object.
   */
  KConfig* kPlayerConfig (void) const
    { return m_config; }

  /** Returns the playlist config object.
   */
  KConfig* kPlayerPlaylistConfig (void) const
    { return m_playlist_config; }

  KActionCollection* actionCollection (void) const
   { return m_ac; }
  void setActionCollection (KActionCollection* collection)
   { m_ac = collection; }

  void setDisplaySize (bool zoom = false, bool resize = false);

  void maintainAspect (bool maintain, QSize aspect);
  void refreshAspect (void);

  /** Resets full screen and maximized states.
   */
  void normal (void);
  /** Zoom to the given factor.
   */
  void zoomTo (int m, int d = 1);
  /** Zoom or seek on mouse wheel event.
   */
  void wheel (int delta, int state);
  /** Toggle full screen on double click. */
  void doubleClick (void);

  /** Disables screen saver if it is enabled.
   */
  void disableScreenSaver (void);
  /** Enables screen saver if it was disabled.
   */
  void enableScreenSaver (void);

  int audioCodecCount (void) const
    { return m_audio_codecs.count(); }
  QString audioCodecName (int index) const
    { return m_audio_codecs[index]; }
  int audioCodecIndex (QString codec) const
    { return listIndex (m_audio_codecs, codec); }

  int audioDriverCount (void) const
    { return m_audio_drivers.count(); }
  QString audioDriverName (int index) const
    { return m_audio_drivers[index]; }
  int audioDriverIndex (QString driver) const
    { return listIndex (m_audio_drivers, driver); }

  int videoCodecCount (void) const
    { return m_video_codecs.count(); }
  QString videoCodecName (int index) const
    { return m_video_codecs[index]; }
  int videoCodecIndex (QString codec) const
    { return listIndex (m_video_codecs, codec); }

  int videoDriverCount (void) const
    { return m_video_drivers.count(); }
  QString videoDriverName (int index) const
    { return m_video_drivers[index]; }
  int videoDriverIndex (QString driver) const
    { return listIndex (m_video_drivers, driver); }

public slots:
  /** Handles workspace resized event.
    */
  void workspaceResized (void);
  /** Receives the refresh signal from KPlayerSettings. Updates the settings.
   */
  void refreshSettings (void);
  /** Receives the refresh signal from KPlayerProperties. Updates the settings.
   */
  void refreshProperties (void);

  /** Receives the stateChanged signal from KPlayerProcess.
    */
  void playerStateChanged (KPlayerProcess::State, KPlayerProcess::State);
  /** Receives the progressChanged signal from KPlayerProcess.
    */
  void playerProgressChanged (float, KPlayerProcess::ProgressType);
  /** Receives the infoAvailable signal from KPlayerProcess.
    */
  void playerInfoAvailable (void);
  /** Receives the sizeAvailable signal from KPlayerProcess.
    */
  void playerSizeAvailable (void);

  /** Displays the Open Subtitle File dialog and loads the chosen subtitles.
      Restarts playback if a file was playing.
    */
  void fileOpenSubtitles (void);
  /** Displays the Open Subtitle URL dialog and loads the chosen subtitles.
      Restarts playback if a file was playing.
    */
  void fileOpenSubtitleUrl (void);
  /** Unloads subtitles and prevents subtitle autoloading.
      Restarts playback if a file was playing.
    */
  void fileUnloadSubtitles (void);
  /** Opens File Properties dialog.
    */
  void fileProperties (void);

  /** Toggles full screen mode.
    */
  void fullScreen (void);
  /** Scales video up.
    */
  void zoomIn (void);
  /** Scales video down.
    */
  void zoomOut (void);
  /** Scales video to 50%.
    */
  void zoom12 (void);
  /** Scales video to 100%.
    */
  void zoom11 (void);
  /** Scales video to 150%.
    */
  void zoom32 (void);
  /** Scales video to 200%.
    */
  void zoom21 (void);
  /** Scales video to 250%.
    */
  void zoom52 (void);
  /** Scales video to 300%.
    */
  void zoom31 (void);
  /** Toggles the option of maintaining the video aspect ratio.
    */
  void maintainAspect (void);
  /** Toggles the option of maintaining the original video aspect ratio.
    */
  void maintainOriginalAspect (void);
  /** Toggles the option of maintaining the current video aspect ratio.
    */
  void maintainCurrentAspect (void);
  /** Forces 4 to 3 video aspect ratio.
    */
  void aspect43 (void);
  /** Forces 16 to 9 video aspect ratio.
    */
  void aspect169 (void);

  /** Plays the currently loaded file.
    */
  void play (void);
  /** Pauses the player.
    */
  void pause (void);
  /** Stops the player.
    */
  void stop (void);
  /** Stops the player and the helper.
    */
  void kill (void);
  /** Seeks forward.
    */
  void forward (void);
  /** Seeks forward fast.
    */
  void fastForward (void);
  /** Seeks backward.
    */
  void backward (void);
  /** Seeks backward fast.
    */
  void fastBackward (void);
  /** Seeks to the beginning.
    */
  void start (void);

  /** Increases the volume.
    */
  void volumeIncrease (void);
  /** Decreases the volume.
    */
  void volumeDecrease (void);
  /** Toggles audio playback.
    */
  void mute (void);
  /** Increases the audio delay.
    */
  void audioDelayIncrease (void);
  /** Decreases the audio delay.
    */
  void audioDelayDecrease (void);

  /** Toggle the soft frame dropping option.
    */
  void softFrameDrop (void);
  /** Toggle the hard frame dropping option.
    */
  void hardFrameDrop (void);

  /** Shows or hides subtitles.
    */
  void subtitlesShow (void);
  /** Moves subtitles down.
    */
  void subtitlesMoveDown (void);
  /** Moves subtitles up.
    */
  void subtitlesMoveUp (void);
  /** Decreases subtitle delay.
    */
  void subtitlesDelayDecrease (void);
  /** Increases subtitle delay.
    */
  void subtitlesDelayIncrease (void);

  /** Increases the video brightness.
    */
  void brightnessIncrease (void);
  /** Decreases the video brightness.
    */
  void brightnessDecrease (void);
  /** Increases the video contrast.
    */
  void contrastIncrease (void);
  /** Decreases the video contrast.
    */
  void contrastDecrease (void);
  /** Increases the video hue.
    */
  void hueIncrease (void);
  /** Decreases the video hue.
    */
  void hueDecrease (void);
  /** Increases the video saturation.
    */
  void saturationIncrease (void);
  /** Decreases the video saturation.
    */
  void saturationDecrease (void);

  /** Moves to the position chosen.
    */
  void progressChanged (int);
  /** Changes the volume level.
    */
  void volumeChanged (int);
  /** Changes the video brightness.
    */
  void brightnessChanged (int);
  /** Changes the video contrast.
    */
  void contrastChanged (int);
  /** Changes the video hue.
    */
  void hueChanged (int);
  /** Changes the video saturation.
    */
  void saturationChanged (int);

  KPlayerProperties* reference (KURL);
  void dereference (KPlayerProperties*);

protected:
  QMap<QString, KPlayerProperties*> m_map;
  QString m_path;

  KPlayerLineOutputProcess* m_player_ac;
  KPlayerLineOutputProcess* m_player_ao;
  KPlayerLineOutputProcess* m_player_vc;
  KPlayerLineOutputProcess* m_player_vo;

  bool m_audio_codecs_ready;
  bool m_audio_drivers_ready;
  bool m_video_codecs_ready;
  bool m_video_drivers_ready;

  QStringList m_audio_codecs;
  QStringList m_audio_drivers;
  QStringList m_video_codecs;
  QStringList m_video_drivers;

  /** The static engine object.
   */
  static KPlayerEngine* m_engine;
  /** The settings object.
   */
  KPlayerSettings* m_settings;
  /** The config object.
   */
  KConfig* m_config;
  /** The playlist config object.
   */
  KConfig* m_playlist_config;
  /** The process object.
   */
  KPlayerProcess* m_process;
  /** The widget object.
   */
  KPlayerWidget* m_widget;
  /** The workspace object.
   */
  KPlayerWorkspace* m_workspace;
  /** The action collection object.
   */
  KActionCollection* m_ac;

  // Following should be private
  /** Indicates what actions should be used.
   */
  bool m_light;
  /** Indicates whether screen saver needs to be reenabled.
   */
  bool m_enable_screen_saver;
  /** Recursion prevention.
   */
  bool m_updating;
  /** Recursion prevention.
   */
  bool m_zooming;
  /** Work around QRangeControl bug.
   */
  int m_progress_factor;
  /** Do not play any more URLs.
   */
  bool m_stop;

signals:
  /** Emitted to let the main window syncronize full screen and maximized settings.
   */
  void syncronize (bool);
  /** Emitted when display size changes.
   */
  void zoom (void);
  /** Emitted when the workspace size needs adjustment.
   */
  void correctSize (void);
  /** Emitted when the original display size of a file becomes known.
   */
  void initialSize (void);
  /** Emitted when all drivers and codecs have been loaded.
   */
  void refresh (void);

protected slots:
  /** Processes an MPlayer audio codec line.
    */
  void receivedAudioCodec (KPlayerLineOutputProcess*, char*, int);
  /** Processes an MPlayer audio driver line.
    */
  void receivedAudioDriver (KPlayerLineOutputProcess*, char*, int);
  /** Processes an MPlayer video codec line.
    */
  void receivedVideoCodec (KPlayerLineOutputProcess*, char*, int);
  /** Processes an MPlayer video driver line.
    */
  void receivedVideoDriver (KPlayerLineOutputProcess*, char*, int);
  /** Finishes refreshing audio codecs.
    */
  void audioCodecProcessExited (KProcess*);
  /** Finishes refreshing audio drivers.
    */
  void audioDriverProcessExited (KProcess*);
  /** Finishes refreshing video codecs.
    */
  void videoCodecProcessExited (KProcess*);
  /** Finishes refreshing video drivers.
    */
  void videoDriverProcessExited (KProcess*);
};

inline KPlayerEngine* kPlayerEngine (void)
{
  return KPlayerEngine::kPlayerEngine();
}

inline KPlayerProcess* kPlayerProcess (void)
{
  return kPlayerEngine() -> kPlayerProcess();
}

inline KConfig* kPlayerConfig (void)
{
  return kPlayerEngine() -> kPlayerConfig();
}

inline KConfig* kPlayerPlaylistConfig (void)
{
  return kPlayerEngine() -> kPlayerPlaylistConfig();
}

inline KPlayerSettings* kPlayerSettings (void)
{
  return kPlayerEngine() -> kPlayerSettings();
}

inline KPlayerWidget* kPlayerWidget (void)
{
  return kPlayerEngine() -> kPlayerWidget();
}

inline KPlayerWorkspace* kPlayerWorkspace (void)
{
  return kPlayerEngine() -> kPlayerWorkspace();
}

#endif
