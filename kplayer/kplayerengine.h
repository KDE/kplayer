/***************************************************************************
                          kplayerengine.h
                          ---------------
    begin                : Tue Feb 10 2004
    copyright            : (C) 2004-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERENGINE_H
#define KPLAYERENGINE_H

#include <kaction.h>
#include <kdiroperator.h>
#include <kfiledialog.h>

#include "kplayerprocess.h"

class KPlayerConfiguration;
class KPlayerPopupSliderAction;
class KPlayerSliderAction;
class KPlayerSettings;
class KPlayerSubtitleTrackActionList;
class KPlayerTrackActionList;
class KPlayerTrackProperties;
class KPlayerWidget;
class KPlayerWorkspace;

int listIndex (const QStringList&, const QString&);

/** The KPlayer file dialog.
  *@author kiriuja
  */
class KPlayerFileDialog : public KFileDialog
{
   Q_OBJECT

public: 
  /** Constructor. Parameters are passed to the KFileDialog constructor. */
  KPlayerFileDialog (const QString& dir, const QString& filter, QWidget* parent, const char* name);

  /** Returns the last used directory. */
  QString directory (void)
    { return ops -> url().path (-1); }
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

  /** Returns whether the engine is running in the KPart mode. */
  bool light (void)
    { return m_light; }

  /** Returns whether playback is being stopped at user request. */
  bool stopped (void)
    { return m_stop; }

  /** Stops the player and the helper. */
  void kill (void);

  /** Starts loading of all mplayer drivers and codecs.
   * The updated signal is emitted when loading finishes. */
  void getLists (QString = QString::null);

  /** Starts amixer process to get the current ALSA volume. */
  void getAlsaVolume (void);

  /** Creates actions and connects signals to slots. */
  void setupActions (void);
  /** Enables or disables player actions. */
  void enablePlayerActions (void);
  /** Enables or disables video actions. */
  void enableVideoActions (void);
  /** Enables or disables subtitle actions. */
  void enableSubtitleActions (void);
  /** Enables or disables, checks or unchecks zoom actions. */
  void enableZoomActions (void);

  /** Loads the given URL and optionally starts playback. */
  void load (KURL);
  /** Autoloads subtitles as specified in the settings. */
  void autoloadSubtitles (void);
  /** Loads the given subtitle URL and restarts playback if needed. */
  void loadSubtitle (KURL);

  /** Displays the open file dialog and lets the user choose a file or files.
      Returns the list of chosen URLs or an empty list if the user did not choose any. */
  KURL::List openFiles (const QString& title, QWidget* = 0);
  /** Displays the URL requester dialog and lets the user enter a URL.
      Returns the entered URL or an empty URL if the user did not enter any. */
  KURL::List openUrl (const QString& title, QWidget* = 0);
  /** Displays the open file dialog and lets the user choose a subtitle file.
      Returns the chosen URL or an empty URL if the user did not choose any. */
  KURL openSubtitle (QWidget* = 0);
  /** Displays the URL requester dialog and lets the user enter a subtitle URL.
      Returns the entered URL or an empty URL if the user did not enter any. */
  //KURL openSubtitleUrl (QWidget* = 0);

  /** Retrieves an action from the actionCollection by name. */
  KAction* action (const char* name) const
    { return m_ac -> action (name); }
  /** Retrieves a toggle action from the actionCollection by name. */
  KToggleAction* toggleAction (const char* name) const
    { return (KToggleAction*) action (name); }
  /** Retrieves a slider action from the actionCollection by name. */
  KPlayerSliderAction* sliderAction (const char* name) const
    { return (KPlayerSliderAction*) action (name); }
  /** Retrieves a slider action from the actionCollection by name. */
  KPlayerPopupSliderAction* popupAction (const char* name) const
    { return (KPlayerPopupSliderAction*) action (name); }

  /** Returns the video action list. */
  KPlayerTrackActionList* videoActionList (void) const
    { return m_video_action_list; }
  /** Returns the audio action list. */
  KPlayerTrackActionList* audioActionList (void) const
    { return m_audio_action_list; }
  /** Returns the subtitle action list. */
  KPlayerSubtitleTrackActionList* subtitleActionList (void) const
    { return m_subtitle_action_list; }

  /** Sets the sound volume. */
  void setVolume (void);
  /** Sets the video contrast. */
  void setContrast (void);
  /** Sets the video brightness. */
  void setBrightness (void);
  /** Sets the video hue. */
  void setHue (void);
  /** Sets the video saturation. */
  void setSaturation (void);

  /** Clear the given store section and all subsections. */
  void clearStoreSections (const QString& section);

  /** Initializes the engine. */
  static void initialize (KActionCollection* ac, QWidget* parent, const char* name = 0, KConfig* = 0);
  /** Terminates the engine. */
  static void terminate (void);

  /** Returns the static engine object. */
  static KPlayerEngine* engine (void)
    { return m_engine; }

  /** Returns the widget object. */
  KPlayerWidget* widget (void) const
    { return m_widget; }

  /** Returns the workspace object. */
  KPlayerWorkspace* workspace (void) const
    { return m_workspace; }

  /** Returns the process object. */
  KPlayerProcess* process (void) const
    { return m_process; }

  /** Returns the settings object. */
  KPlayerSettings* settings (void) const
    { return m_settings; }

  /** Returns the properties object. */
  KPlayerTrackProperties* properties (void) const;

  /** Returns the configuration object. */
  KPlayerConfiguration* configuration (void)
    { return m_configuration; }

  /** Returns the config object. */
  KConfig* config (void)
    { return m_config; }
  /** Returns the store object. */
  KConfig* store (void)
    { return m_store; }
  /** Returns the meta information object. */
  KConfig* meta (void)
    { return m_meta; }
  /** Returns the meta information storage set to the given group. */
  KConfig* meta (const QString& group)
  {
    meta() -> setGroup (group);
    return meta();
  }
  /** Returns the value of the given key with the given default. */
  QString meta (const QString& group, const QString& key, const QString& value = QString::null)
    { return meta (group) -> readEntry (key, value); }

  KActionCollection* actionCollection (void) const
   { return m_ac; }
  void setActionCollection (KActionCollection* collection)
   { m_ac = collection; }

  void setDisplaySize (bool zoom = false, bool resize = false);

  void maintainAspect (bool maintain, QSize aspect);
  void refreshAspect (void);

  /** Resets full screen and maximized states. */
  void normal (void);
  /** Zoom to the given factor. */
  void zoomTo (int m, int d = 1);
  /** Zoom or seek on mouse wheel event. */
  void wheel (int delta, int state);
  /** Toggle full screen on double click. */
  void doubleClick (void);

  /** Emits windowStateChanged signal. */
  void emitWindowStateChanged (uint wid);

  /** Disables screen saver if it is enabled. */
  void disableScreenSaver (void);
  /** Enables screen saver if it was disabled. */
  void enableScreenSaver (void);

  int demuxerCount (void) const
    { return m_demuxers.count(); }
  const QString& demuxerName (int index) const
    { return m_demuxers [index]; }
  int demuxerIndex (const QString& codec) const
    { return listIndex (m_demuxers, codec); }

  int audioCodecCount (void) const
    { return m_audio_codecs.count(); }
  const QString& audioCodecName (int index) const
    { return m_audio_codecs[index]; }
  int audioCodecIndex (const QString& codec) const
    { return listIndex (m_audio_codecs, codec); }

  int audioDriverCount (void) const
    { return m_audio_drivers.count(); }
  const QString& audioDriverName (int index) const
    { return m_audio_drivers[index]; }
  int audioDriverIndex (const QString& driver) const
    { return listIndex (m_audio_drivers, driver); }

  int videoCodecCount (void) const
    { return m_video_codecs.count(); }
  const QString& videoCodecName (int index) const
    { return m_video_codecs[index]; }
  int videoCodecIndex (const QString& codec) const
    { return listIndex (m_video_codecs, codec); }

  int videoDriverCount (void) const
    { return m_video_drivers.count(); }
  const QString& videoDriverName (int index) const
    { return m_video_drivers[index]; }
  int videoDriverIndex (const QString& driver) const
    { return listIndex (m_video_drivers, driver); }

public slots:
  /** Handles workspace resized event. */
  void workspaceResized (void);
  /** Handles workspace user resize event. */
  void workspaceUserResize (void);
  /** Receives the updated signal from KPlayerSettings. Updates the settings. */
  void refreshSettings (void);
  /** Receives the updated signal from KPlayerProperties. Updates the settings. */
  void refreshProperties (void);

  /** Receives the stateChanged signal from KPlayerProcess. */
  void playerStateChanged (KPlayerProcess::State, KPlayerProcess::State);
  /** Receives the progressChanged signal from KPlayerProcess. */
  void playerProgressChanged (float, KPlayerProcess::ProgressType);
  /** Receives the infoAvailable signal from KPlayerProcess. */
  void playerInfoAvailable (void);
  /** Receives the sizeAvailable signal from KPlayerProcess. */
  void playerSizeAvailable (void);

  /** Displays the Open Subtitle File dialog and loads the chosen subtitles.
      Restarts playback if a file was playing. */
  void fileOpenSubtitles (void);
  /** Displays the Open Subtitle URL dialog and loads the chosen subtitles.
      Restarts playback if a file was playing. */
  //void fileOpenSubtitleUrl (void);
  /** Unloads subtitles and prevents subtitle autoloading.
      Restarts playback if a file was playing. */
  //void fileUnloadSubtitles (void);
  /** Opens File Properties dialog. */
  void fileProperties (void);

  /** Toggles full screen mode. */
  void fullScreen (void);
  /** Scales video up. */
  void zoomIn (void);
  /** Scales video down. */
  void zoomOut (void);
  /** Scales video to 50%. */
  void zoom12 (void);
  /** Scales video to 100%. */
  void zoom11 (void);
  /** Scales video to 150%. */
  void zoom32 (void);
  /** Scales video to 200%. */
  void zoom21 (void);
  /** Scales video to 250%. */
  void zoom52 (void);
  /** Scales video to 300%. */
  void zoom31 (void);
  /** Toggles the option of maintaining the video aspect ratio. */
  void maintainAspect (void);
  /** Toggles the option of maintaining the original video aspect ratio. */
  void maintainOriginalAspect (void);
  /** Toggles the option of maintaining the current video aspect ratio. */
  void maintainCurrentAspect (void);
  /** Forces 4 to 3 video aspect ratio. */
  void aspect43 (void);
  /** Forces 16 to 9 video aspect ratio. */
  void aspect169 (void);

  /** Plays the currently loaded file. */
  void play (void);
  /** Pauses the player. */
  void pause (void);
  /** Stops the player. */
  void stop (void);

  /** Seeks forward. */
  void forward (void);
  /** Seeks forward fast. */
  void fastForward (void);
  /** Seeks backward. */
  void backward (void);
  /** Seeks backward fast. */
  void fastBackward (void);
  /** Seeks to the beginning. */
  void start (void);

  /** Increases the volume. */
  void volumeIncrease (void);
  /** Decreases the volume. */
  void volumeDecrease (void);
  /** Toggles audio playback. */
  void mute (void);
  /** Increases the audio delay. */
  void audioDelayIncrease (void);
  /** Decreases the audio delay. */
  void audioDelayDecrease (void);
  /** Sets the audio stream. */
  void audioStream (int index);

  /** Toggle the soft frame dropping option. */
  void softFrameDrop (void);
  /** Toggle the hard frame dropping option. */
  void hardFrameDrop (void);

  /** Moves subtitles down. */
  void subtitlesMoveDown (void);
  /** Moves subtitles up. */
  void subtitlesMoveUp (void);
  /** Decreases subtitle delay. */
  void subtitlesDelayDecrease (void);
  /** Increases subtitle delay. */
  void subtitlesDelayIncrease (void);
  /** Sets the subtitle stream. */
  void subtitleStream (int index);

  /** Increases the video brightness. */
  void brightnessIncrease (void);
  /** Decreases the video brightness. */
  void brightnessDecrease (void);
  /** Increases the video contrast. */
  void contrastIncrease (void);
  /** Decreases the video contrast. */
  void contrastDecrease (void);
  /** Increases the video hue. */
  void hueIncrease (void);
  /** Decreases the video hue. */
  void hueDecrease (void);
  /** Increases the video saturation. */
  void saturationIncrease (void);
  /** Decreases the video saturation. */
  void saturationDecrease (void);
  /** Sets the video stream. */
  void videoStream (int index);

  /** Moves to the position chosen. */
  void progressChanged (int);
  /** Changes the volume level. */
  void volumeChanged (int);
  /** Changes the video brightness. */
  void brightnessChanged (int);
  /** Changes the video contrast. */
  void contrastChanged (int);
  /** Changes the video hue. */
  void hueChanged (int);
  /** Changes the video saturation. */
  void saturationChanged (int);

protected:
  /** Handles workspace resize based on the user flag. */
  void workspaceResize (bool user);
  /** Runs amixer with the given command and parameter. */
  void runAmixer (const QString& command, const QString& parameter = QString::null);

  /** Sets up the progress slider. */
  void setupProgressSlider (int maximum);
  /** Tells the process to start playback. */
  void startPlaying (void);
  /** Shows the selected subtitles. */
  void showSubtitles (void);

  QString m_path;

  /** Video track action list. */
  KPlayerTrackActionList* m_video_action_list;
  /** Audio track action list. */
  KPlayerTrackActionList* m_audio_action_list;
  /** Subtitle track action list. */
  KPlayerSubtitleTrackActionList* m_subtitle_action_list;

  bool m_audio_codecs_ready;
  bool m_audio_drivers_ready;
  bool m_video_codecs_ready;
  bool m_video_drivers_ready;
  bool m_demuxers_ready;

  /** List of available audio codecs. */
  QStringList m_audio_codecs;
  /** List of available audio drivers. */
  QStringList m_audio_drivers;
  /** List of available video codecs. */
  QStringList m_video_codecs;
  /** List of available video drivers. */
  QStringList m_video_drivers;
  /** List of available demuxers. */
  QStringList m_demuxers;

  bool m_amixer_running, m_amixer_found_control;
  int m_last_volume, m_amixer_volume, m_amixer_volume_first, m_amixer_volume_second;

  /** The static engine object. */
  static KPlayerEngine* m_engine;
  /** Main config. */
  KConfig* m_config;
  /** Store config. */
  KConfig* m_store;
  /** Meta config. */
  KConfig* m_meta;
  /** Configuration properties. */
  KPlayerConfiguration* m_configuration;
  /** The settings object. */
  KPlayerSettings* m_settings;
  /** The process object. */
  KPlayerProcess* m_process;
  /** The widget object. */
  KPlayerWidget* m_widget;
  /** The workspace object. */
  KPlayerWorkspace* m_workspace;
  /** The action collection object. */
  KActionCollection* m_ac;

  // Following should be private
  /** Indicates what actions should be used. */
  bool m_light;
  /** Indicates whether screen saver needs to be reenabled. */
  bool m_enable_screen_saver;
  /** Recursion prevention. */
  bool m_updating;
  /** Recursion prevention. */
  bool m_zooming;
  /** Work around QRangeControl bug. */
  int m_progress_factor;
  /** Do not play any more URLs. */
  bool m_stop;
  /** Start playing when size is known. */
  bool m_play_pending;

signals:
  /** Emitted when a window state changes. */
  void windowStateChanged (uint wid);
  /** Emitted to let the main window syncronize full screen and maximized settings. */
  void syncronize (bool);
  /** Emitted when display size changes. */
  void zoom (void);
  /** Emitted when the workspace size needs adjustment. */
  void correctSize (void);
  /** Emitted when the original display size of a file becomes known. */
  void initialSize (void);
  /** Emitted when all drivers and codecs have been loaded. */
  void updated (void);

protected slots:
  /** Processes an MPlayer output line. */
  void receivedOutput (KPlayerLineOutputProcess*, char*, int);
  /** Finishes refreshing lists. */
  void processExited (KProcess*);

  /** Processes an amixer output line. */
  void amixerOutput (KPlayerLineOutputProcess*, char*, int);
  /** Finishes refreshing ALSA volume. */
  void amixerExited (KProcess*);
};

inline KPlayerEngine* kPlayerEngine (void)
{
  return KPlayerEngine::engine();
}

inline KPlayerProcess* kPlayerProcess (void)
{
  return kPlayerEngine() -> process();
}

inline KConfig* kPlayerConfig (void)
{
  return kPlayerEngine() -> config();
}

inline KPlayerSettings* kPlayerSettings (void)
{
  return kPlayerEngine() -> settings();
}

inline KPlayerWidget* kPlayerWidget (void)
{
  return kPlayerEngine() -> widget();
}

inline KPlayerWorkspace* kPlayerWorkspace (void)
{
  return kPlayerEngine() -> workspace();
}

#endif
