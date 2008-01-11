/***************************************************************************
                          kplayerlogwindow.cpp
                          --------------------
    begin                : Fri May 9 2003
    copyright            : (C) 2003-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klocale.h>
#include <qaction.h>
#include <qevent.h>
#include <qmenu.h>
#include <qscrollbar.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_LOG
#endif

#include "kplayerlogwindow.h"
#include "kplayerlogwindow.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

KPlayerLogWindow::KPlayerLogWindow (KActionCollection* ac, QWidget* parent)
  : QDockWidget (parent)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Creating log window\n";
#endif
  hide();
  setObjectName ("log");
  setWidget (new KPlayerLogWidget (ac, this));
  //setResizeEnabled (true);
  //setCloseMode (QDockWidget::Always);
  //setNewLine (true);
  setWindowTitle (i18n("Messages"));
  setAllowedAreas (Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
  setFeatures (DockWidgetClosable | DockWidgetMovable | DockWidgetFloatable | DockWidgetVerticalTitleBar);
  setWhatsThis (i18n("Message log is a window where KPlayer shows messages it receives from MPlayer. KPlayer can show it automatically when it detects an MPlayer error if that option is selected in KPlayer settings."));
  connect (this, SIGNAL (visibilityChanged (bool)), SLOT (setVisibility (bool)));
}

void KPlayerLogWindow::initialize (QMenu* menu)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Initializing log window\n";
#endif
  logWidget() -> setPopupMenu (menu);
}

void KPlayerLogWindow::setVisibility (bool visibility)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log visibility " << visibility << " docked " << docked() << "\n";
#endif
  m_visibility = visibility;
}

void KPlayerLogWindow::setError (bool flag)
{
  if ( flag && ! logWidget() -> hasError() && ! isVisible() && logWidget() -> width() < 300 )
  {
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log resizing widget\n";
    kdDebugTime() << "Log widget " << logWidget() -> width() << "x" << logWidget() -> height() << "\n";
#endif
    logWidget() -> resize (KPlayerEngine::engine() -> configuration() -> preferredVideoWidth(), logWidget() -> height());
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log widget " << logWidget() -> width() << "x" << logWidget() -> height() << "\n";
#endif
  }
  logWidget() -> setError (flag);
}

void KPlayerLogWindow::moveEvent (QMoveEvent* event)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "LogWindow::moveEvent " << event -> oldPos().x() << "x" << event -> oldPos().y() << " -> " << event -> pos().x() << "x" << event -> pos().y() << " " << isHidden() << " " << docked() << "\n";
#endif
  QDockWidget::moveEvent (event);
  if ( ! isHidden() )
    emit moved (docked());
}

void KPlayerLogWindow::resizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "LogWindow::resizeEvent " << event -> oldSize().width() << "x" << event -> oldSize().height() << " -> " << event -> size().width() << "x" << event -> size().height() << " " << isHidden() << " " << docked() << "\n";
#endif
  QDockWidget::resizeEvent (event);
  if ( ! isHidden() )
    emit resized();
}

KPlayerLogWidget::KPlayerLogWidget (KActionCollection* ac, QWidget* parent)
  : KTextEdit (parent)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Creating log widget\n";
#endif
  m_ac = ac;
  //setTextFormat (Qt::PlainText);
  setReadOnly (true);
  setError (false);
  KAction* action = new KAction (actionCollection());
  actionCollection() -> addAction ("log_select_all", action);
  connect (action, SIGNAL (triggered()), SLOT (selectAll()));
  action -> setText (i18n("&Select All"));
  action -> setStatusTip (i18n("Selects all messages in the message log"));
  action -> setWhatsThis (i18n("Select All command selects all messages in the message log."));
  action = new KAction (actionCollection());
  actionCollection() -> addAction ("log_copy", action);
  connect (action, SIGNAL (triggered()), SLOT (copy()));
  action -> setText (i18n("&Copy"));
  action -> setStatusTip (i18n("Copies the text selected in the message log to the clipboard"));
  action -> setWhatsThis (i18n("Copy command copies the text selected in the message log to the clipboard."));
  action = new KAction (actionCollection());
  actionCollection() -> addAction ("log_clear", action);
  connect (action, SIGNAL (triggered()), SLOT (clear()));
  action -> setText (i18n("C&lear"));
  action -> setStatusTip (i18n("Clears all messages from the message log"));
  action -> setWhatsThis (i18n("Clear command removes all messages from the message log."));
  connect (this, SIGNAL (selectionChanged()), SLOT (updateActions()));
  m_height = kPlayerConfig() -> group ("General Options").readEntry ("Message Log Height", 250);
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << " Height " << m_height << "\n";
#endif
}

void KPlayerLogWidget::updateActions (void)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::updateActions\n";
#endif
  bool enable = ! document() -> isEmpty();
  action ("log_select_all") -> setEnabled (enable);
  action ("log_copy") -> setEnabled (textCursor().hasSelection());
  action ("log_clear") -> setEnabled (enable);
}

void KPlayerLogWidget::clear (void)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::clear\n";
#endif
  KTextEdit::clear();
  setError (false);
}

void KPlayerLogWidget::setError (bool flag)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::setError " << flag << "\n";
#endif
  if ( ! flag )
    m_location = 0;
  else if ( ! m_error )
  {
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log::stopScrolling\n";
    if ( verticalScrollBar() )
      kdDebugTime() << "Log " << width() << " " << height() << " " << verticalScrollBar() -> value() << " / " << verticalScrollBar() -> maximum() << "\n";
#endif
    //scrollToBottom();
    if ( verticalScrollBar() )
      verticalScrollBar() -> setValue (verticalScrollBar() -> maximum());
    //scrollBy (0, -1);
    m_location = isVisible() || ! verticalScrollBar() ? 0 : verticalScrollBar() -> value();
#ifdef DEBUG_KPLAYER_LOG
    if ( verticalScrollBar() )
      kdDebugTime() << "Log " << width() << " " << height() << " " << verticalScrollBar() -> value() << " / " << verticalScrollBar() -> maximum() << "\n";
#endif
  }
  m_error = flag;
}

QSize KPlayerLogWidget::sizeHint (void) const
{
  return QSize (KTextEdit::sizeHint().width(), m_height);
}

void KPlayerLogWidget::showEvent (QShowEvent* event)
{
  KTextEdit::showEvent (event);
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::showEvent\n";
  if ( verticalScrollBar() )
    kdDebugTime() << "Log " << width() << " " << height() << " " << verticalScrollBar() -> value() << " / " << verticalScrollBar() -> maximum() << "\n";
#endif
  if ( ! m_error )
  {
    if ( verticalScrollBar() )
      verticalScrollBar() -> setValue (verticalScrollBar() -> maximum());
  }
  else if ( m_location )
  {
    if ( verticalScrollBar() )
      verticalScrollBar() -> setValue (m_location);
    //setContentsPos (0, m_location);
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log scrolling to " << m_location << "\n";
    if ( verticalScrollBar() )
      kdDebugTime() << "Log " << width() << " " << height() << " " << verticalScrollBar() -> value() << " / " << verticalScrollBar() -> maximum() << "\n";
#endif
    m_location = 0;
  }
}

void KPlayerLogWidget::resizeEvent (QResizeEvent* event)
{
  bool at_bottom = ! verticalScrollBar() || verticalScrollBar() -> value() == verticalScrollBar() -> maximum();
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "LogWidget::resizeEvent " << event -> oldSize().width() << "x" << event -> oldSize().height() << " -> " << event -> size().width() << "x" << event -> size().height() << " " << at_bottom << "\n";
  if ( verticalScrollBar() )
    kdDebugTime() << " Log " << width() << "x" << height() << " " << verticalScrollBar() -> value() << "/" << verticalScrollBar() -> maximum() << "\n";
#endif
  KTextEdit::resizeEvent (event);
#ifdef DEBUG_KPLAYER_LOG
  if ( verticalScrollBar() )
    kdDebugTime() << " Log " << width() << "x" << height() << " " << verticalScrollBar() -> value() << "/" << verticalScrollBar() -> maximum() << "\n";
#endif
  if ( at_bottom && verticalScrollBar() && verticalScrollBar() -> value() != verticalScrollBar() -> maximum() )
  {
    verticalScrollBar() -> setValue (verticalScrollBar() -> maximum());
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << " Log: scrolling to bottom\n";
    if ( verticalScrollBar() )
      kdDebugTime() << "Log " << width() << "x" << height() << " " << verticalScrollBar() -> value() << "/" << verticalScrollBar() -> maximum() << "\n";
#endif
  }
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << " Mouse down " << KPlayerEngine::engine() -> settings() -> anyButton() << "\n";
  kdDebugTime() << " Docked " << parent() -> docked() << "\n";
  kdDebugTime() << " Visible " << parent() -> visible() << "\n";
#endif
  if ( KPlayerEngine::engine() -> settings() -> anyButton() && ! KPlayerEngine::engine() -> layoutInProgress()
    && parent() -> visible() )
  {
    rememberHeight();
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << " Height " << m_height << "\n";
#endif
  }
}

void KPlayerLogWidget::contextMenuEvent (QContextMenuEvent* event)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "KPlayerLogWidget::contextMenuEvent\n";
#endif
  KTextEdit::contextMenuEvent (event);
  m_popup -> popup (event -> globalPos());
  event -> accept();
}

void KPlayerLogWidget::terminate (void)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "KPlayerLogWidget::terminate\n";
#endif
  kPlayerConfig() -> group ("General Options").writeEntry ("Message Log Height", m_height);
}
