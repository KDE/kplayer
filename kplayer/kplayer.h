/***************************************************************************
                          kplayer.h
                          ---------
    begin                : Sat Nov 16 10:12:50 EST 2002
    copyright            : (C) 2002-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
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

#include <kaction.h>
#include <kxmlguiwindow.h>
#include <kuniqueapplication.h>

#include "kplayerengine.h"

class KPlayerActionList;
class KPlayerLibraryWindow;
class KPlayerLogWindow;
class KPlayerNodeView;
class KPlayerPlaylist;
class KPlayerWorkspace;
class KToggleAction;
class KUrl;
class QLabel;

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

  /** Shows or hides the menu bar according to the current settings and updates the user interface. */
  void showMenubar (void);
  /** Shows or hides the given toolbar according to the current settings and updates the user interface. */
  void showToolbar (int);
  /** Shows or hides the given toolbar and updates the user interface. */
  void showToolbar (int, bool);
  /** Shows or hides the status bar according to the current settings and updates the user interface. */
  void showStatusbar (void);

  /** Shows or hides the library according to the current settings and updates the user interface. */
  void showLibrary (void);
  /** Shows or hides the library and updates the user interface. */
  void showLibrary (bool);

  /** Shows or hides the message log according to the current settings and updates the user interface. */
  void showMessageLog (void);
  /** Shows or hides the message log and updates the user interface. */
  void showMessageLog (bool);

  /** Calculates the size hint for the main window. */
  //virtual QSize sizeHint (void) const;
  /** Calculates the minimum size hint for the main window. */
  //virtual QSize minimumSizeHint (void) const;
  /** Sets the minimum size of the main widget. */
  virtual void setMinimumSize (int, int);

  /** Returns the full screen state. */
  bool isFullScreen (void) const;
  /** Changes to full screen. */
  void toFullScreen (void);
  /** Changes to normal screen size. */
  void toNormalScreen (void);

  /** Replacement for broken QWidget::isMaximized. */
  bool isMaximized (void) const;
  /** Replacement for broken QWidget::showMaximized. */
  virtual void showMaximized (void);
  /** Calls base class implementation. */
  virtual void showNormal (void);

protected:
  /** Returns the total available geometry. */
  //QRect availableGeometry (void) const;
  /** Resizes the main window in response to a zoom request. */
  //void do_zoom (void);
  /** Moves the main window if it does not fit the available screen space. */
  //void do_move (const QRect& frame);
  /** Sets up actions and connects signals to slots. */
  void initActions (void);
  /** Sets up the status bar. */
  void initStatusBar (void);

  /** Save general options like all bar positions and status as well as the geometry
      and the recent file list to the configuration file. */
  void saveOptions (void);
  /** Read general options again and initialize all variables like the recent file list. */
  void readOptions (void);
  /** Saves the current URLs during session end to the session config file.
      @see KMainWindow#saveProperties */
  //virtual void saveProperties (KConfig*) const;
  /** Restores the URLs from the session config file.
      @see KMainWindow#readProperties */
  //virtual void readProperties (KConfig*);

  /** Enables or disables player actions. */
  void enablePlayerActions (void);
  /** Enables or disables video actions. */
  void enableVideoActions (void);
  /** Enables or disables subtitle actions. */
  void enableSubtitleActions (void);
  /** Enables or disables, checks or unchecks zoom actions. */
  void enableZoomActions (void);
  /** Checks or unchecks bar actions. */
//void checkBarActions (void);

  /** Enables or disables submenus of the given menu with the given name. */
  //void enableSubmenu (QMenuData* data, const QString& name, bool enable);
  /** Returns the top level popup menu at the given position. */
  //QMenu* popupMenu (int index);

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

  /** Processes the widget events. Passes them on to KMainWindow. */
  //virtual bool event (QEvent*);
  /** Sets the window geometry on the initial show event. */
  virtual void showEvent (QShowEvent*);
  virtual void windowActivationChange (bool);
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

  /** Rearranges child windows and clears events. */
  void activateLayout (void);
  /** Syncronizes X Server and Qt events. */
  //void syncronizeEvents (void);

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
  bool m_set_display_size, m_initial_show, m_error_detected, m_maximized;
  bool m_full_screen, m_show_log, m_show_library;
  Toolbar m_toolbar [KPLAYER_TOOLBARS];
  QSize m_previous_size, m_video_size;
  QRect m_normal_geometry;
  QLabel *m_status_label, *m_state_label, *m_progress_label;

public slots:
  /** Resets the zooming flag when the main window state changes. */
  void windowStateChanged (uint wid);
  /** Syncronizes full screen and maximized settings. */
  void syncronize (bool);
  /** Zooms the video to the correct size. */
  //void zoom (void);
  /** Sets the correct display size. */
  void correctSize (void);
  /** Adjusts the initial display size. */
  void initialSize (void);

  /** Enables or disables submenus with the given name. */
  void enableSubmenu (const QString& name, bool enable);

  /** Shows the message log if there have been errors. */
  void statusPressed (void);

  /** Resets the player status. */
  void playlistStarted (void);
  /** Updates the caption and progress information. */
  void playlistActivated (void);
  /** Clears the error condition if any. */
  void playlistStopped (void);

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

  /** Opens the KPlayer manual section on bug reporting. */
  void helpReportBug (void);

  /** Changes the statusbar test temporarily, used to display a hint on the currently highlighted action. */
  //void setStatusMessage (const QString&);
  /** Clears the temporary statusbar text when the action is no longer highlighted. */
  //void clearStatusMessage (void);

  /** Handles bar orientation change event. */
//void barOrientationChanged (Orientation);
  /** Handles bar place change event. */
//void barPlaceChanged (QDockWindow::Place);
  /** Handles bar visibility change event. */
//void barVisibilityChanged (bool);

protected slots:
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
  /** Receives the windowHidden signal from KPlayerLogWindow. Updates the menu item. */
  void logWindowHidden (void);
  /** Receives the windowHidden signal from KPlayerLibraryWindow. Updates the menu item. */
  void libraryHidden (void);
  /** Receives the updated signal from KPlayerSettings. Updates the slider settings. */
  void refreshSettings (void);
  /** Receives the updated signal from KPlayerProperties. Updates the window caption. */
  void refreshProperties (void);
  /** Sets the desired display size. */
  void setDisplaySize (void);

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

  /** Creates the main window on the first invokation. Calls KPlayer::start(). */
  virtual int newInstance (void);

  /** Remembers the shift key state. */
  virtual bool notify (QObject*, QEvent*);

  /** Process application X11 events. */
  virtual bool x11EventFilter (XEvent* event);

  //int x11ClientMessage (QWidget* widget, XEvent* event, bool passive_only);
};

#endif
