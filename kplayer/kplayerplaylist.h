/***************************************************************************
                          kplayerplaylist.h
                          ------------------
    begin                : Wed Sep 3 2003
    copyright            : (C) 2003-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERPLAYLIST_H
#define KPLAYERPLAYLIST_H

#include <kaction.h>
#include <kcombobox.h>
#include <klistview.h>
#include <qdockwindow.h>

#include "kplayerprocess.h"
#include "kplayerproperties.h"

class KPlayerPlaylistWidget;
class KPlayerProcess;

/**Playlist item class, contains the playlist item data.
  *@author kiriuja
  */
class KPlayerPlaylistItem : public QObject
{
  Q_OBJECT

public:
  /** The KPlayerPlaylistItem constructor. Adds a new list item to the list with the given URL.
    */
  KPlayerPlaylistItem (const KURL&);
  /** The KPlayerPlaylistItem destructor. Saves the playlist item data.
    */
  virtual ~KPlayerPlaylistItem();

  /** Returns the properties of the playlist entry.
   */
  KPlayerProperties* properties (void)
    { return m_properties; }

public slots:
  /** Receives the refresh signal from KPlayerProperties, updates the item.
   */
  virtual void refresh (void);

protected:
  /** The item properties.
   */
  KPlayerProperties* m_properties;
};

/**Playlist class, contains the list of playlist items.
  *@author kiriuja
  */
class KPlayerPlaylist : protected QPtrList<KPlayerPlaylistItem>
{
public:
  /** The KPlayerPlaylist constructor. Creates an empty playlist.
    */
  KPlayerPlaylist (void);
  /** The KPlayerPlaylist destructor.
    */
  virtual ~KPlayerPlaylist();

  /** Returns whether the playlist is empty.
   */
  bool isEmpty (void) const
   { return QPtrList<KPlayerPlaylistItem>::isEmpty(); }
  /** Returns the number of items on the playlist.
   */
  uint count (void) const
   { return QPtrList<KPlayerPlaylistItem>::count(); }
  /** Returns whether the given item is on the playlist.
   */
  bool has (KPlayerPlaylistItem*);
  /** Returns whether the playlist is temporary and has autodelete option set.
   */
  bool isTemporary (void) const
   { return autoDelete(); }
  /** Sets the playlist to temporary and also sets the autodelete option.
   */
  void setTemporary (bool temporary)
   { setAutoDelete (temporary); }

  /** Returns whether the playlist loops.
   */
  bool loop (void) const
   { return m_loop; }
  /** Sets the playlist loop option.
   */
  void setLoop (bool loop)
   { m_loop = loop; }
  /** Returns whether the playlist is shuffled.
   */
  bool shuffle (void) const
   { return m_shuffle; }
  /** Sets the playlist shuffle option and shuffles the playlist if the option is on.
   */
  void setShuffle (bool shuffle);

  /** Returns the index of the current item or -1 when at the list end.
   */
  int index (void) const
    { return QPtrList<KPlayerPlaylistItem>::at(); }
  /** Returns the current playlist item.
   */
  KPlayerPlaylistItem* current (void) const
    { return QPtrList<KPlayerPlaylistItem>::current(); }
  /** Returns the playlist item with the given index.
   */
  KPlayerPlaylistItem* item (int index)
    { return at (index); }
  /** Sets the current playlist item.
   */
  virtual void setCurrent (KPlayerPlaylistItem* item);
  /** Sets the current playlist item.
   */
  void setCurrent (int index);

  /** Goes to the next item on the playlist.
   */
  virtual bool next (void);
  /** Goes to the previous item on the playlist.
   */
  virtual bool previous (void);
  /** Removes all items from the playlist.
   */
  virtual void clear (void);

  /** Adds the given item to the end of the playlist.
    */
  virtual void add (KPlayerPlaylistItem*);
  /** Removes the given item from the playlist.
    */
  virtual void remove (KPlayerPlaylistItem*);
  /** Moves the given item after the other given item.
    */
  virtual void move (KPlayerPlaylistItem*, KPlayerPlaylistItem*);
  /** Renames the given item with the given name.
   */
  virtual void rename (KPlayerPlaylistItem*, const QString&);

  /** Shuffles the playlist entries.
   */
  virtual void randomize (KPlayerPlaylistItem* item = 0);

protected:
  /** The loop option.
   */
  bool m_loop;
  /** The shuffle option.
   */
  bool m_shuffle;
};

/**The playlist combobox widget.
  *@author kiriuja
  */
class KPlayerPlaylistCombobox : public KComboBox, public KPlayerPlaylist
{
  Q_OBJECT

public:
  /** The KPlayerPlaylistCombobox constructor.
    */
  KPlayerPlaylistCombobox (QWidget* parent = 0, const char* name = 0);

  /** Updates the name of the given item.
   */
  void setName (KPlayerPlaylistItem* item);

  /** Sets the current playlist item.
   */
  virtual void setCurrent (KPlayerPlaylistItem* item);

  /** Goes to the next item on the playlist.
   */
  virtual bool next (void);
  /** Goes to the previous item on the playlist.
   */
  virtual bool previous (void);
  /** Removes all items from the playlist.
   */
  virtual void clear (void);

  /** Adds the given item to the end of the playlist.
    */
  virtual void add (KPlayerPlaylistItem*);
  /** Removes the given item from the playlist.
    */
  virtual void remove (KPlayerPlaylistItem*);
  /** Moves the given item after the other given item.
    */
  virtual void move (KPlayerPlaylistItem*, KPlayerPlaylistItem*);
  /** Renames the given item with the given name.
   */
  virtual void rename (KPlayerPlaylistItem*, const QString&);

  /** Shuffles the playlist entries.
   */
  virtual void randomize (KPlayerPlaylistItem* item = 0);

  /** The size hint.
   */
  virtual QSize sizeHint() const;
  /** The minimum size hint.
   */
  virtual QSize minimumSizeHint() const;
};

/**Playlist view item class, good for insertion into a list view.
  *@author kiriuja
  */
class KPlayerPlaylistViewItem : public KListViewItem, public KPlayerPlaylistItem
{
public:
  /** The KPlayerPlaylistViewItem constructor. Adds a new list item to the list with the given URL.
    */
  KPlayerPlaylistViewItem (KPlayerPlaylistWidget*, const KURL&);
  /** The KPlayerPlaylistViewItem destructor.
    */
  virtual ~KPlayerPlaylistViewItem();

  /** Receives the refresh signal from KPlayerProperties, updates the item.
   */
  virtual void refresh (void);

protected:
  /** Updates name and information columns.
   */
  void refreshColumns (void);
};

/**Playlist combobox action suitable for insertion into a toolbar.
  *@author kiriuja
  */
class KPlayerPlaylistAction : public KWidgetAction
{
  Q_OBJECT

public:
  /** The KPlayerPlaylistAction constructor. Parameters are passed on to KAction.
    */
  KPlayerPlaylistAction (const QString& text, const KShortcut&, const QObject* receiver,
    const char* slot, KActionCollection* parent = 0, const char* name = 0);
  /** The KPlayerPlaylistAction destructor. Does nothing.
    */
  virtual ~KPlayerPlaylistAction();

  /** Returns a pointer to the playlist combobox object.
    */
  KPlayerPlaylistCombobox* playlist (void)
    { return (KPlayerPlaylistCombobox*) widget(); }

public slots:
  /** Selects the given item and emits the play signal.
   */
  void play (int index);
};

/**The KPlayer playlist widget.
  *@author kiriuja
  */
class KPlayerPlaylistWidget : public KListView
{
  Q_OBJECT

public: 
  KPlayerPlaylistWidget (KActionCollection* ac, QWidget* parent = 0, const char* name = 0);
  virtual ~KPlayerPlaylistWidget();

  KPlayerPlaylist* playlist (void) const
   { return m_list; }
  void setPlaylist (KPlayerPlaylist* list)
   { m_list = list; }
  KActionCollection* actionCollection (void) const
   { return m_ac; }
  void setActionCollection (KActionCollection* collection)
   { m_ac = collection; }

  /** Retrieves an action from the actionCollection by name.
    */
  KAction* action (const char* name) const
    { return m_ac -> action (name); }
  /** Retrieves a toggle action from the actionCollection by name.
    */
  KToggleAction* toggleAction (const char* name) const
    { return (KToggleAction*) action (name); }
  /** Retrieves the playlist action from the actionCollection.
    */
  KPlayerPlaylistAction* playlistAction (void) const
    { return (KPlayerPlaylistAction*) action ("playlist_list"); }
  /** Retrieves the recent files action from the actionCollection.
    */
  KRecentFilesAction* recentFilesAction (void) const
    { return (KRecentFilesAction*) action (KStdAction::stdName (KStdAction::OpenRecent)); }

  /** Creates actions and connects signals to slots.
   */
  void setupActions (void);

  /** Adds the given list of URLs to the playlist, loads it and optionally starts playback.
    */
  void openUrls (KURL::List);
  /** Loads the given URL and optionally starts playback.
    */
  void load (KURL);

  /** Returns the URL of the current item on the playlist or an empty
      URL if at the list end. */
  KURL current (void);
  /** Returns the URL of the next item on the playlist or an empty
      URL if at the list end. */
  KURL next (void);
  /** Returns the URL of the previous item on the playlist or an empty
      URL if at the list end. */
  KURL previous (void);

  /** Returns whether the playlist is locked.
   */
  bool locked (void) const
   { return m_locked; }
  /** Sets the option to lock the playlist and not accept more items.
   */
  void setLocked (bool locked)
   { m_locked = locked; }

  /** Returns whether only the current selection is played.
   */
  bool selectionOnly (void) const
   { return m_selection_only; }
  /** Sets the option to play only the current selection.
   */
  void setSelectionOnly (bool selection_only);

  /** Returns whether the playlist loops.
   */
  bool loop (void) const
   { return m_list -> loop(); }
  /** Sets the playlist loop option.
   */
  void setLoop (bool loop);

  /** Returns whether the playlist is shuffled.
   */
  bool shuffle (void) const
   { return m_list -> shuffle(); }
  /** Sets the playlist shuffle option and reloads the playlist if the option is off.
   */
  void setShuffle (bool shuffle);

  /** Returns whether the playlist is cleared before adding new entries.
   */
  bool transitory (void) const
   { return m_list -> isTemporary() ? m_temp_transitory : m_main_transitory; }
  /** Sets the option to clear the playlist before adding new entries.
   */
  void setTransitory (bool transitory)
   { (m_list -> isTemporary() ? m_temp_transitory : m_main_transitory) = transitory; }

  /** Save playlist contents and options to the configuration file.
   */
  void saveOptions (void) const;
  /** Read playlist contents and options and initialize the playlist.
    */
  void readOptions (void);
  /** Saves playlist options and removes all items.
   */
  void terminate (void);

  /** Enables or disables the next and previous actions according to the current state.
    */
  void enableNextPrevious (bool same_url = false);
  /** Enables or disables playlist actions according to the current state.
    */
  void enableActions (void);

public slots:
  /** Displays the Open File dialog and starts playing the chosen file.
    */
  void fileOpen (void);
  /** Displays the Open URL dialog and starts playing from the entered URL.
    */
  void fileOpenUrl (void);
  /** Opens a file from the recent files menu.
    */
  void fileOpenRecent (const KURL&);
  /** Plays the current item on the playlist.
   */
  void playlistPlay (void);
  /** Plays the next item on the playlist.
   */
  void playlistNext (void);
  /** Plays the previous item on the playlist.
   */
  void playlistPrevious (void);
  /** Toggles the option to play selection only or the entire playlist.
   */
  void playlistSelection (void);
  /** Locks the playlist so no new items are added or unlocks it.
   */
  void playlistLock (void);
  /** Toggles the option to loop through the list.
   */
  void playlistLoop (void);
  /** Toggles the option to play items in random order.
   */
  void playlistShuffle (void);
  /** Toggles the option to clear list before adding new entries.
   */
  void playlistTransitory (void);

  /** Starts playing the current item.
   */
  void play (void);
  /** Starts playing the given item.
   */
  void play (QListViewItem*);
  /** Opens the Properties dialog for the current item.
   */
  void properties (void);
  /** Starts renaming the current item.
   */
  void rename (void);
  /** Renames the given item.
   */
  virtual void rename (QListViewItem*, int);
  /** Renames the given item.
   */
  void rename (QListViewItem*, const QString&, int);
  /** Moves the given item.
   */
  void move (QListViewItem*, QListViewItem*, QListViewItem*);
  /** Adds items to the playlist after the given item.
   */
  void add (KURL::List&, KPlayerPlaylistViewItem*);
  /** Moves the currently selected items up in the playlist.
   */
  void moveUp (void);
  /** Moves the currently selected items down in the playlist.
   */
  void moveDown (void);
  /** Removes the currently selected items from the playlist.
   */
  void remove (void);
  /** Removes all items from the playlist.
   */
  virtual void clear (void);

  /** Receives the stateChanged signal from KPlayerProcess.
    */
  void playerStateChanged (KPlayerProcess::State, KPlayerProcess::State);

  /** Sets selection of the current item and updates playlist if the
      selectionOnly option is set. */
  void updateSelection (void);

  /** Adjusts the playlist to the user preferences.
   */
  void refreshSettings (void);

  virtual void resizeEvent (QResizeEvent*);

signals:
  /** Emitted when playback starts. */
  void started (void);

  /** Emitted when a new item is chosen from the playlist and loaded into the engine. */
  void activated (void);

  /** Emitted when an item is finished playing and no new item will be loaded. */
  void stopped (void);

protected:
  /** Accepts URL drags.
   */
  virtual bool acceptDrag (QDropEvent*) const;
  /** Handles the drop event.
   */
  virtual void contentsDropEvent (QDropEvent*);
  virtual void focusOutEvent (QFocusEvent*);
  virtual void focusInEvent (QFocusEvent*);
  virtual void windowActivationChange (bool);

  /** Reloads the current playlist if it was temporary.
    */
  void reload (void);
  /** Makes enough room for the given number of new items.
   */
  bool makeRoom (uint, KPlayerPlaylistViewItem* = 0);
  /** Updates the playlist actions.
   */
  virtual void showEvent (QShowEvent*);
  /** Updates the playlist actions.
   */
  virtual void hideEvent (QHideEvent*);

  /** Enables or disables the player shortcuts to avoid clashes with the line edit.
   */
  void enablePlayerShortcuts (bool);

  bool m_locked, m_selection_only, m_main_transitory, m_temp_transitory, m_shortcuts_enabled;
  bool m_enable_play, m_enable_pause, m_enable_stop;
  KPlayerPlaylist* m_list;
  KActionCollection* m_ac;
};

/**The KPlayer playlist window.
  *@author kiriuja
  */
class KPlayerPlaylistWindow : public QDockWindow
{
  Q_OBJECT

public: 
  KPlayerPlaylistWindow (KActionCollection* ac, QWidget* parent = 0, const char* name = 0);

  KPlayerPlaylistWidget* kPlayerPlaylistWidget (void)
    { return m_playlist_widget; }

protected:
  virtual void hideEvent (QHideEvent*);
  virtual void focusOutEvent (QFocusEvent*);
  virtual void focusInEvent (QFocusEvent*);
  virtual void windowActivationChange (bool);

  KPlayerPlaylistWidget* m_playlist_widget;

signals:
  void windowHidden (void);
};

#endif
