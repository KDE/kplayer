/***************************************************************************
                          kplayer.h
                          ---------
    begin                : Sat Nov 16 10:12:50 EST 2002
    copyright            : (C) 2002-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYER_H
#define KPLAYER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <KAction>
#include <KXmlGuiWindow>
#include <KUniqueApplication>

#include "kplayerengine.h"
#include "kplayersettings.h"

class KPlayerActionList;
class KPlayerLibraryWindow;
class KPlayerLogWindow;
class KPlayerPlaylist;
class KToggleAction;
class QLabel;
class QTabBar;

#define MAIN_TOOLBAR         0
#define PLAYLIST_TOOLBAR     1
#define FIRST_SLIDER_TOOLBAR 2
#define PROGRESS_TOOLBAR     2
#define VOLUME_TOOLBAR       3
#define FIRST_VIDEO_TOOLBAR  4
#define CONTRAST_TOOLBAR     4
#define BRIGHTNESS_TOOLBAR   5
#define HUE_TOOLBAR          6
#define SATURATION_TOOLBAR   7
#define KPLAYER_TOOLBARS     8

/** The KPlayer main window class.
  *@author kiriuja
  */
class KPlayerWindow : public KXmlGuiWindow
{
  Q_OBJECT 

  typedef struct _Toolbar
  {
    const char* name;
    const char* action;
    bool normally_visible;
    bool fullscreen_visible;
    bool auto_break;
  } Toolbar;

public:
  /** Constructor. Parameters are passed to KMainWindow constructor. */
  KPlayerWindow (QWidget* parent = 0);
  /** Destructor. */
  virtual ~KPlayerWindow();

  /** Engine. */
  KPlayerEngine* engine (void) const
    { return KPlayerEngine::engine(); }
  /** Configuration. */
  KPlayerConfiguration* configuration (void) const
    { return engine() -> configuration(); }
  /** Settings. */
  KPlayerSettings* settings (void) const
    { return engine() -> settings(); }
  /** Process. */
  KPlayerProcess* process (void) const
    { return engine() -> process(); }

  /** Main config. */
  KConfig* config (void) const
    { return KPlayerEngine::engine() -> config(); }

  /** Playlist. */
  KPlayerPlaylist* playlist (void) const
    { return m_playlist; }
  /** Library. */
  KPlayerLibraryWindow* library (void) const
    { return m_library; }
  /** Log. */
  KPlayerLogWindow* log (void) const
    { return m_log; }

  /** Starts playing a new file. */
  void start (void);
  /** Logs a message if logging of KPlayer messages is enabled. */
  void log (QString);

  /** Returns whether the menu bar should be currently visible. */
  bool showMenubar (void) const
    { return settings() -> fullScreen() ? m_menubar_fullscreen_visible : m_menubar_normally_visible; }
  /** Sets whether the menu bar should be currently visible. */
  void showMenubar (bool show)
    { (settings() -> fullScreen() ? m_menubar_fullscreen_visible : m_menubar_normally_visible) = show; }

  /** Returns whether the status bar should be currently visible. */
  bool showStatusbar (void) const
    { return settings() -> fullScreen() ? m_statusbar_fullscreen_visible : m_statusbar_normally_visible; }
  /** Sets whether the status bar should be currently visible. */
  void showStatusbar (bool show)
    { (settings() -> fullScreen() ? m_statusbar_fullscreen_visible : m_statusbar_normally_visible) = show; }

  /** Returns whether the given toolbar should be currently visible. */
  bool showToolbar (int index) const
    { return toggleAction (m_toolbar[index].action) -> isEnabled()
      && (settings() -> fullScreen() ? m_toolbar[index].fullscreen_visible : m_toolbar[index].normally_visible); }
  /** Sets whether the given toolbar should be currently visible. */
  void showToolbar (int index, bool show)
    { (settings() -> fullScreen() ? m_toolbar[index].fullscreen_visible : m_toolbar[index].normally_visible) = show; }

  /** Returns whether the library should be currently visible. */
  bool showLibrary (void) const
    { return settings() -> fullScreen() ? m_library_fullscreen_visible : m_library_normally_visible; }
  /** Sets whether the library should be currently visible. */
  void showLibrary (bool show)
    { (settings() -> fullScreen() ? m_library_fullscreen_visible : m_library_normally_visible) = show; }

  /** Returns whether the message log should be currently visible. */
  bool showMessageLog (void) const
    { return settings() -> fullScreen() ? m_messagelog_fullscreen_visible : m_messagelog_normally_visible; }
  /** Sets whether the message log should be currently visible. */
  void showMessageLog (bool show)
    { (settings() -> fullScreen() ? m_messagelog_fullscreen_visible : m_messagelog_normally_visible) = show; }

  /** Calculates the size hint for the main window. */
  virtual QSize sizeHint (void) const;

  /** Returns the full screen state. */
  bool fullScreen (void) const;
  /** Returns the maximized state. */
  bool maximized (void) const;
  /** Returns the minimized state. */
  bool minimized (void) const;
  /** Returns the shaded state. */
  bool shaded (void) const;

  /** Returns whether the message log should be currently visible. */
  bool logVisible (void) const
    { return settings() -> fullScreen() ? m_messagelog_fullscreen_visible : m_messagelog_normally_visible; }

protected:
  /** Returns the total available geometry. */
  QRect availableGeometry (void) const;

  /** Sets up actions and connects signals to slots. */
  void initActions (void);
  /** Sets up the status bar. */
  void initStatusBar (void);

  /** Finds the tab bar where the widget is docked. */
  QTabBar* findTabBar (QDockWidget* widget, int* index = 0);

  /** Save general options like all bar positions and status as well as the geometry
      and the recent file list to the configuration file. */
  void saveOptions (void);
  /** Read general options again and initialize all variables like the recent file list. */
  void readOptions (void);

  /** Enables or disables playlist actions. */
  void enablePlaylistActions (void);
  /** Enables or disables player actions. */
  void enablePlayerActions (void);
  /** Enables or disables video actions. */
  void enableVideoActions (void);
  /** Enables or disables subtitle actions. */
  void enableSubtitleActions (void);
  /** Enables or disables, checks or unchecks zoom actions. */
  void enableZoomActions (void);

  /** Enables a toolbar.
   * @param index the toolbar number
   */
  void enableToolbar (int index);
  /** Disables a toolbar.
   * @param index the toolbar number
   */
  void disableToolbar (int index);

  /** Changes the statusbar contents for the standard label permanently, used to indicate current actions. */
  void setStatusText (const QString&);

  /** Sets the window geometry on the initial show event. */
  virtual void showEvent (QShowEvent*);
  virtual void changeEvent (QEvent*);
  virtual void focusInEvent (QFocusEvent*);
  virtual void focusOutEvent (QFocusEvent*);
  /** Handles main window movement. */
  virtual void moveEvent (QMoveEvent*);
  /** Handles main window resizing. */
  virtual void resizeEvent (QResizeEvent*);
  /** Displays the right click popup menu. */
  virtual void contextMenuEvent (QContextMenuEvent*);
  /** Handles the drag enter event. */
  virtual void dragEnterEvent (QDragEnterEvent*);
  /** Handles the drop event. */
  virtual void dropEvent (QDropEvent*);
  /** Saves options and stops KPlayer engine. */
  virtual void closeEvent (QCloseEvent*);

  /** Retrieves an action from the actionCollection by name. */
  QAction* action (const char* name) const
    { return actionCollection() -> action (name); }
  /** Retrieves a toggle action from the actionCollection by name. */
  KToggleAction* toggleAction (const char* name) const
    { return (KToggleAction*) action (name); }

  /** The log window object. */
  KPlayerLogWindow* m_log;
  /** The playlist object. */
  KPlayerPlaylist* m_playlist;
  /** The library window. */
  KPlayerLibraryWindow* m_library;

  // Following should be private.
  bool m_menubar_normally_visible, m_menubar_fullscreen_visible;
  bool m_statusbar_normally_visible, m_statusbar_fullscreen_visible;
  bool m_messagelog_normally_visible, m_messagelog_fullscreen_visible;
  bool m_library_normally_visible, m_library_fullscreen_visible;
  //bool m_toggling_full_screen, m_active_window;
  bool m_initial_show, m_error_detected;
  bool m_handle_layout, m_show_library, m_show_log;
  Toolbar m_toolbar [KPLAYER_TOOLBARS];
  QSize m_previous_size;
  //QRect m_normal_geometry;
  QLabel *m_status_label, *m_state_label, *m_progress_label;

public slots:
  /** Clears all messages from the message log. */
  void fileClearLog (void);
  /** Ends the program by closing the main window. */
  void fileQuit (void);

  /** Shows or hides the playlist. */
  void viewPlaylist (void);
  /** Shows or hides the multimedia library. */
  void viewLibrary (void);
  /** Shows or hides the message log. */
  void viewMessageLog (void);

  /** Toggles the menu bar. */
  void viewMenubar (void);
  /** Toggles the status bar. */
  void viewStatusbar (void);

  /** Toggles the main toolbar. */
  void viewMainToolbar (void);
  /** Toggles the progress toolbar. */
  void viewProgressToolbar (void);
  /** Toggles the volume toolbar. */
  void viewVolumeToolbar (void);
  /** Toggles the brightness toolbar. */
  void viewBrightnessToolbar (void);
  /** Toggles the contrast toolbar. */
  void viewContrastToolbar (void);
  /** Toggles the hue toolbar. */
  void viewHueToolbar (void);
  /** Toggles the Saturation toolbar. */
  void viewSaturationToolbar (void);
  /** Opens the the standard key binding dialog. */
  void settingsKeyBindings (void);
  /** Opens the the standard toolbar configuration dialog. */
  void settingsConfigureToolbars (void);
  /** Applies changes made in the toolbar configuration dialog. */
  void newToolbarConfiguration (void);
  /** Opens the KPlayer configuration dialog. */
  void settingsConfigure (void);

  /** Changes the statusbar test temporarily, used to display a hint on the currently highlighted action. */
  //void setStatusMessage (const QString&);
  /** Clears the temporary statusbar text when the action is no longer highlighted. */
  //void clearStatusMessage (void);

protected slots:
  /** Resets the zooming flag when the main window state changes. */
  void windowStateChanged (uint wid);
  /** Synchronizes full screen and maximized settings. */
  void synchronizeState (bool* pending);
  /** Synchronizes controls. */
  void synchronizeControls (void);
  /** Updates the layout of the window controls. */
  void updateLayout (const QSize&);
  /** Resizes the window to the correct size. */
  void zoom (void);
  /** Finalizes the window layout. */
  void finalizeLayout (void);
  /** Sets the correct display size. */
  void correctSize (void);
  /** Sets the correct display size. */
  void dockWidgetVisibility (void);

  /** Enables or disables submenus with the given name. */
  void enableSubmenu (const QString& name, bool enable);

  /** Shows the message log if there have been errors. */
  void showErrors (const QString&);

  /** Resets the player status. */
  void playlistStarted (void);
  /** Updates the caption and progress information. */
  void playlistActivated (void);
  /** Clears the error condition if any. */
  void playlistStopped (void);

  /** Receives the stateChanged signal from KPlayerProcess. */
  void playerStateChanged (KPlayerProcess::State, KPlayerProcess::State);
  /** Receives the progressChanged signal from KPlayerProcess. */
  void playerProgressChanged (float, KPlayerProcess::ProgressType);
  /** Receives the infoAvailable signal from KPlayerProcess. */
  void playerInfoAvailable (void);
  /** Receives the sizeAvailable signal from KPlayerProcess. */
  void playerSizeAvailable (void);
  /** Receives the messageReceived signal from KPlayerProcess. */
  void playerMessageReceived (QString);
  /** Receives the errorDetected signal from KPlayerProcess. */
  void playerErrorDetected (void);
  /** Receives the updated signal from KPlayerSettings. Updates the slider settings. */
  void refreshSettings (void);
  /** Receives the updated signal from KPlayerProperties. Updates the window caption. */
  void refreshProperties (void);

  /** Shows the library window. */
  void makeLibraryVisible (void);
  /** Unplugs the action list. */
  void actionListUpdating (KPlayerActionList* list);
  /** Plugs the action list and enables the menu. */
  void actionListUpdated (KPlayerActionList* list);
  /** Plugs the action list and enables the menu if the library is visible. */
  void libraryActionListUpdated (KPlayerActionList* list);

  /** Displays the right click popup menu at the given position. */
  void contextMenu (const QPoint& global_position);
};

/** The KPlayer application class.
  *@author kiriuja
  */
class KPlayerApplication : public KUniqueApplication
{
  Q_OBJECT 

public:
  /** Constructs the KPlayer application. */
  KPlayerApplication (void);
  /** KPlayer application destructor. */
  virtual ~KPlayerApplication();

  /** Creates the main window on the first invocation. Calls KPlayer::start(). */
  virtual int newInstance (void);

  /** Remembers the shift key state. */
  virtual bool notify (QObject*, QEvent*);

  /** Process application X11 events. */
  virtual bool x11EventFilter (XEvent* event);
};

#endif
