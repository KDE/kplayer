/***************************************************************************
                          kplayeractionlist.h
                          -------------------
    begin                : Thu Apr 13 2006
    copyright            : (C) 2006-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERACTIONLIST_H
#define KPLAYERACTIONLIST_H

#include <KLocalizedString>
#include <QObject>
#include <QList>

class QAction;
class QActionGroup;

/**Action list.
  *@author kiriuja
  */
class KPlayerActionList : public QObject
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerActionList();

  /** Returns the list of actions. */
  const QList<QAction*>& actions (void) const
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
  virtual void updateAction (QAction* action);
  /** Selects the item with the given index by emitting the activated signal. */
  virtual void actionActivated (QAction* action, int index);

  /** Action text template. */
  KLocalizedString m_text;
  /** Action status text template. */
  KLocalizedString m_status;
  /** Action whats this text template. */
  KLocalizedString m_whatsthis;
  /** Action list. */
  QList<QAction*> m_actions;
};

/**Simple action list.
  *@author kiriuja
  */
class KPlayerSimpleActionList : public KPlayerActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerSimpleActionList (const QStringList& names, const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerSimpleActionList();

  /** Updates the action list. */
  void update (void);

protected:
  /** Updates the action text, status, and whats this. */
  virtual void updateAction (QAction* action);

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
  KPlayerToggleActionList (const QStringList& names, const QMap<QString, bool>& states, const KLocalizedString& ontext,
    const KLocalizedString& offtext, const KLocalizedString& onstatus, const KLocalizedString& offstatus,
    const KLocalizedString& onwhatsthis, const KLocalizedString& offwhatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerToggleActionList();

protected:
  /** Updates the action text, status, and whats this. */
  virtual void updateAction (QAction* action);
  /** Selects the item with the given index by emitting the activated signal. */
  virtual void actionActivated (QAction* action, int index);

  /** Action states. */
  const QMap<QString, bool>& m_states;
  /** Action on text template. */
  KLocalizedString m_on_text;
  /** Action on status text template. */
  KLocalizedString m_on_status;
  /** Action on whats this text template. */
  KLocalizedString m_on_whatsthis;
};

/**Track action list.
  *@author kiriuja
  */
class KPlayerTrackActionList : public KPlayerActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerTrackActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerTrackActionList();

  /** Updates the track action list. */
  void update (const QMap<int, QString>& ids, int id);

protected:
  /** Adds actions for the given IDs to the list. */
  void addActions (const QMap<int, QString>& ids, int id);

  /** Selects the track with the given index by emitting the activated signal. */
  virtual void actionActivated (QAction* action, int index);

  /** Exclusive action group. */
  QActionGroup* m_action_group;
};

/**Subtitle track action list.
  *@author kiriuja
  */
class KPlayerSubtitleTrackActionList : public KPlayerTrackActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerSubtitleTrackActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerSubtitleTrackActionList();

  /** Updates the track action list. */
  void update (bool show, const QMap<int, QString>& sids, int sid, const QMap<int, QString>& vsids,
    int vsid, QStringList files, const QString& vobsub, const QString& current);

protected:
  /** Selects the track with the given index by emitting the activated signal. */
  virtual void actionActivated (QAction* action, int index);
};

#endif
