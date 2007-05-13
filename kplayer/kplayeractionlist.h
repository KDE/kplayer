/***************************************************************************
                          kplayeractionlist.h
                          -------------------
    begin                : Thu Apr 13 2006
    copyright            : (C) 2006-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERACTIONLIST_H
#define KPLAYERACTIONLIST_H

#include <qobject.h>
#include <qptrlist.h>

class KAction;

/**Action list.
  *@author kiriuja
  */
class KPlayerActionList : public QObject
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerActionList (const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name);
  /** Destructor. */
  virtual ~KPlayerActionList();

  /** Returns the list of actions. */
  const QPtrList<KAction>& actions (void) const
    { return m_actions; }
  /** Returns the number of actions on the list. */
  int count (void) const
    { return actions().count(); }
  /** Returns whether the list is empty. */
  bool isEmpty (void) const
    { return actions().isEmpty(); }

signals:
  /** Emitted when the action list is going to be updated. */
  void updating (KPlayerActionList* list);
  /** Emitted when the action list has been updated. */
  void updated (KPlayerActionList* list);
  /** Emitted when an item is selected from the list. Provides the item number. */
  void activated (int index);

protected slots:
  /** Emits the activated signal with the selected item number. */
  void actionActivated (void);

protected:
  /** Plugs the action list and emits the updated signal. */
  void plug (void);
  /** Unplugs the action list. */
  void unplug (void);
  /** Updates the action text, status, and whats this. */
  virtual void updateAction (KAction* action);
  /** Selects the item with the given index by emitting the activated signal. */
  virtual void actionActivated (KAction* action, int index);

  /** Action text template. */
  QString m_text;
  /** Action status text template. */
  QString m_status;
  /** Action whats this text template. */
  QString m_whatsthis;
  /** Action list. */
  QPtrList<KAction> m_actions;
};

/**Simple action list.
  *@author kiriuja
  */
class KPlayerSimpleActionList : public KPlayerActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerSimpleActionList (const QStringList& names, const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name);
  /** Destructor. */
  virtual ~KPlayerSimpleActionList();

  /** Updates the action list. */
  void update (void);

protected:
  /** Updates the action text, status, and whats this. */
  virtual void updateAction (KAction* action);

  /** Action names. */
  const QStringList& m_names;
};

/**Toggle action list.
  *@author kiriuja
  */
class KPlayerToggleActionList : public KPlayerSimpleActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerToggleActionList (const QStringList& names, const QMap<QString, bool>& states,
    const QString& ontext, const QString& offtext, const QString& onstatus, const QString& offstatus,
    const QString& onwhatsthis, const QString& offwhatsthis, QObject* parent, const char* name);
  /** Destructor. */
  virtual ~KPlayerToggleActionList();

protected:
  /** Updates the action text, status, and whats this. */
  virtual void updateAction (KAction* action);
  /** Selects the item with the given index by emitting the activated signal. */
  virtual void actionActivated (KAction* action, int index);

  /** Action states. */
  const QMap<QString, bool>& m_states;
  /** Action on text template. */
  QString m_on_text;
  /** Action on status text template. */
  QString m_on_status;
  /** Action on whats this text template. */
  QString m_on_whatsthis;
};

/**Track action list.
  *@author kiriuja
  */
class KPlayerTrackActionList : public KPlayerActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerTrackActionList (const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name);
  /** Destructor. */
  virtual ~KPlayerTrackActionList();

  /** Updates the track action list. */
  void update (const QMap<int, QString>& ids, int id);

protected:
  /** Adds actions for the given IDs to the list. */
  void addActions (const QMap<int, QString>& ids, int id);

  /** Selects the track with the given index by emitting the activated signal. */
  virtual void actionActivated (KAction* action, int index);
};

/**Subtitle track action list.
  *@author kiriuja
  */
class KPlayerSubtitleTrackActionList : public KPlayerTrackActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerSubtitleTrackActionList (const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name);
  /** Destructor. */
  virtual ~KPlayerSubtitleTrackActionList();

  /** Updates the track action list. */
  void update (bool show, const QMap<int, QString>& sids, int sid, const QMap<int, QString>& vsids,
    int vsid, const QStringList& files, const QString& current, bool external);

protected:
  /** Selects the track with the given index by emitting the activated signal. */
  virtual void actionActivated (KAction* action, int index);
};

#endif
