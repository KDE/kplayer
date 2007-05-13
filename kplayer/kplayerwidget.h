/***************************************************************************
                          kplayerwidget.h
                          ---------------
    begin                : Sun Dec 01 2002
    copyright            : (C) 2002-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERWIDGET_H
#define KPLAYERWIDGET_H

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
  KPlayerWidget (QWidget* parent = 0, const char* name = 0);
  /** The widget destructor. Frees allocated memory. */
  virtual ~KPlayerWidget();

  void mapHandler (uint wid);
  void unmapHandler (uint wid);

  void sendConfigureEvent (void);

protected:
  /** Processes the widget resize event. Ensures that MPlayer
      continues to display video. Emits resized signal. */
  virtual void resizeEvent (QResizeEvent*);
  /** Processes the widget show event. Ensures that MPlayer
      continues to display video with correct size. */
  virtual void showEvent (QShowEvent*);
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
  KPlayerWorkspace (QWidget* parent = 0, const char* name = 0);

  /** Returns a pointer to the KPlayerWidget object. */
  KPlayerWidget* widget (void) const
    { return m_widget; }

  /** Resizes the widget to the given size. */
  void setDisplaySize (QSize);

  /** Handles resizing done by the window manager.
   *  @param resizing true if resizing has started, false if resizing has completed
   */
  void resizeHandler (bool resizing);

  /** Returns whether the workspace is being resized by the user. */
  bool isResizing (void) const
    { return m_resizing; }

protected:
  /** Processes the widget resize event. Resizes the KPlayerWidget to the video size. */
  virtual void resizeEvent (QResizeEvent*);
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
  virtual void windowActivationChange (bool);
  virtual void focusInEvent (QFocusEvent*);
  virtual void focusOutEvent (QFocusEvent*);

  /** The pointer to the KPlayerWidget object. */
  KPlayerWidget* m_widget;

  // Following should be private
  /** Window manager is resizing the top level window. */
  bool m_resizing;

signals:
  /** Emitted when the widget is resized. */
  void resized (void);
  /** Emitted when the widget is resized by the user. */
  void userResize (void);
  /** Emitted when the context menu event is received. */
  void contextMenu (const QPoint& global_position);
};

#endif
