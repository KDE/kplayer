/***************************************************************************
                          kplayersource.cpp
                          -----------------
    begin                : Thu Jan 12 2006
    copyright            : (C) 2006-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klocale.h>
#include <qregexp.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_SOURCE
#endif

#include "kplayersource.h"
#include "kplayersource.moc"
#include "kplayernode.h"
#include "kplayerproperties.h"

static QRegExp re_semicolon ("; *");

KPlayerSource::KPlayerSource (KPlayerContainerNode* parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating source\n";
#endif
  m_parent = parent;
  m_iterator = 0;
}

KPlayerSource::~KPlayerSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying source\n";
#endif
  delete m_iterator;
}

void KPlayerSource::connectOrigin (void)
{
}

bool KPlayerSource::has (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::has " << groups << "\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  bool group;
  QString id;
  start (groups);
  return next (group, id);
}

void KPlayerSource::connectNodes (KPlayerContainerNode* node)
{
  connect (node, SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  connect (node, SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  KPlayerNodeListIterator iterator (node -> nodes());
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( node -> isContainer() )
      connectNodes ((KPlayerContainerNode*) node);
    ++ iterator;
  }
}

void KPlayerSource::start (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::start\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
  kdDebugTime() << " Groups " << groups << "\n";
#endif
  m_groups = groups;
  delete m_iterator;
  if ( parent() -> populated() || groups && parent() -> groupsPopulated() )
    m_iterator = new KPlayerNodeListIterator (parent() -> nodes());
  else
  {
    m_iterator = 0;
    enumStart (groups);
  }
}

bool KPlayerSource::next (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::next\n";
#endif
  if ( ! m_iterator )
    return enumNext (group, id);
  while ( KPlayerNode* node = m_iterator -> current() )
  {
    group = node -> isContainer();
    id = node -> id();
#ifdef DEBUG_KPLAYER_SOURCE
    kdDebugTime() << " Group  " << group << "\n";
    kdDebugTime() << " ID     " << id << "\n";
#endif
    ++ *m_iterator;
    if ( group || ! m_groups )
      return true;
  }
  delete m_iterator;
  m_iterator = 0;
  return false;
}

bool KPlayerSource::verify (const QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::verify '" << id << "'\n";
#endif
  bool group;
  QString groupid;
  start (true);
  while ( next (group, groupid) )
    if ( groupid == id )
      return true;
  return false;
}

void KPlayerSource::save (void)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::save\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
}

bool KPlayerSource::canAddLeaves (void) const
{
  return true;
}

bool KPlayerSource::canAddBranches (void) const
{
  return true;
}

bool KPlayerSource::canRemove (void) const
{
  return true;
}

KPlayerNode* KPlayerSource::add (const KPlayerNodeList&, bool, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::add\n";
#endif
  return 0;
}

void KPlayerSource::added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::added\n";
#endif
}

void KPlayerSource::remove (const KPlayerNodeList&)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::remove\n";
#endif
}

void KPlayerSource::removed (KPlayerContainerNode*, const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::removed\n";
#endif
  KPlayerNodeList list;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( ! find (parent() -> origin(), node -> id()) )
      list.append (node);
    ++ iterator;
  }
  parent() -> removed (list);
  emit nodesRemoved (list);
}

bool KPlayerSource::find (KPlayerContainerNode* node, const QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSource::find '" << id << "'\n";
#endif
  if ( node -> nodeById (id) )
    return true;
  KPlayerNodeListIterator iterator (node -> nodes());
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( ! node -> isContainer() )
      break;
    if ( find ((KPlayerContainerNode*) node, id) )
      return true;
    ++ iterator;
  }
  return false;
}

KPlayerListSource::KPlayerListSource (KPlayerContainerNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating list source\n";
#endif
}

KPlayerListSource::~KPlayerListSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying list source\n";
#endif
}

void KPlayerListSource::enumStart (bool)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerListSource::start\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  const QStringList& children (parent() -> media() -> children());
  m_iterator = children.begin();
  m_end = children.end();
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << " Count  " << children.count() << "\n";
#endif
}

bool KPlayerListSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerListSource::next\n";
#endif
  if ( m_iterator == m_end )
    return false;
  group = true;
  id = *m_iterator;
  ++ m_iterator;
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << " Group  " << group << "\n";
  kdDebugTime() << " ID     " << id << "\n";
#endif
  return ! id.isEmpty();
}

void KPlayerListSource::save (void)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerListSource::save\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  parent() -> save();
}

void KPlayerListSource::remove (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerListSource::remove\n";
#endif
  parent() -> removed (nodes);
}

KPlayerFixedSource::KPlayerFixedSource (KPlayerContainerNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating fixed source\n";
#endif
}

KPlayerFixedSource::~KPlayerFixedSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying fixed source\n";
#endif
}

bool KPlayerFixedSource::canAddLeaves (void) const
{
  return false;
}

bool KPlayerFixedSource::canAddBranches (void) const
{
  return false;
}

bool KPlayerFixedSource::canRemove (void) const
{
  return false;
}

bool KPlayerFixedSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerFixedSource::next\n";
#endif
  if ( m_fixed_ids.isEmpty() )
    return false;
  id = m_fixed_ids.first();
  group = true;
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << " Group  " << group << "\n";
  kdDebugTime() << " ID     " << id << "\n";
#endif
  m_fixed_ids.remove (id);
  return true;
}

KPlayerRootSource::KPlayerRootSource (KPlayerContainerNode* parent)
  : KPlayerFixedSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating root source\n";
#endif
}

KPlayerRootSource::~KPlayerRootSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying root source\n";
#endif
}

void KPlayerRootSource::enumStart (bool)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerRootSource::start\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  m_fixed_ids = parent() -> defaultIds();
}

KPlayerDevicesSource::KPlayerDevicesSource (KPlayerContainerNode* parent)
  : KPlayerListSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating devices source\n";
#endif
}

KPlayerDevicesSource::~KPlayerDevicesSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying devices source\n";
#endif
}

bool KPlayerDevicesSource::canAddLeaves (void) const
{
  return false;
}

void KPlayerDevicesSource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDevicesSource::start\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  parent() -> update();
  m_pending = parent() -> devices();
  KPlayerListSource::enumStart (groups);
}

bool KPlayerDevicesSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDevicesSource::next\n";
#endif
  if ( ! KPlayerListSource::enumNext (group, id) )
  {
    if ( m_pending.isEmpty() )
      return false;
    id = m_pending.first();
    group = true;
#ifdef DEBUG_KPLAYER_SOURCE
    kdDebugTime() << " Group  " << group << "\n";
    kdDebugTime() << " ID     " << id << "\n";
#endif
  }
  m_pending.remove (id);
  return true;
}

KPlayerDeviceSource::KPlayerDeviceSource (KPlayerContainerNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating device source\n";
#endif
}

KPlayerDeviceSource::~KPlayerDeviceSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying device source\n";
#endif
}

bool KPlayerDeviceSource::canAddBranches (void) const
{
  return false;
}

bool KPlayerDeviceSource::canAddLeaves (void) const
{
  return false;
}

bool KPlayerDeviceSource::canRemove (void) const
{
  return false;
}

KPlayerDiskSource::KPlayerDiskSource (KPlayerContainerNode* parent)
  : KPlayerDeviceSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating disk device source\n";
#endif
}

KPlayerDiskSource::~KPlayerDiskSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying disk device source\n";
#endif
}

void KPlayerDiskSource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDiskSource::start\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  m_track = 0;
  m_tracks = groups ? 0 : parent() -> tracks();
}

bool KPlayerDiskSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDiskSource::next\n";
#endif
  if ( m_track < m_tracks )
  {
    ++ m_track;
    group = false;
    id = QString::number (m_track);
    return true;
  }
  return false;
}

KPlayerTunerSource::KPlayerTunerSource (KPlayerContainerNode* parent)
  : KPlayerDeviceSource (parent), m_list (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating TV/DVB device source\n";
#endif
}

KPlayerTunerSource::~KPlayerTunerSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying TV/DVB device source\n";
#endif
}

void KPlayerTunerSource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerTunerSource::start\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  if ( groups )
    m_pending.clear();
  else
  {
    m_pending = ((KPlayerTunerProperties*)parent() -> media()) -> channels();
    m_list.start (groups);
  }
}

bool KPlayerTunerSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerTunerSource::next\n";
#endif
  if ( m_groups )
    return false;
  if ( ! m_list.next (group, id) )
  {
    if ( m_pending.isEmpty() )
      return false;
    id = m_pending.first();
    group = false;
#ifdef DEBUG_KPLAYER_SOURCE
    kdDebugTime() << " Group  " << group << "\n";
    kdDebugTime() << " ID     " << id << "\n";
#endif
  }
  m_pending.remove (id);
  return true;
}

KPlayerStoreSource::KPlayerStoreSource (KPlayerContainerNode* parent)
  : KPlayerListSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating store source\n";
#endif
}

KPlayerStoreSource::~KPlayerStoreSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying store source\n";
#endif
}

bool KPlayerStoreSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerStoreSource::next\n";
#endif
  while ( KPlayerListSource::enumNext (group, id) )
  {
    group = id.find ('/') < 0;
    if ( group || ! m_groups )
    {
#ifdef DEBUG_KPLAYER_SOURCE
      kdDebugTime() << " Group  " << group << "\n";
      kdDebugTime() << " ID     " << id << "\n";
#endif
      return true;
    }
  }
  return false;
}

KPlayerNode* KPlayerStoreSource::add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerStoreSource::add " << (link ? "link" : "copy") << " nodes\n";
#endif
  return parent() -> added (nodes, link, after);
}

KPlayerDirectorySource::KPlayerDirectorySource (KPlayerContainerNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating directory source\n";
#endif
  m_iterator = 0;
  m_directory = parent -> url().path();
  m_directory.setFilter (QDir::All);
  m_directory.setSorting (QDir::Name);
  m_watch.addDir (m_directory.path());
  if ( m_watch.internalMethod() != KDirWatch::Stat )
  {
    m_watch.startScan();
    connect (&m_watch, SIGNAL (dirty (const QString&)), SLOT (dirty (const QString&)));
  }
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << " Path   " << m_directory.path() << "\n";
  kdDebugTime() << " Method " << m_watch.internalMethod() << "\n";
#endif
}

KPlayerDirectorySource::~KPlayerDirectorySource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying directory source\n";
#endif
  delete m_iterator;
}

bool KPlayerDirectorySource::canAddLeaves (void) const
{
  return false;
}

bool KPlayerDirectorySource::canAddBranches (void) const
{
  return false;
}

bool KPlayerDirectorySource::canRemove (void) const
{
  return false;
}

bool checkMimeType (const QString& path)
{
  KMimeType::Ptr mimetype (KMimeType::findByPath (path));
  const QString& name (mimetype -> name().lower());
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << " Mime Type " << name << "\n";
#endif
  if ( name.startsWith ("video/") )
    return name != "video/x-mng";
  else if ( name.startsWith ("audio/") )
    return name != "audio/x-karaoke" && name != "audio/x-midi";
  else
    return name == "application/ogg" || name == "application/smil" || name == "application/vnd.ms-asf"
      || name.startsWith ("application/vnd.rn-realmedia") || name == "application/x-cda"
      || name == "application/x-cue" || name == "application/x-mplayer2" || name == "application/x-ogg"
      || name == "application/x-smil" || name == "application/x-streamingmedia"
      || name.startsWith ("uri/mms") || name.startsWith ("uri/pnm") || name.startsWith ("uri/rtsp");
}

bool checkFileInfo (const QFileInfo* info)
{
  return info -> fileName() != "." && info -> fileName() != ".."
    && info -> exists() && info -> isReadable()
    && (info -> isDir() || checkMimeType (info -> filePath()));
}

void KPlayerDirectorySource::dirty (const QString&)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDirectorySource::dirty\n";
  kdDebugTime() << " Path   " << m_directory.path() << "\n";
#endif
  const QFileInfoList* list = m_directory.entryInfoList();
  if ( ! list )
    return;
  QStringList previous;
  for ( QFileInfoListIterator iterator (*list); iterator.current(); ++ iterator )
    previous << iterator.current() -> fileName();
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << " Previous " << previous.count() << "\n";
#endif
  m_directory = m_directory.path();
  list = m_directory.entryInfoList();
  if ( list )
  {
#ifdef DEBUG_KPLAYER_SOURCE
    kdDebugTime() << " Current " << list -> count() << "\n";
#endif
    QFileInfoList current;
    for ( QFileInfoListIterator it (*list); it.current(); ++ it )
    {
      QFileInfo* info = it.current();
      QString name (info -> fileName());
      if ( previous.contains (name) )
        previous.remove (name);
      else if ( checkFileInfo (info) )
        current.append (info);
    }
#ifdef DEBUG_KPLAYER_SOURCE
    kdDebugTime() << " Added " << current.count() << "\n";
    kdDebugTime() << " Removed " << previous.count() << "\n";
#endif
    parent() -> added (current);
    parent() -> removed (previous);
  }
}

bool KPlayerDirectorySource::verify (const QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDirectorySource::verify '" << id << "'\n";
#endif
  QFileInfo info (m_directory, id);
  return info.exists() && info.isDir();
}

void KPlayerDirectorySource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDirectorySource::start " << groups << "\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  delete m_iterator;
  m_iterator = 0;
  const QFileInfoList* list = m_directory.entryInfoList();
  if ( list )
  {
    m_iterator = new QFileInfoListIterator (*list);
#ifdef DEBUG_KPLAYER_SOURCE
    kdDebugTime() << " Count  " << list -> count() << "\n";
#endif
  }
}

bool KPlayerDirectorySource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerDirectorySource::next\n";
#endif
  if ( ! m_iterator )
    return false;
  QFileInfo* info;
  do
  {
    info = m_iterator -> current();
    ++ *m_iterator;
    if ( ! info )
    {
      delete m_iterator;
      m_iterator = 0;
      return false;
    }
    group = info -> isDir();
  }
  while ( m_groups && ! group || ! checkFileInfo (info) );
  id = info -> fileName(); // group ? : KURL::fromPathOrURL (info -> filePath()).url();
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << " Group  " << group << "\n";
  kdDebugTime() << " ID     " << id << "\n";
#endif
  return true;
}

KPlayerOriginSource::KPlayerOriginSource (KPlayerContainerNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating origin source\n";
#endif
}

KPlayerOriginSource::~KPlayerOriginSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying origin source\n";
#endif
}

void KPlayerOriginSource::connectOrigin (void)
{
  connect (parent() -> origin(), SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  connect (parent() -> origin(), SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  connect (parent() -> origin(), SIGNAL (detached()), SLOT (detach()));
}

void KPlayerOriginSource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::start " << groups << "\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  parent() -> origin() -> source() -> start (groups);
}

bool KPlayerOriginSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::next\n";
#endif
  if ( ! parent() -> origin() -> source() -> next (group, id) )
    return false;
  if ( ! group && parent() -> isGroup() && ! parent() -> origin() -> isGroup() )
    id = parent() -> origin() -> metaurl (id).url();
  return true;
}

KPlayerNode* KPlayerOriginSource::add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::add " << (link ? "link" : "copy") << " nodes\n";
#endif
  parent() -> detachOrigin();
  return parent() -> source() -> add (nodes, link, after);
}

void KPlayerOriginSource::added (KPlayerContainerNode*, const KPlayerNodeList& nodes, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::added\n";
#endif
  if ( after )
  {
    QString id (after -> id());
    if ( ! after -> isContainer() && parent() -> isGroup() && ! parent() -> origin() -> isGroup() )
      id = parent() -> origin() -> metaurl (id).url();
    after = parent() -> nodeById (id);
    if ( ! after )
      after = parent();
  }
  parent() -> added (nodes, true, after);
}

void KPlayerOriginSource::remove (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::remove\n";
#endif
  parent() -> detachOrigin();
  parent() -> source() -> remove (nodes);
}

void KPlayerOriginSource::removed (KPlayerContainerNode*, const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::removed\n";
#endif
  QStringList ids;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    QString id (node -> id());
    if ( ! node -> isContainer() && parent() -> isGroup() && ! parent() -> origin() -> isGroup() )
      id = parent() -> origin() -> metaurl (id).url();
    ids << id;
    ++ iterator;
  }
  parent() -> removed (ids);
}

void KPlayerOriginSource::detach (void)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::detach\n";
#endif
  parent() -> releaseOrigin();
}

void KPlayerOriginSource::save (void)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerOriginSource::save\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  parent() -> save();
}

#if 0
KPlayerKeySource::KPlayerKeySource (KPlayerCollectionNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating key source\n";
#endif
}

KPlayerKeySource::~KPlayerKeySource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying key source\n";
#endif
}

void KPlayerKeySource::connectOrigin (void)
{
  connect (parent() -> origin() -> source(), SIGNAL (nodesAdded (const KPlayerNodeList&)),
    SLOT (added (const KPlayerNodeList&)));
  connect (parent() -> origin() -> source(), SIGNAL (nodesRemoved (const KPlayerNodeList&)),
    SLOT (removed (const KPlayerNodeList&)));
}

void KPlayerKeySource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerKeySource::start " << groups << "\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  if ( groups == parent() -> isGrouped() )
    parent() -> origin() -> source() -> start (groups);
}

bool KPlayerKeySource::match (const QString& id)
{
  QString value (meta (id, parent() -> parentKey()));
  QStringList values (QStringList::split (re_semicolon, value));
  return values.contains (parent() -> id()) > 0;
}

bool KPlayerKeySource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerKeySource::next\n";
#endif
  while ( parent() -> origin() -> source() -> next (group, id) )
    if ( match (id) )
    {
      if ( parent() -> isGrouped() )
      {
        id = meta (id, parent() -> groupingKey(), i18n("unknown"));
        group = true;
      }
#ifdef DEBUG_KPLAYER_SOURCE
      kdDebugTime() << " Group  " << group << "\n";
      kdDebugTime() << " ID     " << id << "\n";
#endif
      return true;
    }
  return false;
}

KPlayerNode* KPlayerKeySource::add (const KPlayerNodeList& nodes, bool link, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerKeySource::add " << (link ? "link" : "copy") << " nodes\n";
#endif
  brand (nodes);
  return parent() -> origin() -> source() -> add (nodes, link);
}

void KPlayerKeySource::added (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerKeySource::added\n";
#endif
  KPlayerNodeList list, groups;
  groups.setAutoDelete (true);
  KPlayerNodeMap map;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( match (node -> url().url()) )
    {
      list.append (node);
      if ( ! parent() -> groupingKey().isEmpty() )
      {
        QString value (node -> meta (parent() -> groupingKey()));
        if ( ! map.contains (value) && ! parent() -> nodeById (value) )
        {
          groups.append (new KPlayerCollectionNode);
          groups.current() -> setup (0, value);
          map.insert (value, groups.current());
        }
      }
    }
    ++ iterator;
  }
  if ( parent() -> isGrouped() )
  {
    parent() -> added (groups, false);
    emit nodesAdded (list);
  }
  else
    parent() -> added (list, false);
}

void KPlayerKeySource::brand (const KPlayerNodeList& nodes)
{
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( node -> isContainer() )
      brand (((KPlayerContainerNode*) node) -> nodes());
    else
    {
      QString value (node -> meta (parent() -> parentKey()));
      if ( value.isEmpty() )
        value = parent() -> id();
      else
        value += ';' + parent() -> id();
      node -> meta() -> writeEntry (parent() -> parentKey(), value);
    }
    ++ iterator;
  }
}

void KPlayerKeySource::remove (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerKeySource::remove\n";
#endif
  KPlayerNodeList list;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( node -> isContainer() )
    {
      KPlayerContainerNode* container = (KPlayerContainerNode*) node;
      container -> source() -> remove (container -> nodes());
    }
    else
    {
      QString value (node -> meta (parent() -> parentKey()));
      QStringList values (QStringList::split (re_semicolon, value));
      if ( values.remove (parent() -> id()) <= 0 || values.isEmpty() )
        list.append (node);
      else
        node -> meta() -> writeEntry (parent() -> parentKey(), values.join (";"));
    }
    ++ iterator;
  }
  if ( ! list.isEmpty() )
    parent() -> origin() -> source() -> remove (list);
}

void KPlayerKeySource::removed (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerKeySource::removed\n";
#endif
  parent() -> removed (nodes);
  emit nodesRemoved (nodes);
}

KPlayerSearchesSource::KPlayerSearchesSource (KPlayerContainerNode* parent)
  : KPlayerListSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating searches source\n";
#endif
}

KPlayerSearchesSource::~KPlayerSearchesSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying searches source\n";
#endif
}

bool KPlayerSearchesSource::canAddLeaves (void) const
{
  return false;
}

bool KPlayerSearchesSource::canAddBranches (void) const
{
  return false;
}

KPlayerSearchSource::KPlayerSearchSource (KPlayerContainerNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating search source\n";
#endif
}

KPlayerSearchSource::~KPlayerSearchSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying search source\n";
#endif
}

void KPlayerSearchSource::connectOrigin (void)
{
  connectNodes (parent() -> origin());
  connect (parent() -> origin(), SIGNAL (detached()), SLOT (detach()));
}

bool KPlayerSearchSource::canAddLeaves (void) const
{
  return false;
}

bool KPlayerSearchSource::canAddBranches (void) const
{
  return false;
}

bool KPlayerSearchSource::canRemove (void) const
{
  return false;
}

void KPlayerSearchSource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSearchSource::start " << groups << "\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  parent() -> origin() -> source() -> start (groups);
}

bool KPlayerSearchSource::match (const QString& id)
{
  return meta (id, "Name") == parent() -> id();
}

bool KPlayerSearchSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSearchSource::next\n";
#endif
  while ( parent() -> origin() -> source() -> next (group, id) )
    if ( ! group && match (id) )
    {
#ifdef DEBUG_KPLAYER_SOURCE
      kdDebugTime() << " Group  " << group << "\n";
      kdDebugTime() << " ID     " << id << "\n";
#endif
      return true;
    }
  return false;
}

void KPlayerSearchSource::added (KPlayerContainerNode*, const KPlayerNodeList& nodes, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSearchSource::added\n";
#endif
  KPlayerNodeList list;
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( ! node -> isContainer() && match (node -> url().url()) )
      list.append (node);
    ++ iterator;
  }
  parent() -> added (list, false);
}

void KPlayerSearchSource::detach (void)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerSearchSource::detach\n";
#endif
  KPlayerNodeList list;
  list.append (parent());
  parent() -> parent() -> remove (list);
}

KPlayerGroupSource::KPlayerGroupSource (KPlayerCollectionNode* parent)
  : KPlayerSource (parent)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Creating group source\n";
#endif
  m_current = 0;
}

KPlayerGroupSource::~KPlayerGroupSource()
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "Destroying group source\n";
#endif
}

void KPlayerGroupSource::connectOrigin (void)
{
  connectNodes (parent() -> origin());
}

void KPlayerGroupSource::enumStart (bool groups)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerGroupSource::start\n";
  kdDebugTime() << " ID     " << parent() -> id() << "\n";
#endif
  if ( groups == parent() -> isGrouped() )
  {
    m_current = parent() -> origin();
    m_current -> source() -> start (false);
  }
}

bool KPlayerGroupSource::enumNext (bool& group, QString& id)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerGroupSource::next\n";
#endif
  while ( m_current && m_current != parent() -> origin() -> parent() )
  {
    while ( m_current -> source() -> next (group, id) )
    {
      if ( ! group )
      {
        if ( parent() -> isGrouped() )
        {
          id = meta (id, parent() -> groupingKey(), i18n("unknown"));
          group = true;
        }
#ifdef DEBUG_KPLAYER_SOURCE
        kdDebugTime() << " Group  " << group << "\n";
        kdDebugTime() << " ID     " << id << "\n";
#endif
        return true;
      }
      KPlayerNode* node = m_current -> nodeById (id);
      m_current = node && node -> isContainer() ? (KPlayerContainerNode*) node : 0;
      if ( ! m_current )
        return false;
      m_current -> source() -> start (false);
    }
    m_current = m_current -> parent();
  }
  return false;
}

KPlayerNode* KPlayerGroupSource::add (const KPlayerNodeList& nodes, bool link, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerGroupSource::add " << (link ? "link" : "copy") << " nodes\n";
#endif
  return parent() -> origin() -> source() -> add (nodes, link);
}

void KPlayerGroupSource::added (KPlayerContainerNode*, const KPlayerNodeList& nodes, KPlayerNode*)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerGroupSource::added\n";
#endif
  if ( parent() -> isGrouped() )
  {
    KPlayerNodeList groups;
    groups.setAutoDelete (true);
    KPlayerNodeMap map;
    KPlayerNodeListIterator iterator (nodes);
    while ( KPlayerNode* node = iterator.current() )
    {
      QString value (node -> meta (parent() -> groupingKey()));
      if ( ! map.contains (value) && ! parent() -> nodeById (value) )
      {
        groups.append (new KPlayerCollectionNode);
        groups.current() -> setup (0, value);
        map.insert (value, groups.current());
      }
      ++ iterator;
    }
    parent() -> added (groups, false);
    emit nodesAdded (nodes);
  }
  else
    parent() -> added (nodes, false);
}

void KPlayerGroupSource::remove (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_SOURCE
  kdDebugTime() << "KPlayerGroupSource::remove\n";
#endif
  remove (parent() -> origin(), nodes);
}

void KPlayerGroupSource::remove (KPlayerContainerNode* node, const KPlayerNodeList& nodes)
{
  if ( ! node )
    return;
  node -> remove (nodes);
  bool group;
  QString id;
  node -> source() -> start (true);
  while ( node -> source() -> next (group, id) )
    remove ((KPlayerContainerNode*) node -> nodeById (id), nodes);
}
#endif
