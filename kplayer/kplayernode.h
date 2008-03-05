/***************************************************************************
                          kplayernode.h
                          -------------
    begin                : Wed Feb 16 2005
    copyright            : (C) 2005-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERNODE_H
#define KPLAYERNODE_H

#include "kplayerproperties.h"
#include "kplayersource.h"

#include <qmap.h>
#include <solid/device.h>
#include <solid/solidnamespace.h>

class KPlayerContainerNode;
class KPlayerMediaNode;
class KPlayerNode;
class KPlayerRootNode;
class KPlayerDeviceNode;

/**Node map by ID.
  * @author kiriuja
  */
typedef QMap<QString, KPlayerNode*> KPlayerNodeMap;

/**Container node map by ID.
  * @author kiriuja
  */
typedef QMap<QString, KPlayerContainerNode*> KPlayerContainerNodeMap;

/**Node list.
  *@author kiriuja
  */
class KPlayerNodeList : public QList<KPlayerNode*>
{
public:
  /** Constructor. */
  KPlayerNodeList (void) { }
  /** Destructor. */
  virtual ~KPlayerNodeList();

  /** Sorts the node list by name. */
  virtual void sort (void);

  /** Releases all nodes on the list. */
  void releaseAll (void) const;

  /** Returns the first node or zero. */
  KPlayerNode* first (void) const
    { return isEmpty() ? 0 : QList<KPlayerNode*>::first(); }

  /** Returns the last node or zero. */
  KPlayerNode* last (void) const
    { return isEmpty() ? 0 : QList<KPlayerNode*>::last(); }

  /** Creates a node list from the given URL list. */
  static KPlayerNodeList fromUrlList (const KUrl::List& urls);
};

/**Node list sorted by name.
  *@author kiriuja
  */
class KPlayerNodeListByName : public KPlayerNodeList
{
public:
  /** Constructor. */
  KPlayerNodeListByName (void) { }
  /** Destructor. */
  virtual ~KPlayerNodeListByName();

  /** Sorts the node list by name. */
  virtual void sort (void);
};

/**Playlist node list.
  *@author kiriuja
  */
class KPlayerPlaylistNodeList : public KPlayerNodeList
{
public:
  /** Constructor. */
  KPlayerPlaylistNodeList (void) { }
  /** Destructor. */
  virtual ~KPlayerPlaylistNodeList();

  /** Sorts the node list by name. */
  virtual void sort (void);
};

/**Node in the hierarchy.
  *@author kiriuja
  */
class KPlayerNode : public QObject
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerNode();

  /** Initializes the node. */
  void setup (KPlayerContainerNode* parent, const QString& id, KPlayerContainerNode* origin = 0);

  /** Unique identifier of the node within its parent node. */
  const QString& id (void) const
    { return m_id; }
  /** Suggested identifier for a copy of this node. */
  virtual QString suggestId (void) const;

  /** Node name. */
  QString name (void) const
    { return media() -> name(); }

  /** Node URL. Gives node location in the hierarchy. */
  virtual KUrl url (void) const;
  /** The URL of a subnode with the given ID. */
  KUrl url (const QString& id) const;
  /** Media URL. Refers to the meta information storage of the node. */
  virtual KUrl metaurl (void) const;
  /** Media URL of a subnode with the given ID. */
  KUrl metaurl (const QString& id) const;

  /** Parent node. Null for root node. */
  KPlayerContainerNode* parent (void) const
    { return m_parent; }
  /** Sets the parent node. */
  void setParent (KPlayerContainerNode* node)
    { m_parent = node; }

  /** Media properties. */
  KPlayerGenericProperties* media (void) const
    { return m_media; }

  /** Returns icon name. */
  virtual QString icon (void) const;

  /** Configuration. */
  KPlayerConfiguration* configuration (void) const
    { return KPlayerEngine::engine() -> configuration(); }
  /** Settings. */
  KPlayerSettings* settings (void) const
    { return KPlayerEngine::engine() -> settings(); }

  /** Returns the top level node. */
  KPlayerContainerNode* topLevelNode (void) const;

  /** Returns whether the node is ready for playing. */
  virtual bool ready (void) const;

  /** Returns whether the node is a container. */
  virtual bool isContainer (void) const;

  /** Returns whether the node has media properties. */
  virtual bool hasProperties (void) const;

  /** Returns whether the node can be renamed. */
  virtual bool canRename (void) const;

  /** Compares this node to the the given node by name. */
  int compareByName (KPlayerNode* node) const;
  /** Compares this node to the the given node using the current criteria. */
  int compare (KPlayerNode* node) const;

  /** Adds the node attribute counts to the given map. */
  void countAttributes (KPlayerPropertyCounts& counts) const;

  /** Adds a reference keeping the node in memory. */
  void reference (void)
    { ++ m_references; }
  /** Release a reference and delete the node if no references remain. */
  void release (void);
  /** Detaches the node and its subnodes. */
  void detach (void);

  /** Returns the previous media node starting from this node. */
  KPlayerMediaNode* previousMediaNode (void);
  /** Returns the last media node starting from this node. */
  virtual KPlayerMediaNode* lastMediaNode (void) = 0;

  /** Returns the root node of the hierarchy. */
  static KPlayerRootNode* root (void)
    { return m_root; }

  /** Initializes the node hierarchy. */
  static void initialize (void);
  /** Terminates the node hierarchy. */
  static void terminate (void);

  /** Sets the sorting key and ascending order. */
  static void setSorting (const QString& key, bool ascending);
  /** Returns whether nodes are being sorted by name. */
  static bool sortByName (void)
    { return m_sort_by_name; }
  /** Returns the sort key. */
  static const QString& sortKey (void)
    { return m_sort_key; }
  /** Returns whether to sort in ascending order. */
  static bool sortAscending (void)
    { return m_sort_ascending; }

protected slots:
  /** Checks if the meta information has changed and emits the meta signals. */
  virtual void updated (void);

protected:
  /** Initializes the node media. */
  virtual void setupMedia (void);
  /** Initializes the node children. */
  virtual void setupChildren (KPlayerContainerNode* origin);

  /** Unique identifier of the node within its parent node. */
  QString m_id;
  /** Parent node. Null for root node. */
  KPlayerContainerNode* m_parent;
  /** Reference count of the node. */
  int m_references;

  /** Media properties. */
  KPlayerGenericProperties* m_media;

  /** Root node of the hierarchy. */
  static KPlayerRootNode* m_root;

  /** Flag indicating whether the list is being sorted by name. */
  static bool m_sort_by_name;
  /** Sort key. */
  static QString m_sort_key;
  /** Sort in ascending order. */
  static bool m_sort_ascending;
};

/**Media node.
  *@author kiriuja
  */
class KPlayerMediaNode : public KPlayerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerMediaNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerMediaNode();

  /** Returns whether the node has media properties. */
  virtual bool hasProperties (void) const;

  /** Returns the last media node starting from this node. */
  virtual KPlayerMediaNode* lastMediaNode (void);

protected:
  /** Initializes the node media. */
  virtual void setupMedia (void);
  /** Initializes the node URL. */
  virtual void setupUrl (void);
};

/**File node.
  *@author kiriuja
  */
class KPlayerFileNode : public KPlayerMediaNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerFileNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerFileNode();

  /** Returns whether the node can be renamed. */
  virtual bool canRename (void) const;
};

/**Track node.
  *@author kiriuja
  */
class KPlayerTrackNode : public KPlayerMediaNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerTrackNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerTrackNode();

  /** Parent node. */
  KPlayerDiskNode* parent (void) const
    { return (KPlayerDiskNode*) m_parent; }

  /** Media URL. Refers to the meta information storage of the node. */
  virtual KUrl metaurl (void) const;
};

/**Channel node.
  *@author kiriuja
  */
class KPlayerChannelNode : public KPlayerMediaNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerChannelNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerChannelNode();

  /** Parent node. */
  KPlayerDeviceNode* parent (void) const
    { return (KPlayerDeviceNode*) m_parent; }
  /** Media properties. */
  KPlayerChannelProperties* media (void) const
    { return (KPlayerChannelProperties*) m_media; }
};

/**Item node.
  *@author kiriuja
  */
class KPlayerItemNode : public KPlayerMediaNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerItemNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerItemNode();

  /** Media URL. Refers to the meta information storage of the node. */
  virtual KUrl metaurl (void) const;

protected:
  /** Initializes the node URL. */
  virtual void setupUrl (void);

  /** Media URL. Refers to the meta information storage of the node. */
  KUrl m_url;
};

#if 0
/**Search item node.
  *@author kiriuja
  */
class KPlayerSearchItemNode : public KPlayerItemNode
{
  //Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerSearchItemNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerSearchItemNode();
};
#endif

/**Container node.
  *@author kiriuja
  */
class KPlayerContainerNode : public KPlayerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerContainerNode (void);
  /** Destructor. Frees resources. */
  virtual ~KPlayerContainerNode();

  /** Origin node, for linked group nodes only. */
  KPlayerContainerNode* origin (void) const
    { return m_origin; }
  /** Sets the origin node. */
  void setOrigin (KPlayerContainerNode* node)
    { m_origin = node; }

  /** Source of subnodes. */
  KPlayerSource* source (void) const
    { return m_source; }
  /** Sets the source. */
  void setSource (KPlayerSource* source)
    { m_source = source; }

  /** List of subnodes. */
  const KPlayerNodeList& nodes (void) const
    { return m_nodes; }

  /** Returns whether the node is ready for playing. */
  virtual bool ready (void) const;

  /** Returns icon name. */
  virtual QString icon (void) const;
  /** Returns open icon name. */
  virtual QString openIcon (void) const;

  /** Suggested identifier for a copy of this node. */
  virtual QString suggestId (void) const;

  /** Returns whether the node is a container. */
  virtual bool isContainer (void) const;
  /** Returns whether the node is a group. */
  virtual bool isGroup (void) const;
  /** Returns whether the node is a directory. */
  virtual bool isDirectory (void) const;
  /** Returns whether the node is a collection group. */
  virtual bool isCollection (void) const;
  /** Indicates whether the node is a playlist. */
  virtual bool isPlaylist (void) const;
  /** Indicates whether the node is a recent node. */
  virtual bool isRecent (void) const;
  /** Returns whether the node is a now playing group. */
  virtual bool isNowPlaying (void) const;
  /** Indicates whether the node is the device list. */
  virtual bool isDevices (void) const;

  /** Returns whether the node and its subnodes can be queued for playing. */
  bool canQueue (void) const;

#if 0
  /** Returns whether the node can be grouped. */
  bool canGroup (void) const;
#endif

  /** Returns whether the node can be saved as a playlist. */
  virtual bool canSaveAsPlaylist (void) const;

  /** Indicates whether leaves can be added. */
  bool canAddLeaves (void) const
    { return source() -> canAddLeaves(); }
  /** Indicates whether branches can be added. */
  bool canAddBranches (void) const
    { return source() -> canAddBranches(); }
  /** Indicates whether nodes can be removed. */
  bool canRemove (void) const
    { return source() -> canRemove(); }

  /** Indicates whether nodes are arranged in a custom order. */
  bool customOrder (void) const;
  /** Sets whether nodes are arranged in a custom order. */
  void setCustomOrder (bool custom);
  /** Sets custom order by name if not already set. */
  void customOrderByName (void);

  /** Returns whether custom order is allowed. */
  virtual bool allowsCustomOrder (void) const;

  /** Indicates whether duplicates are acceptable. */
  virtual bool acceptsDuplicates (void) const;

  /** Indicates whether groups should be put before items when sorting. */
  bool groupsFirst (void) const;

  /** Compares the given nodes by their position in the list of nodes. */
  int compareByPosition (KPlayerNode* node1, KPlayerNode* node2);

  /** Returns whether there are child nodes. */
  bool hasNodes (void) const
    { return m_source -> has (false); }
  /** Returns whether there are child groups. */
  bool hasGroups (void) const
    { return m_source -> has (true); }

  /** Returns whether the given node can be copied into this container. */
  bool canCopy (const KPlayerNodeList& nodes) const;
  /** Returns whether the given nodes can be linked into this container. */
  bool canLink (const KPlayerNodeList& nodes) const;
  /** Returns whether the given node can be linked into this container. */
  virtual bool canLink (KPlayerContainerNode* node) const;
  /** Returns whether the given node belongs to the same top level node as this one. */
  bool preferMove (KPlayerNode* node) const;

  /** Returns a node for the given ID from the node list. */
  KPlayerNode* nodeById (const QString& id) const;
  /** Returns a node for the given ID if it exists. */
  KPlayerContainerNode* getNodeById (const QString& id);
  /** Returns a node at the given path if it exists. */
  virtual KPlayerContainerNode* getNodeByPath (const QString& path);

  /** Returns the next node after the given one. */
  KPlayerNode* nextNode (KPlayerNode*);
  /** Returns the last media node starting from this node. */
  virtual KPlayerMediaNode* lastMediaNode (void);

  /** Returns whether nodes are populated. */
  bool populated (void)
    { return m_populate_nodes > 0; }
  /** Returns whether groups are populated. */
  bool groupsPopulated (void)
    { return m_populate_groups > 0; }

  /** Populates the list of all subnodes. */
  void populate (void);
  /** Populates the list of group subnodes. */
  void populateGroups (void);
  /** Recursively populates the node hierarchy. */
  void populateAll (void);

  /** Vacates the list of all subnodes. */
  void vacate (void);
  /** Vacates the list of group subnodes. */
  void vacateGroups (void);
  /** Recursively vacates the node hierarchy. */
  void vacateAll (void);

  /** Saves the node data into the store. */
  void save (void);

  /** Adds a new branch node with the given name. */
  void addBranch (const QString& name, KPlayerNode* after = 0);
  /** Adds a new branch node with the given name at the top of the node list. */
  void prependBranch (const QString& name)
    { addBranch (name, this); }
  /** Adds a new branch node with the given name at the end of the node list. */
  void appendBranch (const QString& name);

  /** Adds the given nodes to this node, optionally linking to the origin. */
  void add (const KPlayerNodeList& nodes, bool link = true, KPlayerNode* after = 0);
  /** Adds the given nodes at the top of the node list, linking to the origin. */
  void prepend (const KPlayerNodeList& nodes)
    { add (nodes, true, this); }
  /** Adds the given nodes at the end of the node list, linking to the origin. */
  void append (const KPlayerNodeList& nodes);

  /** Adds the given URL list to this node, linking local directories to the origin. */
  void add (const KUrl::List& urls, bool link = true, KPlayerNode* after = 0);
  /** Adds the given URL list at the top of the node list, linking to the origin. */
  void prepend (const KUrl::List& urls)
    { add (urls, true, this); }
  /** Adds the given URL list at the end of the node list, linking to the origin. */
  void append (const KUrl::List& urls);

  /** Moves the given nodes after the given node. */
  void move (const KPlayerNodeList& nodes, KPlayerNode* after);

  /** Removes the given nodes from this node. */
  void remove (const KPlayerNodeList& nodes);

  /** Replaces the existing nodes with the given ones. */
  void replace (const KPlayerNodeList& list)
  {
    remove (nodes());
    append (list);
  }

  /** Replaces the existing nodes with the given URL list. */
  void replace (const KUrl::List& urls)
  {
    remove (nodes());
    append (urls);
  }

  /** Adds the given nodes to the list of nodes, optionally linking to the origin. */
  KPlayerNode* added (const KPlayerNodeList& nodes, bool link, KPlayerNode* after = 0);
  /** Adds all subnodes of the given node to the list of nodes, optionally linking to the origin. */
  void added (KPlayerContainerNode* node, bool link);
  /** Adds the given files to the list of nodes. */
  void added (const QFileInfoList& list);
  /** Adds the given branches to the list of nodes. */
  void addedBranches (const QStringList& list);
  /** Adds the given leaves to the list of nodes. */
  void addedLeaves (const QStringList& list);

  /** Moves the given nodes after the given node. */
  KPlayerNode* moved (const KPlayerNodeList& nodes, KPlayerNode* after);

  /** Removes the given nodes from the list of nodes. */
  virtual void removed (const KPlayerNodeList& nodes);
  /** Removes the node and all subnodes. */
  virtual void removed (void);
  /** Removes nodes with the given IDs from the list of nodes. */
  void removed (const QStringList& ids);
  /** Finishes removing the given nodes and counts and emits the removed signal. */
  void removed (const KPlayerNodeList& nodes, const KPlayerPropertyCounts& counts);

  /** Removes the given node from the list of nodes. */
  bool remove (KPlayerNode* node);
  /** Detaches the node and its subnodes. */
  void detach (void);

  /** Releases the origin and switches to store source. */
  virtual void releaseOrigin (void);
  /** Detaches the origin and switches to store source recursively. */
  void detachOrigin (void);

  /** Remove the given node from the list of subnodes and release the reference. */
  void release (KPlayerNode* node);
  /** Release a reference and delete the node if no references remain. */
  void release (void)
    { KPlayerNode::release(); }

  /** Returns names and counts of attributes found in subnodes. */
  const KPlayerPropertyCounts& attributeCounts (void) const
    { return m_attribute_counts; }

  /** Updates attribute counts. */
  void updateAttributes (KPlayerNode* node)
  {
    const KPlayerPropertyCounts& added = node -> media() -> added();
    const KPlayerPropertyCounts& removed = node -> media() -> removed();
    m_attribute_counts.add (added);
    m_attribute_counts.subtract (removed);
    if ( ! added.isEmpty() || ! removed.isEmpty() )
      emitAttributesUpdated (added, removed);
    emitUpdated (node);
  }

  /** Emits nodesAdded signal. */
  void emitAdded (const KPlayerNodeList& nodes, KPlayerNode* after = 0)
    { emit nodesAdded (this, nodes, after); }
  /** Emits nodesRemoved signal. */
  void emitRemoved (const KPlayerNodeList& nodes)
    { emit nodesRemoved (this, nodes); }
  /** Emits detached signal. */
  void emitDetached (void)
    { emit detached(); }
  /** Emits nodeUpdated signal. */
  void emitUpdated (KPlayerNode* node)
    { emit nodeUpdated (this, node); }
  /** Emits attributesUpdated signal. */
  void emitAttributesUpdated (const KPlayerPropertyCounts& added, const KPlayerPropertyCounts& removed)
    { emit attributesUpdated (added, removed); }

signals:
  /** Emitted when nodes have been added. */
  void nodesAdded (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after);
  /** Emitted when nodes have been removed. */
  void nodesRemoved (KPlayerContainerNode* parent, const KPlayerNodeList& nodes);
  /** Emitted when the node has been detached. */
  void detached (void);
  /** Emitted when a node has been updated. */
  void nodeUpdated (KPlayerContainerNode*, KPlayerNode*);
  /** Emitted when some node attributes have been updated. */
  void attributesUpdated (const KPlayerPropertyCounts& added, const KPlayerPropertyCounts& removed);

protected:
  /** Initializes the node children. */
  virtual void setupChildren (KPlayerContainerNode* origin);
  /** Initializes the node origin. */
  virtual void setupOrigin (void);
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Creates a new leaf node with the given id. */
  virtual KPlayerNode* createLeaf (const QString& id);
  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);

  /** Inserts the given node and adds a reference if necessary. */
  void insert (KPlayerNode* node, KPlayerNode* after);
  /** Appends the given node to the node list and map. */
  void append (KPlayerNode* node);

  /** Inserts a new leaf subnode and adds a reference if necessary. */
  KPlayerNode* insertLeaf (const QString& id, KPlayerNode* after = 0);
  /** Inserts a new branch subnode and adds references as necessary. */
  KPlayerContainerNode* insertBranch (const QString& id, KPlayerNode* after = 0, KPlayerContainerNode* origin = 0);

  /** Populates the list of all subnodes. */
  void doPopulate (void);
  /** Populates the list of group subnodes. */
  void doPopulateGroups (void);
  /** Refreshes the list of subnodes. */
  void refreshNodes (void);

  /** Returns the last node. */
  KPlayerNode* lastNode (void);

  /** Arranges nodes in the correct order. */
  void applyCustomOrder (void);

  /** List of all subnodes of this node. */
  KPlayerNodeList m_nodes;
  /** Reference count for all nodes. */
  int m_populate_nodes;
  /** Reference count for group nodes. */
  int m_populate_groups;
  /** Origin node, for linked group nodes only. */
  KPlayerContainerNode* m_origin;
  /** Responsible for routing of subnodes into and out of the node. */
  KPlayerSource* m_source;
  /** Maps node ids to pointers for quick lookup. */
  KPlayerNodeMap m_node_map;
  /** Names and counts of subnode attributes. */
  KPlayerPropertyCounts m_attribute_counts;
};

/**Temporary node.
  *@author kiriuja
  */
class KPlayerTemporaryNode : public KPlayerContainerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerTemporaryNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerTemporaryNode();

  /** Creates and returns a temporary item node. */
  KPlayerNode* temporaryItem (const QString& id);
};

/**Root node.
  *@author kiriuja
  */
class KPlayerRootNode : public KPlayerContainerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerRootNode (void);
  /** Destructor. Frees resources. */
  virtual ~KPlayerRootNode();

  /** Node URL. Gives node location in the hierarchy. */
  virtual KUrl url (void) const;

  /** Temporary node. */
  KPlayerTemporaryNode* temporaryNode (void) const
    { return m_temp; }

  /** Returns the list of default IDs. */
  const QStringList& defaultIds (void)
    { return m_default_ids; }

  /** Returns a node from the hierarchy given a URL. */
  KPlayerContainerNode* getNodeByUrl (const KUrl& url);

protected:
  /** Initializes the node children. */
  virtual void setupSource (void);

  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);

  /** List of default IDs. */
  QStringList m_default_ids;
  /** Map of standard nodes. */
  KPlayerContainerNodeMap m_defaults;
  /** External node map. */
  KPlayerContainerNodeMap m_externals;
  /** Temporary node. */
  KPlayerTemporaryNode* m_temp;
};

/**Directory node.
  *@author kiriuja
  */
class KPlayerDirectoryNode : public KPlayerContainerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerDirectoryNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerDirectoryNode();

  /** Returns whether the node is a directory. */
  virtual bool isDirectory (void) const;

  /** Returns whether the node can be renamed. */
  virtual bool canRename (void) const;

  /** Returns whether custom order is allowed. */
  virtual bool allowsCustomOrder (void) const;

protected:
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Creates a new leaf node with the given id. */
  virtual KPlayerNode* createLeaf (const QString& id);
  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);
};

/**External node.
  *@author kiriuja
  */
class KPlayerExternalNode : public KPlayerDirectoryNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerExternalNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerExternalNode();

  /** Node URL. Gives node location in the hierarchy. */
  virtual KUrl url (void) const;
};

/**Group node.
  *@author kiriuja
  */
class KPlayerGroupNode : public KPlayerContainerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerGroupNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerGroupNode();

  /** Returns icon name. */
  virtual QString icon (void) const;
  /** Returns open icon name. */
  virtual QString openIcon (void) const;

  /** Returns whether the node is a group. */
  virtual bool isGroup (void) const;

  /** Returns whether the node can be renamed. */
  virtual bool canRename (void) const;

protected:
  /** Initializes the node source. */
  virtual void setupSource (void);
};

/**Collection node.
  *@author kiriuja
  */
class KPlayerCollectionNode : public KPlayerGroupNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerCollectionNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerCollectionNode();

  /** Returns whether the node is a collection group. */
  virtual bool isCollection (void) const;

  /** Returns whether the given node can be linked into this container. */
  virtual bool canLink (KPlayerContainerNode* node) const;

  /** Returns whether custom order is allowed. */
  virtual bool allowsCustomOrder (void) const;

#if 0
  /** The key that is used to create group nodes. */
  const QString& groupingKey (void) const
    { return media() -> groupingKey(); }
  /** Returns whether the node is grouped. */
  bool isGrouped (void) const
    { return (! origin() || ! origin() -> canGroup()) && ! groupingKey().isEmpty(); }

  /** Parent's grouping key. */
  const QString& parentKey (void) const
    { return parent() -> canGroup() ? ((KPlayerCollectionNode*) parent()) -> groupingKey() : QString::null; }
  /** Returns whether the node is grouped. */
  bool isParentGrouped (void) const
    { return parent() -> canGroup() && ((KPlayerCollectionNode*) parent()) -> isGrouped(); }

  /** Groups the subnodes by key. */
  void group (const QString& key);
  /** Groups the subnodes by folder. */
  void ungroup (void)
    { group (QString::null); }
  /** Flattens the subnode hierarchy. */
  void flatten (void)
    { group (""); }
#endif

protected:
  /** Initializes the node origin. */
  virtual void setupOrigin (void);

  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);
};

/**Playlist node.
  *@author kiriuja
  */
class KPlayerPlaylistNode : public KPlayerGroupNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerPlaylistNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerPlaylistNode();

  /** Returns icon name. */
  virtual QString icon (void) const;
  /** Returns open icon name. */
  virtual QString openIcon (void) const;

  /** Returns whether the given node can be linked into this container. */
  virtual bool canLink (KPlayerContainerNode* node) const;

  /** Indicates whether duplicates are acceptable. */
  virtual bool acceptsDuplicates (void) const;

  /** Indicates whether the node is a playlist. */
  virtual bool isPlaylist (void) const;

  /** Releases the origin and switches to store source. */
  virtual void releaseOrigin (void);

protected slots:
  /** Removes duplicate nodes if needed. */
  virtual void configurationUpdated (void);

  /** Receives updated signal from the origin and updates media if needed. */
  void originUpdated (KPlayerContainerNode*, KPlayerNode* node);

protected:
  /** Initializes the node origin. */
  virtual void setupOrigin (void);
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);

  /** Duplicate indicator. */
  bool m_duplicates;
};

/**Now playing node.
  *@author kiriuja
  */
class KPlayerNowPlayingNode : public KPlayerPlaylistNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerNowPlayingNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerNowPlayingNode();

  /** Returns icon name. */
  virtual QString icon (void) const;
  /** Returns open icon name. */
  virtual QString openIcon (void) const;

  /** Returns whether the node is a now playing group. */
  virtual bool isNowPlaying (void) const;

  /** Returns whether the given node can be linked into this container. */
  virtual bool canLink (KPlayerContainerNode* node) const;

protected slots:
  /** Receives updated signal from the origin and updates media if needed. */
  void originUpdated (KPlayerContainerNode*, KPlayerNode* node);

protected:
  /** Initializes the node origin. */
  virtual void setupOrigin (void);

  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);
};

/**Recent node.
  *@author kiriuja
  */
class KPlayerRecentNode : public KPlayerPlaylistNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerRecentNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerRecentNode();

  /** Returns icon name. */
  virtual QString icon (void) const;
  /** Returns open icon name. */
  virtual QString openIcon (void) const;

  /** Indicates whether the node is a recent node. */
  virtual bool isRecent (void) const;

  /** Returns whether the given node can be linked into this container. */
  virtual bool canLink (KPlayerContainerNode* node) const;

protected:
  /** Creates a new branch node with the given id. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);
};

/**Recents node.
  *@author kiriuja
  */
class KPlayerRecentsNode : public KPlayerRecentNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerRecentsNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerRecentsNode();

  /** Adds the given nodes to this node as a single node, linking to the origin. */
  void addRecent (const KPlayerNodeList& nodes);

protected:
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Removes excess nodes. */
  virtual void configurationUpdated (void);
};

/**Devices node.
  *@author kiriuja
  */
class KPlayerDevicesNode : public KPlayerContainerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerDevicesNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerDevicesNode();

  /** Returns icon name. */
  virtual QString icon (void) const;
  /** Returns open icon name. */
  virtual QString openIcon (void) const;

  /** Indicates whether the node is the device list. */
  virtual bool isDevices (void) const;
  /** Returns whether the node can be renamed. */
  virtual bool canRename (void) const;
  /** Returns whether the node can be saved as a playlist. */
  virtual bool canSaveAsPlaylist (void) const;

  /** Returns a node for the given ID from the node list. */
  KPlayerDeviceNode* nodeById (const QString& id)
    { return (KPlayerDeviceNode*) KPlayerContainerNode::nodeById (id); }
  /** Returns a node at the given path if it exists. */
  virtual KPlayerContainerNode* getNodeByPath (const QString& path);

  /** Source of subnodes. */
  KPlayerDevicesSource* source (void) const
    { return (KPlayerDevicesSource*) m_source; }

  /** Returns the list of device paths. */
  const QStringList& devices (void)
    { return m_devices; }

  /** Returns the type of the given device. */
  const QString& deviceType (const QString& id)
    { return m_type_map [id]; }

  /** Returns the suggested name of the given device. */
  const QString& deviceName (const QString& id)
    { return m_name_map [id]; }
  /** Returns the UDI of the given device. */
  const QString& deviceUdi (const QString& id)
    { return m_device_udi [id]; }

  /** Updates the list of devices. */
  void update (void);

  /** Removes the given nodes from the list of nodes. */
  virtual void removed (const KPlayerNodeList& nodes);

protected slots:
  /** Receives notification of an added device. */
  void deviceAdded (const QString& udi);
  /** Receives notification of a removed device. */
  void deviceRemoved (const QString& udi);

protected:
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);

  /** Updates the list of devices. */
  void update (QStringList& current, QStringList& previous);

  void updateDevice (const QString& udi, const QString& path, const char* type,
    QStringList& paths, QStringList& current, QStringList& previous);
  void updateDeviceNames (QStringList& paths, const char* type);

  /** Device paths. */
  QStringList m_devices;
  /** Device types. */
  QMap<QString, QString> m_type_map;
  /** Device names. */
  QMap<QString, QString> m_name_map;
  /** Device UDI. */
  QMap<QString, QString> m_device_udi;
};

/**Device node.
  *@author kiriuja
  */
class KPlayerDeviceNode : public KPlayerContainerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerDeviceNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerDeviceNode();

  /** Parent node. */
  KPlayerDevicesNode* parent (void) const
    { return (KPlayerDevicesNode*) m_parent; }
  /** Source of subnodes. */
  KPlayerDeviceSource* source (void) const
    { return (KPlayerDeviceSource*) m_source; }
  /** Media properties. */
  KPlayerDeviceProperties* media (void) const
    { return (KPlayerDeviceProperties*) m_media; }

  /** Returns icon name. */
  virtual QString icon (void) const;
  /** Returns open icon name. */
  virtual QString openIcon (void) const;

  /** Returns whether the node has media properties. */
  virtual bool hasProperties (void) const;

  /** Returns whether custom order is allowed. */
  virtual bool allowsCustomOrder (void) const;

  /** Returns whether the device is a disk device. */
  virtual bool diskDevice (void);

  /** Removes the node and all subnodes. */
  virtual void removed (void);

  /** Returns solid device. */
  Solid::Device& solidDevice (void)
    { return m_solid_device; }

protected:
  /** Solid device. */
  Solid::Device m_solid_device;
};

/**Disk node.
  *@author kiriuja
  */
class KPlayerDiskNode : public KPlayerDeviceNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerDiskNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerDiskNode();

  /** Returns whether the node is ready for playing. */
  virtual bool ready (void) const;

  /** Suggested identifier for a copy of this node. */
  virtual QString suggestId (void) const;

  /** Source of subnodes. */
  KPlayerDiskSource* source (void) const
    { return (KPlayerDiskSource*) m_source; }

  /** Returns the device properties. */
  KPlayerDeviceProperties* device (void) const
    { return m_device; }
  /** Returns the disk properties. */
  KPlayerDiskProperties* disk (void) const
    { return m_disk; }

  /** Returns icon name. */
  virtual QString icon (void) const;

  /** Returns the number of tracks and starts autodetection if needed. */
  int tracks (void);
  /** Starts autodetection unconditionally. */
  void loadDisk (void);

  /** Returns the local path if known. */
  const QString& localPath (void) const
    { return m_local_path; }
  /** Returns whether the local path is known. */
  bool hasLocalPath (void) const
    { return ! m_local_path.isEmpty(); }
  /** Starts the process of local path acquisition. */
  void getLocalPath (void);

  /** Updates the node with the disk properties. */
  void diskInserted (const char* type, const QString& path, const QString& udi);
  /** Updates the node with the device properties. */
  void diskRemoved (void);

  /** Returns whether the device is a disk device. */
  virtual bool diskDevice (void);
  /** Returns whether the disk is of a media type. */
  bool mediaDisk (void);
  /** Returns whether the disk is of a data type. */
  bool dataDisk (void);

  /** Returns solid device. */
  Solid::Device& solidDisk (void)
    { return m_solid_disk; }

protected slots:
  /** Receives notification of an added device. */
  void deviceAdded (const QString& udi);
  /** Receives notification of a removed device. */
  void deviceRemoved (const QString& udi);

  /** Processes mount completion notification. */
  void mountDone (Solid::ErrorType error, QVariant errorData, const QString &udi);

  /** Processes an MPlayer output line. */
  void receivedOutput (KPlayerLineOutputProcess*, char*);
  /** Finishes refreshing lists. */
  void processFinished (KPlayerLineOutputProcess*);

  /** Processes an MPlayer output line. */
  void receivedCddbOutput (KPlayerLineOutputProcess*, char*);
  /** Deletes the process. */
  void cddbProcessFinished (KPlayerLineOutputProcess*);

protected:
  /** Initializes the node media. */
  virtual void setupMedia (void);
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Creates a new leaf node with the given id. */
  virtual KPlayerNode* createLeaf (const QString& id);

  /** Checks Solid list of devices for disk in this device. */
  void checkDisk (void);
  /** Starts disk autodetection. */
  void autodetect (void);
  /** Wraps up autodetection. */
  void autodetected (void);
  /** Updates track nodes. */
  void updateTracks (void);

  /** Identifies the disk and returns its properties. */
  bool accessDisk (void);
  /** Updates the node with the disk properties. */
  void diskDetected (const QString& diskid);

  /** Sets the disk type and updates the default name. */
  void setDiskType (const QString& type);

  /** Disk properties. */
  KPlayerDeviceProperties* m_device;
  /** Disk properties. */
  KPlayerDiskProperties* m_disk;
  /** Track lengths. */
  QList<float> m_track_lengths;
  /** URL currently being autodetected. */
  QString m_url;
  /** Audio CD disk ID. */
  QString m_disk_id;
  /** Audio CD disk ID for matching against the one above. */
  QString m_cddb_id;
  /** Number of detected tracks. */
  int m_detected_tracks;
  /** Indicates autodetection success. */
  bool m_autodetected;
  /** Artist. */
  QString m_artist;
  /** Album. */
  QString m_album;
  /** Year. */
  int m_year;
  /** Genre. */
  QString m_genre;
  /** Fast autodetect when type is known from media slave. */
  bool m_fast_autodetect;
  /** Local path. */
  QString m_local_path;
  /** Solid disk. */
  Solid::Device m_solid_disk;
};

/**Tuner node.
  *@author kiriuja
  */
class KPlayerTunerNode : public KPlayerDeviceNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerTunerNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerTunerNode();

  /** Source of subnodes. */
  KPlayerTunerSource* source (void) const
    { return (KPlayerTunerSource*) m_source; }
  /** Media properties. */
  KPlayerTunerProperties* media (void) const
    { return (KPlayerTunerProperties*) m_media; }

protected slots:
  /** Checks if the channel list has changed and updates subnodes as necessary. */
  virtual void updated (void);

protected:
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Creates a new leaf node with the given id. */
  virtual KPlayerNode* createLeaf (const QString& id);

  /** Current channel list. */
  QString m_channel_list;
};

/**TV node.
  *@author kiriuja
  */
class KPlayerTVNode : public KPlayerTunerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerTVNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerTVNode();

  /** Media properties. */
  KPlayerTVProperties* media (void) const
    { return (KPlayerTVProperties*) m_media; }

protected:
  /** Initializes the node media. */
  virtual void setupMedia (void);
};

/**DVB node.
  *@author kiriuja
  */
class KPlayerDVBNode : public KPlayerTunerNode
{
  Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerDVBNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerDVBNode();

  /** Media properties. */
  KPlayerDVBProperties* media (void) const
    { return (KPlayerDVBProperties*) m_media; }

protected:
  /** Initializes the node media. */
  virtual void setupMedia (void);
};

#if 0
/**Searches node.
  *@author kiriuja
  */
class KPlayerSearchesNode : public KPlayerContainerNode
{
  //Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerSearchesNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerSearchesNode();

  /** Returns whether the node can be renamed. */
  virtual bool canRename (void) const;

protected:
  /** Creates a new branch node with the given id and origin. */
  virtual KPlayerContainerNode* createBranch (const QString& id, KPlayerContainerNode* origin = 0);
};

/**Search node.
  *@author kiriuja
  */
class KPlayerSearchNode : public KPlayerGroupNode
{
  //Q_OBJECT

public:
  /** Default constructor. Initializes the node. */
  KPlayerSearchNode (void) { }
  /** Destructor. Frees resources. */
  virtual ~KPlayerSearchNode();

  /** Returns whether custom order is allowed. */
  virtual bool allowsCustomOrder (void) const;

protected:
  /** Initializes the node source. */
  virtual void setupSource (void);

  /** Creates a new leaf node with the given id. */
  virtual KPlayerNode* createLeaf (const QString& id);
};
#endif

#endif
