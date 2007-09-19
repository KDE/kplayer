/***************************************************************************
                          kplayerlogwindow.cpp
                          --------------------
    begin                : Fri May 9 2003
    copyright            : (C) 2003-2007 by kiriuja
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
  setWidget (new KPlayerLogWidget (ac, this));
  //setResizeEnabled (true);
  //setCloseMode (QDockWidget::Always);
  //setNewLine (true);
  setWindowTitle (i18n("Messages"));
}

void KPlayerLogWindow::initialize (QMenu* menu)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Initializing log window\n";
#endif
  logWidget() -> setPopupMenu (menu);
}

void KPlayerLogWindow::setError (bool flag)
{
  if ( flag && ! logWidget() -> hasError() && ! isVisible() && logWidget() -> width() < 300 )
  {
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log resizing widget\n";
    kdDebugTime() << "Log widget " << logWidget() -> width() << "x" << logWidget() -> height() << "\n";
#endif
    logWidget() -> resize (KPlayerEngine::engine() -> configuration() -> minimumInitialWidth(), logWidget() -> height());
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log widget " << logWidget() -> width() << "x" << logWidget() -> height() << "\n";
#endif
  }
  logWidget() -> setError (flag);
}

void KPlayerLogWindow::hideEvent (QHideEvent* event)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::hideEvent\n";
#endif
  QDockWidget::hideEvent (event);
  if ( isHidden() ) // || ! event -> spontaneous() )
    emit windowHidden();
}

KPlayerLogWidget::KPlayerLogWidget (KActionCollection* ac, QWidget* parent)
  : KTextEdit (parent)
{
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
  kdDebugTime() << "Log::resizeEvent " << event -> size(). width() << " " << event -> size(). height() << " " << event -> oldSize(). width() << " " << event -> oldSize(). height() << " at_bottom " << at_bottom << "\n";
  if ( verticalScrollBar() )
    kdDebugTime() << "Log " << width() << " " << height() << " " << verticalScrollBar() -> value() << " / " << verticalScrollBar() -> maximum() << "\n";
#endif
  KTextEdit::resizeEvent (event);
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "                 " << event -> size(). width() << " " << event -> size(). height() << " " << event -> oldSize(). width() << " " << event -> oldSize(). height() << " at_bottom " << at_bottom << "\n";
  if ( verticalScrollBar() )
    kdDebugTime() << "Log " << width() << " " << height() << " " << verticalScrollBar() -> value() << " / " << verticalScrollBar() -> maximum() << "\n";
#endif
  if ( at_bottom && verticalScrollBar() && verticalScrollBar() -> value() != verticalScrollBar() -> maximum() )
  {
    //scrollToBottom();
    verticalScrollBar() -> setValue (verticalScrollBar() -> maximum());
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log: scrolling to bottom\n";
    if ( verticalScrollBar() )
      kdDebugTime() << "Log " << width() << " " << height() << " " << verticalScrollBar() -> value() << " / " << verticalScrollBar() -> maximum() << "\n";
#endif
  }
}

void KPlayerLogWidget::contextMenuEvent (QContextMenuEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerLogWidget::contextMenuEvent\n";
#endif
  KTextEdit::contextMenuEvent (event);
  m_popup -> popup (event -> globalPos());
  event -> accept();
}
