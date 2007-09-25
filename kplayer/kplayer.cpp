/***************************************************************************
                          kplayer.cpp
                          -----------
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
#include <qevent.h>
#include <qeventloop.h>
#include <qlabel.h>
#include <qobject.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_WINDOW
//#define DEBUG_KPLAYER_RESIZING
//#define DEBUG_KPLAYER_SIZING_HACKS
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
//void KPlayerX11SetInputFocus (uint id);
void KPlayerX11DiscardConfigureEvents (uint id);
void KPlayerX11GetKeyboardMouseState (uint id);
//void KPlayerX11SendConfigureEvent (uint id, int x, int y, int w, int h);

#define ID_STATUS_MSG   1
#define ID_STATE_MSG    2
#define ID_PROGRESS_MSG 3

QString resourcePath (const QString& filename);
extern KAboutData* about;

KPlayerApplication::KPlayerApplication (void)
  : KUniqueApplication (true, true)
{
}

KPlayerApplication::~KPlayerApplication()
{
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
          << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
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
          << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
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
          << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
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
            << " spontaneous " << keyevent.spontaneous() << " receiver " << (object ? object -> className() : "<none>")
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
      kPlayerSettings() -> setControl (((QMouseEvent*) event) -> modifiers() & Qt::ControlModifier);
      kPlayerSettings() -> setShift (((QMouseEvent*) event) -> modifiers() & Qt::ShiftModifier);
#ifdef DEBUG_KPLAYER_NOTIFY_MOUSE
      kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
        << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
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
        kPlayerSettings() -> setControl (mevent -> modifiers() & Qt::ControlModifier);
        kPlayerSettings() -> setShift (mevent -> modifiers() & Qt::ShiftModifier);
#ifdef DEBUG_KPLAYER_NOTIFY_MOUSE
        kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
          << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
          << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
      }
      break;
    case QEvent::Wheel:
      kPlayerSettings() -> setControl (((QWheelEvent*) event) -> modifiers() & Qt::ControlModifier);
      kPlayerSettings() -> setShift (((QWheelEvent*) event) -> modifiers() & Qt::ShiftModifier);
#ifdef DEBUG_KPLAYER_NOTIFY_WHEEL
      kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
        << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
        << " control " << kPlayerSettings() -> control() << " shift " << kPlayerSettings() -> shift() << "\n";
#endif
      break;
#ifdef DEBUG_KPLAYER_NOTIFY_MENU
    case QEvent::ContextMenu:
      //kPlayerSettings() -> setControl (((QContextMenuEvent*) event) -> modifiers() & Qt::ControlModifier);
      //kPlayerSettings() -> setShift (((QContextMenuEvent*) event) -> modifiers() & Qt::ShiftModifier);
      kdDebugTime() << "KPlayerApplication::notify: event type " << event -> type()
        << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
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
          << " spontaneous " << event -> spontaneous() << " receiver " << (object ? object -> className() : "<none>")
          << " accepted " << devent -> isAccepted() << " action " << devent -> action()
          << " accepted " << devent -> isActionAccepted() << "\n";
      }
      break;
#endif
#ifdef DEBUG_KPLAYER_NOTIFY_DRAG
    case QEvent::DragResponse:
      kdDebugTime() << "KPlayerApplication::notify: event type DragResponse spontaneous " << event -> spontaneous()
        << " receiver " << (object ? object -> className() : "<none>")
        << " accepted " << ((QDragResponseEvent*) event) -> dragAccepted() << "\n";
      break;
    case QEvent::DragLeave:
      kdDebugTime() << "KPlayerApplication::notify: event type DragLeave spontaneous " << event -> spontaneous()
        << " receiver " << (object ? object -> className() : "<none>") << "\n";
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

/*int KPlayerApplication::x11ClientMessage (QWidget* widget, XEvent* event, bool passive_only)
{
  KApplication::x11ClientMessage (widget, event, passive_only);
}*/

#ifdef DEBUG_KPLAYER_RESIZING
void dumpObject (const QObject* object, int indent, int depth = 20)
{
  QString spaces;
  for ( int i = 0; i < indent * 2; i ++ )
    spaces += " ";
  if ( object -> inherits ("QWidget") )
  {
    QWidget* widget = (QWidget*) object;
    kdDebugTime() << spaces << object -> className() << " " << object -> name ("<unnamed>") <<
      " " << widget -> winId() <<
      " " << widget -> minimumSize().width() << "x" << widget -> minimumSize().height() <<
      " " << widget -> minimumSizeHint().width() << "x" << widget -> minimumSizeHint().height() <<
      " " << widget -> sizeHint().width() << "x" << widget -> sizeHint().height() <<
      " " << widget -> geometry().x() << "x" << widget -> geometry().y() <<
      " " << widget -> geometry().width() << "x" << widget -> geometry().height() <<
      " " << widget -> frameGeometry().x() << "x" << widget -> frameGeometry().y() <<
      " " << widget -> frameGeometry().width() << "x" << widget -> frameGeometry().height() << "\n";
  }
  else if ( object -> inherits ("QLayout") )
  {
    QLayout* layout = (QLayout*) object;
    kdDebugTime() << spaces << object -> className() << " " << object -> name ("<unnamed>") <<
      " " << layout -> minimumSize().width() << "x" << layout -> minimumSize().height() <<
      " " << layout -> sizeHint().width() << "x" << layout -> sizeHint().height() <<
      " " << layout -> geometry().width() << "x" << layout -> geometry().height() <<
      " " << layout -> margin() << "+" << layout -> spacing() <<
      " " << layout -> alignment() << " " << layout -> expanding() <<
      " " << layout -> hasHeightForWidth() << " " << layout -> heightForWidth (1) << "\n";
  }
  else
    kdDebugTime() << spaces << object -> className() << " " << object -> name ("<unnamed>") << "\n";
  indent ++;
  QObjectList* objectlist = (QObjectList*) object -> children();
  if ( objectlist && depth -- > 0 )
    for ( object = objectlist -> first(); object; object = objectlist -> next() )
      if ( object )
        dumpObject (object, indent, depth);
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
  m_set_display_size = m_initial_show = m_error_detected = m_maximized = false;
  m_full_screen = m_show_log = m_show_library = false;
  Toolbar toolbars [KPLAYER_TOOLBARS] = {
    { "mainToolBar", KStandardAction::stdName (KStandardAction::ShowToolbar), true, false },
    { "playlistToolBar", "options_show_playlist", true, false },
    { "progressToolBar", "settings_progress_toolbar", true, false },
    { "volumeToolBar", "settings_volume_toolbar", false, false },
    { "contrastToolBar", "settings_contrast_toolbar", false, false },
    { "brightnessToolBar", "settings_brightness_toolbar", false, false },
    { "hueToolBar", "settings_hue_toolbar", false, false },
    { "saturationToolBar", "settings_saturation_toolbar", false, false }
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
    i18n("Main toolbar contains buttons for commonly used operations. Left clicking an icon will activate the corresponding action. Some of the buttons will pop up slider controls when clicked. The sliders let you change various player controls: sound volume, video contrast, brightness, hue and saturation. The video controls will only be available for video files."),
    i18n("Playlist toolbar shows the multimedia title currently loaded or being played, offers commands to go to the next or previous items on the playlist, and also lets you see the entire playlist that consists of items that are currently being played or have been played recently. If you select a different item from the list, KPlayer will load and start playing it. The toolbar also contains options to loop and shuffle the playlist."),
    i18n("Progress and seeking toolbar is shown when the time length of a media file is known. It displays the playback progress and also allows seeking, that is, moving the playback point back and forth within the file. To seek to a specific position, drag the slider thumb to that position with the left mouse button, or simply click at that position with the middle mouse button. To move forward or backward in steps, left click the slider to the left or right of the thumb, or click the Forward and Backward buttons."),
    i18n("Volume toolbar has a volume slider that shows the current sound volume and allows you to change it. This is the same slider you get from the volume pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Volume Toolbar option on the Settings menu. Clicking the Volume button on this toolbar will also hide it. In addition the toolbar has a Mute button that turns the sound off and back on."),
    i18n("Contrast toolbar has a contrast slider that shows the current video contrast and allows you to change it. This is the same slider you get from the contrast pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Contrast Toolbar option on the Settings menu. Clicking the Contrast button on this toolbar will also hide it."),
    i18n("Brightness toolbar has a brightness slider that shows the current video brightness and allows you to change it. This is the same slider you get from the brightness pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Brightness Toolbar option on the Settings menu. Clicking the Brightness button on this toolbar will also hide it."),
    i18n("Hue toolbar has a hue slider that shows the current video hue and allows you to change it. This is the same slider you get from the hue pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Hue Toolbar option on the Settings menu. Clicking the Hue button on this toolbar will also hide it."),
    i18n("Saturation toolbar has a saturation slider that shows the current video saturation and allows you to change it. This is the same slider you get from the saturation pop-up slider button on the main toolbar, but it will not disappear when you click elsewhere. You can show or hide the toolbar using the Show Saturation Toolbar option on the Settings menu. Clicking the Saturation button on this toolbar will also hide it.")
  };
  KPlayerEngine::initialize (actionCollection(), this, KGlobal::config().data());
  connect (engine(), SIGNAL (windowStateChanged (uint)), SLOT (windowStateChanged (uint)));
  connect (engine(), SIGNAL (syncronize (bool)), SLOT (syncronize (bool)));
  connect (engine(), SIGNAL (zoom()), SLOT (zoom()));
  connect (engine(), SIGNAL (correctSize()), SLOT (correctSize()));
  connect (engine(), SIGNAL (initialSize()), SLOT (initialSize()));
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
  connect (log(), SIGNAL (windowHidden()), SLOT (logWindowHidden()));
  connect (action ("log_clear"), SIGNAL (activated()), SLOT (fileClearLog()));
  m_playlist = new KPlayerPlaylist (actionCollection(), this);
  m_library = new KPlayerLibraryWindow (actionCollection(), playlist(), this);
  connect (library(), SIGNAL (windowHidden()), SLOT (libraryHidden()));
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
  setAutoSaveSettings ("MainWindow", false);
  resetAutoSaveSettings();  // saveWindowSize = false;
  readOptions();
  KToolBar* toolbar = toolBar (m_toolbar[PLAYLIST_TOOLBAR].name);
  toolbar -> setAllowedAreas (Qt::TopToolBarArea | Qt::BottomToolBarArea);
  library() -> setAllowedAreas (Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
  log() -> setAllowedAreas (Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
  menuBar() -> setWhatsThis (i18n("Menu bar contains names of drop-down menus. Left click a name to alternately show and hide that menu, or use Alt + the underlined letter in the name as a hot key, for example Alt+F to show the File menu."));
  statusBar() -> setWhatsThis (i18n("Status bar shows general information about the player status and progress."));
  library() -> setWhatsThis (i18n("Multimedia library is a window where that lets you organize your files, streams, devices, manage your playlists, and choose items for playing. It shows various information about your media files and allows you to search and group them and change their properties."));
  log() -> setWhatsThis (i18n("Message log is a window where KPlayer shows messages it receives from MPlayer. KPlayer can show it automatically when it detects an MPlayer error if that option is selected in KPlayer settings."));
  for ( i = 0; i < KPLAYER_TOOLBARS; i ++ )
  {
    toolbar = toolBar (m_toolbar[i].name);
    toolbar -> setWindowTitle (captions[i]);
    toolbar -> setWhatsThis (whatsthis[i]);
    //kdDebugTime() << "Orientation " << sliderAction (m_toolbar[i].action) -> slider() -> orientation() << "\n";
    if ( i >= FIRST_SLIDER_TOOLBAR )
      ((KPlayerSliderAction*) action (actions [i - FIRST_SLIDER_TOOLBAR])) -> slider() -> setOrientation (toolbar -> orientation());
  }
  if ( settings() -> maximized() )
    showMaximized();
  enablePlayerActions();
  enableVideoActions();
  //setEraseColor (QColor (0, 0, 0));
  setAcceptDrops (true);
  //layout() -> setResizeMode (QLayout::FreeResize);
  //KWindowSystem::setState (log() -> winId(), NET::SkipTaskbar | NET::SkipPager);
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
  action -> setWhatsThis (i18n("Quit command saves the playlist and all settings, stops playing and closes KPlayer."));

  action = KStandardAction::showMenubar (this, SLOT (viewMenubar()), actionCollection());
  action -> setStatusTip (i18n("Shows/hides the menu bar"));
  action -> setWhatsThis (i18n("Show Menubar command shows or hides the menu bar."));

  action = KStandardAction::showStatusbar (this, SLOT (viewStatusbar()), actionCollection());
  action -> setStatusTip (i18n("Shows/hides the status bar"));
  action -> setWhatsThis (i18n("Show Statusbar command shows or hides the status bar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_playlist", action);
  connect (action, SIGNAL (triggered()), SLOT (viewPlaylist()));
  action -> setText (i18n("&Show Playlist"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_P);
  action -> setStatusTip (i18n("Shows/hides the playlist"));
  action -> setWhatsThis (i18n("Show Playlist command shows or hides the playlist toolbar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_library", action);
  connect (action, SIGNAL (triggered()), SLOT (viewLibrary()));
  action -> setText (i18n("Sho&w Library"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_L);
  action -> setStatusTip (i18n("Shows/hides the multimedia library"));
  action -> setWhatsThis (i18n("Show Library command shows or hides the multimedia library."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_log", action);
  connect (action, SIGNAL (triggered()), SLOT (viewMessageLog()));
  action -> setText (i18n("Show Message &Log"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_G);
  action -> setStatusTip (i18n("Shows/hides the message log"));
  action -> setWhatsThis (i18n("Show Message Log command shows or hides the message log."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("options_show_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewMainToolbar()));
  action -> setText (i18n("Show Main &Toolbar"));
  action -> setStatusTip (i18n("Shows/hides the main toolbar"));
  action -> setWhatsThis (i18n("Show Main Toolbar command shows or hides the main toolbar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_progress_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewProgressToolbar()));
  action -> setText (i18n("Show &Progress Toolbar"));
  action -> setStatusTip (i18n("Shows/hides the progress toolbar"));
  action -> setWhatsThis (i18n("Show Progress Toolbar command shows or hides the progress toolbar. This command is available when the time length of the current file is known."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_volume_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewVolumeToolbar()));
  action -> setText (i18n("Show &Volume Toolbar"));
  action -> setIcon (KIcon ("volume"));
  action -> setStatusTip (i18n("Shows/hides the volume toolbar"));
  action -> setWhatsThis (i18n("Show Volume Toolbar command shows or hides the volume toolbar."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_contrast_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewContrastToolbar()));
  action -> setText (i18n("Show C&ontrast Toolbar"));
  action -> setIcon (KIcon ("contrast"));
  action -> setStatusTip (i18n("Shows/hides the contrast toolbar"));
  action -> setWhatsThis (i18n("Show Contrast Toolbar command shows or hides the contrast toolbar. This command is available for video files."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_brightness_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewBrightnessToolbar()));
  action -> setText (i18n("Show &Brightness Toolbar"));
  action -> setIcon (KIcon ("brightness"));
  action -> setStatusTip (i18n("Shows/hides the brightness toolbar"));
  action -> setWhatsThis (i18n("Show Brightness Toolbar command shows or hides the brightness toolbar. This command is available for video files."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_hue_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewHueToolbar()));
  action -> setText (i18n("Show H&ue Toolbar"));
  action -> setIcon (KIcon ("hue"));
  action -> setStatusTip (i18n("Shows/hides the hue toolbar"));
  action -> setWhatsThis (i18n("Show Hue Toolbar command shows or hides the hue toolbar. This command is available for video files."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("settings_saturation_toolbar", action);
  connect (action, SIGNAL (triggered()), SLOT (viewSaturationToolbar()));
  action -> setText (i18n("Show &Saturation Toolbar"));
  action -> setIcon (KIcon ("saturation"));
  action -> setStatusTip (i18n("Shows/hides the saturation toolbar"));
  action -> setWhatsThis (i18n("Show Saturation Toolbar command shows or hides the saturation toolbar. This command is available for video files."));

  action = KStandardAction::keyBindings (this, SLOT (settingsKeyBindings()), actionCollection());
  action -> setStatusTip (i18n("Opens the KPlayer key bindings dialog"));
  action -> setWhatsThis (i18n("Configure Shortcuts command opens a dialog that lets you see and change KPlayer's shortcut key bindings, or associations between actions and the corresponding keys or combinations of keys that activate them. If you change the bindings, make sure not to duplicate an existing shortcut and also not to use the Shift key for your new shortcuts, because the Shift key has a special function in KPlayer."));

  action = KStandardAction::configureToolbars (this, SLOT (settingsConfigureToolbars()), actionCollection());
  action -> setStatusTip (i18n("Opens the KPlayer toolbar configuration dialog"));
  action -> setWhatsThis (i18n("Configure Toolbars command opens a dialog that lets you see and change KPlayer's toolbars and controls assigned to them."));

  action = KStandardAction::preferences (this, SLOT (settingsConfigure()), actionCollection());
  action -> setStatusTip (i18n("Opens the KPlayer configuration dialog"));
  action -> setWhatsThis (i18n("Configure KPlayer command opens a dialog that lets you configure the program, modifying various aspects of its functionality, user interface and interaction with MPlayer. For more information see the Configuration dialog chapter and the Advanced configuration micro-HOWTO in the user manual."));

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
    disconnect (action, SIGNAL(activated()), 0, 0);
    connect (action, SIGNAL(activated()), SLOT(helpReportBug()));
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
  statusBar() -> insertItem (i18n("Ready"), ID_STATUS_MSG, 1);
  statusBar() -> insertItem (i18n("Idle"), ID_STATE_MSG, 1);
  statusBar() -> insertItem ("", ID_PROGRESS_MSG, 1);
  QObjectList children (statusBar() -> children());
  for ( QObjectList::ConstIterator iterator (children.constBegin()); iterator != children.constEnd(); ++ iterator )
  {
    QObject* child = *iterator;
    if ( child -> inherits ("QLabel") )
    {
      QLabel* label = (QLabel*) child;
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
    connect (m_status_label, SIGNAL (itemPressed (int)), this, SLOT (statusPressed()));
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
//activateLayout();
  if ( ! settings() -> properties() -> url().isEmpty() )
    setCaption (settings() -> properties() -> caption());
  enableVideoActions();
}

void KPlayerWindow::saveOptions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Saving options\n";
#endif
  config() -> deleteGroup ("General Options");
  saveMainWindowSettings (config() -> group ("General Options"));
  /*
  config() -> setGroup ("General Options");
  Qt::Dock dock = DockTornOff;
  int index = 0, offset = 0;
  bool newline = true;
  bool docked = getLocation (log(), dock, index, newline, offset) &&
    (dock == DockTop || dock == DockBottom || dock == DockRight || dock == DockLeft);
  if ( docked )
  {
    config() -> writeEntry ("Message Log Dock", dock);
    config() -> writeEntry ("Message Log Index", index);
    config() -> writeEntry ("Message Log New Line", newline);
    config() -> writeEntry ("Message Log Offset", offset);
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "Log dock " << dock << " " << index << " " << newline << " " << offset << " " << log() -> x() << "x" << log() -> y() << " " << log() -> width() << "x" << log() -> height() << "\n";
#endif
  }
  config() -> writeEntry ("Message Log Left", log() -> x());
  config() -> writeEntry ("Message Log Top", log() -> y());
  config() -> writeEntry ("Message Log Width", log() -> width());
  config() -> writeEntry ("Message Log Height", log() -> height());
  config() -> writeEntry ("Message Log Docked", docked);
  dock = DockTornOff;
  index = offset = 0;
  newline = true;
  docked = getLocation (library(), dock, index, newline, offset) &&
    (dock == DockTop || dock == DockBottom || dock == DockRight || dock == DockLeft);
  if ( docked )
  {
    config() -> writeEntry ("Playlist Dock", dock);
    config() -> writeEntry ("Playlist Index", index);
    config() -> writeEntry ("Playlist New Line", newline);
    config() -> writeEntry ("Playlist Offset", offset);
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "Library dock " << dock << " " << index << " " << newline << " " << offset << " " << library() -> x() << "x" << library() -> y() << " " << library() -> width() << "x" << library() -> height() << "\n";
#endif
  }
  config() -> writeEntry ("Playlist Left", library() -> x());
  config() -> writeEntry ("Playlist Top", library() -> y());
  config() -> writeEntry ("Playlist Width", library() -> width());
  config() -> writeEntry ("Playlist Height", library() -> height());
  config() -> writeEntry ("Playlist Docked", docked);
  if ( ! m_full_screen && ! isMaximized() )
  {
    config() -> writeEntry ("Main Window Left", x());
    config() -> writeEntry ("Main Window Top", y());
    config() -> writeEntry ("Main Window Width", width());
    config() -> writeEntry ("Main Window Height", height());
  }
  else
  {
    config() -> writeEntry ("Main Window Left", m_normal_geometry.x());
    config() -> writeEntry ("Main Window Top", m_normal_geometry.y());
    config() -> writeEntry ("Main Window Width", m_normal_geometry.width());
    config() -> writeEntry ("Main Window Height", m_normal_geometry.height());
  }
  config() -> writeEntry ("Menu Bar Normally Visible", m_menubar_normally_visible);
  config() -> writeEntry ("Menu Bar FullScreen Visible", m_menubar_fullscreen_visible);
  config() -> writeEntry ("Status Bar Normally Visible", m_statusbar_normally_visible);
  config() -> writeEntry ("Status Bar FullScreen Visible", m_statusbar_fullscreen_visible);
  config() -> writeEntry ("Message Log Normally Visible", m_messagelog_normally_visible);
  config() -> writeEntry ("Message Log FullScreen Visible", m_messagelog_fullscreen_visible);
  config() -> writeEntry ("Playlist Editor Normally Visible", m_library_normally_visible);
  config() -> writeEntry ("Playlist Editor FullScreen Visible", m_library_fullscreen_visible);
  for ( int i = 0; i < KPLAYER_TOOLBARS; i ++ )
  {
    config() -> writeEntry (QString (m_toolbar[i].name) + " Normally Visible", m_toolbar[i].normally_visible);
    config() -> writeEntry (QString (m_toolbar[i].name) + " FullScreen Visible", m_toolbar[i].fullscreen_visible);
  }
  */
}

void KPlayerWindow::readOptions (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Reading options\n";
#endif
  applyMainWindowSettings (config() -> group ("General Options"));
  /*
  config() -> setGroup ("General Options");
  QRect available (availableGeometry());
  int width = config() -> readNumEntry ("Main Window Width", 500);
  if ( width > available.width() )
    width = available.width();
  int height = config() -> readNumEntry ("Main Window Height", 350);
  if ( height > available.height() )
    height = available.height();
  int x = config() -> readNumEntry ("Main Window Left", -1);
  if ( x + width > available.width() )
    x = available.width() - width;
  int y = config() -> readNumEntry ("Main Window Top", -1);
  if ( y + height > available.height() )
    y = available.height() - height;
  if ( x >= 0 && y >= 0 )
    move (x, y);
  if ( width >= minimumWidth() && height >= minimumHeight() )
    resize (width, height);
  m_normal_geometry.setRect (x, y, width, height);
  Qt::Dock dock = (Qt::ToolBarDock) config() -> readNumEntry ("Message Log Dock", Qt::DockBottom);
  bool docked = config() -> readBoolEntry ("Message Log Docked", true);
  bool newline = config() -> readBoolEntry ("Message Log New Line", true);
  int index = config() -> readNumEntry ("Message Log Index", 0);
  int offset = config() -> readNumEntry ("Message Log Offset", 0);
  int left = config() -> readNumEntry ("Message Log Left", 0);
  int top = config() -> readNumEntry ("Message Log Top", 0);
  width = config() -> readNumEntry ("Message Log Width", 0);
  height = config() -> readNumEntry ("Message Log Height", 0);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Log dock " << dock << " " << index << " " << newline << " " << offset << " " << left << "x" << top << " " << width << "x" << height << "\n";
#endif
  if ( width > 0 && height > 0 )
  {
    log() -> setFixedExtentWidth (width);
    log() -> setFixedExtentHeight (height);
  }
  moveDockWindow (log(), dock, newline, index, offset);
  if ( ! docked )
    log() -> undock();
  if ( width > 0 && height > 0 )
    log() -> setGeometry (left, top, width, height);
  dock = (Qt::ToolBarDock) config() -> readNumEntry ("Playlist Dock", Qt::DockBottom);
  docked = config() -> readBoolEntry ("Playlist Docked", true);
  newline = config() -> readBoolEntry ("Playlist New Line", true);
  index = config() -> readNumEntry ("Playlist Index", 0);
  offset = config() -> readNumEntry ("Playlist Offset", 0);
  left = config() -> readNumEntry ("Playlist Left", 0);
  top = config() -> readNumEntry ("Playlist Top", 0);
  width = config() -> readNumEntry ("Playlist Width", 600);
  height = config() -> readNumEntry ("Playlist Height", 300);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Library dock " << dock << " " << index << " " << newline << " " << offset << " " << left << "x" << top << " " << width << "x" << height << "\n";
#endif
  if ( width > 0 && height > 0 )
  {
    library() -> setFixedExtentWidth (width);
    library() -> setFixedExtentHeight (height);
  }
  moveDockWindow (library(), dock, newline, index, offset);
  if ( ! docked )
    library() -> undock();
  if ( width > 0 && height > 0 )
    library() -> setGeometry (left, top, width, height);
  m_menubar_normally_visible = config() -> readBoolEntry ("Menu Bar Normally Visible", m_menubar_normally_visible);
  m_menubar_fullscreen_visible = config() -> readBoolEntry ("Menu Bar FullScreen Visible", m_menubar_fullscreen_visible);
  showMenubar();
  m_statusbar_normally_visible = config() -> readBoolEntry ("Status Bar Normally Visible", m_statusbar_normally_visible);
  m_statusbar_fullscreen_visible = config() -> readBoolEntry ("Status Bar FullScreen Visible", m_statusbar_fullscreen_visible);
  showStatusbar();
  for ( int i = 0; i < KPLAYER_TOOLBARS; i ++ )
  {
    m_toolbar[i].normally_visible = config() -> readBoolEntry (QString (m_toolbar[i].name) + " Normally Visible", m_toolbar[i].normally_visible);
    m_toolbar[i].fullscreen_visible = config() -> readBoolEntry (QString (m_toolbar[i].name) + " FullScreen Visible", m_toolbar[i].fullscreen_visible);
    //kdDebugTime() << "Show " << m_toolbar[i].name << " " << m_toolbar[i].action << "\n";
    showToolbar (i);
  }
  m_messagelog_normally_visible = config() -> readBoolEntry ("Message Log Normally Visible", m_messagelog_normally_visible);
  m_messagelog_fullscreen_visible = config() -> readBoolEntry ("Message Log FullScreen Visible", m_messagelog_fullscreen_visible);
  showMessageLog();
  m_library_normally_visible = config() -> readBoolEntry ("Playlist Editor Normally Visible", m_library_normally_visible);
  m_library_fullscreen_visible = config() -> readBoolEntry ("Playlist Editor FullScreen Visible", m_library_fullscreen_visible);
  showLibrary();
  */
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Done reading options\n";
#endif
}

/*void KPlayerWindow::saveProperties (KConfig* config) const
{
  config -> writeEntry ("File URL", settings() -> url().url());
}

void KPlayerWindow::readProperties (KConfig* config)
{
  library() -> load (config -> readEntry ("File URL"), false);
}*/

void KPlayerWindow::enableToolbar (int index)
{
  Toolbar& toolbar = m_toolbar [index];
  KToggleAction* action = toggleAction (toolbar.action);
//if ( action -> isEnabled() )
//  return;
  action -> setEnabled (true);
  showToolbar (index);
}

void KPlayerWindow::disableToolbar (int index)
{
  Toolbar& toolbar = m_toolbar [index];
  KToggleAction* action = toggleAction (toolbar.action);
//if ( ! action -> isEnabled() )
//  return;
  if ( action -> isChecked() )
  {
    action -> setChecked (false);
    toolBar (toolbar.name) -> hide();
  }
  action -> setEnabled (false);
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

/*
QMenu* KPlayerWindow::popupMenu (int index)
{
  QMenuData* data = menuBar();
  if ( ! data )
    return 0;
  int id = data -> idAt (index);
  if ( id == -1 )
    return 0;
  QMenuItem* item = data -> findItem (id);
  if ( ! item )
    return 0;
  return item -> popup();
}
*/

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
  enableSubmenu (name, has_actions && library() -> isVisible());
}

/*
void KPlayerWindow::enableSubmenu (QMenuData* data, const QString& name, bool enable)
{
  for ( uint i = 0; i < data -> count(); i ++ )
  {
    int id = data -> idAt (i);
    if ( id != -1 )
    {
      QMenuItem* item = data -> findItem (id);
      if ( item )
      {
        QMenu* popup = item -> popup();
        if ( popup )
        {
          if ( popup -> name() == name )
            data -> setItemEnabled (id, enable && popup -> count() > 0);
          else
            enableSubmenu (popup, name, enable);
        }
      }
    }
  }
}
*/

void KPlayerWindow::enableSubmenu (const QString& name, bool enable)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::enableSubmenu " << name << " " << enable << "\n";
#endif
  /*
  enableSubmenu (menuBar(), name, enable);
  QMenu* popup = (QMenu*) factory() -> container ("player_popup", this);
  if ( popup )
    enableSubmenu (popup, name, enable);
  popup = (QMenu*) factory() -> container ("library_popup", this);
  if ( popup )
    enableSubmenu (popup, name, enable);
  */
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

bool KPlayerWindow::isFullScreen (void) const
{
  return (KWindowSystem::windowInfo (winId(), NET::WMState).state() & NET::FullScreen) == NET::FullScreen;
}

bool KPlayerWindow::isMaximized (void) const
{
  return (KWindowSystem::windowInfo (winId(), NET::WMState).state() & NET::Max) == NET::Max;
}

void KPlayerWindow::showMaximized (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  bool maximized = isMaximized();
#endif
  KMainWindow::showMaximized();
  KWindowSystem::clearState (winId(), 1 << 9); // KDE 3.2 FullScreen
  KWindowSystem::setState (winId(), NET::Max);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::showMaximized " << maximized << " -> " << isMaximized() << "\n";
#endif
  //syncronizeEvents();
  //activateLayout();
}

void KPlayerWindow::showNormal (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  bool maximized = isMaximized();
#endif
  KMainWindow::showNormal();
  //KMainWindow::showNormal();
  KWindowSystem::clearState (winId(), NET::Max | (1 << 9)); // KDE 3.2 FullScreen
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::showNormal " << maximized << " -> " << isMaximized() << "\n";
#endif
  //syncronizeEvents();
  //activateLayout();
}

void KPlayerWindow::start (void)
{
  if ( isMinimized() )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "Restoring minimized window\n";
#endif
    //clearWState (WState_Visible); // or showNormal won't work
    showNormal();
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

//bool KPlayerX11TestGrab (Display*, int);

/*bool KPlayerWindow::event (QEvent* ev)
{
  kdDebugTime() << "KPlayer event: type " << ev -> type()
    << " spontaneous " << ev -> spontaneous() << "\n";
//if ( ev -> type() == QEvent::LayoutRequest && isFullScreen() )
//  return false;
  return KMainWindow::event (ev);
}*/

void KPlayerWindow::contextMenu (const QPoint& global_position)
{
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Main " << winId() << " wspace " << kPlayerWorkspace() -> winId()
    << " widget " << kPlayerWidget() -> winId() << "\n";
  dumpObject (this, 0);
  dumpObject (actionCollection(), 0);
#endif
  QMenu* popup = (QMenu*) factory() -> container ("player_popup", this);
  if ( popup )
    popup -> popup (global_position);
}

void KPlayerWindow::contextMenuEvent (QContextMenuEvent* event)
{
  KMainWindow::contextMenuEvent (event);
  contextMenu (event -> globalPos());
  event -> accept();
}

void KPlayerWindow::playlistStarted (void)
{
  setStatusText (i18n("Ready"));
  log() -> setError (false);
  if ( m_status_label )
    m_status_label -> unsetCursor();
}

void KPlayerWindow::playlistActivated (void)
{
  setCaption (settings() -> properties() -> caption());
  //clearStatusMessage();
  if ( m_progress_label )
    m_progress_label -> setText ("");
  connect (settings() -> properties(), SIGNAL (updated()), this, SLOT (refreshProperties()));
  enableVideoActions();
}

void KPlayerWindow::playlistStopped (void)
{
  if ( m_error_detected && ! log() -> isEmpty() )
    log() -> setError (true);
  m_error_detected = false;
  playerProgressChanged (process() -> position(), KPlayerProcess::Position);
}

void KPlayerWindow::statusPressed (void)
{
  if ( log() -> hasError() || m_error_detected )
    showMessageLog (true);
}

void KPlayerWindow::refreshProperties (void)
{
  setCaption (settings() -> properties() -> caption());
  enableVideoActions();
}

void KPlayerWindow::closeEvent (QCloseEvent* event)
{
  disconnect (log(), SIGNAL (windowHidden()), this, SLOT (logWindowHidden()));
  disconnect (library(), SIGNAL (windowHidden()), this, SLOT (libraryHidden()));
  disconnect (settings() -> properties(), SIGNAL (updated()), this, SLOT (refreshProperties()));
  disconnect (configuration(), SIGNAL (updated()), this, SLOT (refreshSettings()));
  saveOptions();
  library() -> library() -> terminate();
  playlist() -> terminate();
  KPlayerEngine::terminate();
  KMainWindow::closeEvent (event);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::closeEvent\n";
#endif
}

void KPlayerWindow::fileClearLog (void)
{
  setStatusText (i18n("Ready"));
  if ( m_status_label )
    m_status_label -> unsetCursor();
}

void KPlayerWindow::fileQuit (void)
{
  close();
}

void KPlayerWindow::viewPlaylist (void)
{
  showToolbar (PLAYLIST_TOOLBAR, toggleAction (m_toolbar[PLAYLIST_TOOLBAR].action) -> isChecked());
}

void KPlayerWindow::viewLibrary (void)
{
  showLibrary (toggleAction ("options_show_library") -> isChecked());
}

void KPlayerWindow::makeLibraryVisible (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::makeLibraryVisible\n";
#endif
  showLibrary (true);
}

void KPlayerWindow::showLibrary (bool show)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "showLibrary " << show << " => " << settings() -> fullScreen() << "\n";
#endif
  if ( settings() -> fullScreen() )
    m_library_fullscreen_visible = show;
  else
    m_library_normally_visible = show;
  showLibrary();
}

void KPlayerWindow::showLibrary (void)
{
  bool show = settings() -> fullScreen() ? m_library_fullscreen_visible : m_library_normally_visible;
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "showLibrary " << show << " <= " << settings() -> fullScreen() << "\n";
#endif
  toggleAction ("options_show_library") -> setChecked (show);
  m_show_library = show;
  if ( show )
  {
    library() -> show();
    library() -> setFocus();
  }
  else
    library() -> hide();
}

void KPlayerWindow::showMenubar (void)
{
  bool show = settings() -> fullScreen() ? m_menubar_fullscreen_visible : m_menubar_normally_visible;
  toggleAction (KStandardAction::stdName (KStandardAction::ShowMenubar)) -> setChecked (show);
  toggleAction (KStandardAction::stdName (KStandardAction::ShowMenubar)) -> setStatusTip (i18n("Shows/hides the menu bar"));
  if ( show )
    menuBar() -> show();
  else
    menuBar() -> hide();
}

void KPlayerWindow::viewMenubar (void)
{
  bool show = toggleAction (KStandardAction::stdName (KStandardAction::ShowMenubar)) -> isChecked();
  if ( settings() -> fullScreen() )
    m_menubar_fullscreen_visible = show;
  else
    m_menubar_normally_visible = show;
  if ( show )
    menuBar() -> show();
  else
    menuBar() -> hide();
}

void KPlayerWindow::showStatusbar (void)
{
  bool show = settings() -> fullScreen() ? m_statusbar_fullscreen_visible : m_statusbar_normally_visible;
  toggleAction (KStandardAction::stdName (KStandardAction::ShowStatusbar)) -> setChecked (show);
  toggleAction (KStandardAction::stdName (KStandardAction::ShowStatusbar)) -> setStatusTip (i18n("Shows/hides the status bar"));
  if ( show )
    statusBar() -> show();
  else
    statusBar() -> hide();
}

void KPlayerWindow::viewStatusbar (void)
{
  bool show = toggleAction (KStandardAction::stdName (KStandardAction::ShowStatusbar)) -> isChecked();
  if ( settings() -> fullScreen() )
    m_statusbar_fullscreen_visible = show;
  else
    m_statusbar_normally_visible = show;
  if ( show )
    statusBar() -> show();
  else
    statusBar() -> hide();
}

void KPlayerWindow::showMessageLog (void)
{
  bool show = settings() -> fullScreen() ? m_messagelog_fullscreen_visible : m_messagelog_normally_visible;
  toggleAction ("options_show_log") -> setChecked (show);
  m_show_log = show;
  if ( show )
    log() -> show();
  else
    log() -> hide();
}

void KPlayerWindow::showMessageLog (bool show)
{
  if ( settings() -> fullScreen() )
    m_messagelog_fullscreen_visible = show;
  else
    m_messagelog_normally_visible = show;
  showMessageLog();
  setStatusText (i18n("Ready"));
  log() -> setError (false);
  if ( m_status_label )
    m_status_label -> unsetCursor();
}

void KPlayerWindow::viewMessageLog (void)
{
  showMessageLog (toggleAction ("options_show_log") -> isChecked());
}

void KPlayerWindow::showToolbar (int index)
{
  bool show = settings() -> fullScreen() ? m_toolbar[index].fullscreen_visible : m_toolbar[index].normally_visible;
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "showToolbar '" << m_toolbar[index].name << "' " << show << " <= " << settings() -> fullScreen() << "\n";
#endif
  toggleAction (m_toolbar[index].action) -> setChecked (show);
  if ( show )
  {
    KToolBar* toolbar = toolBar (m_toolbar[index].name);
    toolbar -> show();
    int hint = toolbar -> minimumSizeHint().height();
    if ( toolbar -> orientation() == Qt::Vertical && kPlayerWorkspace() -> height() < hint
      && ! settings() -> fullScreen() && ! settings() -> maximized() )
    {
      settings() -> setDisplaySize (QSize (settings() -> displaySize().width(), hint));
      //zoom();
    }
  }
  else
    toolBar (m_toolbar[index].name) -> hide();
}

void KPlayerWindow::showToolbar (int index, bool show)
{
  if ( settings() -> fullScreen() )
    m_toolbar[index].fullscreen_visible = show;
  else
    m_toolbar[index].normally_visible = show;
  showToolbar (index);
}

void KPlayerWindow::viewMainToolbar (void)
{
  showToolbar (MAIN_TOOLBAR, toggleAction (m_toolbar[MAIN_TOOLBAR].action) -> isChecked());
}

void KPlayerWindow::viewProgressToolbar (void)
{
  showToolbar (PROGRESS_TOOLBAR, toggleAction (m_toolbar[PROGRESS_TOOLBAR].action) -> isChecked());
}

void KPlayerWindow::viewVolumeToolbar (void)
{
  showToolbar (VOLUME_TOOLBAR, toggleAction (m_toolbar[VOLUME_TOOLBAR].action) -> isChecked());
}

void KPlayerWindow::viewContrastToolbar (void)
{
  showToolbar (CONTRAST_TOOLBAR, toggleAction (m_toolbar[CONTRAST_TOOLBAR].action) -> isChecked());
}

void KPlayerWindow::viewBrightnessToolbar (void)
{
  showToolbar (BRIGHTNESS_TOOLBAR, toggleAction (m_toolbar[BRIGHTNESS_TOOLBAR].action) -> isChecked());
}

void KPlayerWindow::viewHueToolbar (void)
{
  showToolbar (HUE_TOOLBAR, toggleAction (m_toolbar[HUE_TOOLBAR].action) -> isChecked());
}

void KPlayerWindow::viewSaturationToolbar (void)
{
  showToolbar (SATURATION_TOOLBAR, toggleAction (m_toolbar[SATURATION_TOOLBAR].action) -> isChecked());
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
}

void KPlayerWindow::playerSizeAvailable (void)
{
  enableVideoActions();
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
  setStatusText (i18n("Error"));
  if ( log() -> isHidden() && m_status_label )
    m_status_label -> setCursor (Qt::PointingHandCursor);
  m_error_detected = true;
  if ( configuration() -> showMessagesOnError() )
    showMessageLog (true);
}

void KPlayerWindow::logWindowHidden (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Log window hidden signal\n";
  if ( log() -> isVisible() )
    kdDebugTime() << "  but the log window is visible\n";
  else if ( ! log() -> isHidden() )
    kdDebugTime() << "  but the log window is not hidden\n";
#endif
  showMessageLog (false);
}

void KPlayerWindow::libraryHidden (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Library window hidden signal\n";
  if ( library() -> isVisible() )
    kdDebugTime() << "  but the library is visible\n";
  else if ( ! library() -> isHidden() )
    kdDebugTime() << "  but the library is not hidden\n";
#endif
  showLibrary (false);
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
  KMainWindow::showEvent (event);
  if ( m_initial_show )
    return;
  m_initial_show = true;
  if ( x() != m_normal_geometry.x() && m_normal_geometry.x() >= 0
    && y() != m_normal_geometry.y() && m_normal_geometry.y() >= 0 )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "KPlayerWindow::showEvent: adjusting position to " << m_normal_geometry.x() << "x" << m_normal_geometry.y() << "\n";
#endif
    move (m_normal_geometry.x(), m_normal_geometry.y());
  }
  if ( width() != m_normal_geometry.width() && m_normal_geometry.width() >= minimumWidth()
    && height() != m_normal_geometry.height() && m_normal_geometry.height() >= minimumHeight() )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "KPlayerWindow::showEvent: adjusting size to " << m_normal_geometry.width() << "x" << m_normal_geometry.height() << "\n";
#endif
    resize (m_normal_geometry.width(), m_normal_geometry.height());
  }
/*if ( focusProxy() )
    KPlayerX11SetInputFocus (focusProxy() -> winId());
#ifdef DEBUG_KPLAYER_WINDOW
  else
    kdDebugTime() << "KPlayerWindow::showEvent: no focus proxy\n";
#endif*/
}

void KPlayerWindow::windowActivationChange (bool old)
{
  KMainWindow::windowActivationChange (old);
  bool active = isActiveWindow();
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Main window activation " << old << " -> " << active << "\n";
#endif
  if ( active )
    KPlayerX11GetKeyboardMouseState (winId());
/*if ( active && focusProxy() )
    KPlayerX11SetInputFocus (focusProxy() -> winId());
#ifdef DEBUG_KPLAYER_WINDOW
  else if ( active )
    kdDebugTime() << "  no focus proxy\n";
#endif*/
}

void KPlayerWindow::focusInEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Window focus in event\n";
#endif
  KMainWindow::focusInEvent (event);
}

void KPlayerWindow::focusOutEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Window focus out event\n";
#endif
  KMainWindow::focusOutEvent (event);
}

void KPlayerWindow::moveEvent (QMoveEvent* event)
{
#ifdef DEBUG_KPLAYER_RESIZING
  bool maximized = isMaximized();
#endif
  KMainWindow::moveEvent (event);
  if ( ! m_full_screen && ! m_set_display_size && ! settings() -> maximized() && ! isMaximized() && m_initial_show )
    m_normal_geometry.setRect (x(), y(), width(), height());
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "WiMove " << event -> oldPos().x() << "x" << event -> oldPos().y()
    << " => " << event -> pos().x() << "x" << event -> pos().y() << ", "
    << maximized << " -> " << isMaximized() << "\n";
  kdDebugTime() << "             Normal geometry " << m_normal_geometry.x() << "x" << m_normal_geometry.y()
    << " " << m_normal_geometry.width() << "x" << m_normal_geometry.height() << "\n";
#endif
}

void KPlayerWindow::resizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_RESIZING
  bool maximized = isMaximized();
#endif
  KMainWindow::resizeEvent (event);
  if ( ! m_full_screen && ! m_set_display_size && ! settings() -> maximized() && ! isMaximized() && m_initial_show )
    m_normal_geometry.setSize (QSize (width(), height()));
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "WiSize " << event -> oldSize(). width() << "x" << event -> oldSize(). height()
    << " => " << event -> size(). width() << "x" << event -> size(). height() << ", maximized "
    << maximized << " -> " << isMaximized() << " spontaneous " << event -> spontaneous() << "\n";
  kdDebugTime() << "             Normal geometry " << m_normal_geometry.x() << "x" << m_normal_geometry.y()
    << " " << m_normal_geometry.width() << "x" << m_normal_geometry.height() << "\n";
#endif
  if ( m_set_display_size )
  {
    if ( kPlayerWorkspace() -> isResizing() )
      m_set_display_size = false;
    else
      QTimer::singleShot (0, this, SLOT (setDisplaySize()));
  }
}

void KPlayerWindow::setDisplaySize (void)
{
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "KPlayerWindow::setDisplaySize\n";
#endif
#ifdef DEBUG_KPLAYER_SIZING_HACKS
  if ( ! m_full_screen && ! isMaximized() )
  {
    move (m_normal_geometry.x(), m_normal_geometry.y());
    resize (m_normal_geometry.width(), m_normal_geometry.height());
  }
#endif
  m_set_display_size = false;
  engine() -> setDisplaySize();
}

void KPlayerWindow::setMinimumSize (int w, int h)
{
  QSize prev (size()), msh (minimumSizeHint());
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Set minimum size " << w << "x" << h << " => " << msh.width() << "x" << msh.height() << "\n";
#endif
  w = msh.width();
  h = msh.height();
  KMainWindow::setMinimumSize (w, h);
#ifdef DEBUG_KPLAYER_RESIZING
  dumpObject (this, 0, 1);
  if ( prev != size() )
    kdDebugTime() << "             Size changed\n";
#endif
  if ( prev != size() && m_previous_size == prev )
    m_previous_size = size();
}

/*QSize KPlayerWindow::sizeHint (void) const
{
  return minimumSizeHint() + kPlayerWidget() -> videoSize();
}*/

/*
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

void KPlayerWindow::toFullScreen (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::toFullScreen\n";
#endif
  bool active = isActiveWindow();
  if ( isMaximized() )
    m_maximized = true;
  else if ( ! m_full_screen && ! m_set_display_size )
    m_normal_geometry.setRect (x(), y(), width(), height());
  setAcceptDrops (false);
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Pre full screen: " << geometry().x() << "x" << geometry().y() << " " << width() << "x" << height() << "\n";
  kdDebugTime() << "                              " << frameGeometry().x() << "x" << frameGeometry().y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
  kdDebugTime() << "                              " << m_normal_geometry.x() << "x" << m_normal_geometry.y() << " " << m_normal_geometry.width() << "x" << m_normal_geometry.height() << "\n";
#endif
  showMenubar();
  showStatusbar();
  showMessageLog();
  for ( int i = 0; i < KPLAYER_TOOLBARS; i ++ )
    showToolbar (i);
  showLibrary();
  if ( ! m_full_screen )
  {
    m_full_screen = true;
    showFullScreen();
  }
//syncronizeEvents();
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Full screen: " << geometry().x() << "x" << geometry().y() << " " << width() << "x" << height() << "\n";
  kdDebugTime() << "                          " << frameGeometry().x() << "x" << frameGeometry().y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
#endif
  layout() -> setSpacing (-1);
  activateLayout();
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Full screen: " << geometry().x() << "x" << geometry().y() << " " << width() << "x" << height() << "\n";
  kdDebugTime() << "                          " << frameGeometry().x() << "x" << frameGeometry().y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
  dumpObject (this, 0);
#endif
  if ( active )
  {
    activateWindow();
    KWindowSystem::forceActiveWindow (winId());
  }
  setAcceptDrops (true);
  enablePlayerActions();
}

void KPlayerWindow::toNormalScreen (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::toNormalScreen\n";
#endif
#ifdef DEBUG_KPLAYER_RESIZING
  dumpObject (this, 0);
#endif
  bool active = isActiveWindow();
  layout() -> setSpacing (0);
  setAcceptDrops (false);
  showMenubar();
  showStatusbar();
  showMessageLog();
  for ( int i = 0; i < KPLAYER_TOOLBARS; i ++ )
    showToolbar (i);
  showLibrary();
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Full screen: " << geometry().x() << "x" << geometry().y() << " " << width() << "x" << height() << "\n";
  kdDebugTime() << "                          " << frameGeometry().x() << "x" << frameGeometry().y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
#endif
#ifdef DEBUG_KPLAYER_SIZING_HACKS
  if ( ! m_maximized )
  {
    //move (m_normal_geometry.x(), m_normal_geometry.y());
    //resize (m_normal_geometry.width(), m_normal_geometry.height());
    setGeometry (0, 0, width(), height() - 2);
    setGeometry (m_normal_geometry);
    activateLayout();
    //m_initial_show = false;
  }
#endif
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Full screen: " << geometry().x() << "x" << geometry().y() << " " << width() << "x" << height() << "\n";
  kdDebugTime() << "                          " << frameGeometry().x() << "x" << frameGeometry().y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
#endif
  if ( m_full_screen )
  {
    m_full_screen = false;
    showNormal();
  }
#ifdef DEBUG_KPLAYER_SIZING_HACKS
  /*if ( ! m_maximized )
  {
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "  Using normal geometry\n";
#endif
    move (m_normal_geometry.x(), m_normal_geometry.y());
    resize (m_normal_geometry.width(), m_normal_geometry.height());
  }*/
#endif
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Normal: " << geometry().x() << "x" << geometry().y() << " " << width() << "x" << height() << "\n";
  kdDebugTime() << "                     " << frameGeometry().x() << "x" << frameGeometry().y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
#endif
  if ( active )
  {
    activateWindow();
    KWindowSystem::forceActiveWindow (winId());
  }
  setAcceptDrops (true);
  enablePlayerActions();
}

void KPlayerWindow::initialSize (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::initialSize (" << isMaximized() << ")\n";
#endif
  QSize size (settings() -> displaySize());
  if ( settings() -> constrainedSize() || settings() -> properties() -> hasVideo() || ! size.isEmpty() )
    return;
  for ( int i = 0; i <= VOLUME_TOOLBAR; i ++ )
    if ( toolBar (m_toolbar[i].name) -> orientation() == Qt::Vertical && m_toolbar[i].normally_visible )
//      && toggleAction (m_toolbar[i].action) -> isChecked() )
    {
      size.setHeight (kPlayerWorkspace() -> height());
      break;
    }
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "             " << log() -> isHidden() << " " << log() -> place() << " " << library() -> isHidden() << " " << library() -> place() << "\n";
#endif
  if ( library() -> isVisible() // && library() -> place() == QDockWindow::InDock
      || log() -> isVisible() /* && log() -> place() == QDockWindow::InDock */ )
    size.setWidth (kPlayerWorkspace() -> width());
  settings() -> setDisplaySize (size);
}

void KPlayerWindow::correctSize (void)
{
  //syncronizeEvents();
  bool maximized = isMaximized();
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::correctSize (" << maximized << ")\n";
#endif
  if ( settings() -> fullScreen() )
    layout() -> setSpacing (-1);
  else if ( ! maximized && settings() -> maximized() )
  {
#ifdef DEBUG_KPLAYER_SIZING_HACKS
    if ( m_maximized )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << " Using normal geometry\n";
#endif
      //syncronizeEvents();
      move (m_normal_geometry.x(), m_normal_geometry.y());
      resize (m_normal_geometry.width(), m_normal_geometry.height());
      m_maximized = false;
      activateLayout();
    }
#endif
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << "KPlayerWindow::correctSize: setMaximized (false)\n";
#endif
    settings() -> setMaximized (maximized);
  }
  if ( settings() -> constrainedSize() )
  {
    if ( maximized && ! settings() -> maximized() )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << "KPlayerWindow::correctSize: setMaximized (true)\n";
#endif
      settings() -> setMaximized (maximized);
    }
    return;
  }
  if ( m_previous_size != size() )
  {
    if ( maximized )
    {
#ifdef DEBUG_KPLAYER_WINDOW
      kdDebugTime() << "KPlayerWindow::correctSize: setMaximized (true)\n";
#endif
      settings() -> setMaximized (maximized);
    }
    else
      settings() -> setDisplaySize (settings() -> adjustSize (kPlayerWorkspace() -> size(), m_previous_size.width() == width()));
  }
#ifdef DEBUG_KPLAYER_WINDOW
  else
    kdDebugTime() << "             Using previous size\n";
#endif
}

void KPlayerWindow::windowStateChanged (uint wid)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::windowStateChanged (" << wid << ")\n";
#endif
  if ( wid == winId() )
  {
    m_set_display_size = true;
    m_full_screen = isFullScreen();
#ifdef DEBUG_KPLAYER_WINDOW
    kdDebugTime() << " Full screen " << m_full_screen << "\n";
#endif
    bool full_screen = settings() -> fullScreen();
    settings() -> setFullScreen (m_full_screen);
    if ( ! full_screen && m_full_screen )
      toFullScreen();
    else if ( full_screen && ! m_full_screen )
      toNormalScreen();
  }
}

void KPlayerWindow::syncronize (bool user_resize)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::syncronize (" << user_resize << ")\n";
#endif
  if ( settings() -> fullScreen() )
  {
    if ( ! m_full_screen && ! user_resize )
      toFullScreen();
    return;
  }
  if ( m_full_screen )
  {
    if ( user_resize )
      return;
    else
      toNormalScreen();
  }
  if ( settings() -> maximized() )
  {
    if ( ! isMaximized() )
      showMaximized();
    return;
  }
  if ( isMaximized() )
    showNormal();
}

//extern uint qGlobalPostedEventsCount (void);

/*void KPlayerWindow::syncronizeEvents (void)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::syncronizeEvents\n";
#endif
  QApplication::syncX();
  bool rv = QApplication::eventLoop() -> processEvents (QEventLoop::ExcludeUserInput | QEventLoop::ExcludeSocketNotifiers);
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::syncronizeEvents: processEvents returned " << rv << "\n";
  //kdDebugTime() << "KPlayerWindow::syncronizeEvents: posted event count " << qGlobalPostedEventsCount() << "\n";
#endif
  //QApplication::sendPostedEvents();
  //QApplication::sendPostedEvents (this, QEvent::Resize);
  //QApplication::sendPostedEvents (this, QEvent::Move);
  //QApplication::sendPostedEvents (this, QEvent::LayoutRequest);
  //QApplication::sendPostedEvents (kPlayerWorkspace(), QEvent::Resize);
#ifdef DEBUG_KPLAYER_WINDOW
  //kdDebugTime() << "KPlayerWindow::syncronizeEvents: posted event count " << qGlobalPostedEventsCount() << "\n";
  //kdDebugTime() << "KPlayerWindow::syncronizeEvents done\n";
#endif
}*/

/*
void KPlayerWindow::zoom (void)
{
  if ( m_full_screen )
    return;
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerWindow::zoom\n";
#endif
  QSize target (minimumSizeHint());
  QRect available (availableGeometry());
  KMainWindow::setMinimumSize (target.width(), target.height());
  do_zoom();
  target = frameGeometry().size() + settings() -> displaySize().expandedTo (QSize (1, 1)) - centralWidget() -> size();
  Qt::Dock dock;
  int index, offset;
  bool newline;
  if ( ! log() -> isHidden() && getLocation (log(), dock, index, newline, offset)
    && ((dock == Qt::DockLeft || dock == Qt::DockRight) && target.width() >= available.width()
    || (dock == Qt::DockTop || dock == Qt::DockBottom) && target.height() >= available.height()) )
  {
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << "Log dock " << dock << " " << index << " " << newline << " " << offset << " " << log() -> x() << "x" << log() -> y() << " " << log() -> width() << "x" << log() -> height() << "\n";
    kdDebugTime() << "Minimum log size " << log() -> minimumWidth() << "x" << log() -> minimumHeight() << "\n";
#endif
    if ( (dock == Qt::DockLeft || dock == Qt::DockRight)
      && log() -> width() - log() -> minimumWidth() > target.width() - available.width() )
    {
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << "New log width " << (log() -> width() - target.width() + available.width()) << "\n";
#endif
      log() -> setFixedExtentWidth (log() -> width() - target.width() + available.width());
    }
    else if ( (dock == Qt::DockTop || dock == Qt::DockBottom)
      && log() -> height() - log() -> minimumHeight() > target.height() - available.height() )
    {
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << "New log height " << (log() -> height() - target.height() + available.height() - 1) << "\n";
#endif
      log() -> setFixedExtentHeight (log() -> height() - target.height() + available.height() - 1);
    }
    else if ( m_show_log )
      log() -> undock();
    else
      log() -> hide();
    do_zoom();
    target = frameGeometry().size() + settings() -> displaySize().expandedTo (QSize (1, 1)) - centralWidget() -> size();
  }
  if ( ! library() -> isHidden() && getLocation (library(), dock, index, newline, offset)
    && ((dock == Qt::DockLeft || dock == Qt::DockRight) && target.width() >= available.width()
    || (dock == Qt::DockTop || dock == Qt::DockBottom) && target.height() >= available.height()) )
  {
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << "Library dock " << dock << " " << index << " " << newline << " " << offset << " " << library() -> x() << "x" << library() -> y() << " " << library() -> width() << "x" << library() -> height() << "\n";
    kdDebugTime() << "Minimum library size " << library() -> minimumWidth() << "x" << library() -> minimumHeight() << "\n";
#endif
    if ( (dock == Qt::DockLeft || dock == Qt::DockRight)
      && library() -> width() - library() -> minimumWidth() > target.width() - available.width() )
    {
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << "New library width " << (library() -> width() - target.width() + available.width()) << "\n";
#endif
      library() -> setFixedExtentWidth (library() -> width() - target.width() + available.width());
    }
    else if ( (dock == Qt::DockTop || dock == Qt::DockBottom)
      && library() -> height() - library() -> minimumHeight() > target.height() - available.height() )
      //>= target.height() - available.height() )
    {
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << "New library height " << (library() -> height() - target.height() + available.height() - 1) << "\n";
#endif
      library() -> setFixedExtentHeight (library() -> height() - target.height() + available.height() - 1);
    }
    else if ( m_show_library )
      library() -> undock();
    else
      library() -> hide();
    do_zoom();
    target = frameGeometry().size() + settings() -> displaySize().expandedTo (QSize (1, 1)) - centralWidget() -> size();
  }
  int i = 0;
  while ( centralWidget() -> size() != settings() -> displaySize().expandedTo (QSize (1, 1)) && i ++ < 5 )
  {
    settings() -> setDisplaySize (settings() -> constrainSize (centralWidget() -> size()
      + available.size() - frameGeometry().size() - QSize (1, 1)));
    do_zoom();
  }
  m_show_log = m_show_library = false;
  do_move (frameGeometry());
  m_previous_size = size();
  activateLayout();
//KPlayerX11SendConfigureEvent (winId(), geometry().x(), geometry().y(), width(), height());
//syncronizeEvents();
//layout() -> invalidate();
//QApplication::postEvent (this, new QResizeEvent (size(), size()));
//KPlayerX11SendConfigureEvent (winId(), geometry().x(), geometry().y(), width(), height());
  QApplication::syncX();
//KPlayerX11DiscardConfigureEvents (winId());
  KPlayerX11DiscardConfigureEvents (kPlayerWorkspace() -> winId());
//syncronizeEvents();
  QApplication::postEvent (this, new QEvent (QEvent::LayoutRequest));
//checkBarActions();
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "Zoom done\n";
#endif
}

void KPlayerWindow::do_move (const QRect& frame)
{
  QRect available (availableGeometry());
  bool xl = geometry().left() < available.left();
  bool xr = frame.right() > available.right();
  bool yt = frame.top() < available.top();
  bool yb = frame.bottom() > available.bottom();
  if ( xl || xr || yt || yb )
  {
    int xc = ! xl && ! xr ? x() : xr && available.width() > frame.width() ?  available.right() - frame.width()
      : available.left();
    int yc = ! yt && ! yb ? y() : yb && available.height() > frame.height() ?  available.bottom() - frame.height()
      : available.top();
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << "Moving to " << xc << "x" << yc << "\n";
#endif
    move (xc, yc);
  }
}
*/

void KPlayerWindow::activateLayout (void)
{
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "KPlayerWindow::activateLayout\n";
#endif
  layout() -> activate();
  QApplication::sendPostedEvents (this, QEvent::Resize);
  QApplication::sendPostedEvents (this, QEvent::Move);
  QApplication::sendPostedEvents (this, QEvent::LayoutRequest);
  QApplication::sendPostedEvents (kPlayerWorkspace(), QEvent::Resize);
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "KPlayerWindow::activateLayout done\n";
#endif
}

#if 0
void KPlayerWindow::do_zoom (void)
{
  QRect available (availableGeometry());
  //if ( isMaximized() )
  //  showNormal();
  activateLayout();
  QSize minimum (minimumSizeHint());
  QSize video (settings() -> displaySize().expandedTo (QSize (1, 1)));
  QSize cwsize (centralWidget() -> size());
  QSize target (size() + video - cwsize);
  if ( target.width() < minimum.width() || target.height() < minimum.height() )
  {
    settings() -> setDisplaySize (cwsize);
    video = settings() -> displaySize().expandedTo (QSize (1, 1));
    target = size();
  }
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Display " << video.width() << "x" << video.height() << "\n";
  kdDebugTime() << "             Window  " << geometry().x() << "x" << geometry().y() << " " << width() << "x" << height() << "\n";
  kdDebugTime() << "             WFrame  " << x() << "x" << y() << " " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
  kdDebugTime() << "             WSpace  " << cwsize.width() << "x" << cwsize.height() << "\n";
  kdDebugTime() << "             Widget  " << kPlayerWidget() -> size().width() << "x" << kPlayerWidget() -> size().height() << "\n";
  kdDebugTime() << "             Target  " << target.width() << "x" << target.height() << "\n";
/*  << "\n Total  " << minimumSize().width() << "x" << minimumSize().height()
    << " " << minimumSizeHint().width() << "x" << minimumSizeHint().height()
    << " " << sizeHint().width() << "x" << sizeHint().height()
    << "\n Top    " << topDock() -> minimumSize().width() << "x" << topDock() -> minimumSize().height()
    << " " << topDock() -> minimumSizeHint().width() << "x" << topDock() -> minimumSizeHint().height()
    << " " << topDock() -> sizeHint().width() << "x" << topDock() -> sizeHint().height()
    << "\n Bottom " << bottomDock() -> minimumSize().width() << "x" << bottomDock() -> minimumSize().height()
    << " " << bottomDock() -> minimumSizeHint().width() << "x" << bottomDock() -> minimumSizeHint().height()
    << " " << bottomDock() -> sizeHint().width() << "x" << bottomDock() -> sizeHint().height()
    << "\n Layout " << layout() -> minimumSize().width() << "x" << layout() -> minimumSize().height()
    << " " << layout() -> sizeHint().width() << "x" << layout() -> sizeHint().height()
    << "\n        " << layout() -> minimumSize().width() << "x" << layout() -> minimumSize().height()
    << " " << layout() -> sizeHint().width() << "x" << layout() -> sizeHint().height() << "\n"; */
#endif
  int i = 0;
  while ( target != size() && i ++ < 5 )
  {
    cwsize = target - size();
    QRect rect (frameGeometry());
    rect.addCoords (0, 0, cwsize.width(), cwsize.height());
    if ( rect.width() > available.width() || rect.height() > available.height() )
      break;
    do_move (rect);
    resize (target);
    //if ( isMaximized() )
    //  showNormal();
    activateLayout();
    cwsize = centralWidget() -> size();
    target = size() + video - cwsize;
    if ( target.width() < minimum.width() || target.height() < minimum.height() )
    {
      settings() -> setDisplaySize (cwsize);
      video = settings() -> displaySize().expandedTo (QSize (1, 1));
      target = size();
    }
#ifdef DEBUG_KPLAYER_RESIZING
    kdDebugTime() << "Window  " << width() << "x" << height() << "\n";
    kdDebugTime() << "             WFrame  " << frameGeometry().width() << "x" << frameGeometry().height() << "\n";
    kdDebugTime() << "             WSpace  " << cwsize.width() << "x" << cwsize.height() << "\n";
    kdDebugTime() << "             Widget  " << kPlayerWidget() -> size().width() << "x" << kPlayerWidget() -> size().height() << "\n";
    kdDebugTime() << "             Target  " << target.width() << "x" << target.height() << "\n";
/*    << " Total  " << minimumSize().width() << "x" << minimumSize().height()
      << " " << minimumSizeHint().width() << "x" << minimumSizeHint().height()
      << " " << sizeHint().width() << "x" << sizeHint().height()
      << "\n Top    " << topDock() -> minimumSize().width() << "x" << topDock() -> minimumSize().height()
      << " " << topDock() -> minimumSizeHint().width() << "x" << topDock() -> minimumSizeHint().height()
      << " " << topDock() -> sizeHint().width() << "x" << topDock() -> sizeHint().height()
      << "\n Bottom " << bottomDock() -> minimumSize().width() << "x" << bottomDock() -> minimumSize().height()
      << " " << bottomDock() -> minimumSizeHint().width() << "x" << bottomDock() -> minimumSizeHint().height()
      << " " << bottomDock() -> sizeHint().width() << "x" << bottomDock() -> sizeHint().height()
      << "\n Layout " << layout() -> minimumSize().width() << "x" << layout() -> minimumSize().height()
      << " " << layout() -> sizeHint().width() << "x" << layout() -> sizeHint().height()
      << "\n        " << layout() -> minimumSize().width() << "x" << layout() -> minimumSize().height()
      << " " << layout() -> sizeHint().width() << "x" << layout() -> sizeHint().height() << "\n"; */
#endif
  }
#ifdef DEBUG_KPLAYER_RESIZING
  if ( target != size() )
    kdDebugTime() << "Zoom unsuccessful\n";
#endif
}

QRect KPlayerWindow::availableGeometry (void) const
{
  QRect rect;
  if ( ! QApplication::desktop() -> isVirtualDesktop() )
  {
    rect = QApplication::desktop() -> availableGeometry();
#ifdef DEBUG_KPLAYER_RESIZING
      kdDebugTime() << "Screen " << QApplication::desktop() -> screenGeometry().x()
        << "x" << QApplication::desktop() -> screenGeometry().y()
        << " " << QApplication::desktop() -> screenGeometry().width()
        << "x" << QApplication::desktop() -> screenGeometry().height() << "\n";
#endif
  }
  else
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
#ifdef DEBUG_KPLAYER_RESIZING
  kdDebugTime() << "Desktop " << QApplication::desktop() -> width() << "x" << QApplication::desktop() -> height() << "\n";
  kdDebugTime() << "Total available geometry " << rect.width() << "x" << rect.height() << "\n";
#endif
  return rect;
}
#endif

/*void KPlayerWindow::barOrientationChanged (Orientation)
{
  kdDebugTime() << "A bar orientation has changed.\n";
  kdDebugTime() << "Central Widget Size: " << centralWidget() -> width() << "x" << centralWidget() -> height()
    << ", KPlayer Widget Size: " << kPlayerWidget() -> width() << "x" << kPlayerWidget() -> height() << ".\n";
}

void KPlayerWindow::barPlaceChanged (QDockWindow::Place)
{
  kdDebugTime() << "A bar place has changed.\n";
  kdDebugTime() << "Central Widget Size: " << centralWidget() -> width() << "x" << centralWidget() -> height()
    << ", KPlayer Widget Size: " << kPlayerWidget() -> width() << "x" << kPlayerWidget() -> height() << ".\n";
}

void KPlayerWindow::barVisibilityChanged (bool)
{
  kdDebugTime() << "A bar visibility has changed.\n";
  kdDebugTime() << "Central Widget Size: " << centralWidget() -> width() << "x" << centralWidget() -> height()
    << ", KPlayer Widget Size: " << kPlayerWidget() -> width() << "x" << kPlayerWidget() -> height() << ".\n";
}*/
