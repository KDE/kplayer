/***************************************************************************
                          kplayernode.cpp
                          ---------------
    begin                : Wed Feb 16 2005
    copyright            : (C) 2005-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kplayernode.h"
#include "kplayernode.moc"
#include "kplayerprocess.h"
#include "kplayersettings.h"
#include "kplayersource.h"

#include <kde_file.h>
#include <KLocale>
#include <KCodecs>
#include <KIO/NetAccess>
#include <KProcess>
#include <QRegExp>
#include <Solid/AudioInterface>
#include <Solid/Block>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/DvbInterface>
#include <Solid/GenericInterface>
#include <Solid/OpticalDisc>
#include <Solid/OpticalDrive>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>
#include <solid/video.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_NODE
#define DEBUG_KPLAYER_SOLID
#endif

KPlayerRootNode* KPlayerNode::m_root = 0;
QString KPlayerNode::m_sort_key ("Name");
bool KPlayerNode::m_sort_by_name = true;
bool KPlayerNode::m_sort_ascending = true;

KPlayerNode::~KPlayerNode()
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Destroying node\n";
  kdDebugTime() << " ID     " << id() << "\n";
#endif
  KPlayerMedia::release (media());
}

void KPlayerNode::setup (KPlayerContainerNode* parent, const QString& id, KPlayerContainerNode* origin)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Initializing node\n";
  kdDebugTime() << " ID     " << id << "\n";
  if ( parent )
    kdDebugTime() << " Parent " << parent -> url().url() << "\n";
#endif
  m_references = 0;
  setParent (parent);
  m_id = id;
  if ( parent )
    parent -> reference();
  setupMedia();
  setupChildren (origin);
}

void KPlayerNode::setupMedia (void)
{
  m_media = KPlayerMedia::genericProperties (metaurl());
  connect (media(), SIGNAL (updated()), SLOT (updated()));
}

void KPlayerNode::setupChildren (KPlayerContainerNode*)
{
}

bool KPlayerNode::ready (void) const
{
  return true;
}

QString KPlayerNode::suggestId (void) const
{
  return metaurl().url();
}

KUrl KPlayerNode::url (void) const
{
  return parent() -> url (id());
}

KUrl KPlayerNode::url (const QString& id) const
{
  KUrl idurl (url());
  idurl.addPath (id);
  return idurl;
}

KUrl KPlayerNode::metaurl (void) const
{
  return url();
}

KUrl KPlayerNode::metaurl (const QString& id) const
{
  KUrl idurl (media() -> url());
  idurl.addPath (id);
  return idurl;
}

QString KPlayerNode::icon (void) const
{
  return media() -> icon();
}

KPlayerContainerNode* KPlayerNode::topLevelNode (void) const
{
  if ( ! parent() )
    return 0;
  if ( ! parent() -> parent() )
    return (KPlayerContainerNode*) this;
  KPlayerContainerNode* node = parent();
  while ( node -> parent() -> parent() )
    node = node -> parent();
  return node;
}

bool KPlayerNode::isContainer (void) const
{
  return false;
}

bool KPlayerNode::hasProperties (void) const
{
  return false;
}

bool KPlayerNode::canRename (void) const
{
  return true;
}

int KPlayerNode::compare (KPlayerNode* node) const
{
  if ( parent() -> customOrder() )
    return parent() -> compareByPosition ((KPlayerNode*) this, node);
  if ( isContainer() != node -> isContainer() )
    return isContainer() == parent() -> groupsFirst() ? -1 : 1;
  int result = media() -> compare (node -> media(), sortKey());
  if ( result == 0 && ! sortByName() )
    result = compareStrings (name(), node -> name());
  return sortAscending() ? result : - result;
}

int KPlayerNode::compareByName (KPlayerNode* node) const
{
  return parent() -> customOrder() ? parent() -> compareByPosition ((KPlayerNode*) this, node)
    : compareStrings (name(), node -> name());
}

void KPlayerNode::countAttributes (KPlayerPropertyCounts& counts) const
{
  media() -> count (counts);
}

void KPlayerNode::updated (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNode::updated\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  parent() -> updateAttributes (this);
}

void KPlayerNode::release (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Releasing node\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
  kdDebugTime() << " References " << (m_references - 1) << "\n";
#endif
  if ( -- m_references )
    return;
  if ( parent() )
  {
    parent() -> release (this);
    parent() -> release();
  }
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Deleting node\n";
  kdDebugTime() << " ID     " << id() << "\n";
#endif
  delete this;
}

void KPlayerNode::detach (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Detaching node\n";
  kdDebugTime() << " ID     " << url().url() << "\n";
#endif
  if ( parent() )
    parent() -> release (this);
}

KPlayerMediaNode* KPlayerNode::previousMediaNode (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNode::previousMediaNode\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  for ( KPlayerNode* node = this; node -> parent(); node = node -> parent() )
  {
    int index = node -> parent() -> nodes().indexOf (node);
    if ( index > 0 )
      return node -> parent() -> nodes().at (index - 1) -> lastMediaNode();
  }
  return 0;
}

void KPlayerNode::initialize (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Initializing node hierarchy\n";
#endif
  m_root = new KPlayerRootNode;
  root() -> setup (0, "kplayer:/");
  root() -> populate();
}

void KPlayerNode::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Terminating node hierarchy\n";
#endif
  root() -> temporaryNode() -> release();
  root() -> vacate();
}

void KPlayerNode::setSorting (const QString& key, bool ascending)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNode::setSorting\n";
  kdDebugTime() << " Key    " << key << "\n";
  kdDebugTime() << " Ascending " << ascending << "\n";
#endif
  m_sort_key = key;
  m_sort_by_name = key == "Name";
  m_sort_ascending = ascending;
}

KPlayerMediaNode::~KPlayerMediaNode()
{
}

void KPlayerMediaNode::setupMedia (void)
{
  setupUrl();
  m_media = KPlayerMedia::trackProperties (metaurl());
  connect (media(), SIGNAL (updated()), SLOT (updated()));
}

void KPlayerMediaNode::setupUrl (void)
{
}

bool KPlayerMediaNode::hasProperties (void) const
{
  return true;
}

KPlayerMediaNode* KPlayerMediaNode::lastMediaNode (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerMediaNode::lastMediaNode\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  return this;
}

KPlayerFileNode::~KPlayerFileNode()
{
}

bool KPlayerFileNode::canRename (void) const
{
  return false;
}

KPlayerTrackNode::~KPlayerTrackNode()
{
}

KUrl KPlayerTrackNode::metaurl (void) const
{
  KUrl url (parent() -> media() -> url());
  url.addPath (id());
  return url;
}

KPlayerChannelNode::~KPlayerChannelNode()
{
}

KPlayerItemNode::~KPlayerItemNode()
{
}

void KPlayerItemNode::setupUrl (void)
{
  m_url = id();
  m_url.setRef (QString::null);
}

KUrl KPlayerItemNode::metaurl (void) const
{
  return m_url;
}

#if 0
KPlayerSearchItemNode::~KPlayerSearchItemNode()
{
}
#endif

KPlayerContainerNode::KPlayerContainerNode (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Creating container node\n";
#endif
  m_populate_nodes = m_populate_groups = 0;
  setParent (0);
  setOrigin (0);
}

KPlayerContainerNode::~KPlayerContainerNode()
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Destroying container node\n";
  kdDebugTime() << " ID     " << id() << "\n";
#endif
  if ( origin() )
    origin() -> release();
  delete source();
}

void KPlayerContainerNode::setupChildren (KPlayerContainerNode* origin)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Creating node children\n";
  kdDebugTime() << " ID     " << id() << "\n";
#endif
  if ( ! origin && parent() && parent() -> origin() )
    origin = parent() -> origin() -> getNodeById (id());
  if ( ! origin )
  {
    const KUrl& originurl (media() -> origin());
    if ( ! originurl.isEmpty() && originurl != url() )
      origin = root() -> getNodeByUrl (originurl);
  }
  setOrigin (origin);
  setupOrigin();
  if ( m_origin )
  {
    m_origin -> reference();
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Origin " << m_origin -> url().url() << "\n";
#endif
  }
  setupSource();
  source() -> connectOrigin();
}

void KPlayerContainerNode::setupOrigin (void)
{
}

void KPlayerContainerNode::setupSource (void)
{
  m_source = new KPlayerListSource (this);
}

bool KPlayerContainerNode::ready (void) const
{
  return ! origin() || origin() -> ready();
}

QString KPlayerContainerNode::icon (void) const
{
  return origin() ? origin() -> icon() : "folder";
}

QString KPlayerContainerNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder";
}

QString KPlayerContainerNode::suggestId (void) const
{
  return id();
}

bool KPlayerContainerNode::isContainer (void) const
{
  return true;
}

bool KPlayerContainerNode::isGroup (void) const
{
  return false;
}

bool KPlayerContainerNode::isDirectory (void) const
{
  return false;
}

bool KPlayerContainerNode::isCollection (void) const
{
  return false;
}

bool KPlayerContainerNode::isPlaylist (void) const
{
  return false;
}

bool KPlayerContainerNode::isRecent (void) const
{
  return false;
}

bool KPlayerContainerNode::isNowPlaying (void) const
{
  return false;
}

bool KPlayerContainerNode::isDevices (void) const
{
  return false;
}

bool KPlayerContainerNode::canQueue (void) const
{
  return ! isNowPlaying();
}

#if 0
bool KPlayerContainerNode::canGroup (void) const
{
  return isCollection();
}
#endif

bool KPlayerContainerNode::canSaveAsPlaylist (void) const
{
  return true;
}

bool KPlayerContainerNode::groupsFirst (void) const
{
  return ! isPlaylist();
}

bool KPlayerContainerNode::canCopy (const KPlayerNodeList& nodes) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::canCopy\n";
  kdDebugTime() << " Target " << url().url() << "\n";
#endif
  if ( ! canAddLeaves() )
    return false;
  KPlayerNodeList::ConstIterator iterator (nodes.constBegin());
  while ( iterator != nodes.constEnd() )
  {
    KPlayerNode* node = *iterator;
    if ( node -> isContainer() )
    {
      const KPlayerContainerNode* parent = this;
      while ( parent )
      {
        if ( node == parent )
          return false;
        parent = parent -> parent();
      }
    }
    ++ iterator;
  }
  return true;
}

bool KPlayerContainerNode::canLink (const KPlayerNodeList& nodes) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::canLink\n";
  kdDebugTime() << " Target " << url().url() << "\n";
#endif
  KPlayerNodeList::ConstIterator iterator (nodes.constBegin());
  while ( iterator != nodes.constEnd() )
  {
    KPlayerNode* node = *iterator;
    if ( node -> isContainer() && canLink ((KPlayerContainerNode*) node) )
      return true;
    ++ iterator;
  }
  return false;
}

bool KPlayerContainerNode::canLink (KPlayerContainerNode* node) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::canLink\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  return false;
}

bool KPlayerContainerNode::preferMove (KPlayerNode* node) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::preferMove\n";
  kdDebugTime() << " Target " << url().url() << "\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  return topLevelNode() == node -> topLevelNode();
}

void KPlayerContainerNode::release (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Releasing child node\n";
  kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  if ( remove (node) )
  {
    if ( populated() )
      node -> release();
    if ( groupsPopulated() && node -> isContainer() )
      node -> release();
  }
}

void KPlayerContainerNode::addBranch (const QString& name, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::addBranch\n";
  kdDebugTime() << " Name   " << name << "\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  KPlayerContainerNode* node = new KPlayerExternalNode;
  node -> setup (0, name);
  node -> reference();
  KPlayerNodeList list;
  list.append (node);
  add (list, false, after);
  node -> release();
}

void KPlayerContainerNode::appendBranch (const QString& name)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::appendBranch\n";
  kdDebugTime() << " Name   " << name << "\n";
#endif
  populate();
  addBranch (name, lastNode());
  vacate();
}

void KPlayerContainerNode::add (const KUrl::List& urls, bool link, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::add url list\n";
#endif
  if ( urls.isEmpty() )
    return;
  KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
  if ( ! list.isEmpty() )
  {
    add (list, link, after);
    list.releaseAll();
  }
}

void KPlayerContainerNode::append (const KUrl::List& urls)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::append url list\n";
#endif
  populate();
  add (urls, true, lastNode());
  vacate();
}

void KPlayerContainerNode::add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << (link ? "Linking" : "Copying") << " nodes\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  KPlayerItemProperties::resetMetaInfoTimer();
  source() -> add (nodes, link, after);
}

void KPlayerContainerNode::append (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::append\n";
#endif
  populate();
  add (nodes, true, lastNode());
  vacate();
}

void KPlayerContainerNode::move (const KPlayerNodeList& nodes, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Moving nodes\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  populate();
  KPlayerContainerNode* parent = nodes.first() -> parent();
  KPlayerNodeList list;
  KPlayerNodeList::ConstIterator iterator (nodes.constBegin());
  while ( iterator != nodes.constEnd() )
  {
    KPlayerNode* node = *iterator;
    if ( parent != node -> parent() )
    {
      if ( parent == this )
        after = moved (list, after);
      else
      {
        after = source() -> add (list, false, after);
        parent -> remove (list);
      }
      list.clear();
      parent = node -> parent();
    }
    list.append (node);
    ++ iterator;
  }
  if ( parent == this )
    moved (list, after);
  else
  {
    source() -> add (list, false, after);
    parent -> remove (list);
  }
  vacate();
}

void KPlayerContainerNode::remove (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Removing nodes\n";
#endif
  source() -> remove (nodes);
}

KPlayerNode* KPlayerContainerNode::createLeaf (const QString& id)
{
  KPlayerNode* node = new KPlayerItemNode;
  node -> setup (this, id);
  return node;
}

KPlayerContainerNode* KPlayerContainerNode::createBranch (const QString&, KPlayerContainerNode*)
{
  return 0;
}

void KPlayerContainerNode::insert (KPlayerNode* node, KPlayerNode* after)
{
  int index;
  if ( after == this )
    m_nodes.prepend (node);
  else if ( after && (index = nodes().indexOf (after)) >= 0 )
    m_nodes.insert (index + 1, node);
  else
  {
    m_nodes.append (node);
#ifdef DEBUG_KPLAYER_NODE
    if ( after )
      kdDebugTime() << " After node not found\n";
#endif
  }
  m_node_map.insert (node -> id(), node);
}

void KPlayerContainerNode::append (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Appending node\n";
  kdDebugTime() << " ID     " << node -> id() << "\n";
#endif
  m_nodes.append (node);
  m_node_map.insert (node -> id(), node);
}

KPlayerNode* KPlayerContainerNode::insertLeaf (const QString& id, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Inserting leaf node\n";
  kdDebugTime() << " ID     " << id << "\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  KPlayerNode* node = createLeaf (id);
  if ( node )
  {
    insert (node, after);
    if ( populated() )
      node -> reference();
  }
  return node;
}

KPlayerContainerNode* KPlayerContainerNode::insertBranch (const QString& id,
  KPlayerNode* after, KPlayerContainerNode* origin)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Inserting branch node\n";
  kdDebugTime() << " ID     " << id << "\n";
  if ( origin )
    kdDebugTime() << " Origin " << origin -> url().url() << "\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  KPlayerContainerNode* node = createBranch (id, origin);
  if ( node )
  {
    insert (node, after);
    if ( populated() )
      node -> reference();
    if ( groupsPopulated() )
      node -> reference();
  }
  return node;
}

void KPlayerContainerNode::populate (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::populate\n";
  kdDebugTime() << " Count  " << m_populate_nodes << "\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  reference();
  if ( ! populated() )
    doPopulate();
  m_populate_nodes ++;
}

void KPlayerContainerNode::doPopulate (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Populating node\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  bool apply_custom_order = media() -> customOrder() && (origin() || ! nodes().isEmpty());
  if ( origin() )
    origin() -> populate();
  KPlayerNodeList previous (nodes());
  KPlayerNodeMap map (m_node_map);
  m_nodes.clear();
  m_node_map.clear();
  bool group;
  QString id;
  source() -> start (false);
  while ( source() -> next (group, id) )
  {
    KPlayerNode* node = 0;
    if ( group )
    {
      KPlayerNodeMap::ConstIterator iterator = map.constFind (id);
      if ( iterator != map.constEnd() )
      {
        node = *iterator;
        previous.removeAll (node);
        iterator = map.constEnd();
        map.remove (id);
      }
    }
    else if ( ! acceptsDuplicates() )
      id = id.section ('#', 0, 0);
    if ( nodeById (id) )
      continue;
    if ( node )
      append (node);
    else
      node = group ? insertBranch (id) : insertLeaf (id);
    if ( node )
    {
      node -> reference();
      node -> countAttributes (m_attribute_counts);
    }
  }
  if ( ! previous.isEmpty() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Remaining " << previous.count() << "\n";
#endif
    KPlayerNodeList::ConstIterator iterator (previous.constBegin());
    while ( iterator != previous.constEnd() )
    {
      append (*iterator);
      ++ iterator;
    }
  }
  if ( apply_custom_order )
    applyCustomOrder();
}

void KPlayerContainerNode::populateGroups (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::populateGroups\n";
  kdDebugTime() << " Count  " << m_populate_groups << "\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  reference();
  if ( ! groupsPopulated() )
  {
    if ( populated() )
    {
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Adding references\n";
#endif
      if ( origin() )
        origin() -> populateGroups();
      KPlayerNodeList::ConstIterator iterator (nodes().constBegin());
      while ( iterator != nodes().constEnd() )
      {
	KPlayerNode* node = *iterator;
        if ( node -> isContainer() )
          node -> reference();
        ++ iterator;
      }
    }
    else
      doPopulateGroups();
  }
  m_populate_groups ++;
}

void KPlayerContainerNode::doPopulateGroups (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Populating groups\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  bool apply_custom_order = media() -> customOrder() && (origin() || ! nodes().isEmpty());
  if ( origin() )
    origin() -> populateGroups();
  KPlayerNodeList previous (nodes());
  KPlayerNodeMap map (m_node_map);
  m_nodes.clear();
  m_node_map.clear();
  bool group;
  QString id;
  source() -> start (true);
  while ( source() -> next (group, id) )
  {
    KPlayerNodeMap::ConstIterator iterator = map.constFind (id);
    KPlayerNode* node = iterator == map.constEnd() ? 0 : *iterator;
    if ( node )
    {
      previous.removeAll (node);
      map.remove (id);
      append (node);
    }
    else
      node = insertBranch (id);
    if ( node )
      node -> reference();
  }
  if ( ! previous.isEmpty() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Remaining " << previous.count() << "\n";
#endif
    KPlayerNodeList::ConstIterator iterator (previous.constBegin());
    while ( iterator != previous.constEnd() )
    {
      append (*iterator);
      ++ iterator;
    }
  }
  if ( apply_custom_order )
    applyCustomOrder();
}

void KPlayerContainerNode::populateAll (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::populateAll\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  populate();
  for ( KPlayerNodeList::ConstIterator iterator = nodes().constBegin(); iterator != nodes().constEnd(); ++ iterator )
  {
    KPlayerNode* node = *iterator;
    if ( node -> isContainer() )
      ((KPlayerContainerNode*) node) -> populateAll();
  }
}

void KPlayerContainerNode::vacate (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::vacate\n";
  kdDebugTime() << " Count  " << m_populate_nodes << "\n";
  kdDebugTime() << " ID     " << url().url() << "\n";
#endif
  if ( m_populate_nodes == 1 )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << "Vacating node\n";
    kdDebugTime() << " ID     " << url().url() << "\n";
#endif
    KPlayerNodeList list (nodes());
    for ( KPlayerNodeList::ConstIterator iterator = list.constBegin(); iterator != list.constEnd(); ++ iterator )
      (*iterator) -> release();
    if ( origin() )
      origin() -> vacate();
    m_attribute_counts.clear();
  }
  m_populate_nodes --;
  release();
}

void KPlayerContainerNode::vacateGroups (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::vacateGroups\n";
  kdDebugTime() << " Count  " << m_populate_groups << "\n";
  kdDebugTime() << " ID     " << url().url() << "\n";
#endif
  if ( m_populate_groups == 1 )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << "Vacating groups\n";
    kdDebugTime() << " URL    " << url().url() << "\n";
#endif
    KPlayerNodeList list (nodes());
    for ( KPlayerNodeList::ConstIterator iterator = list.constBegin(); iterator != list.constEnd(); ++ iterator )
    {
      KPlayerNode* node = *iterator;
      if ( node -> isContainer() )
        node -> release();
    }
    if ( origin() )
      origin() -> vacateGroups();
  }
  m_populate_groups --;
  release();
}

void KPlayerContainerNode::vacateAll (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::vacateAll\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  for ( KPlayerNodeList::ConstIterator iterator = nodes().constBegin(); iterator != nodes().constEnd(); ++ iterator )
  {
    KPlayerNode* node = *iterator;
    if ( node -> isContainer() )
      ((KPlayerContainerNode*) node) -> vacateAll();
  }
  vacate();
}

void KPlayerContainerNode::refreshNodes (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::refreshNodes\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  removed (nodes());
  if ( groupsPopulated() )
  {
    int count = m_populate_groups;
    m_populate_groups = 0;
    doPopulateGroups();
    m_populate_groups = count;
  }
  if ( populated() )
  {
    int count = m_populate_nodes;
    m_populate_nodes = 0;
    doPopulate();
    m_populate_nodes = count;
  }
  if ( ! attributeCounts().isEmpty() )
    emitAttributesUpdated (attributeCounts(), KPlayerPropertyCounts());
  emitAdded (nodes());
}

KPlayerNode* KPlayerContainerNode::nodeById (const QString& id) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::nodeById " << id << "\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeMap::ConstIterator iterator = m_node_map.constFind (id);
  return iterator == m_node_map.constEnd() ? 0 : *iterator;
}

KPlayerContainerNode* KPlayerContainerNode::getNodeById (const QString& id)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::getNodeById\n";
  kdDebugTime() << " ID     " << id << "\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNode* node = nodeById (id);
  if ( ! node || node -> isContainer() )
  {
    KPlayerContainerNode* container = (KPlayerContainerNode*) node;
    if ( ! container && source() -> verify (id) )
      container = insertBranch (id);
    if ( container )
      return container;
  }
  reference();
  release();
  return 0;
}

KPlayerContainerNode* KPlayerContainerNode::getNodeByPath (const QString& path)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::getNodeByPath\n";
  kdDebugTime() << " Path   " << path << "\n";
#endif
  KPlayerContainerNode* node = getNodeById (path.section ('/', 0, 0, QString::SectionSkipEmpty));
  if ( node )
  {
    QString subpath (path.section ('/', 1, 0xffffffff, QString::SectionSkipEmpty));
    if ( ! subpath.isEmpty() )
      node = node -> getNodeByPath (subpath);
  }
  return node;
}

KPlayerNode* KPlayerContainerNode::nextNode (KPlayerNode* node)
{
  int index = nodes().indexOf (node);
  return index >= 0 ? nodes().at (index + 1) : 0;
}

KPlayerMediaNode* KPlayerContainerNode::lastMediaNode (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::lastMediaNode\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  if ( ! nodes().isEmpty() )
  {
    KPlayerNodeList::ConstIterator iterator (nodes().constEnd());
    do
    {
      -- iterator;
      KPlayerMediaNode* medianode = (*iterator) -> lastMediaNode();
      if ( medianode )
      {
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " Found  " << medianode -> url().url() << "\n";
#endif
        return medianode;
      }
    }
    while ( iterator != nodes().constBegin() );
  }
  return previousMediaNode();
}

void KPlayerContainerNode::customOrderByName (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::customOrderByName\n";
#endif
  if ( ! customOrder() && allowsCustomOrder() )
  {
    QString key (sortKey());
    bool ascending = sortAscending();
    setSorting ("Name", true);
    m_nodes.sort();
    setSorting (key, ascending);
    setCustomOrder (true);
  }
}

bool KPlayerContainerNode::customOrder (void) const
{
  return ! origin() || media() -> hasCustomOrder() ? ! parent() || media() -> customOrder() : origin() -> customOrder();
}

void KPlayerContainerNode::setCustomOrder (bool custom)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::setCustomOrder\n";
  kdDebugTime() << " Custom " << custom << "\n";
#endif
  if ( parent() && allowsCustomOrder() )
  {
    if ( custom || (origin() && origin() -> customOrder() ) )
      media() -> setCustomOrder (custom);
    else
      media() -> resetCustomOrder();
    media() -> commit();
  }
}

bool KPlayerContainerNode::allowsCustomOrder (void) const
{
  return true;
}

bool KPlayerContainerNode::acceptsDuplicates (void) const
{
  return false;
}

int KPlayerContainerNode::compareByPosition (KPlayerNode* node1, KPlayerNode* node2)
{
  int i1 = nodes().indexOf (node1);
  int i2 = nodes().indexOf (node2);
  return i1 == i2 ? 0 : i1 < i2 ? -1 : 1;
}

void KPlayerContainerNode::addedLeaves (const QStringList& list)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::addedLeaves\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList nodes;
  if ( ! list.isEmpty() && populated() )
  {
    KPlayerPropertyCounts counts;
    QStringList::ConstIterator iterator (list.constBegin());
    while ( iterator != list.constEnd() )
    {
      KPlayerNode* node = insertLeaf (*iterator);
      if ( node )
      {
        node -> countAttributes (counts);
        nodes.append (node);
      }
      ++ iterator;
    }
    source() -> save();
    if ( ! counts.isEmpty() )
    {
      m_attribute_counts.add (counts);
      emitAttributesUpdated (counts, KPlayerPropertyCounts());
    }
  }
  emitAdded (nodes);
}

void KPlayerContainerNode::addedBranches (const QStringList& list)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::addedBranches\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList nodes;
  if ( ! list.isEmpty() && (populated() || groupsPopulated()) )
  {
    KPlayerPropertyCounts counts;
    QStringList::ConstIterator iterator (list.constBegin());
    while ( iterator != list.constEnd() )
    {
      KPlayerNode* node = insertBranch (*iterator);
      if ( node )
      {
        node -> countAttributes (counts);
        nodes.append (node);
      }
      ++ iterator;
    }
    if ( populated() )
    {
      source() -> save();
      if ( ! counts.isEmpty() )
      {
        m_attribute_counts.add (counts);
        emitAttributesUpdated (counts, KPlayerPropertyCounts());
      }
    }
  }
  emitAdded (nodes);
}

void KPlayerContainerNode::added (const QFileInfoList& list)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::added file list\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList nodes;
  if ( ! list.isEmpty() && (populated() || groupsPopulated()) )
  {
    KPlayerPropertyCounts counts;
    for ( QFileInfoList::ConstIterator iterator (list.constBegin()); iterator != list.constEnd(); ++ iterator )
    {
      const QFileInfo& info = *iterator;
      bool group = info.isDir();
      if ( group || populated() )
      {
        QString id (info.fileName());
        KPlayerNode* node = group ? insertBranch (id) : insertLeaf (id);
        if ( node )
        {
          node -> countAttributes (counts);
          nodes.append (node);
        }
      }
    }
    if ( populated() )
    {
      source() -> save();
      if ( ! counts.isEmpty() )
      {
        m_attribute_counts.add (counts);
        emitAttributesUpdated (counts, KPlayerPropertyCounts());
      }
    }
  }
  emitAdded (nodes);
}

void KPlayerContainerNode::added (KPlayerContainerNode* node, bool link)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::added subnodes\n";
#endif
  node -> populate();
  if ( ! node -> nodes().isEmpty() )
    added (node -> nodes(), link);
  node -> vacate();
}

KPlayerNode* KPlayerContainerNode::added (const KPlayerNodeList& list, bool link, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::added " << (link ? "link" : "copy") << " nodes\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  populate();
  if ( ! allowsCustomOrder() )
    after = 0;
  else if ( after && ! customOrder() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Key    " << sortKey() << "\n";
#endif
    m_nodes.sort();
    setCustomOrder (true);
  }
  KPlayerNodeList save;
  KPlayerPropertyCounts counts;
  KPlayerNode* saveAfter = after;
  KPlayerNodeList::ConstIterator iterator (list.constBegin());
  while ( iterator != list.constEnd() )
  {
    KPlayerNode* node = *iterator;
    QString id (node -> suggestId());
    KPlayerNode* subnode = nodeById (id);
    if ( subnode && acceptsDuplicates() )
    {
      if ( node -> isContainer() )
      {
        QString base (id);
        for ( int i = 0; subnode; i ++ )
        {
          id = base + QString::number (i);
          subnode = nodeById (id);
        }
      }
      else
      {
        KUrl url (id);
        for ( int i = 0; subnode; i ++ )
        {
          url.setRef (QString::number (i));
          id = url.url();
          subnode = nodeById (id);
        }
      }
    }
    if ( subnode )
    {
      if ( after || customOrder() )
      {
        m_nodes.removeAll (subnode);
        insert (subnode, after);
      }
      if ( subnode -> isContainer() && node -> isContainer() )
          ((KPlayerContainerNode*) subnode) -> added ((KPlayerContainerNode*) node, link);
    }
    else
    {
      if ( node -> isContainer() )
      {
        KPlayerContainerNode* container = (KPlayerContainerNode*) node;
        KPlayerContainerNode* origin = link ? container : container -> origin();
        while ( origin && ! canLink (origin) )
          origin = origin -> origin();
        KPlayerContainerNode* branch = insertBranch (id, after, origin);
        subnode = branch;
        if ( branch )
        {
          subnode -> media() -> setName (node -> name());
          if ( origin )
            branch -> save();
          else
            branch -> added (container, link);
        }
      }
      else
        subnode = insertLeaf (id, after);
      if ( subnode )
      {
        subnode -> countAttributes (counts);
        save.append (subnode);
      }
    }
    if ( after )
      after = subnode;
    ++ iterator;
  }
  if ( origin() && customOrder() && ! media() -> customOrder() )
  {
    m_nodes.clear();
    KPlayerNodeList::ConstIterator originit (origin() -> nodes().constBegin());
    while ( originit != origin() -> nodes().constEnd() )
    {
      KPlayerNode* originnode = *originit;
      KPlayerNode* node = nodeById (originnode -> id());
      if ( node )
        m_nodes.append (node);
#ifdef DEBUG_KPLAYER_NODE
      else
        kdDebugTime() << " Missing node " << originnode -> id() << "\n";
#endif
      ++ originit;
    }
  }
  if ( ! counts.isEmpty() )
  {
    m_attribute_counts.add (counts);
    emitAttributesUpdated (counts, KPlayerPropertyCounts());
  }
  emitAdded (save, saveAfter);
  source() -> save();
  vacate();
  return after && nodes().indexOf (after) >= 0 ? after : 0;
}

KPlayerNode* KPlayerContainerNode::moved (const KPlayerNodeList& list, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::moved\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  populate();
  if ( ! allowsCustomOrder() )
    after = 0;
  else if ( after )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Key    " << sortKey() << "\n";
#endif
    m_nodes.sort();
    setCustomOrder (true);
  }
  KPlayerNode* saveAfter = after;
  KPlayerNodeList::ConstIterator iterator (list.constBegin());
  while ( iterator != list.constEnd() )
  {
    KPlayerNode* node = *iterator;
    if ( after || customOrder() )
    {
      m_nodes.removeAll (node);
      insert (node, after);
    }
    if ( after )
      after = node;
    ++ iterator;
  }
  source() -> save();
  emitAdded (KPlayerNodeList(), saveAfter);
  vacate();
  return after && nodes().indexOf (after) >= 0 ? after : 0;
}

KPlayerNode* KPlayerContainerNode::lastNode (void)
{
  if ( nodes().isEmpty() )
    return this;
  if ( ! customOrder() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Sorting by " << sortKey() << "\n";
#endif
    m_nodes.sort();
    setCustomOrder (true);
  }
  return nodes().last();
}

bool KPlayerContainerNode::remove (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::remove\n";
  kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  bool found = m_nodes.removeAll (node);
  if ( found )
    m_node_map.remove (node -> id());
  return found;
}

void KPlayerContainerNode::removed (const KPlayerNodeList& nodes, const KPlayerPropertyCounts& counts)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::removed nodes and counts\n";
#endif
  emitRemoved (nodes);
  if ( ! counts.isEmpty() )
  {
    m_attribute_counts.subtract (counts);
    emitAttributesUpdated (KPlayerPropertyCounts(), counts);
  }
  nodes.releaseAll();
}

void KPlayerContainerNode::removed (const QStringList& ids)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::removed ID list\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList nodes;
  KPlayerPropertyCounts counts;
  if ( ! ids.isEmpty() )
  {
    populate();
    for ( QStringList::ConstIterator iterator (ids.constBegin()); iterator != ids.constEnd(); ++ iterator )
    {
      KPlayerNode* node = nodeById (*iterator);
      if ( node )
      {
        node -> countAttributes (counts);
        node -> reference();
        node -> detach();
        nodes.append (node);
      }
    }
    source() -> save();
    vacate();
  }
  removed (nodes, counts);
}

void KPlayerContainerNode::removed (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::removed node\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  removed (nodes());
  media() -> purge();
}

void KPlayerContainerNode::removed (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::removed\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList list (nodes);
  KPlayerPropertyCounts counts;
  if ( ! nodes.isEmpty() )
  {
    populate();
    KPlayerNodeList::ConstIterator iterator (list.constBegin());
    while ( iterator != list.constEnd() )
    {
      KPlayerNode* node = *iterator;
      if ( node -> isContainer() )
        ((KPlayerContainerNode*) node) -> removed();
      node -> countAttributes (counts);
      node -> reference();
      node -> detach();
      ++ iterator;
    }
    source() -> save();
    vacate();
  }
  removed (list, counts);
}

void KPlayerContainerNode::save (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::save\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  if ( ! origin() || media() -> customOrder() )
  {
    populate();
    QStringList children;
    KPlayerNodeList::ConstIterator iterator (nodes().constBegin());
    while ( iterator != nodes().constEnd() )
    {
      KPlayerNode* node = *iterator;
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Child  " << node -> url().url() << "\n";
#endif
      children.append (node -> id());
      ++ iterator;
    }
    vacate();
    media() -> setChildren (children);
  }
  else
    media() -> resetChildren();
  if ( origin() && ! url().isLocalFile() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Origin " << origin() -> url().url() << "\n";
#endif
    media() -> setOrigin (origin() -> url());
  }
  else
    media() -> resetOrigin();
  media() -> commit();
}

void KPlayerContainerNode::applyCustomOrder (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::applyCustomOrder\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList list (m_nodes);
  m_nodes.clear();
  const QStringList& children (media() -> children());
  QStringList::ConstIterator iterator (children.constBegin());
  while ( iterator != children.constEnd() )
  {
    QString id (*iterator);
    for ( KPlayerNodeList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++ it )
    {
      KPlayerNode* node = *it;
      if ( node -> id() == id )
      {
        list.removeAll (node);
        m_nodes.append (node);
        break;
      }
    }
    ++ iterator;
  }
  for ( KPlayerNodeList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++ it )
    m_nodes.append (*it);
}

void KPlayerContainerNode::detach (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Detaching subnodes\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList list (nodes());
  for ( KPlayerNodeList::ConstIterator iterator = list.constBegin(); iterator != list.constEnd(); ++ iterator )
    (*iterator) -> detach();
  emitDetached();
  KPlayerNode::detach();
}

void KPlayerContainerNode::detachOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNode::detachOrigin\n";
#endif
  populate();
  releaseOrigin();
  KPlayerNodeList::ConstIterator iterator (nodes().constBegin());
  while ( iterator != nodes().constEnd() )
  {
    KPlayerNode* node = *iterator;
    if ( node -> isContainer() )
    {
      KPlayerContainerNode* container = (KPlayerContainerNode*) node;
      if ( container -> origin() )
        container -> source() -> save();
    }
    ++ iterator;
  }
  source() -> save();
  vacate();
  if ( parent() && parent() -> origin() )
    parent() -> detachOrigin();
}

void KPlayerContainerNode::releaseOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::releaseOrigin\n";
#endif
  if ( populated() )
    origin() -> vacate();
  if ( groupsPopulated() )
    origin() -> vacateGroups();
  origin() -> release();
  setOrigin (0);
  source() -> deleteLater();
  setSource (new KPlayerStoreSource (this));
}

KPlayerTemporaryNode::~KPlayerTemporaryNode()
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Destroying temporary node\n";
#endif
}

KPlayerNode* KPlayerTemporaryNode::temporaryItem (const QString& id)
{
  KPlayerNode* node = nodeById (id);
  if ( ! node )
    node = insertLeaf (id);
  if ( node )
    node -> reference();
  return node;
}

KPlayerRootNode::KPlayerRootNode (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Creating root node\n";
#endif
}

KPlayerRootNode::~KPlayerRootNode()
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Destroying root node\n";
#endif
  m_root = 0;
}

void KPlayerRootNode::setupSource (void)
{
  QString home ("file:" + QDir::homePath());
  m_default_ids << "nowplaying" << "recent" << "playlists" << "collection" << "devices" << "file:/" << home;
  KPlayerContainerNode* node = new KPlayerNowPlayingNode;
  node -> setup (this, "nowplaying");
  node -> media() -> setDefaultName (i18n("Now Playing"));
  m_defaults.insert ("nowplaying", node);
  node = new KPlayerPlaylistNode;
  node -> setup (this, "playlists");
  node -> media() -> setDefaultName (i18n("Playlists"));
  m_defaults.insert ("playlists", node);
  node = new KPlayerCollectionNode;
  node -> setup (this, "collection");
  node -> media() -> setDefaultName (i18n("Collection"));
  m_defaults.insert ("collection", node);
  node = new KPlayerDevicesNode;
  node -> setup (this, "devices");
  node -> media() -> setDefaultName (i18n("Devices"));
  m_defaults.insert ("devices", node);
  node = new KPlayerRecentsNode;
  node -> setup (this, "recent");
  node -> media() -> setDefaultName (i18n("Recent"));
  m_defaults.insert ("recent", node);
#if 0
  node = new KPlayerSearchesNode;
  node -> setup (this, "searches");
  node -> media() -> setDefaultName (i18n("Searches"));
  m_defaults.insert ("searches", node);
#endif
  node = new KPlayerExternalNode;
  node -> setup (this, "file:/");
  node -> media() -> setDefaultName (i18n("Root Directory"));
  m_defaults.insert ("file:/", node);
  m_externals.insert (node -> media() -> url().url(), node);
  node = new KPlayerExternalNode;
  node -> setup (this, home);
  node -> media() -> setDefaultName (i18n("Home Directory"));
  m_defaults.insert (home, node);
  m_externals.insert (node -> media() -> url().url(), node);
  m_temp = new KPlayerTemporaryNode;
  temporaryNode() -> setup (this, "temp");
  temporaryNode() -> reference();
  m_source = new KPlayerRootSource (this);
}

KUrl KPlayerRootNode::url (void) const
{
  return id();
}

KPlayerContainerNode* KPlayerRootNode::createBranch (const QString& id, KPlayerContainerNode*)
{
  KPlayerContainerNode* node;
  KPlayerContainerNodeMap::ConstIterator iterator = m_defaults.constFind (id);
  if ( iterator == m_defaults.constEnd() )
  {
    node = getNodeByUrl (id);
    /*if ( node -> parent() -> parent() )
    {
      //origin = node;
      node -> reference();
      node -> release();
      node = new KPlayerExternalNode;
      node -> setup (this, id);
    }*/
  }
  else
    node = *iterator;
  return node;
}

KPlayerContainerNode* KPlayerRootNode::getNodeByUrl (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNode::getNodeByUrl\n";
  kdDebugTime() << " URL    " << url.url() << "\n";
  kdDebugTime() << " Path   " << url.path() << "\n";
#endif
  QString path (url.path());
  KPlayerContainerNode* node = 0;
  if ( url.protocol() == "kplayer" )
    node = root();
  else
  {
    int length = 0;
    QString urls (url.url());
    KPlayerContainerNodeMap::ConstIterator iterator (m_externals.constBegin());
    while ( iterator != m_externals.constEnd() )
    {
      if ( urls.startsWith (iterator.key()) && iterator.key().length() > length
        && url.path().startsWith (iterator.value() -> url().path()) )
      {
        node = iterator.value();
        length = iterator.key().length();
        path = url.path().mid (node -> url().path().length());
      }
      ++ iterator;
    }
    if ( ! node )
    {
      QString id (url.protocol() + ":/");
      node = new KPlayerExternalNode;
      node -> setup (root(), id);
      m_externals.insert (node -> media() -> url().url(), node);
    }
  }
  return path.section ('/', 0, 0, QString::SectionSkipEmpty).isEmpty() ? node
    : node -> getNodeByPath (path);
}

KPlayerExternalNode::~KPlayerExternalNode()
{
}

KUrl KPlayerExternalNode::url (void) const
{
  return id();
}

KPlayerDirectoryNode::~KPlayerDirectoryNode()
{
}

void KPlayerDirectoryNode::setupSource (void)
{
//m_source = origin() ? (KPlayerSource*) new KPlayerOriginSource (this)
//  : (KPlayerSource*) new KPlayerDirectorySource (this);
  m_source = new KPlayerDirectorySource (this);
}

bool KPlayerDirectoryNode::isDirectory (void) const
{
  return true;
}

bool KPlayerDirectoryNode::canRename (void) const
{
  return false;
}

bool KPlayerDirectoryNode::allowsCustomOrder (void) const
{
  return false;
}

KPlayerNode* KPlayerDirectoryNode::createLeaf (const QString& id)
{
  KPlayerNode* node = new KPlayerFileNode;
  node -> setup (this, id);
  return node;
}

KPlayerContainerNode* KPlayerDirectoryNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  KPlayerContainerNode* node = new KPlayerDirectoryNode;
  node -> setup (this, id, origin);
  return node;
}

KPlayerGroupNode::~KPlayerGroupNode()
{
}

void KPlayerGroupNode::setupSource (void)
{
  m_source = origin() ? (KPlayerSource*) new KPlayerOriginSource (this) : (KPlayerSource*) new KPlayerStoreSource (this);
}

QString KPlayerGroupNode::icon (void) const
{
  return origin() ? origin() -> icon() : "folder-green";
}

QString KPlayerGroupNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder-green";
}

bool KPlayerGroupNode::isGroup (void) const
{
  return true;
}

bool KPlayerGroupNode::canRename (void) const
{
  return parent() && parent() -> parent();
}

KPlayerCollectionNode::~KPlayerCollectionNode()
{
}

void KPlayerCollectionNode::setupOrigin (void)
{
#if 0
#ifdef DEBUG_KPLAYER_NODE
  if ( isGrouped() )
    kdDebugTime() << " Key    " << groupingKey() << "\n";
#endif
  if ( isParentGrouped() )
    setOrigin (parent());
  else if ( isGrouped() )
    setOrigin (createBranch (id(), origin()));
#endif
}

#if 0
void KPlayerCollectionNode::setupSource (void)
{
  m_source =
    isParentGrouped() ? (KPlayerSource*) new KPlayerKeySource (this) :
    isGrouped() ? (KPlayerSource*) new KPlayerGroupSource (this) :
    origin() ? (KPlayerSource*) new KPlayerOriginSource (this) : (KPlayerSource*) new KPlayerStoreSource (this);
}
#endif

bool KPlayerCollectionNode::isCollection (void) const
{
  return true;
}

bool KPlayerCollectionNode::canLink (KPlayerContainerNode* node) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerCollectionNode::canLink\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  return node -> isDirectory();
}

bool KPlayerCollectionNode::allowsCustomOrder (void) const
{
  return false;
}

KPlayerContainerNode* KPlayerCollectionNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  KPlayerContainerNode* node = new KPlayerCollectionNode;
  node -> setup (this, id, origin);
  return node;
}

#if 0
void KPlayerCollectionNode::group (const QString& key)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Group nodes\n";
  if ( ! key.isNull() )
    kdDebugTime() << " Key    " << key << "\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  if ( groupingKey() == key || key.isNull() && parent() && isParentGrouped() )
    return;
  media() -> setGroupingKey (key);
  KPlayerNodeList list (nodes());
  for ( KPlayerNodeList::ConstIterator iterator = list.constBegin(); iterator != list.constEnd(); ++ iterator )
    (*iterator) -> detach();
  emitRemoved (nodes());
  m_nodes.clear();
  m_node_map.clear();
  if ( key.isNull() )
  {
    if ( origin() && origin() -> canGroup() )
    {
      origin() -> release();
      setOrigin (0);
    }
    delete m_source;
    m_source = m_origin ?
        (KPlayerSource*) new KPlayerOriginSource (this)
      : (KPlayerSource*) new KPlayerStoreSource (this);
  }
  else
  {
    if ( ! origin() || ! origin() -> canGroup() )
    {
      delete m_source;
      m_source = new KPlayerGroupSource (this);
    }
    if ( ! origin() )
      setOrigin (createBranch (id()));
  }
  if ( populated() )
    doPopulate();
  else if ( groupsPopulated() )
    doPopulateGroups();
}
#endif

KPlayerPlaylistNode::~KPlayerPlaylistNode()
{
}

void KPlayerPlaylistNode::setupSource (void)
{
  m_duplicates = acceptsDuplicates();
  KPlayerGroupNode::setupSource();
  connect (configuration(), SIGNAL (updated()), SLOT (configurationUpdated()));
  if ( ! parent() -> parent() && id() == "playlists" && ! media() -> hasChildren() )
  {
    KConfigGroup group (KPlayerEngine::engine() -> meta() -> group ("Playlist Entries"));
    int entries = group.readEntry ("Entries", 0);
    if ( entries )
    {
      KPlayerNode* np = parent() -> nodeById ("nowplaying");
      if ( np && ! np -> media() -> hasChildren() )
      {
        QString name (i18n("My Playlist"));
        QStringList children;
        children.append (name);
        media() -> setChildren (children);
        media() -> commit();
        QString urls (url (name).url());
        KConfigGroup config (media() -> config() -> group (urls));
        for ( int i = 0; i < entries; i ++ )
        {
          QString no (QString::number (i));
          config.writeEntry ("Child" + no, group.readEntry ("Entry " + no));
        }
        config.writeEntry ("Children", entries);
        np -> media() -> setChildren (children);
        np -> media() -> commit();
        media() -> config() -> group (np -> url (name).url()).writeEntry ("Origin", urls);
      }
    }
    KPlayerEngine::engine() -> meta() -> deleteGroup ("Playlist Entries");
  }
}

void KPlayerPlaylistNode::setupOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNode::setupOrigin\n";
  if ( origin() )
    kdDebugTime() << " Origin " << origin() -> url().url() << "\n";
#endif
  KPlayerGroupNode::setupOrigin();
  if ( origin() && origin() -> hasProperties() )
  {
    media() -> setDefaultName (origin() -> name());
    connect (origin() -> parent(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
      SLOT (originUpdated (KPlayerContainerNode*, KPlayerNode*)));
  }
}

void KPlayerPlaylistNode::originUpdated (KPlayerContainerNode*, KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNode::originUpdated\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  if ( node == origin() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Name   " << name() << "\n";
#endif
    if ( name() != origin() -> name() )
    {
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Origin " << origin() -> name() << "\n";
#endif
      media() -> setName (origin() -> name());
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Name   " << name() << "\n";
#endif
    }
    media() -> commit();
  }
}

void KPlayerPlaylistNode::releaseOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNode::releaseOrigin\n";
#endif
  if ( origin() && origin() -> hasProperties() )
    disconnect (origin() -> parent(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
      this, SLOT (originUpdated (KPlayerContainerNode*, KPlayerNode*)));
  KPlayerGroupNode::releaseOrigin();
}

QString KPlayerPlaylistNode::icon (void) const
{
  return origin() ? origin() -> icon() : "folder-violet";
}

QString KPlayerPlaylistNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder-violet";
}

bool KPlayerPlaylistNode::canLink (KPlayerContainerNode* node) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNode::canLink\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  return node -> isCollection() || node -> isDirectory();
}

bool KPlayerPlaylistNode::isPlaylist (void) const
{
  return true;
}

bool KPlayerPlaylistNode::acceptsDuplicates (void) const
{
  return configuration() -> allowDuplicateEntries();
}

KPlayerContainerNode* KPlayerPlaylistNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  KPlayerContainerNode* node = new KPlayerPlaylistNode;
  node -> setup (this, id, origin);
  return node;
}

void KPlayerPlaylistNode::configurationUpdated (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNode::configurationUpdated\n";
#endif
  if ( ! acceptsDuplicates() && m_duplicates )
  {
    if ( populated() && ! origin() )
    {
      KPlayerNodeMap map;
      KPlayerNodeList list;
      KPlayerNodeList::ConstIterator iterator (nodes().constBegin());
      while ( iterator != nodes().constEnd() )
      {
	KPlayerNode* node = *iterator;
        QString id (node -> id().section ('#', 0, 0));
        KPlayerNodeMap::ConstIterator iterator = map.constFind (id);
        if ( iterator == map.constEnd() )
          map.insert (id, node);
        else
          list.append (node);
        ++ iterator;
      }
      if ( ! list.isEmpty() )
        removed (list);
    }
    m_duplicates = false;
  }
}

KPlayerNowPlayingNode::~KPlayerNowPlayingNode()
{
}

KPlayerContainerNode* KPlayerNowPlayingNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  KPlayerContainerNode* node = new KPlayerNowPlayingNode;
  node -> setup (this, id, origin);
  return node;
}

bool KPlayerNowPlayingNode::canLink (KPlayerContainerNode* node) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNowPlayingNode::canLink\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  return node -> isCollection() || node -> isDirectory() || node -> hasProperties()
    || ( node -> isPlaylist() && ! node -> isRecent() && ! node -> isNowPlaying() );
}

bool KPlayerNowPlayingNode::isNowPlaying (void) const
{
  return true;
}

QString KPlayerNowPlayingNode::icon (void) const
{
  return origin() ? origin() -> icon() : "folder-red";
}

QString KPlayerNowPlayingNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder-red";
}

void KPlayerNowPlayingNode::setupOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNowPlayingNode::setupOrigin\n";
  if ( origin() )
    kdDebugTime() << " Origin " << origin() -> url().url() << "\n";
#endif
  KPlayerPlaylistNode::setupOrigin();
  if ( origin() && origin() -> hasProperties() && ((KPlayerDeviceNode*) origin()) -> diskDevice() )
  {
    KPlayerDiskNode* disk = (KPlayerDiskNode*) origin();
    if ( disk -> dataDisk() )
    {
      disk -> getLocalPath();
      if ( disk -> hasLocalPath() )
      {
        KPlayerContainerNode* origin = root() -> getNodeByUrl (disk -> localPath());
        if ( origin )
        {
          disconnect (m_origin -> parent(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
            this, SLOT (originUpdated (KPlayerContainerNode*, KPlayerNode*)));
          m_origin -> reference();
          m_origin -> release();
          setOrigin (origin);
        }
      }
    }
  }
}

void KPlayerNowPlayingNode::originUpdated (KPlayerContainerNode*, KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNowPlayingNode::originUpdated\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  if ( node == origin() && node -> hasProperties() && ((KPlayerDeviceNode*) node) -> diskDevice() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Name   " << name() << "\n";
#endif
    if ( name() != origin() -> name() )
    {
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Origin " << origin() -> name() << "\n";
#endif
      media() -> setName (origin() -> name());
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Name   " << name() << "\n";
#endif
    }
    KPlayerDiskNode* disk = (KPlayerDiskNode*) node;
    if ( disk -> dataDisk() )
    {
      disk -> getLocalPath();
      if ( disk -> hasLocalPath() )
      {
        KPlayerContainerNode* origin = root() -> getNodeByUrl (disk -> localPath());
        if ( origin )
        {
          disconnect (node -> parent(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
            this, SLOT (originUpdated (KPlayerContainerNode*, KPlayerNode*)));
          node -> release();
          origin -> reference();
          setOrigin (origin);
          if ( groupsPopulated() )
          {
            disk -> vacateGroups();
            origin -> populateGroups();
          }
          if ( populated() )
          {
            disk -> vacate();
            origin -> populate();
          }
          added (origin, true);
          media() -> commit();
        }
      }
    }
    media() -> commit();
  }
}

KPlayerRecentNode::~KPlayerRecentNode()
{
}

bool KPlayerRecentNode::isRecent (void) const
{
  return true;
}

bool KPlayerRecentNode::canLink (KPlayerContainerNode* node) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerRecentNode::canLink\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  return node -> isCollection() || node -> isDirectory() || node -> hasProperties()
    || ( node -> isPlaylist() && ! node -> isRecent() && ! node -> isNowPlaying() );
}

KPlayerContainerNode* KPlayerRecentNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  KPlayerContainerNode* node = new KPlayerRecentNode;
  node -> setup (this, id, origin);
  return node;
}

QString KPlayerRecentNode::icon (void) const
{
  return origin() ? origin() -> icon() : "folder-red";
}

QString KPlayerRecentNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder-red";
}

KPlayerRecentsNode::~KPlayerRecentsNode()
{
}

void KPlayerRecentsNode::setupSource (void)
{
  KPlayerRecentNode::setupSource();
  if ( ! media() -> hasChildren() )
  {
    KConfigGroup group (KPlayerEngine::engine() -> config() -> group ("Recent Files"));
    int limit = configuration() -> recentListSize();
    QStringList children;
    for ( int i = 1; i <= limit; i ++ )
    {
      QString name (group.readEntry ("File" + QString::number (i)));
      if ( name.isEmpty() )
        break;
      children.append (name);
    }
    if ( ! children.isEmpty() )
    {
      media() -> setChildren (children);
      setCustomOrder (true);
    }
    KPlayerEngine::engine() -> config() -> deleteGroup ("Recent Files");
  }
}

void KPlayerRecentsNode::addRecent (const KPlayerNodeList& list)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerRecentsNode::addRecent\n";
#endif
  if ( configuration() -> recentListSize() > 0 && ! list.isEmpty() )
  {
    populate();
    bool just_move = true;
    KPlayerNodeList::ConstIterator iterator (list.constBegin());
    while ( iterator != list.constEnd() )
    {
      if ( (*iterator) -> parent() != this )
      {
        just_move = false;
        break;
      }
      ++ iterator;
    }
    if ( just_move )
      move (list, this);
    else if ( list.count() == 1 )
    {
      KPlayerNode* listnode = list.first();
      bool container = listnode -> isContainer();
      KPlayerNodeList previous;
      KPlayerNodeList::ConstIterator iterator (nodes().constBegin());
      while ( iterator != nodes().constEnd() )
      {
	KPlayerNode* node = *iterator;
        if ( ( container && node -> isContainer() && ((KPlayerContainerNode*) node) -> origin() == listnode )
            || ( ! container && ! node -> isContainer() && node -> media() == listnode -> media() ) )
          previous.append (node);
        ++ iterator;
      }
      if ( ! previous.isEmpty() )
        remove (previous);
      prepend (list);
      if ( container && ! listnode -> hasProperties() )
      {
        KPlayerNode* firstnode = nodes().first();
        firstnode -> media() -> setName (i18n("%1 in %2", listnode -> name(), listnode -> parent() -> name()));
        firstnode -> media() -> commit();
      }
    }
    else
    {
      QString name (list.count() == 2 ? i18n("%1 and %2", list.first() -> name(), list.last() -> name())
        : i18n("%1 and %2 more", list.first() -> name(), QString::number (list.count() - 1)));
      QString id (name);
      for ( int i = 0; nodeById (id); i ++ )
        id = name + QString::number (i);
      prependBranch (id);
      KPlayerContainerNode* container = getNodeById (id);
      container -> media() -> setName (name);
      container -> prepend (list);
    }
    configurationUpdated();
    vacate();
  }
}

void KPlayerRecentsNode::configurationUpdated (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerRecentsNode::configurationUpdated\n";
#endif
  KPlayerPlaylistNode::configurationUpdated();
  int limit = configuration() -> recentListSize();
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " Limit  " << limit << "\n";
#endif
  populate();
  if ( int (nodes().count()) > limit )
  {
    QList<KPlayerNode*> list (nodes().mid (limit));
    remove ((const KPlayerNodeList&) list);
  }
  vacate();
}

KPlayerDevicesNode::~KPlayerDevicesNode()
{
}

void KPlayerDevicesNode::setupSource (void)
{
  connect (Solid::DeviceNotifier::instance(), SIGNAL (deviceAdded (const QString&)), SLOT (deviceAdded (const QString&)));
  connect (Solid::DeviceNotifier::instance(), SIGNAL (deviceRemoved (const QString&)), SLOT (deviceRemoved (const QString&)));
  m_source = new KPlayerDevicesSource (this);
}

KPlayerContainerNode* KPlayerDevicesNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  QMap<QString, QString>::ConstIterator iterator = m_type_map.constFind (id);
  QString type (iterator == m_type_map.constEnd() ? media() -> type (id) : iterator.value());
  KPlayerDeviceNode* node = type == "TV" ? (KPlayerDeviceNode*) new KPlayerTVNode
    : type == "DVB" ? (KPlayerDeviceNode*) new KPlayerDVBNode : (KPlayerDeviceNode*) new KPlayerDiskNode;
  node -> setup (this, id, origin);
  return node;
}

QString KPlayerDevicesNode::icon (void) const
{
  return "computer";
}

QString KPlayerDevicesNode::openIcon (void) const
{
  return icon();
}

bool KPlayerDevicesNode::isDevices (void) const
{
  return true;
}

bool KPlayerDevicesNode::canRename (void) const
{
  return false;
}

bool KPlayerDevicesNode::canSaveAsPlaylist (void) const
{
  return false;
}

KPlayerContainerNode* KPlayerDevicesNode::getNodeByPath (const QString& path)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::getNodeByPath\n";
  kdDebugTime() << " Path   " << path << "\n";
#endif
  return getNodeById ('/' + path);
}

void KPlayerDevicesNode::deviceAdded (const QString& udi)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::deviceAdded\n";
  kdDebugTime() << " Device " << udi << "\n";
#endif
  QStringList current, previous;
  update (current, previous);
  if ( ! current.isEmpty() )
    addedBranches (current);
  if ( ! previous.isEmpty() )
    KPlayerContainerNode::removed (previous);
}

void KPlayerDevicesNode::deviceRemoved (const QString& udi)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::deviceRemoved\n";
  kdDebugTime() << " Device " << udi << "\n";
#endif
  QStringList current, previous;
  update (current, previous);
  if ( ! current.isEmpty() )
    addedBranches (current);
  if ( ! previous.isEmpty() )
    KPlayerContainerNode::removed (previous);
}

void KPlayerDevicesNode::update (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::updateSolid\n";
#endif
#ifdef DEBUG_KPLAYER_SOLID
  foreach ( const Solid::Device& device, Solid::Device::allDevices() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Device " << device.udi().toLatin1().constData() << "\n";
    kdDebugTime() << "  Parent  " << device.parentUdi().toLatin1().constData() << "\n";
    if ( ! device.vendor().isEmpty() )
      kdDebugTime() << "  Vendor  " << device.vendor().toLatin1().constData() << "\n";
    if ( ! device.product().isEmpty() )
      kdDebugTime() << "  Product " << device.product().toLatin1().constData() << "\n";
    if ( ! device.icon().isEmpty() )
      kdDebugTime() << "  Icon    " << device.icon().toLatin1().constData() << "\n";
#endif
    if ( device.isDeviceInterface (Solid::DeviceInterface::Block) )
    {
      const Solid::Block* block = device.as<Solid::Block>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Device  " << block -> device().toLatin1().constData() << "\n";
      kdDebugTime() << "  Major   " << block -> deviceMajor() << "\n";
      kdDebugTime() << "  Minor   " << block -> deviceMinor() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::StorageAccess) )
    {
      const Solid::StorageAccess* access = device.as<Solid::StorageAccess>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Mounted " << access -> isAccessible() << "\n";
      if ( ! access -> filePath().isEmpty() )
        kdDebugTime() << "  Path    " << access -> filePath().toLatin1().constData() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::StorageDrive) )
    {
      const Solid::StorageDrive* drive = device.as<Solid::StorageDrive>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Bus     " << drive -> bus() << "\n";
      kdDebugTime() << "  Drive   " << drive -> driveType() << "\n";
      kdDebugTime() << "  Remove  " << drive -> isRemovable() << "\n";
      kdDebugTime() << "  Hotplug " << drive -> isHotpluggable() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::OpticalDrive) )
    {
      const Solid::OpticalDrive* drive = device.as<Solid::OpticalDrive>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Support " << drive -> supportedMedia() << "\n";
      kdDebugTime() << "  Read    " << drive -> readSpeed() << "\n";
      kdDebugTime() << "  Write   " << drive -> writeSpeed() << "\n";
      kdDebugTime() << "  Speeds  " << drive -> writeSpeeds() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::StorageVolume) )
    {
      const Solid::StorageVolume* volume = device.as<Solid::StorageVolume>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Usage   " << volume -> usage() << "\n";
      if ( ! volume -> fsType().isEmpty() )
        kdDebugTime() << "  System  " << volume -> fsType().toLatin1().constData() << "\n";
      if ( ! volume -> label().isEmpty() )
        kdDebugTime() << "  Label   " << volume -> label().toLatin1().constData() << "\n";
      if ( ! volume -> uuid().isEmpty() )
        kdDebugTime() << "  ID      " << volume -> uuid().toLatin1().constData() << "\n";
      kdDebugTime() << "  Size    " << volume -> size() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::OpticalDisc) )
    {
      const Solid::OpticalDisc* disc = device.as<Solid::OpticalDisc>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Disc    " << disc -> discType() << "\n";
      kdDebugTime() << "  Content " << disc -> availableContent() << "\n";
      kdDebugTime() << "  Append  " << disc -> isAppendable() << "\n";
      kdDebugTime() << "  Blank   " << disc -> isBlank() << "\n";
      kdDebugTime() << "  Rewrite " << disc -> isRewritable() << "\n";
      kdDebugTime() << "  Capacity " << disc -> capacity() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::AudioInterface) )
    {
      const Solid::AudioInterface* interface = device.as<Solid::AudioInterface>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Audio   " << interface -> deviceType() << "\n";
      kdDebugTime() << "  Type    " << interface -> soundcardType() << "\n";
      kdDebugTime() << "  Driver  " << interface -> driver() << "\n";
      if ( ! interface -> name().isEmpty() )
        kdDebugTime() << "  Name    " << interface -> name().toLatin1().constData() << "\n";
      if ( interface -> driver() == Solid::AudioInterface::Alsa )
      {
        QVariantList list = interface -> driverHandle().toList();
        if ( list.count() )
          kdDebugTime() << "  Card    " << list[0].toString().toLatin1().constData() << "\n";
        if ( list.count() > 1 )
          kdDebugTime() << "  Device  " << list[1].toInt() << "\n";
        if ( list.count() > 2 )
          kdDebugTime() << "  Subdevice " << list[2].toInt() << "\n";
      }
      if ( interface -> driver() == Solid::AudioInterface::OpenSoundSystem )
        kdDebugTime() << "  Path    " << interface -> driverHandle().toString().toLatin1().constData() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::DvbInterface) )
    {
      const Solid::DvbInterface* interface = device.as<Solid::DvbInterface>();
#ifdef DEBUG_KPLAYER_NODE
      if ( ! interface -> device().isEmpty() )
        kdDebugTime() << "  DVB     " << interface -> device().toLatin1().constData() << "\n";
      kdDebugTime() << "  Adapter " << interface -> deviceAdapter() << "\n";
      kdDebugTime() << "  Type    " << interface -> deviceType() << "\n";
      kdDebugTime() << "  Index   " << interface -> deviceIndex() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::Video) )
    {
      const Solid::Video* video = device.as<Solid::Video>();
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << "  Protos  " << video -> supportedProtocols() << "\n";
      kdDebugTime() << "  Drivers " << video -> supportedDrivers() << "\n";
      kdDebugTime() << "  Path    " << video -> driverHandle ("video4linux").toString() << "\n";
#endif
    }
    if ( device.isDeviceInterface (Solid::DeviceInterface::GenericInterface) )
    {
      const Solid::GenericInterface* interface = device.as<Solid::GenericInterface>();
      QMap<QString, QVariant> properties = interface -> allProperties();
      QMap<QString, QVariant>::const_iterator iterator = properties.constBegin();
      while ( iterator != properties.constEnd() )
      {
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << "   " << iterator.key().toLatin1().constData() << " " << iterator.value() << "\n";
#endif
        ++ iterator;
      }
    }
  }
#endif
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::update\n";
#endif
  QStringList current, previous;
  update (current, previous);
}

void KPlayerDevicesNode::update (QStringList& current, QStringList& previous)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::update\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  previous = m_devices;
  m_name_map.clear();
  QStringList paths, dvdpaths;
  foreach ( const Solid::Device& device, Solid::Device::listFromType (Solid::DeviceInterface::OpticalDrive) )
    if ( const Solid::OpticalDrive* drive = device.as<Solid::OpticalDrive>() )
      if ( const Solid::Block* block = device.as<Solid::Block>() )
      {
        QString path = block -> device();
        if ( ! path.isEmpty() )
        {
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Device " << path.toLatin1().constData() << "\n";
          kdDebugTime() << " Support " << drive -> supportedMedia() << "\n";
#endif
          if ( drive -> supportedMedia() >= Solid::OpticalDrive::Dvd )
            updateDevice (device.udi(), path, I18N_NOOP("DVD"), dvdpaths, current, previous);
          else
            updateDevice (device.udi(), path, I18N_NOOP("CD"), paths, current, previous);
        }
      }
  updateDeviceNames (paths, "CD");
  updateDeviceNames (dvdpaths, "DVD");
  foreach ( const Solid::Device& device, Solid::Device::listFromType (Solid::DeviceInterface::DvbInterface) )
    if ( const Solid::DvbInterface* dvb = device.as<Solid::DvbInterface>() )
      if ( dvb -> deviceAdapter() >= 0 && dvb -> deviceType() == Solid::DvbInterface::DvbVideo )
      {
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " Device " << dvb -> device().toLatin1().constData() << "\n";
        kdDebugTime() << " Adapter " << dvb -> deviceAdapter() << "\n";
        kdDebugTime() << " Type   " << dvb -> deviceType() << "\n";
        kdDebugTime() << " Index  " << dvb -> deviceIndex() << "\n";
#endif
        updateDevice (device.udi(), "/dev/dvb/adapter" + QString::number (dvb -> deviceAdapter()), I18N_NOOP("DVB"),
          paths, current, previous);
      }
  updateDeviceNames (paths, "DVB");
  foreach ( const Solid::Device& device, Solid::Device::listFromType (Solid::DeviceInterface::Video) )
    if ( const Solid::Video* video = device.as<Solid::Video>() )
    {
      QString path = video -> driverHandle ("video4linux").toString();
      if ( path.contains ("video") )
      {
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " Device " << path.toLatin1().constData() << "\n";
        kdDebugTime() << " Protocols " << video -> supportedProtocols() << "\n";
        kdDebugTime() << " Drivers " << video -> supportedDrivers() << "\n";
#endif
        updateDevice (device.udi(), path, I18N_NOOP("TV"), paths, current, previous);
      }
    }
  updateDeviceNames (paths, "TV");
  QStringList::ConstIterator slit (previous.constBegin());
  while ( slit != previous.constEnd() )
  {
    m_devices.removeAll (*slit);
    m_type_map.remove (*slit);
    m_device_udi.remove (*slit);
    ++ slit;
  }
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " Added  " << current.count() << "\n";
  kdDebugTime() << " Removed " << previous.count() << "\n";
  kdDebugTime() << " Devices " << m_devices.count() << "\n";
#endif
}

void KPlayerDevicesNode::updateDevice (const QString& udi, const QString& path, const char* type,
  QStringList& paths, QStringList& current, QStringList& previous)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::updateDevice\n";
#endif
  QFileInfo info (path);
  if ( info.isReadable() && ! media() -> hidden (path) )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Path   " << path.toLatin1().constData() << "\n";
    kdDebugTime() << " Type   " << type << "\n";
#endif
    if ( m_type_map.contains (path) )
      previous.removeAll (path);
    else
    {
      current.append (path);
      m_devices.append (path);
    }
    m_type_map.insert (path, type);
    m_name_map.insert (path, i18n("%1 Device", type));
    m_device_udi.insert (path, udi);
    paths.append (path);
  }
}

void KPlayerDevicesNode::updateDeviceNames (QStringList& paths, const char* type)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::updateDeviceNames\n";
  kdDebugTime() << " Type   " << type << "\n";
  kdDebugTime() << " Count  " << paths.count() << "\n";
#endif
  if ( paths.count() > 1 )
  {
    paths.sort();
    QStringList::ConstIterator iterator (paths.constBegin());
    int i = 1;
    while ( iterator != paths.constEnd() )
    {
      m_name_map.insert (*iterator, i18n("%1 Device %2", type, i));
      ++ iterator;
      ++ i;
    }
  }
  paths.clear();
}

void KPlayerDevicesNode::removed (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::removed\n";
#endif
  KPlayerNodeList::ConstIterator nlit (nodes.constBegin());
  while ( nlit != nodes.constEnd() )
  {
    QString id ((*nlit) -> id());
    m_devices.removeAll (id);
    m_type_map.remove (id);
    ++ nlit;
  }
  KPlayerContainerNode::removed (nodes);
}

KPlayerDeviceNode::~KPlayerDeviceNode()
{
}

QString KPlayerDeviceNode::icon (void) const
{
  return "video-television";
}

QString KPlayerDeviceNode::openIcon (void) const
{
  return icon();
}

bool KPlayerDeviceNode::hasProperties (void) const
{
  return true;
}

bool KPlayerDeviceNode::allowsCustomOrder (void) const
{
  return false;
}

bool KPlayerDeviceNode::diskDevice (void)
{
  return false;
}

void KPlayerDeviceNode::removed (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDeviceNode::removed\n";
  kdDebugTime() << " URL    " << metaurl().url() << "\n";
#endif
  KPlayerContainerNode::removed();
  parent() -> media() -> setHidden (id(), true);
}

KPlayerDiskNode::~KPlayerDiskNode()
{
  if ( disk() )
    KPlayerMedia::release (device());
}

void KPlayerDiskNode::setupMedia (void)
{
  m_fast_autodetect = false;
  m_media = m_device = KPlayerMedia::deviceProperties (metaurl());
  m_disk = 0;
  if ( ! media() -> hasType() )
    media() -> setType (parent() -> deviceType (id()));
  media() -> setDefaultName (parent() -> deviceName (id()));
  checkDisk();
  if ( ! disk() )
    connect (media(), SIGNAL (updated()), SLOT (updated()));
}

void KPlayerDiskNode::setupSource (void)
{
  connect (Solid::DeviceNotifier::instance(), SIGNAL (deviceAdded (const QString&)), SLOT (deviceAdded (const QString&)));
  connect (Solid::DeviceNotifier::instance(), SIGNAL (deviceRemoved (const QString&)), SLOT (deviceRemoved (const QString&)));
  m_solid_device = Solid::Device (parent() -> deviceUdi (id()));
  m_source = new KPlayerDiskSource (this);
}

KPlayerNode* KPlayerDiskNode::createLeaf (const QString& id)
{
  if ( disk() && disk() -> type() == "Video CD" && disk() -> msf (id) == 6 )
    return 0;
  KPlayerNode* node = new KPlayerTrackNode;
  node -> setup (this, id);
  return node;
}

QString KPlayerDiskNode::icon (void) const
{
  const QString& type (media() -> type());
  return type == "DVD" ? "media-optical" : type == "Audio CD" ? "media-optical-audio" : "media-optical";
  // : type.startsWith ("Data") ? "cd-data" : "cd";
}

bool KPlayerDiskNode::ready (void) const
{
  return m_url.isNull() && (KPlayerEngine::engine() -> process() -> state() == KPlayerProcess::Idle
    || KPlayerEngine::engine() -> properties() -> parent() != disk());
}

QString KPlayerDiskNode::suggestId (void) const
{
  return media() -> url().fileName();
}

void KPlayerDiskNode::setDiskType (const QString& type)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::setDiskType\n";
  kdDebugTime() << " Type   " << type << "\n";
#endif
  disk() -> setType (type);
  disk() -> setDefaultName (i18n("%1 in %2", i18n(type.toUtf8()), device() -> name()));
}

bool KPlayerDiskNode::diskDevice (void)
{
  return true;
}

bool KPlayerDiskNode::mediaDisk (void)
{
  return disk() && disk() -> hasType() && ! disk() -> type().startsWith ("Data ");
}

bool KPlayerDiskNode::dataDisk (void)
{
  return disk() && disk() -> type().startsWith ("Data ");
}

static QRegExp re_track_length ("^ID_(?:DVD_TITLE|VCD_TRACK|CDDA_TRACK)_(\\d+)_(?:LENGTH|MSF)=([0-9.:]+)$");

bool KPlayerDiskNode::accessDisk (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::accessDisk\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
  kdDebugTime() << " Path   " << id() << "\n";
#endif
  QFile file (id());
  if ( file.open (QIODevice::ReadOnly) )
  {
    char data [65536];
    int length = file.read (data, sizeof (data));
    file.close();
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Length " << length << "\n";
#endif
    if ( length > 0 )
    {
      KMD5 digest (data, length);
      diskDetected (digest.hexDigest());
      return true;
    }
  }
  return false;
}

void KPlayerDiskNode::deviceAdded (const QString& udi)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::deviceAdded\n";
  kdDebugTime() << " Device " << udi << "\n";
#endif
  checkDisk();
}

void KPlayerDiskNode::deviceRemoved (const QString& udi)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::deviceRemoved\n";
  kdDebugTime() << " Device " << udi << "\n";
#endif
  if ( udi == solidDisk().udi() )
    diskRemoved();
}

void KPlayerDiskNode::checkDisk (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::checkDisk\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  foreach ( const Solid::Device& device, Solid::Device::listFromType (Solid::DeviceInterface::OpticalDisc) )
    if ( const Solid::OpticalDisc* disc = device.as<Solid::OpticalDisc>() )
      if ( const Solid::Block* block = device.as<Solid::Block>() )
      {
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " Disc   " << disc -> discType() << "\n";
        kdDebugTime() << " Content " << disc -> availableContent() << "\n";
        kdDebugTime() << " Blank  " << disc -> isBlank() << "\n";
#endif
        QString path = block -> device(), udi = device.udi();
        if ( path == id() && udi != solidDisk().udi() && ! disc -> isBlank() )
        {
          const char* type;
          if ( disc -> discType() >= Solid::OpticalDisc::DvdRom )
            if ( (disc -> availableContent() & Solid::OpticalDisc::VideoDvd) == Solid::OpticalDisc::VideoDvd )
              type = "DVD";
            else
              type = I18N_NOOP("Data DVD");
          else
            if ( (disc -> availableContent() & Solid::OpticalDisc::VideoCd) == Solid::OpticalDisc::VideoCd
                || (disc -> availableContent() & Solid::OpticalDisc::SuperVideoCd) == Solid::OpticalDisc::SuperVideoCd )
              type = I18N_NOOP("Video CD");
            else if ( (disc -> availableContent() & Solid::OpticalDisc::Audio) == Solid::OpticalDisc::Audio )
              type = I18N_NOOP("Audio CD");
            else
              type = I18N_NOOP("Data CD");
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Type   " << type << "\n";
#endif
          if ( const Solid::StorageAccess* access = device.as<Solid::StorageAccess>() )
          {
            path = access -> filePath();
#ifdef DEBUG_KPLAYER_NODE
            kdDebugTime() << " Mounted " << access -> isAccessible() << "\n";
            if ( ! path.isEmpty() )
              kdDebugTime() << " Path   " << path.toLatin1().constData() << "\n";
#endif
          }
          diskInserted (type, path, udi);
        }
      }
}

void KPlayerDiskNode::diskDetected (const QString& diskid)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::diskDetected\n";
  kdDebugTime() << " ID     " << diskid << "\n";
#endif
  if ( diskid == suggestId() )
    return;
  KPlayerContainerNode::removed (nodes());
  KPlayerGenericProperties* previous = media();
  previous -> disconnect (this);
  m_media = m_disk = KPlayerMedia::diskProperties (m_device, "kplayer:/disks/" + diskid);
  connect (media(), SIGNAL (updated()), SLOT (updated()));
  if ( ! disk() -> hasType() )
    disk() -> setDefaultName (i18n("Disk in %1", device() -> name()));
  if ( previous != device() && previous -> url().url().indexOf ('/', 15) >= 0
      && previous -> name() != previous -> defaultName() && media() -> name() == media() -> defaultName() )
    media() -> setName (previous -> name());
  media() -> diff (previous);
  media() -> commit();
  if ( previous != device() )
    KPlayerMedia::release (previous);
}

void KPlayerDiskNode::diskInserted (const char* type, const QString& path, const QString& udi)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::diskInserted\n";
#endif
  m_fast_autodetect = true;
  m_local_path = path;
  m_solid_disk = Solid::Device (udi);
  if ( Solid::StorageAccess* access = solidDevice().as<Solid::StorageAccess>() )
    connect (access, SIGNAL (setupDone (Solid::ErrorType error, QVariant errorData, const QString &udi)),
      SLOT (mountDone (Solid::ErrorType error, QVariant errorData, const QString &udi)));
  if ( disk() )
  {
    if ( disk() -> type() != type )
    {
      KPlayerContainerNode::removed (nodes());
      KPlayerMedia* previous = media();
      previous -> disconnect (this);
      QString urls ("kplayer:/disks" + id());
      KPlayerEngine::engine() -> meta() -> deleteGroup (urls);
      m_media = m_disk = KPlayerMedia::diskProperties (m_device, urls);
      connect (media(), SIGNAL (updated()), SLOT (updated()));
      setDiskType (type);
      media() -> diff (previous);
      if ( previous != device() )
        KPlayerMedia::release (previous);
    }
    else
      disk() -> commit();
  }
  else
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Type   " << type << "\n";
#endif
    media() -> disconnect (this);
    QString urls ("kplayer:/disks" + id());
    KPlayerEngine::engine() -> meta() -> deleteGroup (urls);
    m_media = m_disk = KPlayerMedia::diskProperties (device(), urls);
    connect (media(), SIGNAL (updated()), SLOT (updated()));
    setDiskType (type);
    media() -> diff (device());
  }
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::diskInserted done\n";
  kdDebugTime() << " Populated " << populated() << "\n";
  kdDebugTime() << " Ready  " << ready() << "\n";
  kdDebugTime() << " Media disk " << mediaDisk() << "\n";
#endif
  if ( populated() && ready() && mediaDisk() && ! disk() -> hasTracks() )
    autodetect();
}

void KPlayerDiskNode::diskRemoved (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::diskRemoved\n";
#endif
  m_fast_autodetect = false;
  m_local_path = QString::null;
  m_url = QString::null;
  m_solid_disk = Solid::Device();
  if ( disk() )
  {
    KPlayerContainerNode::removed (nodes());
    media() -> disconnect (this);
    KPlayerDiskProperties* d = disk();
    m_disk = 0;
    m_media = device();
    connect (media(), SIGNAL (updated()), SLOT (updated()));
    media() -> diff (d);
    KPlayerMedia::release (d);
  }
  else
    media() -> commit();
}

void KPlayerDiskNode::getLocalPath (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::getLocalPath\n";
#endif
  if ( hasLocalPath() || ! dataDisk() || ! ready() )
    return;
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " Valid  " << solidDevice().isValid() << "\n";
#endif
  if ( ! solidDevice().isValid() )
    return;
  if ( Solid::StorageAccess* access = solidDevice().as<Solid::StorageAccess>() )
  {
    m_local_path = access -> filePath();
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Mounted " << access -> isAccessible() << "\n";
    if ( hasLocalPath() )
      kdDebugTime() << " Path   " << localPath().toLatin1().constData() << "\n";
#endif
    if ( ! hasLocalPath() )
    {
      m_url = "path://";
      access -> setup();
    }
  }
}

void KPlayerDiskNode::mountDone (Solid::ErrorType error, QVariant errorData, const QString &udi)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::mountDone\n";
  kdDebugTime() << " UDI    " << udi << "\n";
#endif
  m_url = QString::null;
  if ( error != Solid::NoError )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Error  " << error << ": " << errorData.toString() << "\n";
#endif
    if ( disk() )
      disk() -> commit();
  }
  else if ( Solid::StorageAccess* access = solidDevice().as<Solid::StorageAccess>() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Mounted " << access -> isAccessible() << "\n";
#endif
    if ( ! access -> filePath().isEmpty() )
    {
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Path   " << access -> filePath().toLatin1().constData() << "\n";
#endif
      m_local_path = access -> filePath();
      disk() -> commit();
    }
  }
}

int KPlayerDiskNode::tracks (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::tracks\n";
#endif
  int n = disk() ? disk() -> tracks() : 0;
  if ( n == 0 && ready() && ! dataDisk() )
  {
    autodetect();
    if ( disk() )
      n = disk() -> tracks();
  }
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " Tracks " << n << "\n";
#endif
  return n;
}

void KPlayerDiskNode::loadDisk (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::loadDisk\n";
#endif
  if ( ready() && disk() && disk() -> hasTracks() )
    diskRemoved();
  if ( ready() )
    autodetect();
}

void KPlayerDiskNode::autodetect (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::autodetect\n";
  kdDebugTime() << " Fast   " << m_fast_autodetect << "\n";
  if ( disk() && disk() -> hasType() )
    kdDebugTime() << " Type   " << disk() -> type() << "\n";
#endif
  m_autodetected = false;
  m_detected_tracks = 0;
  m_track_lengths.clear();
  m_disk_id = QString::null;
  m_local_path = QString::null;
  if ( ! disk() || ! disk() -> hasType() )
    m_fast_autodetect = false;
  if ( ( ! m_fast_autodetect && ready() ) || ( m_fast_autodetect && disk() -> type() == "DVD" && m_url != "dvd://" ) )
    m_url = "dvd://";
  else if ( ( ! m_fast_autodetect && m_url == "dvd://" )
      || ( m_fast_autodetect && disk() -> type() == "Audio CD" && m_url != "cdda://" ) )
    m_url = "cdda://";
  else if ( ( ! m_fast_autodetect && m_url == "cdda://" ) || ( m_fast_autodetect && dataDisk() && m_url != "data://" ) )
  {
    if ( dataDisk() )
    {
      m_url = QString::null;
      if ( Solid::StorageAccess* access = solidDevice().as<Solid::StorageAccess>() )
      {
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " Mounted " << access -> isAccessible() << "\n";
#endif
        if ( ! access -> filePath().isEmpty() )
        {
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Path   " << access -> filePath().toLatin1().constData() << "\n";
#endif
          m_local_path = access -> filePath();
        }
      }
      disk() -> commit();
    }
    else
    {
      m_url = "data://";
      autodetect();
    }
    return;
  }
  else if ( ( ! m_fast_autodetect && m_url == "data://" )
      || ( m_fast_autodetect && disk() -> type() == "Video CD" && m_url != "vcd://" ) )
    m_url = "vcd://";
  else
  {
    diskRemoved();
    return;
  }
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " URL    " << m_url << "\n";
#endif
  KPlayerLineOutputProcess* process = new KPlayerLineOutputProcess;
  *process << media() -> executablePath() << (m_url == "dvd://" ? "-dvd-device" : "-cdrom-device") << id()
    << "-msglevel" << "identify=6" << "-ao" << "null" << "-vo" << "null" << "-frames" << "0";
  if ( m_url == "vcd://" )
    *process << "vcd://1" << "vcd://2";
  else
    *process << m_url;
  connect (process, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*)),
    SLOT (receivedOutput (KPlayerLineOutputProcess*, char*)));
  connect (process, SIGNAL (processFinished (KPlayerLineOutputProcess*)),
    SLOT (processFinished (KPlayerLineOutputProcess*)));
  process -> start();
}

void KPlayerDiskNode::receivedOutput (KPlayerLineOutputProcess*, char* str)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " <<<<<< " << str << "\n";
#endif
  static QRegExp re_tracks ("^ID_(?:DVD_TITLES|VCD_END_TRACK|CDDA_TRACKS)=(\\d+)$");
  static QRegExp re_msf ("^(\\d+):(\\d+):(\\d+)$");
  static QRegExp re_filename ("^ID_FILENAME=");
  if ( m_detected_tracks == 0 && re_tracks.indexIn (str) >= 0 )
    m_detected_tracks = re_tracks.cap(1).toUInt();
  else if ( m_detected_tracks && re_filename.indexIn (str) >= 0 )
    m_autodetected = true;
  else if ( m_track_lengths.count() < m_detected_tracks && re_track_length.indexIn (str) >= 0 )
  {
    int track = re_track_length.cap(1).toInt() - 1;
    if ( track == m_track_lengths.count() )
    {
      QString length (re_track_length.cap(2));
      if ( m_url == "cdda://" )
        m_disk_id += length;
      if ( m_url == "dvd://" )
        m_track_lengths.append (length.toFloat());
      else if ( re_msf.indexIn (length) >= 0 )
        m_track_lengths.append (re_msf.cap(1).toInt() * 60 + re_msf.cap(2).toInt() + re_msf.cap(3).toFloat() / 75);
    }
  }
}

void KPlayerDiskNode::processFinished (KPlayerLineOutputProcess* process)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::processFinished\n";
#endif
  delete process;
  if ( ready() )
    return;
  if ( m_autodetected && m_url == "cdda://" )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Length " << m_disk_id.length() << "\n";
#endif
    KMD5 digest (m_disk_id.toLatin1());
    diskDetected (digest.hexDigest());
    autodetected();
    m_artist = m_album = m_genre = m_cddb_id = QString::null;
    m_year = 0;
    KPlayerLineOutputProcess* process = new KPlayerLineOutputProcess;
    *process << media() -> executablePath() << "-cdrom-device" << id() << "-v"
      << "-identify" << "-ao" << "null" << "-vo" << "null" << "-frames" << "0" << "cddb://";
    connect (process, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*)),
      SLOT (receivedCddbOutput (KPlayerLineOutputProcess*, char*)));
    connect (process, SIGNAL (processFinished (KPlayerLineOutputProcess*)),
      SLOT (cddbProcessFinished (KPlayerLineOutputProcess*)));
    process -> start();
    return;
  }
  if ( m_autodetected || ( m_url == "cdda://" && ! m_fast_autodetect ) )
  {
    if ( ! accessDisk() )
    {
      diskRemoved();
      return;
    }
    else if ( dataDisk() || ( mediaDisk() && disk() -> hasTracks() ) )
    {
      setDiskType (disk() -> type());
      updateTracks();
      m_url = QString::null;
      disk() -> commit();
      return;
    }
  }
  if ( m_autodetected && disk() )
    autodetected();
  else if ( m_url != "vcd://" || ( m_fast_autodetect && disk() && disk() -> hasType() ) )
    autodetect();
  else
  {
    m_url = QString::null;
    if ( disk() )
    {
      setDiskType (I18N_NOOP("Data Disk"));
      disk() -> commit();
    }
  }
}

void KPlayerDiskNode::autodetected (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::autodetected\n";
#endif
  QString type (m_url == "cdda://" ? "Audio CD" : m_url == "dvd://" ? "DVD"
    : m_url == "vcd://" ? "Video CD" : disk() -> type());
  setDiskType (type);
  if ( m_detected_tracks )
    disk() -> setTracks (m_detected_tracks);
  updateTracks();
  m_url = QString::null;
  disk() -> commit();
}

void KPlayerDiskNode::updateTracks (void)
{
  if ( ! mediaDisk() )
    return;
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::updateTracks\n";
  kdDebugTime() << " Tracks " << disk() -> tracks() << "\n";
#endif
  if ( int (nodes().count()) != disk() -> tracks() )
  {
    KPlayerContainerNode::removed (nodes());
    QString key (disk() -> type() == "Video CD" ? "MSF" : "Length");
    QStringList tracklist;
    for ( int track = 1; track <= disk() -> tracks(); ++ track )
    {
      tracklist.append (QString::number (track));
      if ( track <= int (m_track_lengths.count()) )
      {
        KUrl url (disk() -> url());
        url.addPath (QString::number (track));
        KPlayerTrackProperties* media = KPlayerMedia::trackProperties (url);
        media -> setupInfo();
        if ( ! media -> has (key) )
        {
          media -> setFloat (key, m_track_lengths [track - 1]);
          media -> commit();
        }
        KPlayerMedia::release (media);
      }
    }
    addedLeaves (tracklist);
  }
}

void KPlayerDiskNode::receivedCddbOutput (KPlayerLineOutputProcess*, char* str)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " <<<<<< " << str << "\n";
#endif
  if ( ! disk() )
    return;
  if ( re_track_length.indexIn (str) >= 0 )
    m_cddb_id += re_track_length.cap(2);
  else if ( m_cddb_id == m_disk_id )
  {
    QString line (QString::fromLocal8Bit (str));
    if ( line.startsWith ("DTITLE=") )
    {
      line.remove (0, 7);
      m_artist = line.section ('/', 0, 0).simplified();
      m_album = line.section ('/', 1).simplified();
      if ( ! m_artist.isEmpty() || ! m_album.isEmpty() )
      {
        disk() -> setName (m_album.isEmpty() ? m_artist : m_artist.isEmpty() ? m_album : m_album + " - " + m_artist);
        disk() -> commit();
      }
    }
    else if ( line.startsWith ("DYEAR=") )
    {
      line.remove (0, 6);
      m_year = line.toInt();
      if ( m_year > 10 && m_year < 100 )
        m_year += 1900;
    }
    else if ( line.startsWith ("DGENRE=") )
    {
      line.remove (0, 7);
      m_genre = line.simplified();
    }
    else if ( line.startsWith ("TTITLE") )
    {
      line.remove (0, 6);
      bool ok;
      int track = line.section ('=', 0, 0).toInt (&ok) + 1;
      if ( ok )
      {
        KUrl url (disk() -> url());
        url.addPath (QString::number (track));
        KPlayerTrackProperties* media = KPlayerMedia::trackProperties (url);
        QString title (line.section ('=', 1).simplified());
        if ( ! title.isEmpty() )
          media -> setName (title);
        if ( ! m_artist.isEmpty() )
          media -> setString ("Artist", m_artist);
        if ( ! m_album.isEmpty() )
          media -> setString ("Album", m_album);
        if ( m_year )
          media -> setString ("Year", QString::number (m_year));
        if ( ! m_genre.isEmpty() )
          media -> setString ("Genre", m_genre);
        media -> commit();
        KPlayerMedia::release (media);
      }
    }
  }
}

void KPlayerDiskNode::cddbProcessFinished (KPlayerLineOutputProcess* process)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::cddbProcessExited\n";
#endif
  delete process;
}

KPlayerTunerNode::~KPlayerTunerNode()
{
}

void KPlayerTunerNode::setupSource (void)
{
  m_solid_device = Solid::Device (parent() -> deviceUdi (id()));
  m_channel_list = media() -> channelList();
  m_source = new KPlayerTunerSource (this);
}

KPlayerNode* KPlayerTunerNode::createLeaf (const QString& id)
{
  KPlayerChannelNode* node = new KPlayerChannelNode;
  node -> setup (this, id);
  return node;
}

void KPlayerTunerNode::updated (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerTunerNode::updated\n";
#endif
  if ( m_channel_list != media() -> channelList() )
  {
    m_channel_list = media() -> channelList();
    refreshNodes();
  }
}

KPlayerTVNode::~KPlayerTVNode()
{
}

void KPlayerTVNode::setupMedia (void)
{
  m_media = KPlayerMedia::tvProperties (metaurl());
  if ( ! media() -> hasType() )
    media() -> setType (parent() -> deviceType (id()));
  media() -> setDefaultName (parent() -> deviceName (id()));
  connect (media(), SIGNAL (updated()), SLOT (updated()));
}

KPlayerDVBNode::~KPlayerDVBNode()
{
}

void KPlayerDVBNode::setupMedia (void)
{
  m_media = KPlayerMedia::dvbProperties (metaurl());
  if ( ! media() -> hasType() )
    media() -> setType (parent() -> deviceType (id()));
  media() -> setDefaultName (parent() -> deviceName (id()));
  connect (media(), SIGNAL (updated()), SLOT (updated()));
}

#if 0
KPlayerSearchesNode::~KPlayerSearchesNode()
{
}

KPlayerContainerNode* KPlayerSearchesNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  KPlayerContainerNode* node = new KPlayerSearchNode;
  node -> setup (this, id, origin);
  return node;
}

bool KPlayerSearchesNode::canRename (void) const
{
  return false;
}

KPlayerSearchNode::~KPlayerSearchNode()
{
}

void KPlayerSearchNode::setupSource (void)
{
  m_source = new KPlayerSearchSource (this);
}

KPlayerNode* KPlayerSearchNode::createLeaf (const QString& id)
{
  KPlayerNode* node = new KPlayerSearchItemNode;
  node -> setup (this, id);
  return node;
}

bool KPlayerSearchNode::allowsCustomOrder (void) const
{
  return false;
}
#endif

KPlayerNodeList::~KPlayerNodeList()
{
}

bool compareNodes (KPlayerNode* left, KPlayerNode* right)
{
  return left -> compare (right) < 0;
}

void KPlayerNodeList::sort (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNodeList::sort\n";
#endif
  //qSort (constBegin(), constEnd(), compareNodes);
}

void KPlayerNodeList::releaseAll (void) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNodeList::releaseAll\n";
#endif
  KPlayerNodeList::ConstIterator iterator (constBegin());
  while ( iterator != constEnd() )
  {
    (*iterator) -> release();
    ++ iterator;
  }
}

KPlayerNodeList KPlayerNodeList::fromUrlList (const KUrl::List& urls)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNodeList::fromUrlList\n";
#endif
  KPlayerNodeList list;
  KUrl::List::ConstIterator iterator (urls.constBegin());
  while ( iterator != urls.constEnd() )
  {
    KUrl url (*iterator);
    if ( url.path().isEmpty() && url.host().isEmpty() && url.protocol() == "file" )
      continue;
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " URL    " << url.url() << "\n";
    kdDebugTime() << " Path   " << url.path() << "\n";
#endif
    if ( url.protocol() == "rtspt" || url.protocol() == "rtspu" )
      url.setProtocol ("rtsp");
    bool group = false, media = url.protocol() == "media";
    KPlayerNode* node = 0;
    if ( media || url.protocol() == "system" )
    {
      if ( media || url.url().startsWith ("system:/media/") )
        node = KPlayerNode::root() -> getNodeByUrl ("kplayer:/devices/dev" + (media ? url.path() : url.path().mid(6)));
      if ( ! node )
      {
        KIO::UDSEntry entry;
        if ( KIO::NetAccess::stat (url, entry, 0) )
        {
          KFileItem item (entry, url);
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Type   " << item.mimetype() << "\n";
#endif
          QString path (item.entry().stringValue (KIO::UDSEntry::UDS_LOCAL_PATH));
          if ( ! path.isEmpty() )
          {
            url = path;
#ifdef DEBUG_KPLAYER_NODE
            kdDebugTime() << " Local  " << path << "\n";
#endif
          }
        }
#ifdef DEBUG_KPLAYER_NODE
        else
          kdDebugTime() << " Error  " << KIO::NetAccess::lastErrorString() << "\n";
#endif
      }
    }
    if ( url.isLocalFile() )
    {
      QFileInfo info (url.path());
      group = info.isDir();
      if ( group )
        node = KPlayerNode::root() -> getNodeByUrl (url);
    }
    if ( node )
      node -> reference();
    else
      node = KPlayerNode::root() -> temporaryNode() -> temporaryItem (url.url());
    if ( node )
      list.append (node);
    ++ iterator;
  }
  return list;
}

KPlayerNodeListByName::~KPlayerNodeListByName()
{
}

bool compareNodesByName (KPlayerNode* left, KPlayerNode* right)
{
  return left -> compareByName (right) < 0;
}

void KPlayerNodeListByName::sort (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNodeListByName::sort\n";
#endif
  //qSort (constBegin(), constEnd(), compareNodesByName);
}

KPlayerPlaylistNodeList::~KPlayerPlaylistNodeList()
{
}

bool compareNodesByPosition (KPlayerNode* left, KPlayerNode* right)
{
  while ( left -> parent() )
  {
    KPlayerNode* node = right;
    while ( node -> parent() )
    {
      if ( left == node -> parent() )
        return true;
      if ( node == left -> parent() )
        return false;
      if ( left -> parent() == node -> parent() )
        return left -> compare (node) < 0;
      node = node -> parent();
    }
    left = left -> parent();
  }
  return false;
}

void KPlayerPlaylistNodeList::sort (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNodeList::sort\n";
#endif
  //qSort (constBegin(), constEnd(), compareNodesByPosition);
}
