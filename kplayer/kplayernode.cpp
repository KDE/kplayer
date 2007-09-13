/***************************************************************************
                          kplayernode.cpp
                          ---------------
    begin                : Wed Feb 16 2005
    copyright            : (C) 2005-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kde_file.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kio/netaccess.h>
#include <qregexp.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_NODE
#endif

#include "kplayernode.h"
#include "kplayernode.moc"
#include "kplayerprocess.h"
#include "kplayersettings.h"
#include "kplayersource.h"

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
    return parent() -> compareByPosition (this, node);
  if ( isContainer() != node -> isContainer() )
    return isContainer() == parent() -> groupsFirst() ? -1 : 1;
  int result = media() -> compare (node -> media(), sortKey());
  if ( result == 0 && ! sortByName() )
    result = compareStrings (name(), node -> name());
  return sortAscending() ? result : - result;
}

int KPlayerNode::compareByName (KPlayerNode* node) const
{
  return parent() -> customOrder() ? parent() -> compareByPosition (this, node)
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
  kdDebugTime() << " URL    " << url() << "\n";
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
  kdDebugTime() << " ID     " << url() << "\n";
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
    KPlayerNodeList nodes (node -> parent() -> nodes());
    nodes.findRef (node);
    if ( nodes.prev() )
      return nodes.current() -> lastMediaNode();
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
  return origin() ? origin() -> icon() : "folder_open";
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
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
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
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
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
  kdDebugTime() << " Node   " << node -> url() << "\n";
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
  kdDebugTime() << " URL    " << node -> url() << "\n";
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
  KPlayerContainerNode* parent = nodes.getFirst() -> parent();
  KPlayerNodeList list;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
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
  if ( after == this )
    m_nodes.prepend (node);
  else if ( after && m_nodes.findRef (after) >= 0 )
    m_nodes.insert (nodes().at() + 1, node);
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
      KPlayerNodeMap::ConstIterator iterator = map.find (id);
      if ( iterator != map.end() )
      {
        node = *iterator;
        previous.removeRef (node);
        iterator = map.end();
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
    KPlayerNodeListIterator iterator (previous);
    while ( KPlayerNode* node = iterator.current() )
    {
      append (node);
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
    if ( populated() )
    {
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Adding references\n";
#endif
      if ( origin() )
        origin() -> populateGroups();
      KPlayerNodeListIterator iterator (nodes());
      while ( KPlayerNode* node = iterator.current() )
      {
        if ( node -> isContainer() )
          node -> reference();
        ++ iterator;
      }
    }
    else
      doPopulateGroups();
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
    KPlayerNodeMap::ConstIterator iterator = map.find (id);
    KPlayerNode* node = iterator == map.end() ? 0 : *iterator;
    if ( node )
    {
      previous.removeRef (node);
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
    KPlayerNodeListIterator iterator (previous);
    while ( KPlayerNode* node = iterator.current() )
    {
      append (node);
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
  for ( KPlayerNode* node = m_nodes.first(); node; node = m_nodes.next() )
    if ( node -> isContainer() )
      ((KPlayerContainerNode*) node) -> populateAll();
}

void KPlayerContainerNode::vacate (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::vacate\n";
  kdDebugTime() << " Count  " << m_populate_nodes << "\n";
  kdDebugTime() << " ID     " << url() << "\n";
#endif
  if ( m_populate_nodes == 1 )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << "Vacating node\n";
    kdDebugTime() << " ID     " << url() << "\n";
#endif
    KPlayerNodeList nodes (m_nodes);
    for ( KPlayerNode* node = nodes.first(); node; node = nodes.next() )
      node -> release();
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
  kdDebugTime() << " ID     " << url() << "\n";
#endif
  if ( m_populate_groups == 1 )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << "Vacating groups\n";
    kdDebugTime() << " URL    " << url() << "\n";
#endif
    KPlayerNodeList nodes (m_nodes);
    for ( KPlayerNode* node = nodes.first(); node; node = nodes.next() )
      if ( node -> isContainer() )
        node -> release();
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
  for ( KPlayerNode* node = m_nodes.first(); node; node = m_nodes.next() )
    if ( node -> isContainer() )
      ((KPlayerContainerNode*) node) -> vacateAll();
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
  kdDebugTime() << "KPlayerContainerNode::nodeById '" << id << "'\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeMap::ConstIterator iterator = m_node_map.find (id);
  return iterator == m_node_map.end() ? 0 : *iterator;
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
  return m_nodes.findRef (node) >= 0 ? m_nodes.next() : 0;
}

KPlayerMediaNode* KPlayerContainerNode::lastMediaNode (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::lastMediaNode\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeListIterator iterator (m_nodes);
  iterator.toLast();
  while ( KPlayerNode* node = iterator.current() )
  {
    KPlayerMediaNode* medianode = node -> lastMediaNode();
    if ( medianode )
    {
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Found  " << medianode -> url().url() << "\n";
#endif
      return medianode;
    }
    -- iterator;
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
    if ( custom || origin() && origin() -> customOrder() )
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

int KPlayerContainerNode::compareByPosition (const KPlayerNode* node1, const KPlayerNode* node2)
{
  int i1 = m_nodes.findRef (node1);
  int i2 = m_nodes.findRef (node2);
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
    QStringList::ConstIterator iterator (list.begin());
    while ( iterator != list.end() )
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
    QStringList::ConstIterator iterator (list.begin());
    while ( iterator != list.end() )
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

KPlayerNode* KPlayerContainerNode::added (const KPlayerNodeList& nodes, bool link, KPlayerNode* after)
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
    kdDebugTime() << " Key    '" << sortKey() << "'\n";
#endif
    m_nodes.sort();
    setCustomOrder (true);
  }
  KPlayerNodeList list;
  KPlayerPropertyCounts counts;
  KPlayerNode* saveAfter = after;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    QString id (node -> suggestId());
    KPlayerNode* subnode = nodeById (id);
    if ( subnode && acceptsDuplicates() )
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
    if ( subnode )
    {
      if ( after || customOrder() )
      {
        m_nodes.removeRef (subnode);
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
        list.append (subnode);
      }
    }
    if ( after )
      after = subnode;
    ++ iterator;
  }
  if ( origin() && customOrder() && ! media() -> customOrder() )
  {
    m_nodes.clear();
    KPlayerNodeListIterator originit (origin() -> nodes());
    while ( KPlayerNode* originnode = originit.current() )
    {
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
  emitAdded (list, saveAfter);
  source() -> save();
  vacate();
  return after && m_nodes.findRef (after) >= 0 ? after : 0;
}

KPlayerNode* KPlayerContainerNode::moved (const KPlayerNodeList& nodes, KPlayerNode* after)
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
    kdDebugTime() << " Key    '" << sortKey() << "'\n";
#endif
    m_nodes.sort();
    setCustomOrder (true);
  }
  KPlayerNode* saveAfter = after;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( after || customOrder() )
    {
      m_nodes.removeRef (node);
      insert (node, after);
    }
    if ( after )
      after = node;
    ++ iterator;
  }
  source() -> save();
  KPlayerNodeList list;
  emitAdded (list, saveAfter);
  vacate();
  return after && m_nodes.findRef (after) >= 0 ? after : 0;
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
  return nodes().getLast();
}

bool KPlayerContainerNode::remove (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerContainerNode::remove\n";
  kdDebugTime() << " URL    " << node -> url() << "\n";
#endif
  bool found = m_nodes.removeRef (node);
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
    for ( QStringList::ConstIterator iterator (ids.begin()); iterator != ids.end(); ++ iterator )
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
    KPlayerNodeListIterator iterator (list);
    while ( KPlayerNode* node = iterator.current() )
    {
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
  kdDebugTime() << " URL    " << url() << "\n";
#endif
  if ( ! origin() || media() -> customOrder() )
  {
    populate();
    QStringList children;
    KPlayerNodeListIterator iterator (nodes());
    while ( KPlayerNode* node = iterator.current() )
    {
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Child  " << node -> url() << "\n";
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
  KPlayerNode* node;
  KPlayerNodeList list (m_nodes);
  m_nodes.clear();
  const QStringList& children (media() -> children());
  QStringList::ConstIterator iterator (children.begin());
  while ( iterator != children.end() )
  {
    QString id (*iterator);
    for ( node = list.first(); node; node = list.next() )
      if ( node -> id() == id )
        break;
    if ( node )
    {
      list.remove();
      m_nodes.append (node);
    }
    ++ iterator;
  }
  for ( node = list.first(); node; node = list.next() )
    m_nodes.append (node);
}

void KPlayerContainerNode::detach (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "Detaching subnodes\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  KPlayerNodeList nodes (m_nodes);
  for ( KPlayerNode* node = nodes.first(); node; node = nodes.next() )
    node -> detach();
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
  KPlayerNodeListIterator iterator (nodes());
  while ( KPlayerNode* node = iterator.current() )
  {
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
  KPlayerContainerNodeMap::ConstIterator iterator = m_defaults.find (id);
  if ( iterator == m_defaults.end() )
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
    KPlayerContainerNodeMap::ConstIterator iterator (m_externals.begin());
    while ( iterator != m_externals.end() )
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
  return origin() ? origin() -> icon() : "folder_green";
}

QString KPlayerGroupNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder_green_open";
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
  kdDebugTime() << " Node   " << node -> url() << "\n";
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
    kdDebugTime() << " Key    '" << key << "'\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  if ( groupingKey() == key || key.isNull() && parent() && isParentGrouped() )
    return;
  media() -> setGroupingKey (key);
  KPlayerNodeList nodes (m_nodes);
  for ( KPlayerNode* node = nodes.first(); node; node = nodes.next() )
    node -> detach();
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
    QString group ("Playlist Entries");
    KConfig* meta = KPlayerEngine::engine() -> meta();
    meta -> setGroup (group);
    int entries = meta -> readNumEntry ("Entries");
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
        KConfig* config = media() -> config();
        QString urls (url (name).url());
        config -> setGroup (urls);
        for ( int i = 0; i < entries; i ++ )
        {
          QString no (QString::number (i));
          config -> writeEntry ("Child" + no, meta -> readEntry ("Entry " + no, QString()));
        }
        config -> writeEntry ("Children", entries);
        np -> media() -> setChildren (children);
        np -> media() -> commit();
        config -> setGroup (np -> url (name).url());
        config -> writeEntry ("Origin", urls);
      }
    }
    meta -> deleteGroup (group);
  }
}

void KPlayerPlaylistNode::setupOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNode::setupOrigin\n";
  if ( origin() )
    kdDebugTime() << " Origin " << origin() -> url() << "\n";
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
  return origin() ? origin() -> icon() : "folder_violet";
}

QString KPlayerPlaylistNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder_violet_open";
}

bool KPlayerPlaylistNode::canLink (KPlayerContainerNode* node) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerPlaylistNode::canLink\n";
  kdDebugTime() << " Node   " << node -> url() << "\n";
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
      KPlayerNodeListIterator iterator (nodes());
      while ( KPlayerNode* node = iterator.current() )
      {
        QString id (node -> id().section ('#', 0, 0));
        KPlayerNodeMap::ConstIterator iterator = map.find (id);
        if ( iterator == map.end() )
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
  kdDebugTime() << " Node   " << node -> url() << "\n";
#endif
  return node -> isCollection() || node -> isDirectory() || node -> hasProperties()
    || node -> isPlaylist() && ! node -> isRecent() && ! node -> isNowPlaying();
}

bool KPlayerNowPlayingNode::isNowPlaying (void) const
{
  return true;
}

QString KPlayerNowPlayingNode::icon (void) const
{
  return origin() ? origin() -> icon() : "folder_red";
}

QString KPlayerNowPlayingNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder_red_open";
}

void KPlayerNowPlayingNode::setupOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNowPlayingNode::setupOrigin\n";
  if ( origin() )
    kdDebugTime() << " Origin " << origin() -> url() << "\n";
#endif
  KPlayerPlaylistNode::setupOrigin();
  if ( origin() && origin() -> hasProperties() && ((KPlayerDeviceNode*) origin()) -> diskDevice() )
  {
    KPlayerDiskNode* disk = (KPlayerDiskNode*) origin();
    if ( disk -> dataDisk() )
      if ( disk -> hasLocalPath() )
      {
        KPlayerContainerNode* origin = root() -> getNodeByUrl (KUrl::fromPathOrUrl (disk -> localPath()));
        if ( origin )
        {
          disconnect (m_origin -> parent(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
            this, SLOT (originUpdated (KPlayerContainerNode*, KPlayerNode*)));
          m_origin -> reference();
          m_origin -> release();
          setOrigin (origin);
        }
      }
      else
        disk -> getLocalPath();
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
      if ( disk -> hasLocalPath() )
      {
        KPlayerContainerNode* origin = root() -> getNodeByUrl (KUrl::fromPathOrUrl (disk -> localPath()));
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
      else
        disk -> getLocalPath();
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
  kdDebugTime() << " Node   " << node -> url() << "\n";
#endif
  return node -> isCollection() || node -> isDirectory() || node -> hasProperties()
    || node -> isPlaylist() && ! node -> isRecent() && ! node -> isNowPlaying();
}

KPlayerContainerNode* KPlayerRecentNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  KPlayerContainerNode* node = new KPlayerRecentNode;
  node -> setup (this, id, origin);
  return node;
}

QString KPlayerRecentNode::icon (void) const
{
  return origin() ? origin() -> icon() : "folder_red";
}

QString KPlayerRecentNode::openIcon (void) const
{
  return origin() ? origin() -> icon() : "folder_red_open";
}

KPlayerRecentsNode::~KPlayerRecentsNode()
{
}

void KPlayerRecentsNode::setupSource (void)
{
  KPlayerRecentNode::setupSource();
  if ( ! media() -> hasChildren() )
  {
    QString group ("Recent Files");
    KConfig* config = KPlayerEngine::engine() -> config();
    config -> setGroup (group);
    int limit = configuration() -> recentListSize();
    QStringList children;
    for ( int i = 1; i <= limit; i ++ )
    {
      QString name (config -> readEntry ("File" + QString::number (i), QString()));
      if ( name.isEmpty() )
        break;
      children.append (name);
    }
    if ( ! children.isEmpty() )
    {
      media() -> setChildren (children);
      setCustomOrder (true);
    }
    config -> deleteGroup (group);
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
    KPlayerNodeListIterator iterator (list);
    while ( KPlayerNode* node = iterator.current() )
    {
      if ( node -> parent() != this )
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
      KPlayerNode* listnode = list.getFirst();
      bool container = listnode -> isContainer();
      KPlayerNodeList previous;
      KPlayerNodeListIterator iterator (nodes());
      while ( KPlayerNode* node = iterator.current() )
      {
        if ( container && node -> isContainer() && ((KPlayerContainerNode*) node) -> origin() == listnode
            || ! container && ! node -> isContainer() && node -> media() == listnode -> media() )
          previous.append (node);
        ++ iterator;
      }
      if ( ! previous.isEmpty() )
        remove (previous);
      prepend (list);
      if ( container && ! listnode -> hasProperties() )
      {
        KPlayerNode* firstnode = nodes().getFirst();
        firstnode -> media() -> setName (i18n("%1 in %2").arg (listnode -> name(), listnode -> parent() -> name()));
        firstnode -> media() -> commit();
      }
    }
    else
    {
      QString name (list.count() == 2 ? i18n("%1 and %2").arg (list.getFirst() -> name(), list.getLast() -> name())
        : i18n("%1 and %2 more").arg (list.getFirst() -> name(), QString::number (list.count() - 1)));
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
    limit = nodes().count() - limit;
    m_nodes.last();
    KPlayerNodeList list;
    for ( int i = 0; i < limit; i ++ )
    {
      list.append (m_nodes.current());
      m_nodes.prev();
    }
    remove (list);
  }
  vacate();
}

KPlayerDevicesNode::~KPlayerDevicesNode()
{
}

void KPlayerDevicesNode::setupSource (void)
{
  m_complete = false;
  m_directory = "/dev";
  m_directory.setFilter (QDir::AllEntries | QDir::System);
  m_directory.setSorting (QDir::Name);
  m_watch.addDir (m_directory.path());
  m_watch.startScan();
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " Path   " << m_directory.path() << "\n";
  kdDebugTime() << " Method " << m_watch.internalMethod() << "\n";
#endif
  connect (&m_watch, SIGNAL (dirty (const QString&)), SLOT (dirty (const QString&)));
  connect (&m_lister, SIGNAL (completed()), SLOT (completed()));
  connect (&m_lister, SIGNAL (newItems (const KFileItemList&)), SLOT (refresh (const KFileItemList&)));
  connect (&m_lister, SIGNAL (deleteItem (KFileItem*)), SLOT (removed (KFileItem*)));
  QStringList mimetypes;
  mimetypes.append ("media/audiocd");
  mimetypes.append ("media/cdrom_mounted");
  mimetypes.append ("media/cdrom_unmounted");
  mimetypes.append ("media/cdwriter_mounted");
  mimetypes.append ("media/cdwriter_unmounted");
  mimetypes.append ("media/dvd_mounted");
  mimetypes.append ("media/dvd_unmounted");
  mimetypes.append ("media/dvdvideo");
  mimetypes.append ("media/svcd");
  mimetypes.append ("media/vcd");
  m_lister.setMimeFilter (mimetypes);
  m_lister.setAutoErrorHandlingEnabled (false, 0);
  m_lister.openUrl (KUrl ("media:/"));
  m_source = new KPlayerDevicesSource (this);
}

KPlayerContainerNode* KPlayerDevicesNode::createBranch (const QString& id, KPlayerContainerNode* origin)
{
  QMap<QString, QString>::ConstIterator iterator = m_type_map.find (id);
  QString type (iterator == m_type_map.end() ? media() -> type (id) : iterator.value());
  KPlayerDeviceNode* node = type == "TV" ? (KPlayerDeviceNode*) new KPlayerTVNode
    : type == "DVB" ? (KPlayerDeviceNode*) new KPlayerDVBNode : (KPlayerDeviceNode*) new KPlayerDiskNode;
  node -> setup (this, id, origin);
  return node;
}

QString KPlayerDevicesNode::icon (void) const
{
  return "system";
}

QString KPlayerDevicesNode::openIcon (void) const
{
  return "system";
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
  return getNodeById ("/" + path);
}

void KPlayerDevicesNode::dirty (const QString&)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::dirty\n";
#endif
  QStringList current, previous;
  update (current, previous);
  addedBranches (current);
  if ( ! previous.isEmpty() )
    KPlayerContainerNode::removed (previous);
}

void KPlayerDevicesNode::update (void)
{
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
  kdDebugTime() << " URL    " << url() << "\n";
#endif
  const char* paths[] = { "/dev", "/dev", "/dev", "/dev/dvb" };
  const char* globs[] = { "cdr*", "dvd*", "video*", "adapter*" };
  const char* types[] = { I18N_NOOP("CD"), I18N_NOOP("DVD"), I18N_NOOP("TV"), I18N_NOOP("DVB") };
  QMap<QString, int> maps [sizeof (paths) / sizeof (const char*)];
  previous = m_devices;
  for ( uint i = 0; i < sizeof (paths) / sizeof (const char*); i ++ )
  {
    QDir dir (paths[i], globs[i], QDir::Unsorted, QDir::AllEntries | QDir::System);
    QFileInfoList list (dir.entryInfoList());
    for ( QFileInfoList::ConstIterator filit (list.constBegin()); filit != list.constEnd(); ++ filit )
    {
      QFileInfo info (*filit);
      QString name (info.fileName());
      if ( info.exists() && info.isReadable() )
      {
        QString path (info.filePath());
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " Device " << path << "\n";
#endif
        for ( int l = 0; info.isSymLink() && l < 5; l ++ )
        {
          path = info.readLink();
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Link to " << path << "\n";
#endif
          path = dir.filePath (path);
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Absolute " << path << "\n";
#endif
          path = QDir::cleanPath (path);
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Cleaned " << path << "\n";
#endif
          info.setFile (path);
        }
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " ID     " << path << "\n";
#endif
        if ( ! media() -> hidden (path) )
        {
          QRegExp re_no ("(\\d+)$");
          int no = re_no.indexIn (name) >= 0 ? re_no.cap(1).toInt() : -1;
#ifdef DEBUG_KPLAYER_NODE
          kdDebugTime() << " Number " << no << "\n";
#endif
          if ( i == 2 )
          {
            KDE_struct_stat st;
            if ( KDE_lstat (path.toLatin1(), &st) != 0 )
              continue;
#ifdef DEBUG_KPLAYER_NODE
            kdDebugTime() << " Device " << st.st_dev << "\n";
            kdDebugTime() << " Major  " << major (st.st_dev) << "\n";
            kdDebugTime() << " Minor  " << minor (st.st_dev) << "\n";
            kdDebugTime() << " RDev   " << st.st_rdev << "\n";
            kdDebugTime() << " Major  " << major (st.st_rdev) << "\n";
            kdDebugTime() << " Minor  " << minor (st.st_rdev) << "\n";
#endif
            if ( major (st.st_rdev) != 81 )
              continue;
          }
          for ( uint j = 0; j < i; j ++ )
            maps[j].remove (path);
          maps[i].insert (path, no);
          if ( m_type_map.contains (path) )
          {
            previous.removeAll (path);
            m_type_map [path] = types[i];
#ifdef DEBUG_KPLAYER_NODE
            kdDebugTime() << " Found  " << path << " " << types[i] << "\n";
#endif
          }
          else
          {
            current.append (path);
            m_devices.append (path);
            m_type_map.insert (path, types[i]);
#ifdef DEBUG_KPLAYER_NODE
            kdDebugTime() << " Added  " << path << " " << types[i] << "\n";
#endif
          }
        }
      }
    }
  }
  QStringList::ConstIterator slit (previous.begin());
  while ( slit != previous.end() )
  {
    m_devices.removeAll (*slit);
    m_type_map.remove (*slit);
    ++ slit;
  }
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " CDs    " << maps[0].count() << "\n";
  kdDebugTime() << " DVDs   " << maps[1].count() << "\n";
  kdDebugTime() << " TVs    " << maps[2].count() << "\n";
  kdDebugTime() << " DVBs   " << maps[3].count() << "\n";
  kdDebugTime() << " Added  " << current.count() << "\n";
  kdDebugTime() << " Removed " << previous.count() << "\n";
  kdDebugTime() << " Devices " << m_devices.count() << "\n";
#endif
  m_name_map.clear();
  addToNameMap (maps[0], i18n("CD Device"), i18n("CD Device %1"));
  addToNameMap (maps[1], i18n("DVD Device"), i18n("DVD Device %1"));
  addToNameMap (maps[2], i18n("TV Device"), i18n("TV Device %1"));
  addToNameMap (maps[3], i18n("DVB Device"), i18n("DVB Device %1"));
}

void KPlayerDevicesNode::addToNameMap (QMap<QString, int>& map, const QString& device, const QString& deviceno)
{
  if ( map.count() == 1 )
    m_name_map.insert (map.begin().key(), device);
  else if ( map.count() > 1 )
  {
    int index = 0;
    bool found;
    do
    {
      found = false;
      QMap<QString, int>::Iterator imit (map.begin());
      while ( imit != map.end() )
      {
        if ( imit.value() == index )
        {
          m_name_map.insert (imit.key(), deviceno.arg (index));
          map.remove (imit.key());
          found = true;
          break;
        }
        ++ imit;
      }
      ++ index;
    }
    while ( found || index == 1 );
    if ( map.count() == 1 )
      m_name_map.insert (map.begin().key(), device);
    else
    {
      QMap<QString, int>::ConstIterator imit (map.begin());
      while ( imit != map.end() )
      {
        m_name_map.insert (imit.key(), deviceno.arg (index));
        ++ index;
        ++ imit;
      }
    }
  }
}

void KPlayerDevicesNode::completed (void)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::completed\n";
#endif
  refresh (m_lister.items());
  m_complete = true;
}

void KPlayerDevicesNode::refreshItem (KFileItem* item)
{
  QString path ("/dev" + item -> url().path());
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::refreshItem\n";
  kdDebugTime() << " Item   " << item -> url() << "\n";
  kdDebugTime() << " Type   " << item -> mimetype() << "\n";
  kdDebugTime() << " Text   " << item -> text() << "\n";
  kdDebugTime() << " Name   " << item -> name() << "\n";
  kdDebugTime() << " Local  " << item -> entry().stringValue (KIO::UDSEntry::UDS_LOCAL_PATH) << "\n";
  kdDebugTime() << " Comment " << item -> mimeComment() << "\n";
  kdDebugTime() << " Icon   " << item -> iconName() << "\n";
  kdDebugTime() << " URL    " << item -> url().url() << "\n";
  kdDebugTime() << " Permissions " << item -> permissions() << "\n";
  kdDebugTime() << " Mode   " << item -> mode() << "\n";
  kdDebugTime() << " User   " << item -> user() << "\n";
  kdDebugTime() << " Group  " << item -> group() << "\n";
  kdDebugTime() << " Size   " << item -> size() << "\n";
  kdDebugTime() << " Overlays " << item -> overlays() << "\n";
  kdDebugTime() << " Status " << item -> getStatusBarInfo() << "\n";
  kdDebugTime() << " Drops  " << item -> acceptsDrops() << "\n";
  kdDebugTime() << " Marked " << item -> isMarked() << "\n";
  kdDebugTime() << " Path   " << path << "\n";
#endif
  QString type (item -> mimetype() == "media/audiocd" ? I18N_NOOP("Audio CD")
    : item -> mimetype() == "media/dvdvideo" ? "DVD"
    : item -> mimetype() == "media/svcd" || item -> mimetype() == "media/vcd" ? I18N_NOOP("Video CD")
    : item -> mimetype().startsWith ("media/dvd") ? I18N_NOOP("Data DVD") : I18N_NOOP("Data CD"));
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << " Type   " << type << "\n";
#endif
  m_disk_types.insert (path, type);
  KPlayerDeviceNode* node = nodeById (path);
  if ( node && node -> diskDevice() )
  {
    KPlayerDiskNode* disk = (KPlayerDiskNode*) node;
    disk -> diskInserted (item -> entry().stringValue (KIO::UDSEntry::UDS_LOCAL_PATH));
    /*if ( type == "DVD" )
    {
      QString name (item -> name());
#ifdef DEBUG_KPLAYER_NODE
      kdDebugTime() << " Name   " << name << "\n";
#endif
      if ( ! name.isEmpty() && disk -> disk() && disk -> disk() -> name() == disk -> disk() -> defaultName() )
      {
        bool inword = false, capsonly = true;
        int i, length = name.length();
        for ( i = 0; i < length; ++ i )
        {
          QChar c = name.constref (i);
          if ( c.upper() != c )
          {
            capsonly = false;
            break;
          }
        }
        for ( i = 0; i < length; ++ i )
        {
          QChar& c = name.ref (i);
          if ( c == '_' )
            c = ' ';
          else if ( inword && c.isLetter() )
            c = c.lower();
          inword = c.isLetter();
        }
#ifdef DEBUG_KPLAYER_NODE
        kdDebugTime() << " Title  " << name << "\n";
#endif
        disk -> disk() -> setName (name);
        disk -> disk() -> commit();
      }
    }*/
  }
}

void KPlayerDevicesNode::refresh (const KFileItemList& items)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::refresh\n";
#endif
  for ( KFileItemList::ConstIterator iterator (items.begin()); iterator != items.end(); ++ iterator )
    refreshItem (*iterator);
}

void KPlayerDevicesNode::removed (KFileItem* item)
{
  QString path ("/dev" + item -> url().path());
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::removed\n";
  kdDebugTime() << " Item   " << item -> url() << "\n";
  kdDebugTime() << " Type   " << item -> mimetype() << "\n";
  kdDebugTime() << " Text   " << item -> text() << "\n";
  kdDebugTime() << " Name   " << item -> name() << "\n";
  kdDebugTime() << " Local  " << item -> entry().stringValue (KIO::UDSEntry::UDS_LOCAL_PATH) << "\n";
  kdDebugTime() << " Comment " << item -> mimeComment() << "\n";
  kdDebugTime() << " Icon   " << item -> iconName() << "\n";
  kdDebugTime() << " Path   " << path << "\n";
#endif
  m_disk_types.remove (path);
  KPlayerDeviceNode* node = nodeById (path);
  if ( node && node -> diskDevice() )
    ((KPlayerDiskNode*) node) -> diskRemoved();
}

void KPlayerDevicesNode::removed (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDevicesNode::removed\n";
#endif
  KPlayerNodeListIterator nlit (nodes);
  while ( KPlayerNode* node = nlit.current() )
  {
    QString id (node -> id());
    m_devices.removeAll (id);
    m_type_map.remove (id);
    m_disk_types.remove (id);
    ++ nlit;
  }
  KPlayerContainerNode::removed (nodes);
}

KPlayerDeviceNode::~KPlayerDeviceNode()
{
}

QString KPlayerDeviceNode::icon (void) const
{
  return "tv";
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
  diskInserted();
  if ( ! disk() )
    connect (media(), SIGNAL (updated()), SLOT (updated()));
}

void KPlayerDiskNode::setupSource (void)
{
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
  return type == "DVD" ? "dvd_unmount" : type == "Audio CD" ? "cdaudio_unmount" : "cdrom_unmount";
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
  disk() -> setDefaultName (i18n("%1 in %2").arg (i18n(type.toUtf8()), device() -> name()));
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
  kdDebugTime() << " URL    " << url() << "\n";
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
  const QString& type (parent() -> diskType (id()));
  if ( type.isNull() )
    disk() -> setDefaultName (i18n("Disk in %2").arg (device() -> name()));
  else
    setDiskType (type);
  if ( previous != device() && previous -> url().url().indexOf ('/', 15) >= 0
      && previous -> name() != previous -> defaultName() && media() -> name() == media() -> defaultName() )
    media() -> setName (previous -> name());
  media() -> diff (previous);
  media() -> commit();
  if ( previous != device() )
    KPlayerMedia::release (previous);
}

void KPlayerDiskNode::diskInserted (const QString& path)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::diskInserted\n";
#endif
  const QString& type (parent() -> diskType (id()));
  if ( type.isNull() )
    return;
  m_fast_autodetect = true;
  m_local_path = path;
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
  m_url = "list://";
  KIO::ListJob* job = KIO::listDir ("media:/" + url().fileName(), false, false);
  connect (job, SIGNAL(result(KIO::Job*)), SLOT(listResult(KIO::Job*)));
}

void KPlayerDiskNode::listResult (KIO::Job* job)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::listResult\n";
#endif
  if ( job -> error() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Error  " << job -> error() << " " << job -> errorString() << "\n";
#endif
    m_url = "mount://";
    KIO::SimpleJob* job = KIO::mount (true, 0, id(), QString::null, false);
    connect (job, SIGNAL(result(KIO::Job*)), SLOT(mountResult(KIO::Job*)));
  }
  else
  {
    m_url = "path://";
    KIO::StatJob* job = KIO::stat ("media:/" + url().fileName(), false);
    connect (job, SIGNAL(result(KIO::Job*)), SLOT(pathResult(KIO::Job*)));
  }
}

void KPlayerDiskNode::mountResult (KIO::Job* job)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::mountResult\n";
  if ( job -> error() )
    kdDebugTime() << " Error  " << job -> error() << " " << job -> errorString() << "\n";
#endif
  m_url = "path://";
  job = KIO::stat ("media:/" + url().fileName(), false);
  connect (job, SIGNAL(result(KIO::Job*)), SLOT(pathResult(KIO::Job*)));
}

void KPlayerDiskNode::pathResult (KIO::Job* job)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::pathResult\n";
#endif
  m_url = QString::null;
  if ( job -> error() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Error  " << job -> error() << " " << job -> errorString() << "\n";
#endif
    if ( disk() )
      disk() -> commit();
  }
  else
  {
    KFileItem item (((KIO::StatJob*) job) -> statResult(), "media:/" + url().fileName());
    if ( ! item.entry().stringValue (KIO::UDSEntry::UDS_LOCAL_PATH).isEmpty() )
      parent() -> refreshItem (&item);
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
  if ( ! m_fast_autodetect && ready() || m_fast_autodetect && disk() -> type() == "DVD" && m_url != "dvd://" )
    m_url = "dvd://";
  else if ( ! m_fast_autodetect && m_url == "dvd://"
      || m_fast_autodetect && disk() -> type() == "Audio CD" && m_url != "cdda://" )
    m_url = "cdda://";
  else if ( ! m_fast_autodetect && m_url == "cdda://"
    || m_fast_autodetect && disk() -> type().startsWith ("Data ") && m_url != "data://" )
  {
    m_url = "data://";
    KIO::StatJob* job = KIO::stat ("media:/" + url().fileName(), false);
    connect (job, SIGNAL(result(KIO::Job*)), SLOT(statResult(KIO::Job*)));
    return;
  }
  else if ( ! m_fast_autodetect && m_url == "data://"
      || m_fast_autodetect && disk() -> type() == "Video CD" && m_url != "vcd://" )
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
  connect (process, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*, int)),
    SLOT (receivedOutput (KPlayerLineOutputProcess*, char*, int)));
  connect (process, SIGNAL (processExited (K3Process*)), SLOT (processExited (K3Process*)));
  process -> start (K3Process::NotifyOnExit, K3Process::All);
}

void KPlayerDiskNode::receivedOutput (KPlayerLineOutputProcess*, char* str, int)
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
    uint track = re_track_length.cap(1).toUInt() - 1;
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

void KPlayerDiskNode::processExited (K3Process* process)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::processExited\n";
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
    connect (process, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*, int)),
      SLOT (receivedCddbOutput (KPlayerLineOutputProcess*, char*, int)));
    connect (process, SIGNAL (processExited (K3Process*)), SLOT (cddbProcessExited (K3Process*)));
    process -> start (K3Process::NotifyOnExit, K3Process::All);
    return;
  }
  if ( m_autodetected || m_url == "cdda://" && ! m_fast_autodetect )
    if ( ! accessDisk() )
    {
      diskRemoved();
      return;
    }
    else if ( dataDisk() || mediaDisk() && disk() -> hasTracks() )
    {
      setDiskType (disk() -> type());
      updateTracks();
      m_url = QString::null;
      disk() -> commit();
      return;
    }
  if ( m_autodetected && disk() )
    autodetected();
  else if ( m_url != "vcd://" || m_fast_autodetect && disk() && disk() -> hasType() )
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

void KPlayerDiskNode::statResult (KIO::Job* job)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerDiskNode::statResult\n";
#endif
  if ( job -> error() )
  {
#ifdef DEBUG_KPLAYER_NODE
    kdDebugTime() << " Error  " << job -> error() << " " << job -> errorString() << "\n";
#endif
  }
  else
  {
    KFileItem item (((KIO::StatJob*) job) -> statResult(), "media:/" + url().fileName());
    if ( ! item.entry().stringValue (KIO::UDSEntry::UDS_LOCAL_PATH).isEmpty() )
      parent() -> refreshItem (&item);
  }
  if ( dataDisk() )
  {
    m_url = QString::null;
    disk() -> commit();
  }
  else
    autodetect();
}

void KPlayerDiskNode::receivedCddbOutput (KPlayerLineOutputProcess*, char* str, int)
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

void KPlayerDiskNode::cddbProcessExited (K3Process* process)
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

int KPlayerNodeList::compareItems (Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
{
  return ((KPlayerNode*) item1) -> compare ((KPlayerNode*) item2);
}

void KPlayerNodeList::releaseAll (void) const
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNodeList::releaseAll\n";
#endif
  KPlayerNodeListIterator iterator (*this);
  while ( KPlayerNode* node = iterator.current() )
  {
    node -> release();
    ++ iterator;
  }
}

KPlayerNodeList KPlayerNodeList::fromUrlList (const KUrl::List& urls)
{
#ifdef DEBUG_KPLAYER_NODE
  kdDebugTime() << "KPlayerNodeList::fromUrlList\n";
#endif
  KPlayerNodeList list;
  KUrl::List::ConstIterator iterator (urls.begin());
  while ( iterator != urls.end() )
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
            url = KUrl::fromPathOrUrl (path);
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

int KPlayerNodeListByName::compareItems (Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
{
  return ((KPlayerNode*) item1) -> compareByName ((KPlayerNode*) item2);
}

KPlayerPlaylistNodeList::~KPlayerPlaylistNodeList()
{
}

int KPlayerPlaylistNodeList::compareItems (Q3PtrCollection::Item item1, Q3PtrCollection::Item item2)
{
  KPlayerNode* node1 = (KPlayerNode*) item1;
  KPlayerNode* node2 = (KPlayerNode*) item2;
  while ( node1 -> parent() )
  {
    KPlayerNode* node = node2;
    while ( node -> parent() )
    {
      if ( node1 == node -> parent() )
        return -1;
      if ( node == node1 -> parent() )
        return 1;
      if ( node1 -> parent() == node -> parent() )
        return node1 -> compare (node);
      node = node -> parent();
    }
    node1 = node1 -> parent();
  }
  return 0;
}
