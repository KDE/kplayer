/***************************************************************************
                          kplayernodeaction.cpp
                          ---------------------
    begin                : Wed Apr 05 2006
    copyright            : (C) 2006-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klocale.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_NODEACTION
#endif

#include "kplayernodeaction.h"
#include "kplayernodeaction.moc"
#include "kplayernode.h"

KPlayerNodeActionList::KPlayerNodeActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name)
  : KPlayerActionList (text, status, whatsthis, parent, name)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Creating node action list\n";
#endif
  m_node = 0;
  m_size = INT_MAX;
}

KPlayerNodeActionList::~KPlayerNodeActionList()
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Destroying node action list\n";
#endif
}

void KPlayerNodeActionList::initialize (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Initializing node action list\n";
  kdDebugTime() << " URL    " << url.url() << "\n";
#endif
  m_node = KPlayerNode::root() -> getNodeByUrl (url);
  if ( node() )
  {
    node() -> reference();
    populateNode();
    updateActions();
    connect (node(), SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
      SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
    connect (node(), SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
      SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
    connect (node(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
      SLOT (updated (KPlayerContainerNode*, KPlayerNode*)));
  }
}

void KPlayerNodeActionList::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Terminating node action list\n";
#endif
  for ( QList<QAction*>::ConstIterator iterator (actions().begin()); iterator != actions().end(); ++ iterator )
    delete *iterator;
  m_actions.clear();
  if ( node() )
  {
    vacateNode();
    node() -> release();
  }
}

void KPlayerNodeActionList::populateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::populateNode\n";
#endif
  node() -> populate();
}

void KPlayerNodeActionList::vacateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::vacateNode\n";
#endif
  node() -> vacate();
}

void KPlayerNodeActionList::added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::added\n";
#endif
  updateActions();
}

void KPlayerNodeActionList::removed (KPlayerContainerNode*, const KPlayerNodeList&)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::removed\n";
#endif
  updateActions();
}

void KPlayerNodeActionList::updated (KPlayerContainerNode*, KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::updated\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  for ( QList<QAction*>::ConstIterator actit (actions().begin()); actit != actions().end(); ++ actit )
  {
    QAction* action = *actit;
    if ( action -> parent() == node )
    {
      QString name (node -> name());
      if ( action -> text() != name )
      {
#ifdef DEBUG_KPLAYER_NODEACTION
        kdDebugTime() << " Name   " << name << "\n";
#endif
        action -> setText (name);
        updateAction (action);
      }
      break;
    }
  }
}

QAction* KPlayerNodeActionList::createAction (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::createAction\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  QString name (node -> name());
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << " Name   " << name << "\n";
#endif
  QAction* action = new KAction (node);
  action -> setText (m_text.subs (name).toString());
  connect (action, SIGNAL (triggered()), SLOT (actionActivated()));
  updateAction (action);
  return action;
}

bool KPlayerNodeActionList::canInclude (KPlayerNode*) const
{
  return true;
}

void KPlayerNodeActionList::updateActions (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::updateActions\n";
  kdDebugTime() << " Name   " << objectName() << "\n";
#endif
  QList<QAction*> actlist (actions());
  unplug();
  int count = 0;
  KPlayerNodeListByName nodes ((const KPlayerNodeListByName&) node() -> nodes());
  if ( ! node() -> customOrder() )
    nodes.sort();
  KPlayerNodeList::ConstIterator iterator (nodes.constBegin());
  while ( iterator != nodes.constEnd() && count < maximumSize() )
  {
    KPlayerNode* node = *iterator;
    if ( canInclude (node) )
    {
#ifdef DEBUG_KPLAYER_NODEACTION
      kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
      QAction* action = 0;
      for ( QList<QAction*>::ConstIterator actit (actlist.constBegin()); actit != actlist.constEnd(); ++ actit )
      {
	action = *actit;
        if ( action -> parent() == node )
        {
          actlist.removeAll (action);
          break;
        }
      }
      if ( ! action )
        action = createAction (node);
      m_actions.append (action);
      ++ count;
    }
    ++ iterator;
  }
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << " Current " << actions().count() << "\n";
  kdDebugTime() << " Removed " << actlist.count() << "\n";
#endif
  plug();
  for ( QList<QAction*>::ConstIterator iterator (actlist.constBegin()); iterator != actlist.constEnd(); ++ iterator )
    delete *iterator;
}

void KPlayerNodeActionList::actionActivated (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::actionActivated\n";
  if ( sender() )
    kdDebugTime() << " Sender " << sender() -> metaObject() -> className() << "\n";
  if ( sender() && sender() -> parent() )
    kdDebugTime() << " Parent " << sender() -> parent() -> metaObject() -> className() << "\n";
#endif
  if ( sender() && sender() -> parent() && sender() -> parent() -> inherits ("KPlayerNode") )
    emit activated ((KPlayerNode*) sender() -> parent());
}

KPlayerContainerActionList::KPlayerContainerActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name)
  : KPlayerNodeActionList (text, status, whatsthis, parent, name)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Creating container action list\n";
#endif
}

KPlayerContainerActionList::~KPlayerContainerActionList()
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Destroying container action list\n";
#endif
}

void KPlayerContainerActionList::populateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerContainerActionList::populateNode\n";
#endif
  node() -> populateGroups();
}

void KPlayerContainerActionList::vacateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerContainerActionList::vacateNode\n";
#endif
  node() -> vacateGroups();
}

bool KPlayerContainerActionList::canInclude (KPlayerNode* node) const
{
  return node -> isContainer();
}

KPlayerDevicesActionList::KPlayerDevicesActionList (const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name)
  : KPlayerContainerActionList (text, status, whatsthis, parent, name)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Creating devices action list\n";
#endif
}

KPlayerDevicesActionList::~KPlayerDevicesActionList()
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Destroying devices action list\n";
#endif
}

QAction* KPlayerDevicesActionList::createAction (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDevicesActionList::createAction\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
  kdDebugTime() << " Name   " << node -> name() << "\n";
#endif
  KPlayerDeviceNode* device = (KPlayerDeviceNode*) node;
  const QString& type = device -> media() -> type();
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << " Type   " << type << "\n";
#endif
  KPlayerDeviceActionMenu* action = type == "TV" || type == "DVB" ?
    (KPlayerDeviceActionMenu*) new KPlayerDeviceActionMenu (device) :
    (KPlayerDeviceActionMenu*) new KPlayerDiskActionMenu (device);
  updateAction (action);
  action -> setup();
  connect (action, SIGNAL (activated (KPlayerNode*)), parent(), SLOT (play (KPlayerNode*)));
  return action;
}

KPlayerDeviceActionMenu::KPlayerDeviceActionMenu (KPlayerDeviceNode* device)
  : KActionMenu (device -> name(), device)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Creating device action menu\n";
  kdDebugTime() << " URL    " << device -> url().url() << "\n";
#endif
  setDelayed (false);
  m_populated = false;
  device -> reference();
  connect (device, SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  connect (device, SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  connect (device, SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
    SLOT (updated (KPlayerContainerNode*, KPlayerNode*)));
}

KPlayerDeviceActionMenu::~KPlayerDeviceActionMenu()
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Destroying device action menu\n";
  kdDebugTime() << " ID     " << device() -> id() << "\n";
#endif
  if ( m_populated )
    device() -> vacate();
  device() -> release();
}

void KPlayerDeviceActionMenu::setup (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDeviceActionMenu::setup\n";
#endif
  m_populated = true;
  device() -> populate();
  updateActions();
}

void KPlayerDeviceActionMenu::play (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDeviceActionMenu::play\n";
  if ( sender() )
    kdDebugTime() << " Sender " << sender() -> metaObject() -> className() << "\n";
  if ( sender() && sender() -> parent() )
    kdDebugTime() << " Parent " << sender() -> parent() -> metaObject() -> className() << " " << sender() -> parent() -> objectName() << "\n";
#endif
  if ( sender() && sender() -> parent() && sender() -> parent() -> inherits ("KPlayerNode") )
    emit activated ((KPlayerNode*) sender() -> parent());
}

void KPlayerDeviceActionMenu::updateActions (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDeviceActionMenu::updateActions\n";
#endif
  QList<QAction*> actlist (actions());
  m_actions.clear();
  for ( QList<QAction*>::ConstIterator actlit (actlist.constBegin()); actlit != actlist.constEnd(); ++ actlit )
    removeAction (*actlit);
  KPlayerNodeList::ConstIterator iterator (device() -> nodes().begin());
  while ( iterator != device() -> nodes().end() )
  {
    KPlayerNode* node = *iterator;
#ifdef DEBUG_KPLAYER_NODEACTION
    kdDebugTime() << " Track  " << node -> url().url() << "\n";
#endif
    QAction* action = 0;
    for ( QList<QAction*>::ConstIterator actit (actlist.constBegin()); actit != actlist.constEnd(); ++ actit )
    {
      action = *actit;
      if ( action -> parent() == node )
      {
        actlist.removeAll (action);
        break;
      }
    }
    if ( ! action )
    {
      action = new KAction (node);
      action -> setText (i18n("Play %1", node -> name()));
      connect (action, SIGNAL (triggered()), SLOT (play()));
    }
    m_actions.append (action);
    ++ iterator;
  }
  for ( QList<QAction*>::ConstIterator actmit (actions().begin()); actmit != actions().end(); ++ actmit )
    addAction (*actmit);
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << " Current " << actions().count() << "\n";
  kdDebugTime() << " Removed " << actlist.count() << "\n";
#endif
  for ( QList<QAction*>::ConstIterator iterator (actlist.constBegin()); iterator != actlist.constEnd(); ++ iterator )
    delete *iterator;
}

void KPlayerDeviceActionMenu::added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDeviceActionMenu::added\n";
  kdDebugTime() << " URL    " << device() -> url().url() << "\n";
#endif
  updateActions();
}

void KPlayerDeviceActionMenu::removed (KPlayerContainerNode*, const KPlayerNodeList&)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDeviceActionMenu::removed\n";
#endif
  updateActions();
}

void KPlayerDeviceActionMenu::updated (KPlayerContainerNode*, KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDeviceActionMenu::updated\n";
  kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  for ( QList<QAction*>::ConstIterator actit (actions().begin()); actit != actions().end(); ++ actit )
  {
    QAction* action = *actit;
    if ( action -> parent() == node )
    {
      QString name (i18n("Play %1", node -> name()));
      name.replace ("&", "&&");
      if ( action -> text() != name )
        action -> setText (name);
      break;
    }
  }
}

KPlayerDiskActionMenu::KPlayerDiskActionMenu (KPlayerDeviceNode* device)
  : KPlayerDeviceActionMenu (device)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Creating disk action menu\n";
  kdDebugTime() << " URL    " << device -> url().url() << "\n";
#endif
  m_play_disk = false;
  m_load_action = new KAction (device);
  m_load_action -> setText (i18n("&Load Disk"));
  connect (m_load_action, SIGNAL (triggered()), SLOT (loadDisk()));
  m_play_action = new KAction (device);
  m_play_action -> setText (i18n("&Play Disk"));
  connect (m_play_action, SIGNAL (triggered()), SLOT (playDisk()));
}

KPlayerDiskActionMenu::~KPlayerDiskActionMenu()
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Destroying disk action menu\n";
  kdDebugTime() << " ID     " << device() -> id() << "\n";
#endif
}

void KPlayerDiskActionMenu::setup (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDiskActionMenu::setup\n";
#endif
  //setText (device() -> name());
  updateActions();
}

void KPlayerDiskActionMenu::setText (const QString& text)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDiskActionMenu::setText\n";
  kdDebugTime() << " URL    " << device() -> url().url() << "\n";
  kdDebugTime() << " Text   " << text << "\n";
  kdDebugTime() << " Populated " << m_populated << "\n";
  kdDebugTime() << " Populate  " << device() -> populated() << "\n";
  if ( device() -> disk() )
    kdDebugTime() << " Disk type " << device() -> disk() -> type() << "\n";
  kdDebugTime() << " Complete  " << device() -> parent() -> complete() << "\n";
#endif
  KPlayerDeviceActionMenu::setText (text);
  bool use_name = device() -> disk() && device() -> name() != device() -> disk() -> defaultName();
  m_play_action -> setText (use_name ? i18n("&Play %1", text) : i18n("&Play Disk"));
  if ( ! m_populated && ! device() -> dataDisk() && (device() -> populated()
    || device() -> disk() && device() -> parent() -> complete()) )
  {
    m_populated = true;
    device() -> populate();
  }
}

void KPlayerDiskActionMenu::loadDisk (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDiskActionMenu::loadDisk\n";
#endif
  if ( ! m_populated )
  {
    m_populated = true;
    device() -> populate();
  }
  device() -> loadDisk();
}

void KPlayerDiskActionMenu::playDisk (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDiskActionMenu::playDisk\n";
#endif
  if ( device() -> dataDisk() || device() -> populated() && device() -> nodes().count() )
    emit activated (device());
  else
  {
    m_play_disk = true;
    loadDisk();
  }
}

void KPlayerDiskActionMenu::updateActions (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDiskActionMenu::updateActions\n";
#endif
  if ( m_play_disk )
  {
    m_play_disk = false;
    if ( device() -> dataDisk() || device() -> populated() && device() -> nodes().count() )
      playDisk();
  }
  removeAction (m_load_action);
  removeAction (m_play_action);
  QString name (device() -> name());
  addAction (m_play_action);
  KPlayerDeviceActionMenu::updateActions();
  addAction (m_load_action);
}
