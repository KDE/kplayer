/***************************************************************************
                          kplayerwidget.cpp
                          -----------------
    begin                : Sun Dec 01 2002
    copyright            : (C) 2002-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klocale.h>
#include <qwhatsthis.h>

#include <kdebug.h>

#include "kplayerwidget.h"
#include "kplayerwidget.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

void KPlayerX11SetInputFocus (uint id);
void KPlayerX11MapWindow (uint id);
void KPlayerX11UnmapWindow (uint id);
void KPlayerX11ClearExposeWindow (uint id);
void KPlayerX11SendConfigureEvent (uint id, int w, int h);
void KPlayerSetX11EventFilter (void);
void KPlayerResetX11EventFilter (void);

#define DEBUG_KPLAYER_WIDGET
#define DEBUG_KPLAYER_WORKSPACE

// WM_NORMAL_HINTS, XSizeHints, XSetWMNormalHints() ???

void KPlayerWidgetResizeHandler (bool resizing)
{
  if ( kPlayerWorkspace() )
    kPlayerWorkspace() -> resizeHandler (resizing);
}

void KPlayerWidgetMapHandler (uint wid)
{
  if ( kPlayerWidget() )
    kPlayerWidget() -> mapHandler (wid);
}

void KPlayerWidgetUnmapHandler (uint wid)
{
  if ( kPlayerWidget() )
    kPlayerWidget() -> unmapHandler (wid);
}

KPlayerWidget::KPlayerWidget (QWidget *parent, const char *name)
  : QWidget (parent, name)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Creating widget\n";
#endif
  connect (kPlayerProcess(), SIGNAL (stateChanged (KPlayerProcess::State, KPlayerProcess::State)), this, SLOT (playerStateChanged (KPlayerProcess::State, KPlayerProcess::State)));
  QWhatsThis::add (this, i18n("Video area is the central part of KPlayer. When playing a file that has video, it will display the video and optionally subtitles. Normally it will be hidden when playing an audio only file."));
  setFocusPolicy (QWidget::NoFocus);
  //setEnabled (false);
  setEraseColor (QColor (0, 0, 0));
  setMinimumSize (QSize (0, 0));
  KPlayerSetX11EventFilter();
}

KPlayerWidget::~KPlayerWidget()
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Destroying widget\n";
#endif
  KPlayerResetX11EventFilter();
}

void KPlayerWidget::sendConfigureEvent (void)
{
  KPlayerX11SendConfigureEvent (winId(), width(), height());
}

void KPlayerWidget::mapHandler (uint wid)
{
  if ( wid == winId() )
  {
#ifdef DEBUG_KPLAYER_WIDGET
    kdDebugTime() << "Mapping; process state " << kPlayerProcess() -> state() << "\n";
#endif
    //if ( ! kPlayerProcess() -> is09Version() )
    KPlayerX11UnmapWindow (winId());
    show();
    //if ( ! kPlayerProcess() -> is09Version() )
    KPlayerX11MapWindow (winId());
  }
}

void KPlayerWidget::unmapHandler (uint wid)
{
  if ( wid == winId() )
  {
#ifdef DEBUG_KPLAYER_WIDGET
    kdDebugTime() << "Unmapping " << wid << "; process state " << kPlayerProcess() -> state() << "\n";
#endif
    hide();
    KPlayerX11MapWindow (winId());
    //if ( false && kPlayerProcess() -> is09Version() )
    //  KPlayerX11ClearExposeWindow (winId());
    sendConfigureEvent();
  }
}

void KPlayerWidget::showEvent (QShowEvent* event)
{
  QWidget::showEvent (event);
  //if ( kPlayerProcess() -> is09Version() )
  sendConfigureEvent();
}

void KPlayerWidget::resizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Widget " << event -> oldSize().width() << "x" << event -> oldSize().height()
    << " => " << event -> size().width() << "x" << event -> size().height() << "\n";
#endif
  QWidget::resizeEvent (event);
/*if ( kPlayerProcess() -> is09Version() )
  {
    KPlayerX11UnmapWindow (winId());
    KPlayerX11MapWindow (winId());
  }*/
  /*if ( false && kPlayerProcess() -> is09Version() )
  {
    KPlayerX11UnmapWindow (winId());
    KPlayerX11MapWindow (winId());
    KPlayerX11ClearExposeWindow (winId());
    KPlayerX11SendConfigureEvent (winId(), event -> size().width(), event -> size().height());
  }*/
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Widget resize; process state " << kPlayerProcess() -> state() << "\n";
#endif
}

void KPlayerWidget::mousePressEvent (QMouseEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "KPlayerWidget mouse press event\n";
#endif
  QWidget::mousePressEvent (event);
  event -> ignore();
}

void KPlayerWidget::mouseReleaseEvent (QMouseEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "KPlayerWidget mouse release event\n";
#endif
  QWidget::mouseReleaseEvent (event);
  event -> ignore();
}

void KPlayerWidget::mouseDoubleClickEvent (QMouseEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "KPlayerWidget mouse double click\n";
#endif
  QWidget::mouseDoubleClickEvent (event);
  event -> ignore();
}

void KPlayerWidget::contextMenuEvent (QContextMenuEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "KPlayerWidget context menu event\n";
#endif
  QWidget::contextMenuEvent (event);
  event -> ignore();
}

void KPlayerWidget::wheelEvent (QWheelEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Widget wheel event: delta " << event -> delta() << " position " << event -> x() << "x" << event -> y()
    << " global " << event -> globalX() << "x" << event -> globalY() << " state " << event -> state() << " orientation "
    << event -> orientation() << " accepted " << event -> isAccepted() << " spontaneous " << event -> spontaneous() << "\n";
#endif
  QWidget::wheelEvent (event);
  event -> ignore();
}

void KPlayerWidget::focusInEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Widget focus in event\n";
#endif
  QWidget::focusInEvent (event);
}

void KPlayerWidget::focusOutEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Widget focus out event\n";
#endif
  QWidget::focusOutEvent (event);
}

void KPlayerWidget::playerStateChanged (KPlayerProcess::State state, KPlayerProcess::State)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "State change received by the widget: " << state << "\n";
#endif
  /*if ( false && kPlayerProcess() -> is09Version() && state == KPlayerProcess::Playing )
  {
    KPlayerX11MapWindow (winId());
    KPlayerX11ClearExposeWindow (winId());
  }*/
  if ( kPlayerProcess() -> is09Version() && state == KPlayerProcess::Playing )
    KPlayerX11ClearExposeWindow (winId());
  sendConfigureEvent();
}

KPlayerWorkspace::KPlayerWorkspace (QWidget* parent, const char* name)
  : QWidget (parent, name)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Creating workspace\n";
#endif
  m_resizing = false;
  m_widget = new KPlayerWidget (this);
  QWhatsThis::add (this, i18n("Video area is the central part of KPlayer. When playing a file that has video, it will display the video and optionally subtitles. Normally it will be hidden when playing an audio only file."));
  setEraseColor (QColor (0, 0, 0));
  setMinimumSize (QSize (0, 0));
  setFocusPolicy (QWidget::StrongFocus);
  QWidget* proxy = new QWidget (parent);
  proxy -> setEraseColor (QColor (0, 0, 0));
  proxy -> setFocusPolicy (QWidget::StrongFocus);
  proxy -> setGeometry (0, 0, 1, 1);
  proxy -> lower();
  proxy -> show();
  setFocusProxy (proxy);
}

void KPlayerWorkspace::setDisplaySize (QSize size)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Workspace::setDisplaySize (" << size.width() << "x" << size.height() << ")\n";
  kdDebugTime() << "Workspace::Widget size " << m_widget -> width() << "x" << m_widget -> height() << "\n";
#endif
  //if ( m_widget -> size() != size )
  m_widget -> setGeometry ((width() - size.width()) / 2, (height() - size.height()) / 2, size.width(), size.height());
}

void KPlayerWorkspace::resizeHandler (bool resizing)
{
  if ( m_resizing == resizing )
    return;
  m_resizing = resizing;
  if ( ! resizing )
    QApplication::postEvent (this, new QResizeEvent (size(), size()));
}

void KPlayerWorkspace::resizeEvent (QResizeEvent* event)
{
  static bool recursion = false;
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "WSpace " << event -> oldSize(). width() << "x" << event -> oldSize(). height()
    << " => " << event -> size(). width() << "x" << event -> size(). height() << "\n";
#endif
  QWidget::resizeEvent (event);
  if ( ! recursion && ! m_resizing )
  {
    recursion = true;
    emit resized();
    recursion = false;
  }
}

void KPlayerWorkspace::contextMenuEvent (QContextMenuEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "KPlayerWorkspace context menu\n";
#endif
  emit contextMenu (event);
  event -> accept();
}

void KPlayerWorkspace::mouseDoubleClickEvent (QMouseEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "KPlayerWorkspace mouse double click\n";
#endif
  QWidget::mouseDoubleClickEvent (event);
  kPlayerEngine() -> doubleClick();
  event -> accept();
}

void KPlayerWorkspace::wheelEvent (QWheelEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Workspace wheel event: delta " << event -> delta() << " position " << event -> x() << "x" << event -> y()
    << " global " << event -> globalX() << "x" << event -> globalY() << " state " << event -> state() << " orientation "
    << event -> orientation() << " accepted " << event -> isAccepted() << " spontaneous " << event -> spontaneous() << "\n";
#endif
  QWidget::wheelEvent (event);
  kPlayerEngine() -> wheel (event -> delta(), event -> state());
  event -> accept();
}

void KPlayerWorkspace::windowActivationChange (bool old)
{
  QWidget::windowActivationChange (old);
  bool active = isActiveWindow();
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Workspace activation " << old << " -> " << active << "\n";
#endif
  if ( active && focusProxy() )
    KPlayerX11SetInputFocus (focusProxy() -> winId());
#ifdef DEBUG_KPLAYER_WORKSPACE
  else if ( active )
    kdDebugTime() << "  no focus proxy\n";
#endif
}

void KPlayerWorkspace::focusInEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Workspace focus in event\n";
#endif
  QWidget::focusInEvent (event);
}

void KPlayerWorkspace::focusOutEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Workspace focus out event\n";
#endif
  QWidget::focusOutEvent (event);
}
