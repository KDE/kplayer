/***************************************************************************
                          kplayerlogwindow.h
                          ------------------
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

#ifndef KPLAYERLOGWINDOW_H
#define KPLAYERLOGWINDOW_H

#include <ktextedit.h>
#include <qdockwindow.h>

/**The KPlayer log widget.
  *@author kiriuja
  */
class KPlayerLogWidget : public KTextEdit
{
   Q_OBJECT

public: 
  KPlayerLogWidget (QWidget* parent = 0, const char* name = 0);

  /** Returns whether there is an error in the log. */
  bool hasError (void)
    { return m_error; }
  /** Sets an error condition. */
  void setError (bool);

public slots:
  virtual void clear (void);

protected:
  virtual void showEvent (QShowEvent*);
  virtual void resizeEvent (QResizeEvent*);

  /** Error condition flag. */
  bool m_error;
  /** Error location. */
  int m_location;
};

/**The KPlayer log window.
  *@author kiriuja
  */

class KPlayerLogWindow : public QDockWindow
{
   Q_OBJECT

public: 
  KPlayerLogWindow (QWidget* parent = 0, const char* name = 0);

  KPlayerLogWidget* kPlayerLogWidget (void)
    { return (KPlayerLogWidget*) widget(); }

  void addLine (const QString& line)
    { ((KPlayerLogWidget*) widget()) -> append (line); }
  void clear (void)
    { ((KPlayerLogWidget*) widget()) -> clear(); }
  bool isEmpty (void)
    { return ((KPlayerLogWidget*) widget()) -> text().isEmpty(); }

  /** Returns whether there is an error in the log. */
  bool hasError (void)
    { return ((KPlayerLogWidget*) widget()) -> hasError(); }
  /** Sets an error condition. */
  void setError (bool);

protected:
  virtual void hideEvent (QHideEvent*);

signals:
  void windowHidden (void);
};

#endif
