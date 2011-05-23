/***************************************************************************
                          kplayernodeview.h
                          ------------------
    begin                : Mon Apr 18 2005
    copyright            : (C) 2005-2008 by Kirill Bulygin
    email                : quattro-kde@nuevoempleo.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERNODEVIEW_H
#define KPLAYERNODEVIEW_H

#include <KDialog>
#include <K3ListView>
#include <QDockWidget>
#include <QSplitter>
#include <QValidator>

#include "kplayeractionlist.h"
#include "kplayernode.h"
#include "ui_kplayerpropertiesdevice.h"

class KPlayerDeviceDialog;
class KPlayerContainerActionList;
class KPlayerNodeView;
class KPlayerListView;
class KPlayerTreeView;
class KPlayerLibrary;
class KPlayerLibraryWindow;
class KPlayerPlaylist;

/**Node name validator.
  *@author kiriuja
  */
class KPlayerNodeNameValidator : public QValidator
{
public:
  /** Constructor. References and stores the given container node. */
  KPlayerNodeNameValidator (KPlayerContainerNode* node, QObject* parent = 0);
  /** Destructor. Releases the container node. */
  virtual ~KPlayerNodeNameValidator();

  /** Validates node name. */
  virtual QValidator::State validate (QString& input, int& position) const;

protected:
  /** Container node. */
  KPlayerContainerNode* m_node;
};

/** The KPlayer device creation dialog page.
  *@author kiriuja
  */
class KPlayerPropertiesDevice : public QFrame, public Ui_KPlayerPropertiesDevicePage
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerPropertiesDevice (QWidget* parent = 0);
  /** Destructor. */
  virtual ~KPlayerPropertiesDevice();

  /** Device dialog. */
  KPlayerDeviceDialog* parent (void)
    { return (KPlayerDeviceDialog*) QFrame::parent(); }

  /** Adds a new device node. */
  void addDevice (void);

protected slots:
  /** Enables or disables the OK button. */
  void pathChanged (const QString&);
  /** Shows or hides channel list and file options. */
  void typeChanged (int);

protected:
  /** Devices node. */
  KPlayerContainerNode* m_node;
};

/** The KPlayer device creation dialog.
  *@author kiriuja
  */
class KPlayerDeviceDialog : public KDialog
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDeviceDialog (QWidget* parent = 0);
  /** Destructor. */
  virtual ~KPlayerDeviceDialog();

  /** Adds a new device node. */
  void addDevice (void)
    { ((KPlayerPropertiesDevice*) mainWidget()) -> addDevice(); }
};

/**Node view item. Represents a node in a node view.
  *@author kiriuja
  */
class KPlayerListViewItem : public K3ListViewItem
{
public:
  /** KPlayerListViewItem constructor. Adds a new item for the given node. */
  KPlayerListViewItem (K3ListView* parent, KPlayerNode* node)
    : K3ListViewItem (parent), m_node (node) { }
  /** KPlayerListViewItem constructor. Adds a new item for the given node after the given item. */
  KPlayerListViewItem (K3ListView* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : K3ListViewItem (parent, after), m_node (node) { }
  /** KPlayerListViewItem constructor. Adds a new item for the given node after the given item. */
  KPlayerListViewItem (KPlayerListViewItem* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : K3ListViewItem (parent, after), m_node (node) { }
  /** The KPlayerListViewItem destructor. */
  virtual ~KPlayerListViewItem();

  /** Initializes and updates the item. */
  virtual void initialize (void);
  /** Terminates and deletes the item. */
  virtual void terminate (void);

  /** Returns the node. */
  KPlayerNode* node (void) const
    { return m_node; }

  /** Returns the node of the given item. */
  KPlayerNode* nodeForItem (Q3ListViewItem* item) const
    { return ((KPlayerListViewItem*) item) -> node(); }

  /** Returns the node view. */
  KPlayerNodeView* nodeView (void) const
    { return (KPlayerNodeView*) K3ListViewItem::listView(); }

  /** Returns icon name. */
  virtual QString icon (void) const;

  /** Returns whether the node has children. */
  virtual bool hasChildren (void);

  /** Resets custom node order. */
  virtual void resetCustomOrder (void);

  /** Updates the item. */
  void update (void);

  /** Updates the given field of the item. */
  void update (const QString& name);

protected:
  /** Compares two list items. */
  virtual int compare (Q3ListViewItem* item, int column, bool ascending) const;

  /** Finds the item corresponding to the given node. */
  KPlayerListViewItem* itemForNode (KPlayerNode* node);

  /** Node this item represents. */
  KPlayerNode* m_node;
};

/**List view group item. Represents a container node in a list view.
  *@author kiriuja
  */
class KPlayerListViewGroupItem : public KPlayerListViewItem
{
public:
  /** KPlayerListViewGroupItem constructor. Adds a new item for the given node. */
  KPlayerListViewGroupItem (K3ListView* parent, KPlayerNode* node)
    : KPlayerListViewItem (parent, node) { }
  /** KPlayerListViewGroupItem constructor. Adds a new item for the given node after the given item. */
  KPlayerListViewGroupItem (K3ListView* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : KPlayerListViewItem (parent, after, node) { }
  /** KPlayerListViewGroupItem constructor. Adds a new item for the given node after the given item. */
  KPlayerListViewGroupItem (KPlayerListViewItem* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : KPlayerListViewItem (parent, after, node) { }
  /** The KPlayerListViewGroupItem destructor. */
  virtual ~KPlayerListViewGroupItem();

  /** Initializes and updates the item. */
  virtual void initialize (void);

  /** Returns the node view. */
  KPlayerListView* listView (void) const
    { return (KPlayerListView*) K3ListViewItem::listView(); }

  /** Returns the node. */
  KPlayerContainerNode* node (void) const
    { return (KPlayerContainerNode*) m_node; }
};

/**List view folder item. Represents an expandable container node in a list view.
  *@author kiriuja
  */
class KPlayerListViewFolderItem : public KPlayerListViewGroupItem
{
public:
  /** KPlayerListViewFolderItem constructor. Adds a new item for the given node. */
  KPlayerListViewFolderItem (K3ListView* parent, KPlayerNode* node)
    : KPlayerListViewGroupItem (parent, node) { }
  /** KPlayerListViewFolderItem constructor. Adds a new item for the given node after the given item. */
  KPlayerListViewFolderItem (K3ListView* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : KPlayerListViewGroupItem (parent, after, node) { }
  /** KPlayerListViewFolderItem constructor. Adds a new item for the given node after the given item. */
  KPlayerListViewFolderItem (KPlayerListViewItem* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : KPlayerListViewGroupItem (parent, after, node) { }
  /** The KPlayerListViewFolderItem destructor. */
  virtual ~KPlayerListViewFolderItem();

  /** Initializes and updates the item. */
  virtual void initialize (void);
  /** Terminates and deletes the item. */
  virtual void terminate (void);

  /** Returns icon name. */
  virtual QString icon (void) const;

  /** Returns whether the node has children. */
  virtual bool hasChildren (void);

  /** Resets custom node order. */
  virtual void resetCustomOrder (void);

  /** Populates or vacates the node groups. */
  virtual void setOpen (bool open);

  /** Adds the given nodes to the list. */
  void added (const KPlayerNodeList& nodes, KPlayerNode* after = 0);
  /** Removes the given nodes from the list. */
  void removed (const KPlayerNodeList& nodes);

protected:
  /** Connects common node signals to node view slots. */
  void connectNodeCommon (void);
  /** Disconnects common node signals from node view slots. */
  void disconnectNodeCommon (void);

  /** Connects node signals to list view slots. */
  virtual void connectNode (void);
  /** Disconnects node signals from list view slots. */
  virtual void disconnectNode (void);

  /** Terminates and deletes all children nodes. */
  void terminateChildren (void);

  /** Creates a new child item for the given node and inserts it after the given item. */
  virtual KPlayerListViewItem* createChild (KPlayerListViewItem* after, KPlayerNode* node);

  /** Populates the node with the children this item will display. */
  virtual void populateNode (void);
  /** Vacates the node children this item displayed. */
  virtual void vacateNode (void);

  /** Updates list view columns. */
  virtual void updateColumns (void);
};

/**Tree view folder item. Represents a container node in a tree view.
  *@author kiriuja
  */
class KPlayerTreeViewFolderItem : public KPlayerListViewFolderItem
{
public:
  /** KPlayerTreeViewFolderItem constructor. Adds a new item for the given node. */
  KPlayerTreeViewFolderItem (K3ListView* parent, KPlayerNode* node)
    : KPlayerListViewFolderItem (parent, node) { }
  /** KPlayerTreeViewFolderItem constructor. Adds a new item for the given node after the given item. */
  KPlayerTreeViewFolderItem (K3ListView* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : KPlayerListViewFolderItem (parent, after, node) { }
  /** KPlayerTreeViewFolderItem constructor. Adds a new item for the given node after the given item. */
  KPlayerTreeViewFolderItem (KPlayerListViewItem* parent, KPlayerListViewItem* after, KPlayerNode* node)
    : KPlayerListViewFolderItem (parent, after, node) { }
  /** The KPlayerTreeViewFolderItem destructor. */
  virtual ~KPlayerTreeViewFolderItem();

  /** Returns whether the node has children. */
  virtual bool hasChildren (void);

protected:
  /** Compares two list items. */
  virtual int compare (Q3ListViewItem* item, int column, bool ascending) const;

  /** Creates a new child item for the given node and inserts it after the given item. */
  virtual KPlayerListViewItem* createChild (KPlayerListViewItem* after, KPlayerNode* node);

  /** Connects node signals to list view slots. */
  virtual void connectNode (void);
  /** Disconnects node signals from list view slots. */
  virtual void disconnectNode (void);

  /** Populates the node with the children this item will display. */
  virtual void populateNode (void);
  /** Vacates the node children this item displayed. */
  virtual void vacateNode (void);

  /** Updates list view columns. */
  virtual void updateColumns (void);
};

/** History entry.
  * @author kiriuja
  */
class KPlayerHistoryEntry
{
public:
  /** Entry URL. */
  KUrl m_url;
  /** Entry name. */
  QString m_name;
  /** List of expanded items. */
  KUrl::List m_expanded;
  /** List of selected item containers. */
  KUrl::List m_selected;
  /** List of selected item IDs. */
  QStringList m_selected_ids;
  /** Current item container. */
  KUrl m_current;
  /** Current item ID. */
  QString m_current_id;
  /** Scrolling position. */
  QPoint m_position;
};

/** History.
  * @author kiriuja
  */
typedef QList<KPlayerHistoryEntry> KPlayerHistory;

/**History action list.
  *@author kiriuja
  */
class KPlayerHistoryActionList : public KPlayerActionList
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerHistoryActionList (KPlayerHistory& history, const KLocalizedString& text, const KLocalizedString& status,
    const KLocalizedString& whatsthis, QObject* parent, const QString& name);
  /** Destructor. */
  virtual ~KPlayerHistoryActionList();

  /** Updates the action list. */
  void update (int current);

signals:
  /** Emitted when the URL is selected. */
  void activated (int index);

protected:
  /** Selects the item with the given index by emitting the activated signal. */
  virtual void actionActivated (QAction* action, int index);

  /** History. */
  KPlayerHistory& m_history;
  /** Index of the bottom entry. */
  int m_bottom;
};

/**The node view.
  *@author kiriuja
  */
class KPlayerNodeView : public K3ListView
{
  Q_OBJECT

public: 
  /** Sets up the node view widget. */
  KPlayerNodeView (QWidget* parent = 0);
  /** Frees up resources. */
  virtual ~KPlayerNodeView();

  /** Initializes the view. */
  virtual void initialize (void);
  /** Frees up resources. */
  virtual void terminate (void);

  /** Main config. */
  KConfig* config (void) const
    { return KPlayerEngine::engine() -> config(); }

  /** Indicates whether the view has focus. */
  bool inFocus (void) const
    { return m_in_focus; }

  /** Returns the number of attributes. */
  int attributeCount (void) const
    { return m_attribute_names.count(); }
  /** Returns the list of attribute names. */
  const QStringList& attributeNames (void) const
    { return m_attribute_names; }

  /** Returns the library widget. */
  KPlayerLibrary* library (void) const
    { return (KPlayerLibrary*) parent(); }

  /** Returns the sibling node view. */
  virtual KPlayerNodeView* sibling (void) const = 0;
  /** Returns the list view. */
  KPlayerListView* listView (void) const;
  /** Returns the tree view. */
  KPlayerTreeView* treeView (void) const;

  /** Returns the playlist action list. */
  KPlayerContainerActionList* playlistActionList (void) const;
  /** Returns the column action list. */
  KPlayerToggleActionList* columnActionList (void) const;
  /** Returns the edit action list. */
  KPlayerSimpleActionList* editActionList (void) const;
  /** Returns the history action list. */
  KPlayerHistoryActionList* historyActionList (void) const;

  /** Returns the current root node. */
  KPlayerContainerNode* rootNode (void) const
    { return m_node; }
  /** Releases the current root node, sets it to the given one and displays its contents. */
  void setRootNode (KPlayerContainerNode* node);

  /** Returns the node of the given item. */
  KPlayerNode* nodeForItem (Q3ListViewItem* item) const
    { return ((KPlayerListViewItem*) item) -> node(); }

  /** Returns the active node. */
  KPlayerNode* activeNode (void) const
    { return currentItem() ? nodeForItem (currentItem()) : 0; }
  /** Returns the active container. */
  KPlayerContainerNode* activeContainer (void) const;

  /** Returns the current node. */
  virtual KPlayerNode* currentNode (void) const;

  /** Returns whether items are being moved within the view. */
  bool moving (void) const
    { return m_moving; }

  /** Stops editing an item that is going to be removed. */
  void itemTerminating (Q3ListViewItem* item);
  /** Changes the current item if the current one is among the children of item. */
  void keepUpCurrentItem (Q3ListViewItem* current, Q3ListViewItem* item);

  /** Returns the playlist object. */
  KPlayerPlaylist* playlist (void) const;

  /** Retrieves an action from the action collection by name. */
  QAction* action (const char* name) const;

  /** Checks whether the current item is being hidden and sets a new one. */
  virtual void setOpen (Q3ListViewItem* item, bool open);

  /** Sets column width and moves the edit box if any. */
  virtual void setColumnWidth (int column, int width);

  /** Fills the given list with selected nodes. */
  virtual KPlayerNodeList getSelectedNodes (void) const = 0;

  /** Updates actions according to the current selection. */
  virtual void updateActions (void) = 0;

  /** Start editing the given column of the given item. */
  void startEditing (Q3ListViewItem* item, int column);
  /** Terminates editing the current item. */
  void stopEditing (bool save = true, Qt::KeyboardModifiers state = 0);

  /** Adds the given list of URLs to a new playlist. */
  void addToNewPlaylistUrls (const KUrl::List& urls);
  /** Adds the given list of URLs to the root playlist. */
  void addToPlaylistsUrls (const KUrl::List& urls);
  /** Adds the given list of URLs to the collection. */
  void addToCollectionUrls (const KUrl::List& urls);

protected slots:
  /** Adds the given nodes to the list. */
  void added (KPlayerContainerNode* parent, const KPlayerNodeList& nodes, KPlayerNode* after = 0);
  /** Removes the given nodes from the list. */
  void removed (KPlayerContainerNode* parent, const KPlayerNodeList& nodes);
  /** Updates the given node attributes. */
  void updated (KPlayerContainerNode* parent, KPlayerNode* node);

  /** Expands, collapses or drills down a group node. */
  void itemExecuted (Q3ListViewItem* item);
  /** Shows the context menu. */
  void showContextMenu (K3ListView*, Q3ListViewItem*, const QPoint&);

  /** Updates the edited field of the current item. */
  void edited (Q3ListViewItem* item, const QString& value, int column);
  /** Moves line edit according to the new column size. */
  void moveLineEdit (int section, int from, int to);
  /** Arranges for the line edit to be moved according to the new content position. */
  void moveLineEdit (int x, int y);
  /** Moves line edit according to the current content position. */
  void moveLineEdit (void);

  /** Processes selection change signal. */
  void itemSelectionChanged (void);

  /** Plays the current selection. */
  void play (void);
  /** Plays the current selection after the currently played item finishes playing. */
  void playNext (void);
  /** Queues the current selection for playing. */
  void queue (void);
  /** Queues the current selection for playing after the currently played item finishes playing. */
  void queueNext (void);

  /** Displays the Open File dialog and adds the chosen files to the selected folder. */
  void addFiles (void);
  /** Displays the Open URL dialog and adds the entered URL to the selected folder. */
  void addUrl (void);
  /** Prompts for a new folder name and adds a new subfolder to the selected folder. */
  void addGroup (void);

  /** Saves the current selection as a new playlist. */
  void addToNewPlaylist (void);
  /** Adds the current selection to the root playlist. */
  void addToPlaylists (void);
  /** Adds the current selection to an existing playlist. */
  void addToPlaylist (KPlayerNode*);
  /** Adds the current selection to the collection. */
  void addToCollection (void);

  /** Starts rename mode for the current item. */
  void editName (void);
  /** Opens the Properties dialog for the current item. */
  void properties (void);

  /** Moves the current selection up in the playlist. */
  void moveUp (void);
  /** Moves the current selection down in the playlist. */
  void moveDown (void);
  /** Removes the current selection. */
  virtual void remove (void);

  /** Opens the origin node of the active node. */
  void goToOrigin (void);

protected:
  /** Sets up node view columns. */
  virtual void setupColumns (void) = 0;

  /** Creates a new child item for the given node and inserts it after the given item. */
  virtual KPlayerListViewItem* createChild (KPlayerListViewItem* after, KPlayerNode* node) = 0;

  /** Connects common node signals to node view slots. */
  void connectNodeCommon (void);
  /** Disconnects common node signals from node view slots. */
  void disconnectNodeCommon (void);

  /** Connects node signals to node view slots. */
  virtual void connectNode (void) = 0;
  /** Disconnects node signals from node view slots. */
  virtual void disconnectNode (void) = 0;

  /** Connects actions to this view. */
  void connectActions (void);
  /** Called when the active item changes. */
  virtual void activeItemChanged (void);

  /** Finds the place where the drop would occur. */
  void findDropTarget (const QPoint& pos, Q3ListViewItem*& parent, Q3ListViewItem*& after);
  /** Finds the place where the drop would occur. */
  virtual void findDrop (const QPoint& pos, Q3ListViewItem*& parent, Q3ListViewItem*& after);

  /** Accepts node and URL drags. */
  virtual bool acceptDrag (QDropEvent*) const;

  /** Sets custom node order if necessary. */
  virtual void setNodeOrder (KPlayerContainerNode* node);

  /** Handles the drop event. */
  virtual void contentsDropEvent (QDropEvent*);
  /** Handles the drag move event. */
  virtual void contentsDragMoveEvent (QDragMoveEvent*);
  /** Handles the drag enter event. */
  virtual void contentsDragEnterEvent (QDragEnterEvent*);
  /** Handles the drag leave event. */
  virtual void contentsDragLeaveEvent (QDragLeaveEvent*);

  /** Handles accel overrides. */
  virtual bool event (QEvent*);
  /** Handles the return key. */
  virtual void keyPressEvent (QKeyEvent*);

  /** Updates actions and connections when the view gets the focus. */
  virtual void focusInEvent (QFocusEvent* event);
  /** Updates actions and connections when the view loses the focus. */
  virtual void focusOutEvent (QFocusEvent* event);

  /** Stores indicator of right mouse button press. */
  virtual void contentsMousePressEvent (QMouseEvent* e);
  /** Stores the last active item. */
  virtual void contentsMouseReleaseEvent (QMouseEvent* e);

  /** Processes keyboard events for rename edit box. */
  virtual bool eventFilter (QObject* object, QEvent* event);

  /** Adds the given nodes to a new playlist. */
  KPlayerContainerNode* addToNewPlaylist (const KPlayerNodeList& list);

  /** Moves the selected children of the given node up in the playlist. */
  void moveUp (KPlayerContainerNode* parent, KPlayerListViewItem* item);
  /** Moves the selected children of the given node down in the playlist. */
  void moveDown (KPlayerContainerNode* parent, KPlayerListViewItem* item);
  /** Removes the selected children of the given node. */
  void remove (KPlayerContainerNode* parent, KPlayerListViewItem* item);

  /** Finds the item corresponding to the given node. */
  KPlayerListViewItem* itemForNode (KPlayerNode* node, bool open = false);

  /** Updates the given item as necessary. */
  virtual void update (KPlayerListViewItem* item);

  /** Currently displayed node. */
  KPlayerContainerNode* m_node;
  /** Last current node. */
  KPlayerNode* m_last_node;
  /** Names of visible node attributes. */
  QStringList m_attribute_names;
  /** Indicates whether items are being moved within the view. */
  bool m_moving;
  /** Indicates whether the view has focus. */
  bool m_in_focus;
  /** Indicates whether the popup menu is shown. */
  bool m_popup_menu_shown;
  /** Indicates whether a mouse button is pressed. */
  bool m_mouse_pressed;
  /** Indicates whether a drag is in progress. */
  bool m_dragging;
  /** Last active item. */
  KPlayerListViewItem* m_last_item;
  /** Item currently being edited. */
  KPlayerListViewItem* m_editing_item;
  /** Column currently being edited. */
  int m_editing_column;
  /** Drag node list. */
  KPlayerNodeList m_drag_node_list;
};

/**The list view.
  *@author kiriuja
  */
class KPlayerListView : public KPlayerNodeView
{
  Q_OBJECT

public: 
  /** Sets up the list view widget. */
  KPlayerListView (QWidget* parent = 0);
  /** Frees up resources. */
  virtual ~KPlayerListView();

  /** Initializes the list view. */
  virtual void initialize (void);
  /** Frees up resources. */
  virtual void terminate (void);

  /** Returns the sibling tree view. */
  virtual KPlayerNodeView* sibling (void) const;

  /** Returns the list of available attribute names. */
  const QStringList& availableAttributes (void) const
    { return m_available_attributes; }

  /** Returns the list of editable attribute names. */
  const QStringList& editableAttributes (void) const
    { return m_editable_attributes; }

  /** Returns the list of all possible attributes. */
  const QStringList& attributeOrder (void) const
    { return m_attribute_order; }

  /** Returns the attribute states. */
  const QMap<QString, bool>& attributeStates (void) const
    { return m_attribute_states; }

  /** Returns the attribute counts. */
  const KPlayerPropertyCounts& attributeCounts (void) const
    { return m_attribute_counts; }

  /** Fills the given list with selected nodes. */
  virtual KPlayerNodeList getSelectedNodes (void) const;
  /** Fills the given list with selected nodes under the given item. */
  void getSelectedNodes (Q3ListViewItem* item, KPlayerNodeList& list) const;

  /** Sets the sorting order. */
  virtual void setSorting (int column, bool ascending = true);

  /** Updates actions according to the current list view selection. */
  virtual void updateActions (void);

  /** Loads and applies column widths. */
  void loadColumnWidths (void);
  /** Saves column widths. */
  void saveColumnWidths (void);

  /** Loads history entry, expanding and selecting items and scrolling the view. */
  void loadHistoryEntry (const KPlayerHistoryEntry& entry);
  /** Saves expanded and selected items and scroll position to the given history entry. */
  void saveHistoryEntry (KPlayerHistoryEntry& entry) const;

  /** Returns whether the list is showing any media items. */
  bool showingMedia (void) const;

public slots:
  /** Updates the attribute counts, add and removes columns as necessary, keeping column widths. */
  void updateAttributes (const KPlayerPropertyCounts& added, const KPlayerPropertyCounts& removed);

protected slots:
  /** Selects all items in the current folder. */
  void selectAll (void);
  /** Starts edit mode for the chosen field of the current item. */
  void editField (int index);
  /** Hides or shows the column at the given index. */
  void toggleColumn (int index);
  /** Opens the parent node of the current root node. */
  void goUp (void);
  /** Opens the selected folder. */
  void goDown (void);

  /** Updates attribute lists to reflect changes in column order. */
  void headerIndexChange (int section, int from, int to);

protected:
  /** Sets up list view columns. */
  virtual void setupColumns (void);

  /** Creates a new child item for the given node and inserts it after the given item. */
  virtual KPlayerListViewItem* createChild (KPlayerListViewItem* after, KPlayerNode* node);

  /** Connects node signals to list view slots. */
  virtual void connectNode (void);
  /** Disconnects node signals from list view slots. */
  virtual void disconnectNode (void);

  /** Inserts an attribute into the list according to the order. */
  void insertAttribute (QStringList& list, const QString& name);

  /** Adds and sets up the column with the given name. */
  int setupColumn (const QString& name);
  /** Loads and applies column the given column width. */
  void loadColumnWidth (int index);
  /** Adjusts the last column width. */
  //void adjustLastColumn (void);
  /** Removes and cleans up the column with the given name. */
  void cleanupColumn (const QString& name);
  /** Returns whether the column with the given name has to be shown. */
  bool showColumn (const QString& name) const
    { return m_column_states.contains (name) && m_column_states [name]; }

  /** Updates the given item as necessary. */
  virtual void update (KPlayerListViewItem* item);

  /** Counts of visible node attributes. */
  KPlayerPropertyCounts m_attribute_counts;
  /** Names of available node attributes. */
  QStringList m_available_attributes;
  /** Names of editable node attributes. */
  QStringList m_editable_attributes;
  /** Names of all possible node attributes. */
  QStringList m_attribute_order;
  /** Attribute states. */
  QMap<QString, bool> m_attribute_states;
  /** Column states. */
  QMap<QString, bool> m_column_states;
  /** Home media. */
  KPlayerGenericProperties* m_home_media;
};

/**The tree view.
  *@author kiriuja
  */
class KPlayerTreeView : public KPlayerNodeView
{
  Q_OBJECT

public: 
  /** Sets up the tree view widget. */
  KPlayerTreeView (QWidget* parent = 0);
  /** Frees up resources. */
  virtual ~KPlayerTreeView();

  /** Initializes the tree view. */
  virtual void initialize (void);
  /** Frees up resources. */
  virtual void terminate (void);

  /** Returns the sibling list view. */
  virtual KPlayerNodeView* sibling (void) const;

  /** Returns the active node. */
  KPlayerContainerNode* activeNode (void) const
    { return (KPlayerContainerNode*) KPlayerNodeView::activeNode(); }
  /** Sets the active node. */
  void setActiveNode (KPlayerContainerNode* node);

  /** Returns the current node. */
  virtual KPlayerNode* currentNode (void) const;

  /** Returns the history. */
  KPlayerHistory& history (void)
    { return m_history; }

  /** Adjusts column width. */
  virtual void setOpen (Q3ListViewItem* item, bool open);

  /** Updates actions according to the current tree view selection. */
  virtual void updateActions (void);
  /** Updates navigation actions. */
  void updateNavigationActions (void);

public slots:
  /** Resets the active node. */
  void resetActiveNode (void);
  /** Activates the current item. */
  void activateCurrentItem (void);

protected slots:
  /** Sets timer to reset the active node if needed. */
  void popupMenuHidden (void);

  /** Removes the current selection. */
  virtual void remove (void);

  /** Goes to the previous URL in the history. */
  void goBack (void);
  /** Goes to the next URL in the history. */
  void goForward (void);
  /** Goes to the given URL in the history. */
  void goToHistory (int index);

protected:
  /** Sets up tree view columns. */
  virtual void setupColumns (void);

  /** Creates a new child item for the given node and inserts it after the given item. */
  virtual KPlayerListViewItem* createChild (KPlayerListViewItem* after, KPlayerNode* node);

  /** Connects node signals to tree view slots. */
  virtual void connectNode (void);
  /** Disconnects node signals from tree view slots. */
  virtual void disconnectNode (void);

  /** Populates the new current node and displays its contents in the list. */
  virtual void activeItemChanged (void);

  /** Fills the given list with selected nodes. */
  virtual KPlayerNodeList getSelectedNodes (void) const;

  /** Prevents K3ListView from resetting the alternate background. */
  virtual bool event (QEvent*);

  /** Sets custom node order by name. */
  virtual void setNodeOrder (KPlayerContainerNode* node);

  /** History. */
  KPlayerHistory m_history;
  /** Current history entry index. */
  int m_current;
  /** Navigation indicator. */
  bool m_navigating;
};

/**The library splitter widget.
  *@author kiriuja
  */
class KPlayerLibrary : public QSplitter
{
  Q_OBJECT

public: 
  /** Sets up the view widget. Adds tree and list children. */
  KPlayerLibrary (KActionCollection* ac, KPlayerPlaylist* playlist, QWidget* parent = 0);
  /** Frees up resources. */
  virtual ~KPlayerLibrary();

  /** Initializes library. */
  void initialize (QMenu* menu);
  /** Frees up resources and saves state. */
  void terminate (void);

  /** Returns the tree view. */
  KPlayerTreeView* treeView (void) const
    { return m_tree; }
  /** Returns the list view. */
  KPlayerListView* listView (void) const
    { return m_list; }

  /** Returns the popup menu. */
  QMenu* popupMenu (void) const
    { return m_popup; }

  /** Returns the parent dock widget. */
  KPlayerLibraryWindow* parent (void) const
    { return (KPlayerLibraryWindow*) QSplitter::parent(); }

  /** Configuration. */
  KConfig* config (void) const
    { return KPlayerEngine::engine() -> config(); }

  /** Returns the playlist. */
  KPlayerPlaylist* playlist (void) const
    { return m_playlist; }

  /** Returns the action collection. */
  KActionCollection* actionCollection (void) const
    { return m_ac; }

  /** Retrieves an action from the action collection by name. */
  QAction* action (const char* name) const
    { return m_ac -> action (name); }

  /** Returns the playlist action list. */
  KPlayerContainerActionList* playlistActionList (void) const
    { return m_playlists; }

  /** Returns the column action list. */
  KPlayerToggleActionList* columnActionList (void) const
    { return m_columns; }

  /** Returns the edit action list. */
  KPlayerSimpleActionList* editActionList (void) const
    { return m_edit; }

  /** Returns the go to action list. */
  KPlayerContainerActionList* goToActionList (void) const
    { return m_go_to; }

  /** Returns the history action list. */
  KPlayerHistoryActionList* historyActionList (void) const
    { return m_history_list; }

  /** Configuration. */
  KPlayerConfiguration* configuration (void) const
    { return KPlayerEngine::engine() -> configuration(); }

  /** Sets the last active node view. */
  void setLastActiveView (KPlayerNodeView* view)
    { m_last_view = view; }

  /** Remembers the widget height. */
  void rememberHeight (void)
    { m_height = height(); }

  /** Disconnects and disables actions. */
  void disconnectActions (void);

  /** Emits the enableActionGroup signal. */
  void emitEnableActionGroup (const QString& name, bool enable)
    { emit enableActionGroup (name, enable); }

  /** Returns the size hint for the workspace. */
  virtual QSize sizeHint (void) const;

signals:
  /** Emitted when the widget is resized. */
  void resized (void);
  /** Emitted when the library needs to be shown. */
  void makeVisible (void);
  /** Emitted when an action group is enabled or disabled. */
  void enableActionGroup (const QString& name, bool enable);

public slots:
  /** Shows library and opens the given node. */
  void open (KPlayerNode*);
  /** Shows library and opens the Now Playing node in it. */
  void editPlaylist (void);
  /** Disconnects actions when parent is hidden. */
  void parentVisibilityChanged (bool);

protected:
  /** Remembers the library height. Emits the resized signal. */
  virtual void resizeEvent (QResizeEvent* event);
  /** Sets the focus to the view that last had focus. */
  virtual void focusInEvent (QFocusEvent* event);
  /** Does nothing. */
  virtual void focusOutEvent (QFocusEvent* event);

  /** Action collection. */
  KActionCollection* m_ac;
  /** Playlist. */
  KPlayerPlaylist* m_playlist;
  /** Popup menu. */
  QMenu* m_popup;
  /** Tree view. */
  KPlayerTreeView* m_tree;
  /** List view. */
  KPlayerListView* m_list;
  /** Last active view. */
  KPlayerNodeView* m_last_view;
  /** Playlist action list. */
  KPlayerContainerActionList* m_playlists;
  /** Column action list. */
  KPlayerToggleActionList* m_columns;
  /** Edit action list. */
  KPlayerSimpleActionList* m_edit;
  /** Go to action list. */
  KPlayerContainerActionList* m_go_to;
  /** History action list. */
  KPlayerHistoryActionList* m_history_list;
  /** Preferred height. */
  int m_height;
};

/**The library window.
  *@author kiriuja
  */
class KPlayerLibraryWindow : public QDockWidget
{
  Q_OBJECT

public: 
  /** Constructor. */
  KPlayerLibraryWindow (KActionCollection* ac, KPlayerPlaylist* playlist, QWidget* parent);

  /** Returns the library. */
  KPlayerLibrary* library (void)
    { return (KPlayerLibrary*) widget(); }

  /** Returns whether the widget is visible. */
  bool visible (void) const
    { return ! isHidden() && m_visibility; }
  /** Returns whether the widget is docked and visible. */
  bool docked (void) const
    { return ! isFloating() && ! isHidden(); }

signals:
  /** Emitted when the widget is moved. */
  void moved (bool docked);
  /** Emitted when the widget is resized. */
  void resized (void);

protected slots:
  /** Sets the visibility flag. */
  void setVisibility (bool visibility);

protected:
  /** Emits the moved signal. */
  virtual void moveEvent (QMoveEvent*);
  /** Emits the resized signal. */
  virtual void resizeEvent (QResizeEvent*);

  /** Sets the focus to the library. */
  virtual void focusInEvent (QFocusEvent* event);
  /** Does nothing. */
  virtual void focusOutEvent (QFocusEvent* event);

  /** Visibility indicator. */
  bool m_visibility;
};

inline QAction* KPlayerNodeView::action (const char* name) const
{
  return library() -> action (name);
}

inline KPlayerPlaylist* KPlayerNodeView::playlist (void) const
{
  return library() -> playlist();
}

inline KPlayerListView* KPlayerNodeView::listView (void) const
{
  return library() -> listView();
}

inline KPlayerTreeView* KPlayerNodeView::treeView (void) const
{
  return library() -> treeView();
}

inline KPlayerContainerActionList* KPlayerNodeView::playlistActionList (void) const
{
  return library() -> playlistActionList();
}

inline KPlayerToggleActionList* KPlayerNodeView::columnActionList (void) const
{
  return library() -> columnActionList();
}

inline KPlayerSimpleActionList* KPlayerNodeView::editActionList (void) const
{
  return library() -> editActionList();
}

inline KPlayerHistoryActionList* KPlayerNodeView::historyActionList (void) const
{
  return library() -> historyActionList();
}

#endif
