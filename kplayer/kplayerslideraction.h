/***************************************************************************
                          kplayerslideraction.h
                          ---------------------
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

#ifndef KPLAYERSLIDERACTION_H
#define KPLAYERSLIDERACTION_H

#include <KAction>
#include <QFrame>
#include <QSlider>
#include <QToolButton>

/**KPlayer's slider widget. Works around the Qt upside-down slider bug.
  *@author kiriuja
  */
class KPlayerSlider : public QSlider
{
  Q_OBJECT

public:
  /** The KPlayerSlider constructor. Parameters are passed on to QSlider. */
  KPlayerSlider (Qt::Orientation orientation, QWidget* parent = 0);
  /** The KPlayerSlider destructor. Does nothing. */
  virtual ~KPlayerSlider();

  /** The size hint. */
  virtual QSize sizeHint() const;
  /** The minimum size hint. */
  virtual QSize minimumSizeHint() const;

  /** Sets up the slider by setting all options in one go. */
  void setup (int minValue, int maxValue, int value, bool tickMarks, int tickInterval, int pageStep, int lineStep);

protected slots:
  /** Changes the slider orientation when the toolbar orientation changes. */
  void parentOrientationChanged (Qt::Orientation);

protected:
  /** Adjusts the hint with the given length and the toolbar icon size. */
  void adjustHint (QSize& hint, int length) const;

  /** Filters keystrokes. */
  virtual void keyPressEvent (QKeyEvent*);
  /** Processes the wheel event. Reverses direction when the slider is horizontal. */
  virtual void wheelEvent (QWheelEvent*);
};

/**KPlayer popup frame.
  *@author kiriuja
  */
class KPlayerPopupFrame : public QFrame
{
  Q_OBJECT

public:
  /** The KPlayerPopupFrame constructor. Parameters are passed on to QHBox. */
  KPlayerPopupFrame (QWidget* parent = 0);
  /** The KPlayerPopupFrame destructor. Does nothing. */
  virtual ~KPlayerPopupFrame();

protected:
  /** Closes the popup frame when Alt, Tab, Esc, Enter or Return is pressed. */
  virtual void keyPressEvent (QKeyEvent*);
};

/**Toolbar button that shows a popup slider.
  *@author kiriuja
  */
class KPlayerPopupToolButton : public QToolButton
{
  Q_OBJECT

public:
  /** The KPlayerPopupToolButton constructor. Parameters are passed on to QToolButton. */
  KPlayerPopupToolButton (QWidget* parent = 0);

  /** Prevents the action from being triggered when clicked. */
  virtual void nextCheckState (void);
};

/**Action representing a popup slider activated by a toolbar button.
  *@author kiriuja
  */
class KPlayerPopupSliderAction : public KAction
{
  Q_OBJECT

public:
  /** The KPlayerPopupSliderAction constructor. Parameters are passed on to KAction. */
  KPlayerPopupSliderAction (QObject* parent = 0);
  /** The KPlayerPopupSliderAction destructor. */
  virtual ~KPlayerPopupSliderAction();

  /** Returns a pointer to the KPlayerSlider object. */
  KPlayerSlider* slider (void)
    { return m_slider; }

public slots:
  /** Pops up the slider. */
  void showSlider (void);

protected:
  /** Connects the button signal. */
  virtual QWidget* createWidget (QWidget* parent);

  /** The slider. */
  KPlayerSlider* m_slider;
  /** The popup frame. */
  KPlayerPopupFrame* m_frame;
};

/**Slider action suitable for insertion into a toolbar.
  *@author kiriuja
  */
class KPlayerSliderAction : public QWidgetAction
{
  Q_OBJECT

public:
  /** The KPlayerSliderAction constructor. Parameters are passed on to KAction. */
  KPlayerSliderAction (QObject* parent = 0);
  /** The KPlayerSliderAction destructor. */
  virtual ~KPlayerSliderAction();

  /** Returns a pointer to the KPlayerSlider object. */
  KPlayerSlider* slider (void)
    { return (KPlayerSlider*) defaultWidget(); }

protected:
  /** Creates and sets up a new slider. */
  virtual QWidget* createWidget (QWidget* parent);
};

#endif
