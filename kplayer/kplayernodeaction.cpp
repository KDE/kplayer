/***************************************************************************
                          kplayernodeaction.cpp
                          ---------------------
    begin                : Wed Apr 05 2006
    copyright            : (C) 2005 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klocale.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_NODEACTION
#endif

#include "kplayernodeaction.h"
#include "kplayernodeaction.moc"
#include "kplayernode.h"

KPlayerNodeActionList::KPlayerNodeActionList (const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name)
  : KPlayerActionList (text, status, whatsthis, parent, name)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Creating node action list\n";
#endif
  m_node = 0;
  m_size = INT_MAX;
  m_actions.setAutoDelete (false);
}

KPlayerNodeActionList::~KPlayerNodeActionList()
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Destroying node action list\n";
#endif
}

void KPlayerNodeActionList::initialize (const KURL& url)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "Initializing node action list\n";
  kdDebugTime() << " URL    " << url.url() << "\n";
#endif
  m_node = KPlayerNode::getNodeByUrl (url);
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
  m_actions.setAutoDelete (true);
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
  QPtrListIterator<KAction> actit (actions());
  while ( KAction* action = actit.current() )
  {
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
    ++ actit;
  }
}

KAction* KPlayerNodeActionList::createAction (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::createAction\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  QString name (node -> name());
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << " Name   " << name << "\n";
#endif
  KAction* action = new KAction (m_text.arg (name), 0, this, SLOT (actionActivated()), node);
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
  kdDebugTime() << " Name   " << name() << "\n";
#endif
  QPtrList<KAction> actions (m_actions);
  unplug();
  int count = 0;
  KPlayerNodeListByName nodes ((const KPlayerNodeListByName&) node() -> nodes());
  if ( ! node() -> customOrder() )
    nodes.sort();
  KPlayerNodeListIterator iterator (nodes);
  KPlayerNode* node;
  while ( (node = iterator.current()) && count < maximumSize() )
  {
    if ( canInclude (node) )
    {
#ifdef DEBUG_KPLAYER_NODEACTION
      kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
      QPtrListIterator<KAction> actit (actions);
      KAction* action;
      while ( (action = actit.current()) )
      {
        if ( action -> parent() == node )
        {
          actions.removeRef (action);
          break;
        }
        ++ actit;
      }
      if ( ! action )
        action = createAction (node);
      m_actions.append (action);
      ++ count;
    }
    ++ iterator;
  }
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << " Current " << m_actions.count() << "\n";
  kdDebugTime() << " Removed " << actions.count() << "\n";
#endif
  plug();
  actions.setAutoDelete (true);
}

void KPlayerNodeActionList::actionActivated (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerNodeActionList::actionActivated\n";
  if ( sender() )
    kdDebugTime() << " Sender " << sender() -> className() << "\n";
  if ( sender() && sender() -> parent() )
    kdDebugTime() << " Parent " << sender() -> parent() -> className() << "\n";
#endif
  if ( sender() && sender() -> parent() && sender() -> parent() -> inherits ("KPlayerNode") )
    emit activated ((KPlayerNode*) sender() -> parent());
}

KPlayerContainerActionList::KPlayerContainerActionList (const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name)
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

KPlayerDevicesActionList::KPlayerDevicesActionList (const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name)
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

KAction* KPlayerDevicesActionList::createAction (KPlayerNode* node)
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
    kdDebugTime() << " Sender " << sender() -> className() << "\n";
  if ( sender() && sender() -> parent() )
    kdDebugTime() << " Parent " << sender() -> parent() -> className() << " " << sender() -> parent() -> name ("<unnamed>") << "\n";
#endif
  if ( sender() && sender() -> parent() && sender() -> parent() -> inherits ("KPlayerNode") )
    emit activated ((KPlayerNode*) sender() -> parent());
}

void KPlayerDeviceActionMenu::updateActions (void)
{
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << "KPlayerDeviceActionMenu::updateActions\n";
#endif
  QPtrList<KAction> actions (m_actions);
  m_actions.clear();
  QPtrListIterator<KAction> actlit (actions);
  KAction* action;
  while ( (action = actlit.current()) )
  {
    remove (action);
    ++ actlit;
  }
  KPlayerNodeListIterator iterator (device() -> nodes());
  while ( KPlayerNode* node = iterator.current() )
  {
#ifdef DEBUG_KPLAYER_NODEACTION
    kdDebugTime() << " Track  " << node -> url().url() << "\n";
#endif
    QPtrListIterator<KAction> actit (actions);
    while ( (action = actit.current()) )
    {
      if ( action -> parent() == node )
      {
        actions.removeRef (action);
        break;
      }
      ++ actit;
    }
    if ( ! action )
      action = new KAction (i18n("Play %1").arg (node -> name()), 0, this, SLOT (play()), node);
    m_actions.append (action);
    ++ iterator;
  }
  QPtrListIterator<KAction> actmit (m_actions);
  while ( (action = actmit.current()) )
  {
    insert (action);
    ++ actmit;
  }
#ifdef DEBUG_KPLAYER_NODEACTION
  kdDebugTime() << " Current " << m_actions.count() << "\n";
  kdDebugTime() << " Removed " << actions.count() << "\n";
#endif
  actions.setAutoDelete (true);
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
  QPtrListIterator<KAction> actit (m_actions);
  while ( KAction* action = actit.current() )
  {
    if ( action -> parent() == node )
    {
      QString name (i18n("Play %1").arg (node -> name()));
      if ( action -> text() != name )
        action -> setText (name);
      break;
    }
    ++ actit;
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
  m_load_action = new KAction (i18n("&Load Disk"), 0, this, SLOT (loadDisk()), device);
  m_play_action = new KAction (i18n("&Play Disk"), 0, this, SLOT (playDisk()), device);
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
  setText (device() -> name());
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
  bool use_name = device() -> disk() && text != device() -> disk() -> defaultName();
  m_load_action -> setText (use_name ? i18n("&Load %1").arg (text) : i18n("&Load Disk"));
  m_play_action -> setText (use_name ? i18n("&Play %1").arg (text) : i18n("&Play Disk"));
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
    playDisk();
  }
  remove (m_load_action);
  remove (m_play_action);
  QString name (device() -> name());
  insert (m_play_action);
  KPlayerDeviceActionMenu::updateActions();
  insert (m_load_action);
}
