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

#include <kdebug.h>
#include <klineedit.h>
#include <klocale.h>
#include <krandomsequence.h>
#include <kurl.h>
#include <kurldrag.h>
#include <qdockarea.h>
#include <qfileinfo.h>
#include <qtooltip.h>

#include "kplayerplaylist.h"
#include "kplayerplaylist.moc"
#include "kplayerengine.h"
#include "kplayerpropertiesdialog.h"
#include "kplayersettings.h"

#define COLUMN_NAME      0
#define COLUMN_TYPE      1
#define COLUMN_LENGTH    2
#define COLUMN_INFO      3
#define COLUMN_PATH      4
#define PLAYLIST_COLUMNS 5

#define DEBUG_KPLAYER_PLAYLIST

static KPlayerPlaylistCombobox* s_playlist = 0;

KPlayerPlaylistItem::KPlayerPlaylistItem (const KURL& url)
{
  m_properties = kPlayerEngine() -> reference (url);
  connect (m_properties, SIGNAL (refresh()), this, SLOT (refresh()));
  /*m_url = url;
  m_name = url.fileName().isEmpty() ? url.prettyURL()
    : QFileInfo (url.fileName()).baseName (true);
  m_length = m_video_bitrate = m_audio_bitrate = 0;*/
}

KPlayerPlaylistItem::~KPlayerPlaylistItem()
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Destroying playlist item\n";
#endif
  kPlayerEngine() -> dereference (m_properties);
}

void KPlayerPlaylistItem::refresh (void)
{
  s_playlist -> setName (this);
}

KPlayerPlaylist::KPlayerPlaylist (void)
{
  m_loop = m_shuffle = false;
}

KPlayerPlaylist::~KPlayerPlaylist()
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Destroying playlist\n";
#endif
}

void KPlayerPlaylist::setShuffle (bool shuffle)
{
  m_shuffle = shuffle;
  randomize();
}

void KPlayerPlaylist::randomize (KPlayerPlaylistItem* item)
{
  if ( ! shuffle() || isEmpty() )
    return;
  KPlayerPlaylistItem* cur = current();
  if ( item && findRef (item) < 0 )
    item = 0;
  if ( item )
    next();
  else
    first();
  QPtrList<KPlayerPlaylistItem> list;
  while ( current() && current() != item )
    list.append (take());
  int offset = count();
  if ( list.first() )
    append (list.take());
  KRandomSequence rs;
  while ( list.first() )
    insertAt (rs.getLong (count() - offset + 1) + offset, list.take());
  if ( cur )
    setCurrent (cur);
}

void KPlayerPlaylist::setCurrent (KPlayerPlaylistItem* item)
{
  if ( ! item )
    return;
  KPlayerPlaylistItem* cur = current();
  int index = findRef (item);
  if ( index < 0 && cur )
    findRef (cur);
}

void KPlayerPlaylist::setCurrent (int index)
{
  setCurrent (at (index));
}

bool KPlayerPlaylist::has (KPlayerPlaylistItem* item)
{
  if ( ! item )
    return false;
  KPlayerPlaylistItem* cur = current();
  int index = findRef (item);
  findRef (cur);
  return index >= 0;
}

bool KPlayerPlaylist::next (void)
{
  bool ok = false;
  if ( ! current() && first() || QPtrList<KPlayerPlaylistItem>::next() )
    ok = true;
  else if ( m_loop && first() )
  {
    randomize();
    first();
    ok = true;
  }
  if ( ! ok )
    last();
  return ok;
}

bool KPlayerPlaylist::previous (void)
{
  bool ok = false;
  if ( ! current() && last() || prev() )
    ok = true;
  else if ( m_loop && last() )
  {
    randomize();
    last();
    ok = true;
  }
  if ( ! ok )
    first();
  return ok;
}

void KPlayerPlaylist::add (KPlayerPlaylistItem* item)
{
  if ( item )
    append (item);
}

void KPlayerPlaylist::remove (KPlayerPlaylistItem* item)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylist::remove\n";
#endif
  KPlayerPlaylistItem* cur = current();
  int index = findRef (item);
  if ( index >= 0 )
    QPtrList<KPlayerPlaylistItem>::remove();
  if ( cur != item )
    setCurrent (cur);
  else if ( current() )
    setCurrent (current());
}

void KPlayerPlaylist::move (KPlayerPlaylistItem* item, KPlayerPlaylistItem* after)
{
  KPlayerPlaylistItem* cur = current();
  int to = after ? findRef (after) + 1 : 0;
  int from = findRef (item);
  if ( from >= 0 )
    insert (to, take());
  if ( cur )
    setCurrent (cur);
}

void KPlayerPlaylist::clear (void)
{
  QPtrList<KPlayerPlaylistItem>::clear();
}

void KPlayerPlaylist::rename (KPlayerPlaylistItem* item, const QString& s)
{
  item -> properties() -> setName (s);
  item -> properties() -> save();
}

KPlayerPlaylistCombobox::KPlayerPlaylistCombobox (QWidget* parent, const char* name)
  : KComboBox (parent, name)
{
  s_playlist = this;
}

QSize KPlayerPlaylistCombobox::sizeHint() const
{
  QSize hint = KComboBox::sizeHint();
  //kdDebugTime() << "KPlayerPlaylistCombobox size hint " << hint.width() << "x" << hint.height() << "\n";
  hint.setWidth (kPlayerSettings() -> preferredSliderLength());
  return hint;
}

QSize KPlayerPlaylistCombobox::minimumSizeHint() const
{
  QSize hint = KComboBox::minimumSizeHint();
  //kdDebugTime() << "KPlayerPlaylistCombobox minimum size hint " << hint.width() << "x" << hint.height() << "\n";
  hint.setWidth (kPlayerSettings() -> minimumSliderLength());
  return hint;
}

void KPlayerPlaylistCombobox::randomize (KPlayerPlaylistItem* item)
{
  if ( ! shuffle() || isEmpty() )
    return;
  KPlayerPlaylistItem* cur = current();
  KPlayerPlaylist::randomize (item);
  KComboBox::clear();
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Randomize: " << KPlayerPlaylist::count() << " items\n";
#endif
  for ( item = first(); item; item = QPtrList<KPlayerPlaylistItem>::next() )
  {
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << "Playlist: Randomize: " << index() << ": " << item -> properties() -> name() << "\n";
#endif
    insertItem (item -> properties() -> name());
  }
  if ( cur )
    setCurrent (cur);
}

void KPlayerPlaylistCombobox::setCurrent (KPlayerPlaylistItem* item)
{
  if ( ! item )
    return;
  KPlayerPlaylistItem* cur = current();
  int index = findRef (item);
  if ( index < 0 )
    findRef (cur);
  else
    setCurrentItem (index);
}

bool KPlayerPlaylistCombobox::next (void)
{
  bool ok = KPlayerPlaylist::next();
  if ( ok )
    setCurrentItem (index());
  return ok;
}

bool KPlayerPlaylistCombobox::previous (void)
{
  bool ok = KPlayerPlaylist::previous();
  if ( ok )
    setCurrentItem (index());
  return ok;
}

void KPlayerPlaylistCombobox::add (KPlayerPlaylistItem* item)
{
  if ( ! item )
    return;
  KPlayerPlaylist::add (item);
  insertItem (item -> properties() -> name());
  setCurrentItem (index());
}

void KPlayerPlaylistCombobox::remove (KPlayerPlaylistItem* item)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylistCombobox::remove\n";
#endif
  KPlayerPlaylistItem* cur = current();
  int index = findRef (item);
  if ( index >= 0 )
  {
    QPtrList<KPlayerPlaylistItem>::remove();
    removeItem (index);
  }
  if ( cur != item )
    setCurrent (cur);
  else if ( current() )
    setCurrent (current());
}

void KPlayerPlaylistCombobox::move (KPlayerPlaylistItem* item, KPlayerPlaylistItem* after)
{
  KPlayerPlaylistItem* cur = current();
  int to = after ? findRef (after) + 1 : 0;
  int from = findRef (item);
  if ( from >= 0 && to >= 0 )
  {
    if ( to > from )
      to --;
    insert (to, take());
    QString txt (text (from));
    removeItem (from);
    insertItem (txt, to);
  }
  if ( cur )
    setCurrent (cur);
}

void KPlayerPlaylistCombobox::clear (void)
{
  KPlayerPlaylist::clear();
  KComboBox::clear();
}

void KPlayerPlaylistCombobox::setName (KPlayerPlaylistItem* item)
{
  KPlayerPlaylistItem* cur = current();
  int index = findRef (item);
  if ( index >= 0 )
  {
    setCurrentItem (index);
    setCurrentText (item -> properties() -> name());
  }
  if ( cur )
    setCurrent (cur);
}

void KPlayerPlaylistCombobox::rename (KPlayerPlaylistItem* item, const QString& s)
{
  KPlayerPlaylist::rename (item, s);
  setName (item);
}

KPlayerPlaylistViewItem::KPlayerPlaylistViewItem (KPlayerPlaylistWidget* parent, const KURL& url)
  : KListViewItem (parent), KPlayerPlaylistItem (url)
{
  refreshColumns();
  setText (COLUMN_TYPE, properties() -> type());
  setText (COLUMN_PATH, url.isLocalFile() ? url.path() : url.prettyURL());
}

KPlayerPlaylistViewItem::~KPlayerPlaylistViewItem()
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Destroying playlist view item\n";
#endif
}

void KPlayerPlaylistViewItem::refreshColumns (void)
{
  setText (COLUMN_NAME, properties() -> name());
  setText (COLUMN_LENGTH, timeString (properties() -> length()));
  QString info;
  if ( ! properties() -> originalSize().isEmpty() )
    info = QString::number (properties() -> originalSize().width()) + "x" + QString::number (properties() -> originalSize().height()) + " ";
  if ( properties() -> framerate() > 0 )
    info += QString::number (properties() -> framerate()) + " " + i18n("fps") + " ";
  if ( properties() -> videoBitrate() > 0 )
    info += QString::number (properties() -> videoBitrate()) + " " + i18n("kbps") + " ";
  if ( ! properties() -> videoCodecValue().isEmpty() )
    info += properties() -> videoCodecValue() + " ";
  if ( properties() -> audioBitrate() > 0 )
    info += QString::number (properties() -> audioBitrate()) + " " + i18n("kbps") + " ";
  if ( ! properties() -> audioCodecValue().isEmpty() )
    info += properties() -> audioCodecValue();
  setText (COLUMN_INFO, info);
}

void KPlayerPlaylistViewItem::refresh (void)
{
  KPlayerPlaylistItem::refresh();
  refreshColumns();
}

KPlayerPlaylistAction::KPlayerPlaylistAction (const QString& text, const KShortcut& cut,
    const QObject* receiver, const char* slot, KActionCollection* parent, const char* name)
  : KWidgetAction (new KPlayerPlaylistCombobox (0, name), text, cut, receiver, slot, parent, name)
{
  setAutoSized (true);
  setShortcutConfigurable (false);
  connect (playlist(), SIGNAL (activated(int)), this, SLOT (play(int)));
  if ( ! text.isEmpty() )
    QToolTip::add (widget(), text);
}

KPlayerPlaylistAction::~KPlayerPlaylistAction()
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Destroying playlist action\n";
#endif
}

void KPlayerPlaylistAction::play (int index)
{
  playlist() -> KPlayerPlaylist::setCurrent (index);
  emit activated();
}

KPlayerPlaylistWindow::KPlayerPlaylistWindow (KActionCollection* ac, QWidget* parent, const char* name)
  : QDockWindow (parent, name)
{
  m_playlist_widget = new KPlayerPlaylistWidget (ac, this);
  boxLayout() -> addWidget (new QDockArea (Qt::Vertical, QDockArea::Normal, this, "playlistleftdock"));
  boxLayout() -> addWidget (m_playlist_widget);
  boxLayout() -> addWidget (new QDockArea (Qt::Vertical, QDockArea::Normal, this, "playlistrightdock"));
  setResizeEnabled (true);
  setCloseMode (QDockWindow::Always);
  setNewLine (true);
  setCaption (i18n("Playlist Editor"));
}

void KPlayerPlaylistWindow::hideEvent (QHideEvent* event)
{
  QDockWindow::hideEvent (event);
  if ( isHidden() )
    emit windowHidden();
}

void KPlayerPlaylistWindow::focusOutEvent (QFocusEvent* event)
{
  QDockWindow::focusOutEvent (event);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Window focus out " << QFocusEvent::reason() << " " << event -> spontaneous() << "\n";
#endif
}

void KPlayerPlaylistWindow::focusInEvent (QFocusEvent* event)
{
  QDockWindow::focusInEvent (event);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Window focus in " << QFocusEvent::reason() << " " << event -> spontaneous() << "\n";
#endif
}

void KPlayerPlaylistWindow::windowActivationChange (bool old)
{
  QDockWindow::windowActivationChange (old);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Window activation " << old << " -> " << isActiveWindow() << "\n";
#endif
}

KPlayerPlaylistWidget::KPlayerPlaylistWidget (KActionCollection* ac, QWidget* parent, const char* name)
  : KListView (parent, name)
{
  m_locked = m_selection_only = m_main_transitory = m_temp_transitory = false;
  m_shortcuts_enabled = true;
  m_enable_play = m_enable_pause = m_enable_stop = false;
  m_list = 0;
  m_ac = ac;
  connect (this, SIGNAL (executed(QListViewItem*)), this, SLOT (play (QListViewItem*)));
  connect (this, SIGNAL (itemRenamed (QListViewItem*, const QString&, int)), this, SLOT (rename (QListViewItem*, const QString&, int)));
  connect (this, SIGNAL (moved (QListViewItem*, QListViewItem*, QListViewItem*)), this, SLOT (move (QListViewItem*, QListViewItem*, QListViewItem*)));
  connect (this, SIGNAL (selectionChanged()), this, SLOT (updateSelection()));
  connect (kPlayerProcess(), SIGNAL (stateChanged(KPlayerProcess::State, KPlayerProcess::State)), this, SLOT (playerStateChanged(KPlayerProcess::State, KPlayerProcess::State)));
  connect (kPlayerSettings(), SIGNAL (refresh()), this, SLOT (refreshSettings()));
  setupActions();
}

KPlayerPlaylistWidget::~KPlayerPlaylistWidget() 
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Destroying playlist widget\n";
#endif
}

void KPlayerPlaylistWidget::setupActions (void)
{
  if ( ! m_ac )
    return;
  KAction* action = KStdAction::open (this, SLOT (fileOpen()), m_ac);
  action -> setStatusText (i18n("Opens an existing file"));
  action -> setWhatsThis (i18n("Open command displays the standard Open File dialog and lets you choose a file or several files to put on the playlist and start playing."));
  action = new KAction (i18n("Open &URL..."), "fileopenurl", CTRL + Key_U, this, SLOT (fileOpenUrl()), m_ac, "file_open_url");
  action -> setStatusText (i18n("Opens a URL"));
  action -> setWhatsThis (i18n("Open URL command displays the standard Open URL dialog and lets you type or paste in a URL to put on the playlist and start playing. The URL can be a remote network location, a local file path, a KDE I/O Slave URL or a special URL for playing from various types of devices like DVD, video CD, audio CD, TV or DVB."));
  KRecentFilesAction* recent = KStdAction::openRecent (this, SLOT (fileOpenRecent(const KURL&)), m_ac);
  recent -> setStatusText (i18n("Opens a recently used file"));
  recent -> setWhatsThis (i18n("Open Recent submenu displays a list of the most recently opened files and URLs and lets you choose one to put on the playlist and start playing."));
  recent -> setMaxItems (kPlayerSettings() -> recentFileListSize());
  action = new KPlayerPlaylistAction (i18n("Playlist"), 0, this, SLOT (playlistPlay()), m_ac, "playlist_list");
  action -> setStatusText (i18n("Shows playlist items and allows you to select an item to play"));
  action -> setWhatsThis (i18n("Playlist combo box in the closed state displays the current playlist item. You can drop down the combo box to see the entire list and select a different item to load and play."));
  setPlaylist (playlistAction() -> playlist());
  action = new KAction (i18n("&Next"), "1rightarrow", ALT + Key_Right, this, SLOT (playlistNext()), m_ac, "playlist_next");
  action -> setStatusText (i18n("Plays the next item on the playlist"));
  action -> setWhatsThis (i18n("Next command starts playing the next item on the current playlist."));
  action = new KAction (i18n("&Previous"), "1leftarrow", ALT + Key_Left, this, SLOT (playlistPrevious()), m_ac, "playlist_previous");
  action -> setStatusText (i18n("Plays the previous item on the playlist"));
  action -> setWhatsThis (i18n("Previous command starts playing the previous item on the current playlist."));
  action = new KToggleAction (i18n("&Loop"), "loop", 0, this, SLOT (playlistLoop()), m_ac, "playlist_loop");
  action -> setStatusText (i18n("Turns the option to loop through the list on/off"));
  action -> setWhatsThis (i18n("Loop command toggles the option to start playing items from the beginning of the playlist after playing the last item on the playlist."));
  action = new KToggleAction (i18n("S&huffle"), "shuffle", 0, this, SLOT (playlistShuffle()), m_ac, "playlist_shuffle");
  action -> setStatusText (i18n("Turns the option to play items in random order on/off"));
  action -> setWhatsThis (i18n("Shuffle command toggles the option to play playlist items in a random order."));
  action = new KToggleAction (i18n("Tempor&ary List For New Entries"), "lock", 0, this, SLOT (playlistLock()), m_ac, "playlist_lock");
  action -> setStatusText (i18n("Turns the option to use temporary list for new entries on/off"));
  action -> setWhatsThis (i18n("Temporary List For New Entries command toggles the option to use a temporary playlist when adding new entries. When enabled, this option locks the main playlist, so when you load new items using File Open or File Open URL commands, by dragging and dropping files onto KPlayer window, or by starting KPlayer with file arguments, it creates a temporary playlist, puts the new items on it and starts playing them. To start playing items from the main playlist again, choose Play From Playlist from the Playlist menu."));
  action = new KToggleAction (i18n("Play Selection &Only"), 0, 0, this, SLOT (playlistSelection()), m_ac, "playlist_selection");
  action -> setStatusText (i18n("Toggles the option to play selection only or the entire playlist"));
  action -> setWhatsThis (i18n("Play Selection Only command toggles the option to only play items selected in the playlist editor."));
  action = new KToggleAction (i18n("Clear &Before Adding Entries"), 0, 0, this, SLOT (playlistTransitory()), m_ac, "playlist_transitory");
  action -> setStatusText (i18n("Turns the option to clear list before adding new entries on/off"));
  action -> setWhatsThis (i18n("Clear Before Adding Entries command toggles the option to clear the current playlist before adding new items to it. The current playlist can be either the main playlist or the temporary playlist when the Temporary List For New Entries option is enabled."));
  action = new KAction (i18n("&Play"), 0, CTRL + Key_Return, this, SLOT (play()), m_ac, "playlist_play");
  action -> setStatusText (i18n("Plays the currently selected playlist item"));
  action -> setWhatsThis (i18n("Play command loads and plays the item you right click in the playlist editor."));
  action = new KAction (i18n("Play &From Playlist"), 0, 0, this, SLOT (play()), m_ac, "playlist_playlist");
  action -> setStatusText (i18n("Plays the currently selected playlist item"));
  action -> setWhatsThis (i18n("Play From Playlist command starts playing items from the main playlist. This command is only available when a temporary playlist is being used that has been created because the Temporary List For New Entries option had been enabled."));
  action = new KAction (i18n("&Rename"), 0, Key_F2, this, SLOT (rename()), m_ac, "playlist_rename");
  action -> setStatusText (i18n("Lets you rename the currently selected playlist item"));
  action -> setWhatsThis (i18n("Rename command starts edit mode for the current item in the playlist editor so you can change the item name. This command is available when playlist editor is shown and is not empty."));
  action = new KAction (i18n("Propert&ies..."), 0, 0, this, SLOT (properties()), m_ac, "playlist_properties");
  action -> setStatusText (i18n("Opens Properties dialog for the currently selected playlist item"));
  action -> setWhatsThis (i18n("Properties command opens File Properties dialog for the current item in the playlist editor. See the File properties micro-HOWTO for details. This command is available when playlist editor is shown and is not empty."));
  action = new KAction (i18n("Move &Up"), "1uparrow", 0, this, SLOT (moveUp()), m_ac, "playlist_move_up");
  action -> setStatusText (i18n("Moves the selected items up in the playlist"));
  action -> setWhatsThis (i18n("Move Up command moves the currently selected items up in the playlist editor. This command is available when playlist editor is shown and some items are selected. You can also move items around by clicking and dragging them with the left mouse button."));
  action = new KAction (i18n("Move &Down"), "1downarrow", 0, this, SLOT (moveDown()), m_ac, "playlist_move_down");
  action -> setStatusText (i18n("Moves the selected items down in the playlist"));
  action -> setWhatsThis (i18n("Move Down command moves the currently selected items down in the playlist editor. This command is available when playlist editor is shown and some items are selected. You can also move items around by clicking and dragging them with the left mouse button."));
  action = new KAction (i18n("Re&move"), "editdelete", Key_Delete, this, SLOT (remove()), m_ac, "playlist_remove");
  action -> setStatusText (i18n("Removes the currently selected items from the playlist"));
  action -> setWhatsThis (i18n("Remove command removes from the main playlist items currently selected in the playlist editor. This command is available when playlist editor is shown and some items are selected."));
  action = new KAction (i18n("&Clear"), "editclear", 0, this, SLOT (clear()), m_ac, "playlist_clear");
  action -> setStatusText (i18n("Removes all items from the playlist"));
  action -> setWhatsThis (i18n("Clear command removes all items from the main playlist in the playlist editor. This command is available when playlist editor is shown and is not empty."));
  readOptions();
}

void KPlayerPlaylistWidget::terminate (void)
{
  disconnect (kPlayerSettings(), SIGNAL (refresh()), this, SLOT (refreshSettings()));
  saveOptions();
  KListView::clear();
  m_list -> clear();
  setActionCollection (0);
}

void KPlayerPlaylistWidget::saveOptions (void) const
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Saving playlist options\n";
#endif
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Player Options");
  config -> writeEntry ("Playlist Locked", locked());
  config -> writeEntry ("Playlist Loop", loop());
  config -> writeEntry ("Playlist Shuffle", shuffle());
  config -> writeEntry ("Playlist Selection Only", selectionOnly());
  config -> writeEntry ("Playlist Transitory", m_main_transitory);
  config -> writeEntry ("Temporary Playlist Transitory", m_temp_transitory);
  int i = 0;
  for ( QListViewItem* item = firstChild(); item; item = item -> nextSibling() )
  {
    KPlayerPlaylistViewItem* pli = (KPlayerPlaylistViewItem*) item;
    kPlayerPlaylistConfig() -> setGroup ("Playlist Entries");
    kPlayerPlaylistConfig() -> writeEntry ("Entry " + QString::number (i), pli -> properties() -> url().url());
    kPlayerPlaylistConfig() -> writeEntry ("Selected " + QString::number (i), pli -> isSelected());
    i ++;
  }
  kPlayerPlaylistConfig() -> setGroup ("Playlist Entries");
  kPlayerPlaylistConfig() -> writeEntry ("Entries", i);
  for ( i = 0; i < columns(); i ++ )
    config -> writeEntry ("Playlist Column " + QString::number (i) + " Width", columnWidth (i));
  recentFilesAction() -> saveEntries (config, "Recent Files");
}

void KPlayerPlaylistWidget::readOptions (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Reading playlist options\n";
#endif
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Player Options");
  setLocked (config -> readBoolEntry ("Playlist Locked"));
  setLoop (config -> readBoolEntry ("Playlist Loop"));
  setShuffle (config -> readBoolEntry ("Playlist Shuffle"));
  setSelectionOnly (config -> readBoolEntry ("Playlist Selection Only"));
  m_main_transitory = config -> readBoolEntry ("Playlist Transitory");
  m_temp_transitory = config -> readBoolEntry ("Temporary Playlist Transitory");
  QString captions [PLAYLIST_COLUMNS] = {
    i18n("Name"),
    i18n("Type"),
    i18n("Length"),
    i18n("Information"),
    i18n("Path")
  };
  int i, entries, colwidth, pathwidth = config -> readNumEntry ("Playlist Column 8 Width", -1);
  config -> deleteEntry ("Playlist Column 8 Width");
  for ( i = 0; i < PLAYLIST_COLUMNS; i ++ )
  {
    colwidth = config -> readNumEntry ("Playlist Column " + QString::number (i) + " Width", -1);
    if ( i == COLUMN_PATH && pathwidth >= 0 )
      colwidth = pathwidth;
    if ( i == COLUMN_INFO && pathwidth >= 0 )
    {
      colwidth = 0;
      for ( entries = 3; entries < 8; entries ++ )
      {
        colwidth += config -> readNumEntry ("Playlist Column " + QString::number (entries) + " Width");
        config -> deleteEntry ("Playlist Column " + QString::number (entries) + " Width");
      }
    }
    addColumn (captions [i], colwidth);
  }
  setSorting (-1);
  kPlayerPlaylistConfig() -> setGroup ("Playlist Entries");
  entries = kPlayerPlaylistConfig() -> readNumEntry ("Entries");
  for ( i = 0; i < entries; i ++ )
  {
    kPlayerPlaylistConfig() -> setGroup ("Playlist Entries");
    KURL url (kPlayerPlaylistConfig() -> readEntry ("Entry " + QString::number (i)));
    if ( ! url.isEmpty() )
    {
      KPlayerPlaylistViewItem* item = new KPlayerPlaylistViewItem (this, url);
      item -> setSelected (kPlayerPlaylistConfig() -> readBoolEntry ("Selected " + QString::number (i)));
      item -> moveItem (lastItem());
      item -> refresh();
      if ( ! selectionOnly() || item -> isSelected() )
        m_list -> add (item);
    }
  }
  if ( ! m_list -> isEmpty() )
  {
    m_list -> randomize();
    m_list -> setCurrent (0);
  }
  setSelectionMode (QListView::Extended); // Multi or Extended
  //setSelectionModeExt (KListView::FileManager);
  setAllColumnsShowFocus (true);
  setSorting (-1);
  setResizeMode (QListView::NoColumn);
  setDragEnabled (true);
  setAcceptDrops (true);
  //setDropVisualizer (true);
  setItemsRenameable (true);
  setDefaultRenameAction (Accept);
  setAlternateBackground (KGlobalSettings::alternateBackgroundColor());
  //setTooltipColumn (0);
  //setShowToolTips (true);
  if ( locked() )
    toggleAction ("playlist_lock") -> setChecked (true);
  if ( playlistAction() -> playlist() -> loop() )
    toggleAction ("playlist_loop") -> setChecked (true);
  if ( playlistAction() -> playlist() -> shuffle() )
    toggleAction ("playlist_shuffle") -> setChecked (true);
  if ( selectionOnly() )
    toggleAction ("playlist_selection") -> setChecked (true);
  KRecentFilesAction* recent = recentFilesAction();
  recent -> loadEntries (kPlayerConfig(), "Recent Files");
  recent -> setEnabled (recent -> items().count() > 0);
  refreshSettings();
  enableActions();
}

void KPlayerPlaylistWidget::load (KURL url)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: URL  '" << url.url() << "'\n";
  kdDebugTime() << "Playlist:      '" << url.prettyURL (0, KURL::StripFileProtocol) << "'\n";
#endif
  if ( url.path().isEmpty() && url.host().isEmpty() )
  {
    if ( kPlayerProcess() -> state() == KPlayerProcess::Idle )
    {
      kPlayerEngine() -> stop();
      emit stopped();
    }
    return;
  }
  if ( kPlayerEngine() -> stopped() )
    emit started();
  bool new_url = url != kPlayerSettings() -> url();
  kPlayerEngine() -> load (url);
  if ( new_url )
    emit activated();
}

void KPlayerPlaylistWidget::openUrls (KURL::List urls)
{
  add (urls, (KPlayerPlaylistViewItem*) lastChild());
  enableActions();
}

void KPlayerPlaylistWidget::fileOpen (void)
{
  openUrls (kPlayerEngine() -> openFiles());
}

void KPlayerPlaylistWidget::fileOpenUrl (void)
{
  openUrls (kPlayerEngine() -> openUrl());
}

void KPlayerPlaylistWidget::fileOpenRecent (const KURL& url)
{
  openUrls (url);
  recentFilesAction() -> setCurrentItem (-1);
}

void KPlayerPlaylistWidget::playlistPlay (void)
{
  load (current());
}

void KPlayerPlaylistWidget::playlistNext (void)
{
  load (next());
}

void KPlayerPlaylistWidget::playlistPrevious (void)
{
  load (previous());
}

void KPlayerPlaylistWidget::playlistSelection (void)
{
  setSelectionOnly (toggleAction ("playlist_selection") -> isChecked());
}

void KPlayerPlaylistWidget::playlistLock (void)
{
  setLocked (toggleAction ("playlist_lock") -> isChecked());
}

void KPlayerPlaylistWidget::playlistLoop (void)
{
  setLoop (toggleAction ("playlist_loop") -> isChecked());
}

void KPlayerPlaylistWidget::playlistShuffle (void)
{
  setShuffle (toggleAction ("playlist_shuffle") -> isChecked());
}

void KPlayerPlaylistWidget::playlistTransitory (void)
{
  setTransitory (toggleAction ("playlist_transitory") -> isChecked());
}

void KPlayerPlaylistWidget::refreshSettings (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist::refreshSettings\n";
#endif
  recentFilesAction() -> setMaxItems (kPlayerSettings() -> recentFileListSize());
  makeRoom (0);
  uint i;
  if ( ! kPlayerSettings() -> allowDuplicateEntries() )
  {
    KPlayerPlaylistItem *item, *cur = m_list -> current();
    if ( m_list -> isTemporary() )
      for ( i = 0; i < m_list -> count() - 1; i ++ )
      {
        item = m_list -> item (i);
        if ( item )
        {
          KURL url (item -> properties() -> url());
          for ( uint j = i + 1; j < m_list -> count(); j ++ )
          {
            item = m_list -> item (j);
            if ( item && item -> properties() -> url() == url )
            {
              if ( item == cur )
                m_list -> setCurrent (cur);
              m_list -> remove (item);
              if ( item == cur )
                cur = m_list -> current();
              j --;
            }
          }
        }
      }
    for ( QListViewItem* li = firstChild(); li; li = li -> nextSibling() )
    {
      KPlayerPlaylistViewItem* pli = (KPlayerPlaylistViewItem*) li;
      KURL url (pli -> properties() -> url());
      for ( QListViewItem *next, *lj = li -> nextSibling(); lj; lj = next )
      {
        KPlayerPlaylistViewItem* plj = (KPlayerPlaylistViewItem*) lj;
        next = lj -> nextSibling();
        if ( plj -> properties() -> url() == url )
        {
          if ( cur == (KPlayerPlaylistItem*) plj )
            m_list -> setCurrent (cur);
          m_list -> remove (plj);
          if ( cur == (KPlayerPlaylistItem*) plj )
            cur = m_list -> current();
          delete plj;
        }
      }
    }
    if ( cur )
      m_list -> setCurrent (cur);
  }
  QStringList groups (kPlayerPlaylistConfig() -> groupList());
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Cache has " << groups.count() << " entries\n";
#endif
  if ( int (groups.count()) > kPlayerSettings() -> cacheSizeLimit() + 1 )
  {
    QMap<QString,QString> map;
    i = 0;
    for ( QStringList::Iterator it = groups.begin(); it != groups.end(); ++ it )
      if ( *it != "Entries" )
      {
        kPlayerPlaylistConfig() -> setGroup (*it);
        QDateTime dt (kPlayerPlaylistConfig() -> readDateTimeEntry ("Date"));
        if ( ! dt.isNull() )
          map.insert (dt.toString (Qt::ISODate) + QString().sprintf ("-%04u", i ++), *it);
      }
    i = groups.count() - kPlayerSettings() -> cacheSizeLimit();
    for ( QMap<QString,QString>::Iterator mapit = map.begin(); i > 1 && mapit != map.end(); ++ mapit )
    {
#ifdef DEBUG_KPLAYER_PLAYLIST
      kdDebugTime() << "Deleting entry for " << mapit.data() << " from " << mapit.key() << "\n";
#endif
      kPlayerPlaylistConfig() -> deleteGroup (mapit.data());
      i --;
    }
  }
  enableActions();
}

void KPlayerPlaylistWidget::enableNextPrevious (bool same_url)
{
  if ( ! m_ac )
    return;
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Enabling next and previous actions\n";
#endif
  if ( ! same_url && m_list -> current() )
    same_url = kPlayerSettings() -> url() == m_list -> current() -> properties() -> url();
  int count = m_list -> count();
  m_ac -> action ("playlist_next") -> setEnabled (count > 0 && ! same_url
    || count > 1 && (loop() || m_list -> index() < count - 1));
  m_ac -> action ("playlist_previous") -> setEnabled (count > 1
    && (loop() || m_list -> index() > 0));
}

void KPlayerPlaylistWidget::enableActions (void)
{
  if ( ! m_ac )
    return;
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Enabling playlist actions\n";
#endif
  bool selection = false;
  for ( QListViewItem* li = firstChild(); li; li = li -> nextSibling() )
    if ( li -> isSelected() )
    {
      selection = true;
      break;
    }
  bool visible = isVisible();
  m_ac -> action ("playlist_playlist") -> setEnabled (firstChild() != 0 && m_list -> isTemporary());
  m_ac -> action ("playlist_selection") -> setEnabled (! m_list -> isTemporary());
  toggleAction ("playlist_transitory") -> setChecked (transitory());
  m_ac -> action ("playlist_play") -> setEnabled (visible && firstChild() != 0);
  m_ac -> action ("playlist_rename") -> setEnabled (visible && currentItem() != 0);
  m_ac -> action ("playlist_properties") -> setEnabled (visible && currentItem() != 0);
  m_ac -> action ("playlist_move_up") -> setEnabled (visible && selection);
  m_ac -> action ("playlist_move_down") -> setEnabled (visible && selection);
  m_ac -> action ("playlist_remove") -> setEnabled (visible && selection);
  m_ac -> action ("playlist_clear") -> setEnabled (visible && firstChild() != 0);
  enableNextPrevious();
}

KURL KPlayerPlaylistWidget::current (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  if ( m_list -> current() )
    kdDebugTime() << "Playlist::current: Current URL '" << m_list -> current() -> properties() -> url().url() << "'\n";
  else
    kdDebugTime() << "Playlist::current: No current URL\n";
#endif
  KURL url (m_list -> current() ? m_list -> current() -> properties() -> url() : KURL());
  enableNextPrevious (! url.isEmpty());
  return url;
}

KURL KPlayerPlaylistWidget::next (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist::next: Previous URL '" << kPlayerSettings() -> url().url() << "'\n";
#endif
  KURL url (kPlayerSettings() -> url() != current() || m_list -> next() ? current() : KURL());
  enableNextPrevious (! url.isEmpty());
  return url;
}

KURL KPlayerPlaylistWidget::previous (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist::previous: Previous URL '" << kPlayerSettings() -> url().url() << "'\n";
#endif
  KURL url (kPlayerSettings() -> url() != current() || m_list -> previous() ? current() : KURL());
  enableNextPrevious (! url.isEmpty());
  return url;
}

void KPlayerPlaylistWidget::setSelectionOnly (bool selection_only)
{
  if ( m_selection_only == selection_only )
    return;
  m_selection_only = selection_only;
  if ( m_list -> isTemporary() )
    return;
  reload();
  enableNextPrevious();
}

void KPlayerPlaylistWidget::setLoop (bool loop)
{
  m_list -> setLoop (loop);
  enableNextPrevious();
}

void KPlayerPlaylistWidget::setShuffle (bool shuffle)
{
  if ( shuffle == m_list -> shuffle() )
    return;
  m_list -> setShuffle (shuffle);
  if ( ! shuffle && ! m_list -> isTemporary() )
    reload();
  enableNextPrevious();
}

void KPlayerPlaylistWidget::reload (void)
{
  KPlayerPlaylistItem* cur = m_list -> current();
  m_list -> clear();
  KPlayerPlaylistViewItem* item ((KPlayerPlaylistViewItem*) firstChild());
  bool find_selected = false;
  while ( item )
  {
    if ( ! selectionOnly() || item -> isSelected() )
      m_list -> add (item);
    if ( find_selected && item -> isSelected() )
    {
      cur = item;
      find_selected = false;
    }
    else if ( selectionOnly() && ! shuffle() && ! item -> isSelected()
        && cur == (KPlayerPlaylistItem*) item )
    {
      find_selected = true;
      cur = 0;
    }
    item = (KPlayerPlaylistViewItem*) item -> nextSibling();
  }
  if ( ! m_list -> isEmpty() )
  {
    m_list -> randomize();
    m_list -> setCurrent (0);
    if ( cur )
      m_list -> setCurrent (cur);
  }
  m_list -> setTemporary (false);
}

void KPlayerPlaylistWidget::play (void)
{
  QListViewItem* item = currentItem();
  if ( ! item )
    item = firstChild();
  if ( item )
    play (item);
}

void KPlayerPlaylistWidget::play (QListViewItem* i)
{
  enablePlayerShortcuts (true);
  enablePlayerShortcuts (false);
  if ( m_list -> isTemporary() )
    reload();
  KPlayerPlaylistViewItem* item = (KPlayerPlaylistViewItem*) i;
  if ( selectionOnly() && ! i -> isSelected() )
  {
    i -> setSelected (true);
    updateSelection();
  }
  m_list -> setCurrent (item);
  enableActions();
  playlistPlay();
}

void KPlayerPlaylistWidget::properties (void)
{
  KPlayerPlaylistViewItem* item = (KPlayerPlaylistViewItem*) currentItem();
  if ( ! item )
    return;
  KPlayerPropertiesDialog dialog (item -> properties());
  dialog.exec();
}

void KPlayerPlaylistWidget::rename (void)
{
  QListViewItem* item = currentItem();
  if ( item )
    rename (item, 0);
}

void KPlayerPlaylistWidget::rename (QListViewItem* i, int column)
{
  KListView::rename (i, column);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Renaming item " << i -> text (COLUMN_NAME) << "\n";
#endif
  enablePlayerShortcuts (false);
}

void KPlayerPlaylistWidget::rename (QListViewItem* i, const QString& name, int column)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Rename signal: column " << column << " " << name << "\n";
#endif
  enablePlayerShortcuts (true);
  if ( column != COLUMN_NAME )
    return;
  KPlayerPlaylistViewItem* item = (KPlayerPlaylistViewItem*) i;
  m_list -> rename (item, name);
}

void KPlayerPlaylistWidget::updateSelection (void)
{
  if ( selectionOnly() && ! m_list -> isTemporary() )
  {
    if ( shuffle() )
    {
      QPtrList<KPlayerPlaylistItem> selected, unselected;
      KPlayerPlaylistItem *pli, *cur = m_list -> current();
      for ( QListViewItem* item = firstChild(); item; item = item -> nextSibling() )
      {
        pli = (KPlayerPlaylistViewItem*) item;
        if ( m_list -> has (pli) != item -> isSelected() )
          (item -> isSelected() ? selected : unselected).append (pli);
      }
      for ( pli = selected.first(); pli; pli = selected.next() )
        m_list -> add (pli);
      if ( cur )
        m_list -> setCurrent (cur);
      else
        m_list -> setCurrent (0);
      m_list -> randomize (m_list -> current());
      for ( pli = unselected.first(); pli; pli = unselected.next() )
        m_list -> remove (pli);
    }
    else
      reload();
  }
  enableActions();
}

void KPlayerPlaylistWidget::move (QListViewItem* item, QListViewItem*, QListViewItem* after)
{
  if ( ! selectionOnly() && ! shuffle() && ! m_list -> isTemporary() )
    m_list -> move ((KPlayerPlaylistViewItem*) item, (KPlayerPlaylistViewItem*) after);
  enableNextPrevious();
}

bool KPlayerPlaylistWidget::makeRoom (uint room, KPlayerPlaylistViewItem* pli)
{
  KPlayerPlaylistItem* cur = m_list -> current();
  uint limit = kPlayerSettings() -> playlistSizeLimit();
  bool result = false;
  while ( childCount() + room > limit && childCount() > 0 )
  {
    KPlayerPlaylistViewItem* item = (KPlayerPlaylistViewItem*) firstChild();
    if ( item == pli )
    {
#ifdef DEBUG_KPLAYER_PLAYLIST
      kdDebugTime() << "Playlist: Removing the given item\n";
#endif
      result = true;
    }
    m_list -> remove (item);
    delete item;
  }
  while ( m_list -> count() + room > limit && m_list -> count() > 0 )
  {
    m_list -> setCurrent (0);
    m_list -> remove (m_list -> current());
  }
  if ( cur )
    m_list -> setCurrent (cur);
  return result;
}

void KPlayerPlaylistWidget::add (KURL::List& urls, KPlayerPlaylistViewItem* after)
{
  if ( urls.isEmpty() || urls.count() == 1 && urls.first().path().isEmpty() && urls.first().host().isEmpty() )
    return;
  if ( locked() && ! m_list -> isTemporary() )
  {
    m_list -> clear();
    m_list -> setTemporary (true);
  }
  else if ( ! locked() && m_list -> isTemporary() )
    reload();
  if ( transitory() )
  {
    clear();
    after = 0;
  }
  KPlayerPlaylistItem* item;
  if ( ! kPlayerSettings() -> allowDuplicateEntries() )
  {
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << "Playlist: Checking for duplicate URLs\n";
#endif
    for ( QValueListIterator<KURL> it = urls.begin(); it != urls.end(); ++ it )
    {
      QValueListIterator<KURL> iter (it);
      for ( ++ iter; iter != urls.end(); *iter == *it ? (iter = urls.remove (iter)) : ++ iter );
    }
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << "Playlist: Removing duplicate entries\n";
#endif
    if ( m_list -> isTemporary() )
      for ( uint i = 0; i < m_list -> count(); i ++ )
      {
	item = m_list -> item (i);
	if ( item )
          for ( QValueListConstIterator<KURL> it = urls.begin(); it != urls.end(); ++ it )
            if ( item -> properties() -> url() == *it )
            {
              m_list -> remove (item);
              i --;
	      break;
	    }
      }
    else
    {
      KPlayerPlaylistViewItem* last = 0;
      for ( QListViewItem *n, *i = firstChild(); i; i = n )
      {
	n = i -> nextSibling();
        KPlayerPlaylistViewItem* pli = (KPlayerPlaylistViewItem*) i;
	bool deleted = false;
        for ( QValueListConstIterator<KURL> it = urls.begin(); ! deleted && it != urls.end(); ++ it )
          if ( pli -> properties() -> url() == *it )
          {
#ifdef DEBUG_KPLAYER_PLAYLIST
            kdDebugTime() << "Playlist: Removing entry: " << pli -> properties() -> url().prettyURL() << "\n";
#endif
	    if ( after == pli )
	      after = last;
            m_list -> remove (pli);
            delete pli;
	    deleted = true;
          }
	if ( ! deleted )
	  last = pli;
      }
    }
  }
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Making room for new entries\n";
#endif
  if ( makeRoom (urls.count(), after) )
    after = 0;
  KPlayerPlaylistItem* cur = m_list -> current();
  KPlayerPlaylistItem* after_item = ! locked() && ! shuffle() ? after
    : m_list -> isEmpty() ? 0 : m_list -> item (m_list -> count() - 1);
#ifdef DEBUG_KPLAYER_PLAYLIST
  if ( after_item )
    kdDebugTime() << "Playlist: List after item: " << after_item -> properties() -> url().prettyURL() << "\n";
  if ( after && ! locked() )
    kdDebugTime() << "Playlist: Editor after item: " << after -> properties() -> url().prettyURL() << "\n";
#endif
  if ( after && selectionOnly() && ! shuffle() && ! after -> isSelected() )
  {
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << "Playlist: Looking for a selected entry to insert after\n";
#endif
    after_item = 0;
    for ( QListViewItem* i = firstChild(); i; i = i -> nextSibling() )
      if ( i -> isSelected() )
	after_item = (KPlayerPlaylistViewItem*) i;
      else if ( i == after )
	break;
  }
  bool at_end = after_item == (m_list -> isEmpty() ? 0 : m_list -> item (m_list -> count() - 1));
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: At end: " << at_end << "\n";
#endif
  KPlayerPlaylistItem* prev = after_item;
#ifdef DEBUG_KPLAYER_PLAYLIST
  if ( prev != (KPlayerPlaylistItem*) after )
    kdDebugTime() << "Playlist: Different after item\n";
#endif
//if ( prev )
//{
//  m_list -> setCurrent (prev);
//#ifdef DEBUG_KPLAYER_PLAYLIST
//  kdDebugTime() << "Playlist: Index " << m_list -> index() << "\n";
//#endif
//}
#ifdef DEBUG_KPLAYER_PLAYLIST
//else
//  kdDebugTime() << "Playlist: After item is zero\n";
  kdDebugTime() << "Playlist: Adding new entries\n";
#endif
  for ( QValueListConstIterator<KURL> it = urls.begin(); it != urls.end(); ++ it )
  {
    if ( (*it).path().isEmpty() && (*it).host().isEmpty() )
      continue;
    if ( locked() )
      item = new KPlayerPlaylistItem (*it);
    else
    {
#ifdef DEBUG_KPLAYER_PLAYLIST
      kdDebugTime() << "Playlist: Creating list view item\n";
#endif
      KPlayerPlaylistViewItem* pli = new KPlayerPlaylistViewItem (this, *it);
      if ( after )
      {
#ifdef DEBUG_KPLAYER_PLAYLIST
        kdDebugTime() << "Playlist: Moving the new item\n";
#endif
        pli -> moveItem (after);
      }
      if ( selectionOnly() )
        pli -> setSelected (true);
      item = pli;
      after = pli;
    }
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << "Playlist: Adding the new item\n";
#endif
    m_list -> add (item);
    if ( ! at_end )
    {
#ifdef DEBUG_KPLAYER_PLAYLIST
      kdDebugTime() << "Playlist: Moving the new item\n";
#endif
      m_list -> move (item, after_item);
      after_item = item;
    }
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << "Playlist: Added entry: " << item -> properties() -> url().prettyURL() << "\n";
#endif
  }
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Randomizing\n";
#endif
  m_list -> randomize (prev);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Index " << m_list -> index() << "\n";
#endif
  if ( cur )
    m_list -> setCurrent (cur);
  else
    m_list -> setCurrent (0);
  if ( kPlayerSettings() -> startPlayingImmediately()
    && (! kPlayerSettings() -> startPlayingOnlyIfIdle() || kPlayerProcess() -> state() == KPlayerProcess::Idle) )
  {
    if ( prev )
    {
      m_list -> setCurrent (prev);
#ifdef DEBUG_KPLAYER_PLAYLIST
      kdDebugTime() << "Playlist: Index " << m_list -> index() << "\n";
#endif
      m_list -> next();
    }
    else
      m_list -> setCurrent (0);
#ifdef DEBUG_KPLAYER_PLAYLIST
    kdDebugTime() << "Playlist: Index " << m_list -> index() << "\n";
#endif
    playlistPlay();
  }
  if ( kPlayerSettings() -> recentFileListSize() < 1 )
    return;
  KRecentFilesAction* recent = recentFilesAction();
  recent -> setEnabled (true);
  QValueListIterator<KURL> it (urls.end());
  do
    recent -> addURL (* -- it);
  while ( it != urls.begin() );
}

void KPlayerPlaylistWidget::moveUp (void)
{
  QListViewItem *item (firstChild()), *previous, *after = 0;
  while ( item && item -> isSelected() )
  {
    after = item;
    item = item -> nextSibling();
  }
  if ( ! item )
    return;
  item = item -> nextSibling();
  while ( item )
  {
    previous = item;
    item = item -> nextSibling();
    if ( previous -> isSelected() )
    {
      if ( after )
        previous -> moveItem (after);
      else
      {
	bool is_current = currentItem() == previous;
        takeItem (previous);
        insertItem (previous);
	if ( is_current )
	  setCurrentItem (previous);
      }
      if ( ! selectionOnly() && ! shuffle() && ! m_list -> isTemporary() )
        m_list -> move ((KPlayerPlaylistViewItem*) previous, (KPlayerPlaylistViewItem*) after);
      after = previous;
    }
    else if ( after )
      after = after -> nextSibling();
    else
      after = firstChild();
  }
  enableNextPrevious();
}

void KPlayerPlaylistWidget::moveDown (void)
{
  QListViewItem *item (firstChild()), *previous, *after (0), *next (item);
  while ( item && ! item -> isSelected() )
  {
    after = item;
    item = item -> nextSibling();
  }
  while ( item )
  {
    previous = item;
    item = item -> nextSibling();
    if ( previous -> isSelected() )
      next = previous;
    else if ( (after ? after -> nextSibling() : firstChild()) == previous )
      after = previous;
    else
    {
      if ( after )
	previous -> moveItem (after);
      else
      {
	bool is_current = currentItem() == previous;
	takeItem (previous);
	insertItem (previous);
	if ( is_current )
	  setCurrentItem (previous);
      }
      if ( ! selectionOnly() && ! shuffle() && ! m_list -> isTemporary() )
        m_list -> move ((KPlayerPlaylistViewItem*) previous, (KPlayerPlaylistViewItem*) after);
      after = next;
    }
  }
  enableNextPrevious();
}

void KPlayerPlaylistWidget::remove (void)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "KPlayerPlaylistWidget::remove\n";
#endif
  QListViewItem *item (firstChild()), *previous;
  while ( item )
  {
    previous = item;
    item = item -> nextSibling();
    if ( previous -> isSelected() )
    {
      m_list -> remove ((KPlayerPlaylistViewItem*) previous);
      delete previous;
#ifdef DEBUG_KPLAYER_PLAYLIST
      kdDebugTime() << "Playlist: Item deleted\n";
#endif
    }
  }
  enableActions();
}

void KPlayerPlaylistWidget::clear (void)
{
  if ( ! m_list -> isTemporary() )
    KListView::clear();
  m_list -> clear();
  enableActions();
}

void KPlayerPlaylistWidget::playerStateChanged (KPlayerProcess::State state, KPlayerProcess::State previous)
{
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: State change: " << previous << " => " << state << "\n";
#endif
  if ( state != KPlayerProcess::Idle || previous == state )
    return;
  if ( kPlayerEngine() -> stopped() )
    emit stopped();
  else
    load (next());
}

bool KPlayerPlaylistWidget::acceptDrag (QDropEvent* event) const
{
  return KURLDrag::canDecode (event) || KListView::acceptDrag (event);
}

void KPlayerPlaylistWidget::contentsDropEvent (QDropEvent* event)
{
  if ( KURLDrag::canDecode (event) )
  {
    cleanDropVisualizer();
    cleanItemHighlighter();
    event -> acceptAction();
    QListViewItem *parent, *after;
    findDrop (event -> pos(), parent, after);
    KURL::List urls;
    if ( ! KURLDrag::decode (event, urls) )
      return;
    bool locked = m_locked;
    m_locked = false;
    if ( m_list -> isTemporary() )
      reload();
    add (urls, (KPlayerPlaylistViewItem*) after);
    m_locked = locked;
  }
  else
    KListView::contentsDropEvent (event);
  enableActions();
}

void KPlayerPlaylistWidget::showEvent (QShowEvent* event)
{
  KListView::showEvent (event);
  ensureItemVisible (currentItem());
  enableActions();
}

void KPlayerPlaylistWidget::hideEvent (QHideEvent* event)
{
  KListView::hideEvent (event);
  enableActions();
}

void KPlayerPlaylistWidget::resizeEvent (QResizeEvent* event)
{
  bool at_bottom = contentsHeight() == contentsY() + visibleHeight();
  KListView::resizeEvent (event);
  if ( at_bottom && contentsHeight() != contentsY() + visibleHeight() )
    setContentsPos (contentsX(), contentsHeight() - visibleHeight());
}

void KPlayerPlaylistWidget::focusOutEvent (QFocusEvent* event)
{
  KListView::focusOutEvent (event);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Widget focus out " << QFocusEvent::reason() << " " << event -> spontaneous() << "\n";
#endif
}

void KPlayerPlaylistWidget::focusInEvent (QFocusEvent* event)
{
  KListView::focusInEvent (event);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Widget focus in " << QFocusEvent::reason() << " " << event -> spontaneous() << "\n";
#endif
}

void KPlayerPlaylistWidget::enablePlayerShortcuts (bool enable)
{
  if ( enable == m_shortcuts_enabled || ! m_ac )
    return;
  if ( ! enable && (! renameLineEdit() || ! renameLineEdit() -> isVisible()) )
    return;
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist::enablePlayerShortcut (" << enable << ")\n";
#endif
  KShortcut shortcut (Key_Return);
  KAction* action = m_ac -> action ("player_play");
  if ( enable && m_enable_play )
  {
    action -> setShortcut (shortcut);
    m_enable_play = false;
  }
  else if ( ! enable && action -> shortcut() == shortcut )
  {
    action -> setShortcut (KShortcut::null());
    m_enable_play = true;
  }
  shortcut.init (Key_Space);
  action = m_ac -> action ("player_pause");
  if ( enable && m_enable_pause )
  {
    action -> setShortcut (shortcut);
    m_enable_pause = false;
  }
  else if ( ! enable && action -> shortcut() == shortcut )
  {
    action -> setShortcut (KShortcut::null());
    m_enable_pause = true;
  }
  shortcut.init (Key_Escape);
  action = m_ac -> action ("player_stop");
  if ( enable && m_enable_stop )
  {
    action -> setShortcut (shortcut);
    m_enable_stop = false;
  }
  else if ( ! enable && action -> shortcut() == shortcut )
  {
    action -> setShortcut (KShortcut::null());
    m_enable_stop = true;
  }
  m_shortcuts_enabled = enable;
}

void KPlayerPlaylistWidget::windowActivationChange (bool old)
{
  KListView::windowActivationChange (old);
#ifdef DEBUG_KPLAYER_PLAYLIST
  kdDebugTime() << "Playlist: Widget activation " << old << " -> " << isActiveWindow() << "\n";
#endif
  enablePlayerShortcuts (! isActiveWindow());
}
