/***************************************************************************
                          kplayerlogwindow.cpp
                          --------------------
    begin                : Fri May 9 2003
    copyright            : (C) 2003-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>
#include <qlayout.h>

#include "kplayerlogwindow.h"
#include "kplayerlogwindow.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

#define DEBUG_KPLAYER_LOG

KPlayerLogWindow::KPlayerLogWindow (QWidget* parent, const char* name)
  : QDockWindow (parent, name)
{
  setWidget (new KPlayerLogWidget (this));
  setResizeEnabled (true);
  setCloseMode (QDockWindow::Always);
  setNewLine (true);
  setCaption (i18n("Messages"));
}

void KPlayerLogWindow::setError (bool flag)
{
  KPlayerLogWidget* log = kPlayerLogWidget();
  if ( flag && ! log -> hasError() && ! isVisible() && log -> width() < 300 )
  {
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log resizing widget\n";
    kdDebugTime() << "Log widget " << log -> width() << "x" << log -> height() << "\n";
#endif
    log -> resize (kPlayerSettings() -> minimumInitialWidth(), log -> height());
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log widget " << log -> width() << "x" << log -> height() << "\n";
#endif
  }
  kPlayerLogWidget() -> setError (flag);
}

void KPlayerLogWindow::hideEvent (QHideEvent* event)
{
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::hideEvent\n";
#endif
  QDockWindow::hideEvent (event);
  if ( isHidden() ) // || ! event -> spontaneous() )
    emit windowHidden();
}

KPlayerLogWidget::KPlayerLogWidget (QWidget* parent, const char* name)
  : KTextEdit (parent, name)
{
  setTextFormat (Qt::PlainText);
  setReadOnly (true);
  setError (false);
}

void KPlayerLogWidget::clear (void)
{
  KTextEdit::clear();
  setError (false);
}

void KPlayerLogWidget::setError (bool flag)
{
  if ( ! flag )
    m_location = 0;
  else if ( ! m_error )
  {
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log::stopScrolling\n";
    kdDebugTime() << "Log " << width() << " " << height() << " " << contentsWidth() << "x" << contentsHeight() << " " << visibleWidth() << "x" << visibleHeight() << " " << contentsX() << ":" << contentsY() << "\n";
#endif
    scrollToBottom();
    scrollBy (0, -1);
    m_location = isVisible() ? 0 : contentsY();
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log " << width() << " " << height() << " " << contentsWidth() << "x" << contentsHeight() << " " << visibleWidth() << "x" << visibleHeight() << " " << contentsX() << ":" << contentsY() << "\n";
#endif
  }
  m_error = flag;
}

void KPlayerLogWidget::showEvent (QShowEvent* event)
{
  KTextEdit::showEvent (event);
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::showEvent\n";
  kdDebugTime() << "Log " << width() << " " << height() << " " << contentsWidth() << "x" << contentsHeight() << " " << visibleWidth() << "x" << visibleHeight() << " " << contentsX() << ":" << contentsY() << "\n";
#endif
  if ( ! m_error )
    scrollToBottom();
  else if ( m_location )
  {
    sync();
    setContentsPos (0, m_location);
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log scrolling to " << m_location << "\n";
    kdDebugTime() << "Log " << width() << " " << height() << " " << contentsWidth() << "x" << contentsHeight() << " " << visibleWidth() << "x" << visibleHeight() << " " << contentsX() << ":" << contentsY() << "\n";
#endif
    m_location = 0;
  }
}

void KPlayerLogWidget::resizeEvent (QResizeEvent* event)
{
  bool at_bottom = contentsHeight() == contentsY() + visibleHeight();
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "Log::resizeEvent " << event -> size(). width() << " " << event -> size(). height() << " " << event -> oldSize(). width() << " " << event -> oldSize(). height() << " at_bottom " << at_bottom << "\n";
  kdDebugTime() << "Log " << width() << " " << height() << " " << contentsWidth() << "x" << contentsHeight() << " " << visibleWidth() << "x" << visibleHeight() << " " << contentsX() << ":" << contentsY() << "\n";
#endif
  KTextEdit::resizeEvent (event);
#ifdef DEBUG_KPLAYER_LOG
  kdDebugTime() << "                 " << event -> size(). width() << " " << event -> size(). height() << " " << event -> oldSize(). width() << " " << event -> oldSize(). height() << " at_bottom " << at_bottom << "\n";
  kdDebugTime() << "Log " << width() << " " << height() << " " << contentsWidth() << "x" << contentsHeight() << " " << visibleWidth() << "x" << visibleHeight() << " " << contentsX() << ":" << contentsY() << "\n";
#endif
  if ( at_bottom && contentsHeight() != contentsY() + visibleHeight() )
  {
    scrollToBottom();
#ifdef DEBUG_KPLAYER_LOG
    kdDebugTime() << "Log: scrolling to bottom\n";
    kdDebugTime() << "Log " << width() << " " << height() << " " << contentsWidth() << "x" << contentsHeight() << " " << visibleWidth() << "x" << visibleHeight() << " " << contentsX() << ":" << contentsY() << "\n";
#endif
  }
}
