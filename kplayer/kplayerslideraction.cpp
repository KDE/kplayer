/***************************************************************************
                          kplayerslideraction.cpp
                          -----------------------
    begin                : Sat Jan 11 2003
    copyright            : (C) 2003-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kapplication.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <qcursor.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kdebug.h>

#include "kplayerslideraction.h"
#include "kplayerslideraction.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

#define DEBUG_KPLAYER_SLIDERS

void KPlayerPopupFrame::keyPressEvent (QKeyEvent* ev)
{
  switch ( ev -> key() )
  {
    case Key_Alt:
    case Key_Tab:
    case Key_Escape:
    case Key_Return:
    case Key_Enter:
      close();
  }
}

KPlayerPopupSliderAction::KPlayerPopupSliderAction (const QString& text,
    const QString& pix, const KShortcut& shortcut, const QObject* receiver,
    const char* slot, QObject* parent, const char* name)
  : KAction (text, pix, shortcut, parent, name)
{
  m_frame = new KPlayerPopupFrame;
  m_frame -> setFrameStyle (QFrame::PopupPanel | QFrame::Raised);
  m_frame -> setLineWidth (2);
  m_slider = new KPlayerSlider (Qt::Vertical, m_frame);
  m_frame -> resize (36, m_slider -> sizeHint().height() + 4);
  m_slider -> setGeometry (m_frame -> contentsRect());
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Popup slider size " << m_slider -> width() << "x" << m_slider -> height() << "\n";
#endif
  connect (m_slider, SIGNAL (changed (int)), receiver, slot);
  if ( ! text.isEmpty() )
    QToolTip::add (m_slider, text);
}

KPlayerPopupSliderAction::~KPlayerPopupSliderAction()
{
  delete m_frame;
  m_frame = 0;
}

void KPlayerPopupSliderAction::slotActivated (void)
{
  KAction::slotActivated();
  QWidget* button = 0;
#ifdef DEBUG_KPLAYER_SLIDERS
  if ( sender() )
    kdDebugTime() << "Sender class name: " << sender() -> className() << "\n";
#endif
  if ( sender() && sender() -> inherits ("KToolBarButton") )
    button = (QWidget*) sender();
  else
  {
    KToolBar* toolbar = 0;
    int index;
    if ( sender() -> inherits ("KToolBar") )
      toolbar = (KToolBar*) sender();
    else
      for ( index = 0; index < containerCount() && (! toolbar || ! toolbar -> isVisible()); index ++ )
	toolbar = toolBar (index);
    if ( toolbar && toolbar -> isVisible() )
    {
      int index = findContainer (toolbar);
      if ( index >= 0 )
        button = toolbar -> getButton (itemId (index));
    }
  }
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
  QWhatsThis::add (m_frame, whatsThis());
  QWhatsThis::add (m_slider, whatsThis());
  m_frame -> move (point);
  m_frame -> show();
  m_slider -> setFocus();
}

KPlayerSliderAction::KPlayerSliderAction (const QString& text, const KShortcut& cut,
    const QObject* receiver, const char* slot, KActionCollection* parent, const char* name)
  : KWidgetAction (new KPlayerSlider (Qt::Horizontal, 0, name), text, cut, 0, 0, parent, name)
{
  setAutoSized (true);
  setShortcutConfigurable (false);
  connect (slider(), SIGNAL (changed (int)), receiver, slot);
}

KPlayerSliderAction::~KPlayerSliderAction()
{
  delete slider();
}

int KPlayerSliderAction::plug (QWidget* widget, int index)
{
  int result = KWidgetAction::plug (widget, index);
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
  KWidgetAction::unplug (widget);
  if ( ! slider() || ! isPlugged() || widget != slider() -> parent() )
    return;
  disconnect (widget, SIGNAL (orientationChanged (Orientation)), this, SLOT (orientationChanged (Orientation)));
}

void KPlayerSliderAction::orientationChanged (Orientation orientation)
{
  if ( slider() )
    slider() -> setOrientation (orientation);
}

KPlayerSlider::KPlayerSlider (Qt::Orientation orientation, QWidget* parent, const char* name)
  : QSlider (orientation, parent, name)
{
  m_dragging = m_changing_orientation = false;
  setTickmarks (QSlider::Both);
  connect (this, SIGNAL (valueChanged (int)), this, SLOT (sliderValueChanged (int)));
  //connect (this, SIGNAL (sliderPressed()), this, SLOT (sliderThumbPressed()));
  //connect (this, SIGNAL (sliderReleased()), this, SLOT (sliderThumbReleased()));
}

KPlayerSlider::~KPlayerSlider()
{
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "KPlayerSlider destroyed\n";
#endif
}

QSize KPlayerSlider::sizeHint() const
{
  QSize hint = QSlider::sizeHint();
  int length = kPlayerSettings() -> preferredSliderLength();
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
  return hint;
}

QSize KPlayerSlider::minimumSizeHint() const
{
//kdDebugTime() << "KPlayerSlider minimum size hint\n";
  QSize hint = QSlider::minimumSizeHint();
  int length = kPlayerSettings() -> minimumSliderLength();
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
  return hint;
}

void KPlayerSlider::setOrientation (Orientation o)
{
  if ( o == orientation() )
    return;
  m_changing_orientation = true;
  int minValue = QSlider::minValue();
  int maxValue = QSlider::maxValue();
  int value = QSlider::value();
  QSlider::setOrientation (o);
  QSlider::setMinValue (- maxValue);
  QSlider::setMaxValue (- minValue);
  QSlider::setValue (- value);
  m_changing_orientation = false;
}

int KPlayerSlider::minValue (void) const
{
  if ( orientation() == Qt::Horizontal )
    return QSlider::minValue();
  return - QSlider::maxValue();
}

void KPlayerSlider::setMinValue (int minValue)
{
  if ( orientation() == Qt::Horizontal )
    QSlider::setMinValue (minValue);
  else
    QSlider::setMaxValue (- minValue);
}

int KPlayerSlider::maxValue (void) const
{
  if ( orientation() == Qt::Horizontal )
    return QSlider::maxValue();
  return - QSlider::minValue();
}

void KPlayerSlider::setMaxValue (int maxValue)
{
  if ( orientation() == Qt::Horizontal )
    QSlider::setMaxValue (maxValue);
  else
    QSlider::setMinValue (- maxValue);
}

int KPlayerSlider::value (void) const
{
  if ( orientation() == Qt::Horizontal )
    return QSlider::value();
  return - QSlider::value();
}

void KPlayerSlider::setValue (int value, int)
{
  if ( orientation() == Qt::Horizontal )
    QSlider::setValue (value);
  else
    QSlider::setValue (- value);
}

void KPlayerSlider::setup (int minValue, int maxValue, int value, int tickInterval, int pageStep, int lineStep)
{
  setMinValue (minValue);
  setMaxValue (maxValue);
  setLineStep (lineStep);
  setPageStep (pageStep);
  setTickInterval (tickInterval);
  setValue (value);
}

/*void KPlayerSlider::sliderThumbPressed (void)
{
  m_dragging = true;
}

void KPlayerSlider::sliderThumbReleased (void)
{
  m_dragging = false;
}*/

void KPlayerSlider::sliderValueChanged (int)
{
  if ( ! m_changing_orientation )
    emit changed (value());
}

void KPlayerSlider::mousePressEvent (QMouseEvent* event)
{
  m_dragging = (event -> stateAfter() & (Qt::LeftButton | Qt::MidButton)) != 0;
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Popup slider mouse press " << event -> state() << " -> " << event -> stateAfter() << " -> " << m_dragging << "\n";
#endif
  QSlider::mousePressEvent (event);
}

void KPlayerSlider::mouseReleaseEvent (QMouseEvent* event)
{
  m_dragging = (event -> stateAfter() & (Qt::LeftButton | Qt::MidButton)) != 0;
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Popup slider mouse release " << event -> state() << " -> " << event -> stateAfter() << " -> " << m_dragging << "\n";
#endif
  QSlider::mouseReleaseEvent (event);
}

void KPlayerSlider::keyPressEvent (QKeyEvent* event)
{
  int key = event -> key();
  if ( key == Key_Left || key == Key_Right || (event -> state() & KeyButtonMask) != 0
    && (key == Key_Up || key == Key_Down || key == Key_PageUp || key == Key_PageDown
    || key == Key_Home || key == Key_End) )
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
  delta *= (event -> state() & Qt::ControlButton) == Qt::ControlButton ? pageStep() : lineStep();
  int offset = (delta + remainder + (delta > 0 ? 60 : -60)) / 120;
  remainder += delta - offset * 120;
#ifdef DEBUG_KPLAYER_SLIDERS
  kdDebugTime() << "Slider wheel event: delta " << event -> delta() << " offset " << offset << " remainder " << remainder
    << " position " << event -> x() << "x" << event -> y() << " global " << event -> globalX() << "x" << event -> globalY()
    << " state " << event -> state() << " orientation " << event -> orientation() << " accepted " << event -> isAccepted()
    << " spontaneous " << event -> spontaneous() << "\n";
#endif
  if ( offset != 0 )
    QSlider::setValue (QSlider::value() + offset);
  event -> accept();
}
