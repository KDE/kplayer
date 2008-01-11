/***************************************************************************
                          kplayernodeaction.h
                          -------------------
    begin                : Wed Apr 05 2006
    copyright            : (C) 2006-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERNODEACTION_H
#define KPLAYERNODEACTION_H

#include <kactionmenu.h>

#include "kplayeractionlist.h"

class KUrl;
class KPlayerNode;
class KPlayerContainerNode;
class KPlayerDeviceNode;
class KPlayerDiskNode;
class KPlayerNodeList;

/**Node action list.
  *@author kiriuja
  */
class KPlayerNodeActionList : public KPlayerActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerNodeActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerNodeActionList();

  /** Initializes the node and actions. */
  void initialize (const KUrl& url);

  /** Populates the node. */
  virtual void populateNode (void);
  /** Vacates the node. */
  virtual void vacateNode (void);

  /** Returns the node. */
  KPlayerContainerNode* node (void) const
    { return m_node; }

  /** Returns the maximum list size. */
  int maximumSize (void) const
    { return m_size; }
  /** Sets the maximum list size. */
  void setMaximumSize (int size)
    { m_size = size; }

  /** Returns whether the given node can be included on the list. */
  virtual bool canInclude (KPlayerNode* node) const;

  /** Frees up resources. */
  void terminate (void);

protected slots:
  /** Adds the given nodes to the list. */
  void added (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after = 0);
  /** Removes the given nodes from the list. */
  void removed (KPlayerContainerNode* parent, const KPlayerNodeList& nodes);
  /** Updates the given node attributes. */
  void updated (KPlayerContainerNode* parent, KPlayerNode* node);
  /** Emits the activated signal with the selected node. */
  void actionActivated (void);

protected:
  /** Updates the node list. */
  void updateActions (void);
  /** Creates an action for the given node. */
  virtual QAction* createAction (KPlayerNode* node);

  /** Container node. */
  KPlayerContainerNode* m_node;
  /** Maximum list size. */
  int m_size;

signals:
  /** Emitted when the node is selected. */
  void activated (KPlayerNode*);
};

/**Container action list.
  *@author kiriuja
  */
class KPlayerContainerActionList : public KPlayerNodeActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerContainerActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerContainerActionList();

  /** Populates the node. */
  virtual void populateNode (void);
  /** Vacates the node. */
  virtual void vacateNode (void);

  /** Returns whether the given node can be included on the list. */
  virtual bool canInclude (KPlayerNode* node) const;
};

/**Devices action list.
  *@author kiriuja
  */
class KPlayerDevicesActionList : public KPlayerContainerActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDevicesActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerDevicesActionList();

protected:
  /** Creates an action for the given node. */
  virtual QAction* createAction (KPlayerNode* node);
  /** Updates the action text, status, and whats this. */
  virtual void updateAction (QAction* action);
};

/**Device action menu class.
  *@author kiriuja
  */
class KPlayerDeviceActionMenu : public KActionMenu
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDeviceActionMenu (KPlayerDeviceNode* node);
  /** Destructor. */
  virtual ~KPlayerDeviceActionMenu();

  /** Updates device actions. */
  virtual void setup (void);
  /** Updates the extra actions and populates the node as necessary. */
  virtual void update (void);

  /** Returns the node. */
  KPlayerDeviceNode* device (void) const
    { return (KPlayerDeviceNode*) parent(); }

  /** Returns the list of actions. */
  const QList<QAction*>& actions (void) const
    { return m_actions; }

protected slots:
  /** Plays the selected item. */
  void play (void);

  /** Adds the given nodes to the list. */
  void added (KPlayerContainerNode*, const KPlayerNodeList& nodes, KPlayerNode* after);
  /** Removes the given nodes from the list. */
  void removed (KPlayerContainerNode*, const KPlayerNodeList& nodes);
  /** Updates the given node attributes. */
  void updated (KPlayerContainerNode* parent, KPlayerNode* node);

protected:
  /** Updates the device action list. */
  virtual void updateActions (void);

  /** Device actions. */
  QList<QAction*> m_actions;
  /** Indicates whether the device node was populated. */
  bool m_populated;

signals:
  /** Emitted when the node is selected. */
  void activated (KPlayerNode*);
};

/**Disk action menu class.
  *@author kiriuja
  */
class KPlayerDiskActionMenu : public KPlayerDeviceActionMenu
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDiskActionMenu (KPlayerDeviceNode* node);
  /** Destructor. */
  virtual ~KPlayerDiskActionMenu();

  /** Updates device actions. */
  virtual void setup (void);
  /** Updates the extra actions and populates the node as necessary. */
  virtual void update (void);

  /** Returns the node. */
  KPlayerDiskNode* device (void) const
    { return (KPlayerDiskNode*) parent(); }

protected slots:
  /** Loads the disk. */
  void loadDisk (void);
  /** Plays the disk. */
  void playDisk (void);

protected:
  /** Updates the device action list. */
  virtual void updateActions (void);

  /** Load disk action. */
  QAction* m_load_action;
  /** Play disk action. */
  QAction* m_play_action;
  /** Indicates disk being loaded for playing. */
  bool m_play_disk;
};

#endif
