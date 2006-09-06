/***************************************************************************
                          kplayerplaylist.cpp
                          --------------------
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

#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <krandomsequence.h>
#include <kurl.h>
#include <kurldrag.h>
#include <qdockarea.h>
#include <qfileinfo.h>
#include <qpopupmenu.h>
#include <qtooltip.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_PLAYLIST
#endif

#include "kplayerplaylist.h"
#include "kplayerplaylist.moc"
#include "kplayerengine.h"
#include "kplayernode.h"
#include "kplayernodeview.h"
#include "kplayerpropertiesdialog.h"
#include "kplayersettings.h"

KPlayerPlaylist::KPlayerPlaylist (KActionCollection* ac, QObject* parent, const char* name)
  : QObject (parent, name)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Creating playlist\n";
#endif
  m_ac = ac;
  m_current = 0;
  KAction* action = KStdAction::open (this, SLOT (filePlay()), m_ac);
  action -> setText (i18n("&Play..."));
  action -> setStatusText (i18n("Plays an existing file"));
  action -> setWhatsThis (i18n("Play command displays the standard Open File dialog and lets you choose a file or several files to put on the playlist and start playing."));
  action = new KAction (i18n("Play &URL..."), "fileopenurl", CTRL + Key_U, this, SLOT (filePlayUrl()), m_ac, "file_open_url");
  action -> setStatusText (i18n("Plays a URL"));
  action -> setWhatsThis (i18n("Play URL command displays the standard URL dialog and lets you type or paste in a URL to put on the playlist and start playing. The URL can be a remote network location, a local file path, or a KDE I/O Slave URL."));
  m_playlists = new KPlayerContainerActionList ("%1", i18n("Plays the %1 list"),
    i18n("Play List %1 starts playing the list."), this, "play_list");
  connect (playlistActionList(), SIGNAL (activated (KPlayerNode*)), SLOT (play (KPlayerNode*)));
  m_recent = new KPlayerNodeActionList ("%1", i18n("Plays %1"),
    i18n("Play Recent %1 starts playing the recently played item."), this, "file_recent");
  connect (recentActionList(), SIGNAL (activated (KPlayerNode*)), SLOT (play (KPlayerNode*)));
  refreshSettings();
  m_devices = new KPlayerDevicesActionList ("%1", i18n("Shows commands and options available for %1"),
    i18n("Submenu that shows commands and options available for %1."), this, "file_devices");
  action = new KAction (i18n("&Next"), "1rightarrow", ALT + Key_Down, this, SLOT (next()), m_ac, "player_next");
  action -> setStatusText (i18n("Plays the next item on the playlist"));
  action -> setWhatsThis (i18n("Next command starts playing the next item on the current playlist."));
  action = new KAction (i18n("P&revious"), "1leftarrow", ALT + Key_Up, this, SLOT (previous()), m_ac, "player_previous");
  action -> setStatusText (i18n("Plays the previous item on the playlist"));
  action -> setWhatsThis (i18n("Previous command starts playing the previous item on the current playlist."));
  KWidgetAction* widget = new KWidgetAction (new KPlayerPlaylistCombobox, i18n("Playlist"), 0, 0, 0, m_ac, "player_list");
  widget -> setAutoSized (true);
  widget -> setShortcutConfigurable (false);
  widget -> setStatusText (i18n("Shows playlist items and allows you to select an item to play"));
  widget -> setWhatsThis (i18n("Playlist combo box in the closed state displays the current playlist item. You can drop down the combo box to see the entire list and select a different item to load and play."));
  connect (playlist(), SIGNAL (activated(int)), SLOT (play(int)));
  QToolTip::add (playlist(), i18n("Playlist"));
  KToggleAction* toggle = new KToggleAction (i18n("&Loop"), "loop", 0, this, SLOT (loop()), m_ac, "player_loop");
  toggle -> setStatusText (i18n("Turns the option to loop through the list on/off"));
  toggle -> setWhatsThis (i18n("Loop command toggles the option to start playing items from the beginning of the playlist after playing the last item on the playlist."));
  if ( configuration() -> loop() )
    toggle -> setChecked (true);
  toggle = new KToggleAction (i18n("S&huffle"), "shuffle", 0, this, SLOT (shuffle()), m_ac, "player_shuffle");
  toggle -> setStatusText (i18n("Turns the option to play items in random order on or off"));
  toggle -> setWhatsThis (i18n("Shuffle command toggles the option to play items in a random order."));
  if ( configuration() -> shuffle() )
    toggle -> setChecked (true);
  action = new KAction (i18n("&Files..."), 0, 0, this, SLOT (addFiles()), m_ac, "playlist_add_files");
  action -> setStatusText (i18n("Adds files to the playlist"));
  action -> setWhatsThis (i18n("Add files command displays the standard Open File dialog and lets you choose a file or several files to add to the playlist."));
  action = new KAction (i18n("&URL..."), 0, 0, this, SLOT (addUrl()), m_ac, "playlist_add_url");
  action -> setStatusText (i18n("Adds a URL to the playlist"));
  action -> setWhatsThis (i18n("Add URL command displays the standard Open URL dialog and lets you type or paste in a URL to add to the playlist."));
  action = new KAction (i18n("&New Playlist..."), 0, 0, this, SLOT (addToNewPlaylist()), m_ac, "playlist_add_to_new_playlist");
  action -> setStatusText (i18n("Saves the playlist under a new name"));
  action -> setWhatsThis (i18n("Add to new playlist command prompts for a new playlist name and saves the playlist under the new name."));
  action = new KAction (i18n("&Playlists"), 0, 0, this, SLOT (addToPlaylists()), m_ac, "playlist_add_to_playlists");
  action -> setStatusText (i18n("Adds the playlist items to the root playlist"));
  action -> setWhatsThis (i18n("Add to playlists command adds the playlist items to the root playlist."));
  m_playlists_add = new KPlayerContainerActionList ("%1", i18n("Adds playlist items to %1 playlist"),
    i18n("Add to playlist command adds the playlist items to the %1 playlist."), this, "playlist_add_to_playlist");
  playlistAddActionList() -> setMaximumSize (configuration() -> playlistMenuSize());
  action = new KAction (i18n("&Collection"), 0, 0, this, SLOT (addToCollection()), m_ac, "playlist_add_to_collection");
  action -> setStatusText (i18n("Adds playlist items to the collection"));
  action -> setWhatsThis (i18n("Add to collection command adds the playlist items to the multimedia collection."));
  connect (process(), SIGNAL (stateChanged(KPlayerProcess::State, KPlayerProcess::State)),
    SLOT (playerStateChanged(KPlayerProcess::State, KPlayerProcess::State)));
  connect (configuration(), SIGNAL (updated()), this, SLOT (refreshSettings()));
  m_nowplaying = (KPlayerNowPlayingNode*) KPlayerNode::getNodeByUrl ("kplayer:/nowplaying");
  nowplaying() -> reference();
}

KPlayerPlaylist::~KPlayerPlaylist()
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Destroying playlist\n";
#endif
}

void KPlayerPlaylist::initialize (QPopupMenu* menu)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Initializing playlist\n";
#endif
  playlist() -> setPopupMenu (menu);
  attach (nowplaying());
  added (nowplaying(), nowplaying() -> nodes());
  playlistActionList() -> initialize ("kplayer:/playlists");
  recentActionList() -> initialize ("kplayer:/recent");
  devicesActionList() -> initialize ("kplayer:/devices");
}

void KPlayerPlaylist::terminate (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Terminating playlist\n";
#endif
  devicesActionList() -> terminate();
  recentActionList() -> terminate();
  playlistActionList() -> terminate();
  nowplaying() -> vacateAll();
  nowplaying() -> release();
}

void KPlayerPlaylist::enableNextPrevious (void) const
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::enableNextPrevious\n";
#endif
  action ("player_next") -> setEnabled (nextNode() || count() > 1 || count() > 0 && ! currentNode());
  action ("player_previous") -> setEnabled (count() > 1 || count() > 0 && ! currentNode());
}

void KPlayerPlaylist::updateActions (void) const
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::updateActions\n";
#endif
  bool enable = ! nodes().isEmpty();
  KPlayerPlaylist* that = (KPlayerPlaylist*) this;
  emit that -> enableActionGroup ("playlist_add_to", enable);
  action ("playlist_add_to_new_playlist") -> setEnabled (enable);
  action ("playlist_add_to_playlists") -> setEnabled (enable);
  action ("playlist_add_to_collection") -> setEnabled (enable);
  enableNextPrevious();
}

void KPlayerPlaylist::attach (KPlayerContainerNode* node)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::attach\n";
#endif
  node -> populate();
  connect (node, SIGNAL (nodesAdded (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)),
    SLOT (added (KPlayerContainerNode*, const KPlayerNodeList&, KPlayerNode*)));
  connect (node, SIGNAL (nodesRemoved (KPlayerContainerNode*, const KPlayerNodeList&)),
    SLOT (removed (KPlayerContainerNode*, const KPlayerNodeList&)));
  connect (node, SIGNAL (nodeUpdated (KPlayerContainerNode*, KPlayerNode*)),
    SLOT (updated (KPlayerContainerNode*, KPlayerNode*)));
}

void KPlayerPlaylist::detach (KPlayerContainerNode* node)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::detach\n";
#endif
  node -> disconnect (this);
  node -> vacate();
}

void KPlayerPlaylist::append (const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::append\n";
#endif
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
    if ( node -> isContainer() )
    {
      KPlayerContainerNode* container = (KPlayerContainerNode*) node;
      attach (container);
      append (container -> nodes());
    }
    else
    {
#ifdef DEBUG_KPLAYER_PLAYLIST
      if ( node -> url().url() != node -> metaurl().url() )
        kdDebugTime() << " Meta   " << node -> metaurl().url() << "\n";
#endif
      m_nodes.append (node);
    }
    ++ iterator;
  }
}

int KPlayerPlaylist::insert (const KPlayerNodeList& nodes, int index)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::insert\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
    if ( node -> isContainer() )
    {
      KPlayerContainerNode* container = (KPlayerContainerNode*) node;
      attach (container);
      index = insert (container -> nodes(), index);
    }
    else
    {
#ifdef DEBUG_KPLAYER_PLAYLIST
      if ( node -> url().url() != node -> metaurl().url() )
        kdDebugTime() << " Meta   " << node -> metaurl().url() << "\n";
#endif
      m_nodes.insert (index, node);
      playlist() -> insertItem (node -> name(), index);
      ++ index;
    }
    ++ iterator;
  }
  return index;
}

void KPlayerPlaylist::add (KPlayerPlaylistNodeList& previous)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Adding nodes to the playlist\n";
#endif
  KPlayerNodeListIterator prit (previous);
  KPlayerNode* node = m_nodes.first();
  while ( node )
  {
    while ( node && node != prit.current() )
    {
      if ( previous.findRef (node) < 0 )
      {
#ifdef DEBUG_KPLAYER_PLAYLIST
        kdDebugTime() << " Name   " << node -> name() << "\n";
#endif
        playlist() -> insertItem (node -> name(), nodes().at());
      }
      node = m_nodes.next();
    }
    if ( node )
      node = m_nodes.next();
    if ( prit.current() )
      ++ prit;
  }
}

void KPlayerPlaylist::added (KPlayerContainerNode*, const KPlayerNodeList& nodes, KPlayerNode* after)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::added\n";
  if ( ! nodes.isEmpty() )
    kdDebugTime() << " URL    " << nodes.getFirst() -> parent() -> url().url() << "\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  if ( configuration() -> shuffle() )
  {
    after = m_nodes.getLast();
    insert (nodes, count());
    randomize (after);
  }
  else
  {
    KPlayerPlaylistNodeList previous (m_nodes);
    append (nodes);
    m_nodes.sort();
    add (previous);
  }
  updateActions();
}

void KPlayerPlaylist::removed (KPlayerContainerNode*, const KPlayerNodeList& nodes)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::removed\n";
#endif
  KPlayerNode* next = currentNode();
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( node -> isContainer() )
      detach ((KPlayerContainerNode*) node);
    else if ( m_nodes.findRef (node) >= 0 )
    {
      bool last = node == m_nodes.getLast();
#ifdef DEBUG_KPLAYER_PLAYLIST
      kdDebugTime() << " URL    " << node -> url() << "\n";
      kdDebugTime() << " Last   " << last << "\n";
#endif
      playlist() -> removeItem (m_nodes.at());
      m_nodes.remove();
      if ( node == next )
      {
        m_current = 0;
        next = last ? 0 : m_nodes.current();
        last = next == m_nodes.getLast();
#ifdef DEBUG_KPLAYER_PLAYLIST
        kdDebugTime() << " Removed current node\n";
        if ( next )
          kdDebugTime() << " Next   " << next -> url() << "\n";
#endif
      }
      m_next.removeRef (node);
    }
    ++ iterator;
  }
  if ( next && next != currentNode() && m_next.isEmpty() )
  {
    m_next.append (next);
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << " Next   " << nextNode() -> url() << "\n";
#endif
  }
  updateActions();
}

void KPlayerPlaylist::updated (KPlayerContainerNode*, KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::updated\n";
  kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
  if ( m_nodes.findRef (node) >= 0 )
  {
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << " URL    " << node -> url().url() << "\n";
#endif
    playlist() -> renameItem (nodes().at(), node -> name());
  }
}

bool KPlayerPlaylist::checkUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::checkUrls\n";
#endif
  if ( urls.isEmpty() )
    return false;
  const KURL& url (urls.first());
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << " URL    " << url.url() << "\n";
#endif
  if ( urls.count() == 1 && url.path().isEmpty() && url.host().isEmpty() )
    return false;
  return true;
}

void KPlayerPlaylist::playUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::playUrls\n";
  kdDebugTime() << " URLs   " << urls.count() << "\n";
#endif
  if ( checkUrls (urls) )
  {
    KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
    if ( ! list.isEmpty() )
    {
      play (list);
      list.releaseAll();
    }
  }
}

void KPlayerPlaylist::queueUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::queueUrls\n";
  kdDebugTime() << " URLs   " << urls.count() << "\n";
#endif
  if ( checkUrls (urls) )
  {
    KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
    if ( ! list.isEmpty() )
    {
      queue (list);
      list.releaseAll();
    }
  }
}

void KPlayerPlaylist::playNextUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::playNextUrls\n";
  kdDebugTime() << " URLs   " << urls.count() << "\n";
#endif
  if ( checkUrls (urls) )
  {
    KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
    if ( ! list.isEmpty() )
    {
      playNext (list);
      list.releaseAll();
    }
  }
}

void KPlayerPlaylist::queueNextUrls (const KURL::List& urls)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::queueNextUrls\n";
  kdDebugTime() << " URLs   " << urls.count() << "\n";
#endif
  if ( checkUrls (urls) )
  {
    KPlayerNodeList list (KPlayerNodeList::fromUrlList (urls));
    if ( ! list.isEmpty() )
    {
      queueNext (list);
      list.releaseAll();
    }
  }
}

void KPlayerPlaylist::filePlay (void)
{
  playUrls (engine() -> openFiles (i18n("Play Files")));
}

void KPlayerPlaylist::filePlayUrl (void)
{
  playUrls (engine() -> openUrl (i18n("Play URL")));
}

void KPlayerPlaylist::play (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPlaylist::play\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  KPlayerNodeList list;
  list.append (node);
  play (list);
}

void KPlayerPlaylist::play (int index)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::play " << index << "\n";
#endif
  m_next.clear();
  m_next.append (m_nodes.at (index));
  play();
}

void KPlayerPlaylist::play (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::play\n";
#endif
  if ( nextNode() )
  {
    KURL url (nextNode() -> metaurl());
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << " Parent " << nextNode() -> parent() -> url().url() << "\n";
    kdDebugTime() << " URL    " << url.url() << "\n";
    kdDebugTime() << "        " << url.prettyURL (0, KURL::StripFileProtocol) << "'\n";
#endif
    if ( engine() -> stopped() )
      emit started();
    engine() -> load (url);
    if ( currentNode() != nextNode() )
    {
      setCurrentNode (nextNode());
      emit activated();
    }
    m_next.removeFirst();
  }
  else if ( process() -> state() == KPlayerProcess::Idle )
  {
    engine() -> stop();
    emit stopped();
  }
  enableNextPrevious();
}

void KPlayerPlaylist::play (const KPlayerNodeList& list)
{
  if ( ! list.isEmpty() )
  {
    playNext (list);
    next();
  }
}

void KPlayerPlaylist::playNext (const KPlayerNodeList& list)
{
  if ( ! list.isEmpty() )
  {
    nowplaying() -> replace (list);
    recent() -> addRecent (list);
    setNextNodes (nodes());
  }
}

void KPlayerPlaylist::queue (const KPlayerNodeList& list)
{
  if ( ! list.isEmpty() && list.getFirst() -> topLevelNode() != nowplaying() )
  {
    nowplaying() -> append (list);
    recent() -> addRecent (list);
  }
}

void KPlayerPlaylist::queueNext (const KPlayerNodeList& list)
{
  if ( ! list.isEmpty() )
    if ( list.getFirst() -> topLevelNode() == nowplaying() )
      setNextNodes (list);
    else
    {
      KPlayerNodeList previous (nodes());
      nowplaying() -> append (list);
      recent() -> addRecent (list);
      setNextNodes (nodes());
      KPlayerNodeListIterator iterator (previous);
      while ( KPlayerNode* node = iterator.current() )
      {
        m_next.removeRef (node);
        ++ iterator;
      }
    }
}

void KPlayerPlaylist::next (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::next\n";
  if ( ! settings() -> properties() -> url().isEmpty() )
    kdDebugTime() << " Loaded " << settings() -> properties() -> url().url() << "\n";
  if ( currentNode() )
    kdDebugTime() << " Old    " << currentNode() -> metaurl().url() << "\n";
#endif
  if ( isEmpty() )
    return;
  if ( ! nextNode() )
  {
    if ( currentNode() && m_nodes.findRef (currentNode()) >= 0 && m_nodes.next() )
      m_next.append (nodes().current());
    else
    {
      randomize();
      m_next.append (nodes().getFirst());
    }
  }
#ifdef DEBUG_KPLAYER_PLAYLIST
  if ( nextNode() )
    kdDebugTime() << " New    " << nextNode() -> metaurl().url() << "\n";
#endif
  play();
}

void KPlayerPlaylist::previous (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::previous\n";
  if ( ! settings() -> properties() -> url().isEmpty() )
    kdDebugTime() << " Loaded " << settings() -> properties() -> url().url() << "\n";
  if ( currentNode() )
    kdDebugTime() << " Old    " << currentNode() -> metaurl().url() << "\n";
#endif
  if ( isEmpty() )
    return;
  m_next.clear();
  if ( currentNode() && m_nodes.findRef (currentNode()) >= 0 && m_nodes.prev() )
    m_next.append (nodes().current());
  else
  {
    randomize();
    m_next.append (nodes().getLast());
  }
#ifdef DEBUG_KPLAYER_PLAYLIST
  if ( nextNode() )
    kdDebugTime() << " New    " << nextNode() -> metaurl().url() << "\n";
#endif
  play();
}

void KPlayerPlaylist::loop (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::loop\n";
#endif
  configuration() -> setLoop (toggleAction ("player_loop") -> isChecked());
}

void KPlayerPlaylist::shuffle (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::shuffle\n";
#endif
  configuration() -> setShuffle (toggleAction ("player_shuffle") -> isChecked());
  if ( configuration() -> shuffle() )
    randomize();
  else
  {
    playlist() -> clear();
    m_nodes.sort();
    KPlayerPlaylistNodeList empty;
    add (empty);
  }
  KPlayerNode* node = currentNode();
  if ( ! node || m_nodes.findRef (node) < 0 )
    node = nextNode();
  if ( ! node || m_nodes.findRef (node) < 0 )
    node = nodes().getFirst();
  setCurrentNode (node);
}

void KPlayerPlaylist::addFiles (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPlaylist::addFiles\n";
#endif
  nowplaying() -> append (kPlayerEngine() -> openFiles (i18n("Add Files")));
}

void KPlayerPlaylist::addUrl (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPlaylist::addUrl\n";
#endif
  nowplaying() -> append (kPlayerEngine() -> openUrl (i18n("Add URL")));
}

void KPlayerPlaylist::addToNewPlaylist (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPlaylist::addToNewPlaylist\n";
#endif
  if ( ! nodes().isEmpty() )
  {
    KPlayerNodeNameValidator validator (nowplaying());
    QString name = KInputDialog::text (i18n("Add to new playlist"), i18n("Playlist name"),
      QString::null, 0, 0, 0, &validator, QString::null,
      i18n("Playlist name field allows you to enter a name for a new playlist. OK button will be enabled when you enter a unique and valid name."));
    if ( ! name.isNull() )
    {
      nowplaying() -> addBranch (name);
      KPlayerContainerNode* container = nowplaying() -> getNodeById (name);
      if ( container )
        container -> append (nodes());
    }
  }
}

void KPlayerPlaylist::addToPlaylists (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPlaylist::addToPlaylists\n";
#endif
  if ( ! nodes().isEmpty() )
    KPlayerNode::getNodeByUrl ("kplayer:/playlists") -> append (nodes());
}

void KPlayerPlaylist::addToPlaylist (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPlaylist::addToPlaylist\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  if ( ! nodes().isEmpty() && node -> isContainer() )
    ((KPlayerContainerNode*) node) -> append (nodes());
}

void KPlayerPlaylist::addToCollection (void)
{
#ifdef DEBUG_KPLAYER_NODEVIEW
  kdDebugTime() << "KPlayerPlaylist::addToCollection\n";
#endif
  if ( ! nodes().isEmpty() )
    KPlayerNode::getNodeByUrl ("kplayer:/collection") -> add (nodes());
}

void KPlayerPlaylist::playerStateChanged (KPlayerProcess::State state, KPlayerProcess::State previous)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: State change: " << previous << " => " << state << "\n";
#endif
  if ( state != KPlayerProcess::Idle || previous == state )
    return;
  if ( ! engine() -> stopped() && (nextNode() || currentNode() != nodes().getLast() || configuration() -> loop()) )
    next();
  else
  {
    m_next.clear();
    play();
  }
}

void KPlayerPlaylist::refreshSettings (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::refreshSettings\n";
#endif
  playlistActionList() -> setMaximumSize (configuration() -> playlistMenuSize());
  recentActionList() -> setMaximumSize (configuration() -> recentMenuSize());
}

void KPlayerPlaylist::randomize (KPlayerNode* after)
{
  if ( ! configuration() -> shuffle() || isEmpty() )
    return;
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::randomize\n";
  if ( after )
    kdDebugTime() << " After  " << after -> url().url() << "\n";
#endif
  if ( after && m_nodes.findRef (after) < 0 )
    after = 0;
  if ( after )
    m_nodes.next();
  else
    m_nodes.first();
  KPlayerNodeList list;
  while ( nodes().current() && nodes().current() != after )
  {
    playlist() -> removeItem (nodes().at());
    list.append (m_nodes.take());
  }
  int offset = count();
  if ( list.first() )
  {
    m_nodes.append (list.take());
    playlist() -> insertItem (nodes().current() -> name(), nodes().at());
  }
  KRandomSequence rs;
  while ( list.first() )
  {
    m_nodes.insert (rs.getLong (count() - offset + 1) + offset, list.take());
    playlist() -> insertItem (nodes().current() -> name(), nodes().at());
  }
}

void KPlayerPlaylist::setCurrentNode (KPlayerNode* node)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::setCurrentNode\n";
  kdDebugTime() << " Node   " << node -> url().url() << "\n";
#endif
  if ( node && m_nodes.findRef (node) >= 0 )
  {
    m_current = node;
    playlist() -> setCurrentItem (nodes().at());
  }
}

void KPlayerPlaylist::setNextNodes (const KPlayerNodeList& nodes)
{
  m_next.clear();
  addNextNodes (nodes);
}

void KPlayerPlaylist::addNextNodes (const KPlayerNodeList& nodes)
{
  KPlayerNodeListIterator iterator (nodes);
  while ( KPlayerNode* node = iterator.current() )
  {
    if ( node -> isContainer() )
      addNextNodes (((KPlayerContainerNode*) node) -> nodes());
    else if ( m_nodes.findRef (node) >= 0 )
      m_next.append (node);
    ++ iterator;
  }
}

KPlayerPlaylistCombobox::KPlayerPlaylistCombobox (QWidget* parent, const char* name)
  : KComboBox (parent, name)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Creating playlist combobox\n";
#endif
}

void KPlayerPlaylistCombobox::renameItem (int index, const QString& name)
{
  if ( index == currentItem() )
    setCurrentText (name);
  else
  {
    removeItem (index);
    insertItem (name, index);
  }
}

QSize KPlayerPlaylistCombobox::sizeHint() const
{
  QSize hint = KComboBox::sizeHint();
  hint.setWidth (configuration() -> preferredSliderLength());
  return hint;
}

QSize KPlayerPlaylistCombobox::minimumSizeHint() const
{
  QSize hint = KComboBox::minimumSizeHint();
  hint.setWidth (configuration() -> minimumSliderLength());
  return hint;
}

void KPlayerPlaylistCombobox::contextMenuEvent (QContextMenuEvent* event)
{
#ifdef DEBUG_KPLAYER_WINDOW
  kdDebugTime() << "KPlayerPlaylistCombobox::contextMenuEvent\n";
#endif
  KComboBox::contextMenuEvent (event);
  m_popup -> popup (event -> globalPos());
  event -> accept();
}
