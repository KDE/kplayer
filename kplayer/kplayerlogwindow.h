/***************************************************************************
                          kplayerlogwindow.h
                          ------------------
    begin                : Fri May 9 2003
    copyright            : (C) 2003-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERLOGWINDOW_H
#define KPLAYERLOGWINDOW_H

#include <KActionCollection>
#include <KTextEdit>
#include <QDockWidget>

class KPlayerLogWindow;

/**The KPlayer log widget.
  *@author kiriuja
  */
class KPlayerLogWidget : public KTextEdit
{
   Q_OBJECT

public: 
  KPlayerLogWidget (KActionCollection* ac, QWidget* parent = 0);

  /** Returns whether there is an error in the log. */
  bool hasError (void)
    { return m_error; }
  /** Sets an error condition. */
  void setError (bool);

  /** Returns the popup menu. */
  QMenu* popupMenu (void) const
    { return m_popup; }
  /** Sets the popup menu. */
  void setPopupMenu (QMenu* menu)
    { m_popup = menu; }

  /** Returns the parent dock widget. */
  KPlayerLogWindow* parent (void) const
    { return (KPlayerLogWindow*) KTextEdit::parent(); }

  /** Returns the action collection. */
  KActionCollection* actionCollection (void) const
    { return m_ac; }

  /** Retrieves an action from the action collection by name. */
  QAction* action (const char* name) const
    { return m_ac -> action (name); }

  /** Remembers the widget height. */
  void rememberHeight (void)
    { m_height = height(); }

  /** Returns the size hint for the workspace. */
  virtual QSize sizeHint (void) const;

  /** Frees up resources and saves state. */
  void terminate (void);

public slots:
  virtual void clear (void);

protected slots:
  /** Updates actions according to the current state. */
  void updateActions (void);

protected:
  virtual void showEvent (QShowEvent*);
  /** Remembers the log height. */
  virtual void resizeEvent (QResizeEvent*);
  /** Displays the right click popup menu. */
  virtual void contextMenuEvent (QContextMenuEvent*);

  /** Action collection. */
  KActionCollection* m_ac;
  /** Popup menu. */
  QMenu* m_popup;
  /** Error condition flag. */
  bool m_error;
  /** Error location. */
  int m_location;
  /** Preferred height. */
  int m_height;
};

/**The KPlayer log window.
  *@author kiriuja
  */
class KPlayerLogWindow : public QDockWidget
{
   Q_OBJECT

public: 
  KPlayerLogWindow (KActionCollection* ac, QWidget* parent = 0);

  void initialize (QMenu* menu);

  KPlayerLogWidget* logWidget (void)
    { return (KPlayerLogWidget*) widget(); }

  /** Returns whether the widget is visible. */
  bool visible (void) const
    { return ! isHidden() && m_visibility; }
  /** Returns whether the widget is docked and visible. */
  bool docked (void) const
    { return ! isFloating() && ! isHidden(); }

  void addLine (const QString& line)
    { ((KPlayerLogWidget*) widget()) -> append (line); }
  bool isEmpty (void)
    { return ((KPlayerLogWidget*) widget()) -> document() -> isEmpty(); }

  /** Returns whether there is an error in the log. */
  bool hasError (void)
    { return ((KPlayerLogWidget*) widget()) -> hasError(); }
  /** Sets an error condition. */
  void setError (bool);

signals:
  /** Emitted when the widget is moved. */
  void moved (bool docked);
  /** Emitted when the widget is resized. */
  void resized (void);

protected slots:
  /** Sets the visibility flag. */
  void setVisibility (bool visibility);

protected:
  /** Emits the moved signal. */
  virtual void moveEvent (QMoveEvent*);
  /** Emits the resized signal. */
  virtual void resizeEvent (QResizeEvent*);

  /** Visibility indicator. */
  bool m_visibility;
};

#endif
