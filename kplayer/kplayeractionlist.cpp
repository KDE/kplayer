/***************************************************************************
                          kplayeractionlist.cpp
                          ---------------------
    begin                : Thu Apr 13 2006
    copyright            : (C) 2006-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kaction.h>
#include <klocale.h>
#include <qpopupmenu.h>
#include <qregexp.h>

#ifdef DEBUG
#include <kdebug.h>
kdbgstream kdDebugTime (void);
#define DEBUG_KPLAYER_ACTIONLIST
#endif

#include "kplayeractionlist.h"
#include "kplayeractionlist.moc"
#include "kplayerproperties.h"

KPlayerActionList::KPlayerActionList (const QString& text, const QString& status,
    const QString& whatsthis, QObject* parent, const char* name)
  : QObject (parent, name), m_text (text), m_status (status), m_whatsthis (whatsthis)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Creating action list\n";
#endif
  m_actions.setAutoDelete (true);
}

KPlayerActionList::~KPlayerActionList()
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Destroying action list\n";
#endif
  m_actions.clear();
}

void KPlayerActionList::plug (void)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerActionList::plug\n";
  kdDebugTime() << " Name   " << name() << "\n";
#endif
  emit updated (this);
  if ( ! isEmpty() )
  {
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << "Inserting separators\n";
#endif
    KAction* action = actions().getFirst();
    int i, count = action -> containerCount();
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << " Containers " << count << "\n";
#endif
    for ( i = 0; i < count; i ++ )
    {
      QWidget* container = action -> container (i);
#ifdef DEBUG_KPLAYER_ACTIONLIST
      kdDebugTime() << " Container " << container -> className() << " " << container -> name ("<unnamed>") << "\n";
#endif
      if ( container -> inherits ("QPopupMenu") )
      {
        QPopupMenu* menu = (QPopupMenu*) container;
        int index = menu -> indexOf (action -> itemId (i));
#ifdef DEBUG_KPLAYER_ACTIONLIST
        kdDebugTime() << " Index  " << index << "\n";
#endif
        if ( index > 0 )
        {
          QMenuItem* item = menu -> findItem (menu -> idAt (index - 1));
          if ( ! item || ! item -> isSeparator() )
            menu -> insertSeparator (index);
        }
      }
    }
    action = actions().getLast();
    count = action -> containerCount();
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << " Containers " << count << "\n";
#endif
    for ( i = 0; i < count; i ++ )
    {
      QWidget* container = action -> container (i);
#ifdef DEBUG_KPLAYER_ACTIONLIST
      kdDebugTime() << " Container " << container -> className() << " " << container -> name ("<unnamed>") << "\n";
#endif
      if ( container -> inherits ("QPopupMenu") )
      {
        QPopupMenu* menu = (QPopupMenu*) container;
        int index = menu -> indexOf (action -> itemId (i)) + 1;
#ifdef DEBUG_KPLAYER_ACTIONLIST
        kdDebugTime() << " Index  " << index << "\n";
#endif
        if ( index > 0 && index < int (menu -> count()) )
        {
          QMenuItem* item = menu -> findItem (menu -> idAt (index));
          if ( ! item || ! item -> isSeparator() )
            menu -> insertSeparator (index);
        }
      }
    }
  }
}

void KPlayerActionList::unplug (void)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerActionList::unplug\n";
  kdDebugTime() << " Name   " << name() << "\n";
#endif
  if ( ! isEmpty() )
  {
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << "Removing separators\n";
#endif
    KAction* action = actions().getFirst();
    int i, count = action -> containerCount();
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << " Containers " << count << "\n";
#endif
    for ( i = 0; i < count; i ++ )
    {
      QWidget* container = action -> container (i);
#ifdef DEBUG_KPLAYER_ACTIONLIST
      kdDebugTime() << " Container " << container -> className() << " " << container -> name ("<unnamed>") << "\n";
#endif
      if ( container -> inherits ("QPopupMenu") )
      {
        QPopupMenu* menu = (QPopupMenu*) container;
        int index = menu -> indexOf (action -> itemId (i));
#ifdef DEBUG_KPLAYER_ACTIONLIST
        kdDebugTime() << " Index  " << index << "\n";
#endif
        if ( index > 0 )
        {
          QMenuItem* item = menu -> findItem (menu -> idAt (index - 1));
          if ( ! item || item -> isSeparator() )
            menu -> removeItemAt (index - 1);
        }
      }
    }
    action = actions().getLast();
    count = action -> containerCount();
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << " Containers " << count << "\n";
#endif
    for ( i = 0; i < count; i ++ )
    {
      QWidget* container = action -> container (i);
#ifdef DEBUG_KPLAYER_ACTIONLIST
      kdDebugTime() << " Container " << container -> className() << " " << container -> name ("<unnamed>") << "\n";
#endif
      if ( container -> inherits ("QPopupMenu") )
      {
        QPopupMenu* menu = (QPopupMenu*) container;
        int index = menu -> indexOf (action -> itemId (i)) + 1;
#ifdef DEBUG_KPLAYER_ACTIONLIST
        kdDebugTime() << " Index  " << index << "\n";
#endif
        if ( index > 0 && index < int (menu -> count()) )
        {
          QMenuItem* item = menu -> findItem (menu -> idAt (index));
          if ( ! item || item -> isSeparator() )
            menu -> removeItemAt (index);
        }
      }
    }
  }
  emit updating (this);
  m_actions.clear();
}

void KPlayerActionList::updateAction (KAction* action)
{
  QString text (action -> text());
  action -> setStatusText (m_status.arg (text));
  action -> setWhatsThis (m_whatsthis.arg (text));
  text = m_text.arg (text);
  text.replace ("&", "&&");
  action -> setText (text);
}

void KPlayerActionList::actionActivated (void)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerActionList::actionActivated\n";
#endif
  if ( sender() && sender() -> inherits ("KAction") )
  {
    KAction* action = (KAction*) sender();
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << " Name   " << action -> text() << "\n";
#endif
    int index = 0;
    QPtrListIterator<KAction> iterator (actions());
    while ( iterator.current() )
    {
      if ( iterator.current() == action )
      {
        actionActivated (action, index);
        break;
      }
      ++ index;
      ++ iterator;
    }
  }
}

void KPlayerActionList::actionActivated (KAction*, int index)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerActionList::actionActivated\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  emit activated (index);
}

KPlayerSimpleActionList::KPlayerSimpleActionList (const QStringList& names, const QString& text,
  const QString& status, const QString& whatsthis, QObject* parent, const char* name)
  : KPlayerActionList (text, status, whatsthis, parent, name), m_names (names)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Creating simple action list\n";
#endif
}

KPlayerSimpleActionList::~KPlayerSimpleActionList()
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Destroying simple action list\n";
#endif
}

void KPlayerSimpleActionList::update (void)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerSimpleActionList::update\n";
#endif
  unplug();
  QStringList::ConstIterator iterator (m_names.constBegin());
  while ( iterator != m_names.constEnd() )
  {
    KAction* action = new KAction (*iterator, 0, this, SLOT(actionActivated()), this);
    updateAction (action);
    m_actions.append (action);
    ++ iterator;
  }
  plug();
}

void KPlayerSimpleActionList::updateAction (KAction* action)
{
  QString text (action -> text());
  KPlayerPropertyInfo* info = KPlayerMedia::info (text);
  QString caption (info -> caption());
  if ( caption.isEmpty() )
    caption = i18n(text.utf8());
  action -> setStatusText (m_status.arg (caption));
  action -> setWhatsThis (m_whatsthis.arg (caption));
  caption = m_text.arg (caption);
  caption.replace ("&", "&&");
  action -> setText (caption);
}

KPlayerToggleActionList::KPlayerToggleActionList (const QStringList& names, const QMap<QString, bool>& states,
  const QString& ontext, const QString& offtext, const QString& onstatus, const QString& offstatus,
  const QString& onwhatsthis, const QString& offwhatsthis, QObject* parent, const char* name)
  : KPlayerSimpleActionList (names, offtext, offstatus, offwhatsthis, parent, name),
    m_states (states), m_on_text (ontext), m_on_status (onstatus), m_on_whatsthis (onwhatsthis)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Creating toggle action list\n";
#endif
}

KPlayerToggleActionList::~KPlayerToggleActionList()
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Destroying toggle action list\n";
#endif
}

void KPlayerToggleActionList::updateAction (KAction* action)
{
  QString text (action -> text());
  KPlayerPropertyInfo* info = KPlayerMedia::info (text);
  QString caption (info -> caption());
  if ( caption.isEmpty() )
    caption = i18n(text.utf8());
  bool on = m_states [text];
  action -> setStatusText ((on ? m_on_status : m_status).arg (caption));
  action -> setWhatsThis ((on ? m_on_whatsthis : m_whatsthis).arg (caption));
  caption = (on ? m_on_text : m_text).arg (caption);
  caption.replace ("&", "&&");
  action -> setText (caption);
}

void KPlayerToggleActionList::actionActivated (KAction* action, int index)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerToggleActionList::actionActivated\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  KPlayerSimpleActionList::actionActivated (action, index);
  action -> setText (m_names [index]);
  updateAction (action);
}

QString languageName (int id, QString language)
{
  static const QString middle ("|ave|bam|ben|bih|bos|ces|che|chu|chv|div|epo|est|ewe|fao|fij|fry|glv|grn|hat|hmo|ido|iku|ile|ina|ind|ipk|jav|kal|kan|kas|khm|lao|lav|lin|lit|lug|mah|mal|mri|mar|mlg|mlt|mon|mri|nav|nob|orm|pli|pol|pus|run|sag|slk|sme|snd|sot|tat|tgl|tsn|tuk|tur|uig|zha|");
  static const QString right ("|aar|abk|afr|aka|amh|ara|asm|ava|aym|aze|bak|bel|bis|bod|bre|cat|cha|cos|cre|cym|dan|deu|dzo|ell|eng|eus|fas|fin|fra|fre|glg|guj|hau|heb|hin|hrv|hun|hye|iii|isl|ita|kat|kik|kor|kur|lat|lim|lub|mkd|mol|msa|mya|nau|nde|nep|nld|nno|nor|nya|oci|oji|ori|oss|pan|que|ron|rus|san|srp|sin|slv|smo|sna|som|sqi|srp|ssw|sun|swa|tam|tel|tgk|tha|tir|ton|tso|twi|ukr|urd|uzb|ven|vie|vol|wol|xho|yid|yor|zho|zul|");
  static const QString other ("|alb|sq|arg|an|arm|hy|baq|eu|bul|bg|bur|my|cze|cs|chi|zh|cor|kw|wel|cy|ger|de|dut|nl|gre|el|per|fa|ful|ff|geo|ka|gla|gd|gle|ga|her|hz|scr|hr|ibo|ig|ice|is|jpn|ja|kau|kr|kaz|kk|kin|rw|kir|ky|kom|kv|kon|kg|kua|kj|ltz|lb|mac|mk|mao|mi|may|ms|nbl|nr|ndo|ng|por|pt|roh|rm|rum|ro|scc|sr|slo|sk|spa|es|srd|sc|swe|sv|tah|ty|tib|bo|wln|wa");
  static QRegExp re_lang_code ("^([^\\[]+)\\[([^\\]]+)\\]");
  if ( language.length() < 2 )
    return i18n("Track %1").arg (id);
  QString name;
  if ( re_lang_code.search (language) >= 0 )
  {
    name = re_lang_code.cap(2).simplifyWhiteSpace();
    if ( ! name.isEmpty() )
      language = name;
    name = re_lang_code.cap(1).simplifyWhiteSpace();
  }
  if ( language.length() == 3 )
  {
    QString code ('|' + language + '|');
    if ( middle.find (code, 0, false) >= 0 )
      language.remove (1, 1);
    else if ( right.find (code, 0, false) >= 0 )
      language.remove (2, 1);
    else
    {
      int index = other.find (code, 0, false);
      if ( index >= 0 )
        language = other.mid (index + 5, 2);
    }
  }
  QString locname (KGlobal::locale() -> twoAlphaToLanguageName (language));
  if ( locname.isEmpty() )
  {
    if ( language == "no" )
      language = I18N_NOOP("Norwegian");
    else if ( ! name.isEmpty() )
      language = name;
    return i18n(language.utf8());
  }
  return locname;
}

KPlayerTrackActionList::KPlayerTrackActionList (const QString& text, const QString& status,
  const QString& whatsthis, QObject* parent, const char* name)
  : KPlayerActionList (text, status, whatsthis, parent, name)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Creating track action list\n";
#endif
}

KPlayerTrackActionList::~KPlayerTrackActionList()
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Destroying track action list\n";
#endif
}

void KPlayerTrackActionList::addActions (const QMap<int, QString>& ids, int id)
{
  QMap<int, QString>::ConstIterator iterator (ids.constBegin()), end (ids.constEnd());
  while ( iterator != end )
  {
    QString text (languageName (iterator.key(), iterator.data()));
#ifdef DEBUG_KPLAYER_ACTIONLIST
    kdDebugTime() << " Stream " << iterator.key() << " " << iterator.data() << " " << text << "\n";
#endif
    KToggleAction* action = new KToggleAction (text, 0, this, SLOT(actionActivated()), this);
    updateAction (action);
    action -> setExclusiveGroup (name());
    if ( id == iterator.key() )
      action -> setChecked (true);
    m_actions.append (action);
    ++ iterator;
  }
}

void KPlayerTrackActionList::update (const QMap<int, QString>& ids, int id)
{
  unplug();
  if ( ids.count() > 1 )
    addActions (ids, id);
  plug();
}

void KPlayerTrackActionList::actionActivated (KAction* action, int index)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerTrackActionList::actionActivated\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  KPlayerActionList::actionActivated (action, ((KToggleAction*) action) -> isChecked() ? index : -1);
}

KPlayerSubtitleTrackActionList::KPlayerSubtitleTrackActionList (const QString& text, const QString& status,
  const QString& whatsthis, QObject* parent, const char* name)
  : KPlayerTrackActionList (text, status, whatsthis, parent, name)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Creating subtitle track action list\n";
#endif
}

KPlayerSubtitleTrackActionList::~KPlayerSubtitleTrackActionList()
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "Destroying subtitle track action list\n";
#endif
}

void KPlayerSubtitleTrackActionList::update (bool show, const QMap<int, QString>& sids, int sid,
  const QMap<int, QString>& vsids, int vsid, QStringList files, const QString& vobsub, const QString& current)
{
  unplug();
  if ( sids.count() > 0 || vsids.count() > 0 || files.count() > 0 )
  {
    KToggleAction* action = new KToggleAction (i18n("&None"), 0, this, SLOT(actionActivated()), this);
    action -> setStatusText (i18n("Turns off subtitle display"));
    action -> setWhatsThis (i18n("Subtitles None command turns off subtitle display."));
    action -> setExclusiveGroup (name());
    if ( ! show )
      action -> setChecked (true);
    m_actions.append (action);
    addActions (sids, sid);
    addActions (vsids, vsid);
    if ( vsids.isEmpty() )
      files << vobsub;
    QStringList::ConstIterator iterator (files.constBegin()), end (files.constEnd());
    while ( iterator != end )
    {
      QString text ((*iterator).section ('/', -1, -1));
#ifdef DEBUG_KPLAYER_ACTIONLIST
      kdDebugTime() << " Subtitle file '" << text << "' " << *iterator << "\n";
#endif
      if ( ! text.isEmpty() )
      {
        action = new KToggleAction (text, 0, this, SLOT (actionActivated()), this);
        updateAction (action);
        action -> setText (text);
        action -> setExclusiveGroup (name());
        if ( show && sid < 0 && vsid < 0 && *iterator == current )
          action -> setChecked (true);
        m_actions.append (action);
      }
      ++ iterator;
    }
  }
  plug();
}

void KPlayerSubtitleTrackActionList::actionActivated (KAction* action, int index)
{
#ifdef DEBUG_KPLAYER_ACTIONLIST
  kdDebugTime() << "KPlayerSubtitleTrackActionList::actionActivated\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  if ( ! ((KToggleAction*) action) -> isChecked() )
  {
    index = 0;
    ((KToggleAction*) actions().getFirst()) -> setChecked (true);
  }
  KPlayerActionList::actionActivated (action, index);
}
