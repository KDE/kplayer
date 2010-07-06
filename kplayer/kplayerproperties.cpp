/***************************************************************************
                          kplayerproperties.cpp
                          ---------------------
    begin                : Tue Feb 10 2004
    copyright            : (C) 2004-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <fcntl.h>
#include <kconfig.h>
#include <klocale.h>
#include <qfileinfo.h>
#include <qregexp.h>

//#define KPLAYER_EXTRACT_META_INFORMATION
#define KPLAYER_PROCESS_SIZE_IDS
#ifdef DEBUG
#define DEBUG_KPLAYER_PROPERTIES
#endif

#include "kplayerproperties.h"
#include "kplayerengine.h"
#include "kplayersettings.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

const KUrl KPlayerProperties::nullUrl;
const QSize KPlayerProperties::nullSize;
const QString KPlayerProperties::nullString;
const QStringList KPlayerProperties::nullStringList;
const QMap<int, QString> KPlayerProperties::nullIntegerStringMap;
KPlayerPropertyInfoMap KPlayerProperties::m_info;
KPlayerStringPropertyInfo KPlayerProperties::m_meta_info;
QStringList KPlayerProperties::m_meta_attributes;
KPlayerMediaMap KPlayerMedia::m_media_map;
int KPlayerItemProperties::m_meta_info_timer = 0;

#ifdef DEBUG
kdbgstream kdDebugTime (void)
{
  return kDebugStream (QtDebugMsg, 0).nospace() << QTime::currentTime().toString ("hh:mm:ss:zzz ").toLatin1().data();
}
#endif

int compareStrings (const QString& s1, const QString& s2)
{
  int r = s1.toLower().localeAwareCompare (s2.toLower());
  return r ? r : s1.localeAwareCompare (s2);
}

QString timeString (float l, bool zero_ok)
{
  l += 0.02;
  if ( ! zero_ok && l < 0.05 )
    return QString ("");
  int lHour = int (l) / 3600;
  if ( lHour > 0 )
    l -= lHour * 3600;
  if ( l >= 3600 )
  {
    lHour ++;
    l -= 3600;
  }
  int lMinute = int (l) / 60;
  if ( lMinute > 0 )
    l -= lMinute * 60;
  if ( l >= 60 )
  {
    lMinute ++;
    l -= 60;
  }
  QString s;
  if ( lHour > 0 )
    s.sprintf ("%u:%02u:%04.1f", lHour, lMinute, l);
  else if ( lMinute > 0 )
    s.sprintf ("%u:%04.1f", lMinute, l);
  else
    s.sprintf ("%03.1f", l);
  return s;
}

float stringToFloat (QString stime)
{
  int comma = stime.indexOf (',');
  if ( comma >= 0 )
    stime [comma] = '.';
  QStringList sl = stime.split (':');
  int i = 0, n = 0;
  if ( sl.count() > 4 || sl.count() < 1 )
    return 0;
  if ( sl.count() > 3 )
    n = sl[i++].toInt() * 86400;
  if ( sl.count() > 2 )
    n += sl[i++].toInt() * 3600;
  if ( sl.count() > 1 )
    n += sl[i++].toInt() * 60;
  return sl[i].toFloat() + n;
}

static void capitalize (QString& key)
{
  for ( int n = 0; n < key.length(); n ++ )
    if ( key[n].isLetter() )
    {
      key[n] = key[n].toUpper();
      return;
    }
}

static void capitalizeWords (QString& key)
{
  bool inword = false;
  for ( int n = 0; n < key.length(); n ++ )
    if ( ! key[n].isLetter() )
      inword = false;
    else if ( ! inword )
    {
      key[n] = key[n].toUpper();
      inword = true;
    }
}

KPlayerPropertyInfo::KPlayerPropertyInfo (void)
{
  m_group = -1;
  m_show = false;
  m_can_edit = false;
  m_can_reset = true;
  m_override = false;
}

KPlayerPropertyInfo::~KPlayerPropertyInfo()
{
}

bool KPlayerPropertyInfo::exists (KPlayerProperties* properties, const QString& name) const
{
  return properties -> configGroup().hasKey (name);
}

KPlayerBooleanPropertyInfo::KPlayerBooleanPropertyInfo (void)
{
  m_default = false;
}

KPlayerBooleanPropertyInfo::~KPlayerBooleanPropertyInfo()
{
}

KPlayerProperty* KPlayerBooleanPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerBooleanProperty;
}

KPlayerProperty* KPlayerBooleanPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerBooleanProperty (* (KPlayerBooleanProperty*) property);
}

KPlayerIntegerPropertyInfo::KPlayerIntegerPropertyInfo (void)
{
  m_default = 0;
}

KPlayerIntegerPropertyInfo::~KPlayerIntegerPropertyInfo()
{
}

KPlayerProperty* KPlayerIntegerPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerIntegerProperty;
}

KPlayerProperty* KPlayerIntegerPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerIntegerProperty (* (KPlayerIntegerProperty*) property);
}

KPlayerRelativePropertyInfo::~KPlayerRelativePropertyInfo()
{
}

KPlayerProperty* KPlayerRelativePropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerRelativeProperty;
}

KPlayerProperty* KPlayerRelativePropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerRelativeProperty (* (KPlayerRelativeProperty*) property);
}

KPlayerCachePropertyInfo::~KPlayerCachePropertyInfo()
{
}

KPlayerProperty* KPlayerCachePropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerCacheProperty;
}

KPlayerProperty* KPlayerCachePropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerCacheProperty (* (KPlayerCacheProperty*) property);
}

KPlayerFrequencyPropertyInfo::~KPlayerFrequencyPropertyInfo()
{
}

KPlayerProperty* KPlayerFrequencyPropertyInfo::create (KPlayerProperties* properties) const
{
  return new KPlayerFrequencyProperty (properties);
}

KPlayerProperty* KPlayerFrequencyPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerFrequencyProperty (* (KPlayerFrequencyProperty*) property);
}

bool KPlayerFrequencyPropertyInfo::exists (KPlayerProperties* properties, const QString&) const
{
  return properties -> needsFrequency();
}

KPlayerFloatPropertyInfo::KPlayerFloatPropertyInfo (void)
{
  m_default = 0;
}

KPlayerFloatPropertyInfo::~KPlayerFloatPropertyInfo()
{
}

KPlayerProperty* KPlayerFloatPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerFloatProperty;
}

KPlayerProperty* KPlayerFloatPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerFloatProperty (* (KPlayerFloatProperty*) property);
}

KPlayerLengthPropertyInfo::~KPlayerLengthPropertyInfo()
{
}

KPlayerProperty* KPlayerLengthPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerLengthProperty;
}

KPlayerProperty* KPlayerLengthPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerLengthProperty (* (KPlayerLengthProperty*) property);
}

KPlayerSizePropertyInfo::~KPlayerSizePropertyInfo()
{
}

KPlayerProperty* KPlayerSizePropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerSizeProperty;
}

KPlayerProperty* KPlayerSizePropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerSizeProperty (* (KPlayerSizeProperty*) property);
}

KPlayerDisplaySizePropertyInfo::~KPlayerDisplaySizePropertyInfo()
{
}

KPlayerProperty* KPlayerDisplaySizePropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerDisplaySizeProperty;
}

KPlayerProperty* KPlayerDisplaySizePropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerDisplaySizeProperty (* (KPlayerDisplaySizeProperty*) property);
}

KPlayerStringPropertyInfo::~KPlayerStringPropertyInfo()
{
}

KPlayerProperty* KPlayerStringPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerStringProperty;
}

KPlayerProperty* KPlayerStringPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerStringProperty (* (KPlayerStringProperty*) property);
}

KPlayerComboStringPropertyInfo::~KPlayerComboStringPropertyInfo()
{
}

KPlayerProperty* KPlayerComboStringPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerComboStringProperty;
}

KPlayerProperty* KPlayerComboStringPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerComboStringProperty (* (KPlayerComboStringProperty*) property);
}

bool KPlayerComboStringPropertyInfo::exists (KPlayerProperties* properties, const QString& name) const
{
  return KPlayerStringPropertyInfo::exists (properties, name)
    || KPlayerStringPropertyInfo::exists (properties, name + " Option");
}

KPlayerTranslatedStringPropertyInfo::~KPlayerTranslatedStringPropertyInfo()
{
}

KPlayerProperty* KPlayerTranslatedStringPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerTranslatedStringProperty;
}

KPlayerProperty* KPlayerTranslatedStringPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerTranslatedStringProperty (* (KPlayerTranslatedStringProperty*) property);
}

/*
KPlayerStringHistoryPropertyInfo::~KPlayerStringHistoryPropertyInfo()
{
}

KPlayerProperty* KPlayerStringHistoryPropertyInfo::create (KPlayerProperties* properties) const
{
  return new KPlayerStringHistoryProperty;
}

KPlayerProperty* KPlayerStringHistoryPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerStringHistoryProperty (* (KPlayerStringHistoryProperty*) property);
}

bool KPlayerStringHistoryPropertyInfo::exists (KPlayerProperties* properties, const QString& name) const
{
  return KPlayerStringPropertyInfo::exists (properties, name)
    || KPlayerStringPropertyInfo::exists (properties, name + " 0");
}
*/

KPlayerNamePropertyInfo::~KPlayerNamePropertyInfo()
{
}

KPlayerProperty* KPlayerNamePropertyInfo::create (KPlayerProperties* properties) const
{
  return new KPlayerNameProperty (properties);
}

KPlayerProperty* KPlayerNamePropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerNameProperty (* (KPlayerNameProperty*) property);
}

bool KPlayerNamePropertyInfo::exists (KPlayerProperties* properties, const QString&) const
{
  return properties != properties -> configuration();
}

KPlayerAppendablePropertyInfo::~KPlayerAppendablePropertyInfo()
{
}

KPlayerProperty* KPlayerAppendablePropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerAppendableProperty;
}

KPlayerProperty* KPlayerAppendablePropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerAppendableProperty (* (KPlayerAppendableProperty*) property);
}

bool KPlayerAppendablePropertyInfo::exists (KPlayerProperties* properties, const QString& name) const
{
  return KPlayerStringPropertyInfo::exists (properties, name)
    || KPlayerStringPropertyInfo::exists (properties, name + " Option");
}

KPlayerStringListPropertyInfo::~KPlayerStringListPropertyInfo()
{
}

KPlayerProperty* KPlayerStringListPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerStringListProperty;
}

KPlayerProperty* KPlayerStringListPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerStringListProperty (* (KPlayerStringListProperty*) property);
}

KPlayerIntegerStringMapPropertyInfo::~KPlayerIntegerStringMapPropertyInfo()
{
}

KPlayerProperty* KPlayerIntegerStringMapPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerIntegerStringMapProperty;
}

KPlayerProperty* KPlayerIntegerStringMapPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerIntegerStringMapProperty (* (KPlayerIntegerStringMapProperty*) property);
}

KPlayerUrlPropertyInfo::~KPlayerUrlPropertyInfo()
{
}

KPlayerProperty* KPlayerUrlPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerUrlProperty;
}

KPlayerProperty* KPlayerUrlPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerUrlProperty (* (KPlayerUrlProperty*) property);
}

KPlayerPersistentUrlPropertyInfo::~KPlayerPersistentUrlPropertyInfo()
{
}

KPlayerProperty* KPlayerPersistentUrlPropertyInfo::create (KPlayerProperties*) const
{
  return new KPlayerPersistentUrlProperty;
}

KPlayerProperty* KPlayerPersistentUrlPropertyInfo::copy (const KPlayerProperty* property) const
{
  return new KPlayerPersistentUrlProperty (* (KPlayerPersistentUrlProperty*) property);
}

KPlayerProperty::~KPlayerProperty()
{
}

QString KPlayerProperty::asString (void) const
{
  return QString();
}

void KPlayerProperty::fromString (const QString&)
{
}

int KPlayerProperty::compare (KPlayerProperty*) const
{
  return 0;
}

void KPlayerProperty::read (KConfigGroup&, const QString&)
{
}

void KPlayerProperty::save (KConfigGroup&, const QString&) const
{
}

bool KPlayerProperty::defaults (bool can_reset)
{
  return can_reset;
}

KPlayerBooleanProperty::~KPlayerBooleanProperty()
{
}

QString KPlayerBooleanProperty::asString (void) const
{
  return value() ? i18n("yes") : i18n("no");
}

int KPlayerBooleanProperty::compare (KPlayerProperty* property) const
{
  return value() == ((KPlayerBooleanProperty*) property) -> value() ? 0 : value() ? 1 : -1;
}

void KPlayerBooleanProperty::read (KConfigGroup& config, const QString& name)
{
  setValue (config.readEntry (name, value()));
}

void KPlayerBooleanProperty::save (KConfigGroup& config, const QString& name) const
{
  config.writeEntry (name, value());
}

KPlayerIntegerProperty::~KPlayerIntegerProperty()
{
}

QString KPlayerIntegerProperty::asString (void) const
{
  return QString::number (value());
}

void KPlayerIntegerProperty::fromString (const QString& value)
{
  setValue (value.toInt());
}

int KPlayerIntegerProperty::compare (KPlayerProperty* property) const
{
  int property_value = ((KPlayerIntegerProperty*) property) -> value();
  return value() == property_value ? 0 : value() > property_value ? 1 : -1;
}

void KPlayerIntegerProperty::read (KConfigGroup& config, const QString& name)
{
  setValue (config.readEntry (name, value()));
}

void KPlayerIntegerProperty::save (KConfigGroup& config, const QString& name) const
{
  config.writeEntry (name, value());
}

KPlayerRelativeProperty::~KPlayerRelativeProperty()
{
}

int KPlayerRelativeProperty::value (int current) const
{
  return option() == 1 ? current + value() : option() == 2 ? current - value() : value();
}

void KPlayerRelativeProperty::setValue (int value, int current)
{
  if ( value > current )
  {
    setOption (1);
    setValue (value - current);
  }
  else
  {
    setOption (2);
    setValue (current - value);
  }
}

QString KPlayerRelativeProperty::asString (void) const
{
  return (option() == 0 ? '=' : option() > 0 ? '+' : '-') + KPlayerIntegerProperty::asString();
}

int KPlayerRelativeProperty::compare (KPlayerProperty* property) const
{
  int result, property_option = ((KPlayerRelativeProperty*) property) -> option();
  if ( option() == property_option )
  {
    result = KPlayerIntegerProperty::compare (property);
    if ( option() == 2 )
      result = - result;
  }
  else
    result = option() == 1 ? 1 : property_option == 1 ? -1 : option() > property_option ? -1 : 1;
  return result;
}

void KPlayerRelativeProperty::read (KConfigGroup& config, const QString& name)
{
  KPlayerIntegerProperty::read (config, name);
  setOption (config.readEntry (name + " Option", 0));
}

void KPlayerRelativeProperty::save (KConfigGroup& config, const QString& name) const
{
  if ( option() )
  {
    KPlayerIntegerProperty::save (config, name);
    config.writeEntry (name + " Option", option());
  }
}

KPlayerCacheProperty::~KPlayerCacheProperty()
{
}

QString KPlayerCacheProperty::asString (void) const
{
  return value() > 3 ? KPlayerIntegerProperty::asString() : QString();
}

int KPlayerCacheProperty::compare (KPlayerProperty* property) const
{
  int property_value = ((KPlayerCacheProperty*) property) -> value();
  return value() == property_value || value() < 4 && property_value < 4 ? 0 : value() > property_value ? 1 : -1;
}

void KPlayerCacheProperty::read (KConfigGroup& config, const QString& name)
{
  KPlayerIntegerProperty::read (config, name);
  if ( value() == 2 )
    setValue (config.readEntry (name + " Size", value()));
}

KPlayerFrequencyProperty::~KPlayerFrequencyProperty()
{
}

QString KPlayerFrequencyProperty::asString (void) const
{
  return QString::number ((float) value() / 1000);
}

int KPlayerFrequencyProperty::compare (KPlayerProperty* property) const
{
  int my_value = value();
  int property_value = ((KPlayerFrequencyProperty*) property) -> value();
  return my_value == property_value ? 0 : my_value > property_value ? 1 : -1;
}

void KPlayerFrequencyProperty::save (KConfigGroup& config, const QString& name) const
{
  if ( m_value )
    KPlayerIntegerProperty::save (config, name);
}

bool KPlayerFrequencyProperty::defaults (bool)
{
  m_value = 0;
  return false;
}

KPlayerFloatProperty::~KPlayerFloatProperty()
{
}

QString KPlayerFloatProperty::asString (void) const
{
  return QString::number (value());
}

int KPlayerFloatProperty::compare (KPlayerProperty* property) const
{
  float property_value = ((KPlayerFloatProperty*) property) -> value();
  return value() == property_value ? 0 : value() > property_value ? 1 : -1;
}

void KPlayerFloatProperty::read (KConfigGroup& config, const QString& name)
{
  setValue (config.readEntry (name, (double) value()));
}

void KPlayerFloatProperty::save (KConfigGroup& config, const QString& name) const
{
  config.writeEntry (name, (double) value());
}

KPlayerLengthProperty::~KPlayerLengthProperty()
{
}

QString KPlayerLengthProperty::asString (void) const
{
  return timeString (value());
}

KPlayerSizeProperty::~KPlayerSizeProperty()
{
}

QString KPlayerSizeProperty::asString (void) const
{
  return QString::number (value().width()) + "x" + QString::number (value().height());
}

int KPlayerSizeProperty::compare (KPlayerProperty* property) const
{
  const QSize& property_value = ((KPlayerSizeProperty*) property) -> value();
  int area = value().width() * value().height();
  int property_area = property_value.width() * property_value.height();
  return area == property_area ? 0 : area > property_area ? 1 : -1;
}

void KPlayerSizeProperty::read (KConfigGroup& config, const QString& name)
{
  setValue (config.readEntry (name, value()));
}

void KPlayerSizeProperty::save (KConfigGroup& config, const QString& name) const
{
  config.writeEntry (name, value());
}

KPlayerDisplaySizeProperty::~KPlayerDisplaySizeProperty()
{
}

QSize KPlayerDisplaySizeProperty::value (const QSize& current) const
{
  if ( option() == 1 )
    return value();
  return QSize (current.width(), value().height() * current.width() / value().width());
}

void KPlayerDisplaySizeProperty::setValue (const QSize& value, int option)
{
  setOption (option);
  setValue (value);
}

QString KPlayerDisplaySizeProperty::asString (void) const
{
  QString s (KPlayerSizeProperty::asString());
  return QString (option() == 1 ? i18n("size %1", s) : i18n("aspect %1", s));
}

int KPlayerDisplaySizeProperty::compare (KPlayerProperty* property) const
{
  int property_option = ((KPlayerDisplaySizeProperty*) property) -> option();
  return option() == property_option ? KPlayerSizeProperty::compare (property) : option() > property_option ? 1 : -1;
}

void KPlayerDisplaySizeProperty::read (KConfigGroup& config, const QString& name)
{
  KPlayerSizeProperty::read (config, name);
  setOption (config.readEntry (name + " Option", 1));
}

void KPlayerDisplaySizeProperty::save (KConfigGroup& config, const QString& name) const
{
  KPlayerSizeProperty::save (config, name);
  if ( option() != 1 )
    config.writeEntry (name + " Option", option());
}

KPlayerStringProperty::~KPlayerStringProperty()
{
}

QString KPlayerStringProperty::asString (void) const
{
  return value();
}

void KPlayerStringProperty::fromString (const QString& value)
{
  setValue (value);
}

int KPlayerStringProperty::compare (KPlayerProperty* property) const
{
  return compareStrings (asString(), property -> asString());
}

void KPlayerStringProperty::read (KConfigGroup& config, const QString& name)
{
  m_value = config.readEntry (name, value());
}

void KPlayerStringProperty::save (KConfigGroup& config, const QString& name) const
{
  if ( ! value().isEmpty() )
    config.writeEntry (name, value());
}

KPlayerComboStringProperty::~KPlayerComboStringProperty()
{
}

QString KPlayerComboStringProperty::asString (void) const
{
  return option().isNull() ? value() : option();
}

void KPlayerComboStringProperty::read (KConfigGroup& config, const QString& name)
{
  KPlayerStringProperty::read (config, name);
  setOption (config.readEntry (name + " Option", option()));
}

void KPlayerComboStringProperty::save (KConfigGroup& config, const QString& name) const
{
  KPlayerStringProperty::save (config, name);
  if ( ! option().isNull() )
    config.writeEntry (name + " Option", option());
}

bool KPlayerComboStringProperty::defaults (bool)
{
  setOption (QString());
  return value().isEmpty();
}

KPlayerTranslatedStringProperty::~KPlayerTranslatedStringProperty()
{
}

QString KPlayerTranslatedStringProperty::asString (void) const
{
  if ( value().indexOf ('/') < 0 )
    return i18n(value().toUtf8());
  if ( value() == "video/avi" || value() == "video/x-msvideo" )
    return i18n("AVI Video");
  KMimeType::Ptr mimetype (KMimeType::mimeType (value()));
  if ( mimetype.isNull() || mimetype -> name() == "application/octet-stream" || mimetype -> comment().isEmpty() )
    return value();
  return mimetype -> comment();
}

/*
KPlayerStringHistoryProperty::~KPlayerStringHistoryProperty()
{
}

void KPlayerStringHistoryProperty::setValue (const QString& value)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerStringHistoryProperty::setValue\n";
#endif
  KPlayerStringProperty::setValue (value);
  if ( ! value.isEmpty() )
  {
    m_history.remove (value);
    m_history.prepend (value);
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " History " << value << "\n";
#endif
  }
}

void KPlayerStringHistoryProperty::read (KConfigGroup& config, const QString& name)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerStringHistoryProperty::read\n";
#endif
  KPlayerStringProperty::read (config, name);
  if ( ! value().isEmpty() )
  {
    m_history << value();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " History " << value() << "\n";
#endif
  }
  QString history;
  for ( int i = 0; i < 10 && config.hasKey (history = name + " " + QString::number (i)); i ++ )
  {
    m_history << config.readEntry (history);
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " History " << m_history.last() << "\n";
#endif
  }
}

void KPlayerStringHistoryProperty::save (KConfigGroup& config, const QString& name) const
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerStringHistoryProperty::save\n";
#endif
  KPlayerStringProperty::save (config, name);
  QStringList::ConstIterator it (history().constBegin());
  if ( it != history().constEnd() && *it == value() )
    ++ it;
  for ( int i = 0; i < 10 && it != history().constEnd(); i ++ )
  {
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " History " << *it << "\n";
#endif
    config.writeEntry (name + " " + QString::number (i), * it ++);
  }
}
*/

KPlayerNameProperty::~KPlayerNameProperty()
{
}

QString KPlayerNameProperty::asString (void) const
{
  return value().isEmpty() ? m_properties -> defaultName() : value();
}

void KPlayerNameProperty::save (KConfigGroup& config, const QString& name) const
{
  if ( ! value().isEmpty() && value() != m_properties -> defaultName() )
    KPlayerStringProperty::save (config, name);
}

bool KPlayerNameProperty::defaults (bool)
{
  setValue (QString());
  return false;
}

KPlayerAppendableProperty::~KPlayerAppendableProperty()
{
}

QString KPlayerAppendableProperty::appendableValue (const QString& current) const
{
  if ( option() == 0 )
    return value();
  return value().isEmpty() ? current : current.isEmpty() ? value() : current + " " + value();
}

void KPlayerAppendableProperty::setAppendableValue (const QString& value, bool append)
{
  setOption (append);
  setValue (value);
}

void KPlayerAppendableProperty::read (KConfigGroup& config, const QString& name)
{
  KPlayerStringProperty::read (config, name);
  setOption (config.readEntry (name + " Option", false));
}

void KPlayerAppendableProperty::save (KConfigGroup& config, const QString& name) const
{
  KPlayerStringProperty::save (config, name);
  if ( option() )
    config.writeEntry (name + " Option", option());
}

KPlayerStringListProperty::~KPlayerStringListProperty()
{
}

bool KPlayerStringListProperty::defaults (bool)
{
  return false;
}

void KPlayerStringListProperty::read (KConfigGroup& config, const QString& name)
{
  int total = config.readEntry (name, 0);
  for ( int i = 0; i < total; i ++ )
    m_value.append (config.readEntry ("Child" + QString::number (i)));
}

void KPlayerStringListProperty::save (KConfigGroup& config, const QString& name) const
{
  int i = 0;
  QStringList::ConstIterator iterator (value().constBegin());
  while ( iterator != value().constEnd() )
  {
    config.writeEntry ("Child" + QString::number (i), *iterator);
    ++ iterator;
    ++ i;
  }
  if ( value().count() )
    config.writeEntry (name, value().count());
}

KPlayerIntegerStringMapProperty::~KPlayerIntegerStringMapProperty()
{
}

void KPlayerIntegerStringMapProperty::read (KConfigGroup& config, const QString& name)
{
  static QRegExp re_indexvalue ("^(\\d+)=(.*)$");
  QStringList values (config.readEntry (name).split (':'));
  QStringList::ConstIterator iterator (values.constBegin());
  while ( iterator != values.constEnd() )
  {
    if ( re_indexvalue.indexIn (*iterator) >= 0 )
      m_value.insert (re_indexvalue.cap (1).toInt(), re_indexvalue.cap (2));
    else
      m_value.insert ((*iterator).toInt(), QString());
    ++ iterator;
  }
}

void KPlayerIntegerStringMapProperty::save (KConfigGroup& config, const QString& name) const
{
  if ( value().count() > 1 || (! value().isEmpty()
    && ! ((KPlayerIntegerStringMapPropertyInfo*) KPlayerProperties::info (name)) -> multipleEntriesRequired() ) )
  {
    QStringList values;
    QMap<int, QString>::ConstIterator iterator (value().constBegin());
    while ( iterator != value().constEnd() )
    {
      QString value (QString::number (iterator.key()));
      if ( ! iterator.value().isEmpty() )
        value += "=" + iterator.value();
      values.append (value);
      ++ iterator;
    }
    config.writeEntry (name, values.join (":"));
  }
}

KPlayerUrlProperty::~KPlayerUrlProperty()
{
}

QString KPlayerUrlProperty::asString (void) const
{
  return value().isLocalFile() ? value().path() : value().prettyUrl();
}

int KPlayerUrlProperty::compare (KPlayerProperty* property) const
{
  const KUrl& property_url (((KPlayerUrlProperty*) property) -> value());
  return value().isLocalFile() == property_url.isLocalFile() ? value().url().compare (property_url.url())
    : value().isLocalFile() ? 1 : -1;
}

KPlayerPersistentUrlProperty::~KPlayerPersistentUrlProperty()
{
}

void KPlayerPersistentUrlProperty::read (KConfigGroup& config, const QString& name)
{
  setValue (config.readEntry (name, value().url()));
}

void KPlayerPersistentUrlProperty::save (KConfigGroup& config, const QString& name) const
{
  config.writeEntry (name, value().url());
}

bool KPlayerPersistentUrlProperty::defaults (bool)
{
  return true;
}

int KPlayerPropertyCounts::count (const QString& key) const
{
  KPlayerPropertyCounts::ConstIterator iterator = find (key);
  return iterator == constEnd() ? 0 : iterator.value();
}

void KPlayerPropertyCounts::add (const KPlayerPropertyCounts& counts)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Adding property counts\n";
#endif
  for ( KPlayerPropertyCounts::ConstIterator iterator = counts.constBegin(); iterator != counts.constEnd(); ++ iterator )
  {
    KPlayerPropertyCounts::Iterator it = find (iterator.key());
    if ( it == end() )
      insert (iterator.key(), iterator.value());
    else
      *it += iterator.value();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " " << iterator.key().toLatin1().data() << " + " << iterator.value() << " = " << count (iterator.key()) << "\n";
#endif
  }
}

void KPlayerPropertyCounts::subtract (const KPlayerPropertyCounts& counts)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Subtracting property counts\n";
#endif
  for ( KPlayerPropertyCounts::ConstIterator iterator = counts.constBegin(); iterator != counts.constEnd(); ++ iterator )
  {
    int value = count (iterator.key());
    if ( value > iterator.value() )
      insert (iterator.key(), value - iterator.value());
    else if ( value > 0 )
      remove (iterator.key());
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " " << iterator.key().toLatin1().data() << " - " << iterator.value() << " = " << count (iterator.key()) << "\n";
#endif
  }
}

KPlayerProperties::KPlayerProperties (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating properties\n";
#endif
}

KPlayerProperties::~KPlayerProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying properties\n";
#endif
  cleanup();
  KPlayerPropertyMap::ConstIterator iterator = properties().constBegin();
  while ( iterator != properties().constEnd() )
  {
    delete iterator.value();
    ++ iterator;
  }
}

void KPlayerProperties::setup (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::setup\n";
#endif
  setupInfo();
  load();
  setupMeta();
  cleanup();
}

void KPlayerProperties::setupInfo (void)
{
  m_config_group = config() -> group (configGroupName());
  if ( configGroup().hasKey ("Subtitle Position") )
  {
    int value = configGroup().readEntry ("Subtitle Position", 100);
    if ( value < 0 || value > 100 )
      configGroup().deleteEntry ("Subtitle Position");
  }
}

void KPlayerProperties::setupMeta (void)
{
}

void KPlayerProperties::defaults (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::defaults\n";
#endif
  for ( KPlayerPropertyInfoMap::ConstIterator iterator = m_info.constBegin(); iterator != m_info.constEnd(); ++ iterator )
    if ( has (iterator.key()) && m_properties [iterator.key()] -> defaults (iterator.value() -> canReset()) )
    {
      delete m_properties [iterator.key()];
      m_properties.remove (iterator.key());
    }
}

void KPlayerProperties::load (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::load\n";
  kdDebugTime() << " Group  " << configGroupName() << "\n";
#endif
  KPlayerPropertyInfoMap::ConstIterator iterator (m_info.constBegin());
  while ( iterator != m_info.constEnd() )
  {
    if ( iterator.value() -> exists (this, iterator.key()) )
    {
      KPlayerProperty* property = iterator.value() -> create (this);
      property -> read (configGroup(), iterator.key());
      m_properties.insert (iterator.key(), property);
#ifdef DEBUG_KPLAYER_PROPERTIES
      kdDebugTime() << " " << iterator.key().toLatin1().data() << " " << property -> asString().toLatin1().data() << "\n";
#endif
    }
    ++ iterator;
  }
  if ( configGroup().hasKey ("Keys") )
  {
    QStringList keys (configGroup().readEntry ("Keys").split (';'));
    QStringList::ConstIterator keysit (keys.constBegin());
    while ( keysit != keys.constEnd() )
    {
      if ( configGroup().hasKey (*keysit) )
      {
        KPlayerProperty* property = m_meta_info.create (this);
        property -> read (configGroup(), *keysit);
        m_properties.insert (*keysit, property);
#ifdef DEBUG_KPLAYER_PROPERTIES
        kdDebugTime() << " " << *keysit << " " << property -> asString().toLatin1().data() << "\n";
#endif
      }
      ++ keysit;
    }
  }
}

void KPlayerProperties::save (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::save\n";
  kdDebugTime() << " Group  " << configGroupName() << "\n";
#endif
  purge();
  QStringList keys;
  KPlayerPropertyMap::ConstIterator iterator (m_properties.constBegin());
  while ( iterator != m_properties.constEnd() )
  {
    iterator.value() -> save (configGroup(), iterator.key());
    if ( ! m_info.contains (iterator.key()) )
      keys.append (iterator.key());
    ++ iterator;
  }
  if ( ! keys.isEmpty() )
    configGroup().writeEntry ("Keys", keys.join (";"));
  if ( config() == KPlayerEngine::engine() -> meta() && ! configGroup().keyList().isEmpty() )
    configGroup().writeEntry ("Date", QDateTime::currentDateTime());
}

void KPlayerProperties::commit (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::commit\n";
#endif
  save();
  update();
  configGroup().sync();
}

void KPlayerProperties::update (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::update\n";
#endif
  emit updated();
  cleanup();
}

void KPlayerProperties::cleanup (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::cleanup\n";
#endif
  for ( KPlayerPropertyMap::ConstIterator iterator = m_previous.constBegin(); iterator != m_previous.constEnd(); ++ iterator )
    delete iterator.value();
  m_previous.clear();
  m_added.clear();
  m_changed.clear();
  m_removed.clear();
}

void KPlayerProperties::diff (KPlayerProperties* properties)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::diff\n";
#endif
  for ( KPlayerPropertyMap::ConstIterator iterator = m_properties.constBegin();
      iterator != m_properties.constEnd(); ++ iterator )
    if ( ! properties -> has (iterator.key()) )
      m_added.insert (iterator.key(), 1);
    else if ( compare (properties, iterator.key()) != 0 )
      m_changed.insert (iterator.key(), 1);
  for ( KPlayerPropertyMap::ConstIterator iterator = properties -> properties().constBegin();
      iterator != properties -> properties().constEnd(); ++ iterator )
    if ( ! has (iterator.key()) )
      m_removed.insert (iterator.key(), 1);
  update();
}

int KPlayerProperties::compare (KPlayerProperties* properties, const QString& key) const
{
  KPlayerProperty* p = property (key);
  KPlayerProperty* property = properties -> property (key);
  return p && property ? p -> compare (property) : p ? -1 : property ? 1 : 0;
}

QStringList KPlayerProperties::defaultOrder (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::defaultOrder\n";
#endif
  QStringList order (defaultAttributes());
  order << "";
  KPlayerPropertyInfoMap::ConstIterator iterator (m_info.constBegin());
  while ( iterator != m_info.constEnd() )
  {
    const QString& name (iterator.key());
    KPlayerPropertyInfo* info = iterator.value();
    if ( info -> canShow() && name != "Track" )
    {
      QStringList::Iterator it (order.begin());
      while ( it != order.end() )
      {
        KPlayerPropertyInfo* i = KPlayerMedia::info (*it);
        if ( i -> group() > info -> group() || (i -> group() == info -> group() && *it > name) )
          break;
        ++ it;
      }
      order.insert (it, name);
    }
    ++ iterator;
  }
  return order;
}

KPlayerPropertyInfo* KPlayerProperties::info (const QString& key)
{
  KPlayerPropertyInfoMap::ConstIterator iterator = m_info.constFind (key);
  return iterator == m_info.constEnd() ? &m_meta_info : *iterator;
}

KPlayerProperty* KPlayerProperties::property (const QString& key) const
{
  KPlayerPropertyMap::ConstIterator iterator = m_properties.find (key);
  return iterator == m_properties.constEnd() ? 0 : iterator.value();
}

KPlayerProperty* KPlayerProperties::get (const QString& key)
{
/*#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::get " << key << "\n";
  if ( m_info.contains (key) )
    kdDebugTime() << " has info for " << key << "\n";
  if ( has (key) )
    kdDebugTime() << " has property " << key << "\n";
#endif*/
  beginUpdate();
  KPlayerProperty* p = property (key);
  if ( ! p )
  {
    p = info (key) -> create (this);
    m_properties.insert (key, p);
  }
  return p;
}

void KPlayerProperties::reset (const QString& key)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerProperties::reset " << key << "\n";
#endif
  if ( has (key) )
  {
    beginUpdate();
    delete m_properties [key];
    m_properties.remove (key);
    m_added.remove (key);
    m_changed.remove (key);
    if ( had (key) )
      m_removed.insert (key, 1);
  }
}

void KPlayerProperties::beginUpdate (void)
{
  if ( m_previous.isEmpty() )
    for ( KPlayerPropertyMap::ConstIterator iterator = m_properties.constBegin(); iterator != m_properties.constEnd(); ++ iterator )
      m_previous.insert (iterator.key(), info (iterator.key()) -> copy (iterator.value()));
}

void KPlayerProperties::updated (const QString& key)
{
  if ( m_previous.isEmpty() )
    return;
  m_removed.remove (key);
  if ( had (key) )
  {
    m_added.remove (key);
    if ( m_previous [key] -> compare (m_properties [key]) )
      m_changed.insert (key, 1);
    else
      m_changed.remove (key);
  }
  else
  {
    m_changed.remove (key);
    m_added.insert (key, 1);
  }
}

int KPlayerProperties::getBooleanOption (const QString& key) const
{
  return ! has (key) ? 0 : getBoolean (key) ? 1 : 2;
}

void KPlayerProperties::setBooleanOption (const QString& key, int value)
{
  if ( value == 0 )
    reset (key);
  else
  {
    ((KPlayerBooleanProperty*) get (key)) -> setValue (value == 1);
    updated (key);
  }
}

void KPlayerProperties::set (const QString& key, int value)
{
  ((KPlayerIntegerProperty*) get (key)) -> setValue (value);
  updated (key);
}

int KPlayerProperties::getIntegerOption (const QString& key) const
{
  return has (key) ? getInteger (key) + 1 : 0;
}

void KPlayerProperties::setIntegerOption (const QString& key, int value)
{
  if ( value == 0 )
    reset (key);
  else
    set (key, value - 1);
}

int KPlayerProperties::getRelative (const QString& key) const
{
  return limit (getRelativeValue (key), getInteger (key + " Minimum"), getInteger (key + " Maximum"));
}

int KPlayerProperties::getRelativeOption (const QString& key) const
{
  return has (key) ? ((KPlayerRelativeProperty*) m_properties [key]) -> option() + 1 : 0;
}

void KPlayerProperties::setRelativeOption (const QString& key, int value, int option)
{
  if ( option == 0 || (value == 0 && option > 1 && option < 4) )
    reset (key);
  else
  {
    KPlayerRelativeProperty* property = (KPlayerRelativeProperty*) get (key);
    property -> setValue (value);
    property -> setOption (option - 1);
    updated (key);
  }
}

int KPlayerProperties::getCacheOption (const QString& key) const
{
  return has (key) ? getCache (key) + 1 : 0;
}

void KPlayerProperties::setCache (int cache, int size)
{
  setInteger ("Cache", cache < 2 ? cache : size <= 0 ? 1 : size < 4 ? 4 : size);
}

void KPlayerProperties::setCacheOption (int cache, int size)
{
  setIntegerOption ("Cache", cache < 3 ? cache : size <= 0 ? 2 : size < 4 ? 5 : size + 1);
}

void KPlayerProperties::set (const QString& key, float value)
{
  ((KPlayerFloatProperty*) get (key)) -> setValue (value);
  updated (key);
}

const QSize& KPlayerProperties::getSize (const QString& key) const
{
  return has (key) ? ((KPlayerSizeProperty*) m_properties [key]) -> value() : nullSize;
}

int KPlayerProperties::getSizeOption (const QString& key) const
{
  return has (key) ? ((KPlayerDisplaySizeProperty*) m_properties [key]) -> option() : 0;
}

void KPlayerProperties::setSize (const QString& key, const QSize& value)
{
  if ( value.isEmpty() )
    reset (key);
  else
  {
    ((KPlayerSizeProperty*) get (key)) -> setValue (value);
    updated (key);
  }
}

void KPlayerProperties::setSize (const QString& key, const QSize& value, int option)
{
  if ( value.isValid() && value.width() > 0 && option != 0 )
  {
    ((KPlayerDisplaySizeProperty*) get (key)) -> setValue (value, option);
    updated (key);
  }
  else
    reset (key);
}

QSize KPlayerProperties::getDisplaySize (const QString& key) const
{
  return getSize (key);
}

QString KPlayerProperties::asString (const QString& key) const
{
  return has (key) ? ((KPlayerProperty*) m_properties [key]) -> asString() : QString();
}

QString KPlayerProperties::asIntegerString (const QString& key) const
{
  return has (key) ? ((KPlayerIntegerProperty*) m_properties [key]) -> KPlayerIntegerProperty::asString() : QString();
}

void KPlayerProperties::fromString (const QString& key, const QString& value)
{
  if ( (value.isEmpty() && has (key)) || (! value.isEmpty() && value != asString (key)) )
  {
    if ( value.isEmpty() && m_properties [key] -> defaults (true) )
      reset (key);
    else
      get (key) -> fromString (value);
    updated (key);
    commit();
  }
}

void KPlayerProperties::set (const QString& key, const QString& value)
{
  ((KPlayerStringProperty*) get (key)) -> setValue (value);
  updated (key);
}

void KPlayerProperties::setString (const QString& key, const QString& value)
{
  const QString& d (stringInfo (key) -> defaultValue());
  if ( (d.isNull() && value.isEmpty()) || value == d )
    reset (key);
  else
    set (key, value);
}

const QString& KPlayerProperties::getStringValue (const QString& key) const
{
  return has (key) ? ((KPlayerStringProperty*) m_properties [key]) -> value() : nullString;
}

void KPlayerProperties::setComboValue (const QString& key, const QString& value)
{
  if ( value.isEmpty() )
    reset (key);
  else
    set (key, value);
}

bool KPlayerProperties::hasComboValue (const QString& key) const
{
  return has (key) && ! ((KPlayerStringProperty*) m_properties [key]) -> value().isEmpty();
}

const QString& KPlayerProperties::getStringOption (const QString& key) const
{
  return has (key) ? ((KPlayerComboStringProperty*) m_properties [key]) -> option() : nullString;
}

void KPlayerProperties::setStringOption (const QString& key, const QString& value)
{
  if ( value.isNull() && ! hasComboValue (key) )
    reset (key);
  else
  {
    ((KPlayerComboStringProperty*) get (key)) -> setOption (value);
    updated (key);
  }
}

bool KPlayerProperties::hasComboString (const QString& key) const
{
  return ! getComboString (key).isEmpty();
}

/*const QStringList& KPlayerProperties::getHistory (const QString& key) const
{
  return has (key) ? ((KPlayerStringHistoryProperty*) m_properties [key]) -> history() : nullStringList;
}*/

int KPlayerProperties::getAppendableOption (const QString& key) const
{
  return ! has (key) ? 0 : ((KPlayerAppendableProperty*) m_properties [key]) -> option() ? 2 : 1;
}

void KPlayerProperties::setAppendable (const QString& key, const QString& value, int option)
{
  if ( option == 0 || (option == 2 && value.isEmpty()) )
    reset (key);
  else
  {
    ((KPlayerAppendableProperty*) get (key)) -> setAppendableValue (value, option == 2);
    updated (key);
  }
}

const QStringList& KPlayerProperties::getStringList (const QString& key) const
{
  return has (key) ? ((KPlayerStringListProperty*) m_properties [key]) -> value() : nullStringList;
}

void KPlayerProperties::addStringListEntry (const QString& key, const QString& entry)
{
  ((KPlayerStringListProperty*) get (key)) -> addEntry (entry);
  updated (key);
}

void KPlayerProperties::setStringList (const QString& key, const QStringList& value)
{
  if ( value.isEmpty() )
    reset (key);
  else
  {
    ((KPlayerStringListProperty*) get (key)) -> setValue (value);
    updated (key);
  }
}

const QMap<int, QString>& KPlayerProperties::getIntegerStringMap (const QString& key) const
{
  return has (key) ? ((KPlayerIntegerStringMapProperty*) m_properties [key]) -> value() : nullIntegerStringMap;
}

void KPlayerProperties::setIntegerStringMapKey (const QString& key, int id)
{
  KPlayerIntegerStringMapProperty* property = (KPlayerIntegerStringMapProperty*) get (key);
  if ( ! property -> value().contains (id) )
  {
    property -> setValue (id, QString());
    updated (key);
  }
}

void KPlayerProperties::setIntegerStringMapKeyValue (const QString& key, int id, const QString& value)
{
  KPlayerIntegerStringMapProperty* property = (KPlayerIntegerStringMapProperty*) get (key);
  if ( property -> value() [id] != value )
  {
    property -> setValue (id, value);
    updated (key);
  }
}

bool KPlayerProperties::hasIntegerStringMapKey (const QString& key, int id) const
{
  return has (key) && ((KPlayerIntegerStringMapProperty*) m_properties [key]) -> value().contains (id);
}

bool KPlayerProperties::hasIntegerStringMapValue (const QString& key, int id) const
{
  return has (key) && ! ((KPlayerIntegerStringMapProperty*) m_properties [key]) -> value() [id].isEmpty();
}

const KUrl& KPlayerProperties::getUrl (const QString& key) const
{
  return has (key) ? ((KPlayerUrlProperty*) m_properties [key]) -> value() : nullUrl;
}

void KPlayerProperties::setUrl (const QString& key, const KUrl& value)
{
  if ( value.isValid() )
  {
    ((KPlayerUrlProperty*) get (key)) -> setValue (value);
    updated (key);
  }
  else
    reset (key);
}

bool KPlayerProperties::autoloadSubtitles (const QString&) const
{
  return false;
}

bool KPlayerProperties::getVobsubSubtitles (const QString&, const KUrl&) const
{
  return false;
}

bool KPlayerProperties::getPlaylist (const QString&, const KUrl&) const
{
  return false;
}

bool KPlayerProperties::getUseKioslave (const QString&, const KUrl&) const
{
  return false;
}

bool KPlayerProperties::needsFrequency (void) const
{
  return false;
}

void KPlayerProperties::count (KPlayerPropertyCounts& counts) const
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Counting properties\n";
#endif
  for ( KPlayerPropertyMap::ConstIterator iterator = properties().constBegin(); iterator != properties().constEnd(); ++ iterator )
  {
    KPlayerPropertyCounts::Iterator it = counts.find (iterator.key());
    if ( it == counts.end() )
#ifdef DEBUG_KPLAYER_PROPERTIES
      it =
#endif
        counts.insert (iterator.key(), 1);
    else
      ++ *it;
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " " << iterator.key().toLatin1().data() << " ++ = " << *it << "\n";
#endif
  }
}

void KPlayerProperties::initialize (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Initializing properties\n";
#endif
  KPlayerPropertyInfo* info = new KPlayerUrlPropertyInfo;
  info -> setCaption (i18n("Path"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_LOCATION);
  info -> setCanReset (false);
  m_info.insert ("Path", info);
  info = new KPlayerNamePropertyInfo;
  info -> setCaption (i18n("Name"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_BASIC);
  info -> setCanEdit (true);
  m_info.insert ("Name", info);
  info = new KPlayerStringListPropertyInfo;
  m_info.insert ("Children", info);
  info = new KPlayerPersistentUrlPropertyInfo;
  m_info.insert ("Origin", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Custom Order", info);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Group By", info);
  info = new KPlayerComboStringPropertyInfo;
  info -> setCaption (i18n("Demuxer"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_FORMAT);
  m_info.insert ("Demuxer", info);
  info = new KPlayerDisplaySizePropertyInfo;
  m_info.insert ("Display Size", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Full Screen", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Maximized", info);
  KPlayerBooleanPropertyInfo* boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Maintain Aspect", boolinfo);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Autoload Subtitles", boolinfo);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Subtitle Visibility", boolinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Closed Caption", info);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Subtitle Encoding", info);
  info = new KPlayerFloatPropertyInfo;
  m_info.insert ("Subtitle Framerate", info);
  KPlayerIntegerPropertyInfo* intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Subtitle Position", intinfo);
  info = new KPlayerFloatPropertyInfo;
  m_info.insert ("Subtitle Delay", info);
//info = new KPlayerBooleanPropertyInfo;
//m_info.insert ("Autoexpanded", info);
  intinfo = new KPlayerRelativePropertyInfo;
  intinfo -> setDefaultValue (50);
  m_info.insert ("Volume", intinfo);
  info = new KPlayerFloatPropertyInfo;
  m_info.insert ("Audio Delay", info);
  info = new KPlayerRelativePropertyInfo;
  m_info.insert ("Contrast", info);
  info = new KPlayerRelativePropertyInfo;
  m_info.insert ("Brightness", info);
  info = new KPlayerRelativePropertyInfo;
  m_info.insert ("Hue", info);
  info = new KPlayerRelativePropertyInfo;
  m_info.insert ("Saturation", info);
  KPlayerStringPropertyInfo* strinfo = new KPlayerStringPropertyInfo;
  strinfo -> setDefaultValue ("xv");
  m_info.insert ("Video Driver", strinfo);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Video Device", info);
  info = new KPlayerComboStringPropertyInfo;
  info -> setCaption (i18n("Video codec"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_VIDEO);
  m_info.insert ("Video Codec", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Video Scaler", info);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Double Buffering", boolinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Direct Rendering", info);
  strinfo = new KPlayerStringPropertyInfo;
  strinfo -> setDefaultValue ("alsa");
  m_info.insert ("Audio Driver", strinfo);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Audio Device", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Software Volume", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (200);
  m_info.insert ("Maximum Software Volume", intinfo);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Mixer Device", info);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Mixer Channel", info);
  info = new KPlayerComboStringPropertyInfo;
  info -> setCaption (i18n("Audio codec"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_AUDIO);
  m_info.insert ("Audio Codec", info);
  strinfo = new KPlayerStringPropertyInfo;
  strinfo -> setDefaultValue ("mplayer");
  m_info.insert ("Executable Path", strinfo);
  info = new KPlayerAppendablePropertyInfo;
  m_info.insert ("Command Line", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Frame Dropping", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Cache", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Build New Index", intinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Use KIOSlave", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Use Temporary File For KIOSlave", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("OSD Level", info);
  info = new KPlayerTranslatedStringPropertyInfo;
  info -> setCaption (i18n("Type"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_FORMAT);
  info -> setCanReset (false);
  m_info.insert ("Type", info);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Icon", info);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Channel List", info);
  strinfo = new KPlayerStringPropertyInfo;
  strinfo -> setDefaultValue ("v4l2");
  m_info.insert ("Input Driver", strinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Video Input", intinfo);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Video Format", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Video Norm", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Audio Mode", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Audio Input", intinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Immediate Mode", info);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("ALSA Capture", boolinfo);
  info = new KPlayerStringPropertyInfo;
  m_info.insert ("Capture Device", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Decimation", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Compression", intinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Tracks", info);
  info = new KPlayerFrequencyPropertyInfo;
  info -> setCaption (i18n("Frequency"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_GENERAL);
  m_info.insert ("Frequency", info);
  info = new KPlayerLengthPropertyInfo;
  info -> setCaption (i18n("Length"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_GENERAL);
  info -> setCanReset (false);
  m_info.insert ("Length", info);
  info = new KPlayerLengthPropertyInfo;
  m_info.insert ("MSF", info);
  info = new KPlayerSizePropertyInfo;
  info -> setCaption (i18n("Resolution"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_SIZE);
  info -> setCanReset (false);
  m_info.insert ("Resolution", info);
  info = new KPlayerSizePropertyInfo;
  info -> setCaption (i18n("Video size"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_SIZE);
  info -> setCanReset (false);
  m_info.insert ("Video Size", info);
  info = new KPlayerSizePropertyInfo;
  info -> setCanReset (false);
  m_info.insert ("Current Resolution", info);
  info = new KPlayerSizePropertyInfo;
  info -> setCanReset (false);
  m_info.insert ("Current Size", info);
  info = new KPlayerIntegerPropertyInfo;
  info -> setCaption (i18n("Video bit rate"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_VIDEO);
  info -> setCanReset (false);
  m_info.insert ("Video Bitrate", info);
  info = new KPlayerFloatPropertyInfo;
  info -> setCaption (i18n("Frame rate"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_VIDEO);
  info -> setCanReset (false);
  m_info.insert ("Framerate", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Video ID", intinfo);
  info = new KPlayerIntegerStringMapPropertyInfo (true);
  m_info.insert ("Video IDs", info);
  info = new KPlayerIntegerPropertyInfo;
  info -> setCaption (i18n("Audio bit rate"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_AUDIO);
  info -> setCanReset (false);
  m_info.insert ("Audio Bitrate", info);
  info = new KPlayerIntegerPropertyInfo;
  info -> setCaption (i18n("Sample rate"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_AUDIO);
  info -> setCanReset (false);
  m_info.insert ("Samplerate", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setCaption (i18n("Channels"));
  intinfo -> setGroup (KPLAYER_PROPERTY_GROUP_AUDIO);
  intinfo -> setCanReset (false);
  intinfo -> setDefaultValue (2);
  m_info.insert ("Channels", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Audio ID", intinfo);
  info = new KPlayerIntegerStringMapPropertyInfo (true);
  m_info.insert ("Audio IDs", info);
  info = new KPlayerIntegerPropertyInfo;
  info -> setCaption (i18n("Track"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_INFO);
  info -> setCanEdit (true);
  m_info.insert ("Track", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Playlist", info);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Has Video", boolinfo);
  info = new KPlayerPersistentUrlPropertyInfo;
  info -> setCaption (i18n("External Subtitles"));
  info -> setGroup (KPLAYER_PROPERTY_GROUP_LOCATION);
  m_info.insert ("Subtitle URL", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Vobsub", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Subtitle ID", intinfo);
  info = new KPlayerIntegerStringMapPropertyInfo (false);
  m_info.insert ("Subtitle IDs", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Vobsub ID", intinfo);
  info = new KPlayerIntegerStringMapPropertyInfo (false);
  m_info.insert ("Vobsub IDs", info);
  m_meta_info.setGroup (KPLAYER_PROPERTY_GROUP_INFO);
  m_meta_info.setCanEdit (true);
  m_meta_attributes << I18N_NOOP("Track") << I18N_NOOP("Title") << I18N_NOOP("Album")
    << I18N_NOOP("Artist") << I18N_NOOP("Year") << I18N_NOOP("Genre");
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Resize Main Window Automatically", boolinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (500);
  m_info.insert ("Minimum Initial Width", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Position X", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-1);
  m_info.insert ("Position Y", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (10);
  m_info.insert ("Recent File List Size", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Playlist Size Limit", intinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Playlist Loop", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Playlist Shuffle", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Allow Duplicate Entries", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (10);
  m_info.insert ("Playlist Menu Size", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (10000);
  m_info.insert ("Cache Size Limit", intinfo);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Remember With Shift", boolinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Size", info);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Remember Aspect", boolinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Full Screen", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Maximized", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Maintain Aspect", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Volume", info);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Remember Audio Delay", boolinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Frame Drop", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Contrast", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Brightness", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Hue", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Saturation", info);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Remember Subtitle URL", boolinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Remember Subtitle Position", info);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Remember Subtitle Delay", boolinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (200);
  m_info.insert ("Preferred Slider Length", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Minimum Slider Length", intinfo);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Show Slider Marks", boolinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (10);
  m_info.insert ("Slider Marks", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Normal Seek", intinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Normal Seek Units", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (10);
  m_info.insert ("Fast Seek", intinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Fast Seek Units", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Volume Minimum", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Volume Maximum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Volume Step", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (50);
  m_info.insert ("Volume Default", intinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Volume Reset", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Volume Every", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-100);
  m_info.insert ("Contrast Minimum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Contrast Maximum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Contrast Step", intinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Contrast Default", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Contrast Reset", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Contrast Every", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-50);
  m_info.insert ("Brightness Minimum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (50);
  m_info.insert ("Brightness Maximum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Brightness Step", intinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Brightness Default", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Brightness Reset", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Brightness Every", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-100);
  m_info.insert ("Hue Minimum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Hue Maximum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Hue Step", intinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Hue Default", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Hue Reset", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Hue Every", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (-100);
  m_info.insert ("Saturation Minimum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Saturation Maximum", intinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Saturation Step", intinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Saturation Default", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Saturation Reset", info);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Saturation Every", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Mute", info);
  KPlayerFloatPropertyInfo* floatinfo = new KPlayerFloatPropertyInfo;
  floatinfo -> setDefaultValue (0.1);
  m_info.insert ("Audio Delay Step", floatinfo);
  strinfo = new KPlayerStringPropertyInfo;
  strinfo -> setDefaultValue ("Sans");
  m_info.insert ("Subtitle Font Name", strinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Subtitle Font Bold", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Subtitle Font Italic", info);
  floatinfo = new KPlayerFloatPropertyInfo;
  floatinfo -> setDefaultValue (3);
  m_info.insert ("Subtitle Text Size", floatinfo);
  boolinfo = new KPlayerBooleanPropertyInfo;
  boolinfo -> setDefaultValue (true);
  m_info.insert ("Subtitle Autoscale", boolinfo);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (100);
  m_info.insert ("Subtitle Text Width", intinfo);
  floatinfo = new KPlayerFloatPropertyInfo;
  floatinfo -> setDefaultValue (0.75);
  m_info.insert ("Subtitle Font Outline", floatinfo);
  info = new KPlayerIntegerPropertyInfo;
  m_info.insert ("Subtitle Autoexpand", info);
  intinfo = new KPlayerIntegerPropertyInfo;
  intinfo -> setDefaultValue (1);
  m_info.insert ("Subtitle Position Step", intinfo);
  floatinfo = new KPlayerFloatPropertyInfo;
  floatinfo -> setDefaultValue (0.1);
  m_info.insert ("Subtitle Delay Step", floatinfo);
  strinfo = new KPlayerStringPropertyInfo;
  strinfo -> setDefaultValue ("aqt ass idx ifo js jss rt smi srt ssa sub txt utf utf8 utf-8");
  m_info.insert ("Autoload Extension List", strinfo);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Show Messages On Error", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Use KIOSlave For HTTP", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Use KIOSlave For FTP", info);
  info = new KPlayerBooleanPropertyInfo;
  m_info.insert ("Use KIOSlave For SMB", info);
  strinfo = new KPlayerStringPropertyInfo;
  strinfo -> setDefaultValue ("mpegps|mpegpes|mpeges|mpeg4es|h264es|mkv");
  m_info.insert ("Switch Audio Demuxers", strinfo);
}

void KPlayerProperties::terminate (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Terminating properties\n";
#endif
  KPlayerPropertyInfoMap::ConstIterator iterator (m_info.constBegin());
  while ( iterator != m_info.constEnd() )
  {
    delete iterator.value();
    ++ iterator;
  }
}

KPlayerConfiguration::KPlayerConfiguration (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating configuration\n";
#endif
}

KPlayerConfiguration::~KPlayerConfiguration()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying configuration\n";
#endif
}

KConfig* KPlayerConfiguration::config (void) const
{
  return KPlayerEngine::engine() -> config();
}

QString KPlayerConfiguration::configGroupName (void) const
{
  return "Player Options";
}

bool KPlayerConfiguration::getBoolean (const QString& key) const
{
  return has (key) ? ((KPlayerBooleanProperty*) m_properties [key]) -> value() : booleanInfo (key) -> defaultValue();
}

void KPlayerConfiguration::setBoolean (const QString& key, bool value)
{
  if ( value == (booleanInfo (key) -> defaultValue()) )
    reset (key);
  else
  {
    ((KPlayerBooleanProperty*) get (key)) -> setValue (value);
    updated (key);
  }
}

int KPlayerConfiguration::getInteger (const QString& key) const
{
  return has (key) ? ((KPlayerIntegerProperty*) m_properties [key]) -> value() : integerInfo (key) -> defaultValue();
}

void KPlayerConfiguration::setInteger (const QString& key, int value)
{
  if ( value == integerInfo (key) -> defaultValue() )
    reset (key);
  else
    set (key, value);
}

int KPlayerConfiguration::getRelativeValue (const QString& key) const
{
  return getInteger (key);
}

void KPlayerConfiguration::setRelative (const QString& key, int value)
{
  setInteger (key, value);
}

void KPlayerConfiguration::adjustRelative (const QString& key, int value)
{
  setInteger (key, value);
}

int KPlayerConfiguration::getCache (const QString& key) const
{
  int cache = has (key) ? ((KPlayerCacheProperty*) m_properties [key]) -> value() : integerInfo (key) -> defaultValue();
  return cache > 2 ? 2 : cache;
}

int KPlayerConfiguration::getCacheSize (const QString& key) const
{
  int size = has (key) ? ((KPlayerCacheProperty*) m_properties [key]) -> value() : integerInfo (key) -> defaultValue();
  return size > 3 ? size : 0;
}

float KPlayerConfiguration::getFloat (const QString& key) const
{
  return has (key) ? ((KPlayerFloatProperty*) m_properties [key]) -> value() : floatInfo (key) -> defaultValue();
}

void KPlayerConfiguration::setFloat (const QString& key, float value)
{
  if ( value == floatInfo (key) -> defaultValue() )
    reset (key);
  else
    set (key, value);
}

const QString& KPlayerConfiguration::getString (const QString& key) const
{
  return has (key) ? ((KPlayerStringProperty*) m_properties [key]) -> value() : stringInfo (key) -> defaultValue();
}

const QString& KPlayerConfiguration::getComboString (const QString& key) const
{
  return getString (key);
}

QString KPlayerConfiguration::getAppendable (const QString& key) const
{
  return getString (key);
}

bool KPlayerConfiguration::autoloadSubtitles (const QString& key) const
{
  return getBoolean (key);
}

bool vobsub (const QString& path);

bool KPlayerConfiguration::getVobsubSubtitles (const QString&, const KUrl& url) const
{
  return url.isValid() && url.isLocalFile() && vobsub (url.path());
}

bool KPlayerConfiguration::getPlaylist (const QString&, const KUrl& url) const
{
  static QRegExp re_playlist_url ("^(?:file|http|http_proxy|ftp|smb):/.*\\.(?:ram|smi|smil|rpm|asx|wax|wvx|pls|m3u|strm)(?:\\?|$)", Qt::CaseInsensitive);
  return re_playlist_url.indexIn (url.url()) >= 0;
}

bool KPlayerConfiguration::getUseKioslave (const QString&, const KUrl& url) const
{
  static QRegExp re_remote_url ("^[A-Za-z]+:/");
  static QRegExp re_mplayer_url ("^(?:file|http_proxy|mms|mmst|mmsu|rtp|rtsp|sip|pnm|dvd|vcd|tv|dvb|mf|cdda|cddb|cue|sdp|mpst|tivo):/", Qt::CaseInsensitive);
  static QRegExp re_http_url ("^http:/", Qt::CaseInsensitive);
  static QRegExp re_ftp_url ("^ftp:/", Qt::CaseInsensitive);
  static QRegExp re_smb_url ("^smb:/", Qt::CaseInsensitive);
  if ( re_http_url.indexIn (url.url()) >= 0 )
    return configuration() -> useKioslaveForHttp();
  if ( re_ftp_url.indexIn (url.url()) >= 0 )
    return configuration() -> useKioslaveForFtp();
  if ( re_smb_url.indexIn (url.url()) >= 0 )
    return configuration() -> useKioslaveForSmb();
  return re_remote_url.indexIn (url.url()) >= 0 && re_mplayer_url.indexIn (url.url()) < 0;
}

void KPlayerConfiguration::itemReset (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS
  kdDebugTime() << "Configuration::itemReset\n";
#endif
  if ( resetVolumeEveryFile() )
    setVolume (initialVolume());
  if ( resetContrastEveryFile() )
    setContrast (initialContrast());
  if ( resetBrightnessEveryFile() )
    setBrightness (initialBrightness());
  if ( resetHueEveryFile() )
    setHue (initialHue());
  if ( resetSaturationEveryFile() )
    setSaturation (initialSaturation());
  setSubtitleDelay (0);
  setAudioDelay (0);
  KPlayerPropertyInfoMap::ConstIterator iterator (m_info.constBegin());
  while ( iterator != m_info.constEnd() )
  {
    iterator.value() -> setOverride (false);
    ++ iterator;
  }
}

QStringList KPlayerConfiguration::subtitleExtensions (void) const
{
  static QRegExp re_split ("\\s*[,;:. ]\\s*");
  QStringList exts, extlist (autoloadExtensionList().split (re_split));
  QStringList::ConstIterator extiterator (extlist.constBegin());
  while ( extiterator != extlist.constEnd() )
  {
    if ( ! (*extiterator).isEmpty() )
      exts.append ('.' + *extiterator);
    ++ extiterator;
  }
  return exts;
}

QSize KPlayerConfiguration::autoexpandAspect (void) const
{
  int option = subtitleAutoexpand();
  return option == 1 ? QSize (1, 1) : option == 2 ? QSize (4, 3) : option == 3 ? QSize (16, 9) : QSize();
}

int KPlayerConfiguration::sliderMarksInterval (int span) const
{
  int interval = span * sliderMarks() / 100;
  return interval > 0 ? interval : 1;
}

KPlayerMedia::KPlayerMedia (KPlayerProperties* parent, const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating media properties\n";
  kdDebugTime() << " URL    " << url.url() << "\n";
#endif
  m_parent = parent;
  m_url = url;
  m_references = 1;
}

KPlayerMedia::~KPlayerMedia()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying media\n";
#endif
  if ( parent() != configuration() )
    KPlayerMedia::release ((KPlayerMedia*) parent());
}

void KPlayerMedia::setParent (KPlayerMedia* media)
{
  if ( media != parent() )
  {
    if ( parent() != configuration() )
      KPlayerMedia::release ((KPlayerMedia*) parent());
    m_parent = media;
    media -> reference();
    setupInfo();
  }
}

QString KPlayerMedia::configGroupName (void) const
{
  return url().url();
}

bool KPlayerMedia::release (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerMedia::release\n";
  kdDebugTime() << " URL    " << m_url.url() << "\n";
  kdDebugTime() << " References " << m_references << "\n";
#endif
  -- m_references;
  bool result = m_references == 0;
  if ( result )
    delete this;
  return result;
}

bool KPlayerMedia::getBoolean (const QString& key) const
{
  return has (key) ? ((KPlayerBooleanProperty*) m_properties [key]) -> value() : parent() -> getBoolean (key);
}

void KPlayerMedia::setBoolean (const QString& key, bool value)
{
  if ( value == parent() -> getBoolean (key) )
    reset (key);
  else
  {
    ((KPlayerBooleanProperty*) get (key)) -> setValue (value);
    updated (key);
  }
}

int KPlayerMedia::getInteger (const QString& key) const
{
  return has (key) ? ((KPlayerIntegerProperty*) m_properties [key]) -> value() : parent() -> getInteger (key);
}

void KPlayerMedia::setInteger (const QString& key, int value)
{
  if ( value == parent() -> getInteger (key) )
    reset (key);
  else
    set (key, value);
}

int KPlayerMedia::getRelativeValue (const QString& key) const
{
  int current = parent() -> getRelativeValue (key);
  return has (key) ? ((KPlayerRelativeProperty*) m_properties [key]) -> value (current) : current;
}

void KPlayerMedia::setRelative (const QString& key, int value)
{
  int current = parent() -> getRelativeValue (key);
  if ( value == current )
    reset (key);
  else
  {
    ((KPlayerRelativeProperty*) get (key)) -> setValue (value, current);
    updated (key);
  }
}

void KPlayerMedia::adjustRelative (const QString& key, int value)
{
  int option = getRelativeOption (key);
  if ( option == 1 )
  {
    configuration() -> setInteger (key, value);
    info (key) -> setOverride (true);
  }
  else
  {
    if ( option > 1 )
    {
      int current = getInteger (key);
      if ( option == 2 )
        value -= current;
      else
        value += current;
    }
    else
      reset (key);
    parent() -> adjustRelative (key, value);
  }
}

int KPlayerMedia::getCache (const QString& key) const
{
  if ( ! has (key) )
    return parent() -> getCache (key);
  int cache = ((KPlayerCacheProperty*) m_properties [key]) -> value();
  return cache > 2 ? 2 : cache;
}

int KPlayerMedia::getCacheSize (const QString& key) const
{
  if ( ! has (key) )
    return parent() -> getCacheSize (key);
  int size = ((KPlayerCacheProperty*) m_properties [key]) -> value();
  return size > 3 ? size : 0;
}

float KPlayerMedia::getFloat (const QString& key) const
{
  return has (key) ? ((KPlayerFloatProperty*) m_properties [key]) -> value() : parent() -> getFloat (key);
}

void KPlayerMedia::setFloat (const QString& key, float value)
{
  if ( value == parent() -> getFloat (key) )
    reset (key);
  else
    set (key, value);
}

const QString& KPlayerMedia::getString (const QString& key) const
{
  return has (key) ? ((KPlayerStringProperty*) m_properties [key]) -> value() : parent() -> getString (key);
}

const QString& KPlayerMedia::getComboString (const QString& key) const
{
  return has (key) ? ((KPlayerComboStringProperty*) m_properties [key]) -> option() : parent() -> getComboString (key);
}

QString KPlayerMedia::getAppendable (const QString& key) const
{
  QString current (parent() -> getAppendable (key));
  return has (key) ? ((KPlayerAppendableProperty*) m_properties [key]) -> appendableValue (current) : current;
}

KPlayerGenericProperties* KPlayerMedia::genericProperties (const KUrl& url)
{
  QString urls (url.url());
  KPlayerGenericProperties* properties = (KPlayerGenericProperties*) reference (urls);
  if ( ! properties )
  {
    properties = new KPlayerGenericProperties (KPlayerEngine::engine() -> configuration(), url);
    properties -> setup();
    m_media_map.insert (urls, properties);
  }
  return properties;
}

KPlayerDeviceProperties* KPlayerMedia::deviceProperties (const KUrl& url)
{
  QString urls (url.url());
  KPlayerDeviceProperties* properties = (KPlayerDeviceProperties*) reference (urls);
  if ( ! properties )
  {
    properties = new KPlayerDeviceProperties (KPlayerEngine::engine() -> configuration(), url);
    properties -> setup();
    m_media_map.insert (urls, properties);
  }
  return properties;
}

KPlayerTVProperties* KPlayerMedia::tvProperties (const KUrl& url)
{
  QString urls (url.url());
  KPlayerTVProperties* properties = (KPlayerTVProperties*) reference (urls);
  if ( ! properties )
  {
    properties = new KPlayerTVProperties (KPlayerEngine::engine() -> configuration(), url);
    properties -> setup();
    m_media_map.insert (urls, properties);
  }
  return properties;
}

KPlayerDVBProperties* KPlayerMedia::dvbProperties (const KUrl& url)
{
  QString urls (url.url());
  KPlayerDVBProperties* properties = (KPlayerDVBProperties*) reference (urls);
  if ( ! properties )
  {
    properties = new KPlayerDVBProperties (KPlayerEngine::engine() -> configuration(), url);
    properties -> setup();
    m_media_map.insert (urls, properties);
  }
  return properties;
}

KPlayerDiskProperties* KPlayerMedia::diskProperties (const KUrl& url)
{
  QString urls (url.url());
  KPlayerDiskProperties* properties = (KPlayerDiskProperties*) reference (urls);
  if ( ! properties )
  {
    properties = new KPlayerDiskProperties (KPlayerEngine::engine() -> configuration(), url);
    properties -> setup();
    m_media_map.insert (urls, properties);
  }
  return properties;
}

KPlayerDiskProperties* KPlayerMedia::diskProperties (KPlayerDeviceProperties* parent, const KUrl& url)
{
  QString urls (url.url());
  KPlayerDiskProperties* properties = (KPlayerDiskProperties*) reference (urls);
  if ( properties )
    properties -> setParent (parent);
  else
  {
    parent -> reference();
    properties = new KPlayerDiskProperties (parent, url);
    properties -> setup();
    m_media_map.insert (urls, properties);
  }
  return properties;
}

KPlayerTrackProperties* KPlayerMedia::trackProperties (const KUrl& url)
{
  QString urls (url.url());
  KPlayerTrackProperties* properties = (KPlayerTrackProperties*) reference (urls);
  if ( ! properties )
  {
    if ( urls.startsWith ("kplayer:") )
    {
      KUrl parenturl (url);
      parenturl.cd ("..");
      if ( urls.startsWith ("kplayer:/disks/") )
        properties = new KPlayerDiskTrackProperties (diskProperties (parenturl), url);
      else
      {
        KPlayerMediaMap::ConstIterator iterator = m_media_map.constFind (urls);
        if ( iterator == m_media_map.constEnd() ? urls.startsWith ("kplayer:/devices/dev/dvb/")
            : ((KPlayerDeviceProperties*) *iterator) -> type() == "DVB" )
          properties = new KPlayerDVBChannelProperties (dvbProperties (parenturl), url);
        else
          properties = new KPlayerTVChannelProperties (tvProperties (parenturl), url);
      }
    }
    else
      properties = new KPlayerItemProperties (KPlayerEngine::engine() -> configuration(), url);
    properties -> setup();
    m_media_map.insert (urls, properties);
  }
  return properties;
}

KPlayerMedia* KPlayerMedia::reference (const QString& urls)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerMedia::reference\n";
  kdDebugTime() << " URL    " << urls << "\n";
#endif
  KPlayerMedia* media = 0;
  KPlayerMediaMap::ConstIterator iterator = m_media_map.constFind (urls);
  if ( iterator != m_media_map.constEnd() )
  {
    media = *iterator;
    media -> reference();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " References " << media -> references() << "\n";
#endif
  }
  return media;
}

void KPlayerMedia::release (KPlayerMedia* media)
{
  QString urls (media -> url().url());
  if ( media -> release() )
    m_media_map.remove (urls);
}

KPlayerGenericProperties::KPlayerGenericProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerMedia (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating generic properties\n";
#endif
}

KPlayerGenericProperties::~KPlayerGenericProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying generic properties\n";
#endif
}

KConfig* KPlayerGenericProperties::config (void) const
{
  return KPlayerEngine::engine() -> store();
}

QString KPlayerGenericProperties::defaultName (void) const
{
  return ! m_default_name.isEmpty() ? m_default_name
    : url().fileName().isEmpty() ? url().prettyUrl() : url().fileName();
}

QString KPlayerGenericProperties::type (const QString& id) const
{
  KUrl u (url());
  u.addPath (id);
  QString urls (u.url());
  KPlayerMediaMap::ConstIterator iterator = m_media_map.constFind (urls);
  if ( iterator != m_media_map.constEnd() )
    return ((KPlayerMediaProperties*) *iterator) -> type();
  return config() -> group (urls).readEntry ("Type");
}

float KPlayerGenericProperties::msf (const QString& id) const
{
  KUrl u (url());
  u.addPath (id);
  return config() -> group (u.url()).readEntry ("MSF", 0.0);
}

bool KPlayerGenericProperties::hidden (const QString& id) const
{
  KUrl u (url());
  u.addPath (id);
  return config() -> group (u.url()).readEntry ("Hidden", false);
}

void KPlayerGenericProperties::setHidden (const QString& id, bool hidden)
{
  KUrl u (url());
  u.addPath (id);
  return config() -> group (u.url()).writeEntry ("Hidden", hidden);
}

QString KPlayerGenericProperties::caption (void) const
{
  QString n (currentName());
  if ( n.isEmpty() && url().isLocalFile() )
    n = url().path();
  if ( n.isEmpty() )
    n = url().pathOrUrl();
  return n;
}

QString KPlayerGenericProperties::icon (void) const
{
  return getString ("Icon");
}

KPlayerMediaProperties::KPlayerMediaProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerGenericProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating media properties\n";
#endif
}

KPlayerMediaProperties::~KPlayerMediaProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying media properties\n";
#endif
}

void KPlayerMediaProperties::setDisplaySize (const QSize& size, int option)
{
  setSize ("Display Size", size, option);
}

QString KPlayerMediaProperties::demuxerString (void) const
{
  return demuxerOption();
  //if ( ! demuxer.isEmpty() )
  //    demuxer += ",";
}

QString KPlayerMediaProperties::audioDriverString (void) const
{
  QString driver (audioDriver());
  if ( ! driver.isEmpty() )
  {
    QString device (audioDevice());
    if ( ! device.isEmpty() )
    {
      device.replace (',', '.');
      device.replace (':', '=');
      if ( driver != "oss" )
        device = "device=" + device;
      driver += ":" + device;
    }
    driver += ",";
  }
  return driver;
}

QString KPlayerMediaProperties::mixerChannelString (void) const
{
  static QRegExp re_mixer_channel ("^(.*) +([0-9]+)$");
  QString channel (mixerChannel());
  if ( re_mixer_channel.indexIn (channel) >= 0 )
    channel = re_mixer_channel.cap(1) + "," + re_mixer_channel.cap(2);
  return channel;
}

QString KPlayerMediaProperties::audioCodecString (void) const
{
  QString codec (audioCodecOption());
  if ( ! codec.isEmpty() )
      codec += ",";
  return codec;
}

QString KPlayerMediaProperties::videoDriverString (void) const
{
  QString driver (videoDriver());
  if ( ! driver.isEmpty() )
  {
    QString device (videoDevice());
    if ( ! device.isEmpty() )
    {
      device.replace (',', '.');
      device.replace (':', '=');
      if ( driver != "oss" )
        device = "device=" + device;
      driver += ":" + device;
    }
    driver += ",";
  }
  return driver;
}

QString KPlayerMediaProperties::videoCodecString (void) const
{
  QString codec (videoCodecOption());
  if ( ! codec.isEmpty() )
      codec += ",";
  return codec;
}

KPlayerDeviceProperties::KPlayerDeviceProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerMediaProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating device properties\n";
#endif
  m_digits = 0;
}

KPlayerDeviceProperties::~KPlayerDeviceProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying device properties\n";
#endif
}

void KPlayerDeviceProperties::setupInfo (void)
{
  KPlayerMediaProperties::setupInfo();
  setPath ("/" + m_url.path().section ('/', 1, 0xffffffff, QString::SectionSkipEmpty));
}

KPlayerTunerProperties::KPlayerTunerProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerDeviceProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating TV/DVB properties\n";
#endif
}

KPlayerTunerProperties::~KPlayerTunerProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying TV/DVB properties\n";
#endif
}

int KPlayerTunerProperties::channelFrequency (const QString& id) const
{
  QMap<QString, int>::ConstIterator iterator = m_frequencies.find (id);
  if ( iterator == m_frequencies.constEnd() )
  {
    ((KPlayerTunerProperties*) this) -> channels();
    iterator = m_frequencies.find (id);
  }
  return iterator == m_frequencies.constEnd() ? 0 : iterator.value();
}

static struct KPlayerChannelGroup
  us_bcast[] = {
    { 2, 3, 0, 0, 55250, 6000 },
    { 5, 2, 0, 0, 77250, 6000 },
    { 7, 7, 0, 0, 175250, 6000 },
    { 14, 70, 0, 0, 471250, 6000 }
  },
  us_cable[] = {
    { 1, 1, 0, 0, 73250, 0 },
    { 2, 3, 0, 0, 55250, 6000 },
    { 5, 2, 0, 0, 77250, 6000 },
    { 7, 7, 0, 0, 175250, 6000 },
    { 14, 9, 0, 0, 121250, 6000 },
    { 23, 72, 0, 0, 217250, 6000 },
    { 95, 5, 0, 0, 91250, 6000 },
    { 100, 26, 0, 0, 649250, 6000 },
    { 7, 8, 0, "T", 8250, 6000 }
  },
  us_hrc[] = {
    { 1, 1, 0, 0, 72000, 0 },
    { 2, 3, 0, 0, 54000, 6000 },
    { 5, 2, 0, 0, 78000, 6000 },
    { 7, 7, 0, 0, 174000, 6000 },
    { 14, 9, 0, 0, 120000, 6000 },
    { 23, 72, 0, 0, 216000, 6000 },
    { 95, 2, 0, 0, 900000, 60000 },
    { 97, 3, 0, 0, 102000, 6000 },
    { 100, 26, 0, 0, 648000, 6000 },
    { 7, 8, 0, "T", 7000, 6000 }
  },
  japan_bcast[] = {
    { 1, 3, 0, 0, 91250, 6000 },
    { 4, 9, 0, 0, 171250, 6000 },
    { 13, 50, 0, 0, 471250, 6000 }
  },
  japan_cable[] = {
    { 13, 9, 0, 0, 109250, 6000 },
    { 22, 2, 0, 0, 165250, 58000 },
    { 24, 4, 0, 0, 231250, 6000 },
    { 28, 36, 0, 0, 253250, 6000 }
  },
  australia[] = {
    { 0, 2, 0, 0, 46250, 11000 },
    { 2, 2, 0, 0, 64250, 22000 },
    { 4, 2, 0, 0, 95250, 7000 },
    { -1, 1, 0, "5A", 138250, 0 },
    { 6, 4, 0, 0, 175250, 7000 },
    { 10, 2, 0, 0, 209250, 7000 },
    { 28, 8, 0, 0, 527250, 7000 },
    { 36, 1, 0, 0, 591250, 0 },
    { 39, 31, 0, 0, 604250, 7000 }
  },
  europe_west[] = {
    { 2, 3, 0, "E", 48250, 7000 },
    { 1, 3, 2, "S", 69250, 7000 },
    { 5, 8, 0, "E", 175250, 7000 },
    { 1, 10, 0, "SE", 105250, 7000 },
    { 11, 10, 0, "SE", 231250, 7000 },
    { 21, 21, 0, "S", 303250, 8000 },
    { 21, 49, 0, 0, 471250, 8000 }
  },
  europe_east[] = {
    { 1, 2, 0, "R", 49750, 9500 },
    { 3, 3, 0, "R", 77250, 8000 },
    { 6, 7, 0, "R", 175250, 8000 },
    { 1, 8, 0, "SR", 111250, 8000 },
    { 11, 9, 0, "SR", 231250, 8000 },
    { 2, 3, 0, "E", 48250, 7000 },
    { 1, 3, 2, "S", 69250, 7000 },
    { 5, 8, 0, "E", 175250, 7000 },
    { 1, 10, 0, "SE", 105250, 7000 },
    { 11, 10, 0, "SE", 231250, 7000 },
    { 21, 21, 0, "S", 303250, 8000 },
    { 21, 49, 0, 0, 471250, 8000 }
  },
  italy[] = {
    { -1, 1, 0, "A", 53750, 0 },
    { -1, 2, 0, "B", 62250, 20000 },
    { -1, 2, 0, "D", 175250, 8500 },
    { -1, 3, 0, "F", 192250, 9000 },
    { 1, 2, 0, "H", 217250, 7000 },
    { 21, 49, 0, 0, 471250, 8000 }
  },
  ireland[] = {
    { 0, 2, 0, "A", 45750, 2250 },
    { 2, 2, 0, "A", 53750, 2250 },
    { 4, 2, 0, "A", 61750, 2250 },
    { 6, 2, 0, "A", 175250, 750 },
    { 8, 2, 0, "A", 183250, 750 },
    { 10, 2, 0, "A", 191250, 750 },
    { 12, 2, 0, "A", 199250, 750 },
    { 14, 2, 0, "A", 207250, 750 },
    { 16, 2, 0, "A", 215250, 750 },
    { 18, 2, 0, "A", 224000, 8000 },
    { 20, 10, 0, "A", 248000, 8000 },
    { 30, 2, 0, "A", 344000, 8000 },
    { 32, 2, 0, "A", 408000, 8000 },
    { 34, 2, 0, "A", 448000, 32000 },
    { 36, 1, 0, "A", 520000, 0 },
    { 21, 49, 0, 0, 471250, 8000 }
  },
  france[] = {
    { 1, 2, 2, "K", 47750, 8000 },
    { 3, 2, 2, "K", 60500, 3250 },
    { 5, 6, 2, "K", 176000, 8000 },
    { -1, 3, 0, "KB", 116750, 12000 },
    { -1, 1, 0, "KE", 159750, 0 },
    { -1, 12, 0, "KF", 164750, 12000 },
    { 1, 19, 2, "H", 303250, 8000 },
    { 21, 49, 0, 0, 471250, 8000 }
  },
  newzealand[] = {
    { 1, 1, 0, 0, 45250, 0 },
    { 2, 2, 0, 0, 55250, 7000 },
    { 4, 8, 0, 0, 175250, 7000 },
    { 21, 49, 0, 0, 471250, 8000 }
  },
  china[] = {
    { 1, 3, 0, 0, 49750, 8000 },
    { 4, 2, 0, 0, 77250, 8000 },
    { 6, 44, 0, 0, 112250, 8000 },
    { 50, 45, 0, 0, 463250, 8000 }
  },
  southafrica[] = {
    { 1, 8, 0, 0, 175250, 8000 },
    { 21, 49, 0, 0, 471250, 8000 }
  },
  argentina[] = {
    { 1, 3, 3, 0, 56250, 6000 },
    { 4, 2, 3, 0, 78250, 6000 },
    { 6, 7, 3, 0, 176250, 6000 },
    { 13, 9, 3, 0, 122250, 6000 },
    { 22, 72, 3, 0, 218250, 6000 }
  },
  russia[] = {
    { 1, 2, 0, 0, 49750, 9500 },
    { 3, 3, 0, 0, 77250, 8000 },
    { 1, 8, 0, "SK", 111250, 8000 },
    { 6, 7, 0, 0, 175250, 8000 },
    { 11, 8, 0, "SK", 231250, 8000 },
    { 19, 22, 0, "S", 295250, 8000 },
    { 21, 49, 0, 0, 471250, 8000 }
  };

struct KPlayerChannelList channellists[] = {
  { "us-bcast", I18N_NOOP("US broadcast"), us_bcast, sizeof (us_bcast) / sizeof (struct KPlayerChannelGroup) },
  { "us-cable", I18N_NOOP("US cable"), us_cable, sizeof (us_cable) / sizeof (struct KPlayerChannelGroup) },
  { "us-cable-hrc", I18N_NOOP("US cable HRC"), us_hrc, sizeof (us_hrc) / sizeof (struct KPlayerChannelGroup) },
  { "japan-bcast", I18N_NOOP("Japan broadcast"), japan_bcast, sizeof (japan_bcast) / sizeof (struct KPlayerChannelGroup) },
  { "japan-cable", I18N_NOOP("Japan cable"), japan_cable, sizeof (japan_cable) / sizeof (struct KPlayerChannelGroup) },
  { "europe-west", I18N_NOOP("Western Europe"), europe_west, sizeof (europe_west) / sizeof (struct KPlayerChannelGroup) },
  { "europe-east", I18N_NOOP("Eastern Europe"), europe_east, sizeof (europe_east) / sizeof (struct KPlayerChannelGroup) },
  { "italy", I18N_NOOP("Italy"), italy, sizeof (italy) / sizeof (struct KPlayerChannelGroup) },
  { "newzealand", I18N_NOOP("New Zealand"), newzealand, sizeof (newzealand) / sizeof (struct KPlayerChannelGroup) },
  { "australia", I18N_NOOP("Australia"), australia, sizeof (australia) / sizeof (struct KPlayerChannelGroup) },
  { "ireland", I18N_NOOP("Ireland"), ireland, sizeof (ireland) / sizeof (struct KPlayerChannelGroup) },
  { "france", I18N_NOOP("France"), france, sizeof (france) / sizeof (struct KPlayerChannelGroup) },
  { "china-bcast", I18N_NOOP("China"), china, sizeof (china) / sizeof (struct KPlayerChannelGroup) },
  { "southafrica", I18N_NOOP("South Africa"), southafrica, sizeof (southafrica) / sizeof (struct KPlayerChannelGroup) },
  { "argentina", I18N_NOOP("Argentina"), argentina, sizeof (argentina) / sizeof (struct KPlayerChannelGroup) },
  { "russia", I18N_NOOP("Russia"), russia, sizeof (russia) / sizeof (struct KPlayerChannelGroup) }
};

const uint channellistcount = sizeof (channellists) / sizeof (struct KPlayerChannelList);

KPlayerTVProperties::KPlayerTVProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerTunerProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating TV properties\n";
#endif
}

KPlayerTVProperties::~KPlayerTVProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying TV properties\n";
#endif
}

QString KPlayerTVProperties::channelListFromCountry (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerTVProperties::channelListFromCountry\n";
#endif
  QString country (KGlobal::locale() -> country().toLower());
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << " Country " << country << "\n";
#endif
  return country == "us" ? "us-bcast" : country == "jp" ? "japan-bcast" : country == "it" ? "italy"
    : country == "nz" ? "newzealand" : country == "au" ? "australia" : country == "ie" ? "ireland"
    : country == "fr" ? "france" : country == "cn" ? "china-bcast" : country == "za" ? "southafrica"
    : country == "ar" ? "argentina" : country == "ru" ? "russia" : country == "by" || country == "bg"
    || country == "cz" || country == "hu" || country == "pl" || country == "md" || country == "ro"
    || country == "sk" || country == "ua" || country == "al" || country == "ba" || country == "hr"
    || country == "mk" || country == "yu" || country == "me" || country == "rs" || country == "si"
    || country == "ee" || country == "lv" || country == "lt" || country == "am" || country == "az"
    || country == "ge" ? "europe-east" : "europe-west";
}

void KPlayerTVProperties::setupMeta (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerTVProperties::setupMeta\n";
#endif
  if ( ! hasChannelList() )
    setChannelList (channelListFromCountry());
}

QStringList KPlayerTVProperties::channels (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerTVProperties::channels\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  m_frequencies.clear();
  QString id;
  QStringList channels;
  const QString& channellist (channelList());
  for ( uint i = 0; i < channellistcount; i ++ )
  {
    const struct KPlayerChannelList& list = channellists[i];
    if ( list.id == channellist )
    {
      uint groups = list.groupcount;
      for ( uint g = 0; g < groups; g ++ )
      {
        const struct KPlayerChannelGroup& group = list.groups[g];
        for ( uint c = 0; c < group.channels; c ++ )
        {
          if ( group.first_channel >= 0 )
          {
            id = QString::number (c + group.first_channel).rightJustified (group.digits, '0');
            if ( group.prefix )
              id = group.prefix + id;
          }
          else
          {
            id = group.prefix;
            QCharRef lastchr (id [id.length() - 1]);
            lastchr = lastchr.unicode() + c;
          }
          channels.append (id);
          m_frequencies.insert (id, group.first_freq + group.freq_step * c);
        }
      }
      m_digits = i > 2 || i == 0 ? 2 : 3;
      break;
    }
  }
  return channels;
}

KPlayerDVBProperties::KPlayerDVBProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerTunerProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating DVB properties\n";
#endif
}

KPlayerDVBProperties::~KPlayerDVBProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying DVB properties\n";
#endif
}

void KPlayerDVBProperties::setupMeta (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerDVBProperties::setupMeta\n";
#endif
  if ( ! hasChannelList() )
  {
    QString paths[] = { QDir::homePath() + "/.mplayer", "/etc/mplayer", "/usr/local/etc/mplayer" };
    const char* globs[] = { "channels.conf", "channels.conf.sat", "channels.conf.ter",
      "channels.conf.cbl", "channels.conf.atsc", "channels.conf.*", "channels.conf*", "*channels.conf" };
    for ( uint i = 0; i < sizeof (paths) / sizeof (QString); i ++ )
    {
#ifdef DEBUG_KPLAYER_PROPERTIES
      kdDebugTime() << " Looking for channels file in " << paths[i] << "\n";
#endif
      for ( uint j = 0; j < sizeof (globs) / sizeof (const char*); j ++ )
      {
#ifdef DEBUG_KPLAYER_PROPERTIES
        kdDebugTime() << " Checking " << globs[j] << "\n";
#endif
        QDir dir (paths[i], globs[j], QDir::Unsorted, QDir::Files);
        QFileInfoList list (dir.entryInfoList());
        QFileInfoList::ConstIterator it (list.constBegin());
        while ( it != list.constEnd() )
        {
          const QFileInfo& info = *it;
          QString path (info.filePath());
#ifdef DEBUG_KPLAYER_PROPERTIES
          kdDebugTime() << " Verifying " << path << "\n";
#endif
          if ( info.exists() && ! info.isDir() && info.isReadable() && info.size() < 1024 * 1024 )
          {
#ifdef DEBUG_KPLAYER_PROPERTIES
            kdDebugTime() << " Found channels file " << path << "\n";
#endif
            setChannelList (path);
            return;
          }
          ++ it;
        }
      }
    }
  }
}

QStringList KPlayerDVBProperties::channels (void)
{
  static QRegExp re_channel ("^([^#:][^:]*):(\\d+):");
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerDVBProperties::channels\n";
  kdDebugTime() << " URL    " << url().url() << "\n";
#endif
  m_names.clear();
  m_frequencies.clear();
  QStringList channels;
  if ( hasChannelList() )
  {
    QByteArray ba;
    ba.resize (1024);
    QFile file (channelList());
    file.open (QIODevice::ReadOnly);
    while ( file.readLine (ba.data(), 1024) >= 0 )
    {
      QString id (ba);
#ifdef DEBUG_KPLAYER_SOURCE
      kdDebugTime() << " Line   " << id << "\n";
#endif
      if ( re_channel.indexIn (id) >= 0 )
      {
        QString name (re_channel.cap(1));
        int frequency = re_channel.cap(2).toInt();
        if ( frequency > 20000000 )
          frequency /= 1000;
        QString base (name);
        base = base.replace ('/', '-').simplified();
        id = base;
        int i = 0;
        while ( m_names.contains (id) )
          id = base + QString::number (i ++);
        channels.append (id);
        m_names.insert (id, name);
        m_frequencies.insert (id, frequency);
      }
    }
  }
  return channels;
}

KPlayerDiskProperties::KPlayerDiskProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerDeviceProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating disk properties\n";
#endif
}

KPlayerDiskProperties::~KPlayerDiskProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying disk properties\n";
#endif
}

void KPlayerDiskProperties::setupInfo (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerDiskProperties::setupInfo\n";
#endif
  KPlayerDeviceProperties::setupInfo();
  if ( parent() != configuration() )
    setPath (((KPlayerDeviceProperties*) parent()) -> path());
}

void KPlayerDiskProperties::setupMeta (void)
{
  int t = tracks();
  for ( m_digits = 1; t > 9; ++ m_digits )
    t /= 10;
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerDiskProperties::setupMeta\n";
  kdDebugTime() << " Digits " << digits() << "\n";
#endif
}

KConfig* KPlayerDiskProperties::config (void) const
{
  return KPlayerEngine::engine() -> meta();
}

KPlayerTrackProperties::KPlayerTrackProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerMediaProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating track properties\n";
#endif
}

KPlayerTrackProperties::~KPlayerTrackProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying track properties\n";
#endif
}

void KPlayerTrackProperties::setDisplaySize (const QSize& size, int option)
{
  if ( ! size.isEmpty() && hasOriginalSize() && (option == 1 && currentSize() == size
      || ( option == 2 && size.width() * currentSize().height() == size.height() * currentSize().width()) ) )
    resetDisplaySize();
  else
    KPlayerMediaProperties::setDisplaySize (size, option);
}

KConfig* KPlayerTrackProperties::config (void) const
{
  return KPlayerEngine::engine() -> meta();
}

QString KPlayerTrackProperties::deviceOption (void) const
{
  return QString();
}

QString KPlayerTrackProperties::deviceSetting (void) const
{
  return QString();
}

QString KPlayerTrackProperties::extension (void) const
{
  return QFileInfo (url().fileName()).suffix().toLower();
}

void KPlayerTrackProperties::setupMeta (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerTrackProperties::setupMeta\n";
#endif
  m_height_adjusted = hasOriginalSize();
}

void KPlayerTrackProperties::importMeta (QString key, QString value)
{
  static QRegExp re_track ("^(?:Track|Pista) *[0-9]*$", Qt::CaseInsensitive);
  key = key.toLower();
  capitalizeWords (key);
  if ( key == "Layer" || key == "Version" || key == "Comment" || key == "Comments"
      || key == "Copyright" || key == "Software" || key == "Encoder" || key == "Aspect Ratio" )
    return;
  if ( key == "Date" )
    key = "Year";
  else if ( key == "Tracknumber" || key == "Track Number" )
    key = "Track";
  else if ( key.right (5) == " Rate" )
    key = key.left (key.length() - 5) + "rate";
  if ( key == "Name" ? hasName() : has (key) )
    return;
  value = value.simplified();
  if ( value.isEmpty() )
    return;
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << " Extracted " << key << " " << value << "\n";
#endif
  if ( key == "Length" )
  {
    if ( ! hasLength() )
      setLength (value.toFloat());
  }
  else if ( key == "Bitrate" )
  {
    if ( ! hasAudioBitrate() )
      setAudioBitrate (value.toInt());
  }
  else if ( key == "Framerate" )
  {
    if ( ! hasFramerate() )
    {
      float fr = value.toFloat();
      setFramerate (fr == 23 ? 23.976 : fr == 29 ? 29.97 : fr == 59 ? 59.94 : fr == 14 ? 14.995 : fr);
    }
  }
  else if ( key == "Samplerate" )
  {
    if ( ! hasSamplerate() )
      setSamplerate (value.toInt());
  }
  else if ( key == "Channels" )
  {
    if ( ! hasChannels() )
      setChannels (value.toInt());
  }
  else if ( key == "Track" )
  {
    if ( ! hasTrack() )
      setTrack (value.toInt());
  }
  else
  {
    QString lovalue (value.toLower());
    if ( lovalue != "no title" && lovalue != "title" && lovalue != "artist"
        && lovalue != "album" && lovalue != "genre" && lovalue != "various artists"
        && lovalue != "various" && lovalue != "unknown" && lovalue != "other"
        && lovalue != "default" && lovalue != "misc" && lovalue != "unknown"
        && lovalue != "general unclassifiable" && (key != "Title" || re_track.indexIn (value) < 0) )
    {
      if ( value == value.toUpper() )
        value = lovalue;
      capitalize (value);
      setString (key, value);
    }
  }
}

void KPlayerTrackProperties::extractMeta (const QString& str, bool update)
{
  static QRegExp re_video ("^V(?:IDE)?O: *\\S+ +(\\d+)x(\\d+)");
  static QRegExp re_vo ("^V(?:IDE)?O:.* (\\d+)x(\\d+) +=> +(\\d+)x(\\d+)");
  static QRegExp re_vc ("^(?:ID_VIDEO_CODEC=|Selected video codec: \\[)([A-Za-z0-9,:.-]+)(?:$|\\])");
  static QRegExp re_ac ("^(?:ID_AUDIO_CODEC=|Selected audio codec: \\[)([A-Za-z0-9,:.-]+)(?:$|\\])");
#ifdef KPLAYER_PROCESS_SIZE_IDS
  static QRegExp re_vw ("^ID_VIDEO_WIDTH=(\\d+)$");
  static QRegExp re_vh ("^ID_VIDEO_HEIGHT=(\\d+)$");
  static QRegExp re_va ("^ID_VIDEO_ASPECT=([0-9.]+)$");
#endif
  static QRegExp re_vbr ("^ID_VIDEO_BITRATE=(\\d+)$");
  static QRegExp re_vfr ("^ID_VIDEO_FPS=([0-9.]+)$");
  static QRegExp re_vid ("^ID_VIDEO_ID=(\\d+)$");
  static QRegExp re_abr ("^ID_AUDIO_BITRATE=(\\d+)$");
  static QRegExp re_asr ("^ID_AUDIO_RATE=(\\d+)$");
  static QRegExp re_ach ("^ID_AUDIO_NCH=(\\d+)$");
  static QRegExp re_aid ("^ID_AUDIO_ID=(\\d+)$");
  static QRegExp re_alang ("^ID_AID_(\\d+)_LANG=(.+)$");
  static QRegExp re_sid ("^ID_SUBTITLE_ID=(\\d+)$");
  static QRegExp re_slang ("^ID_SID_(\\d+)_LANG=(.+)$");
  static QRegExp re_vsid ("^ID_VOBSUB_ID=(\\d+)$");
  static QRegExp re_vslang ("^ID_VSID_(\\d+)_LANG=(.+)$");
  static QRegExp re_length ("^(?:ID|ANS)_LENGTH=(\\d+\\.?\\d*)$");
  static QRegExp re_demuxer ("^ID_DEMUXER=(.+)$");
  static QRegExp re_name_sc ("^Name *: *(.+)$");
  static QRegExp re_name ("^ID_CLIP_INFO_NAME[0-9]+=(.+)$");
  static QRegExp re_value ("^ID_CLIP_INFO_VALUE[0-9]+=(.+)$");
  static QRegExp re_icyinfo ("^ICY Info: StreamTitle='([^']*)'");
  static int width = 0;
  static QString key;
  static bool seen_length = false;
  if ( str.startsWith ("ID_FILENAME=") )
    seen_length = false;
  else if ( str.startsWith ("ID_LENGTH=") )
    seen_length = true;
  if ( re_name.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Clip info name: " << re_name.cap (1) << "\n";
#endif
    key = re_name.cap (1);
  }
  else if ( re_value.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Clip info value: " << re_value.cap (1) << "\n";
#endif
    importMeta (key, re_value.cap (1));
  }
  else if ( ! hasLength() && re_length.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Length: " << re_length.cap(1) << "\n";
#endif
    setLength (re_length.cap(1).toFloat());
    if ( ! hasLength() && has ("MSF") && hasVideoBitrate() && hasAudioBitrate() )
      setLength (getFloat ("MSF") * 1388 / (videoBitrate() + audioBitrate()));
  }
  else if ( (update || ! heightAdjusted()) && re_vo.indexIn (str) >= 0 )
  {
    QSize res (re_vo.cap(1).toInt(), re_vo.cap(2).toInt());
    QSize size (re_vo.cap(3).toInt(), re_vo.cap(4).toInt());
    if ( res == resolution() )
    {
      setOriginalSize (size);
#ifdef DEBUG_KPLAYER_PROPERTIES
      kdDebugTime() << "Process: Adjusted width " << originalSize().width() << " height " << originalSize().height() << "\n";
#endif
      resetCurrentResolution();
      resetCurrentSize();
    }
    else
    {
      setCurrentResolution (res);
      setCurrentSize (size);
#ifdef DEBUG_KPLAYER_PROPERTIES
      kdDebugTime() << "Process: Current width " << currentSize().width() << " height " << currentSize().height() << "\n";
#endif
    }
    m_height_adjusted = update;
  }
  else if ( (update || ! hasResolution()) && re_video.indexIn (str) >= 0 )
  {
    setResolution (QSize (re_video.cap(1).toInt(), re_video.cap(2).toInt()));
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Width " << resolution().width() << " Height " << resolution().height() << "\n";
#endif
  }
#ifdef KPLAYER_PROCESS_SIZE_IDS
  else if ( resolution().width() <= 0 && re_vw.indexIn (str) >= 0 )
  {
    width = re_vw.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Width " << width << "\n";
#endif
  }
  else if ( width > 0 && resolution().height() <= 0 && re_vh.indexIn (str) >= 0 )
  {
    setResolution (QSize (width, re_vh.cap(1).toInt()));
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Height " << resolution().height() << "\n";
#endif
  }
  else if ( (update || ! heightAdjusted()) && re_va.indexIn (str) >= 0 )
  {
    int w = 0, h = 0;
    float a = stringToFloat (re_va.cap(1)), b;
    if ( a > 0 )
    {
      for ( h = 1; h <= 20; h ++ )
      {
        b = a * h;
        w = int (b + 0.5);
        b -= w;
        if ( b < 0.001 && b > -0.001 )
          break;
      }
      if ( h > 20 )
      {
        h = 10000;
        w = int (a * h + 0.5);
      }
#ifdef DEBUG_KPLAYER_PROPERTIES
      kdDebugTime() << "Process: Aspect " << a << " (" << w << "x" << h << ")\n";
#endif
      setOriginalSize (QSize (resolution().width(), (resolution().width() * h + w / 2) / w));
#ifdef DEBUG_KPLAYER_PROPERTIES
      kdDebugTime() << "Process: Adjusted height " << originalSize().height() << "\n";
#endif
      m_height_adjusted = false;
    }
  }
#endif
  else if ( (update || ! hasVideoBitrate()) && re_vbr.indexIn (str) >= 0 )
  {
    int br = re_vbr.cap(1).toInt();
    if ( br )
      setVideoBitrate ((br + 500) / 1000);
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Video bitrate " << videoBitrate() << "\n";
#endif
    if ( seen_length && ! hasLength() && has ("MSF") && hasVideoBitrate() && hasAudioBitrate() )
      setLength (getFloat ("MSF") * 1388 / (videoBitrate() + audioBitrate()));
  }
  else if ( (update || ! hasFramerate()) && re_vfr.indexIn (str) >= 0 )
  {
    float fr = stringToFloat (re_vfr.cap(1));
    if ( fr )
      setFramerate (fr);
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Framerate " << framerate() << "\n";
#endif
  }
  else if ( re_vid.indexIn (str) >= 0 )
  {
    int vid = re_vid.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Video ID " << vid << "\n";
#endif
    if ( ! hasVideoID (vid) )
      addVideoID (vid);
  }
  else if ( (update || ! hasAudioBitrate()) && re_abr.indexIn (str) >= 0 )
  {
    int br = re_abr.cap(1).toInt();
    if ( br )
      setAudioBitrate ((br + 500) / 1000);
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Audio bitrate " << audioBitrate() << "\n";
#endif
    if ( seen_length && ! hasLength() && has ("MSF") && hasVideoBitrate() && hasAudioBitrate() )
      setLength (getFloat ("MSF") * 1388 / (videoBitrate() + audioBitrate()));
  }
  else if ( (update || ! hasSamplerate()) && re_asr.indexIn (str) >= 0 )
  {
    int sr = re_asr.cap(1).toInt();
    if ( sr )
      setSamplerate (sr);
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Audio sample rate " << samplerate() << "\n";
#endif
  }
  else if ( (update || ! hasChannels()) && re_ach.indexIn (str) >= 0 )
  {
    int ch = re_ach.cap(1).toInt();
    if ( ch )
      setChannels (ch);
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Audio channels " << channels() << "\n";
#endif
  }
  else if ( re_aid.indexIn (str) >= 0 )
  {
    int aid = re_aid.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Audio ID " << aid << "\n";
#endif
    if ( ! hasAudioID (aid) )
      addAudioID (aid);
  }
  else if ( re_alang.indexIn (str) >= 0 )
  {
    int aid = re_alang.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Audio ID " << aid << " language " << re_alang.cap(2) << "\n";
#endif
    if ( update || ! hasAudioLanguage (aid) )
      setAudioLanguage (aid, re_alang.cap(2));
  }
  else if ( re_sid.indexIn (str) >= 0 )
  {
    int sid = re_sid.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Subtitle ID " << sid << "\n";
#endif
    if ( ! hasSubtitleID (sid) )
      addSubtitleID (sid);
  }
  else if ( re_slang.indexIn (str) >= 0 )
  {
    int sid = re_slang.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Subtitle ID " << sid << " language " << re_slang.cap(2) << "\n";
#endif
    if ( update || ! hasSubtitleLanguage (sid) )
      setSubtitleLanguage (sid, re_slang.cap(2));
  }
  else if ( re_vsid.indexIn (str) >= 0 )
  {
    int sid = re_vsid.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Vobsub ID " << sid << "\n";
#endif
    if ( ! hasVobsubID() && ! hasVobsubIDs() && showSubtitles() )
      setVobsubID (sid);
    if ( ! hasVobsubID (sid) )
      addVobsubID (sid);
  }
  else if ( re_vslang.indexIn (str) >= 0 )
  {
    int sid = re_vslang.cap(1).toInt();
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Vobsub ID " << sid << " language " << re_vslang.cap(2) << "\n";
#endif
    if ( update || ! hasVobsubLanguage (sid) )
      setVobsubLanguage (sid, re_vslang.cap(2));
  }
  else if ( (update || ! hasVideoCodec()) && ! hasVideoCodecOption() && re_vc.indexIn (str) >= 0 )
  {
    setVideoCodec (re_vc.cap(1));
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Video codec " << videoCodec() << "\n";
#endif
  }
  else if ( (update || ! hasAudioCodec()) && ! hasAudioCodecOption() && re_ac.indexIn (str) >= 0 )
  {
    setAudioCodec (re_ac.cap(1));
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Audio codec " << audioCodec() << "\n";
#endif
  }
  else if ( (update || ! hasDemuxer()) && ! hasDemuxerOption() && re_demuxer.indexIn (str) >= 0 )
  {
    setDemuxer (re_demuxer.cap(1));
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Demuxer " << demuxer() << "\n";
#endif
  }
  else if ( name() == defaultName() && re_name_sc.indexIn (str) >= 0 )
  {
    setName (re_name_sc.cap(1).simplified());
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: Name " << name() << "\n";
#endif
  }
  else if ( update && re_icyinfo.indexIn (str) >= 0 )
  {
    setTemporaryName (re_icyinfo.cap(1).simplified());
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << "Process: ICY Info " << temporaryName() << "\n";
#endif
    commit();
  }
}

int KPlayerTrackProperties::normalSeek (void) const
{
  int seek = configuration() -> progressNormalSeekUnits() ? configuration() -> progressNormalSeek()
    : hasLength() ? int (length() * configuration() -> progressNormalSeek() / 100 + 0.5) : 10;
  if ( seek == 0 )
    seek = 1;
  return seek;
}

int KPlayerTrackProperties::fastSeek (void) const
{
  int seek = configuration() -> progressFastSeekUnits() ? configuration() -> progressFastSeek()
    : hasLength() ? int (length() * configuration() -> progressFastSeek() / 100 + 0.5) : 60;
  if ( seek == 0 )
    seek = 2;
  return seek;
}

QSize KPlayerTrackProperties::getDisplaySize (const QString& key) const
{
  const QSize& current (currentSize());
  return has (key) ? ((KPlayerDisplaySizeProperty*) m_properties [key]) -> value (current) : current;
}

int KPlayerTrackProperties::getTrackOption (const QString& key) const
{
  if ( has (key) )
  {
    int i = 1;
    const QMap<int, QString>& ids (getIntegerStringMap (key + "s"));
    if ( ids.count() > 1 )
    {
      int id = getInteger (key);
      QMap<int, QString>::ConstIterator iterator (ids.constBegin()), end (ids.constEnd());
      while ( iterator != end && iterator.key() != id )
      {
        ++ iterator;
        ++ i;
      }
    }
    return i;
  }
  return 0;
}

void KPlayerTrackProperties::setTrackOption (const QString& key, int value)
{
  if ( value == 0 )
    reset (key);
  else
  {
    int i = 1, id = 0;
    const QMap<int, QString>& ids (getIntegerStringMap (key + "s"));
    QMap<int, QString>::ConstIterator iterator (ids.constBegin()), end (ids.constEnd());
    while ( iterator != end && value != i )
    {
      id = iterator.key();
      ++ iterator;
      ++ i;
    }
    setInteger (key, iterator == end ? id + 1 : iterator.key());
  }
}

int KPlayerTrackProperties::subtitleIndex (void) const
{
  if ( hasVobsubID() )
    return getTrackOption ("Vobsub ID") - 1;
  int vscount = vobsubIDs().count();
  if ( hasSubtitleID() )
    return getTrackOption ("Subtitle ID") + vscount - 1;
  return ! showSubtitles() ? -1 : vobsubSubtitles() ? 0 : vscount + int (subtitleIDs().count());
}

int KPlayerTrackProperties::subtitleOption (void) const
{
  if ( hasSubtitleID() )
    return getTrackOption ("Subtitle ID");
  int scount = subtitleIDs().count();
  if ( hasVobsubID() )
    return getTrackOption ("Vobsub ID") + scount;
  return showSubtitles() ? scount + int (vobsubIDs().count()) + 1 : 0;
}

void KPlayerTrackProperties::setSubtitleOption (int option)
{
  setShowSubtitles (option != 0);
  int scount = subtitleIDs().count();
  if ( option > 0 && option <= scount )
    setTrackOption ("Subtitle ID", option);
  else
    resetSubtitleID();
  if ( option > scount && option <= scount + int (vobsubIDs().count()) )
    setTrackOption ("Vobsub ID", option - scount);
  else
    resetVobsubID();
}

void KPlayerTrackProperties::showSubtitleUrl (const KUrl& url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerTrackProperties::showSubtitleUrl\n";
#endif
  if ( url != subtitleUrl() )
  {
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " Subtitle " << url.url() << "\n";
    kdDebugTime() << "          " << url.pathOrUrl() << "\n";
#endif
    setSubtitleUrl (url);
    resetSubtitleID();
    resetVobsubID();
  }
  setShowSubtitles (true);
}

bool KPlayerTrackProperties::needsExpanding (void) const
{
  //static QRegExp re_expand ("(?:^| )-vf +(?:[^ ]*,)?expand=");
  if ( hasOriginalSize() && configuration() -> hasSubtitleAutoexpand() )
  {
    QSize aspect = configuration() -> autoexpandAspect();
    QSize size = hasDisplaySize() ? displaySize() : currentSize();
    return size.height() * aspect.width() * 20 < size.width() * aspect.height() * 19;
  }
  return false;
}

void KPlayerTrackProperties::autoexpand (void)
{
  static QRegExp re_expand ("((?:^|.* )-vf +[^ ]+)(.*)");
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerTrackProperties::autoexpand\n";
#endif
  if ( hasOriginalSize() && configuration() -> hasSubtitleAutoexpand() )
  {
    QSize aspect = configuration() -> autoexpandAspect();
    const QSize& res = currentResolution();
    const QSize& size = currentSize();
    int height = size.width() * aspect.height() * res.height() / (aspect.width() * size.height());
#ifdef DEBUG_KPLAYER_PROPERTIES
    kdDebugTime() << " Resolution " << res.width() << "x" << res.height() << "\n";
    kdDebugTime() << " Size   " << size.width() << "x" << size.height() << "\n";
    kdDebugTime() << " Aspect " << aspect.width() << "x" << aspect.height() << "\n";
#endif
    if ( height > res.height() )
    {
      QString expand = "expand=" + QString::number (res.width()) + ":" + QString::number (height);
      int offset = res.width() / 10;
      height -= res.height();
      if ( offset + offset > height )
      {
        if ( offset > height )
          offset = height;
        if ( subtitlePosition() > 50 )
          offset = height - offset;
        expand += ":0:" + QString::number (offset);
      }
#ifdef DEBUG_KPLAYER_PROPERTIES
      kdDebugTime() << " Expand " << expand << "\n";
#endif
      if ( ! hasCommandLine() )
        setCommandLineOption ("-vf " + expand, 2);
      else if ( re_expand.indexIn (commandLineValue()) >= 0 )
        setCommandLine (re_expand.cap(1) + "," + expand + re_expand.cap(2));
      else
        setCommandLine (commandLineValue() + " -vf " + expand);
      setCurrentResolution (QSize (size.width(), height));
      setCurrentSize (QSize (size.width(), size.width() * aspect.height() / aspect.width()));
      resetDisplaySize();
    }
  }
}

KPlayerDiskTrackProperties::KPlayerDiskTrackProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerTrackProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating disk track properties\n";
#endif
}

KPlayerDiskTrackProperties::~KPlayerDiskTrackProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying disk track properties\n";
#endif
}

void KPlayerDiskTrackProperties::setupInfo (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerDiskTrackProperties::setupInfo\n";
#endif
  KPlayerTrackProperties::setupInfo();
  QString name (url().fileName().rightJustified (parent() -> digits(), '0'));
  setDefaultName ((parent() -> type() == "DVD" ? i18n("Title %1", name) : i18n("Track %1", name)));
}

QString KPlayerDiskTrackProperties::icon (void) const
{
  return parent() -> getString ("Type") == "Audio CD" ? "audio-basic" : "video";
}

QString KPlayerDiskTrackProperties::deviceOption (void) const
{
  return parent() -> type() == "DVD" ? "-dvd-device" : "-cdrom-device";
}

QString KPlayerDiskTrackProperties::deviceSetting (void) const
{
  return parent() -> pathString();
}

QString KPlayerDiskTrackProperties::urlString (void) const
{
  const QString& type = parent() -> type();
  return (type == "DVD" ? "dvd://" : type == "Video CD" ? "vcd://" : "cdda://") + url().fileName();
}

KPlayerChannelProperties::KPlayerChannelProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerTrackProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating channel properties\n";
#endif
  setDefaultFrequency (0);
}

KPlayerChannelProperties::~KPlayerChannelProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying channel properties\n";
#endif
}

QString KPlayerChannelProperties::icon (void) const
{
  return "video";
}

bool KPlayerChannelProperties::needsFrequency (void) const
{
  return true;
}

int KPlayerChannelProperties::frequency (void) const
{
  return ((KPlayerFrequencyProperty*) m_properties ["Frequency"]) -> value();
}

void KPlayerChannelProperties::setFrequency (int frequency)
{
  ((KPlayerFrequencyProperty*) get ("Frequency")) -> setValue (frequency);
  updated ("Frequency");
}

KPlayerTVChannelProperties::KPlayerTVChannelProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerChannelProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating TV channel properties\n";
#endif
}

KPlayerTVChannelProperties::~KPlayerTVChannelProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying TV channel properties\n";
#endif
}

void KPlayerTVChannelProperties::setupInfo (void)
{
  static QRegExp re_channel ("^([A-Z]*)(\\d+)([A-Z]*)$");
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerTVChannelProperties::setupInfo\n";
#endif
  KPlayerChannelProperties::setupInfo();
  QString id (url().fileName());
  setDefaultName (i18n("Channel %1", re_channel.indexIn (id) < 0 ? id : re_channel.cap(1)
    + QString::number (re_channel.cap(2).toInt()).rightJustified (re_channel.cap(1).isEmpty() ? parent() -> digits()
    : re_channel.cap(1) == "H" && re_channel.cap(2).length() == 1 ? 1 : 2, '0') + re_channel.cap(3)));
  setDefaultFrequency (parent() -> channelFrequency (id));
}

bool KPlayerTVChannelProperties::canChangeFrequency (void) const
{
  return true;
}

QString KPlayerTVChannelProperties::deviceOption (void) const
{
  return "-tv";
}

QString KPlayerTVChannelProperties::deviceSetting (void) const
{
  QString setting = "driver=" + inputDriver() + ":device=" + parent() -> pathString();
  if ( frequency() != defaultFrequency() )
    setting += ":freq=" + QString::number (frequency());
  else
    setting += ":chanlist=" + channelList() + ":channel=" + url().fileName();
  if ( hasVideoNorm() || parent() -> hasVideoNorm() )
  {
    const char* norms[] = { "NTSC", "NTSC-JP", "PAL", "PAL-60",
      "PAL-BG", "PAL-DK", "PAL-I", "PAL-M", "PAL-N", "PAL-NC", "SECAM" };
    int norm = videoNorm();
    if ( norm >= 0 )
      setting += ":normid=" + QString::number (videoNorm());
    else if ( norm < -1 && norm > - int (sizeof (norms) / sizeof (const char*)) - 2 )
      setting += QString (":norm=") + norms [- norm - 2];
  }
  if ( hasVideoFormat() || parent() -> hasVideoFormat() )
    setting += ":outfmt=" + videoFormat();
  if ( hasVideoInput() || parent() -> hasVideoInput() )
    setting += ":input=" + QString::number (videoInput());
  if ( hasAudioMode() || parent() -> hasAudioMode() )
    setting += ":amode=" + QString::number (audioMode()) + ":forceaudio";
  if ( hasAudioInput() || parent() -> hasAudioInput() )
    setting += ":audioid=" + QString::number (audioInput());
  setting += QString (":immediatemode=") + (immediateMode() ? "1" : "0");
  if ( alsaCapture() )
    setting += ":alsa";
  if ( hasCaptureDevice() || parent() -> hasCaptureDevice() )
    setting += ":adevice=" + captureDevice();
  if ( hasMjpegDecimation() || parent() -> hasMjpegDecimation() )
    setting += ":mjpeg:decimation=" + QString::number (mjpegDecimation());
  if ( hasMjpegQuality() || parent() -> hasMjpegQuality() )
      setting += ":quality=" + QString::number (mjpegQuality());
  return setting;
}

QString KPlayerTVChannelProperties::urlString (void) const
{
  return "tv://";
}

KPlayerDVBChannelProperties::KPlayerDVBChannelProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerChannelProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating DVB channel properties\n";
#endif
}

KPlayerDVBChannelProperties::~KPlayerDVBChannelProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying DVB channel properties\n";
#endif
}

void KPlayerDVBChannelProperties::setupInfo (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerDVBChannelProperties::setupInfo\n";
#endif
  KPlayerChannelProperties::setupInfo();
  QString id (url().fileName());
  setDefaultName (parent() -> channelName (id));
  setDefaultFrequency (parent() -> channelFrequency (id));
}

bool KPlayerDVBChannelProperties::canChangeFrequency (void) const
{
  return false;
}

QString KPlayerDVBChannelProperties::deviceOption (void) const
{
  return "-dvbin";
}

QString KPlayerDVBChannelProperties::deviceSetting (void) const
{
  QString setting;
  QRegExp re_card ("^kplayer:/devices/dev/dvb/adapter(\\d+)");
  if ( re_card.indexIn (url().url()) >= 0 )
    setting = "card=" + QString::number (re_card.cap(1).toInt() + 1);
  if ( hasVideoInput() || parent() -> hasVideoInput() )
  {
    if ( ! setting.isEmpty() )
      setting += ":";
    setting += "vid=" + QString::number (videoInput());
  }
  if ( hasAudioInput() || parent() -> hasAudioInput() )
  {
    if ( ! setting.isEmpty() )
      setting += ":";
    setting += "aid=" + QString::number (audioInput());
  }
  if ( hasChannelList() || parent() -> hasChannelList() )
  {
    if ( ! setting.isEmpty() )
      setting += ":";
    setting += "file=" + channelList();
  }
  return setting;
}

QString KPlayerDVBChannelProperties::urlString (void) const
{
  return "dvb://" + defaultName();
}

KPlayerItemProperties::KPlayerItemProperties (KPlayerProperties* parent, const KUrl& url)
  : KPlayerTrackProperties (parent, url)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Creating item properties\n";
#endif
}

KPlayerItemProperties::~KPlayerItemProperties()
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "Destroying item properties\n";
#endif
}

void KPlayerItemProperties::setupInfo (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerItemProperties::setupInfo\n";
#endif
  KPlayerTrackProperties::setupInfo();
  if ( configGroup().readEntry ("Video Size") == "0,0" )
  {
    configGroup().deleteEntry ("Video Size");
    configGroup().writeEntry ("Has Video", false);
  }
  QString value (configGroup().readEntry ("Full Screen"));
  if ( value == "0" )
    configGroup().writeEntry ("Full Screen", false);
  else if ( value == "1" )
    configGroup().writeEntry ("Full Screen", true);
  else if ( value == "2" )
  {
    configGroup().deleteEntry ("Full Screen");
    configGroup().writeEntry ("Maximized", true);
  }
  value = configGroup().readEntry ("Maintain Aspect");
  if ( value == "0" )
    configGroup().writeEntry ("Maintain Aspect", true);
  else if ( value == "1" )
    configGroup().writeEntry ("Maintain Aspect", false);
  value = configGroup().readEntry ("Autoload Subtitles");
  if ( value == "0" )
    configGroup().writeEntry ("Autoload Subtitles", true);
  else if ( value == "1" )
    configGroup().writeEntry ("Autoload Subtitles", false);
  value = configGroup().readEntry ("Subtitle Visibility");
  if ( value == "0" )
    configGroup().writeEntry ("Subtitle Visibility", true);
  else if ( value == "1" )
    configGroup().writeEntry ("Subtitle Visibility", false);
  value = configGroup().readEntry ("Command Line Option");
  if ( value == "1" )
    configGroup().writeEntry ("Command Line Option", true);
  value = configGroup().readEntry ("Playlist");
  if ( value == "1" )
    configGroup().writeEntry ("Playlist", true);
  else if ( value == "2" )
    configGroup().writeEntry ("Playlist", false);
  setPath (m_url);
}

void KPlayerItemProperties::setupMeta (void)
{
#ifdef DEBUG_KPLAYER_PROPERTIES
  kdDebugTime() << "KPlayerItemProperties::setupMeta\n";
#endif
#ifdef KPLAYER_EXTRACT_META_INFORMATION
  if ( ! hasIcon() && m_meta_info_timer < 2000 )
  {
    QTime timer;
    timer.start();
    KMimeType::Ptr mimetype (KMimeType::findByUrl (url()));
    if ( mimetype != KMimeType::defaultMimeTypePtr() )
      setType (mimetype -> name().toLower());
    setIcon (mimetype -> iconName());
    KFileMetaInfo info (url());
    if ( info.isValid() )
    {
      QStringList keys (info.supportedKeys());
      for ( QStringList::ConstIterator iterator = keys.constBegin(); iterator != keys.constEnd(); ++ iterator )
      {
        QString key (*iterator);
        KFileMetaInfoItem item (info.item (key));
        if ( item.isValid() && ! key.isEmpty() )
          if ( item.value().type() == QVariant::Size )
          {
            key = key.toLower();
            capitalizeWords (key);
            //if ( key == "Resolution" )
            //  key = "Video Size";
            if ( ! has (key) )
            {
#ifdef DEBUG_KPLAYER_PROPERTIES
              kdDebugTime() << " Extracted size " << key << " " << item.value().toSize().width() << "x" << item.value().toSize().height() << "\n";
#endif
              setSize (key, item.value().toSize());
            }
          }
          else if ( item.value().type() != QVariant::Bool )
            importMeta (key, item.value().toString());
      }
    }
    int elapsed = timer.elapsed();
    if ( elapsed >= 100 )
      m_meta_info_timer += elapsed;
  }
#endif
  KPlayerTrackProperties::setupMeta();
}

QString KPlayerItemProperties::urlString (void) const
{
  return url().isLocalFile() ? url().path() : url().url();
}

bool KPlayerItemProperties::autoloadSubtitles (const QString& key) const
{
  return url().isLocalFile()
    && (has (key) ? ((KPlayerBooleanProperty*) m_properties [key]) -> value() : parent() -> autoloadSubtitles (key));
}

bool KPlayerItemProperties::getVobsubSubtitles (const QString& key, const KUrl& url) const
{
  return has (key) ? ((KPlayerBooleanProperty*) m_properties [key]) -> value() : parent() -> getVobsubSubtitles (key, url);
}

bool KPlayerItemProperties::getPlaylist (const QString& key, const KUrl& url) const
{
  return has (key) ? ((KPlayerBooleanProperty*) m_properties [key]) -> value() : parent() -> getPlaylist (key, url);
}

bool KPlayerItemProperties::getUseKioslave (const QString& key, const KUrl& url) const
{
  return has (key) ? ((KPlayerBooleanProperty*) m_properties [key]) -> value() : parent() -> getUseKioslave (key, url);
}
