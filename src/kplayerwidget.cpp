/***************************************************************************
                          kplayerwidget.cpp
                          -----------------
    begin                : Sun Dec 01 2002
    copyright            : (C) 2002-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kplayerwidget.h"
#include "kplayerengine.h"
#include "kplayerprocess.h"
#include "kplayersettings.h"

#include <KCursor>
#include <KLocale>
#include <QMouseEvent>
#include <QLayout>
#include <QTimer>

#ifdef DEBUG
#define DEBUG_KPLAYER_WIDGET
#define DEBUG_KPLAYER_WORKSPACE
#endif

void KPlayerX11SetInputFocus (uint id);
void KPlayerX11MapWindow (uint id);
void KPlayerX11UnmapWindow (uint id);

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

KPlayerWidget::KPlayerWidget (QWidget *parent)
  : QWidget (parent)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Creating widget\n";
#endif
  connect (kPlayerProcess(), SIGNAL (stateChanged (KPlayerProcess::State, KPlayerProcess::State)), SLOT (playerStateChanged (KPlayerProcess::State, KPlayerProcess::State)));
  setWhatsThis (i18n("Video area is the central part of KPlayer. When playing a file that has video, it will display the video and optionally subtitles. Normally it will be hidden when playing an audio only file."));
  //setAttribute (Qt::WA_OpaquePaintEvent);
  setFocusPolicy (Qt::NoFocus);
  //setEnabled (false);
  //setEraseColor (QColor (0, 0, 0));
  setPalette (Qt::black);
  setAutoFillBackground (true);
  setMinimumSize (QSize (0, 0));
  setSizePolicy (QSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed));
}

KPlayerWidget::~KPlayerWidget()
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Destroying widget\n";
#endif
}

void KPlayerWidget::mapHandler (uint wid)
{
  if ( wid == winId() )
  {
#ifdef DEBUG_KPLAYER_WIDGET
    kdDebugTime() << "Mapping " << wid << " process state " << kPlayerProcess() -> state() << "\n";
#endif
    KPlayerX11UnmapWindow (winId());
    show();
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
  }
}

void KPlayerWidget::showEvent (QShowEvent* event)
{
  QWidget::showEvent (event);
  //if ( kPlayerProcess() -> is09Version() )
}

void KPlayerWidget::resizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "Widget " << event -> oldSize().width() << "x" << event -> oldSize().height()
    << " => " << event -> size().width() << "x" << event -> size().height() << "\n";
#endif
  QWidget::resizeEvent (event);
}

void KPlayerWidget::mouseMoveEvent (QMouseEvent* event)
{
  QWidget::mouseMoveEvent (event);
  event -> ignore();
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
    << " global " << event -> globalX() << "x" << event -> globalY() << " modifiers " << event -> modifiers() << " orientation "
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
}

QSize KPlayerWidget::sizeHint (void) const
{
  return kPlayerSettings() -> adjustedDisplaySize();
}

KPlayerWorkspace::KPlayerWorkspace (QWidget* parent)
  : QWidget (parent), m_timer (this)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Creating workspace\n";
#endif
  m_mouse_activity = false;
  m_widget = new KPlayerWidget (this);
  setLayout (new QHBoxLayout);
  layout() -> setContentsMargins (0, 0, 0, 0);
  layout() -> addWidget (m_widget);
  m_timer.setSingleShot (true);
  connect (&m_timer, SIGNAL (timeout()), SLOT (cursorTimeout()));
  connect (kPlayerProcess(), SIGNAL (stateChanged (KPlayerProcess::State, KPlayerProcess::State)), SLOT (playerStateChanged (KPlayerProcess::State, KPlayerProcess::State)));
  connect (kPlayerProcess(), SIGNAL (sizeAvailable()), SLOT (setMouseCursorTracking()));
  setWhatsThis (i18n("Video area is the central part of KPlayer. When playing a file that has video, it will display the video and optionally subtitles. Normally it will be hidden when playing an audio only file."));
  //setEraseColor (QColor (0, 0, 0));
  setPalette (Qt::black);
  setAutoFillBackground (true);
  setMinimumSize (QSize (0, 0));
  setFocusPolicy (Qt::StrongFocus);
  QWidget* proxy = new QWidget (parent);
  //proxy -> setEraseColor (QColor (0, 0, 0));
  proxy -> setFocusPolicy (Qt::StrongFocus);
  proxy -> setGeometry (-4, -4, 1, 1);
  proxy -> lower();
  proxy -> show();
  setFocusProxy (proxy);
  m_hidden_widget = new QWidget (this);
  m_hidden_widget -> setGeometry (-10, -10, 5, 5);
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

void KPlayerWorkspace::resizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "WSpace " << event -> oldSize(). width() << "x" << event -> oldSize(). height()
    << " => " << event -> size(). width() << "x" << event -> size(). height() << "\n";
#endif
  QWidget::resizeEvent (event);
  emit resized();
}

void KPlayerWorkspace::mouseMoveEvent (QMouseEvent* event)
{
  QWidget::mouseMoveEvent (event);
  mouseActivity();
}

void KPlayerWorkspace::contextMenuEvent (QContextMenuEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "KPlayerWorkspace context menu\n";
#endif
  emit contextMenu (event -> globalPos());
  event -> accept();
}

void KPlayerWorkspace::mousePressEvent (QMouseEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "KPlayerWorkspace mouse press event\n";
#endif
  QWidget::mousePressEvent (event);
  if ( kPlayerEngine() -> light() && event -> button() == Qt::RightButton )
  {
    emit contextMenu (event -> globalPos());
    event -> accept();
  }
  else
    event -> ignore();
  mouseActivity();
}

void KPlayerWorkspace::mouseReleaseEvent (QMouseEvent* event)
{
#ifdef DEBUG_KPLAYER_WIDGET
  kdDebugTime() << "KPlayerWorkspace mouse release event\n";
#endif
  QWidget::mouseReleaseEvent (event);
  event -> ignore();
  mouseActivity();
}

void KPlayerWorkspace::mouseDoubleClickEvent (QMouseEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "KPlayerWorkspace mouse double click\n";
#endif
  QWidget::mouseDoubleClickEvent (event);
  kPlayerEngine() -> doubleClick();
  event -> accept();
  mouseActivity();
}

void KPlayerWorkspace::wheelEvent (QWheelEvent* event)
{
#ifdef DEBUG_KPLAYER_WORKSPACE
  kdDebugTime() << "Workspace wheel event: delta " << event -> delta() << " position " << event -> x() << "x" << event -> y()
    << " global " << event -> globalX() << "x" << event -> globalY() << " modifiers " << event -> modifiers() << " orientation "
    << event -> orientation() << " accepted " << event -> isAccepted() << " spontaneous " << event -> spontaneous() << "\n";
#endif
  QWidget::wheelEvent (event);
  kPlayerEngine() -> wheel (event -> delta(), event -> modifiers());
  event -> accept();
  mouseActivity();
}

void KPlayerWorkspace::changeEvent (QEvent* event)
{
  QWidget::changeEvent (event);
  if ( event -> type() == QEvent::ActivationChange )
  {
    bool active = isActiveWindow();
#ifdef DEBUG_KPLAYER_WORKSPACE
    kdDebugTime() << "Workspace activation " << active << "\n";
#endif
    if ( active && focusProxy() )
      KPlayerX11SetInputFocus (focusProxy() -> winId());
#ifdef DEBUG_KPLAYER_WORKSPACE
    else if ( active )
      kdDebugTime() << "  no focus proxy\n";
#endif
  }
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

void KPlayerWorkspace::mouseActivity (void)
{
  m_mouse_activity = true;
  setMouseCursor();
  m_timer.start (1000);
}

void KPlayerWorkspace::setMouseCursor (void)
{
  if ( ! m_mouse_activity && kPlayerEngine() -> process() -> state() == KPlayerProcess::Playing
      && kPlayerEngine() -> properties() -> hasVideo() )
  {
    setCursor (Qt::BlankCursor);
    widget() -> setCursor (Qt::BlankCursor);
  }
  else
  {
    unsetCursor();
    widget() -> unsetCursor();
  }
}

void KPlayerWorkspace::cursorTimeout (void)
{
  m_mouse_activity = false;
  setMouseCursor();
}

void KPlayerWorkspace::setMouseCursorTracking (void)
{
  setMouseCursor();
  bool track = kPlayerProcess() -> state() == KPlayerProcess::Playing && kPlayerEngine() -> properties() -> hasVideo();
  setMouseTracking (track);
  widget() -> setMouseTracking (track);
}

void KPlayerWorkspace::playerStateChanged (KPlayerProcess::State, KPlayerProcess::State)
{
  setMouseCursorTracking();
}

QSize KPlayerWorkspace::sizeHint (void) const
{
  return kPlayerSettings() -> displaySize();
}
