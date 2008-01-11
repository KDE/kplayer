/***************************************************************************
                          kplayerwidget.h
                          ---------------
    begin                : Sun Dec 01 2002
    copyright            : (C) 2002-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERWIDGET_H
#define KPLAYERWIDGET_H

#include <qtimer.h>
#include <qwidget.h>

#include "kplayerprocess.h"

/** The KPlayer widget where mplayer video output is displayed.
  *@author kiriuja
  */
class KPlayerWidget : public QWidget
{
   Q_OBJECT

public: 
  /** The widget constructor. Initializes internal data structures.
      Parameters are passed to the QWidget constructor. */
  KPlayerWidget (QWidget* parent);
  /** The widget destructor. Frees allocated memory. */
  virtual ~KPlayerWidget();

  void mapHandler (uint wid);
  void unmapHandler (uint wid);

  /** Returns the size hint for the widget. */
  virtual QSize sizeHint (void) const;

protected:
  /** Processes the widget resize event. Ensures that MPlayer
      continues to display video. Emits resized signal. */
  virtual void resizeEvent (QResizeEvent*);
  /** Processes the widget show event. Ensures that MPlayer
      continues to display video with correct size. */
  virtual void showEvent (QShowEvent*);
  /** Ignores a mouse move event. */
  virtual void mouseMoveEvent (QMouseEvent*);
  /** Ignores a mouse press event. */
  virtual void mousePressEvent (QMouseEvent*);
  /** Ignores a mouse release event. */
  virtual void mouseReleaseEvent (QMouseEvent*);
  /** Ignores a mouse double click event. */
  virtual void mouseDoubleClickEvent (QMouseEvent*);
  /** Ignores a context menu event. */
  virtual void contextMenuEvent (QContextMenuEvent*);
  /** Ignores a wheel event. */
  virtual void wheelEvent (QWheelEvent*);
  virtual void focusInEvent (QFocusEvent*);
  virtual void focusOutEvent (QFocusEvent*);

protected slots:
  /** Receives the stateChanged signal from KPlayerProcess. */
  void playerStateChanged (KPlayerProcess::State, KPlayerProcess::State);
};

/** The KPlayer workspace that contains the video widget.
  *@author kiriuja
  */
class KPlayerWorkspace : public QWidget
{
   Q_OBJECT

public: 
  /** The workspace constructor. Creates the KPlayerWidget.
      Parameters are passed to the QWidget constructor. */
  KPlayerWorkspace (QWidget* parent);

  /** Returns a pointer to the KPlayerWidget object. */
  KPlayerWidget* widget (void) const
    { return m_widget; }

  /** Returns a pointer to the hidden widget object. */
  QWidget* hiddenWidget (void) const
    { return m_hidden_widget; }

  /** Returns the size hint for the workspace. */
  virtual QSize sizeHint (void) const;

  /** Resizes the widget to the given size. */
  void setDisplaySize (QSize);

protected:
  /** Processes the widget resize event. Resizes the KPlayerWidget to the video size. */
  virtual void resizeEvent (QResizeEvent*);
  /** Shows mouse cursor and starts timer. */
  virtual void mouseMoveEvent (QMouseEvent*);
  /** Emits context menu signal on a right click in a part. */
  virtual void mousePressEvent (QMouseEvent*);
  /** Ignores a mouse release event. */
  virtual void mouseReleaseEvent (QMouseEvent*);
  /** Processes mouse double click event. Sends a doubleClick signal to the engine. */
  virtual void mouseDoubleClickEvent (QMouseEvent*);
  /** Processes the wheel event. Sends a wheelZoom signal to the engine. */
  virtual void wheelEvent (QWheelEvent*);
  /** Emits the contextMenu signal. */
  virtual void contextMenuEvent (QContextMenuEvent*);
  virtual void changeEvent (QEvent*);
  virtual void focusInEvent (QFocusEvent*);
  virtual void focusOutEvent (QFocusEvent*);

  /** Starts timer and sets the cursor. */
  void mouseActivity (void);
  /** Sets the mouse cursor to either blank or hand. */
  void setMouseCursor (void);

  /** The pointer to the KPlayerWidget object. */
  KPlayerWidget* m_widget;

  /** The pointer to the hidden widget object. */
  QWidget* m_hidden_widget;

  // Following should be private
  /** Mouse activity indicator. */
  bool m_mouse_activity;
  /** Mouse activity timer. */
  QTimer m_timer;

signals:
  /** Emitted when the widget is resized. */
  void resized (void);
  /** Emitted when the context menu event is received. */
  void contextMenu (const QPoint& global_position);

protected slots:
  /** Set the mouse cursor and tracking. */
  void setMouseCursorTracking (void);
  /** Receives the stateChanged signal from KPlayerProcess. */
  void playerStateChanged (KPlayerProcess::State, KPlayerProcess::State);
  /** Receives the timeout signal from the mouse activity timer. */
  void cursorTimeout (void);
};

#endif
