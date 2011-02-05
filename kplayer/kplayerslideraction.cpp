/***************************************************************************
                          kplayerslideraction.cpp
                          -----------------------
    begin                : Sat Jan 11 2003
    copyright            : (C) 2003-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kplayerslideraction.h"
#include "kplayerengine.h"
#include "kplayersettings.h"

#include <KApplication>
#include <KToolBar>
#include <QByteArray>
#include <QCursor>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QLayout>
#include <QStyle>

#ifdef DEBUG
#define DEBUG_KPLAYER_SLIDERS
#define DEBUG_KPLAYER_SLIDER_HINTS
#endif

KPlayerPopupFrame::KPlayerPopupFrame (QWidget* parent)
  : QFrame (parent, Qt::Popup)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupFrame created\n";
#endif
  setFrameStyle (QFrame::Panel);
  setFrameShadow (QFrame::Raised);
  setLineWidth (2);
  setLayout (new QHBoxLayout);
  layout() -> setContentsMargins (0, 0, 0, 0);
}

KPlayerPopupFrame::~KPlayerPopupFrame()
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupFrame destroyed\n";
#endif
}

void KPlayerPopupFrame::keyPressEvent (QKeyEvent* ev)
{
  switch ( ev -> key() )
  {
    case Qt::Key_Alt:
    case Qt::Key_Tab:
    case Qt::Key_Escape:
    case Qt::Key_Return:
    case Qt::Key_Enter:
      close();
  }
}

KPlayerPopupToolButton::KPlayerPopupToolButton (QWidget* parent)
  : QToolButton (parent)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupToolButton created\n";
#endif
}

void KPlayerPopupToolButton::nextCheckState (void)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Popup button clicked\n";
#endif
}

KPlayerPopupSliderAction::KPlayerPopupSliderAction (QObject* parent)
  : KAction (parent)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupSliderAction created\n";
#endif
  m_frame = new KPlayerPopupFrame;
  m_slider = new KPlayerSlider (Qt::Vertical, m_frame);
  m_frame -> layout() -> addWidget (m_slider);
  m_frame -> resize (32, 200);
  connect (this, SIGNAL (triggered()), SLOT (showSlider()));
}

KPlayerPopupSliderAction::~KPlayerPopupSliderAction()
{
  delete m_frame;
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupSliderAction destroyed\n";
#endif
}

QWidget* KPlayerPopupSliderAction::createWidget (QWidget* parent)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupSliderAction::createWidget\n";
#endif
  if ( parent && parent -> inherits ("QToolBar") )
  {
    QToolBar* toolbar = (QToolBar*) parent;
    QToolButton* button = new KPlayerPopupToolButton (toolbar);
    button -> setAutoRaise (true);
    button -> setFocusPolicy (Qt::NoFocus);
    button -> setIconSize (toolbar -> iconSize());
    button -> setToolButtonStyle (toolbar -> toolButtonStyle());
    button -> connect (toolbar, SIGNAL (iconSizeChanged (QSize)), SLOT (setIconSize (QSize)));
    button -> connect (toolbar, SIGNAL (toolButtonStyleChanged (Qt::ToolButtonStyle)),
      SLOT (setToolButtonStyle (Qt::ToolButtonStyle)));
    button -> setDefaultAction (this);
    connect (button, SIGNAL (clicked()), SLOT (showSlider()));
    return button;
  }
  return KAction::createWidget (parent);
}

void KPlayerPopupSliderAction::showSlider (void)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupSliderAction::showSlider\n";
  if ( sender() )
    kdDebugTime() << " Sender " << sender() -> metaObject() -> className() << "\n";
#endif
  QPoint point;
  int width = m_frame -> width();
  int height = KPlayerEngine::engine() -> configuration() -> preferredSliderLength() + 4;
  QToolButton* button = 0;
  if ( sender() && sender() -> inherits ("QToolButton") )
    button = (QToolButton*) sender();
  else
    foreach ( QWidget* widget, createdWidgets() )
      if ( widget -> isVisible() && widget -> inherits ("QToolButton") )
        button = (QToolButton*) widget;
  if ( button )
  {
    point = button -> mapToGlobal (QPoint (-2, button -> height()));
    if ( point.y() + height > QApplication::desktop() -> height() )
      point.setY (point.y() - button -> height() - height);
    width = button -> width() + 4;
  }
  else
  {
    point = QCursor::pos() - QPoint (width / 2, height / 2);
    if ( point.x() + width > QApplication::desktop() -> width() )
      point.setX (QApplication::desktop() -> width() - width);
    if ( point.y() + height > QApplication::desktop() -> height() )
      point.setY (QApplication::desktop() -> height() - height);
    if ( point.x() < 0 )
      point.setX (0);
    if ( point.y() < 0 )
      point.setY (0);
  }
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << " Point  " << point.x() << "x" << point.y() << "\n";
#endif
  m_frame -> setWhatsThis (whatsThis());
  m_slider -> setWhatsThis (whatsThis());
  m_frame -> resize (width, height);
  m_frame -> move (point);
  m_frame -> show();
  m_slider -> setFocus();
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << " Frame  " << m_frame -> x() << "x" << m_frame -> y() << " " << m_frame -> width() << "x" << m_frame -> height() << "\n";
  kdDebugTime() << " Slider " << m_slider -> x() << "x" << m_slider -> y() << " " << m_slider -> width() << "x" << m_slider -> height() << "\n";
#endif
  point = QPoint();
}

KPlayerSliderAction::KPlayerSliderAction (QObject* parent)
  : QWidgetAction (parent)
{
  setDefaultWidget (new KPlayerSlider (Qt::Horizontal));
  //setAutoSized (true);
  //setShortcutConfigurable (false);
}

KPlayerSliderAction::~KPlayerSliderAction()
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerSliderAction destroyed\n";
#endif
}

QWidget* KPlayerSliderAction::createWidget (QWidget* parent)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerSliderAction::createWidget\n";
#endif
  if ( slider() -> parent() )
    disconnect (slider() -> parent(), SIGNAL (orientationChanged (Qt::Orientation)),
      slider(), SLOT (parentOrientationChanged (Qt::Orientation)));
  connect (parent, SIGNAL (orientationChanged (Qt::Orientation)),
    slider(), SLOT (parentOrientationChanged (Qt::Orientation)));
  //slider() -> setStatusTip (statusTip());
  //slider() -> setWhatsThis (whatsThis());
  return QWidgetAction::createWidget (parent);
}

KPlayerSlider::KPlayerSlider (Qt::Orientation orientation, QWidget* parent)
  : QSlider (orientation, parent)
{
  //m_dragging = false;
  //m_changing_orientation = false;
  //connect (this, SIGNAL (valueChanged (int)), SLOT (sliderValueChanged (int)));
  setSizePolicy (QSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::Slider));
}

KPlayerSlider::~KPlayerSlider()
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerSlider destroyed\n";
#endif
}

void KPlayerSlider::parentOrientationChanged (Qt::Orientation orientation)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerSlider orientation changed to " << orientation << "\n";
#endif
  setOrientation (orientation);
  setMinimumSize (QSize (0, 0));
  if ( parentWidget() )
  {
    parentWidget() -> setMinimumSize (QSize (0, 0));
    parentWidget() -> adjustSize();
  }
}

void KPlayerSlider::adjustHint (QSize& hint, int length) const
{
  QSize size;
  if ( parent() )
    foreach ( QObject* object, parent() -> children() )
      if ( qstrcmp (object -> metaObject() -> className(), "QToolButton") == 0 )
      {
        size = ((QToolButton*) object) -> sizeHint();
        break;
      }
  if ( orientation() == Qt::Horizontal )
  {
    if ( hint.width() < length )
      hint.setWidth (length);
    if ( size.isValid() )
      hint.setHeight (size.height());
  }
  else
  {
    if ( hint.height() < length )
      hint.setHeight (length);
    if ( size.isValid() )
      hint.setWidth (size.width());
    else
    {
      int width = style() ? style() -> pixelMetric (QStyle::PM_ToolBarIconSize) : 32;
      if ( hint.width() > width )
        hint.setWidth (width);
    }
  }
}

QSize KPlayerSlider::sizeHint() const
{
#ifdef DEBUG_KPLAYER_SLIDER_HINTS
  kdDebugTime() << "KPlayerSlider::sizeHint\n";
#endif
  QSize hint = QSlider::sizeHint();
  if ( KPlayerEngine::engine() )
    adjustHint (hint, KPlayerEngine::engine() -> configuration() -> preferredSliderLength());
#ifdef DEBUG_KPLAYER_SLIDER_HINTS
  kdDebugTime() << " Hint   " << hint.width() << "x" << hint.height() << "\n";
#endif
  return hint;
}

QSize KPlayerSlider::minimumSizeHint() const
{
#ifdef DEBUG_KPLAYER_SLIDER_HINTS
  kdDebugTime() << "KPlayerSlider::minimumSizeHint\n";
#endif
  QSize hint = QSlider::minimumSizeHint();
  if ( KPlayerEngine::engine() )
    adjustHint (hint, KPlayerEngine::engine() -> configuration() -> minimumSliderLength());
#ifdef DEBUG_KPLAYER_SLIDER_HINTS
  kdDebugTime() << " Hint   " << hint.width() << "x" << hint.height() << "\n";
#endif
  return hint;
}

void KPlayerSlider::setup (int minimum, int maximum, int value,
  bool tickMarks, int tickInterval, int pageStep, int singleStep)
{
  setMinimum (minimum);
  setMaximum (maximum);
  setSingleStep (singleStep);
  setPageStep (pageStep);
  setTickPosition (tickMarks ? QSlider::TicksBothSides : QSlider::NoTicks);
  setTickInterval (tickInterval);
  setValue (value);
  updateGeometry();
}

void KPlayerSlider::keyPressEvent (QKeyEvent* event)
{
  int key = event -> key();
  if ( key == Qt::Key_Left || key == Qt::Key_Right
    || ((event -> modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) != 0
    && (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_PageUp || key == Qt::Key_PageDown
    || key == Qt::Key_Home || key == Qt::Key_End) ) )
  {
    event -> ignore();
    return;
  }
  QSlider::keyPressEvent (event);
}

void KPlayerSlider::wheelEvent (QWheelEvent* event)
{
  static int remainder = 0;
  static QSlider* owner = 0;
  if ( owner != this )
  {
    owner = this;
    remainder = 0;
  }
  int delta = event -> delta();
  if ( orientation() != Qt::Horizontal )
    delta = - delta;
  delta *= (event -> modifiers() & Qt::ControlModifier) == Qt::ControlModifier ? pageStep() : singleStep();
  int offset = (delta + remainder + (delta > 0 ? 60 : -60)) / 120;
  remainder += delta - offset * 120;
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Slider wheel event: delta " << event -> delta() << " offset " << offset << " remainder " << remainder
    << " position " << event -> x() << "x" << event -> y() << " global " << event -> globalX() << "x" << event -> globalY()
    << " modifiers " << event -> modifiers() << " orientation " << event -> orientation() << " accepted " << event -> isAccepted()
    << " spontaneous " << event -> spontaneous() << "\n";
#endif
  if ( offset != 0 )
    QSlider::setValue (QSlider::value() + offset);
  event -> accept();
}
