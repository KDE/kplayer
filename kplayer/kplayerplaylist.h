/***************************************************************************
                          kplayerplaylist.h
                          ------------------
    begin                : Wed Sep 3 2003
    copyright            : (C) 2003-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERPLAYLIST_H
#define KPLAYERPLAYLIST_H

#include <kaction.h>
#include <kcombobox.h>

#include "kplayerprocess.h"
#include "kplayernode.h"
#include "kplayernodeaction.h"

/**The playlist combobox widget.
  *@author kiriuja
  */
class KPlayerPlaylistCombobox : public QComboBox
{
  Q_OBJECT

public:
  /** The KPlayerPlaylistCombobox constructor. */
  KPlayerPlaylistCombobox (QWidget* parent = 0);

  /** Configuration. */
  KPlayerConfiguration* configuration (void) const
    { return KPlayerEngine::engine() -> configuration(); }

  /** Returns the popup menu. */
  QMenu* popupMenu (void) const
    { return m_popup; }
  /** Sets the popup menu. */
  void setPopupMenu (QMenu* menu)
    { m_popup = menu; }

  /** The size hint. */
  virtual QSize sizeHint() const;
  /** The minimum size hint. */
  virtual QSize minimumSizeHint() const;

protected:
  /** Displays the right click popup menu. */
  virtual void contextMenuEvent (QContextMenuEvent*);

  /** Popup menu. */
  QMenu* m_popup;
};

/**Playlist class, contains the list of playlist items.
  *@author kiriuja
  */
class KPlayerPlaylist : public QObject
{
  Q_OBJECT

public:
  /** The KPlayerPlaylist constructor. Creates an empty playlist. */
  KPlayerPlaylist (KActionCollection* ac, QObject* parent = 0);
  /** The KPlayerPlaylist destructor. */
  virtual ~KPlayerPlaylist();

  /** Initializes playlist. */
  void initialize (QMenu* menu);
  /** Releases referenced nodes. */
  void terminate (void);

  /** Engine. */
  KPlayerEngine* engine (void) const
    { return KPlayerEngine::engine(); }
  /** Configuration. */
  KPlayerConfiguration* configuration (void) const
    { return engine() -> configuration(); }
  /** Settings. */
  KPlayerSettings* settings (void) const
    { return engine() -> settings(); }
  /** Process. */
  KPlayerProcess* process (void) const
    { return engine() -> process(); }

  /** Now playing node. */
  KPlayerNowPlayingNode* nowplaying (void) const
    { return m_nowplaying; }

  /** Returns the action collection. */
  KActionCollection* actionCollection (void) const
    { return m_ac; }

  /** Retrieves an action from the actionCollection by name. */
  QAction* action (const char* name) const
    { return m_ac -> action (name); }
  /** Retrieves a toggle action from the actionCollection by name. */
  KToggleAction* toggleAction (const char* name) const
    { return (KToggleAction*) action (name); }

  /** Returns the playlist action list. */
  KPlayerContainerActionList* playlistActionList (void) const
    { return m_playlists; }

  /** Returns the recent action list. */
  KPlayerNodeActionList* recentActionList (void) const
    { return m_recent; }
  /** Recents node. */
  KPlayerRecentsNode* recent (void) const
    { return (KPlayerRecentsNode*) recentActionList() -> node(); }

  /** Returns the devices action list. */
  KPlayerDevicesActionList* devicesActionList (void) const
    { return m_devices; }

  /** Returns the playlist add action list. */
  KPlayerContainerActionList* playlistAddActionList (void) const
    { return m_playlists_add; }

  /** Returns a pointer to the playlist combobox object. */
  KPlayerPlaylistCombobox* playlist (void)
    { return m_playlist; }

  /** List of nodes. */
  const KPlayerPlaylistNodeList& nodes (void) const
    { return m_nodes; }
  /** Returns whether the playlist is empty. */
  bool isEmpty (void) const
   { return nodes().isEmpty(); }
  /** Returns the number of items on the playlist. */
  uint count (void) const
   { return nodes().count(); }

  /** Returns the current node. */
  KPlayerNode* currentNode (void) const
    { return m_current; }
  /** Sets the current node. */
  void setCurrentNode (KPlayerNode* node);

  /** Returns the next node. */
  KPlayerNode* nextNode (void) const
    { return m_next.first(); }
  /** Sets the next nodes. */
  void setNextNodes (const KPlayerNodeList& nodes);
  /** Adds the given nodes to the list of next nodes. */
  void addNextNodes (const KPlayerNodeList& nodes);

  /** Plays the given nodes. */
  void play (const KPlayerNodeList& list);
  /** Plays the given nodes after the currently played item finishes playing. */
  void playNext (const KPlayerNodeList& list);
  /** Queues the given nodes. */
  void queue (const KPlayerNodeList& list);
  /** Queues the given nodes for playing after the currently played item finishes playing. */
  void queueNext (const KPlayerNodeList& list);

  /** Puts the given list of URLs on the playlist and starts playback. */
  void playUrls (const KUrl::List& list);
  /** Puts the given list of URLs on the playlist and plays them after the currently played item finishes playing. */
  void playNextUrls (const KUrl::List& list);
  /** Adds the given list of URLs to the end of the playlist. */
  void queueUrls (const KUrl::List& list);
  /** Adds the given list of URLs to the playlist and plays them after the currently played item finishes playing. */
  void queueNextUrls (const KUrl::List& list);

  /** Shuffles the playlist entries. */
  void randomize (KPlayerNode* node = 0);

  /** Checks the given list of URLs for validity. */
  static bool checkUrls (const KUrl::List& list);

signals:
  /** Emitted when playback starts. */
  void started (void);

  /** Emitted when a new item is chosen from the playlist and loaded into the engine. */
  void activated (void);

  /** Emitted when an item is finished playing and no new item will be loaded. */
  void stopped (void);

  /** Emitted when an action group is enabled or disabled. */
  void enableActionGroup (const QString& name, bool enable);

public slots:
  /** Plays the given node. */
  void play (KPlayerNode* node);
  /** Plays the selected item. */
  void play (int index);
  /** Plays the current item. */
  void play (void);

  /** Plays the next item. */
  void next (void);
  /** Plays the previous item. */
  void previous (void);

  /** Toggles the option to loop through the playlist. */
  void loop (void);
  /** Toggles the option to play items in random order. */
  void shuffle (void);

  /** Displays the Open File dialog and starts playing the chosen file. */
  void filePlay (void);
  /** Displays the Open URL dialog and starts playing from the entered URL. */
  void filePlayUrl (void);

  /** Displays the Open File dialog and adds the chosen files to the playlist. */
  void addFiles (void);
  /** Displays the Open URL dialog and adds the entered URL to the playlist. */
  void addUrl (void);

  /** Saves the playlist as a new playlist. */
  void addToPlaylists (void);
  /** Adds the playlist items to an existing playlist. */
  void addToPlaylist (KPlayerNode*);
  /** Saves the playlist as a new folder in the collection. */
  void addToCollection (void);

protected slots:
  /** Adds the given nodes to the list. */
  void added (KPlayerContainerNode*, const KPlayerNodeList& nodes, KPlayerNode* after = 0);
  /** Removes the given nodes from the list. */
  void removed (KPlayerContainerNode*, const KPlayerNodeList& nodes);
  /** Updates the given node attributes. */
  void updated (KPlayerContainerNode* parent, KPlayerNode* node);

  /** Updates the playlist combobox. */
  void update (void);

  /** Plays the next item when appropriate. */
  void playerStateChanged (KPlayerProcess::State, KPlayerProcess::State);
  /** Adjusts the playlist to the user preferences. */
  void refreshSettings (void);

protected:
  /** Populates and connects the given node. */
  void attach (KPlayerContainerNode* node);
  /** Vacates and disconnects the given node. */
  void detach (KPlayerContainerNode* node);

  /** Appends the given nodes to the list. */
  void append (const KPlayerNodeList& nodes);
  /** Inserts the given nodes at the given location. */
  int insert (const KPlayerNodeList& nodes, int index);

  /** Enables or disables the next and previous actions according to the current state. */
  void enableNextPrevious (void) const;
  /** Updates actions according to the current state. */
  void updateActions (void) const;

  /** Now Playing node. */
  KPlayerNowPlayingNode* m_nowplaying;
  /** Current node. */
  KPlayerNode* m_current;
  /** Next nodes. */
  KPlayerNodeList m_next;
  /** List of nodes. */
  KPlayerPlaylistNodeList m_nodes;
  /** Action collection. */
  KActionCollection* m_ac;
  /** Set next node indicator. */
  bool m_set_next;
  /** Playlist combo box. */
  KPlayerPlaylistCombobox* m_playlist;
  /** Playlist action list. */
  KPlayerContainerActionList* m_playlists;
  /** Recent action list. */
  KPlayerNodeActionList* m_recent;
  /** Devices action list. */
  KPlayerDevicesActionList* m_devices;
  /** Playlist add action list. */
  KPlayerContainerActionList* m_playlists_add;
  /** Play requested indicator. */
  bool m_play_requested;
};

#endif
