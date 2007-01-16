/***************************************************************************
                          kplayernodeview.cpp
                          --------------------
    begin                : Mon Apr 18 2005
    copyright            : (C) 2005-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurldrag.h>
#include <qcursor.h>
#include <qheader.h>
#include <qpopupmenu.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_NODEVIEW
#endif

#include "kplayernodeview.h"
#include "kplayernodeview.moc"
#include "kplayernodeaction.h"
#include "kplayerplaylist.h"
#include "kplayerpropertiesdialog.h"
#include "kplayersettings.h"
#include "kplayer.h"

KPlayerNodeNameValidator::KPlayerNodeNameValidator (KPlayerContainerNode* node, QObject* parent, const char* name)
  : QValidator (parent, name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Creating node name validator\n";
  kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  m_node = node;
  node -> reference();
  node -> populateGroups();
}

KPlayerNodeNameValidator::~KPlayerNodeNameValidator()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying node name validator\n";
  kdDebugTime() << " URL    " << m_node -> url().url() << "\n";
#endif
  m_node -> vacateGroups();
  m_node -> release();
}

QValidator::State KPlayerNodeNameValidator::validate (QString& input, int&) const
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeNameValidator::validate\n";
  kdDebugTime() << " Input  " << input << "\n";
#endif
  if ( ! input.isEmpty() && input.find ('/') < 0 )
  {
    KPlayerNode* node = m_node -> nodeById (input);
#ifdef DEBUG_KPLAYER_NODEVIEW
    if ( node )
      kdDebugTime() << " Node   " << node -> url().url() << " container " << node -> isContainer() << "\n";
#endif
    if ( ! node || ! node -> isContainer() )
      return QValidator::Acceptable;
  }
  return QValidator::Intermediate;
}

KPlayerPropertiesDevice::KPlayerPropertiesDevice (QWidget* parent, const char* name)
  : KPlayerPropertiesDevicePage (parent, name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Creating device properties page\n";
#endif
  m_node = KPlayerNode::root() -> getNodeByUrl ("kplayer:/devices");
  m_node -> reference();
  m_node -> populateGroups();
  pathChanged (c_path -> text());
}

KPlayerPropertiesDevice::~KPlayerPropertiesDevice()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying device properties page\n";
#endif
  m_node -> vacateGroups();
  m_node -> release();
}

void KPlayerPropertiesDevice::pathChanged (const QString& path)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPropertiesDevice::pathChanged\n";
  kdDebugTime() << " Path   " << path << "\n";
#endif
  QString fullpath (path), slash ("/");
  if ( ! path.startsWith (slash) )
    fullpath.prepend (slash);
  bool enable = ! c_name -> text().isEmpty() && fullpath != slash && ! m_node -> nodeById (fullpath);
  parent() -> enableButton (KDialogBase::Ok, enable);
}

void KPlayerPropertiesDevice::addDevice (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPropertiesDevice::addDevice\n";
#endif
  QString fullpath (c_path -> text()), slash ("/");
  if ( ! fullpath.startsWith (slash) )
    fullpath.prepend (slash);
  KPlayerDeviceProperties* media = KPlayerMedia::deviceProperties (m_node -> url (fullpath));
  media -> setName (c_name -> text());
  int index = c_type -> currentItem();
  media -> setType (index == 1 ? "DVD" : index == 2 ? "TV" : index == 3 ? "DVB" : "CD");
  media -> commit();
  QStringList list;
  list.append (fullpath);
  m_node -> addedBranches (list);
  KPlayerMedia::release (media);
}

KPlayerDeviceDialog::KPlayerDeviceDialog (QWidget* parent, const char* name)
  : KDialogBase (parent, name, true, i18n("Device Properties"), Help | Ok | Cancel)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Creating device dialog\n";
#endif
  setMainWidget (new KPlayerPropertiesDevice (this));
}

KPlayerDeviceDialog::~KPlayerDeviceDialog (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying device dialog\n";
#endif
}

void KPlayerListViewItem::initialize (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Initializing item\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  node() -> reference();
  update();
}

int KPlayerListViewItem::compare (QListViewItem* item, int, bool ascending) const
{
  int result = node() -> compare (nodeForItem (item));
  return ascending ? result : - result;
}

KPlayerListViewItem* KPlayerListViewItem::itemForNode (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListViewItem::itemForNode\n";
#endif
  if ( ! node )
    return 0;
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " URL    " << node -> url() << "\n";
#endif
  KPlayerListViewItem* item = (KPlayerListViewItem*) firstChild();
  while ( item )
  {
    if ( item -> node() == node )
      return item;
    item = (KPlayerListViewItem*) item -> nextSibling();
  }
  return 0;
}

QString KPlayerListViewItem::icon (void) const
{
  return node() -> icon();
}

bool KPlayerListViewItem::hasChildren (void)
{
  return false;
}

void KPlayerListViewItem::resetCustomOrder (void)
{
}

void KPlayerListViewItem::update (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListViewItem::update\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  setPixmap (0, DesktopIcon (icon(), KIcon::SizeSmall));
  QStringList::ConstIterator iterator (nodeView() -> attributeNames().begin());
  for ( int i = 0; iterator != nodeView() -> attributeNames().end(); ++ iterator, ++ i )
  {
    QString text (node() -> media() -> asString (*iterator));
    setText (nodeView() -> header() -> mapToSection (i), text);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " " << *iterator << " " << text << "\n";
#endif
  }
}

void KPlayerListViewItem::update (const QString& name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListViewItem::update\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  int index = nodeView() -> attributeNames().findIndex (name);
  if ( index > 0 )
  {
    QString text (node() -> media() -> asString (name));
    setText (nodeView() -> header() -> mapToSection (index), text);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " " << name << " " << text << "\n";
#endif
  }
}

void KPlayerListViewItem::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Terminating item\n";
  kdDebugTime() << " URL    " << node() -> url() << "\n";
#endif
  nodeView() -> itemTerminating (this);
  delete this;
}

KPlayerListViewItem::~KPlayerListViewItem()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying list view item\n";
#endif
  node() -> release();
}

void KPlayerListViewGroupItem::initialize (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Initializing group item\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  KPlayerListViewItem::initialize();
}

KPlayerListViewGroupItem::~KPlayerListViewGroupItem()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying list view group item\n";
#endif
}

void KPlayerListViewFolderItem::initialize (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Initializing folder item\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  KPlayerListViewGroupItem::initialize();
  connectNode();
}

void KPlayerListViewFolderItem::connectNodeCommon (void)
{
  QObject::connect (node(), SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    nodeView(), SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  QObject::connect (node(), SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    nodeView(), SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  QObject::connect (node(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
    nodeView(), SLOT (updated (KPlayerContainerNode*, KPlayerNode*)));
}

void KPlayerListViewFolderItem::connectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Connecting list view folder item node\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  connectNodeCommon();
  if ( isOpen() )
    QObject::connect (node(), SIGNAL (attributesUpdated (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)),
      listView(), SLOT (updateAttributes (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)));
}

void KPlayerListViewFolderItem::disconnectNodeCommon (void)
{
  QObject::disconnect (node(), SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    nodeView(), SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  QObject::disconnect (node(), SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    nodeView(), SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  QObject::disconnect (node(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
    nodeView(), SLOT (updated (KPlayerContainerNode*, KPlayerNode*)));
}

void KPlayerListViewFolderItem::disconnectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Disconnecting list view folder item node\n";
  kdDebugTime() << " URL    " << node() -> url() << "\n";
#endif
  disconnectNodeCommon();
  QObject::disconnect (node(), SIGNAL (attributesUpdated (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)),
    listView(), SLOT (updateAttributes (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)));
}

void KPlayerListViewFolderItem::populateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Populating list view folder item node\n";
#endif
  node() -> populate();
  listView() -> updateAttributes (node() -> attributeCounts(), KPlayerPropertyCounts());
  QObject::connect (node(), SIGNAL (attributesUpdated (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)),
    listView(), SLOT (updateAttributes (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)));
}

void KPlayerListViewFolderItem::vacateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Vacating list view folder item node\n";
#endif
  QObject::disconnect (node(), SIGNAL (attributesUpdated (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)),
    listView(), SLOT (updateAttributes (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)));
  listView() -> updateAttributes (KPlayerPropertyCounts(), node() -> attributeCounts());
  node() -> vacate();
}

void KPlayerListViewFolderItem::updateColumns (void)
{
  listView() -> loadColumnWidths();
}

void KPlayerListViewFolderItem::resetCustomOrder (void)
{
  if ( isOpen() && childCount() )
  {
    node() -> setCustomOrder (false);
    for ( QListViewItem* item = firstChild(); item; item = item -> nextSibling() )
      ((KPlayerListViewItem*) item) -> resetCustomOrder();
  }
}

void KPlayerListViewFolderItem::setOpen (bool open)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListViewFolderItem::setOpen " << open << "\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  bool change = open != isOpen();
  if ( change )
  {
    terminateChildren();
    if ( open )
      populateNode();
    else
      vacateNode();
  }
  KPlayerListViewGroupItem::setOpen (open);
  if ( change )
  {
    setPixmap (0, DesktopIcon (icon(), KIcon::SizeSmall));
    if ( open )
      added (node() -> nodes());
    updateColumns();
  }
}

QString KPlayerListViewFolderItem::icon (void) const
{
  return isOpen() ? node() -> openIcon() : node() -> icon();
}

bool KPlayerListViewFolderItem::hasChildren (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListViewFolderItem::hasChildren\n";
#endif
  return node() -> hasNodes();
}

KPlayerListViewItem* KPlayerListViewFolderItem::createChild (KPlayerListViewItem* after, KPlayerNode* node)
{
  return node -> isContainer() ? (KPlayerListViewItem*) new KPlayerListViewFolderItem (this, after, node)
    : new KPlayerListViewItem (this, after, node);
}

void KPlayerListViewFolderItem::added (const KPlayerNodeList& nodes, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListViewFolderItem::added\n";
#endif
  if ( isOpen() )
  {
    KPlayerListViewItem* item = after == node() ? 0 : itemForNode (after);
    KPlayerNodeListIterator iterator (nodes);
    while ( KPlayerNode* node = iterator.current() )
    {
      item = createChild (item, node);
      if ( item )
      {
        item -> initialize();
        item -> setExpandable (item -> hasChildren());
        if ( nodeView() -> moving() )
          item -> setSelected (true);
      }
      ++ iterator;
    }
    setExpandable (childCount() > 0);
  }
  else
  {
    bool was_empty = ! parent() && ! isExpandable();
    setExpandable (hasChildren());
    if ( was_empty && isExpandable() && listView() -> childCount() == 1 )
      listView() -> setOpen (this, true);
  }
  repaint();
}

void KPlayerListViewFolderItem::removed (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListViewFolderItem::removed\n";
#endif
  if ( isOpen() )
  {
    KPlayerNodeListIterator iterator (nodes);
    while ( KPlayerNode* node = iterator.current() )
    {
      KPlayerListViewItem* item = itemForNode (node);
      if ( item )
      {
        nodeView() -> keepUpCurrentItem (this, item);
        item -> terminate();
      }
      ++ iterator;
    }
    setExpandable (childCount() > 0);
  }
  else
    setExpandable (hasChildren());
  repaint();
}

void KPlayerListViewFolderItem::terminateChildren (void)
{
  while ( KPlayerListViewItem* item = (KPlayerListViewItem*) firstChild() )
    item -> terminate();
}

void KPlayerListViewFolderItem::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Terminating folder item\n";
  kdDebugTime() << " URL    " << node() -> url() << "\n";
#endif
  disconnectNode();
  if ( isOpen() )
  {
    terminateChildren();
    vacateNode();
  }
  KPlayerListViewGroupItem::terminate();
}

KPlayerListViewFolderItem::~KPlayerListViewFolderItem()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying list view folder item\n";
#endif
}

int KPlayerTreeViewFolderItem::compare (QListViewItem* item, int, bool) const
{
  return node() -> compareByName (nodeForItem (item));
}

void KPlayerTreeViewFolderItem::connectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Connecting tree view folder item node\n";
  kdDebugTime() << " URL    " << node() -> url().url() << "\n";
#endif
  connectNodeCommon();
}

void KPlayerTreeViewFolderItem::disconnectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Disconnecting tree view folder item node\n";
  kdDebugTime() << " URL    " << node() -> url() << "\n";
#endif
  disconnectNodeCommon();
}

void KPlayerTreeViewFolderItem::populateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Populating tree view folder item node groups\n";
#endif
  node() -> populateGroups();
}

void KPlayerTreeViewFolderItem::vacateNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Vacating tree view folder item node groups\n";
#endif
  node() -> vacateGroups();
}

void KPlayerTreeViewFolderItem::updateColumns (void)
{
}

bool KPlayerTreeViewFolderItem::hasChildren (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeViewFolderItem::hasChildren\n";
#endif
  return node() -> hasGroups();
}

KPlayerListViewItem* KPlayerTreeViewFolderItem::createChild (KPlayerListViewItem* item, KPlayerNode* node)
{
  return node -> isContainer() ? (KPlayerListViewItem*) new KPlayerTreeViewFolderItem (this, item, node) : 0;
}

KPlayerTreeViewFolderItem::~KPlayerTreeViewFolderItem()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying tree view folder item\n";
#endif
}

KPlayerHistoryActionList::KPlayerHistoryActionList (KPlayerHistory& history, const KPlayerHistory::Iterator& current,
  const QString& text, const QString& status, const QString& whatsthis, QObject* parent, const char* name)
  : KPlayerActionList (text, status, whatsthis, parent, name), m_history (history), m_current (current)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Creating history action list\n";
#endif
}

KPlayerHistoryActionList::~KPlayerHistoryActionList()
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Destroying history action list\n";
#endif
}

void KPlayerHistoryActionList::update (void)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerHistoryActionList::update\n";
#endif
  unplug();
  if ( m_history.count() > 1 )
  {
    uint limit = KPlayerEngine::engine() -> configuration() -> recentMenuSize();
    uint halflimit = limit / 2;
    uint count = 0;
    KPlayerHistory::ConstIterator iterator (m_history.end());
    do
    {
      -- iterator;
      if ( actions().count() == limit )
        m_actions.removeFirst();
      KToggleAction* action = new KToggleAction ((*iterator).m_name, 0, this, SLOT(actionActivated()), this);
      if ( iterator == m_current )
      {
        action -> setChecked (true);
        ++ count;
      }
      else if ( count )
        ++ count;
      updateAction (action);
      m_actions.append (action);
    }
    while ( iterator != m_history.begin() && (count < halflimit || actions().count() < limit) );
    plug();
  }
}

void KPlayerHistoryActionList::actionActivated (KAction*, int index)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerHistoryActionList::actionActivated\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  if ( ! m_history.isEmpty() )
  {
    KPlayerHistory::Iterator iterator (m_history.end());
    do
    {
      -- iterator;
      -- index;
    }
    while ( iterator != m_history.begin() && index >= 0 );
    if ( index < 0 )
      emit activated (iterator);
  }
}

KPlayerNodeView::KPlayerNodeView (QWidget* parent, const char* name)
  : KListView (parent, name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Creating node view\n";
#endif
  m_node = 0;
  m_last_node = 0;
  m_moving = false;
  m_in_focus = false;
  m_popup_menu_shown = false;
  m_mouse_pressed = false;
  m_dragging = false;
  m_last_item = 0;
  m_editing_item = 0;
  m_editing_column = 0;
}

KPlayerNodeView::~KPlayerNodeView()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying node view\n";
#endif
}

void KPlayerNodeView::initialize (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Initializing node view\n";
#endif
  setAcceptDrops (true);
  setDragEnabled (true);
  setAllColumnsShowFocus (true);
  setDefaultRenameAction (Accept);
  setShowSortIndicator (true);
  setTabOrderedRenaming (false);
  renameLineEdit() -> setFrame (true);
  //header() -> disconnect (SIGNAL (clicked (int)), this);
  //header() -> disconnect (SIGNAL (sectionClicked (int)), this);
  //header() -> disconnect (SIGNAL (sectionHandleDoubleClicked (int)), this);
  connect (this, SIGNAL (selectionChanged()), SLOT (itemSelectionChanged()));
  connect (this, SIGNAL (executed (QListViewItem*)), SLOT (itemExecuted (QListViewItem*)));
  if ( library() -> popupMenu() )
    connect (this, SIGNAL (contextMenu (KListView*, QListViewItem*, const QPoint&)),
      SLOT (showContextMenu (KListView*, QListViewItem*, const QPoint&)));
  connect (this, SIGNAL (itemRenamed (QListViewItem*, const QString&, int)),
    SLOT (edited (QListViewItem*, const QString&, int)));
  header() -> installEventFilter (this);
  renameLineEdit() -> installEventFilter (this);
}

void KPlayerNodeView::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Terminating node view\n";
#endif
  disconnect (this, SIGNAL (selectionChanged()), this, SLOT (itemSelectionChanged()));
  setRootNode (0);
}

void KPlayerNodeView::connectNodeCommon (void)
{
  connect (rootNode(), SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  connect (rootNode(), SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  connect (rootNode(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
    this, SLOT (updated (KPlayerContainerNode*, KPlayerNode*)));
}

void KPlayerNodeView::disconnectNodeCommon (void)
{
  disconnect (rootNode(), SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    this, SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  disconnect (rootNode(), SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    this, SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  disconnect (rootNode(), SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
    this, SLOT (updated (KPlayerContainerNode*, KPlayerNode*)));
}

KPlayerNode* KPlayerNodeView::currentNode (void) const
{
  return activeNode();
}

KPlayerContainerNode* KPlayerNodeView::activeContainer (void) const
{
  return ! currentItem() ? rootNode() : currentItem() -> isSelected() && activeNode() -> isContainer() ?
    (KPlayerContainerNode*) activeNode() : activeNode() -> parent();
}

void KPlayerNodeView::setRootNode (KPlayerContainerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::setRootNode\n";
  if ( node )
    kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  if ( rootNode() )
  {
    disconnectNode();
    while ( KPlayerListViewItem* item = (KPlayerListViewItem*) firstChild() )
      item -> terminate();
    rootNode() -> vacate();
    rootNode() -> release();
  }
  m_node = node;
  if ( node )
  {
    KPlayerItemProperties::resetMetaInfoTimer();
    setRootIsDecorated (! node -> groupsFirst() || ! node -> parent());
    node -> reference();
    node -> populate();
    while ( columns() > 0 )
      removeColumn (0);
    setupColumns();
    added (node, node -> nodes());
    connectNode();
  }
}

KPlayerListViewItem* KPlayerNodeView::itemForNode (KPlayerNode* node, bool open)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::itemForNode\n";
#endif
  if ( ! node )
    return 0;
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " URL    " << node -> url() << "\n";
#endif
  KPlayerListViewItem* item;
  if ( ! node -> parent() || rootNode() == node -> parent() )
    item = (KPlayerListViewItem*) firstChild();
  else
  {
    item = itemForNode (node -> parent(), open);
    if ( ! item || ! open && ! item -> isOpen() )
      return 0;
    if ( ! item -> isOpen() )
      setOpen (item, true);
    item = (KPlayerListViewItem*) item -> firstChild();
  }
  while ( item )
  {
    if ( item -> node() == node )
      return item;
    item = (KPlayerListViewItem*) item -> nextSibling();
  }
  return 0;
}

void KPlayerNodeView::added (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::added\n";
#endif
  if ( ! parent || parent == rootNode() )
  {
    bool was_empty = childCount() == 0;
    KPlayerListViewItem* item = after == parent ? 0 : itemForNode (after);
    KPlayerNodeListIterator iterator (nodes);
    while ( KPlayerNode* node = iterator.current() )
    {
      item = createChild (item, node);
      if ( item )
      {
        item -> initialize();
        update (item);
        item -> setExpandable (item -> hasChildren());
        if ( moving() )
          item -> setSelected (true);
      }
      ++ iterator;
    }
    sort();
    if ( was_empty && childCount() == 1 && nodeForItem (item) -> isContainer() )
      setOpen (item, true);
  }
  else if ( rootIsDecorated() )
  {
    KPlayerListViewFolderItem* item = (KPlayerListViewFolderItem*) itemForNode (parent);
    if ( item )
    {
      item -> added (nodes, after);
      if ( item -> isOpen() )
        item -> sort();
    }
  }
  if ( inFocus() )
    updateActions();
  else if ( sibling() -> inFocus() )
    sibling() -> updateActions();
}

void KPlayerNodeView::removed (KPlayerContainerNode* parent, const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::removed\n";
#endif
  if ( ! parent || parent == rootNode() )
  {
    KPlayerNodeListIterator iterator (nodes);
    while ( KPlayerNode* node = iterator.current() )
    {
      KPlayerListViewItem* item = itemForNode (node);
      if ( item )
        item -> terminate();
      ++ iterator;
    }
  }
  else if ( rootIsDecorated() )
  {
    KPlayerListViewFolderItem* item = (KPlayerListViewFolderItem*) itemForNode (parent);
    if ( item )
      item -> removed (nodes);
  }
  if ( inFocus() )
    updateActions();
  else if ( sibling() -> inFocus() )
    sibling() -> updateActions();
}

void KPlayerNodeView::updated (KPlayerContainerNode*, KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::updated\n";
#endif
  KPlayerListViewItem* item = itemForNode (node);
  if ( item )
  {
    item -> update();
    update (item);
  }
}

void KPlayerNodeView::update (KPlayerListViewItem*)
{
}

void KPlayerNodeView::startEditing (QListViewItem* item, int column)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::startEditing\n";
  kdDebugTime() << " Item   " << item -> text (0) << "\n";
  kdDebugTime() << " Column " << column << "\n";
#endif
  stopEditing();
  sibling() -> stopEditing();
  setRenameable (column, true);
  setItemsRenameable (true);
  if ( item -> itemAbove() )
    ensureItemVisible (item -> itemAbove());
  if ( item -> itemBelow() )
    ensureItemVisible (item -> itemBelow());
  ensureItemVisible (item);
  QRect rect (itemRect (item));
  rect.moveLeft (0);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " ItemRect " << rect.x() << "x" << rect.y() << " " << rect.width() << "x" << rect.height() << "\n";
  kdDebugTime() << " ItemPos " << item -> itemPos() << "\n";
#endif
  viewport() -> repaint (rect, false);
  int pos = header() -> sectionPos (column);
  int width = header() -> sectionSize (column);
  int index = header() -> mapToIndex (column);
  int before = index > 0 ? header() -> sectionSize (header() -> mapToSection (index - 1)) : 0;
  int after = index < columns() - 1 ? header() -> sectionSize (header() -> mapToSection (index + 1)) : 0;
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Index  " << index << "\n";
  kdDebugTime() << " Name   " << attributeNames() [index] << "\n";
  kdDebugTime() << " Position " << pos << "\n";
  kdDebugTime() << " Width  " << width << "\n";
  kdDebugTime() << " Before " << before << "\n";
  kdDebugTime() << " After  " << after << "\n";
  kdDebugTime() << " Visible " << visibleWidth() << "\n";
  kdDebugTime() << " Frame  " << renameLineEdit() -> frame() << "\n";
#endif
  if ( before + width > visibleWidth() - 50 )
    before = 0;
  if ( pos + width + after > contentsX() + visibleWidth() && pos > contentsX() + before || pos < contentsX() + before )
    setContentsPos (pos < contentsX() + before || before + width + after > visibleWidth() ? pos - before
      : pos + width + after - visibleWidth(), contentsY());
  header() -> repaint (false);
  rect = itemRect (item);
  rect.moveLeft (0);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " ItemRect " << rect.x() << "x" << rect.y() << " " << rect.width() << "x" << rect.height() << "\n";
  kdDebugTime() << " ItemPos " << item -> itemPos() << "\n";
#endif
  viewport() -> repaint (rect, false);
  m_editing_item = (KPlayerListViewItem*) item;
  m_editing_column = column;
  rename (item, column);
  connect (this, SIGNAL(contentsMoving(int, int)), SLOT(moveLineEdit(int, int)));
}

void KPlayerNodeView::stopEditing (bool save, int state)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::stopEditing " << save << " " << state << "\n";
#endif
  disconnect (this, SIGNAL(contentsMoving(int, int)), this, SLOT(moveLineEdit(int, int)));
  if ( renameLineEdit() -> isHidden() )
    return;
  QListViewItem* item = m_editing_item;
  QKeyEvent keyevent (QEvent::KeyPress, save ? Qt::Key_Return : Qt::Key_Escape, 0, state);
  QApplication::sendEvent (renameLineEdit(), &keyevent);
  repaintItem (item);
  setItemsRenameable (false);
  m_editing_item = 0;
  m_editing_column = 0;
  treeView() -> resetActiveNode();
}

void KPlayerNodeView::moveLineEdit (int x, int y)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::moveLineEdit\n";
  kdDebugTime() << " From   " << contentsX() << "x" << contentsY() << "\n";
  kdDebugTime() << " To     " << x << "x" << y << "\n";
  kdDebugTime() << " Edit   " << renameLineEdit() -> x() << "x" << renameLineEdit() -> y() << "\n";
#endif
  QTimer::singleShot (0, this, SLOT (moveLineEdit()));
}

void KPlayerNodeView::moveLineEdit (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::moveLineEdit\n";
#endif
  if ( m_editing_item )
  {
    QRect rect (itemRect (m_editing_item));
    int x = rect.x() - 1;
    int index = header() -> mapToIndex (m_editing_column);
    for ( int i = 0; i < index; i ++ )
      x += columnWidth (header() -> mapToSection (i));
    if ( m_editing_column == 0 )
      x += (m_editing_item -> depth() + (rootIsDecorated() ? 1 : 0)) * treeStepSize();
    if ( m_editing_item -> pixmap (m_editing_column) )
      x += m_editing_item -> pixmap (m_editing_column) -> width();
    renameLineEdit() -> move (x, rect.y() - 1);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Moved  " << renameLineEdit() -> x() << "x" << renameLineEdit() -> y() << "\n";
#endif
  }
}

bool KPlayerNodeView::eventFilter (QObject* object, QEvent* event)
{
  static bool recursion = false, move_recursion = false;
  static bool firstcolumn = false, anothercolumn = false;
  if ( object == header() && (event -> type() == QEvent::MouseButtonPress
    || event -> type() == QEvent::MouseButtonRelease || event -> type() == QEvent::MouseMove) )
  {
    QMouseEvent* mouseevent = (QMouseEvent*) event;
    const QPoint& position (mouseevent -> pos());
    int section = header() -> sectionAt (position.x() + header() -> offset());
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << "KPlayerNodeView::eventFilter mouse move " << position.x() << "x" << position.y()
      << " offset " << header() -> offset() << " section " << section << "\n";
#endif
    if ( (mouseevent -> stateAfter() & Qt::LeftButton) == Qt::LeftButton )
    {
      if ( event -> type() == QEvent::MouseButtonPress )
        (section == 0 ? firstcolumn : anothercolumn) = true;
      else if ( event -> type() == QEvent::MouseMove && (firstcolumn || anothercolumn && section <= 0) )
        return true;
    }
    else
    {
      //if ( event -> type() == QEvent::MouseButtonRelease && (firstcolumn && section || anothercolumn && section <= 0) )
      //  return true;
      firstcolumn = anothercolumn = false;
    }
  }
  else if ( ! move_recursion && m_editing_item && object == renameLineEdit() && event -> type() == QEvent::Move )
  {
#ifdef DEBUG_KPLAYER_NODEVIEW
    QMoveEvent* moveevent = (QMoveEvent*) event;
    kdDebugTime() << "KPlayerNodeView::eventFilter move " << moveevent -> oldPos().x() << "x" << moveevent -> oldPos().y()
      << " => " << moveevent -> pos().x() << "x" << moveevent -> pos().y() << " " << event -> spontaneous() << "\n";
#endif
    move_recursion = true;
    moveLineEdit();
    move_recursion = false;
  }
  else if ( ! recursion && m_editing_item && object == renameLineEdit()
    && (event -> type() == QEvent::KeyPress || event -> type() == QEvent::AccelOverride) )
  {
    QKeyEvent* keyevent = (QKeyEvent*) event;
    int key = keyevent -> key();
    int state = keyevent -> state();
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << "KPlayerNodeView::eventFilter " << event -> type() << " " << key << " " << state << "\n";
#endif
    if ( event -> type() == QEvent::AccelOverride )
    {
      if ( key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Escape
        || key == Qt::Key_Tab || key == Qt::Key_Backtab || key == Qt::Key_Up || key == Qt::Key_Down )
      {
        keyevent -> accept();
        return true;
      }
    }
    else if ( key == Qt::Key_Return || key == Qt::Key_Enter )
    {
      recursion = true;
      stopEditing (true, state);
      recursion = false;
      return true;
    }
    else if ( key == Qt::Key_Escape )
    {
      recursion = true;
      stopEditing (false, state);
      recursion = false;
      return true;
    }
    else if ( key == Qt::Key_Tab || key == Qt::Key_Backtab )
    {
      if ( (state & (Qt::AltButton | Qt::ControlButton | Qt::MetaButton)) == 0 )
      {
        KPlayerListViewItem* item = m_editing_item;
        int column = m_editing_column;
        recursion = true;
        stopEditing (true, state);
        recursion = false;
#ifdef DEBUG_KPLAYER_NODEVIEW
        kdDebugTime() << " Item   " << item -> text (0) << "\n";
        kdDebugTime() << " Column " << column << "\n";
#endif
        column = header() -> mapToIndex (column);
#ifdef DEBUG_KPLAYER_NODEVIEW
        kdDebugTime() << " Index  " << column << "\n";
        kdDebugTime() << " Name   " << attributeNames() [column] << "\n";
#endif
        if ( column >= 0 )
        {
          bool edit = false;
          bool container = item -> node() -> isContainer();
          bool tab = key == Qt::Key_Tab && (state & Qt::ShiftButton) == 0;
          do
          {
            while ( ! edit && (container ? column != 0 : tab ? column < attributeCount() - 1 : column > 0) )
            {
              tab ? ++ column : -- column;
#ifdef DEBUG_KPLAYER_NODEVIEW
              kdDebugTime() << " Name   " << attributeNames() [column] << "\n";
              kdDebugTime() << " Index  " << column << "\n";
#endif
              edit = column ? ! container && KPlayerMedia::info (attributeNames() [column]) -> canEdit()
                : item -> node() -> canRename();
            }
            if ( ! edit )
            {
              item = tab ? (KPlayerListViewItem*) item -> itemBelow() : (KPlayerListViewItem*) item -> itemAbove();
              if ( item )
              {
                container = item -> node() -> isContainer();
                column = tab ? -1 : container ? 1 : attributeCount();
              }
            }
          }
          while ( item && ! edit );
          if ( item )
          {
#ifdef DEBUG_KPLAYER_NODEVIEW
            kdDebugTime() << " Name   " << attributeNames() [column] << "\n";
            kdDebugTime() << " Index  " << column << "\n";
#endif
            column = header() -> mapToSection (column);
            if ( column >= 0 )
            {
#ifdef DEBUG_KPLAYER_NODEVIEW
              kdDebugTime() << " Column " << column << "\n";
              kdDebugTime() << " Item   " << item -> text (0) << "\n";
#endif
              startEditing (item, column);
            }
          }
        }
      }
      return true;
    }
    else if ( key == Qt::Key_Up || key == Qt::Key_Down )
    {
#ifdef DEBUG_KPLAYER_NODEVIEW
      kdDebugTime() << "KPlayerNodeView::eventFilter " << (key == Qt::Key_Up ? "Up" : "Down") << "\n";
#endif
      if ( (state & (Qt::AltButton | Qt::ControlButton | Qt::MetaButton)) == 0 )
      {
        KPlayerListViewItem* item = m_editing_item;
        int column = m_editing_column;
        recursion = true;
        stopEditing (true, state);
        recursion = false;
#ifdef DEBUG_KPLAYER_NODEVIEW
        kdDebugTime() << " Item   " << item -> text (0) << "\n";
        kdDebugTime() << " Column " << column << "\n";
#endif
        int index = header() -> mapToIndex (column);
#ifdef DEBUG_KPLAYER_NODEVIEW
        kdDebugTime() << " Index  " << index << "\n";
        kdDebugTime() << " Name   " << attributeNames() [index] << "\n";
#endif
        if ( index >= 0 )
        {
          bool up = key == Qt::Key_Up;
          do
            item = up ? (KPlayerListViewItem*) item -> itemAbove() : (KPlayerListViewItem*) item -> itemBelow();
          while ( item && (index ? item -> node() -> isContainer() : ! item -> node() -> canRename()) );
          if ( item )
          {
#ifdef DEBUG_KPLAYER_NODEVIEW
            kdDebugTime() << " Item   " << item -> text (0) << "\n";
#endif
            startEditing (item, column);
          }
        }
      }
      return true;
    }
  }
  return KListView::eventFilter (object, event);
}

void KPlayerNodeView::findDropTarget (const QPoint& pos, QListViewItem*& parent, QListViewItem*& after)
{
  parent = after = 0;
  if ( ! firstChild() )
    return;
  QPoint p (contentsToViewport (pos));
  KPlayerListViewItem* item = (KPlayerListViewItem*) itemAt (p);
  KPlayerNode* node = item ? item -> node() : rootNode();
  KPlayerContainerNode* container = node -> isContainer() ? (KPlayerContainerNode*) node : node -> parent();
  KPlayerListViewItem* above = (KPlayerListViewItem*) (item ? item -> itemAbove() : lastItem());
  int offset = p.y() - (item ? itemRect (item).topLeft().y() : itemRect (above).bottomLeft().y());
  int height = (item ? item : above) -> height() / 2;
  /*if ( above && offset < height )
  {
    KPlayerNode* aboveNode = above -> node();
    KPlayerContainerNode* aboveContainer = aboveNode -> isContainer() ? (KPlayerContainerNode*) aboveNode
      : aboveNode -> parent();
    if ( aboveContainer -> canAddLeaves() && aboveContainer -> allowsCustomOrder() )
    {
      item = above;
      node = aboveNode;
      container = aboveContainer;
    }
  }*/
  if ( ! item )
    return;
  if ( ! container -> allowsCustomOrder() || node -> isContainer() && offset <= height )
  {
    parent = node -> isContainer() ? item : item -> parent();
    return;
  }
  if ( node -> isContainer() && (p.x() >= depthToPixels (item -> depth() + 1)
    || item -> isOpen() && item -> firstChild()) )
  {
    parent = item;
    after = item;
    return;
  }
  if ( item -> nextSibling() )
  {
    parent = item -> parent();
    after = item;
    return;
  }
  QListViewItem* last = item;
  above = (KPlayerListViewItem*) item -> parent();
  while ( above && ! last -> nextSibling() && p.x() < depthToPixels (last -> depth()) )
  {
    if ( ((KPlayerContainerNode*) above -> node()) -> canAddLeaves() )
      item = above;
    last = above;
    above = (KPlayerListViewItem*) above -> parent();
  }
  after = item;
  parent = item ? item -> parent() : 0;
}

void KPlayerNodeView::findDrop (const QPoint& pos, QListViewItem*& parent, QListViewItem*& after)
{
  findDropTarget (pos, parent, after);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::findDrop\n";
  if ( parent )
    kdDebugTime() << " Parent " << parent -> text (0) << "\n";
  if ( after )
    kdDebugTime() << " After  " << after -> text (0) << "\n";
#endif
  if ( parent == after )
    after = 0;
}

extern QDragManager* qt_dnd_manager;

bool KPlayerNodeView::acceptDrag (QDropEvent* event) const
{
  if ( event -> pos().x() < 0 || event -> pos().y() < 0 )
    return false;
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::acceptDrag\n";
  kdDebugTime() << " Action " << event -> action() << "\n";
#endif
  event -> accept (false);
  event -> acceptAction (false);
  KPlayerNodeView* that = (KPlayerNodeView*) this;
  QListViewItem *parent, *after;
  that -> findDropTarget (event -> pos(), parent, after);
#ifdef DEBUG_KPLAYER_NODEVIEW
  if ( parent )
    kdDebugTime() << " Parent " << parent -> text (0) << "\n";
  if ( after )
    kdDebugTime() << " After  " << after -> text (0) << "\n";
#endif
  KURL::List urls;
  bool urldrag = KURLDrag::canDecode (event);
  if ( urldrag && KURLDrag::decode (event, urls)
    || event -> source() == viewport() || event -> source() == sibling() -> viewport() )
  {
    KPlayerContainerNode* target = parent ? ((KPlayerListViewGroupItem*) parent) -> node() : rootNode();
    const KPlayerNodeView* view = event -> source() == viewport() ? this : sibling();
    if ( urldrag && m_drag_node_list.isEmpty() )
      that -> m_drag_node_list = KPlayerNodeList::fromUrlList (urls);
    KPlayerNodeList nodes (urldrag ? m_drag_node_list : view -> getSelectedNodes());
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Target " << target -> url().url() << "\n";
#endif
    if ( target && ! nodes.isEmpty() && target -> canCopy (nodes) )
    {
      if ( after )
        that -> cleanItemHighlighter();
      else
        that -> cleanDropVisualizer();
      that -> setDropHighlighter (after == 0);
      that -> setDropVisualizer (after != 0);
      KPlayerNode* node = nodes.getFirst();
      bool control = kPlayerSettings() -> control();
      bool shift = kPlayerSettings() -> shift();
      bool canlink = target -> canLink (nodes);
      bool canmove = ! urldrag && node -> parent() -> canRemove();
      if ( control )
      {
        if ( shift )
          shift = canlink;
      }
      else if ( shift )
      {
        shift = canmove || canlink;
        control = ! canmove;
      }
      else if ( canlink )
        control = shift = true;
      else if ( ! urldrag && target -> preferMove (node) )
        shift = true;
      else
        control = true;
      int state = (kPlayerSettings() -> anyShift() ? Qt::ShiftButton : 0)
        | (kPlayerSettings() -> anyControl() ? Qt::ControlButton : 0);
#ifdef DEBUG_KPLAYER_NODEVIEW
      kdDebugTime() << " Accept " << event -> isAccepted() << "\n";
      kdDebugTime() << " Action " << event -> isActionAccepted() << "\n";
      kdDebugTime() << " Control real " << kPlayerSettings() -> control()
        << " current " << kPlayerSettings() -> anyControl() << " need " << control << "\n";
      kdDebugTime() << " Shift   real " << kPlayerSettings() -> shift()
        << " current " << kPlayerSettings() -> anyShift() << " need " << shift << "\n";
#endif
      if ( control && ! kPlayerSettings() -> anyControl() )
      {
        kPlayerSettings() -> setFakeControl();
        QApplication::postEvent (that, new QKeyEvent (QEvent::KeyPress, Qt::Key_Control, 0, state));
      }
      else if ( ! control && kPlayerSettings() -> anyControl() )
      {
        kPlayerSettings() -> setNoControl();
        QApplication::postEvent (that, new QKeyEvent (QEvent::KeyRelease, Qt::Key_Control, 0, state));
      }
      if ( shift && ! kPlayerSettings() -> anyShift() )
      {
        kPlayerSettings() -> setFakeShift();
        QApplication::postEvent (that, new QKeyEvent (QEvent::KeyPress, Qt::Key_Shift, 0, state));
      }
      else if ( ! shift && kPlayerSettings() -> anyShift() )
      {
        kPlayerSettings() -> setNoShift();
        QApplication::postEvent (that, new QKeyEvent (QEvent::KeyRelease, Qt::Key_Shift, 0, state));
      }
      if ( kPlayerSettings() -> anyControl() && kPlayerSettings() -> anyShift() )
        event -> setAction (QDropEvent::Link);
      else if ( kPlayerSettings() -> anyControl() )
        event -> setAction (QDropEvent::Copy);
      else if ( kPlayerSettings() -> anyShift() )
        event -> setAction (QDropEvent::Move);
      event -> acceptAction();
      qt_dnd_manager -> killTimers();
      return true;
    }
  }
  that -> cleanItemHighlighter();
  that -> cleanDropVisualizer();
  that -> setDropHighlighter (false);
  that -> setDropVisualizer (false);
  kPlayerSettings() -> resetControl();
  kPlayerSettings() -> resetShift();
  return false;
}

void KPlayerNodeView::contentsDragMoveEvent (QDragMoveEvent* event)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsDragMoveEvent\n";
#endif
  //if ( acceptDrag (event) )
  KListView::contentsDragMoveEvent (event);
  if ( event -> isAccepted() )
    event -> accept (QRect (QCursor::pos(), QCursor::pos()));
  qt_dnd_manager -> killTimers();
}

void KPlayerNodeView::contentsDragEnterEvent (QDragEnterEvent* event)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsDragEnterEvent\n";
#endif
  m_dragging = true;
  m_drag_node_list.releaseAll();
  m_drag_node_list.clear();
  kPlayerSettings() -> resetControl();
  kPlayerSettings() -> resetShift();
  KListView::contentsDragEnterEvent (event);
}

void KPlayerNodeView::contentsDragLeaveEvent (QDragLeaveEvent* event)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsDragLeaveEvent\n";
#endif
  m_drag_node_list.releaseAll();
  m_drag_node_list.clear();
  kPlayerSettings() -> resetControl();
  kPlayerSettings() -> resetShift();
  KListView::contentsDragLeaveEvent (event);
}

void KPlayerNodeView::contentsDropEvent (QDropEvent* event)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsDropEvent\n";
#endif
  if ( acceptDrag (event) )
  {
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Drag accepted\n";
#endif
    QListViewItem *parent = 0, *after = 0;
    findDropTarget (event -> pos(), parent, after);
#ifdef DEBUG_KPLAYER_NODEVIEW
    if ( parent )
      kdDebugTime() << " Parent " << parent -> text (0) << "\n";
    if ( after )
      kdDebugTime() << " After  " << after -> text (0) << "\n";
#endif
    KURL::List urls;
    bool own = event -> source() == viewport();
    bool urldrag = KURLDrag::canDecode (event) && KURLDrag::decode (event, urls);
    KPlayerContainerNode* target = parent ? ((KPlayerListViewGroupItem*) parent) -> node() : rootNode();
    const KPlayerNodeView* view = own ? this : sibling();
    if ( urldrag && m_drag_node_list.isEmpty() )
      m_drag_node_list = KPlayerNodeList::fromUrlList (urls);
    KPlayerNodeList nodes (urldrag ? m_drag_node_list : view -> getSelectedNodes());
    if ( ! nodes.isEmpty() )
    {
      if ( after )
        setNodeOrder (target);
      KPlayerNode* node = after ? ((KPlayerListViewItem*) after) -> node() : 0;
      if ( event -> isActionAccepted() && event -> action() == QDropEvent::Move )
      {
        m_moving = own && ! urldrag;
        target -> move (nodes, node);
        m_moving = false;
      }
      else
        target -> add (nodes, event -> isActionAccepted() && event -> action() == QDropEvent::Link, node);
    }
  }
  event -> setPoint (QPoint (-1, -1));
  KListView::contentsDropEvent (event);
  m_drag_node_list.releaseAll();
  m_drag_node_list.clear();
  setCurrentItem (m_last_item);
  m_dragging = false;
  m_mouse_pressed = false;
}

void KPlayerNodeView::setNodeOrder (KPlayerContainerNode*)
{
}

void KPlayerNodeView::connectActions (void)
{
  library() -> disconnectActions();
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Connecting actions\n";
#endif
  connect (action ("library_play"), SIGNAL(activated()), SLOT(play()));
  connect (action ("library_play_next"), SIGNAL(activated()), SLOT(playNext()));
  connect (action ("library_queue"), SIGNAL(activated()), SLOT(queue()));
  connect (action ("library_queue_next"), SIGNAL(activated()), SLOT(queueNext()));
  connect (action ("library_add_files"), SIGNAL(activated()), SLOT(addFiles()));
  connect (action ("library_add_url"), SIGNAL(activated()), SLOT(addUrl()));
  connect (action ("library_add_group"), SIGNAL(activated()), SLOT(addGroup()));
  connect (action ("library_add_to_new_playlist"), SIGNAL(activated()), SLOT(addToNewPlaylist()));
  connect (action ("library_add_to_playlists"), SIGNAL(activated()), SLOT(addToPlaylists()));
  connect (playlistActionList(), SIGNAL(activated(KPlayerNode*)), SLOT(addToPlaylist(KPlayerNode*)));
  connect (action ("library_add_to_collection"), SIGNAL(activated()), SLOT(addToCollection()));
  connect (action ("library_rename"), SIGNAL(activated()), SLOT(editName()));
  connect (editActionList(), SIGNAL(activated(int)), listView(), SLOT(editField(int)));
  connect (action ("library_properties"), SIGNAL(activated()), SLOT(properties()));
  connect (action ("library_select_all"), SIGNAL(activated()), listView(), SLOT(selectAll()));
  connect (action ("library_move_up"), SIGNAL(activated()), SLOT(moveUp()));
  connect (action ("library_move_down"), SIGNAL(activated()), SLOT(moveDown()));
  connect (action ("library_remove"), SIGNAL(activated()), SLOT(remove()));
  connect (columnActionList(), SIGNAL(activated(int)), listView(), SLOT(toggleColumn(int)));
  connect (action ("library_go_back"), SIGNAL(activated()), treeView(), SLOT(goBack()));
  connect (action ("library_go_forward"), SIGNAL(activated()), treeView(), SLOT(goForward()));
  connect (action ("library_go_up"), SIGNAL(activated()), listView(), SLOT(goUp()));
  connect (action ("library_go_down"), SIGNAL(activated()), listView(), SLOT(goDown()));
  connect (action ("library_go_to_origin"), SIGNAL(activated()), SLOT(goToOrigin()));
}

void KPlayerNodeView::showContextMenu (KListView*, QListViewItem*, const QPoint& point)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Showing node view popup menu\n";
#endif
  stopEditing();
  sibling() -> stopEditing();
  setFocus();
  connectActions();
  updateActions();
  m_popup_menu_shown = true;
  m_mouse_pressed = false;
  library() -> popupMenu() -> popup (point);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Shown node view popup menu\n";
#endif
}

void KPlayerNodeView::focusInEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "List view getting focus\n";
#endif
  KListView::focusInEvent (event);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "List view got focus\n";
#endif
  m_in_focus = true;
  connectActions();
  updateActions();
  library() -> setLastActiveView (this);
}

void KPlayerNodeView::focusOutEvent (QFocusEvent* event)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "List view loosing focus\n";
#endif
  KListView::focusOutEvent (event);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "List view lost focus\n";
#endif
  m_in_focus = false;
}

void KPlayerNodeView::contentsMousePressEvent (QMouseEvent* e)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsMousePressEvent enter\n";
  kdDebugTime() << " Button " << e -> button() << "\n";
  kdDebugTime() << " State  " << e -> stateAfter() << "\n";
#endif
  m_mouse_pressed = true;
  m_popup_menu_shown = e -> button() == Qt::RightButton;
  m_last_node = currentNode();
  KListView::contentsMousePressEvent (e);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsMousePressEvent leave\n";
#endif
}

void KPlayerNodeView::contentsMouseReleaseEvent (QMouseEvent* e)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsMouseReleaseEvent enter\n";
  kdDebugTime() << " Button " << e -> button() << "\n";
  kdDebugTime() << " State  " << e -> stateAfter() << "\n";
#endif
  KListView::contentsMouseReleaseEvent (e);
  m_mouse_pressed = (e -> stateAfter() & Qt::MouseButtonMask) != Qt::NoButton;
  if ( ! m_mouse_pressed && ! m_popup_menu_shown )
  {
    if ( m_dragging )
    {
      m_dragging = false;
      setCurrentItem (m_last_item);
    }
    else
      itemSelectionChanged();
  }
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::contentsMouseReleaseEvent leave\n";
#endif
}

void KPlayerNodeView::itemExecuted (QListViewItem* item)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Item executed\n";
#endif
  KPlayerNode* node = nodeForItem (item);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  if ( node -> isContainer() )
  {
    if ( ! rootIsDecorated() )
      treeView() -> setActiveNode ((KPlayerContainerNode*) node);
    else if ( activeNode() == m_last_node )
      setOpen (item, ! item -> isOpen());
  }
  else
  {
    item -> setSelected (true);
    item -> repaint();
    play();
  }
}

void KPlayerNodeView::itemSelectionChanged (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Selection changed\n";
#endif
  if ( ! m_mouse_pressed && currentItem() != m_last_item )
  {
    activeItemChanged();
    m_last_item = (KPlayerListViewItem*) currentItem();
    if ( inFocus() )
      updateActions();
    else if ( sibling() -> inFocus() )
      sibling() -> updateActions();
  }
}

void KPlayerNodeView::activeItemChanged (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Active item changed\n";
#endif
}

void KPlayerNodeView::itemTerminating (QListViewItem* item)
{
  if ( item == m_editing_item )
    stopEditing();
  if ( item == m_last_item )
    m_last_item = 0;
}

void KPlayerNodeView::keepUpCurrentItem (QListViewItem* current, QListViewItem* item)
{
  QListViewItem* lvi = currentItem();
  if ( lvi != current )
  {
    while ( lvi && lvi != item )
      lvi = lvi -> parent();
    if ( lvi )
    {
#ifdef DEBUG_KPLAYER_NODEVIEW
      kdDebugTime() << "KPlayerNodeView::keepUpCurrentItem\n";
      kdDebugTime() << " Item    " << item -> text (0) << "\n";
      kdDebugTime() << " Current " << current -> text (0) << "\n";
#endif
      setCurrentItem (current);
      QTimer::singleShot (0, treeView(), SLOT (activateCurrentItem()));
    }
  }
}

void KPlayerNodeView::setOpen (QListViewItem* item, bool open)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::setOpen " << open << "\n";
  kdDebugTime() << " URL    " << nodeForItem (item) -> url().url() << "\n";
#endif
  if ( ! rootIsDecorated() )
    return;
  if ( ! open )
    keepUpCurrentItem (item, item);
  KListView::setOpen (item, open);
}

void KPlayerNodeView::play (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::play\n";
#endif
  playlist() -> play (getSelectedNodes());
}

void KPlayerNodeView::playNext (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::playNext\n";
#endif
  playlist() -> playNext (getSelectedNodes());
}

void KPlayerNodeView::queue (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::queue\n";
#endif
  playlist() -> queue (getSelectedNodes());
}

void KPlayerNodeView::queueNext (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::queueNext\n";
#endif
  playlist() -> queueNext (getSelectedNodes());
}

void KPlayerNodeView::addFiles (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addFiles\n";
#endif
  activeContainer() -> append (kPlayerEngine() -> openFiles (i18n("Add Files")));
}

void KPlayerNodeView::addUrl (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addUrl\n";
#endif
  activeContainer() -> append (kPlayerEngine() -> openUrl (i18n("Add URL")));
}

void KPlayerNodeView::addGroup (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addGroup\n";
#endif
  KPlayerContainerNode* container = activeContainer();
  if ( container -> isDevices() )
  {
    KPlayerDeviceDialog dialog (this);
    if ( dialog.exec() == KDialogBase::Accepted )
      dialog.addDevice();
  }
  else
  {
    KPlayerNodeNameValidator validator (container);
    QString name = KInputDialog::text (i18n("Add folder"), i18n("Folder name"),
      QString::null, 0, 0, 0, &validator, QString::null,
      i18n("Folder name field allows you to enter a name for a new folder. OK button will be enabled when you enter a unique and valid name."));
    if ( ! name.isNull() )
      container -> addBranch (name);
  }
}

void KPlayerNodeView::addToNewPlaylist (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToNewPlaylist\n";
#endif
  KPlayerNodeList list (getSelectedNodes());
  if ( ! list.isEmpty() )
    addToNewPlaylist (list);
}

void KPlayerNodeView::addToNewPlaylistUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToNewPlaylist urls\n";
  kdDebugTime() << " URLs   " << urls.count() << "\n";
#endif
  if ( playlist() -> checkUrls (urls) )
  {
    KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
    if ( ! list.isEmpty() )
    {
      addToNewPlaylist (list);
      list.releaseAll();
    }
  }
}

void KPlayerNodeView::addToNewPlaylist (const KPlayerNodeList& list)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToNewPlaylist list\n";
  kdDebugTime() << " Count  " << list.count() << "\n";
#endif
  KPlayerContainerNode* container = playlistActionList() -> node();
  KPlayerNodeNameValidator validator (container);
  QString name = KInputDialog::text (i18n("Add to new playlist"), i18n("Playlist name"),
    QString::null, 0, 0, 0, &validator, QString::null,
    i18n("Playlist name field allows you to enter a name for a new playlist. OK button will be enabled when you enter a unique and valid name."));
  if ( ! name.isNull() )
  {
    container -> addBranch (name);
    container = container -> getNodeById (name);
    if ( container )
      container -> append (list);
  }
}

void KPlayerNodeView::addToPlaylistsUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToPlaylists urls\n";
  kdDebugTime() << " URLs   " << urls.count() << "\n";
#endif
  if ( playlist() -> checkUrls (urls) )
  {
    KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
    if ( ! list.isEmpty() )
    {
      playlistActionList() -> node() -> append (list);
      list.releaseAll();
    }
  }
}

void KPlayerNodeView::addToPlaylists (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToPlaylists\n";
#endif
  KPlayerNodeList list (getSelectedNodes());
  if ( ! list.isEmpty() )
    playlistActionList() -> node() -> append (list);
}

void KPlayerNodeView::addToPlaylist (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToPlaylist\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  KPlayerNodeList list (getSelectedNodes());
  if ( ! list.isEmpty() && node -> isContainer() )
    ((KPlayerContainerNode*) node) -> append (list);
}

void KPlayerNodeView::addToCollectionUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToCollection urls\n";
  kdDebugTime() << " URLs   " << urls.count() << "\n";
#endif
  if ( playlist() -> checkUrls (urls) )
  {
    KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
    if ( ! list.isEmpty() )
    {
      KPlayerNode::root() -> getNodeByUrl ("kplayer:/collection") -> add (list);
      list.releaseAll();
    }
  }
}

void KPlayerNodeView::addToCollection (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::addToCollection\n";
#endif
  KPlayerNodeList list (getSelectedNodes());
  if ( ! list.isEmpty() )
    KPlayerNode::root() -> getNodeByUrl ("kplayer:/collection") -> add (list);
}

void KPlayerNodeView::editName (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::editName\n";
#endif
  KPlayerListViewItem* item = itemForNode (activeNode());
  if ( item )
  {
    int section = header() -> mapToSection (0);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Section " << section << "\n";
#endif
    startEditing (item, section);
  }
}

void KPlayerNodeView::edited (QListViewItem* item, const QString& value, int column)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::edited column " << column << " to '" << value << "'\n";
#endif
  int index = header() -> mapToIndex (column);
  if ( index >= 0 && index < attributeCount() )
  {
    const QString& name (attributeNames() [index]);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Column " << name << " " << index << "\n";
#endif
    nodeForItem (item) -> media() -> fromString (name, value);
  }
  setItemsRenameable (false);
  m_editing_item = 0;
  m_editing_column = 0;
  treeView() -> resetActiveNode();
}

void KPlayerNodeView::properties (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::properties\n";
#endif
  KPlayerNode* node = activeNode();
  if ( node && ! node -> hasProperties() && node -> isContainer() )
    node = ((KPlayerContainerNode*) node) -> origin();
  if ( node && node -> hasProperties() )
  {
    KPlayerPropertiesDialog* dialog;
    if ( ! node -> isContainer() )
      dialog = KPlayerPropertiesDialog::createDialog ((KPlayerTrackProperties*) node -> media());
    else
    {
      const QString& type = ((KPlayerDeviceProperties*) node -> media()) -> type();
      if ( type == "TV" )
        dialog = new KPlayerTVDevicePropertiesDialog;
      else if ( type == "DVB" )
        dialog = new KPlayerDVBDevicePropertiesDialog;
      else
        dialog = new KPlayerDevicePropertiesDialog;
    }
    dialog -> setup (node -> media() -> url());
    dialog -> exec();
    delete dialog;
  }
}

void KPlayerNodeView::moveUp (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::moveUp\n";
#endif
  moveUp (rootNode(), (KPlayerListViewItem*) firstChild());
}

void KPlayerNodeView::moveUp (KPlayerContainerNode* parent, KPlayerListViewItem* item)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::moveUp\n";
  kdDebugTime() << " URL    " << parent -> url() << "\n";
#endif
  KPlayerNodeList list;
  KPlayerNode *after = parent, *previous = 0;
  while ( item )
  {
    if ( item -> isOpen() && item -> firstChild() )
      moveUp ((KPlayerContainerNode*) item -> node(), (KPlayerListViewItem*) item -> firstChild());
    if ( item -> isSelected() )
    {
      if ( previous )
        list.append (item -> node());
    }
    else
    {
      if ( ! list.isEmpty() )
      {
        m_moving = true;
        parent -> move (list, after);
        m_moving = false;
        list.clear();
      }
      if ( previous )
        after = previous;
      previous = item -> node();
    }
    item = (KPlayerListViewItem*) item -> nextSibling();
  }
  if ( ! list.isEmpty() )
  {
    m_moving = true;
    parent -> move (list, after);
    m_moving = false;
    list.clear();
  }
}

void KPlayerNodeView::moveDown (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::moveDown\n";
#endif
  moveDown (rootNode(), (KPlayerListViewItem*) firstChild());
}

void KPlayerNodeView::moveDown (KPlayerContainerNode* parent, KPlayerListViewItem* item)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::moveDown\n";
  kdDebugTime() << " URL    " << parent -> url() << "\n";
#endif
  KPlayerNodeList list;
  while ( item )
  {
    if ( item -> isOpen() && item -> firstChild() )
      moveDown ((KPlayerContainerNode*) item -> node(), (KPlayerListViewItem*) item -> firstChild());
    KPlayerListViewItem* next = (KPlayerListViewItem*) item -> nextSibling();
    if ( item -> isSelected() )
      list.append (item -> node());
    else if ( ! list.isEmpty() )
    {
      m_moving = true;
      parent -> move (list, item -> node());
      m_moving = false;
      list.clear();
    }
    item = next;
  }
}

void KPlayerNodeView::remove (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::remove\n";
#endif
  remove (rootNode(), (KPlayerListViewItem*) firstChild());
}

void KPlayerNodeView::remove (KPlayerContainerNode* parent, KPlayerListViewItem* item)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::remove\n";
  kdDebugTime() << " URL    " << parent -> url() << "\n";
#endif
  KPlayerNodeList list;
  while ( item )
  {
    if ( item -> isSelected() )
      list.append (item -> node());
    else if ( item -> isOpen() && item -> firstChild() )
      remove ((KPlayerContainerNode*) item -> node(), (KPlayerListViewItem*) item -> firstChild());
    item = (KPlayerListViewItem*) item -> nextSibling();
  }
  if ( ! list.isEmpty() )
    parent -> remove (list);
}

void KPlayerNodeView::goToOrigin (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerNodeView::goToOrigin\n";
#endif
  KPlayerContainerNode* container = activeContainer();
  if ( container && container -> origin() )
    treeView() -> setActiveNode (container -> origin());
}

KPlayerPlaylist* KPlayerNodeView::playlist (void)
{
  return ((KPlayer*) kapp -> mainWidget()) -> playlist();
}

KPlayerListView::KPlayerListView (QWidget* parent, const char* name)
  : KPlayerNodeView (parent, name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Creating list view\n";
#endif
  config() -> setGroup ("Multimedia Library");
  m_attribute_order = config() -> hasKey ("Column Order") ? config() -> readListEntry ("Column Order")
    : KPlayerMedia::defaultOrder();
  QStringList::ConstIterator iterator (attributeOrder().begin());
  while ( iterator != attributeOrder().end() )
  {
    const QString& name (*iterator);
    if ( ! name.isEmpty() && config() -> hasKey ("Show Column " + name) )
      KPlayerMedia::info (name) -> setShow (config() -> readBoolEntry ("Show Column " + name));
    ++ iterator;
  }
}

KPlayerListView::~KPlayerListView()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying list view\n";
#endif
}

void KPlayerListView::initialize (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Initializing list view\n";
#endif
  KPlayerNodeView::initialize();
  setSelectionMode (QListView::Extended);
  connect (header(), SIGNAL (indexChange (int, int, int)), SLOT (headerIndexChange (int, int, int)));
  //connect (header(), SIGNAL (clicked (int)), SLOT (headerClicked (int)));
  //connect (header(), SIGNAL (sectionHandleDoubleClicked (int)), SLOT (headerAdjustColumn (int)));
  //connect (header(), SIGNAL (sizeChange(int, int, int)), SLOT (headerSizeChange (int, int, int)));
  resize (2000, height());
  m_home_media = KPlayerNode::root() -> nodeById ("file:" + QDir::homeDirPath()) -> media();
}

void KPlayerListView::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Terminating list view\n";
#endif
  config() -> setGroup ("Multimedia Library");
  if ( attributeOrder() == KPlayerMedia::defaultOrder() )
    config() -> deleteEntry ("Column Order");
  else
    config() -> writeEntry ("Column Order", attributeOrder());
  KPlayerPropertyInfoMap::ConstIterator iterator (KPlayerMedia::info().begin());
  while ( iterator != KPlayerMedia::info().end() )
  {
    const QString& name (iterator.key());
    KPlayerPropertyInfo* info = iterator.data();
    if ( info -> show() == info -> showByDefault() )
      config() -> deleteEntry ("Show Column " + name);
    else
      config() -> writeEntry ("Show Column " + name, info -> show());
    ++ iterator;
  }
  KPlayerNodeView::terminate();
}

void KPlayerListView::setupColumns (void)
{
  m_attribute_names.clear();
  m_available_attributes.clear();
  m_attribute_states.clear();
  m_column_states.clear();
  m_editable_attributes.clear();
  m_attribute_counts.clear();
  updateAttributes (rootNode() -> attributeCounts(), KPlayerPropertyCounts());
}

KPlayerListViewItem* KPlayerListView::createChild (KPlayerListViewItem* after, KPlayerNode* node)
{
  return ! node -> isContainer() ? new KPlayerListViewItem (this, after, node)
    : rootIsDecorated() ? (KPlayerListViewItem*) new KPlayerListViewFolderItem (this, after, node)
    : (KPlayerListViewItem*) new KPlayerListViewGroupItem (this, after, node);
}

KPlayerNodeView* KPlayerListView::sibling (void) const
{
  return library() -> treeView();
}

void KPlayerListView::connectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Connecting list view node\n";
  kdDebugTime() << " URL    " << rootNode() -> url().url() << "\n";
#endif
  loadColumnWidths();
  connectNodeCommon();
  connect (rootNode(), SIGNAL (attributesUpdated (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)),
    this, SLOT (updateAttributes (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)));
}

void KPlayerListView::disconnectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Disonnecting list view node\n";
  kdDebugTime() << " URL    " << rootNode() -> url() << "\n";
#endif
  saveColumnWidths();
  disconnectNodeCommon();
  disconnect (rootNode(), SIGNAL (attributesUpdated (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)),
    this, SLOT (updateAttributes (const KPlayerPropertyCounts&, const KPlayerPropertyCounts&)));
}

void KPlayerListView::updateActions (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Updating list view actions\n";
#endif
  QListViewItemIterator iterator (this, QListViewItemIterator::Selected);
  bool selection = iterator.current() != 0;
  KPlayerNode* node = activeNode();
  KPlayerContainerNode* container = activeContainer();
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Selection " << selection << "\n";
  if ( node )
    kdDebugTime() << " Active " << node -> url().url() << "\n";
  if ( container )
    kdDebugTime() << " Container " << container -> url().url() << "\n";
#endif
  action ("library_play") -> setEnabled (selection);
  action ("library_play_next") -> setEnabled (selection);
  bool enable = selection && rootNode() -> canQueue();
  action ("library_queue") -> setEnabled (enable);
  action ("library_queue_next") -> setEnabled (enable);
  enable = container && (container -> canAddLeaves() || container -> canAddBranches());
  library() -> emitEnableActionGroup ("library_add", enable);
  enable = container && container -> canAddLeaves();
  action ("library_add_files") -> setEnabled (enable);
  action ("library_add_url") -> setEnabled (enable);
  enable = container && container -> canAddBranches();
  KAction* a = action ("library_add_group");
  a -> setEnabled (enable);
  if ( container && container -> isDevices() )
  {
    a -> setText (i18n("&Device..."));
    a -> setStatusText (i18n("Adds a new device"));
    a -> setWhatsThis (i18n("Add device command allows you to add a new device. You will need to give the new device a unique name and specify the device path and type."));
  }
  else if ( container == playlistActionList() -> node() )
  {
    a -> setText (i18n("&Playlist..."));
    a -> setStatusText (i18n("Adds a new playlist"));
    a -> setWhatsThis (i18n("Add playlist command allows you to add a new playlist. You will need to give the new playlist a unique name."));
  }
  else
  {
    a -> setText (i18n("F&older..."));
    a -> setStatusText (i18n("Adds a new subfolder to the selected folder"));
    a -> setWhatsThis (i18n("Add folder command allows you to add a new subfolder to the selected folder. You will need to give the new subfolder a unique name."));
  }
  enable = selection && rootNode() -> canSaveAsPlaylist();
  library() -> emitEnableActionGroup ("library_add_to", enable);
  action ("library_add_to_new_playlist") -> setEnabled (enable);
  action ("library_add_to_playlists") -> setEnabled (enable);
  action ("library_add_to_collection") -> setEnabled (enable);
  action ("library_rename") -> setEnabled (node && node -> canRename());
  library() -> emitEnableActionGroup ("library_edit", node && ! node -> isContainer());
  action ("library_properties") -> setEnabled (node && (node -> hasProperties() || node -> isContainer()
    && ((KPlayerContainerNode*) node) -> origin() && ((KPlayerContainerNode*) node) -> origin() -> hasProperties()));
  action ("library_select_all") -> setEnabled (firstChild() != 0);
  enable = selection && rootNode() -> allowsCustomOrder();
  action ("library_move_up") -> setEnabled (enable);
  action ("library_move_down") -> setEnabled (enable);
  action ("library_remove") -> setEnabled (selection && rootNode() -> canRemove());
  library() -> emitEnableActionGroup ("library_columns", ! columnActionList() -> isEmpty());
  action ("library_go_down") -> setEnabled (container != rootNode());
  action ("library_go_to_origin") -> setEnabled (container -> origin() != 0);
  treeView() -> updateNavigationActions();
}

void KPlayerListView::selectAll (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::selectAll\n";
#endif
  KListView::selectAll (true);
}

void KPlayerListView::goUp (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::goUp\n";
#endif
  if ( rootNode() -> parent() -> parent() )
    treeView() -> setActiveNode (rootNode() -> parent());
}

void KPlayerListView::goDown (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::goDown\n";
#endif
  treeView() -> setActiveNode (activeContainer());
}

void KPlayerListView::toggleColumn (int index)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::toggleColumn\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  const QString& name (availableAttributes() [index]);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Name " << name << "\n";
#endif
  KPlayerPropertyInfo* info = KPlayerMedia::info (name);
  info -> setShow (! attributeStates() [name]);
  m_attribute_states.insert (name, info -> show());
  m_column_states.insert (name, info -> show());
  saveColumnWidths();
  if ( info -> show() )
  {
    index = setupColumn (name);
    for ( QListViewItem* item = firstChild(); item; item = item -> itemBelow() )
      ((KPlayerListViewItem*) item) -> update (name);
    loadColumnWidth (index);
    if ( index == columns() - 1 )
      loadColumnWidth (index - 1);
  }
  else
    cleanupColumn (name);
  //adjustLastColumn();
}

void KPlayerListView::editField (int index)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::editField\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  const QString& name (editableAttributes() [index]);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Name " << name << "\n";
#endif
  index = attributeNames().findIndex (name);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Index  " << index << "\n";
#endif
  if ( index < 0 )
  {
    index = availableAttributes().findIndex (name);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Index  " << index << "\n";
#endif
    if ( index >= 0 )
      toggleColumn (index);
    index = attributeNames().findIndex (name);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Index  " << index << "\n";
#endif
  }
  if ( index >= 0 )
  {
    index = header() -> mapToSection (index);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Column " << index << "\n";
#endif
    startEditing (currentItem(), index);
  }
}

void KPlayerListView::update (KPlayerListViewItem* item)
{
  if ( nodeForItem (item) -> media() == m_home_media )
    item -> setText (0, nodeForItem (item) -> id());
}

void KPlayerListView::updateAttributes (const KPlayerPropertyCounts& added, const KPlayerPropertyCounts& removed)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::updateAttributes\n";
#endif
  saveColumnWidths();
  if ( columns() )
    loadColumnWidth (columns() - 1);
  m_attribute_counts.add (added);
  m_attribute_counts.subtract (removed);
  KPlayerPropertyCounts::ConstIterator it = attributeCounts().begin();
  while ( it != attributeCounts().end() )
  {
    const QString& name (it.key());
    if ( ! attributeOrder().contains (name) )
      m_attribute_order.insert (m_attribute_order.find (""), name);
    ++ it;
  }
  bool edit = false;
  for ( QListViewItem* item = firstChild(); item; item = item -> itemBelow() )
    if ( ! nodeForItem (item) -> isContainer() )
    {
      edit = true;
      break;
    }
  QStringList::ConstIterator iterator (attributeOrder().begin());
  if ( columns() == 0 )
    setupColumn (*iterator);
  ++ iterator;
  while ( iterator != attributeOrder().end() )
  {
    const QString& name (*iterator);
    if ( ! name.isEmpty() )
    {
      KPlayerPropertyInfo* info = KPlayerMedia::info (name);
      bool editable = edit && info -> canEdit();
      if ( ! editable && (! attributeCounts().contains (name) || rootNode() -> isDirectory() && name == "Path") )
      {
        m_available_attributes.remove (name);
        m_attribute_states.insert (name, false);
      }
      else
      {
        if ( ! availableAttributes().contains (name) )
          insertAttribute (m_available_attributes, name);
        m_attribute_states.insert (name, info -> show() && attributeCounts().contains (name) || showColumn (name));
      }
      if ( ! editable )
        m_editable_attributes.remove (name);
      else if ( ! editableAttributes().contains (name) )
        insertAttribute (m_editable_attributes, name);
      if ( ! attributeNames().contains (name) && attributeStates() [name] )
        setupColumn (name);
      else if ( attributeNames().contains (name) && ! attributeStates() [name] )
        cleanupColumn (name);
    }
    ++ iterator;
  }
  loadColumnWidths();
  columnActionList() -> update();
  editActionList() -> update();
  if ( inFocus() )
    updateActions();
  else if ( sibling() -> inFocus() )
    sibling() -> updateActions();
}

void KPlayerListView::insertAttribute (QStringList& list, const QString& name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::insertAttribute\n";
  kdDebugTime() << " Name   " << name << "\n";
#endif
  list.remove (name);
  QStringList::ConstIterator iterator (attributeOrder().begin());
  QStringList::Iterator it (list.begin());
  while ( it != list.end() && iterator != attributeOrder().end() && *iterator != name )
  {
    if ( *it == *iterator )
      ++ it;
    ++ iterator;
  }
  list.insert (it, name);
}

void KPlayerListView::headerIndexChange (int section, int from, int to)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::headerIndexChange\n";
  kdDebugTime() << " Column " << section << "\n";
  kdDebugTime() << " From   " << from << "\n";
  kdDebugTime() << " To     " << to << "\n";
#endif
  if ( from == to || from == 0 || to == 0 )
    return;
  QString name (attributeNames() [from]);
  QString after (attributeNames() [from < to ? to : to - 1]);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Name   " << name << "\n";
  kdDebugTime() << " After  " << after << "\n";
#endif
  m_attribute_order.remove (name);
  QStringList::Iterator iterator (m_attribute_order.find (after));
  ++ iterator;
  m_attribute_order.insert (iterator, name);
  insertAttribute (m_attribute_names, name);
  insertAttribute (m_available_attributes, name);
  if ( KPlayerMedia::info (name) -> canEdit() )
    insertAttribute (m_editable_attributes, name);
  if ( from == columns() - 1 || to == columns() - 1 )
  {
    loadColumnWidth (from == columns() - 1 ? to : columns() - 2);
    //adjustLastColumn();
  }
  columnActionList() -> update();
  editActionList() -> update();
}

KPlayerNodeList KPlayerListView::getSelectedNodes (void) const
{
  KPlayerNodeList list;
  for ( QListViewItem* item = firstChild(); item; item = item -> nextSibling() )
    if ( item -> isSelected() )
      list.append (nodeForItem (item));
    else
      getSelectedNodes (item, list);
  return list;
}

void KPlayerListView::getSelectedNodes (QListViewItem* item, KPlayerNodeList& list) const
{
  for ( item = item -> firstChild(); item; item = item -> nextSibling() )
    if ( item -> isSelected() )
      list.append (nodeForItem (item));
    else
      getSelectedNodes (item, list);
}

void KPlayerListView::setSorting (int column, bool ascending)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::setSorting\n";
  kdDebugTime() << " Column " << column << "\n";
#endif
  if ( column >= 0 )
  {
    int index = header() -> mapToIndex (column);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Index  " << index << "\n";
#endif
    if ( index >= 0 )
    {
      const QString& name (attributeNames() [index]);
      config() -> setGroup ("Multimedia Library");
      config() -> writeEntry ("Sort Column", name);
      config() -> writeEntry ("Sort Ascending", ascending);
      KPlayerNode::setSorting (name, ascending);
      rootNode() -> setCustomOrder (false);
      for ( QListViewItem* item = firstChild(); item; item = item -> nextSibling() )
        ((KPlayerListViewItem*) item) -> resetCustomOrder();
    }
  }
  KPlayerNodeView::setSorting (column, ascending);
}

void KPlayerListView::cleanupColumn (const QString& name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::cleanupColumn\n";
#endif
  int index = attributeNames().findIndex (name);
  if ( index >= 0 )
  {
    m_attribute_names.remove (m_attribute_names.at (index));
    int section = header() -> mapToSection (index);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " -" << name << " " << index << "/" << section << "\n";
#endif
    removeColumn (section);
  }
}

int KPlayerListView::setupColumn (const QString& name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::setupColumn\n";
#endif
  insertAttribute (m_attribute_names, name);
  int index = attributeNames().findIndex (name);
  const QString& caption (KPlayerMedia::info (name) -> caption());
  int section = addColumn (caption.isEmpty() ? i18n(name.utf8()) : caption);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " +" << name << " " << index << "/" << section << "\n";
#endif
  if ( index != section )
    header() -> moveSection (section, index);
  return index;
}

void KPlayerListView::loadColumnWidth (int index)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::loadColumnWidth\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  int column = header() -> mapToSection (index);
  const QString& name (attributeNames() [index]);
  config() -> setGroup ("Multimedia Library");
  int width = config() -> readNumEntry ("Column " + name + " Width");
  if ( width <= 0 && index == 0 )
    width = 200;
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Column " << column << "\n";
  kdDebugTime() << " Name   " << name << "\n";
  kdDebugTime() << " Width  " << width << "\n";
#endif
  header() -> setStretchEnabled (false, index);
  setColumnWidthMode (column, QListView::Manual);
  if ( width <= 0 )
    adjustColumn (column);
  else
    setColumnWidth (column, width);
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Actual " << columnWidth (column) << "\n";
  kdDebugTime() << " Mode   " << columnWidthMode (column) << "\n";
  kdDebugTime() << " Stretch " << header() -> isStretchEnabled (index) << "\n";
#endif
}

#if 0
void KPlayerListView::adjustLastColumn (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::adjustLastColumn\n";
#endif
  int index = columns() - 1;
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Index  " << index << "\n";
#endif
  if ( index >= 0 )
  {
    if ( header() -> headerWidth() < viewport() -> width() )
      header() -> setStretchEnabled (true, index);
    /*int section = header() -> mapToSection (index);
    int previous_width = columnWidth (section);
    header() -> setStretchEnabled (true, index);
    int new_width = columnWidth (section);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Stretch column index " << index << " section " << section
      << " mode " << columnWidthMode (section) << " stretch " << header() -> isStretchEnabled (index)
      << " actual width " << new_width << "\n";
#endif
    if ( previous_width >= new_width + new_width )
    {
      setColumnWidth (section, previous_width);
#ifdef DEBUG_KPLAYER_NODEVIEW
      kdDebugTime() << " Fix up column index " << index << " section " << section
        << " mode " << columnWidthMode (section) << " stretch " << header() -> isStretchEnabled (index)
        << " actual width " << columnWidth (section) << "\n";
#endif
    }
    if ( ! horizontalScrollBar() -> isVisible() )
    {
      setColumnWidthMode (section, QListView::Maximum);
#ifdef DEBUG_KPLAYER_NODEVIEW
      kdDebugTime() << " Maximum mode index " << index << " section " << section
        << " mode " << columnWidthMode (section) << " stretch " << header() -> isStretchEnabled (index)
        << " column width " << columnWidth (section) << " view width " << width()
        << " viewport width " << viewport() -> width() << "\n";
#endif
    }*/
  }
}
#endif

void KPlayerListView::loadColumnWidths (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::loadColumnWidths\n";
#endif
  int index = 0;
  QStringList::ConstIterator iterator (attributeNames().begin());
  while ( iterator != attributeNames().end() )
  {
    loadColumnWidth (index);
    ++ iterator;
    ++ index;
  }
  //adjustLastColumn();
  config() -> setGroup ("Multimedia Library");
  QString name (config() -> readEntry ("Sort Column"));
  index = attributeNames().findIndex (name);
  bool ascending = config() -> readBoolEntry ("Sort Ascending", true);
  if ( index < 0 )
  {
    index = 0;
    ascending = true;
  }
  KPlayerNodeView::setSorting (header() -> mapToSection (index), ascending);
  KPlayerNode::setSorting (name, ascending);
}

void KPlayerListView::saveColumnWidths (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::saveColumnWidths\n";
#endif
  int index = 0;
  config() -> setGroup ("Multimedia Library");
  QStringList::ConstIterator iterator (attributeNames().begin());
  //while ( (index < columns() - 1 || ! header() -> isStretchEnabled (index)) && iterator != attributeNames().end() )
  while ( index < columns() && iterator != attributeNames().end() )
  {
    int section = header() -> mapToSection (index);
    int width = columnWidth (section);
#ifdef DEBUG_KPLAYER_NODEVIEW
    kdDebugTime() << " Column " << *iterator << " mode " << columnWidthMode (section)
      << " stretch " << header() -> isStretchEnabled (index) << " width <- " << width << "\n";
#endif
    if ( width > 0 )
      config() -> writeEntry ("Column " + *iterator + " Width", width);
    ++ iterator;
    ++ index;
  }
}

#if 0
void KPlayerListView::resizeContents (int width, int height)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::resizeContents " << width << "x" << height << "\n";
#endif
  KPlayerNodeView::resizeContents (width, height);
  adjustLastColumn();
}

void KPlayerListView::viewportResizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::viewportResizeEvent " << event -> size().width() << "x" << event -> size().height() << "\n";
#endif
  KPlayerNodeView::viewportResizeEvent (event);
  adjustLastColumn();
}
#endif

void KPlayerListView::loadHistoryEntry (const KPlayerHistoryEntry& entry)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::loadHistoryEntry\n";
  kdDebugTime() << " Expanded " << entry.m_expanded.count() << "\n";
  kdDebugTime() << " Selected " << entry.m_expanded.count() << "\n";
  kdDebugTime() << " Current container " << entry.m_current.url() << "\n";
  kdDebugTime() << " Current ID " << entry.m_current_id << "\n";
  kdDebugTime() << " Position " << entry.m_position.x() << "x" << entry.m_position.y() << "\n";
#endif
  setContentsPos (entry.m_position.x(), entry.m_position.y());
  KPlayerContainerNode* container;
  KPlayerNode* node;
  KURL::List::ConstIterator iterator (entry.m_expanded.begin());
  while ( iterator != entry.m_expanded.end() )
  {
    node = KPlayerNode::root() -> getNodeByUrl (*iterator);
    if ( node )
    {
      QListViewItem* item = itemForNode (node);
      if ( item )
        item -> setOpen (true);
    }
    ++ iterator;
  }
  QStringList::ConstIterator iditerator (entry.m_selected_ids.begin());
  iterator = entry.m_selected.begin();
  while ( iterator != entry.m_selected.end() && iditerator != entry.m_selected_ids.end() )
  {
    container = KPlayerNode::root() -> getNodeByUrl (*iterator);
    if ( container )
    {
      node = container -> nodeById (*iditerator);
      if ( node )
      {
        QListViewItem* item = itemForNode (node);
        if ( item )
          item -> setSelected (true);
      }
    }
    ++ iditerator;
    ++ iterator;
  }
  if ( ! entry.m_current.isEmpty() && ! entry.m_current_id.isNull() )
  {
    container = KPlayerNode::root() -> getNodeByUrl (entry.m_current);
    if ( container )
    {
      node = container -> nodeById (entry.m_current_id);
      if ( node )
      {
        QListViewItem* item = itemForNode (node);
        if ( item )
          setCurrentItem (item);
      }
    }
  }
}

void KPlayerListView::saveHistoryEntry (KPlayerHistoryEntry& entry) const
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerListView::saveHistoryEntry\n";
#endif
  entry.m_expanded.clear();
  entry.m_selected.clear();
  entry.m_selected_ids.clear();
  KPlayerListViewItem* item = (KPlayerListViewItem*) firstChild();
  while ( item )
  {
    if ( item -> isOpen() && item -> node() -> isContainer() )
      entry.m_expanded.append (item -> node() -> url());
    if ( item -> isSelected() )
    {
      entry.m_selected.append (item -> node() -> parent() -> url());
      entry.m_selected_ids.append (item -> node() -> id());
    }
    item = (KPlayerListViewItem*) item -> itemBelow();
  }
  item = (KPlayerListViewItem*) currentItem();
  entry.m_current = item ? item -> node() -> parent() -> url() : KURL();
  entry.m_current_id = item ? item -> node() -> id() : QString::null;
  entry.m_position.setX (contentsX());
  entry.m_position.setY (contentsY());
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Expanded " << entry.m_expanded.count() << "\n";
  kdDebugTime() << " Selected " << entry.m_expanded.count() << "\n";
  kdDebugTime() << " Current container " << entry.m_current.url() << "\n";
  kdDebugTime() << " Current ID " << entry.m_current_id << "\n";
  kdDebugTime() << " Position " << entry.m_position.x() << "x" << entry.m_position.y() << "\n";
#endif
}

/*
void KPlayerListView::headerSizeChange (int section, int, int)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Setting column " << attributeNames() [section] << " width mode to Manual\n";
#endif
  setColumnWidthMode (section, QListView::Manual);
}

void KPlayerListView::headerAdjustColumn (int section)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Setting column " << attributeNames() [section] << " width mode to Maximum\n";
#endif
  adjustColumn (section);
  setColumnWidthMode (section, QListView::Maximum);
}*/

KPlayerTreeView::KPlayerTreeView (QWidget* parent, const char* name)
  : KPlayerNodeView (parent, name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Creating tree view\n";
#endif
  m_navigating = false;
  m_current = m_history.begin();
}

KPlayerTreeView::~KPlayerTreeView()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying tree view\n";
#endif
}

void KPlayerTreeView::initialize (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Initializing tree view\n";
#endif
  KPlayerNodeView::initialize();
  setAlternateBackground (QColor());
  setSelectionMode (QListView::Single);
  header() -> hide();
  setRootNode (KPlayerNode::root());
  setSelected (firstChild(), true);
  if ( library() -> popupMenu() )
    connect (library() -> popupMenu(), SIGNAL (aboutToHide()), SLOT (popupMenuHidden()));
}

void KPlayerTreeView::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Terminating tree view\n";
#endif
  KPlayerNodeView::terminate();
}

void KPlayerTreeView::setupColumns (void)
{
  m_attribute_names.append ("Name");
  addColumn (i18n("Name"));
  setSorting (header() -> mapToSection (0));
}

KPlayerListViewItem* KPlayerTreeView::createChild (KPlayerListViewItem* item, KPlayerNode* node)
{
  return node -> isContainer() ? (KPlayerListViewItem*) new KPlayerTreeViewFolderItem (this, item, node) : 0;
}

KPlayerNodeView* KPlayerTreeView::sibling (void) const
{
  return library() -> listView();
}

void KPlayerTreeView::connectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Connecting tree view node\n";
  kdDebugTime() << " URL    " << rootNode() -> url().url() << "\n";
#endif
  connectNodeCommon();
}

void KPlayerTreeView::disconnectNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Disonnecting tree view node\n";
  kdDebugTime() << " URL    " << rootNode() -> url() << "\n";
#endif
  disconnectNodeCommon();
}

void KPlayerTreeView::setOpen (QListViewItem* item, bool open)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::setOpen " << open << "\n";
  kdDebugTime() << " URL    " << nodeForItem (item) -> url().url() << "\n";
#endif
  KPlayerNodeView::setOpen (item, open);
  adjustColumn (0);
}

void KPlayerTreeView::remove (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::remove\n";
#endif
  KPlayerNodeList list (getSelectedNodes());
  resetActiveNode();
  if ( ! list.isEmpty() )
    list.first() -> parent() -> remove (list);
}

void KPlayerTreeView::goBack (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::goBack\n";
#endif
  if ( m_current != m_history.end() )
    listView() -> saveHistoryEntry (*m_current);
  KPlayerContainerNode* node = 0;
  while ( ! node && m_current != m_history.begin() )
  {
    -- m_current;
    node = KPlayerNode::root() -> getNodeByUrl ((*m_current).m_url);
    if ( node )
    {
      m_navigating = true;
      setActiveNode (node);
      m_navigating = false;
    }
    else
    {
      m_current = m_history.remove (m_current);
      historyActionList() -> update();
    }
  }
#ifdef DEBUG_KPLAYER_NODEVIEW
  if ( m_current != m_history.end() )
    kdDebugTime() << "Current URL " << (*m_current).m_url.url() << "\n";
#endif
}

void KPlayerTreeView::goForward (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::goForward\n";
#endif
  if ( m_current != m_history.end() )
    listView() -> saveHistoryEntry (*m_current);
  KPlayerContainerNode* node = 0;
  while ( ! node && m_current != m_history.end() )
  {
    ++ m_current;
    if ( m_current != m_history.end() )
    {
      node = KPlayerNode::root() -> getNodeByUrl ((*m_current).m_url);
      if ( node )
      {
        m_navigating = true;
        setActiveNode (node);
        m_navigating = false;
      }
      else
      {
        m_current = m_history.remove (m_current);
        historyActionList() -> update();
      }
    }
  }
#ifdef DEBUG_KPLAYER_NODEVIEW
  if ( m_current != m_history.end() )
    kdDebugTime() << "Current URL " << (*m_current).m_url.url() << "\n";
#endif
}

void KPlayerTreeView::goToHistory (const KPlayerHistory::Iterator& iterator)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::goToHistory\n";
  kdDebugTime() << " URL    " << (*iterator).m_url.url() << "\n";
#endif
  if ( m_current != m_history.end() )
    listView() -> saveHistoryEntry (*m_current);
  if ( iterator != m_current )
  {
    KPlayerContainerNode* node = KPlayerNode::root() -> getNodeByUrl ((*iterator).m_url);
    if ( node )
    {
      m_current = iterator;
      m_navigating = true;
      setActiveNode (node);
      m_navigating = false;
    }
    else
    {
      m_history.remove (iterator);
      historyActionList() -> update();
    }
  }
#ifdef DEBUG_KPLAYER_NODEVIEW
  if ( m_current != m_history.end() )
    kdDebugTime() << "Current URL " << (*m_current).m_url.url() << "\n";
#endif
}

void KPlayerTreeView::updateActions (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Updating tree view actions\n";
#endif
  KPlayerContainerNode* node = activeNode();
  bool selection = currentItem() && currentItem() -> isSelected();
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << " Selection " << selection << "\n";
  if ( node )
    kdDebugTime() << " Active " << node -> url().url() << "\n";
#endif
  action ("library_play") -> setEnabled (selection);
  action ("library_play_next") -> setEnabled (selection);
  bool enable = selection && node -> canQueue();
  action ("library_queue") -> setEnabled (enable);
  action ("library_queue_next") -> setEnabled (enable);
  enable = selection && (node -> canAddLeaves() || node -> canAddBranches());
  library() -> emitEnableActionGroup ("library_add", enable);
  enable = selection && node -> canAddLeaves();
  action ("library_add_files") -> setEnabled (enable);
  action ("library_add_url") -> setEnabled (enable);
  enable = selection && node -> canAddBranches();
  KAction* a = action ("library_add_group");
  a -> setEnabled (enable);
  if ( node && node -> isDevices() )
  {
    a -> setText (i18n("&Device..."));
    a -> setStatusText (i18n("Adds a new device"));
    a -> setWhatsThis (i18n("Add device command allows you to add a new device. You will need to give the new device a unique name and specify the device path and type."));
  }
  else if ( node == playlistActionList() -> node() )
  {
    a -> setText (i18n("&Playlist..."));
    a -> setStatusText (i18n("Adds a new playlist"));
    a -> setWhatsThis (i18n("Add playlist command allows you to add a new playlist. You will need to give the new playlist a unique name."));
  }
  else
  {
    a -> setText (i18n("F&older..."));
    a -> setStatusText (i18n("Adds a new subfolder to the selected folder"));
    a -> setWhatsThis (i18n("Add folder command allows you to add a new subfolder to the selected folder. You will need to give the new subfolder a unique name."));
  }
  enable = selection && node -> canSaveAsPlaylist();
  library() -> emitEnableActionGroup ("library_add_to", enable);
  action ("library_add_to_new_playlist") -> setEnabled (enable);
  action ("library_add_to_playlists") -> setEnabled (enable);
  action ("library_add_to_collection") -> setEnabled (enable);
  action ("library_rename") -> setEnabled (selection && node -> canRename());
  library() -> emitEnableActionGroup ("library_edit", false);
  action ("library_properties") -> setEnabled (selection && (node -> hasProperties()
    || node -> origin() && node -> origin() -> hasProperties()));
  action ("library_select_all") -> setEnabled (node == currentNode());
  enable = selection && node -> allowsCustomOrder();
  action ("library_move_up") -> setEnabled (enable);
  action ("library_move_down") -> setEnabled (enable);
  action ("library_remove") -> setEnabled (selection && node -> parent() -> canRemove());
  library() -> emitEnableActionGroup ("library_columns", ! columnActionList() -> isEmpty());
  action ("library_go_down") -> setEnabled (false);
  action ("library_go_to_origin") -> setEnabled (selection && node -> origin() != 0);
  updateNavigationActions();
}

void KPlayerTreeView::updateNavigationActions (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Updating tree navigation actions\n";
#endif
  bool notempty = m_current != m_history.end();
  action ("library_go_back") -> setEnabled (m_current != m_history.begin());
  if ( notempty )
    ++ m_current;
  action ("library_go_forward") -> setEnabled (m_current != m_history.end());
  if ( notempty )
    -- m_current;
#ifdef DEBUG_KPLAYER_NODEVIEW
  if ( m_current != m_history.end() )
    kdDebugTime() << "Current URL " << (*m_current).m_url.url() << "\n";
#endif
  KPlayerContainerNode* parent = currentNode() -> parent();
  action ("library_go_up") -> setEnabled (parent && parent -> parent());
}

KPlayerNodeList KPlayerTreeView::getSelectedNodes (void) const
{
  KPlayerNodeList list;
  KPlayerNode* node = activeNode();
  if ( node )
    list.append (node);
  return list;
}

bool KPlayerTreeView::event (QEvent* event)
{
  bool result = KListView::event (event);
  if ( event -> type() == QEvent::ApplicationPaletteChange )
    setAlternateBackground (QColor());
  return result;
}

void KPlayerTreeView::setNodeOrder (KPlayerContainerNode* node)
{
  node -> customOrderByName();
}

KPlayerNode* KPlayerTreeView::currentNode (void) const
{
  return listView() -> rootNode();
}

void KPlayerTreeView::setActiveNode (KPlayerContainerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::setActiveNode\n";
  kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  stopEditing();
  sibling() -> stopEditing();
  QListViewItem* item = itemForNode (node, true);
  if ( item )
  {
    QListViewItem* child = item -> firstChild();
    if ( child )
    {
      while ( child -> nextSibling() )
        child = child -> nextSibling();
      if ( child -> itemBelow() )
        child = child -> itemBelow();
      ensureItemVisible (child);
    }
    else if ( item -> itemBelow() )
      ensureItemVisible (item -> itemBelow());
    ensureItemVisible (item);
    setSelected (item, true);
  }
}

void KPlayerTreeView::activeItemChanged (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Active tree view item changed\n";
#endif
  if ( m_popup_menu_shown )
    return;
  if ( ! m_navigating && m_current != m_history.end() )
    listView() -> saveHistoryEntry (*m_current);
  if ( currentItem() && activeNode() != currentNode() )
  {
    listView() -> setRootNode (activeNode());
    if ( m_navigating )
    {
      if ( m_current != m_history.end() )
        listView() -> loadHistoryEntry (*m_current);
    }
    else
    {
      if ( m_current != m_history.end() )
        ++ m_current;
      while ( m_current != m_history.end() )
        m_current = m_history.remove (m_current);
      KPlayerHistoryEntry entry;
      entry.m_url = activeNode() -> url();
      entry.m_name = activeNode() -> name();
      m_history.append (entry);
      -- m_current;
#ifdef DEBUG_KPLAYER_NODEVIEW
      kdDebugTime() << "Current URL " << (*m_current).m_url.url() << "\n";
#endif
    }
    historyActionList() -> update();
  }
}

void KPlayerTreeView::popupMenuHidden (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Popup menu hidden\n";
#endif
  m_popup_menu_shown = false;
  if ( currentItem() && activeNode() != currentNode() && renameLineEdit() -> isHidden() )
    QTimer::singleShot (0, this, SLOT (resetActiveNode()));
}

void KPlayerTreeView::resetActiveNode (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::resetActiveNode\n";
#endif
  if ( ! m_popup_menu_shown && renameLineEdit() -> isHidden() )
    setActiveNode ((KPlayerContainerNode*) currentNode());
}

void KPlayerTreeView::activateCurrentItem (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerTreeView::activateCurrentItem\n";
#endif
  if ( currentItem() && ! currentItem() -> isSelected() )
    setActiveNode (activeNode());
}

KPlayerLibrary::KPlayerLibrary (KActionCollection* ac, QWidget* parent, const char* name)
  : QSplitter (parent, name)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Creating library\n";
#endif
  m_ac = ac;
  KPlayerNode::initialize();
  KAction* action = new KAction (i18n("&Play"), 0, CTRL + Key_Return, m_ac, "library_play");
  action -> setStatusText (i18n("Plays the selected items"));
  action -> setWhatsThis (i18n("Play command plays the selected items."));
  action = new KAction (i18n("Play &Next"), 0, 0, m_ac, "library_play_next");
  action -> setStatusText (i18n("Plays the selected items after the currently played item"));
  action -> setWhatsThis (i18n("Play next command plays the selected items after the currently played item finishes playing."));
  action = new KAction (i18n("&Queue"), 0, 0, m_ac, "library_queue");
  action -> setStatusText (i18n("Queues the selected items"));
  action -> setWhatsThis (i18n("Queue command queues the selected items for playing."));
  action = new KAction (i18n("Queue Ne&xt"), 0, 0, m_ac, "library_queue_next");
  action -> setStatusText (i18n("Queues the selected items for playing after the currently played item"));
  action -> setWhatsThis (i18n("Queue next command queues the selected items for playing after the currently played item finishes playing."));
  action = new KAction (i18n("&Files..."), 0, 0, m_ac, "library_add_files");
  action -> setStatusText (i18n("Adds files to the selected folder"));
  action -> setWhatsThis (i18n("Add files command displays the standard Open File dialog and lets you choose a file or several files to add to the selected folder."));
  action = new KAction (i18n("&URL..."), 0, 0, m_ac, "library_add_url");
  action -> setStatusText (i18n("Adds a URL to the selected folder"));
  action -> setWhatsThis (i18n("Add URL command displays the standard Open URL dialog and lets you type or paste in a URL to add to the selected folder."));
  action = new KAction (i18n("F&older..."), 0, 0, m_ac, "library_add_group");
  action -> setStatusText (i18n("Adds a new subfolder to the selected folder"));
  action -> setWhatsThis (i18n("Add folder command allows you to add a new subfolder to the selected folder. You will need to give the new subfolder a unique name."));
  action = new KAction (i18n("&New Playlist..."), 0, 0, m_ac, "library_add_to_new_playlist");
  action -> setStatusText (i18n("Adds selected items to a new playlist"));
  action -> setWhatsThis (i18n("Add to new playlist command prompts for a new playlist name and adds the selected items to the new playlist."));
  action = new KAction (i18n("&Playlists"), 0, 0, m_ac, "library_add_to_playlists");
  action -> setStatusText (i18n("Adds selected items to the root playlist"));
  action -> setWhatsThis (i18n("Add to playlists command adds the selected items to the root playlist."));
  m_playlists = new KPlayerContainerActionList ("%1", i18n("Adds selected items to %1 playlist"),
    i18n("Add to playlist command adds the selected items to the %1 playlist."), this, "library_add_to_playlist");
  playlistActionList() -> setMaximumSize (configuration() -> playlistMenuSize());
  action = new KAction (i18n("&Collection"), 0, 0, m_ac, "library_add_to_collection");
  action -> setStatusText (i18n("Adds selected items to the collection"));
  action -> setWhatsThis (i18n("Add to collection command adds the selected items to the multimedia collection."));
  action = new KAction (i18n("&Rename"), 0, Key_F2, m_ac, "library_rename");
  action -> setStatusText (i18n("Lets you rename the selected item"));
  action -> setWhatsThis (i18n("Rename command starts edit mode for the current item in the multimedia library so you can change the item name."));
  action = new KAction (i18n("Propert&ies..."), 0, 0, m_ac, "library_properties");
  action -> setStatusText (i18n("Opens Properties dialog for the selected item"));
  action -> setWhatsThis (i18n("Properties command opens File Properties dialog for the current item in the multimedia library. See the File properties micro-HOWTO for details."));
  action = new KAction (i18n("&Select All"), 0, 0, m_ac, "library_select_all");
  action -> setStatusText (i18n("Selects all items in the current folder"));
  action -> setWhatsThis (i18n("Select all command selects all items in the current folder of the multimedia library."));
  action = new KAction (i18n("Move &Up"), "1uparrow", 0, m_ac, "library_move_up");
  action -> setStatusText (i18n("Moves the selected items up in the playlist"));
  action -> setWhatsThis (i18n("Move Up command moves the selected items up in a playlist. You can also move items around by clicking and dragging them with the left mouse button."));
  action = new KAction (i18n("Move &Down"), "1downarrow", 0, m_ac, "library_move_down");
  action -> setStatusText (i18n("Moves the selected items down in the playlist"));
  action -> setWhatsThis (i18n("Move Down command moves the selected items down in a playlist. You can also move items around by clicking and dragging them with the left mouse button."));
  action = new KAction (i18n("Re&move"), "editdelete", Key_Delete, m_ac, "library_remove");
  action -> setStatusText (i18n("Removes the selected items from the multimedia library"));
  action -> setWhatsThis (i18n("Remove command removes the selected items from the multimedia library."));
  m_go_to = new KPlayerContainerActionList ("%1", i18n("Opens %1 in the library window"),
    i18n("Go to %1 command opens the corresponding folder in the library window."), this, "library_go_to_list");
  connect (goToActionList(), SIGNAL(activated(KPlayerNode*)), SLOT(open(KPlayerNode*)));
  action = new KAction (i18n("&Back"), "back", ALT + Key_Left, m_ac, "library_go_back");
  action -> setStatusText (i18n("Opens the previous folder"));
  action -> setWhatsThis (i18n("Go back command opens the previous folder from the history."));
  action = new KAction (i18n("&Forward"), "forward", ALT + Key_Right, m_ac, "library_go_forward");
  action -> setStatusText (i18n("Opens the next folder"));
  action -> setWhatsThis (i18n("Go forward command opens the next folder from the history."));
  action = new KAction (i18n("&Up"), "up", ALT + SHIFT + Key_Up, m_ac, "library_go_up");
  action -> setStatusText (i18n("Opens the parent folder"));
  action -> setWhatsThis (i18n("Go up command opens the parent folder of the current folder."));
  action = new KAction (i18n("&Down"), "down", ALT + SHIFT + Key_Down, m_ac, "library_go_down");
  action -> setStatusText (i18n("Opens the selected folder"));
  action -> setWhatsThis (i18n("Go down command opens the selected folder."));
  action = new KAction (i18n("&To Origin"), 0, ALT + SHIFT + Key_Left, m_ac, "library_go_to_origin");
  action -> setStatusText (i18n("Opens the origin of the current folder"));
  action -> setWhatsThis (i18n("Go to origin command opens the origin folder of the current folder."));
  action = new KAction (i18n("&Edit"), 0, 0, this, SLOT (editPlaylist()), m_ac, "playlist_edit");
  action -> setStatusText (i18n("Opens the playlist in the multimedia library"));
  action -> setWhatsThis (i18n("Edit command opens the playlist in the multimedia library and allows you to edit it."));
  m_tree = new KPlayerTreeView (this);
  setResizeMode (treeView(), QSplitter::KeepSize);
  m_list = new KPlayerListView (this);
  config() -> setGroup ("Multimedia Library");
  int width = config() -> readNumEntry ("Tree View Width");
  if ( width )
  {
    QValueList<int> sizes;
    sizes.append (width);
    setSizes (sizes);
  }
  m_columns = new KPlayerToggleActionList (listView() -> availableAttributes(), listView() -> attributeStates(),
    i18n("Hide %1"), i18n("Show %1"), i18n("Hides %1 column"), i18n("Shows %1 column"),
    i18n("Hide %1 command hides the column."), i18n("Show %1 command shows the column."), this, "library_columns");
  m_edit = new KPlayerSimpleActionList (listView() -> editableAttributes(), "%1", i18n("Starts edit mode for %1 field"),
    i18n("Edit %1 starts edit mode for this field of the current item."), this, "library_edit");
  m_history_list = new KPlayerHistoryActionList (treeView() -> history(), treeView() -> currentHistoryEntry(),
    "%1", i18n("Opens %1 in the library window"),
    i18n("Go to %1 command opens the corresponding folder in the library window."), this, "library_history");
  connect (historyActionList(), SIGNAL(activated(const KPlayerHistory::Iterator&)), treeView(),
    SLOT(goToHistory(const KPlayerHistory::Iterator&)));
  m_last_view = m_tree;
}

KPlayerLibrary::~KPlayerLibrary()
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Destroying library\n";
#endif
}

void KPlayerLibrary::initialize (QPopupMenu* menu)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerLibrary::initialize\n";
#endif
  m_popup = menu;
  listView() -> initialize();
  treeView() -> initialize();
  playlistActionList() -> initialize ("kplayer:/playlists");
  goToActionList() -> initialize ("kplayer:/");
  disconnectActions();
}

void KPlayerLibrary::terminate (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerLibrary::terminate\n";
#endif
  config() -> setGroup ("Multimedia Library");
  config() -> writeEntry ("Tree View Width", sizes().first());
  playlistActionList() -> terminate();
  goToActionList() -> terminate();
  listView() -> terminate();
  treeView() -> terminate();
  KPlayerNode::terminate();
}

void KPlayerLibrary::setFocus (void)
{
  m_last_view -> setFocus();
}

void KPlayerLibrary::disconnectActions (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "Disconnecting actions\n";
#endif
  KAction* a = action ("library_play");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_play_next");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_queue");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_queue_next");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_add_files");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_add_url");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_add_group");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_add_to_new_playlist");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_add_to_playlists");
  a -> disconnect();
  a -> setEnabled (false);
  playlistActionList() -> disconnect (SIGNAL(activated(KPlayerNode*)));
  a = action ("library_add_to_collection");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_rename");
  a -> disconnect();
  a -> setEnabled (false);
  editActionList() -> disconnect (SIGNAL(activated(int)));
  a = action ("library_properties");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_select_all");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_move_up");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_move_down");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_remove");
  a -> disconnect();
  a -> setEnabled (false);
  columnActionList() -> disconnect (SIGNAL(activated(int)));
  a = action ("library_go_back");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_go_forward");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_go_up");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_go_down");
  a -> disconnect();
  a -> setEnabled (false);
  a = action ("library_go_to_origin");
  a -> disconnect();
  a -> setEnabled (false);
  emitEnableActionGroup ("library_add", false);
  emitEnableActionGroup ("library_add_to", false);
  emitEnableActionGroup ("library_columns", false);
  emitEnableActionGroup ("library_edit", false);
}

void KPlayerLibrary::open (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerLibrary::open\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  if ( node -> isContainer() )
  {
    treeView() -> setActiveNode ((KPlayerContainerNode*) node);
    emit makeVisible();
    treeView() -> ensureItemVisible (treeView() -> currentItem());
  }
}

void KPlayerLibrary::editPlaylist (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerLibrary::editPlaylist\n";
#endif
  open (treeView() -> playlist() -> nowplaying());
}

KPlayerLibraryWindow::KPlayerLibraryWindow (KActionCollection* ac, QWidget* parent, const char* name)
  : QDockWindow (parent, name)
{
  setWidget (new KPlayerLibrary (ac, this));
  setResizeEnabled (true);
  setCloseMode (QDockWindow::Always);
  setNewLine (true);
  setCaption (i18n("Multimedia Library"));
}

void KPlayerLibraryWindow::setFocus (void)
{
  library() -> setFocus();
}

void KPlayerLibraryWindow::hideEvent (QHideEvent* event)
{
  QDockWindow::hideEvent (event);
  library() -> disconnectActions();
  if ( isHidden() )
    emit windowHidden();
}
