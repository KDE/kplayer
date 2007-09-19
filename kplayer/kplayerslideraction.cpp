/***************************************************************************
                          kplayerslideraction.cpp
                          -----------------------
    begin                : Sat Jan 11 2003
    copyright            : (C) 2003-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kapplication.h>
#include <ktoolbar.h>
#include <qdesktopwidget.h>

#include <qcursor.h>
#include <qevent.h>
#include <qlayout.h>
#include <qtooltip.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_SLIDERS
#define DEBUG_KPLAYER_SLIDER_HINTS
#endif

#include "kplayerslideraction.h"
#include "kplayerslideraction.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

KPlayerPopupFrame::KPlayerPopupFrame (QWidget* parent)
  : QFrame (parent, Qt::Popup)
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupFrame created\n";
#endif
  setLayout (new QHBoxLayout);
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

KPlayerPopupSliderAction::KPlayerPopupSliderAction (QObject* parent)
  : KAction (parent)
{
  m_frame = new KPlayerPopupFrame;
  m_frame -> setFrameStyle (QFrame::StyledPanel);
  m_frame -> setFrameShadow (QFrame::Raised);
  m_frame -> setLineWidth (2);
  m_slider = new KPlayerSlider (Qt::Vertical);
  m_frame -> layout() -> addWidget (m_slider);
  m_frame -> resize (36, m_slider -> sizeHint().height() + 4);
  m_slider -> setGeometry (m_frame -> contentsRect());
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Popup slider size " << m_slider -> width() << "x" << m_slider -> height() << "\n";
#endif
  connect (this, SIGNAL (triggered()), SLOT (slotActivated()));
}

KPlayerPopupSliderAction::~KPlayerPopupSliderAction()
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerPopupSliderAction destroyed\n";
#endif
}

void KPlayerPopupSliderAction::slotActivated (void)
{
  QWidget* button = defaultWidget();
#ifdef DEBUG_KPLAYER_SLIDERS
  if ( sender() )
    kdDebugTime() << "Sender class name: " << sender() -> metaObject() -> className() << "\n";
  if ( defaultWidget() )
    kdDebugTime() << "Widget class name: " << defaultWidget() -> metaObject() -> className() << "\n";
#endif
  /*if ( sender() && sender() -> inherits ("KToolBarButton") )
    button = (QWidget*) sender();
  else
  {
    KToolBar* toolbar = 0;
    int index;
    if ( sender() && sender() -> inherits ("KToolBar") )
      toolbar = (KToolBar*) sender();
    else
      for ( index = 0; index < associatedWidgets().count(); index ++ )
      {
        QWidget* container = associatedWidgets().value (index);
        if ( container && container -> isVisible() && container -> inherits ("KToolBar") )
        {
          toolbar = (KToolBar*) container;
          break;
        }
      }
    if ( toolbar && toolbar -> isVisible() )
    {
      int index = findContainer (toolbar);
      if ( index >= 0 )
        button = toolbar -> getButton (itemId (index));
    }
  }*/
  QPoint point;
  if ( button )
  {
    point = button -> mapToGlobal (QPoint (0, button -> height()));
    if ( point.y() + m_frame -> height() > QApplication::desktop() -> height() )
      point.setY (point.y() - button -> height() - m_frame -> height());
  }
  else
  {
    point = QCursor::pos() - QPoint (m_frame -> width() / 2, m_frame -> height() / 2);
    if ( point.x() + m_frame -> width() > QApplication::desktop() -> width() )
      point.setX (QApplication::desktop() -> width() - m_frame -> width());
    if ( point.y() + m_frame -> height() > QApplication::desktop() -> height() )
      point.setY (QApplication::desktop() -> height() - m_frame -> height());
    if ( point.x() < 0 )
      point.setX (0);
    if ( point.y() < 0 )
      point.setY (0);
  }
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Point: " << point.x() << "x" << point.y() << "\n";
#endif
  m_frame -> setWhatsThis (whatsThis());
  m_slider -> setWhatsThis (whatsThis());
  m_frame -> move (point);
  m_frame -> show();
  m_slider -> setFocus();
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

/*int KPlayerSliderAction::plug (QWidget* widget, int index)
{
  int result = K3WidgetAction::plug (widget, index);
  if ( result < 0 )
    return result;
  KToolBar* toolbar = (KToolBar*) widget;
  if ( ! text().isEmpty() )
    QToolTip::add (slider(), text());
  orientationChanged (toolbar -> orientation());
  connect (toolbar, SIGNAL (orientationChanged (Orientation)), this, SLOT (orientationChanged (Orientation)));
  return result;
}

void KPlayerSliderAction::unplug (QWidget* widget)
{
  K3WidgetAction::unplug (widget);
  if ( ! slider() || ! isPlugged() || widget != slider() -> parent() )
    return;
  disconnect (widget, SIGNAL (orientationChanged (Orientation)), this, SLOT (orientationChanged (Orientation)));
}

void KPlayerSliderAction::orientationChanged (Qt::Orientation orientation)
{
  if ( slider() )
    slider() -> setOrientation (orientation);
}*/

KPlayerSlider::KPlayerSlider (Qt::Orientation orientation, QWidget* parent)
  : QSlider (orientation, parent)
{
  m_dragging = m_changing_orientation = false;
  connect (this, SIGNAL (valueChanged (int)), SLOT (sliderValueChanged (int)));
}

KPlayerSlider::~KPlayerSlider()
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerSlider destroyed\n";
#endif
}

QSize KPlayerSlider::sizeHint() const
{
#ifdef DEBUG_KPLAYER_SLIDER_HINTS
  kdDebugTime() << "KPlayerSlider::sizeHint\n";
#endif
  QSize hint = QSlider::sizeHint();
  if ( KPlayerEngine::engine() )
  {
    int length = KPlayerEngine::engine() -> configuration() -> preferredSliderLength();
    if ( orientation() == Qt::Horizontal )
    {
      if ( hint.width() < length )
        hint.setWidth (length);
    }
    else
    {
      if ( hint.height() < length )
        hint.setHeight (length);
    }
  }
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
  {
    int length = KPlayerEngine::engine() -> configuration() -> minimumSliderLength();
    if ( orientation() == Qt::Horizontal )
    {
      if ( hint.width() < length )
        hint.setWidth (length);
    }
    else
    {
      if ( hint.height() < length )
        hint.setHeight (length);
    }
  }
#ifdef DEBUG_KPLAYER_SLIDER_HINTS
  kdDebugTime() << " Hint   " << hint.width() << "x" << hint.height() << "\n";
#endif
  return hint;
}

void KPlayerSlider::setOrientation (Qt::Orientation o)
{
  if ( o == orientation() )
    return;
  m_changing_orientation = true;
  int minimum = QSlider::minimum();
  int maximum = QSlider::maximum();
  int value = QSlider::value();
  QSlider::setOrientation (o);
  QSlider::setMinimum (- maximum);
  QSlider::setMaximum (- minimum);
  QSlider::setValue (- value);
  m_changing_orientation = false;
}

int KPlayerSlider::minValue (void) const
{
  if ( orientation() == Qt::Horizontal )
    return QSlider::minimum();
  return - QSlider::maximum();
}

void KPlayerSlider::setMinValue (int minimum)
{
  if ( orientation() == Qt::Horizontal )
    QSlider::setMinimum (minimum);
  else
    QSlider::setMaximum (- minimum);
}

int KPlayerSlider::maxValue (void) const
{
  if ( orientation() == Qt::Horizontal )
    return QSlider::maximum();
  return - QSlider::minimum();
}

void KPlayerSlider::setMaxValue (int maximum)
{
  if ( orientation() == Qt::Horizontal )
    QSlider::setMaximum (maximum);
  else
    QSlider::setMinimum (- maximum);
}

int KPlayerSlider::value (void) const
{
  if ( orientation() == Qt::Horizontal )
    return QSlider::value();
  return - QSlider::value();
}

void KPlayerSlider::setValue (int value)
{
  if ( orientation() == Qt::Horizontal )
    QSlider::setValue (value);
  else
    QSlider::setValue (- value);
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

void KPlayerSlider::sliderValueChanged (int)
{
  if ( ! m_changing_orientation )
    emit changed (value());
}

void KPlayerSlider::mousePressEvent (QMouseEvent* event)
{
  m_dragging = (event -> buttons() & (Qt::LeftButton | Qt::MidButton)) != 0;
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Popup slider mouse press " << event -> modifiers() << " -> " << m_dragging << "\n";
#endif
  QSlider::mousePressEvent (event);
}

void KPlayerSlider::mouseReleaseEvent (QMouseEvent* event)
{
  m_dragging = (event -> buttons() & (Qt::LeftButton | Qt::MidButton)) != 0;
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Popup slider mouse release " << event -> modifiers() << " -> " << m_dragging << "\n";
#endif
  QSlider::mouseReleaseEvent (event);
}

void KPlayerSlider::keyPressEvent (QKeyEvent* event)
{
  int key = event -> key();
  if ( key == Qt::Key_Left || key == Qt::Key_Right
    || (event -> modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) != 0
    && (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_PageUp || key == Qt::Key_PageDown
    || key == Qt::Key_Home || key == Qt::Key_End) )
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
