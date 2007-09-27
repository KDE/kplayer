/***************************************************************************
                          kplayersource.h
                          ---------------
    begin                : Thu Jan 12 2006
    copyright            : (C) 2006-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERSOURCE_H
#define KPLAYERSOURCE_H

#include <kdirwatch.h>
#include <qdir.h>
#include <qlist.h>

class KPlayerCollectionNode;
class KPlayerContainerNode;
class KPlayerRootNode;
class KPlayerDeviceProperties;
class KPlayerDevicesNode;
class KPlayerDeviceNode;
class KPlayerDiskNode;
class KPlayerTVNode;
class KPlayerDVBNode;
class KPlayerNode;
class KPlayerNodeList;

/**Source class, responsible for routing of subnodes into and out of the node.
  *@author kiriuja
  */
class KPlayerSource : public QObject
{
  Q_OBJECT

public:
  /** The KPlayerSource constructor. Sets up a source. */
  KPlayerSource (KPlayerContainerNode*);
  /** The KPlayerSource destructor. Frees resources. */
  virtual ~KPlayerSource();

  /** Connects the source. */
  virtual void connectOrigin (void);

  /** Parent node. */
  KPlayerContainerNode* parent (void) const
    { return m_parent; }

  /** Returns whether there are child items or groups. */
  bool has (bool groups);
  /** Initializes the node retrieval process. */
  void start (bool groups);
  /** Retrieves the id of the next node. */
  bool next (bool& group, QString& id);
  /** Verifies the given id. */
  virtual bool verify (const QString& id);

  /** Saves the node data into the source. */
  virtual void save (void);

  /** Indicates whether leaves can be added. */
  virtual bool canAddLeaves (void) const;
  /** Indicates whether branches can be added. */
  virtual bool canAddBranches (void) const;
  /** Indicates whether nodes can be removed. */
  virtual bool canRemove (void) const;

  /** Adds the given nodes into the list of nodes, optionally linking to the origin. */
  virtual KPlayerNode* add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after = 0);
  /** Removes the given nodes from the list of nodes. */
  virtual void remove (const KPlayerNodeList& nodes);

public slots:
  /** Adds the given nodes into the list of nodes. */
  virtual void added (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after);
  /** Removes the given nodes from the list of nodes. */
  virtual void removed (KPlayerContainerNode* parent, const KPlayerNodeList& nodes);

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups) = 0;
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id) = 0;

  /** Connects the given node and all subnodes. */
  void connectNodes (KPlayerContainerNode* node);
  /** Searches for a node with the given ID in the given node and its subnodes. */
  bool find (KPlayerContainerNode* node, const QString& id);

  /** Parent node. */
  KPlayerContainerNode* m_parent;
  /** Parent node iterator. */
  QList<KPlayerNode*>::ConstIterator m_iterator;
  /** Flag stored by start() for next(). */
  bool m_groups;
  /** Flag indicating iteration over parent nodes. */
  bool m_iterating;

signals:
  /** Emitted when nodes have been added. */
  void nodesAdded (const KPlayerNodeList&);
  /** Emitted when nodes have been removed. */
  void nodesRemoved (const KPlayerNodeList&);
};

/**List source class, handles subnodes of some top level nodes.
  *@author kiriuja
  */
class KPlayerListSource : public KPlayerSource
{
  Q_OBJECT

public:
  /** The KPlayerListSource constructor. Sets up a source. */
  KPlayerListSource (KPlayerContainerNode*);
  /** The KPlayerListSource destructor. Frees resources. */
  virtual ~KPlayerListSource();

  /** Saves the node data into the source. */
  virtual void save (void);

  /** Removes the given nodes from the list of nodes. */
  virtual void remove (const KPlayerNodeList& nodes);

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);

  /** Current iterator. */
  QStringList::ConstIterator m_iterator;
  /** End iterator. */
  QStringList::ConstIterator m_end;
};

/**Fixed source class, handles subnodes of a fixed node.
  *@author kiriuja
  */
class KPlayerFixedSource : public KPlayerSource
{
  Q_OBJECT

public:
  /** The KPlayerFixedSource constructor. Sets up a source. */
  KPlayerFixedSource (KPlayerContainerNode*);
  /** The KPlayerFixedSource destructor. Frees resources. */
  virtual ~KPlayerFixedSource();

  /** Indicates whether leaves can be added. */
  virtual bool canAddLeaves (void) const;
  /** Indicates whether branches can be added. */
  virtual bool canAddBranches (void) const;
  /** Indicates whether nodes can be removed. */
  virtual bool canRemove (void) const;

protected:
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);

  /** List of fixed IDs. */
  QStringList m_fixed_ids;
};

/**Root source class, handles subnodes of the root node.
  *@author kiriuja
  */
class KPlayerRootSource : public KPlayerFixedSource
{
  Q_OBJECT

public:
  /** The KPlayerRootSource constructor. Sets up a source. */
  KPlayerRootSource (KPlayerContainerNode*);
  /** The KPlayerRootSource destructor. Frees resources. */
  virtual ~KPlayerRootSource();

  /** Parent node. */
  KPlayerRootNode* parent (void) const
    { return (KPlayerRootNode*) m_parent; }

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
};

/**Devices source class, handles the devices node.
  *@author kiriuja
  */
class KPlayerDevicesSource : public KPlayerListSource
{
  Q_OBJECT

public:
  /** The KPlayerDevicesSource constructor. Sets up a source. */
  KPlayerDevicesSource (KPlayerContainerNode*);
  /** The KPlayerDevicesSource destructor. Frees resources. */
  virtual ~KPlayerDevicesSource();

  /** Parent node. */
  KPlayerDevicesNode* parent (void) const
    { return (KPlayerDevicesNode*) m_parent; }

  /** Indicates whether leaves can be added. */
  virtual bool canAddLeaves (void) const;

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);

  /** List of pending devices. */
  QStringList m_pending;
};

/**Device source class, handles device nodes.
  *@author kiriuja
  */
class KPlayerDeviceSource : public KPlayerSource
{
  Q_OBJECT

public:
  /** The KPlayerDeviceSource constructor. Sets up a source. */
  KPlayerDeviceSource (KPlayerContainerNode*);
  /** The KPlayerDeviceSource destructor. Frees resources. */
  virtual ~KPlayerDeviceSource();

  /** Indicates whether branches can be added. */
  virtual bool canAddBranches (void) const;
  /** Indicates whether leaves can be added. */
  virtual bool canAddLeaves (void) const;
  /** Indicates whether nodes can be removed. */
  virtual bool canRemove (void) const;
};

/**Disk source class, handles disk device nodes.
  *@author kiriuja
  */
class KPlayerDiskSource : public KPlayerDeviceSource
{
  Q_OBJECT

public:
  /** The KPlayerDiskSource constructor. Sets up a source. */
  KPlayerDiskSource (KPlayerContainerNode*);
  /** The KPlayerDiskSource destructor. Frees resources. */
  virtual ~KPlayerDiskSource();

  /** Parent node. */
  KPlayerDiskNode* parent (void) const
    { return (KPlayerDiskNode*) m_parent; }

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);

  /** Number of tracks. */
  int m_tracks;
  /** Current track. */
  int m_track;
};

/**Tuner source class, handles tuner device nodes.
  *@author kiriuja
  */
class KPlayerTunerSource : public KPlayerDeviceSource
{
  Q_OBJECT

public:
  /** The KPlayerTunerSource constructor. Sets up a source. */
  KPlayerTunerSource (KPlayerContainerNode*);
  /** The KPlayerTunerSource destructor. Frees resources. */
  virtual ~KPlayerTunerSource();

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);

  /** List of pending devices. */
  QStringList m_pending;
  /** List source. */
  KPlayerListSource m_list;
};

/**Store source class, handles subnodes in internal storage.
  *@author kiriuja
  */
class KPlayerStoreSource : public KPlayerListSource
{
  Q_OBJECT

public:
  /** The KPlayerStoreSource constructor. Sets up a source. */
  KPlayerStoreSource (KPlayerContainerNode*);
  /** The KPlayerStoreSource destructor. Frees resources. */
  virtual ~KPlayerStoreSource();

  /** Adds the given nodes into the list of nodes, optionally linking to the origin. */
  virtual KPlayerNode* add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after = 0);

protected:
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);
};

/**Directory source class, handles subnodes in filesystem directory.
  *@author kiriuja
  */
class KPlayerDirectorySource : public KPlayerSource
{
  Q_OBJECT

public:
  /** The KPlayerDirectorySource constructor. Sets up a source. */
  KPlayerDirectorySource (KPlayerContainerNode*);
  /** The KPlayerDirectorySource destructor. Frees resources. */
  virtual ~KPlayerDirectorySource();

  /** Indicates whether leaves can be added. */
  virtual bool canAddLeaves (void) const;
  /** Indicates whether branches can be added. */
  virtual bool canAddBranches (void) const;
  /** Indicates whether nodes can be removed. */
  virtual bool canRemove (void) const;

  /** Verifies the given id. */
  virtual bool verify (const QString& id);

protected slots:
  /** Receives dirty signal from directory watch. */
  void dirty (const QString& path);

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);

  /** Directory. */
  QDir m_directory;
  /** File info list. */
  QFileInfoList m_list;
  /** Entry info iterator. */
  QFileInfoList::ConstIterator m_iterator;
  /** Directory watch. */
  KDirWatch m_watch;
};

/**Origin source class, handles subnodes of a linked node.
  *@author kiriuja
  */
class KPlayerOriginSource : public KPlayerSource
{
  Q_OBJECT

public:
  /** The KPlayerOriginSource constructor. Sets up a source. */
  KPlayerOriginSource (KPlayerContainerNode* parent);
  /** The KPlayerOriginSource destructor. Frees resources. */
  virtual ~KPlayerOriginSource();

  /** Connects the source. */
  virtual void connectOrigin (void);

  /** Adds the given nodes into the list of nodes, optionally linking to the origin. */
  virtual KPlayerNode* add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after = 0);
  /** Removes the given nodes from the list of nodes. */
  virtual void remove (const KPlayerNodeList& nodes);

  /** Saves the node data into the source. */
  virtual void save (void);

public slots:
  /** Adds the given nodes into the list of nodes. */
  virtual void added (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after);
  /** Removes the given nodes from the list of nodes. */
  virtual void removed (KPlayerContainerNode* parent, const KPlayerNodeList& nodes);
  /** Detaches from the origin. */
  virtual void detach (void);

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);
};

#if 0
/**Key source class, handles subnodes of a linked node.
  *@author kiriuja
  */
class KPlayerKeySource : public KPlayerSource
{
  //Q_OBJECT

public:
  /** The KPlayerKeySource constructor. Sets up a source. */
  KPlayerKeySource (KPlayerCollectionNode* parent);
  /** The KPlayerKeySource destructor. Frees resources. */
  virtual ~KPlayerKeySource();

  /** Connects the source. */
  virtual void connectOrigin (void);

  /** Parent node. */
  KPlayerCollectionNode* parent (void) const
    { return (KPlayerCollectionNode*) m_parent; }

  /** Matches the meta information for the given ID against the criteria. */
  bool match (const QString& id);

  /** Adds the given nodes into the list of nodes, optionally linking to the origin. */
  virtual KPlayerNode* add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after = 0);
  /** Removes the given nodes from the list of nodes. */
  virtual void remove (const KPlayerNodeList& nodes);

  /** Sets the node meta information so that it matches the criteria. */
  void brand (const KPlayerNodeList& nodes);

//public slots:
  /** Adds the given nodes into the list of nodes. */
  void added (const KPlayerNodeList& nodes);
  /** Removes the given nodes from the list of nodes. */
  void removed (const KPlayerNodeList& nodes);

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);
};

/**Group source class, handles subnodes of a grouped node.
  *@author kiriuja
  */
class KPlayerGroupSource : public KPlayerSource
{
  //Q_OBJECT

public:
  /** The KPlayerGroupSource constructor. Sets up a source. */
  KPlayerGroupSource (KPlayerCollectionNode* parent);
  /** The KPlayerGroupSource destructor. Frees resources. */
  virtual ~KPlayerGroupSource();

  /** Connects the source. */
  virtual void connectOrigin (void);

  /** Parent node. */
  KPlayerCollectionNode* parent (void) const
    { return (KPlayerCollectionNode*) m_parent; }

  /** Adds the given nodes into the list of nodes, optionally linking to the origin. */
  virtual KPlayerNode* add (const KPlayerNodeList& nodes, bool link, KPlayerNode* after = 0);
  /** Removes the given nodes from the list of nodes. */
  virtual void remove (const KPlayerNodeList& nodes);

//public slots:
  /** Adds the given nodes into the list of nodes. */
  virtual void added (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after);

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);
  /** Removes the given nodes from the given node and its subnodes. */
  void remove (KPlayerContainerNode* node, const KPlayerNodeList& nodes);

  /** Current node. */
  KPlayerContainerNode* m_current;
};

/**Searches source class, handles subnodes of the searches node.
  *@author kiriuja
  */
class KPlayerSearchesSource : public KPlayerListSource
{
  //Q_OBJECT

public:
  /** The KPlayerSearchesSource constructor. Sets up a source. */
  KPlayerSearchesSource (KPlayerContainerNode* parent);
  /** The KPlayerSearchesSource destructor. Frees resources. */
  virtual ~KPlayerSearchesSource();

  /** Indicates whether leaves can be added. */
  virtual bool canAddLeaves (void) const;
  /** Indicates whether branches can be added. */
  virtual bool canAddBranches (void) const;
};

/**Search source class, handles subnodes of a search node.
  *@author kiriuja
  */
class KPlayerSearchSource : public KPlayerSource
{
  //Q_OBJECT

public:
  /** The KPlayerSearchSource constructor. Sets up a source. */
  KPlayerSearchSource (KPlayerContainerNode* parent);
  /** The KPlayerSearchSource destructor. Frees resources. */
  virtual ~KPlayerSearchSource();

  /** Connects the source. */
  virtual void connectOrigin (void);

  /** Indicates whether leaves can be added. */
  virtual bool canAddLeaves (void) const;
  /** Indicates whether branches can be added. */
  virtual bool canAddBranches (void) const;
  /** Indicates whether nodes can be removed. */
  virtual bool canRemove (void) const;

  /** Matches the meta information for the given ID against the criteria. */
  bool match (const QString& id);

//public slots:
  /** Adds the given nodes into the list of nodes. */
  virtual void added (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after);
  /** Detaches from the origin. */
  virtual void detach (void);

protected:
  /** Initializes the node retrieval process. */
  virtual void enumStart (bool groups);
  /** Retrieves the id of the next node. */
  virtual bool enumNext (bool& group, QString& id);

  /** Current node. */
  KPlayerContainerNode* m_current;
};
#endif

#endif
