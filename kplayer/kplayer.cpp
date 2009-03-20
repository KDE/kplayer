/***************************************************************************
                          kplayer.cpp
                          -----------
    begin                : Sat Nov 16 10:12:50 EST 2002
    copyright            : (C) 2002-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kcursor.h>
#include <kedittoolbar.h>
#include <qaction.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kshortcutsdialog.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kurl.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>
#include <qdatetime.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qeventloop.h>
#include <qlabel.h>
#include <qobject.h>

#ifdef DEBUG
//#include <qtoolbutton.h>
#define DEBUG_KPLAYER_WINDOW
#define DEBUG_KPLAYER_RESIZING
//#define DEBUG_KPLAYER_EVENTS
//#define DEBUG_KPLAYER_NOTIFY_KEY
//#define DEBUG_KPLAYER_NOTIFY_MOUSE
//#define DEBUG_KPLAYER_NOTIFY_DRAG
#endif

#include "kplayer.h"
#include "kplayer.moc"
#include "kplayerlogwindow.h"
#include "kplayernodeaction.h"
#include "kplayernodeview.h"
#include "kplayerplaylist.h"
#include "kplayersettings.h"
#include "kplayersettingsdialog.h"
#include "kplayerslideraction.h"
#include "kplayerwidget.h"

void KPlayerProcessX11Event (XEvent* event);
void KPlayerX11GetKeyboardMouseState (uint id);

#define ID_STATUS_MSG   1
#define ID_STATE_MSG    2
#define ID_PROGRESS_MSG 3

QString resourcePath (const QString& filename);
extern KAboutData* about;

KPlayerApplication::KPlayerApplication (void)
  : KUniqueApplication (true, true)
{
  installEventFilter (this);
}

KPlayerApplication::~KPlayerApplication()
{
  removeEventFilter (this);
  delete about;
}

int KPlayerApplication::newInstance (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "New instance\n";
#endif
  static KPlayerWindow* mainWindow = 0;
  if ( ! mainWindow )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "Creating main window\n";
#endif
    mainWindow = new KPlayerWindow();
    /*if ( isRestored() && KMainWindow::canBeRestored (1) )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << "Restoring main window\n";
#endif
      mainWindow -> restore (1);
    }*/
    //setMainWidget (mainWindow);
  }
  mainWindow -> start();
  about -> setLicenseTextFile (resourcePath ("COPYING"));
  return 0;
}

bool KPlayerApplication::notify (QObject* object, QEvent* event)
{
#ifdef DEBUG_KPLAYER_EVENTS
  kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
    << " spontaneous " << event -> spontaneous() << " receiver "
    << (object ? object -> metaObject() -> className() : "<none>")
    << (object && ! object -> objectName().isEmpty() ? object -> objectName() : "<unnamed>") << "\n";
#endif
  static bool overridden = false;
  if ( kPlayerEngine() && kPlayerSettings() && event )
  {
    switch ( event -> type() )
    {
    case QEvent::ShortcutOverride:
      {
        QKeyEvent* kevent = (QKeyEvent*) event;
#ifdef DEBUG_KPLAYER_NOTIFY_KEY
        kdDebugTime() << "KPlayerApplication::notify: event type ShortcutOverride"
          << " spontaneous " << event -> spontaneous() << " receiver "
          << (object ? object -> metaObject() -> className() : "<none>")
          << " key " << kevent -> key() << " text " << kevent -> text()
          << " count " << kevent -> count() << " state " << kevent -> modifiers()
          << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
        bool result = KUniqueApplication::notify (object, event);
        overridden = kevent -> isAccepted();
        return result;
      }
#ifdef DEBUG_KPLAYER_NOTIFY_KEY
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      {
        QKeyEvent* kevent = (QKeyEvent*) event;
        /*if ( event -> spontaneous() )
        {
          kPlayerSettings() -> setControl (kevent -> modifiers() & Qt::ControlModifier);
          kPlayerSettings() -> setShift (kevent -> modifiers() & Qt::ShiftModifier);
        }*/
        kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
          << " spontaneous " << event -> spontaneous() << " receiver "
          << (object ? object -> metaObject() -> className() : "<none>")
          << " key " << kevent -> key() << " text " << kevent -> text()
          << " count " << kevent -> count() << " state " << kevent -> modifiers()
          << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
      }
      break;
#endif
    case QEvent::Shortcut:
      {
        QKeyEvent* kevent = (QKeyEvent*) event;
        //kPlayerSettings() -> setControl (kevent -> modifiers() & Qt::ControlModifier);
        //kPlayerSettings() -> setShift (kevent -> modifiers() & Qt::ShiftModifier);
#ifdef DEBUG_KPLAYER_NOTIFY_KEY
        kdDebugTime() << "KPlayerApplication::notify: event type Shortcut"
          << " spontaneous " << event -> spontaneous() << " receiver "
          << (object ? object -> metaObject() -> className() : "<none>")
          << " key " << kevent -> key() << " text " << kevent -> text()
          << " count " << kevent -> count() << " state " << kevent -> modifiers()
          << " accepted " << kevent -> isAccepted() << " overridden " << overridden << " autorepeat " << kevent -> isAutoRepeat()
          << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
        if ( ! overridden &&
          (kevent -> modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) == (Qt::ShiftModifier | Qt::ControlModifier)
          && (kevent -> key() >= Qt::Key_Exclam && kevent -> key() <= Qt::Key_Ampersand
          || kevent -> key() >= Qt::Key_ParenLeft && kevent -> key() <= Qt::Key_Plus
          || kevent -> key() == Qt::Key_Colon || kevent -> key() == Qt::Key_Less
          || kevent -> key() >= Qt::Key_Greater && kevent -> key() <= Qt::Key_At
          || kevent -> key() == Qt::Key_AsciiCircum || kevent -> key() == Qt::Key_Underscore
          || kevent -> key() >= Qt::Key_BraceLeft && kevent -> key() <= Qt::Key_AsciiTilde) )
        {
          int key;
          switch ( kevent -> key() )
          {
            case Qt::Key_Exclam:
              key = Qt::Key_1;
              break;
            case Qt::Key_QuoteDbl:
              key = Qt::Key_Apostrophe;
              break;
            case Qt::Key_NumberSign:
              key = Qt::Key_3;
              break;
            case Qt::Key_Dollar:
              key = Qt::Key_4;
              break;
            case Qt::Key_Percent:
              key = Qt::Key_5;
              break;
            case Qt::Key_Ampersand:
              key = Qt::Key_7;
              break;
            case Qt::Key_ParenLeft:
              key = Qt::Key_BracketLeft;
              break;
            case Qt::Key_ParenRight:
              key = Qt::Key_BracketRight;
              break;
            case Qt::Key_Asterisk:
              key = Qt::Key_8;
              break;
            case Qt::Key_Plus:
              key = Qt::Key_Equal;
              break;
            case Qt::Key_Colon:
              key = Qt::Key_Semicolon;
              break;
            case Qt::Key_Less:
              key = Qt::Key_Comma;
              break;
            case Qt::Key_Greater:
              key = Qt::Key_Period;
              break;
            case Qt::Key_Question:
              key = Qt::Key_Slash;
              break;
            case Qt::Key_At:
              key = Qt::Key_2;
              break;
            case Qt::Key_AsciiCircum:
              key = Qt::Key_6;
              break;
            case Qt::Key_Underscore:
              key = Qt::Key_Minus;
              break;
            case Qt::Key_BraceLeft:
              key = Qt::Key_BracketLeft;
              break;
            case Qt::Key_Bar:
              key = Qt::Key_Backslash;
              break;
            case Qt::Key_BraceRight:
              key = Qt::Key_BracketRight;
              break;
            case Qt::Key_AsciiTilde:
              key = Qt::Key_QuoteLeft;
              break;
            default:
              key = kevent -> key();
          }
          QKeyEvent keyevent (kevent -> type(), key, kevent -> modifiers() & ~ Qt::ShiftModifier,
            kevent -> text(), kevent -> isAutoRepeat(), kevent -> count());
          if ( kevent -> isAccepted() )
            keyevent.accept();
          else
            keyevent.ignore();
#ifdef DEBUG_KPLAYER_NOTIFY_KEY
          kdDebugTime() << "KPlayerApplication::notify: passing keyboard event " << keyevent.type()
            << " spontaneous " << keyevent.spontaneous() << " receiver "
            << (object ? object -> metaObject() -> className() : "<none>")
            << " key " << keyevent.key() << " text " << keyevent.text()
            << " count " << keyevent.count() << " state " << keyevent.modifiers()
            << " accepted " << keyevent.isAccepted() << " autorepeat " << keyevent.isAutoRepeat()
            << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
          return KUniqueApplication::notify (object, &keyevent);
        }
      }
      break;
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
      KPlayerEngine::engine() -> setModifiers (((QMouseEvent*) event) -> modifiers());
      KPlayerEngine::engine() -> setButtons (((QMouseEvent*) event) -> buttons());
#ifdef DEBUG_KPLAYER_NOTIFY_MOUSE
      kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
        << " spontaneous " << event -> spontaneous() << " receiver "
        << (object ? object -> metaObject() -> className() : "<none>")
        << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
      break;
    case QEvent::MouseMove:
      {
        QMouseEvent* mevent = ((QMouseEvent*) event);
        if ( ((mevent -> modifiers() & Qt::ControlModifier) == Qt::ControlModifier) != kPlayerSettings() -> anyControl()
          || ((mevent -> modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier) != kPlayerSettings() -> anyShift() )
        {
          Qt::KeyboardModifiers state = mevent -> modifiers() & ~ (Qt::ControlModifier | Qt::ShiftModifier);
          if ( kPlayerSettings() -> anyControl() )
            state |= Qt::ControlModifier;
          if ( kPlayerSettings() -> anyShift() )
            state |= Qt::ShiftModifier;
          QMouseEvent mouseevent (QEvent::MouseMove, mevent -> pos(), mevent -> button(), mevent -> buttons(), state);
          if ( mevent -> isAccepted() )
            mouseevent.accept();
          else
            mouseevent.ignore();
          return KUniqueApplication::notify (object, &mouseevent);
        }
        KPlayerEngine::engine() -> setModifiers (mevent -> modifiers());
        KPlayerEngine::engine() -> setButtons (mevent -> buttons());
#ifdef DEBUG_KPLAYER_NOTIFY_MOUSE
        kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
          << " spontaneous " << event -> spontaneous() << " receiver "
          << (object ? object -> metaObject() -> className() : "<none>")
          << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
      }
      break;
    case QEvent::Wheel:
      KPlayerEngine::engine() -> setModifiers (((QWheelEvent*) event) -> modifiers());
      KPlayerEngine::engine() -> setButtons (((QWheelEvent*) event) -> buttons());
#ifdef DEBUG_KPLAYER_NOTIFY_WHEEL
      kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
        << " spontaneous " << event -> spontaneous() << " receiver "
        << (object ? object -> metaObject() -> className() : "<none>")
        << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
      break;
#ifdef DEBUG_KPLAYER_NOTIFY_MENU
    case QEvent::ContextMenu:
      //kPlayerSettings() -> setControl (((QContextMenuEvent*) event) -> modifiers() & Qt::ControlModifier);
      //kPlayerSettings() -> setShift (((QContextMenuEvent*) event) -> modifiers() & Qt::ShiftModifier);
      kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
        << " spontaneous " << event -> spontaneous() << " receiver "
        << (object ? object -> metaObject() -> className() : "<none>")
        << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
      break;
#endif
#ifdef DEBUG_KPLAYER_NOTIFY_DRAG
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::Drop:
      {
        QDropEvent* devent = (QDropEvent*) event;
        kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
          << " spontaneous " << event -> spontaneous() << " receiver "
          << (object ? object -> metaObject() -> className() : "<none>")
          << " accepted " << devent -> isAccepted() << " action " << devent -> action()
          << " accepted " << devent -> isActionAccepted() << "\n";
      }
      break;
#endif
#ifdef DEBUG_KPLAYER_NOTIFY_DRAG
    case QEvent::DragResponse:
      kdDebugTime() << "KPlayerApplication::notify: event type DragResponse spontaneous " << event -> spontaneous()
        << " receiver " << (object ? object -> metaObject() -> className() : "<none>")
        << " accepted " << ((QDragResponseEvent*) event) -> dragAccepted() << "\n";
      break;
    case QEvent::DragLeave:
      kdDebugTime() << "KPlayerApplication::notify: event type DragLeave spontaneous " << event -> spontaneous()
        << " receiver " << (object ? object -> metaObject() -> className() : "<none>") << "\n";
      break;
#endif
#ifdef DEBUG_KPLAYER_NOTIFY_RESIZE
    case QEvent::Resize:
      if ( object == mainWidget() )
      {
        QResizeEvent* revent = (QResizeEvent*) event;
        kdDebugTime() << "KPlayerApplication::notify: Main window resize to " << revent -> size().width() << "x" << revent -> size().height() << "\n";
      }
#endif
    default:
      break;
    }
  }
  return KUniqueApplication::notify (object, event);
}

bool KPlayerApplication::x11EventFilter (XEvent* event)
{
  KPlayerProcessX11Event (event);
  return KUniqueApplication::x11EventFilter (event);
}

#ifdef DEBUG_KPLAYER_RESIZING
void dumpObject (const QObject* object, int depth = 20, int indent = 0)
{
  QByteArray spaces (indent * 2, ' ');
  if ( object -> inherits ("QWidget") )
  {
    QWidget* widget = (QWidget*) object;
    kdDebugTime() << spaces.data() << object -> metaObject() -> className() <<
      " " << object -> objectName().toLatin1().data() << " " << widget -> windowTitle().toLatin1().data() <<
      " " << widget -> winId() << " " << widget -> isVisible() << " " << ! widget -> isHidden() <<
      " " << widget -> minimumSize().width() << "x" << widget -> minimumSize().height() <<
      " " << widget -> minimumSizeHint().width() << "x" << widget -> minimumSizeHint().height() <<
      " " << widget -> sizeHint().width() << "x" << widget -> sizeHint().height() <<
      " " << widget -> geometry().x() << "x" << widget -> geometry().y() <<
      " " << widget -> geometry().width() << "x" << widget -> geometry().height() << "\n";
    if ( object -> inherits ("QToolBar") )
    {
      QToolBar* toolbar = (QToolBar*) object;
      QMainWindow* window = toolbar -> parent() -> inherits ("QMainWindow") ? (QMainWindow*) toolbar -> parent() : 0;
      kdDebugTime() << spaces.data() << "  area " << (window ? window -> toolBarArea (toolbar) : Qt::NoToolBarArea) <<
        " break " << (window ? window -> toolBarBreak (toolbar) : false) << " floating " << toolbar -> isFloating() <<
	" orientation " << toolbar -> orientation() << " style " << toolbar -> toolButtonStyle() << "\n";
    }
    if ( widget -> layout() && widget -> layout() -> parent() != widget )
      dumpObject (widget -> layout(), depth - 1, indent + 1);
  }
  else if ( object -> inherits ("QLayout") )
  {
    QLayout* layout = (QLayout*) object;
    kdDebugTime() << spaces.data() << object -> metaObject() -> className() <<
      " " << object -> objectName().toLatin1().data() <<
      " " << layout -> minimumSize().width() << "x" << layout -> minimumSize().height() <<
      " " << layout -> sizeHint().width() << "x" << layout -> sizeHint().height() <<
      " " << layout -> geometry().width() << "x" << layout -> geometry().height() <<
      " " << layout -> margin() << "+" << layout -> spacing() <<
      " " << layout -> alignment() << " " << layout -> expandingDirections() <<
      " " << layout -> hasHeightForWidth() << " " << layout -> heightForWidth (1) << "\n";
    int i = 0;
    QLayoutItem* item;
    while ( (item = layout -> itemAt (i)) )
    {
      kdDebugTime() << spaces.data() << "  " << i ++ <<
        " " << (item -> widget() ? item -> widget() -> metaObject() -> className() : "") <<
        " " << (item -> widget() ? QByteArray::number (int (item -> widget() -> winId())).data() : "") <<
        " " << item -> minimumSize().width() << "x" << item -> minimumSize().height() <<
        " " << item -> sizeHint().width() << "x" << item -> sizeHint().height() <<
        " " << item -> geometry().width() << "x" << item -> geometry().height() <<
        " " << item -> alignment() << " " << item -> expandingDirections() <<
        " " << item -> hasHeightForWidth() << " " << item -> heightForWidth (1) << "\n";
      if ( item -> layout() )
        dumpObject (item -> layout(), depth - 1, indent + 1);
    }
  }
  else if ( object -> inherits ("QAction") )
  {
    QAction* action = (QAction*) object;
    kdDebugTime() << spaces.data() << object -> metaObject() -> className() <<
      " " << object -> objectName().toLatin1().data() << " " << action -> text() <<
      " " << action -> isVisible() << " " << action -> isEnabled() << " " << action -> isChecked() << "\n";
  }
  else
    kdDebugTime() << spaces.data() << object -> metaObject() -> className() <<
      " " << object -> objectName().toLatin1().data() << "\n";
  indent ++;
  if ( depth -- > 0 )
    foreach ( object, object -> children() )
      if ( object )
        dumpObject (object, depth, indent);
}
#endif

KPlayerWindow::KPlayerWindow (QWidget* parent)
  : KXmlGuiWindow (parent)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Creating main window\n";
#endif
  m_status_label = m_state_label = m_progress_label = 0;
  m_menubar_normally_visible = m_statusbar_normally_visible = true;
  m_menubar_fullscreen_visible = m_statusbar_fullscreen_visible = false;
  m_messagelog_normally_visible = m_messagelog_fullscreen_visible = false;
  m_library_normally_visible = m_library_fullscreen_visible = false;
  //m_toggling_full_screen = m_active_window = false;
  m_initial_show = m_error_detected = false;
  m_handle_layout = m_show_library = m_show_log = false;
  setCorner (Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
  setCorner (Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
  Toolbar toolbars [KPLAYER_TOOLBARS] = {
    { "mainToolBar", KStandardAction::stdName (KStandardAction::ShowToolbar), true, false, false },
    { "playlistToolBar", "options_show_playlist", true, false, false },
    { "progressToolBar", "settings_progress_toolbar", true, false, false },
    { "volumeToolBar", "settings_volume_toolbar", false, false, false },
    { "contrastToolBar", "settings_contrast_toolbar", false, false, false },
    { "brightnessToolBar", "settings_brightness_toolbar", false, false, false },
    { "hueToolBar", "settings_hue_toolbar", false, false, false },
    { "saturationToolBar", "settings_saturation_toolbar", false, false, false }
  };
  QString captions [KPLAYER_TOOLBARS] = {
    i18n("Main Toolbar"),
    i18n("Playlist"),
    i18n("Progress"),
    i18n("Volume"),
    i18n("Contrast"),
    i18n("Brightness"),
    i18n("Hue"),
    i18n("Saturation")
  };
  const char* actions [KPLAYER_TOOLBARS - FIRST_SLIDER_TOOLBAR] = {
    "player_progress",
    "audio_volume",
    "video_contrast",
    "video_brightness",
    "video_hue",
    "video_saturation"
  };
  QString whatsthis [KPLAYER_TOOLBARS] = {
    i18n("The Main toolbar contains buttons for commonly used operations. Left clicking an icon will activate the corresponding action. Some of the buttons will pop up slider controls when clicked. The sliders let you change various player controls: sound volume, video contrast, brightness, hue and saturation. The video controls will only be available for video files."),
    i18n("The Playlist toolbar shows the multimedia title currently loaded or being played, offers commands to go to the next or previous items on the playlist, and also lets you see the entire playlist of items that are currently being played or have been played recently. If you select a different item from the list, KPlayer will load and start playing it. The toolbar also contains options to loop and shuffle the playlist."),
    i18n("The 'Progress and Seeking' toolbar is shown when the time length of a media file is known. It displays the playback progress and also allows seeking, that is, moving the playback point back and forth within the file. To seek to a specific position, drag the slider thumb to that position with the left mouse button, or simply click at that position with the middle mouse button. To move forward or backward in steps, left click the slider to the left or right of the thumb, or click the Forward and Backward buttons."),
    i18n("The Volume toolbar has a volume slider that shows the current sound volume and allows you to change it. This is the same slider that can be reached using the volume pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Volume Toolbar option on the Settings menu. Clicking the Volume button on this toolbar will also hide it. In addition the toolbar has a Mute button that can be used to toggle whether the sound is on or off."),
    i18n("The Contrast toolbar has a contrast slider that shows the current video contrast and allows you to change it. This is the same slider that can be reached using the contrast pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Contrast Toolbar option on the Settings menu. Clicking the Contrast button on this toolbar will also hide it."),
    i18n("The Brightness toolbar has a brightness slider that shows the current video brightness and allows you to change it. This is the same slider that can be reached using the brightness pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Brightness Toolbar option on the Settings menu. Clicking the Brightness button on this toolbar will also hide it."),
    i18n("The Hue toolbar has a hue slider that shows the current video hue and allows you to change it. This is the same slider that can be reached using the hue pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Hue Toolbar option on the Settings menu. Clicking the Hue button on this toolbar will also hide it."),
    i18n("The Saturation toolbar has a saturation slider that shows the current video saturation and allows you to change it. This is the same slider that can be reached using the saturation pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Saturation Toolbar option on the Settings menu. Clicking the Saturation button on this toolbar will also hide it.")
  };
  KPlayerEngine::initialize (actionCollection(), this, KGlobal::config().data());
  connect (engine(), SIGNAL (windowStateChanged (uint)), SLOT (windowStateChanged (uint)));
  connect (engine(), SIGNAL (syncronizeState (bool*)), SLOT (syncronizeState (bool*)));
  connect (engine(), SIGNAL (syncronizeControls (void)), SLOT (syncronizeControls (void)));
  connect (engine(), SIGNAL (updateLayout (const QSize&)), SLOT (updateLayout (const QSize&)));
  connect (engine(), SIGNAL (zoom()), SLOT (zoom()));
  connect (engine(), SIGNAL (finalizeLayout()), SLOT (finalizeLayout()));
  connect (engine(), SIGNAL (correctSize()), SLOT (correctSize()));
  connect (engine(), SIGNAL (dockWidgetVisibilityChanged()), SLOT (dockWidgetVisibility()));
  connect (engine() -> videoActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (engine() -> videoActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (engine() -> audioActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (engine() -> audioActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (engine() -> subtitleActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (engine() -> subtitleActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
  if ( args -> count() > 0 && (args -> isSet ("play") || args -> isSet ("play-next")
      || ! args -> isSet ("queue") && ! args -> isSet ("queue-next") && ! args -> isSet ("add-to-new-playlist")
      && ! args -> isSet ("add-to-playlists") && ! args -> isSet ("add-to-collection")) )
    engine() -> clearStoreSections ("kplayer:/nowplaying");
  KPlayerNode::initialize();
  m_log = new KPlayerLogWindow (actionCollection(), this);
  connect (log(), SIGNAL (visibilityChanged (bool)), engine(), SLOT (dockWidgetVisibility (bool)));
  connect (log(), SIGNAL (moved (bool)), engine(), SLOT (dockWidgetMove (bool)));
  connect (log(), SIGNAL (resized()), engine(), SLOT (dockWidgetResize()));
  connect (action ("log_clear"), SIGNAL (triggered()), SLOT (fileClearLog()));
  m_playlist = new KPlayerPlaylist (actionCollection(), this);
  m_library = new KPlayerLibraryWindow (actionCollection(), playlist(), this);
  connect (library(), SIGNAL (visibilityChanged (bool)), engine(), SLOT (dockWidgetVisibility (bool)));
  connect (library(), SIGNAL (moved (bool)), engine(), SLOT (dockWidgetMove (bool)));
  connect (library(), SIGNAL (resized()), engine(), SLOT (dockWidgetResize()));
  connect (library() -> library(), SIGNAL (makeVisible()), SLOT (makeLibraryVisible()));
  connect (library() -> library(), SIGNAL (enableActionGroup (const QString&, bool)),
    SLOT (enableSubmenu (const QString&, bool)));
  connect (library() -> library() -> playlistActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (library() -> library() -> playlistActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (library() -> library() -> columnActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (library() -> library() -> columnActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (libraryActionListUpdated (KPlayerActionList*)));
  connect (library() -> library() -> editActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (library() -> library() -> editActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (libraryActionListUpdated (KPlayerActionList*)));
  connect (library() -> library() -> goToActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (library() -> library() -> goToActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (library() -> library() -> historyActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (library() -> library() -> historyActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (playlist(), SIGNAL (started()), SLOT (playlistStarted()));
  connect (playlist(), SIGNAL (activated()), SLOT (playlistActivated()));
  connect (playlist(), SIGNAL (stopped()), SLOT (playlistStopped()));
  connect (playlist(), SIGNAL (enableActionGroup (const QString&, bool)), SLOT (enableSubmenu (const QString&, bool)));
  connect (playlist() -> playlistActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (playlist() -> playlistActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (playlist() -> recentActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (playlist() -> recentActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (playlist() -> devicesActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (playlist() -> devicesActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  connect (playlist() -> playlistAddActionList(), SIGNAL (updating (KPlayerActionList*)),
    SLOT (actionListUpdating (KPlayerActionList*)));
  connect (playlist() -> playlistAddActionList(), SIGNAL (updated (KPlayerActionList*)),
    SLOT (actionListUpdated (KPlayerActionList*)));
  //connect (actionCollection(), SIGNAL (actionStatusText(const QString&)), SLOT (setStatusMessage(const QString&)));
  //connect (actionCollection(), SIGNAL (clearStatusText()), SLOT (clearStatusMessage()));
  //actionCollection() -> setHighlightingEnabled (true);
  connect (process(), SIGNAL (stateChanged(KPlayerProcess::State, KPlayerProcess::State)),
    SLOT (playerStateChanged(KPlayerProcess::State, KPlayerProcess::State)));
  connect (process(), SIGNAL (progressChanged(float, KPlayerProcess::ProgressType)),
    SLOT (playerProgressChanged(float, KPlayerProcess::ProgressType)));
  connect (process(), SIGNAL (infoAvailable()), SLOT (playerInfoAvailable()));
  connect (process(), SIGNAL (sizeAvailable()), SLOT (playerSizeAvailable()));
  connect (process(), SIGNAL (messageReceived (QString)), SLOT (playerMessageReceived (QString)));
  connect (process(), SIGNAL (errorDetected()), SLOT (playerErrorDetected()));
  connect (configuration(), SIGNAL (updated()), SLOT (refreshSettings()));
  connect (kPlayerWorkspace(), SIGNAL (contextMenu(const QPoint&)), SLOT (contextMenu(const QPoint&)));
  setCentralWidget (kPlayerWorkspace());
  initStatusBar();
  initActions();
  log() -> initialize ((QMenu*) factory() -> container ("log_popup", this));
  playlist() -> initialize ((QMenu*) factory() -> container ("playlist_popup", this));
  library() -> library() -> initialize ((QMenu*) factory() -> container ("library_popup", this));
  uint i;
  //for ( i = 0; i < menuBar() -> count(); i ++ )
  //  connect (popupMenu (i), SIGNAL (aboutToHide()), SLOT (clearStatusMessage()));
  for ( i = 0; i < KPLAYER_TOOLBARS; i ++ )
    m_toolbar[i] = toolbars[i];
  KToolBar* toolbar = toolBar (m_toolbar[PLAYLIST_TOOLBAR].name);
  toolbar -> setAllowedAreas (Qt::TopToolBarArea | Qt::BottomToolBarArea);
  menuBar() -> setWhatsThis (i18n("The menu bar contains names of drop-down menus. Left click a name to alternately show and hide that menu, or use Alt + the underlined letter in the name as a hot key; for example, Alt+F shows the File menu."));
  statusBar() -> setWhatsThis (i18n("The status bar shows general information about the player's status and progress."));
  for ( i = 0; i < KPLAYER_TOOLBARS; i ++ )
  {
    toolbar = toolBar (m_toolbar[i].name);
    toolbar -> setWindowTitle (captions[i]);
    toolbar -> setWhatsThis (whatsthis[i]);
    //kdDebugTime() << "Orientation " << sliderAction (m_toolbar[i].action) -> slider() -> orientation() << "\n";
    if ( i >= FIRST_SLIDER_TOOLBAR )
      ((KPlayerSliderAction*) action (actions [i - FIRST_SLIDER_TOOLBAR])) -> slider() -> setOrientation (toolbar -> orientation());
  }
  addDockWidget (Qt::BottomDockWidgetArea, library());
  addDockWidget (Qt::BottomDockWidgetArea, log());
  splitDockWidget (library(), log(), Qt::Vertical);
  //tabifyDockWidget (log(), library());
  //setAutoSaveSettings ("MainWindow", false);
  //resetAutoSaveSettings();  // saveWindowSize = false;
  readOptions();
  if ( settings() -> maximized() )
    showMaximized();
  enablePlayerActions();
  enableVideoActions();
  engine() -> handleLayout();
  setAcceptDrops (true);
  setFocusPolicy (Qt::StrongFocus);
  setFocusProxy (kPlayerWorkspace() -> focusProxy());
}

KPlayerWindow::~KPlayerWindow()
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Destroying main window\n";
#endif
}

void KPlayerWindow::initActions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Creating actions\n";
#endif
  QAction* action = KStandardAction::quit (this, SLOT (fileQuit()), actionCollection());
  action -> setStatusTip (i18n("Quits KPlayer"));
  action -> setWhatsThis (i18n("The 'Quit' command saves the playlist and all settings, stops playing and closes KPlayer."));

  action = KStandardAction::showMenubar (this, SLOT (viewMenubar()), actionCollection());
  action -> setStatusTip (i18n("Shows/hides the menu bar"));
  action -> setWhatsThis (i18n("The 'Show Menu Bar' command shows or hides the menu bar."));

  action = KStandardAction::showStatusbar (this, SLOT (viewStatusbar()), actionCollection());
  action -> setStatusTip (i18n("Shows/hides the status bar"));
  action -> setWhatsThis (i18n("The 'Show Status Bar' command shows or hides the status bar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_playlist", action);
  connect (action, SIGNAL (triggered()), SLOT (viewPlaylist()));
  action -> setText (i18n("&Show Playlist"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_P);
  action -> setStatusTip (i18n("Shows/hides the playlist"));
  action -> setWhatsThis (i18n("The 'Show Playlist' command shows or hides the playlist toolbar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_library", action);
  connect (action, SIGNAL (triggered()), SLOT (viewLibrary()));
  action -> setText (i18n("Sho&w Library"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_L);
  action -> setStatusTip (i18n("Shows/hides the multimedia library"));
  action -> setWhatsThis (i18n("The 'Show Library' command shows or hides the multimedia library."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_log", action);
  connect (action, SIGNAL (triggered()), SLOT (viewMessageLog()));
  action -> setText (i18n("Show Message &Log"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_G);
  action -> setStatusTip (i18n("Shows/hides the message log"));
  action -> setWhatsThis (i18n("The 'Show Message Log' command shows or hides the message log."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewMainToolbar()));
  action -> setText (i18n("Show Main &Toolbar"));
  action -> setStatusTip (i18n("Shows/hides the main toolbar"));
  action -> setWhatsThis (i18n("The 'Show Main Toolbar' command shows or hides the main toolbar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_progress_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewProgressToolbar()));
  action -> setText (i18n("Show &Progress Toolbar"));
  action -> setStatusTip (i18n("Shows/hides the progress toolbar"));
  action -> setWhatsThis (i18n("The 'Show Progress Toolbar' command shows or hides the progress toolbar. This command is available when the time length of the current file is known."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_volume_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewVolumeToolbar()));
  action -> setText (i18n("Show &Volume Toolbar"));
  action -> setIcon (KIcon ("player-volume"));
  action -> setStatusTip (i18n("Shows/hides the volume toolbar"));
  action -> setWhatsThis (i18n("The 'Show Volume Toolbar' command shows or hides the volume toolbar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_contrast_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewContrastToolbar()));
  action -> setText (i18n("Show C&ontrast Toolbar"));
  action -> setIcon (KIcon ("contrast"));
  action -> setStatusTip (i18n("Shows/hides the contrast toolbar"));
  action -> setWhatsThis (i18n("The 'Show Contrast Toolbar' command shows or hides the contrast toolbar. This command is available for video files."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_brightness_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewBrightnessToolbar()));
  action -> setText (i18n("Show &Brightness Toolbar"));
  action -> setIcon (KIcon ("brightness"));
  action -> setStatusTip (i18n("Shows/hides the brightness toolbar"));
  action -> setWhatsThis (i18n("The 'Show Brightness Toolbar' command shows or hides the brightness toolbar. This command is available for video files."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_hue_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewHueToolbar()));
  action -> setText (i18n("Show H&ue Toolbar"));
  action -> setIcon (KIcon ("hue"));
  action -> setStatusTip (i18n("Shows/hides the hue toolbar"));
  action -> setWhatsThis (i18n("The 'Show Hue Toolbar' command shows or hides the hue toolbar. This command is available for video files."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_saturation_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewSaturationToolbar()));
  action -> setText (i18n("Show &Saturation Toolbar"));
  action -> setIcon (KIcon ("saturation"));
  action -> setStatusTip (i18n("Shows/hides the saturation toolbar"));
  action -> setWhatsThis (i18n("The 'Show Saturation Toolbar' command shows or hides the saturation toolbar. This command is available for video files."));

  action = KStandardAction::keyBindings (this, SLOT (settingsKeyBindings()), actionCollection());
  action -> setStatusTip (i18n("Opens the KPlayer key bindings dialog"));
  action -> setWhatsThis (i18n("The 'Configure Shortcuts' command opens a dialog that allows you to see and change KPlayer's shortcut key bindings, or associations between actions and the corresponding keys or combinations of keys that activate them. If you change the bindings, make sure not to duplicate an existing shortcut and also not to use the Shift key for your new shortcuts, because the Shift key has a special function in KPlayer."));

  action = KStandardAction::configureToolbars (this, SLOT (settingsConfigureToolbars()), actionCollection());
  action -> setStatusTip (i18n("Opens the KPlayer toolbar configuration dialog"));
  action -> setWhatsThis (i18n("The 'Configure Toolbars' command opens a dialog that allows you to see and change KPlayer's toolbars and controls assigned to them."));

  action = KStandardAction::preferences (this, SLOT (settingsConfigure()), actionCollection());
  action -> setStatusTip (i18n("Opens the KPlayer configuration dialog"));
  action -> setWhatsThis (i18n("The 'Configure KPlayer' command opens a dialog that allows you to configure the program, modifying various aspects of its functionality, user interface and interaction with MPlayer. For more information see the Configuration dialog chapter and the Advanced configuration micro-HOWTO in the user manual."));

  createGUI();

  /*
  action = actionCollection() -> action (KStandardAction::stdName (KStandardAction::ShowMenubar));
  if ( action )
  {
    action -> setStatusTip (i18n("Shows/hides the menu bar"));
    action -> setWhatsThis (i18n("Show Menubar command shows or hides the menu bar."));
  }

  action = actionCollection() -> action (KStandardAction::stdName (KStandardAction::ShowStatusbar));
  if ( action )
  {
    action -> setStatusTip (i18n("Shows/hides the status bar"));
    action -> setWhatsThis (i18n("Show Statusbar command shows or hides the status bar."));
  }

  action = actionCollection() -> action ("help_contents");
  if ( action )
  {
    action -> setStatusTip (i18n("Opens the KPlayer user manual"));
    action -> setWhatsThis (i18n("KPlayer Handbook command opens and displays the KPlayer user manual."));
  }

  action = actionCollection() -> action ("help_whats_this");
  if ( action )
  {
    action -> setStatusTip (i18n("Lets you click any part of KPlayer to get its description"));
    action -> setWhatsThis (i18n("What's This command changes the mouse pointer to a question mark and lets you click a KPlayer interface element to get a quick description of its purpose and functionality."));
  }
  */

  action = this -> action ("help_report_bug");
  if ( action )
  {
    disconnect (action, SIGNAL(triggered()), 0, 0);
    connect (action, SIGNAL(triggered()), SLOT(helpReportBug()));
    //action -> setStatusTip (i18n("Opens the manual section on bug reporting"));
    //action -> setWhatsThis (i18n("Report Bug command opens the section of KPlayer user manual that explains how to report a bug in KPlayer, including all the information that may be helpful in finding and fixing the bug."));
  }

  /*
  action = actionCollection() -> action ("help_about_app");
  if ( action )
  {
    action -> setStatusTip (i18n("Shows information about this version of KPlayer"));
    action -> setWhatsThis (i18n("About KPlayer command displays some brief information about KPlayer's version number, authors and license agreement."));
  }

  action = actionCollection() -> action ("help_about_kde");
  if ( action )
  {
    action -> setStatusTip (i18n("Shows information about your KDE version"));
    action -> setWhatsThis (i18n("About KDE command shows some information about the version of KDE that you are running."));
  }
  */
}

void KPlayerWindow::initStatusBar (void)
{
  statusBar() -> insertPermanentItem (i18n("Ready"), ID_STATUS_MSG, 1);
  statusBar() -> insertPermanentItem (i18n("Idle"), ID_STATE_MSG, 1);
  statusBar() -> insertPermanentItem ("", ID_PROGRESS_MSG, 1);
  QObjectList children (statusBar() -> children());
  for ( QObjectList::ConstIterator iterator (children.constBegin()); iterator != children.constEnd(); ++ iterator )
  {
    QObject* child = *iterator;
    if ( child -> inherits ("QLabel") )
    {
      QLabel* label = (QLabel*) child;
      label -> removeEventFilter (statusBar());
      if ( label -> text() == i18n("Ready") )
        m_status_label = label;
      else if ( label -> text() == i18n("Idle") )
        m_state_label = label;
      else if ( label -> text().isEmpty() )
        m_progress_label = label;
    }
  }
  statusBar() -> setMinimumHeight (statusBar() -> layout() -> minimumSize().height());
  if ( m_status_label )
  {
    connect (m_status_label, SIGNAL (linkActivated (const QString&)), SLOT (showErrors (const QString&)));
    m_status_label -> setWhatsThis (i18n("Status area of the status bar tells you if there have been playback errors."));
  }
  if ( m_state_label )
    m_state_label -> setWhatsThis (i18n("State area of the status bar displays the current player state."));
  if ( m_progress_label )
    m_progress_label -> setWhatsThis (i18n("Progress area of the status bar shows playback progress and the total length if known."));
}

void KPlayerWindow::refreshSettings (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::refreshSettings\n";
#endif
  QApplication::postEvent (this, new QEvent (QEvent::LayoutRequest));
  if ( ! settings() -> properties() -> url().isEmpty() )
    setCaption (settings() -> properties() -> caption());
  enableVideoActions();
  engine() -> handleLayout();
}

void KPlayerWindow::saveOptions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Saving options\n";
#endif
  config() -> deleteGroup ("General Options");
  KConfigGroup group (config() -> group ("General Options"));
  saveMainWindowSettings (group);
  group.writeEntry ("Menu Bar Normally Visible", m_menubar_normally_visible);
  group.writeEntry ("Menu Bar FullScreen Visible", m_menubar_fullscreen_visible);
  group.writeEntry ("Status Bar Normally Visible", m_statusbar_normally_visible);
  group.writeEntry ("Status Bar FullScreen Visible", m_statusbar_fullscreen_visible);
  group.writeEntry ("Message Log Normally Visible", m_messagelog_normally_visible);
  group.writeEntry ("Message Log FullScreen Visible", m_messagelog_fullscreen_visible);
  group.writeEntry ("Playlist Editor Normally Visible", m_library_normally_visible);
  group.writeEntry ("Playlist Editor FullScreen Visible", m_library_fullscreen_visible);
  for ( int i = 0; i < KPLAYER_TOOLBARS; i ++ )
  {
    group.writeEntry (QString (m_toolbar[i].name) + " Normally Visible", m_toolbar[i].normally_visible);
    group.writeEntry (QString (m_toolbar[i].name) + " FullScreen Visible", m_toolbar[i].fullscreen_visible);
    group.writeEntry (QString (m_toolbar[i].name) + " Auto Break", m_toolbar[i].auto_break);
    group.writeEntry (QString (m_toolbar[i].name) + " Button Style",
      (int) toolBar (m_toolbar[i].name) -> toolButtonStyle());
  }
}

void KPlayerWindow::readOptions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Reading options\n";
#endif
  KConfigGroup group (config() -> group ("General Options"));
  applyMainWindowSettings (group);
  m_menubar_normally_visible = group.readEntry ("Menu Bar Normally Visible", m_menubar_normally_visible);
  m_menubar_fullscreen_visible = group.readEntry ("Menu Bar FullScreen Visible", m_menubar_fullscreen_visible);
  m_statusbar_normally_visible = group.readEntry ("Status Bar Normally Visible", m_statusbar_normally_visible);
  m_statusbar_fullscreen_visible = group.readEntry ("Status Bar FullScreen Visible", m_statusbar_fullscreen_visible);
  for ( int i = 0; i < KPLAYER_TOOLBARS; i ++ )
  {
    m_toolbar[i].normally_visible = group.readEntry (QString (m_toolbar[i].name) + " Normally Visible", m_toolbar[i].normally_visible);
    m_toolbar[i].fullscreen_visible = group.readEntry (QString (m_toolbar[i].name) + " FullScreen Visible", m_toolbar[i].fullscreen_visible);
    m_toolbar[i].auto_break = group.readEntry (QString (m_toolbar[i].name) + " Auto Break", m_toolbar[i].auto_break);
    toolBar (m_toolbar[i].name) -> setToolButtonStyle ((Qt::ToolButtonStyle) group.readEntry (QString (m_toolbar[i].name) + " Button Style", (int) Qt::ToolButtonIconOnly));
    //kdDebugTime() << "Show " << m_toolbar[i].name << " " << m_toolbar[i].action << "\n";
  }
  m_messagelog_normally_visible = group.readEntry ("Message Log Normally Visible", m_messagelog_normally_visible);
  m_messagelog_fullscreen_visible = group.readEntry ("Message Log FullScreen Visible", m_messagelog_fullscreen_visible);
  m_library_normally_visible = group.readEntry ("Playlist Editor Normally Visible", m_library_normally_visible);
  m_library_fullscreen_visible = group.readEntry ("Playlist Editor FullScreen Visible", m_library_fullscreen_visible);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Done reading options\n";
#endif
}

void KPlayerWindow::enableToolbar (int index)
{
  toggleAction (m_toolbar[index].action) -> setEnabled (true);
}

void KPlayerWindow::disableToolbar (int index)
{
  KToggleAction* action = toggleAction (m_toolbar[index].action);
  if ( action -> isChecked() )
    action -> setChecked (false);
  action -> setEnabled (false);
}

void KPlayerWindow::enablePlaylistActions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayer: Enabling playlist actions\n";
#endif
  if ( playlist() -> isEmpty() )
    disableToolbar (PLAYLIST_TOOLBAR);
  else
    enableToolbar (PLAYLIST_TOOLBAR);
}

void KPlayerWindow::enablePlayerActions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayer: Enabling player actions\n";
#endif
  if ( process() -> state() == KPlayerProcess::Idle || process() -> isInfoAvailable()
    || settings() -> properties() -> hasLength() )
  {
    if ( settings() -> properties() -> hasLength() )
      enableToolbar (PROGRESS_TOOLBAR);
    else
      disableToolbar (PROGRESS_TOOLBAR);
  }
  enableSubmenu ("seek", settings() -> properties() -> hasLength() && process() -> state() == KPlayerProcess::Playing);
  enableSubmenu ("audio", process() -> state() != KPlayerProcess::Paused);
  enableSubmenu ("advanced", process() -> state() != KPlayerProcess::Paused);
}

void KPlayerWindow::enableVideoActions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayer: Enabling video actions\n";
#endif
  bool video = settings() -> properties() -> hasVideo();
  bool unpaused = video && process() -> state() != KPlayerProcess::Paused;
  if ( process() -> state() != KPlayerProcess::Running )
  {
    for ( int i = FIRST_VIDEO_TOOLBAR; i < KPLAYER_TOOLBARS; i ++ )
      if ( video )
        enableToolbar (i);
      else
        disableToolbar (i);
  }
  enableSubtitleActions();
  enableSubmenu ("video", unpaused);
}

void KPlayerWindow::actionListUpdating (KPlayerActionList* list)
{
  unplugActionList (list -> objectName());
}

void KPlayerWindow::actionListUpdated (KPlayerActionList* list)
{
  bool has_actions = ! list -> isEmpty();
  QString name (list -> objectName());
  if ( has_actions )
    plugActionList (name, list -> actions());
  enableSubmenu (name, has_actions);
}

void KPlayerWindow::libraryActionListUpdated (KPlayerActionList* list)
{
  bool has_actions = ! list -> isEmpty();
  QString name (list -> objectName());
  if ( has_actions )
    plugActionList (name, list -> actions());
  enableSubmenu (name, has_actions && library() -> visible());
}

void KPlayerWindow::enableSubmenu (const QString& name, bool enable)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::enableSubmenu " << name << " " << enable << "\n";
#endif
  QList<QMenu*> menus = findChildren<QMenu*> (name);
  foreach ( QMenu* menu, menus )
    menu -> menuAction() -> setEnabled (enable);
}

void KPlayerWindow::enableSubtitleActions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayer: Enabling subtitle actions\n";
#endif
  enableSubmenu ("subtitles", settings() -> hasSubtitles() && process() -> state() != KPlayerProcess::Paused);
}

/*void KPlayerWindow::checkBarActions (void)
{
  toggleAction (KStandardAction::stdName (KStandardAction::ShowMenubar)) -> setChecked (menuBar() -> isVisible());
  for ( int i = 0; i < KPLAYER_TOOLBARS; i ++ )
    toggleAction (m_toolbar[i].action) -> setChecked (toolBar (m_toolbar[i].name) -> isVisible());
  toggleAction (KStandardAction::stdName (KStandardAction::ShowStatusbar)) -> setChecked (statusBar() -> isVisible());
}*/

void KPlayerWindow::dragEnterEvent (QDragEnterEvent *event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Drag enter event\n";
#endif
  event -> setAccepted (KUrl::List::canDecode (event -> mimeData()));
}

void KPlayerWindow::dropEvent (QDropEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Drop event\n";
#endif
  KUrl::List urls = KUrl::List::fromMimeData (event -> mimeData());
  if ( ! urls.isEmpty() )
    playlist() -> playUrls (urls);
}

bool KPlayerWindow::fullScreen (void) const
{
  return (KWindowSystem::windowInfo (winId(), NET::WMState).state() & NET::FullScreen) == NET::FullScreen;
}

bool KPlayerWindow::maximized (void) const
{
  return (KWindowSystem::windowInfo (winId(), NET::WMState).state() & NET::Max) == NET::Max;
}

bool KPlayerWindow::minimized (void) const
{
  return KWindowSystem::windowInfo (winId(), NET::WMState | NET::XAWMState).isMinimized();
}

bool KPlayerWindow::shaded (void) const
{
  return (KWindowSystem::windowInfo (winId(), NET::WMState).state() & NET::Shaded) == NET::Shaded;
}

void KPlayerWindow::start (void)
{
  if ( minimized() || shaded() )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "Restoring minimized window\n";
#endif
    //clearWState (WState_Visible); // or showNormal won't work
    settings() -> maximized() ? showMaximized() : showNormal();
  }
  else
    show();
  raise();
  activateWindow(); // doesn't work after restoring from minimized state
  KWindowSystem::forceActiveWindow (winId());
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Main window minimum size " << minimumWidth() << "x" << minimumHeight()
    << ", maximum size " << maximumWidth() << "x" << maximumHeight()
    << ", size " << width() << "x" << height() << "\n";
#endif
  KPlayerX11GetKeyboardMouseState (winId());
  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
  if ( args -> count() > 0 )
  {
    KUrl::List urls;
    for ( int i = 0; i < args -> count(); i ++ )
      urls << args -> url (i);
    if ( args -> isSet ("play") )
      playlist() -> playUrls (urls);
    else if ( args -> isSet ("queue") )
      playlist() -> queueUrls (urls);
    else if ( args -> isSet ("play-next") )
      playlist() -> playNextUrls (urls);
    else if ( args -> isSet ("queue-next") )
      playlist() -> queueNextUrls (urls);
    else if ( args -> isSet ("add-to-new-playlist") )
      library() -> library() -> treeView() -> addToNewPlaylistUrls (urls);
    else if ( args -> isSet ("add-to-playlists") )
      library() -> library() -> treeView() -> addToPlaylistsUrls (urls);
    else if ( args -> isSet ("add-to-collection") )
      library() -> library() -> treeView() -> addToCollectionUrls (urls);
    else
      playlist() -> playUrls (urls);
    //openUrl (KCmdLineArgs::makeURL (args -> arg (args -> count() - 1)));
  }
  args -> clear();
}

void KPlayerWindow::contextMenu (const QPoint& global_position)
{
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Main " << winId() << " wspace " << kPlayerWorkspace() -> winId()
    << " widget " << kPlayerWidget() -> winId() << "\n";
  dumpObject (this);
  dumpObject (actionCollection());
#endif
  QMenu* popup = (QMenu*) factory() -> container ("player_popup", this);
  if ( popup )
    popup -> popup (global_position);
}

void KPlayerWindow::contextMenuEvent (QContextMenuEvent* event)
{
  KXmlGuiWindow::contextMenuEvent (event);
  contextMenu (event -> globalPos());
  event -> accept();
}

void KPlayerWindow::playlistStarted (void)
{
  setStatusText (i18n("Ready"));
  log() -> setError (false);
  //if ( m_status_label )
  //  m_status_label -> unsetCursor();
}

void KPlayerWindow::playlistActivated (void)
{
  setCaption (settings() -> properties() -> caption());
  //clearStatusMessage();
  if ( m_progress_label )
    m_progress_label -> setText ("");
  connect (settings() -> properties(), SIGNAL (updated()), this, SLOT (refreshProperties()));
  enablePlaylistActions();
  enableVideoActions();
  engine() -> handleLayout();
}

void KPlayerWindow::playlistStopped (void)
{
  if ( m_error_detected && ! log() -> isEmpty() )
    log() -> setError (true);
  m_error_detected = false;
  playerProgressChanged (process() -> position(), KPlayerProcess::Position);
  enablePlaylistActions();
  engine() -> handleLayout();
}

void KPlayerWindow::showErrors (const QString&)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::showErrors\n";
#endif
  if ( log() -> hasError() || m_error_detected )
  {
    showMessageLog (true);
    m_show_log = true;
    engine() -> handleLayout();
  }
}

void KPlayerWindow::refreshProperties (void)
{
  setCaption (settings() -> properties() -> caption());
  enableVideoActions();
  engine() -> handleLayout();
}

void KPlayerWindow::closeEvent (QCloseEvent* event)
{
  disconnect (log(), SIGNAL (visibilityChanged (bool)), engine(), SLOT (dockWidgetVisibility (bool)));
  disconnect (library(), SIGNAL (visibilityChanged (bool)), engine(), SLOT (dockWidgetVisibility (bool)));
  disconnect (settings() -> properties(), SIGNAL (updated()), this, SLOT (refreshProperties()));
  disconnect (configuration(), SIGNAL (updated()), this, SLOT (refreshSettings()));
  saveOptions();
  log() -> logWidget() -> terminate();
  library() -> library() -> terminate();
  playlist() -> terminate();
  KPlayerEngine::terminate();
  KXmlGuiWindow::closeEvent (event);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::closeEvent\n";
#endif
}

void KPlayerWindow::fileClearLog (void)
{
  setStatusText (i18n("Ready"));
  //if ( m_status_label )
  //  m_status_label -> unsetCursor();
}

void KPlayerWindow::fileQuit (void)
{
  close();
}

void KPlayerWindow::viewPlaylist (void)
{
  showToolbar (PLAYLIST_TOOLBAR, toggleAction (m_toolbar[PLAYLIST_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewLibrary (void)
{
  showLibrary (toggleAction ("options_show_library") -> isChecked());
  m_show_library = showLibrary();
  engine() -> handleLayout();
}

void KPlayerWindow::makeLibraryVisible (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::makeLibraryVisible\n";
#endif
  showLibrary (true);
  m_show_library = true;
  engine() -> handleLayout();
}

void KPlayerWindow::viewMenubar (void)
{
  showMenubar (toggleAction (KStandardAction::stdName (KStandardAction::ShowMenubar)) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewStatusbar (void)
{
  showStatusbar (toggleAction (KStandardAction::stdName (KStandardAction::ShowStatusbar)) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewMessageLog (void)
{
  showMessageLog (toggleAction ("options_show_log") -> isChecked());
  m_show_log = showMessageLog();
  engine() -> handleLayout();
}

QTabBar* KPlayerWindow::findTabBar (QDockWidget* widget, int* index)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::findTabBar\n";
#endif
  QString title (widget -> windowTitle());
  title.remove ('&');
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << " Title  " << title << "\n";
#endif
  QList<QTabBar*> tabbars = findChildren<QTabBar*>();
  foreach ( QTabBar* tabbar, tabbars )
    if ( ! tabbar -> isHidden() )
      for ( int i = 0; i < tabbar -> count(); i ++ )
      {
        QString tabtitle (tabbar -> tabText (i));
        tabtitle.remove ('&');
#ifdef DEBUG_KPLAYER_WINDOW
        kdDebugTime() << " Tab    " << tabtitle << "\n";
#endif
        if ( tabtitle == title )
        {
#ifdef DEBUG_KPLAYER_WINDOW
          kdDebugTime() << " Found  " << tabtitle << "\n";
#endif
          if ( index )
            *index = i;
          return tabbar;
        }
      }
  return 0;
}

void KPlayerWindow::viewMainToolbar (void)
{
  showToolbar (MAIN_TOOLBAR, toggleAction (m_toolbar[MAIN_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewProgressToolbar (void)
{
  showToolbar (PROGRESS_TOOLBAR, toggleAction (m_toolbar[PROGRESS_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewVolumeToolbar (void)
{
  showToolbar (VOLUME_TOOLBAR, toggleAction (m_toolbar[VOLUME_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewContrastToolbar (void)
{
  showToolbar (CONTRAST_TOOLBAR, toggleAction (m_toolbar[CONTRAST_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewBrightnessToolbar (void)
{
  showToolbar (BRIGHTNESS_TOOLBAR, toggleAction (m_toolbar[BRIGHTNESS_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewHueToolbar (void)
{
  showToolbar (HUE_TOOLBAR, toggleAction (m_toolbar[HUE_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::viewSaturationToolbar (void)
{
  showToolbar (SATURATION_TOOLBAR, toggleAction (m_toolbar[SATURATION_TOOLBAR].action) -> isChecked());
  engine() -> handleLayout();
}

void KPlayerWindow::settingsKeyBindings (void)
{
  // reportedly was buggy in KDE 3.0x
  KShortcutsDialog::configure (actionCollection());
//KKeyDialog keydlg (true);
//keydlg.insert (actionCollection());
//keydlg.configure (true);
}

void KPlayerWindow::settingsConfigureToolbars (void)
{
  saveMainWindowSettings (config() -> group ("General Options"));
  KEditToolBar dialog (actionCollection());
  connect (&dialog, SIGNAL (newToolbarConfig()), SLOT (newToolbarConfiguration()));
  dialog.exec();
}

void KPlayerWindow::newToolbarConfiguration (void)
{
  createGUI();
  applyMainWindowSettings (config() -> group ("General Options"));
}

void KPlayerWindow::settingsConfigure (void)
{
  KPlayerSettingsDialog (this).exec();
}

void KPlayerWindow::helpReportBug (void)
{
  KToolInvocation::invokeHelp ("howto-bug-reporting");
}

void KPlayerWindow::playerStateChanged (KPlayerProcess::State state, KPlayerProcess::State previous)
{
  static const QString stateMessages [4] = { i18n("Idle"), i18n("Running"), i18n("Playing"), i18n("Paused") };
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Main window received state change: " << previous << " => " << state << "\n";
#endif
  //clearStatusMessage();
  if ( state != process() -> state() )
    return;
  if ( m_state_label )
    m_state_label -> setText (stateMessages [state]);
  if ( state == KPlayerProcess::Running && previous != state )
  {
    if ( ! m_error_detected )
      action ("log_clear") -> trigger();
    else if ( ! log() -> isEmpty() )
      log() -> addLine ("------------------------------------------------------------");
    if ( m_error_detected && ! log() -> isEmpty() )
      log() -> setError (true);
    m_error_detected = false;
  }
  if ( state == KPlayerProcess::Idle && previous != state && engine() -> stopped() )
  {
    if ( m_error_detected && ! log() -> isEmpty() )
      log() -> setError (true);
    m_error_detected = false;
  }
  enablePlayerActions();
  enableVideoActions();
  engine() -> handleLayout();
}

void KPlayerWindow::playerProgressChanged (float progress, KPlayerProcess::ProgressType type)
{
  if ( ! m_progress_label )
    return;
  if ( type == KPlayerProcess::Position )
    m_progress_label -> setText (process() -> positionString());
  else
  {
    int value = limit (int (progress + 0.5), 0, 100);
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "Progress " << type << " " << progress << " " << value << "\n";
#endif
    m_progress_label -> setText ((type == KPlayerProcess::CacheFill ? i18n("Cache fill: %1%", value)
      : type == KPlayerProcess::IndexGeneration ? i18n("Generating index: %1%", value)
      : i18n("Transferring file: %1%", value)));
  }
}

void KPlayerWindow::playerInfoAvailable (void)
{
  enablePlayerActions();
  if ( settings() -> properties() -> hasLength() )
    playerProgressChanged (process() -> position(), KPlayerProcess::Position);
  engine() -> handleLayout();
}

void KPlayerWindow::playerSizeAvailable (void)
{
  enableVideoActions();
  engine() -> handleLayout();
}

void KPlayerWindow::playerMessageReceived (QString message)
{
  if ( message.isEmpty() )
    return;
  log (message);
}

void KPlayerWindow::playerErrorDetected (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Error detected\n";
#endif
  setStatusText (QString ("<a href=error>%1</a>").arg (i18n("Error")));
  //if ( log() -> isHidden() && m_status_label )
  //  m_status_label -> setCursor (Qt::PointingHandCursor);
  m_error_detected = true;
  if ( configuration() -> showMessagesOnError() )
  {
    showMessageLog (true);
    m_show_log = true;
    engine() -> handleLayout();
  }
}

void KPlayerWindow::log (QString message)
{
  if ( message.isEmpty() )
    return;
  log() -> addLine (message);
}

void KPlayerWindow::setStatusText (const QString& text)
{
  statusBar() -> clearMessage();
//statusBar() -> changeItem (text, ID_STATUS_MSG);
  if ( m_status_label )
    m_status_label -> setText (text);
}

/*
void KPlayerWindow::setStatusMessage (const QString& text)
{
  if ( text.isEmpty() )
  {
    clearStatusMessage();
    return;
  }
  QMenu* popup = (QMenu*) factory() -> container ("player_popup", this);
  if ( ! popup || ! popup -> isVisible() )
    popup = (QMenu*) factory() -> container ("library_popup", this);
  if ( ! popup || ! popup -> isVisible() )
    for ( uint i = 0; i < menuBar() -> count(); i ++ )
      if ( (popup = popupMenu (i)) && popup -> isVisible() )
        break;
  if ( popup && popup -> isVisible() )
  {
    statusBar() -> message (text);
    m_progress_label -> hide();
    m_state_label -> hide();
  }
}

void KPlayerWindow::clearStatusMessage (void)
{
  statusBar() -> clearMessage();
  m_state_label -> show();
  m_progress_label -> show();
}
*/

void KPlayerWindow::showEvent (QShowEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::showEvent\n";
#endif
  KXmlGuiWindow::showEvent (event);
  if ( m_initial_show )
    return;
  QPoint position (configuration() -> preferredWindowPosition());
  if ( x() != position.x() && position.x() >= 0 && y() != position.y() && position.y() >= 0 )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "KPlayerWindow::showEvent: adjusting position to " << position.x() << "x" << position.y() << "\n";
#endif
    move (position.x(), position.y());
  }
}

void KPlayerWindow::changeEvent (QEvent* event)
{
  KXmlGuiWindow::changeEvent (event);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::changeEvent\n";
  kdDebugTime() << " Type   " << event -> type() << "\n";
#endif
  if ( event -> type() == QEvent::WindowActivate )
    KPlayerX11GetKeyboardMouseState (winId());
#ifdef DEBUG_KPLAYER_WINDOW
  else if ( event -> type() == QEvent::WindowStateChange )
    kdDebugTime() << " State  " << ((QWindowStateChangeEvent*) event) -> oldState() << " -> " << windowState() << "\n";
#endif
}

void KPlayerWindow::focusInEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Window focus in event\n";
#endif
  KXmlGuiWindow::focusInEvent (event);
}

void KPlayerWindow::focusOutEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Window focus out event\n";
#endif
  KXmlGuiWindow::focusOutEvent (event);
}

void KPlayerWindow::moveEvent (QMoveEvent* event)
{
  KXmlGuiWindow::moveEvent (event);
  if ( ! fullScreen() && ! m_handle_layout && ! settings() -> maximized() && ! maximized() && m_initial_show )
  {
    //m_normal_geometry.setRect (x(), y(), width(), height());
    if ( event -> spontaneous() )
      configuration() -> setPreferredWindowPosition (KWindowSystem::windowInfo (winId(), NET::WMFrameExtents).frameGeometry().topLeft());
  }
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "WiMove " << event -> oldPos().x() << "x" << event -> oldPos().y()
    << " => " << event -> pos().x() << "x" << event -> pos().y() << " " << fullScreen() << " " << maximized()
    << " " << m_handle_layout << " " << m_initial_show << " " << event -> spontaneous() << "\n";
  //kdDebugTime() << " Normal geometry " << m_normal_geometry.x() << "x" << m_normal_geometry.y()
  //  << " " << m_normal_geometry.width() << "x" << m_normal_geometry.height() << "\n";
#endif
}

void KPlayerWindow::resizeEvent (QResizeEvent* event)
{
  KXmlGuiWindow::resizeEvent (event);
  //if ( ! fullScreen() && ! m_handle_layout && ! settings() -> maximized() && ! maximized() && m_initial_show )
  //  m_normal_geometry.setSize (QSize (width(), height()));
  if ( ! event -> spontaneous() )
    m_previous_size = event -> size();
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "WiSize " << event -> oldSize().width() << "x" << event -> oldSize().height()
    << " => " << event -> size().width() << "x" << event -> size().height() << " " << fullScreen() << " " << maximized()
    << " " << m_handle_layout << " " << m_initial_show << " " << event -> spontaneous() << "\n";
  //kdDebugTime() << " Normal geometry " << m_normal_geometry.x() << "x" << m_normal_geometry.y()
  //  << " " << m_normal_geometry.width() << "x" << m_normal_geometry.height() << "\n";
#endif
  if ( engine() -> resizing() )
  {
    library() -> library() -> rememberHeight();
    log() -> logWidget() -> rememberHeight();
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << " Log height " << log() -> logWidget() -> sizeHint().height() << "\n";
    kdDebugTime() << " Library height " << library() -> library() -> sizeHint().height() << "\n";
#endif
  }
  if ( m_handle_layout )
  {
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << " Handling layout\n";
#endif
    m_handle_layout = false;
    engine() -> handleLayout();
  }
}

/*
void KPlayerWindow::setMinimumSize (int w, int h)
{
  QSize prev (size()), msh (minimumSizeHint());
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Set minimum size " << w << "x" << h << " => " << msh.width() << "x" << msh.height() << "\n";
#endif
  w = msh.width();
  h = msh.height();
  KXmlGuiWindow::setMinimumSize (w, h);
#ifdef DEBUG_KPLAYER_RESIZING
  dumpObject (this, 1);
  if ( prev != size() )
    kdDebugTime() << "             Size changed\n";
#endif
  if ( prev != size() && m_previous_size == prev )
    m_previous_size = size();
}

QSize KPlayerWindow::minimumSizeHint (void) const
{
  KPlayerWindow* that = (KPlayerWindow*) this;
  QSize size (configuration() -> minimumSliderLength(), 0);
  if ( ! that -> menuBar() -> isHidden() )
    size.setHeight (that -> menuBar() -> sizeHint().height());
  size.setHeight (size.height() + topDock() -> height());
  size.setHeight (size.height() + bottomDock() -> height());
  size.setWidth (size.width() + leftDock() -> width());
  size.setWidth (size.width() + rightDock() -> width());
  if ( ! that -> statusBar() -> isHidden() )
  {
    int x = that -> statusBar() -> minimumSizeHint().width();
    if ( x > size.width() )
      size.setWidth (x);
    size.setHeight (size.height() + that -> statusBar() -> minimumHeight());
  }
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Minimum size hint " << size.width() << "x" << size.height() << "\n";
#endif
  return size.boundedTo (availableGeometry().size());
}
*/

QSize KPlayerWindow::sizeHint (void) const
{
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "KPlayerWindow::sizeHint\n";
#endif
  QSize hint (KXmlGuiWindow::sizeHint());
  int i = 0, left = 0, right = 0;
  while ( QLayoutItem* item = layout() -> itemAt (i) )
  {
    if ( item -> widget() && ! item -> widget() -> isHidden() && item -> widget() -> inherits ("QToolBar") )
    {
      QToolBar* toolbar = (QToolBar*) item -> widget();
      if ( ! toolbar -> isFloating() )
      {
        Qt::ToolBarArea area = toolBarArea (toolbar);
        int end = toolbar -> x() + toolbar -> width();
        if ( area == Qt::LeftToolBarArea )
        {
          if ( end > left )
            left = end;
        }
        else if ( area == Qt::RightToolBarArea )
        {
          end -= centralWidget() -> x() + centralWidget() -> width();
          if ( end > right )
            right = end;
        }
        //if ( settings() -> displaySize().isEmpty() && toolbar -> orientation() == Qt::Vertical
        //    && hint.height() < toolbar -> minimumSizeHint().height() && vertical < toolbar -> sizeHint().height() )
        //  vertical = toolbar -> sizeHint().height();
      }
    }
    i ++;
  }
  i = settings() -> displaySize().width() + left + right;
  left = minimumSizeHint().width();
  hint.setWidth (i < left ? left : i);
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << " Hint   " << hint.width() << "x" << hint.height() << "\n";
#endif
  return hint;
}

void KPlayerWindow::correctSize (void)
{
  //bool maximized = isMaximized();
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::correctSize\n";
#endif
  //if ( settings() -> fullScreen() )
  //  layout() -> setSpacing (-1);
/*
  if ( ! settings() -> fullScreen() && ! maximized && settings() -> maximized() )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "KPlayerWindow::correctSize: setMaximized false\n";
#endif
    settings() -> setMaximized (maximized);
  }
  if ( settings() -> constrainedSize() )
  {
    if ( maximized && ! settings() -> maximized() )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << "KPlayerWindow::correctSize: setMaximized true\n";
#endif
      settings() -> setMaximized (maximized);
    }
    return;
  }
*/
  if ( m_previous_size != size() )
/*{
    if ( maximized )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << "KPlayerWindow::correctSize: setMaximized true\n";
#endif
      settings() -> setMaximized (maximized);
    }
    else*/
      settings() -> setDisplaySize (settings() -> adjustSize (kPlayerWorkspace() -> size(),
        m_previous_size.width() == width()));
  //}
#ifdef DEBUG_KPLAYER_WINDOW
  else
    kdDebugTime() << " Using previous size\n";
#endif
}

void KPlayerWindow::dockWidgetVisibility (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::dockWidgetVisibility\n";
#endif
  showLibrary (! library() -> isHidden());
  showMessageLog (! log() -> isHidden());
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << " Library " << showLibrary() << "\n";
  kdDebugTime() << " Log     " << showMessageLog() << "\n";
#endif
}

void KPlayerWindow::windowStateChanged (uint wid)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::windowStateChanged " << wid << "\n";
#endif
  static bool full_screen = false, maximize = false, minimize = false, shade = false;
  if ( wid == winId() )
  {
    if ( full_screen != fullScreen() || maximize != maximized() || minimize != minimized() || shade != shaded() )
      m_handle_layout = true;
    full_screen = fullScreen();
    maximize = maximized();
    minimize = minimized();
    shade = shaded();
    m_initial_show = m_initial_show || frameGeometry() != geometry();
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << " Full screen " << full_screen << "\n";
    kdDebugTime() << " Maximized   " << maximize << "\n";
    kdDebugTime() << " Minimized   " << minimize << "\n";
    kdDebugTime() << " Shaded      " << shade << "\n";
    kdDebugTime() << " Layout      " << m_handle_layout << "\n";
    kdDebugTime() << " Geometry    " << m_initial_show << "\n";
#endif
    if ( ! minimize && ! shade )
    {
      settings() -> setFullScreen (full_screen);
      if ( ! full_screen )
        settings() -> setMaximized (maximize);
    }
    engine() -> doneSyncronizing();
    /*if ( m_toggling_full_screen )
    {
      m_toggling_full_screen = false;
      if ( m_active_window )
      {
        activateWindow();
        KWindowSystem::forceActiveWindow (winId());
      }
    }*/
    //setAcceptDrops (true);
  }
}

void KPlayerWindow::syncronizeState (bool* pending)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::syncronizeState\n";
#endif
  setUpdatesEnabled (false);
  layout() -> setEnabled (false);
  bool full_screen = settings() -> fullScreen();
  bool maximize = settings() -> maximized();
  bool currently_full_screen = fullScreen();
  bool currently_maximized = maximized() && ! currently_full_screen;
  *pending = currently_full_screen != full_screen || ! full_screen && currently_maximized != maximize;
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << " Full screen " << currently_full_screen << " -> " << full_screen << "\n";
  kdDebugTime() << " Maximized " << currently_maximized << " -> " << maximize << "\n";
  kdDebugTime() << " Pending " << *pending << "\n";
#endif
  if ( *pending )
  {
#ifdef DEBUG_KPLAYER_RESIZING
    dumpObject (this);
#endif
    /*m_toggling_full_screen = true;
    m_active_window = isActiveWindow();*/
    //setAcceptDrops (false);
    //setUpdatesEnabled (false);
    if ( full_screen )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << " Showing full screen\n";
#endif
      showFullScreen();
      //layout() -> setSpacing (-1);
    }
    else if ( maximize )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << " Showing maximized\n";
#endif
      showMaximized();
    }
    else
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << " Showing normal\n";
#endif
      showNormal();
    }
  }
}

void KPlayerWindow::syncronizeControls (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::syncronizeControls\n";
  kdDebugTime() << " Menu visible " << ! menuBar() -> isHidden() << " show " << showMenubar() << "\n";
#endif
  if ( showMenubar() == menuBar() -> isHidden() )
    if ( showMenubar() )
      menuBar() -> show();
    else
      menuBar() -> hide();
  toggleAction (KStandardAction::stdName (KStandardAction::ShowMenubar)) -> setChecked (showMenubar());
  toggleAction (KStandardAction::stdName (KStandardAction::ShowMenubar)) -> setStatusTip (i18n("Shows/hides the menu bar"));
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << " Status visible " << ! statusBar() -> isHidden() << " show " << showStatusbar() << "\n";
#endif
  if ( showStatusbar() == statusBar() -> isHidden() )
    if ( showStatusbar() )
      statusBar() -> show();
    else
      statusBar() -> hide();
  toggleAction (KStandardAction::stdName (KStandardAction::ShowStatusbar)) -> setChecked (showStatusbar());
  toggleAction (KStandardAction::stdName (KStandardAction::ShowStatusbar)) -> setStatusTip (i18n("Shows/hides the status bar"));
  for ( int j = 0; j < KPLAYER_TOOLBARS; j ++ )
  {
    KToolBar* toolbar = toolBar (m_toolbar[j].name);
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << " Toolbar " << m_toolbar[j].name << " visible " << ! toolbar -> isHidden() << " show " << showToolbar (j) << "\n";
#endif
    if ( showToolbar (j) == toolbar -> isHidden() )
      if ( showToolbar (j) )
        toolbar -> show();
      else
        toolbar -> hide();
    toggleAction (m_toolbar[j].action) -> setChecked (showToolbar (j));
  }
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << " Library visible " << ! library() -> isHidden() << " shown " << showLibrary() << " show " << m_show_library << "\n";
#endif
  if ( showLibrary() == library() -> isHidden() )
    if ( showLibrary() )
      library() -> show();
    else
      library() -> hide();
  toggleAction ("options_show_library") -> setChecked (library() -> visible());
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << " Log visible " << ! log() -> isHidden() << " shown " << showMessageLog() << " show " << m_show_log << "\n";
#endif
  if ( showMessageLog() == log() -> isHidden() )
    if ( showMessageLog() )
      log() -> show();
    else
      log() -> hide();
  toggleAction ("options_show_log") -> setChecked (log() -> visible());
  int index;
  QTabBar* tabbar = findTabBar (log(), &index);
  if ( tabbar )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << " Tabbar visible " << ! tabbar -> isHidden() << " count " << tabbar -> count() << "\n";
#endif
    if ( ! tabbar -> isHidden() && tabbar -> count() <= 1 )
      tabbar -> hide();
    else if ( m_show_log )
      tabbar -> setCurrentIndex (index);
  }
  tabbar = findTabBar (library(), &index);
  if ( tabbar )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << " Tabbar visible " << ! tabbar -> isHidden() << " count " << tabbar -> count() << "\n";
#endif
    if ( ! tabbar -> isHidden() && tabbar -> count() <= 1 )
      tabbar -> hide();
    else if ( m_show_library )
      tabbar -> setCurrentIndex (index);
  }
  if ( m_show_library )
    library() -> setFocus();
  layout() -> setEnabled (true);
  layout() -> invalidate();
  layout() -> activate();
}

void KPlayerWindow::updateLayout (const QSize& size)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::updateLayout\n";
#endif
#ifdef DEBUG_KPLAYER_RESIZING
  dumpObject (this);
#endif
  int i = 0, position = 0, left = 0, right = 0;
  int space = size.height();
  Qt::ToolBarArea current_area = Qt::NoToolBarArea;
  while ( QLayoutItem* item = layout() -> itemAt (i) )
  {
    if ( item -> widget() && ! item -> widget() -> isHidden() && item -> widget() -> inherits ("QToolBar") )
    {
      QToolBar* toolbar = (QToolBar*) item -> widget();
      bool has_break = toolBarBreak (toolbar);
      Qt::ToolBarArea area = toolBarArea (toolbar);
      if ( area != Qt::NoToolBarArea && ! toolbar -> isFloating() )
      {
        bool auto_break = has_break;
        int j;
        for ( j = 0; j < KPLAYER_TOOLBARS; j ++ )
          if ( toolbar -> objectName() == m_toolbar[j].name )
          {
            auto_break = has_break && m_toolbar[j].auto_break;
            break;
          }
        int end = toolbar -> x() + toolbar -> width();
        if ( area == Qt::LeftToolBarArea )
        {
          if ( end > left )
            left = end;
        }
        else if ( area == Qt::RightToolBarArea )
        {
          end -= centralWidget() -> x() + centralWidget() -> width();
          if ( end > right )
            right = end;
        }
        QSize hint = toolbar -> sizeHint();
        int length = toolbar -> orientation() == Qt::Vertical ? hint.height() : hint.width();
        if ( area != current_area || has_break && ! auto_break )
        {
          position = length;
          if ( current_area < Qt::TopToolBarArea && area >= Qt::TopToolBarArea )
            space = (size.isEmpty() ? configuration() -> preferredVideoWidth() : size.width()) + left + right;
          current_area = area;
        }
        else
        {
          position += length;
          m_toolbar[j].auto_break = auto_break = position > space;
          if ( auto_break )
          {
            position = length;
            if ( ! has_break )
              insertToolBarBreak (toolbar);
          }
          else if ( has_break )
            removeToolBarBreak (toolbar);
        }
      }
    }
    i ++;
  }
}

void KPlayerWindow::zoom (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::zoom\n";
#endif
  const QRect available (availableGeometry());
  QRect frame (KWindowSystem::windowInfo (winId(), NET::WMFrameExtents).frameGeometry());
  QPoint offset (pos() - frame.topLeft());
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << " Frame  " << frame.x() << "x" << frame.y() << " " << frame.width() << "x" << frame.height() << "\n";
  kdDebugTime() << " Frame  " << frameGeometry().x() << "x" << frameGeometry().y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
  kdDebugTime() << " Offset " << offset.x() << "x" << offset.y() << "\n";
  dumpObject (this);
#endif
  QSize hint (sizeHint());
  QSize asize (available.size() - QSize (1, 1));
  QSize fsize (frame.size() - size() + hint);
  int library_height = library() -> widget() -> sizeHint().height();
  int log_height = log() -> widget() -> sizeHint().height();
  if ( fsize.width() > asize.width() || fsize.height() > asize.height() )
  {
    QSize size (settings() -> displaySize());
    if ( fsize.width() > asize.width() )
      size = settings() -> constrainSize (QSize (size.width() + asize.width() - fsize.width(), size.height()));
    int excess = fsize.height() - asize.height();
    if ( excess > 0 )
    {
      int separator = style() -> pixelMetric (QStyle::PM_DockWidgetSeparatorExtent);
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << " Excess " << excess << " separator " << separator << "\n";
#endif
      QTabBar* tabbar = findTabBar (log());
      if ( tabbar && tabbar == findTabBar (library()) && library() -> docked() && log() -> docked() )
      {
        int log_reserve = log() -> sizeHint().height();
        int library_reserve = library() -> sizeHint().height();
        int full_reserve = log_reserve > library_reserve ? log_reserve : library_reserve;
        log_reserve = log() -> minimumSizeHint().height();
        library_reserve = library() -> minimumSizeHint().height();
        int reserve = full_reserve - (log_reserve > library_reserve ? log_reserve : library_reserve);
#ifdef DEBUG_KPLAYER_RESIZING
        kdDebugTime() << " Reserve " << reserve << " full " << full_reserve << "\n";
#endif
        if ( excess > reserve )
        {
          excess -= full_reserve + tabbar -> sizeHint().height() + separator;
          if ( m_show_log )
            log() -> setFloating (true);
          else
            log() -> hide();
          if ( m_show_library )
            library() -> setFloating (true);
          else
            library() -> hide();
          tabbar -> hide();
#ifdef DEBUG_KPLAYER_RESIZING
          kdDebugTime() << " Excess " << excess << "\n";
#endif
        }
        else if ( reserve > 0 )
        {
          if ( reserve > excess )
            reserve = excess;
          excess -= reserve;
          library_height = log_height = full_reserve - reserve;
#ifdef DEBUG_KPLAYER_RESIZING
          kdDebugTime() << " Excess " << excess << "\n";
#endif
        }
      }
      else
      {
        int log_reserve = log() -> sizeHint().height() - log() -> minimumSizeHint().height();
        if ( log_reserve < 0 )
          log_reserve = 0;
        int library_reserve = library() -> sizeHint().height() - library() -> minimumSizeHint().height();
        if ( library_reserve < 0 )
          library_reserve = 0;
#ifdef DEBUG_KPLAYER_RESIZING
        kdDebugTime() << " Library docked " << library() -> docked() << " reserve " << library_reserve << " show " << m_show_library << "\n";
        kdDebugTime() << " Log     docked " << log() -> docked() << " reserve " << log_reserve << " show " << m_show_log << "\n";
#endif
        if ( log() -> docked() && excess > log_reserve + library_reserve )
        {
          excess -= log() -> sizeHint().height() + separator;
          if ( m_show_log )
            log() -> setFloating (true);
          else
            log() -> hide();
#ifdef DEBUG_KPLAYER_RESIZING
          kdDebugTime() << " Excess " << excess << "\n";
#endif
        }
        else if ( log() -> docked() && log_reserve > 0 )
        {
          if ( log_reserve > excess )
            log_reserve = excess;
          excess -= log_reserve;
          log_height = log() -> widget() -> sizeHint().height() - log_reserve;
#ifdef DEBUG_KPLAYER_RESIZING
          kdDebugTime() << " Excess " << excess << "\n";
#endif
        }
        if ( excess > 0 && library() -> docked() && excess > library_reserve )
        {
          excess -= library() -> sizeHint().height() + separator;
          if ( m_show_library )
            library() -> setFloating (true);
          else
            library() -> hide();
#ifdef DEBUG_KPLAYER_RESIZING
          kdDebugTime() << " Excess " << excess << "\n";
#endif
        }
        else if ( excess > 0 && library() -> docked() && library_reserve > 0 )
        {
          if ( library_reserve > excess )
            library_reserve = excess;
          excess -= library_reserve;
          library_height = library() -> widget() -> sizeHint().height() - library_reserve;
#ifdef DEBUG_KPLAYER_RESIZING
          kdDebugTime() << " Excess " << excess << "\n";
#endif
        }
      }
      if ( excess > 0 )
        size = settings() -> constrainSize (QSize (size.width(), size.height() - excess));
    }
    hint -= settings() -> displaySize() - size + QSize (0, fsize.height() - asize.height() - excess);
    settings() -> setDisplaySize (size);
  }
  frame.setSize (frame.size() - size() + hint);
  QPoint position (configuration() -> preferredWindowPosition());
  if ( position.x() < 0 && position.y() < 0 )
    position = frame.topLeft();
  if ( position.x() < available.left() || frame.width() >= available.width() )
    position.setX (available.left());
  else if ( position.x() + frame.width() > available.right() )
    position.setX (available.right() - frame.width());
  if ( position.y() < available.top() || frame.height() >= available.height() )
    position.setY (available.top());
  else if ( position.y() + frame.height() > available.bottom() )
    position.setY (available.bottom() - frame.height());
  if ( pos() != position + offset )
  {
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << "Moving to " << position.x() << "x" << position.y() << " " << (position.x() + offset.x()) << "x" << (position.y() + offset.y()) << "\n";
#endif
    move (position + offset);
  }
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << " Resize " << hint.width() << "x" << hint.height() << "\n";
#endif
  resize (hint);
  kPlayerWorkspace() -> setFixedHeight (settings() -> displaySize().height());
  if ( library() -> docked() )
  {
    library() -> widget() -> setFixedHeight (library_height);
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << " Library height " << library() -> library() -> minimumSize().height() << "\n";
#endif
  }
  if ( log() -> docked() )
  {
    log() -> widget() -> setFixedHeight (log_height);
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << " Log height " << log() -> logWidget() -> minimumSize().height() << "\n";
#endif
  }
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Zoom done\n";
#endif
}

void KPlayerWindow::finalizeLayout (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::finalizeLayout\n";
#endif
  layout() -> invalidate();
  layout() -> activate();
  setUpdatesEnabled (true);
  kPlayerWorkspace() -> setMinimumSize (QSize (0, 0));
  kPlayerWorkspace() -> setMaximumSize (QSize (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  log() -> widget() -> setMinimumSize (QSize (0, 0));
  log() -> widget() -> setMaximumSize (QSize (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  library() -> widget() -> setMinimumSize (QSize (0, 0));
  library() -> widget() -> setMaximumSize (QSize (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
  m_show_library = m_show_log = false;
#ifdef DEBUG_KPLAYER_RESIZING
  dumpObject (this);
#endif
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Layout done\n";
#endif
}

QRect KPlayerWindow::availableGeometry (void) const
{
  QRect rect;
  if ( QApplication::desktop() -> isVirtualDesktop() )
    for ( int i = 0; i < QApplication::desktop() -> numScreens(); i ++ )
    {
      rect |= QApplication::desktop() -> availableGeometry (i);
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << "Screen " << i << "  " << QApplication::desktop() -> screenGeometry (i).x()
        << "x" << QApplication::desktop() -> screenGeometry (i).y()
        << " " << QApplication::desktop() -> screenGeometry (i).width()
        << "x" << QApplication::desktop() -> screenGeometry (i).height() << "\n";
      kdDebugTime() << "Available " << QApplication::desktop() -> availableGeometry (i).x()
        << "x" << QApplication::desktop() -> availableGeometry (i).y()
        << " " << QApplication::desktop() -> availableGeometry (i).width()
        << "x" << QApplication::desktop() -> availableGeometry (i).height() << "\n";
#endif
    }
  else
  {
    rect = QApplication::desktop() -> availableGeometry();
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << "Screen " << QApplication::desktop() -> screenGeometry().x()
        << "x" << QApplication::desktop() -> screenGeometry().y()
        << " " << QApplication::desktop() -> screenGeometry().width()
        << "x" << QApplication::desktop() -> screenGeometry().height() << "\n";
#endif
  }
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Desktop " << QApplication::desktop() -> width() << "x" << QApplication::desktop() -> height() << "\n";
  kdDebugTime() << "Total available geometry " << rect.width() << "x" << rect.height() << "\n";
#endif
  return rect;
}
