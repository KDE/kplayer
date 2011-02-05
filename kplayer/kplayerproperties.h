/***************************************************************************
                          kplayerproperties.h
                          -------------------
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

#ifndef KPLAYERPROPERTIES_H
#define KPLAYERPROPERTIES_H

#include <cfloat>
#include <climits>
#include <cmath>
#include <QObject>
#include <KUrl>

#ifdef DEBUG
#include <KDebug>
#endif

#include "kplayerengine.h"

class KPlayerProperty;
class KPlayerPropertyInfo;
class KPlayerProperties;
class KPlayerMedia;
class KPlayerConfiguration;
class KPlayerGenericProperties;
class KPlayerDeviceProperties;
class KPlayerDiskProperties;
class KPlayerTVProperties;
class KPlayerDVBProperties;
class KPlayerTrackProperties;
class KPlayerChannelProperties;
class KPlayerTVChannelProperties;
class KPlayerDVBChannelProperties;
class KPlayerItemProperties;

#define KPLAYER_PROPERTY_GROUP_BASIC    0
#define KPLAYER_PROPERTY_GROUP_GENERAL  1
#define KPLAYER_PROPERTY_GROUP_FORMAT   2
#define KPLAYER_PROPERTY_GROUP_SIZE     3
#define KPLAYER_PROPERTY_GROUP_VIDEO    4
#define KPLAYER_PROPERTY_GROUP_AUDIO    5
#define KPLAYER_PROPERTY_GROUP_INFO     6
#define KPLAYER_PROPERTY_GROUP_LOCATION 7

/** Compares the given strings. */
extern int compareStrings (const QString& s1, const QString& s2);

/** Returns the given time length as string. */
extern QString timeString (float length, bool zero_ok = false);

/** Converts a string to a floating point number, replacing a comma with a decimal point. */
extern float stringToFloat (QString);

inline int limit (int value, int minValue, int maxValue = INT_MAX)
{
  return value < minValue ? minValue : value > maxValue ? maxValue : value;
}

inline void limit4 (int& lowerValue, int& higherValue, int minValue, int maxValue)
{
  if ( lowerValue > higherValue )
  {
    int i = lowerValue;
    lowerValue = higherValue;
    higherValue = i;
  }
  lowerValue = limit (lowerValue, minValue, maxValue);
  higherValue = limit (higherValue, minValue, maxValue);
}

inline float flimit (float value, float minValue, float maxValue = FLT_MAX)
{
  return value < minValue ? minValue : value > maxValue ? maxValue : value;
}

/** The KPlayer media map by URL string.
  * @author kiriuja
  */
typedef QMap<QString, KPlayerMedia*> KPlayerMediaMap;

/** The KPlayer property map by name.
  * @author kiriuja
  */
typedef QMap<QString, KPlayerProperty*> KPlayerPropertyMap;

/** The KPlayer property info map by name.
  * @author kiriuja
  */
typedef QMap<QString, KPlayerPropertyInfo*> KPlayerPropertyInfoMap;

/** The KPlayer media property.
  * @author kiriuja
  */
class KPlayerProperty
{
public:
  /** Default constructor. */
  KPlayerProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerProperty();

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Sets the value from the given string. */
  virtual void fromString (const QString& value);
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup& config, const QString& name);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup& config, const QString& name) const;

  /** Resets the value to the default and returns true if the property can be deleted. */
  virtual bool defaults (bool can_reset);
};

/** The boolean property.
  * @author kiriuja
  */
class KPlayerBooleanProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerBooleanProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerBooleanProperty();

  /** Returns the property value. */
  bool value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (bool value)
    { m_value = value; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property value. */
  bool m_value;
};

/** The integer property.
  * @author kiriuja
  */
class KPlayerIntegerProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerIntegerProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerIntegerProperty();

  /** Returns the property value. */
  int value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (int value)
    { m_value = value; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Sets the value from the given string. */
  virtual void fromString (const QString& value);
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property value. */
  int m_value;
};

/** The relative integer property.
  * @author kiriuja
  */
class KPlayerRelativeProperty : public KPlayerIntegerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerRelativeProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerRelativeProperty();

  /** Returns the property value. */
  int value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (int value)
    { m_value = value; }

  /** Returns the property value based on the option and the given value. */
  int value (int current) const;
  /** Sets the property value and option. */
  void setValue (int value, int current);

  /** Returns the property option. */
  int option (void) const
    { return m_option; }
  /** Sets the property option. */
  void setOption (int option)
    { m_option = option; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property option. */
  int m_option;
};

/** The cache property.
  * @author kiriuja
  */
class KPlayerCacheProperty : public KPlayerIntegerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerCacheProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerCacheProperty();

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
};

/** The frequency property.
  * @author kiriuja
  */
class KPlayerFrequencyProperty : public KPlayerIntegerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerFrequencyProperty (KPlayerProperties* properties)
  {
    m_properties = (KPlayerChannelProperties*) properties;
    m_value = 0;
  }
  /** Destructor. */
  virtual ~KPlayerFrequencyProperty();

  /** Returns the property value. */
  int value (void) const;
  /** Sets the property value. */
  void setValue (int value);

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

  /** Resets the value to the default and returns true if the property can be deleted. */
  virtual bool defaults (bool can_reset);

protected:
  /** Properties where this property is from. */
  KPlayerChannelProperties* m_properties;
};

/** The float property.
  * @author kiriuja
  */
class KPlayerFloatProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerFloatProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerFloatProperty();

  /** Returns the property value. */
  float value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (float value)
    { m_value = fabs (value) < 0.0001 ? 0 : value; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property value. */
  float m_value;
};

/** The length property.
  * @author kiriuja
  */
class KPlayerLengthProperty : public KPlayerFloatProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerLengthProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerLengthProperty();

  /** Returns the value as a string. */
  virtual QString asString (void) const;
};

/** The size property.
  * @author kiriuja
  */
class KPlayerSizeProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerSizeProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerSizeProperty();

  /** Returns the property value. */
  const QSize& value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (const QSize& value)
    { m_value = value; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property value. */
  QSize m_value;
};

/** The display size property.
  * @author kiriuja
  */
class KPlayerDisplaySizeProperty : public KPlayerSizeProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerDisplaySizeProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerDisplaySizeProperty();

  /** Returns the property value. */
  const QSize& value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (const QSize& value)
    { m_value = value; }

  /** Returns the property value based on the option and the given value. */
  QSize value (const QSize& current) const;
  /** Sets the property value and option. */
  void setValue (const QSize& value, int option);

  /** Returns the property option. */
  int option (void) const
    { return m_option; }
  /** Sets the property option. */
  void setOption (int option)
    { m_option = option; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property option. */
  int m_option;
};

/** The string property.
  * @author kiriuja
  */
class KPlayerStringProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerStringProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerStringProperty();

  /** Returns the property value. */
  const QString& value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (const QString& value)
    { m_value = value; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Sets the value from the given string. */
  virtual void fromString (const QString& value);
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property value. */
  QString m_value;
};

/** The combo string property.
  * @author kiriuja
  */
class KPlayerComboStringProperty : public KPlayerStringProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerComboStringProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerComboStringProperty();

  /** Returns the property option. */
  const QString& option (void) const
    { return m_option; }
  /** Sets the property option. */
  void setOption (const QString& option)
    { m_option = option; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

  /** Resets the value to the default and returns true if the property can be deleted. */
  virtual bool defaults (bool can_reset);

protected:
  /** Property option. */
  QString m_option;
};

/** The translated string property.
  * @author kiriuja
  */
class KPlayerTranslatedStringProperty : public KPlayerStringProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerTranslatedStringProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerTranslatedStringProperty();

  /** Returns the value as a string. */
  virtual QString asString (void) const;
};

#if 0
/** The string history property.
  * @author kiriuja
  */
class KPlayerStringHistoryProperty : public KPlayerStringProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerStringHistoryProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerStringHistoryProperty();

  /** Sets the property value. */
  virtual void setValue (const QString& value);

  /** Returns the string history. */
  const QStringList& history (void) const
    { return m_history; }

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** String history. */
  QStringList m_history;
};
#endif

/** The name property.
  * @author kiriuja
  */
class KPlayerNameProperty : public KPlayerStringProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerNameProperty (KPlayerProperties* properties)
    { m_properties = (KPlayerGenericProperties*) properties; }
  /** Destructor. */
  virtual ~KPlayerNameProperty();

  /** Returns the value as a string. */
  virtual QString asString (void) const;

  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup& config, const QString& name) const;

  /** Resets the value to the default and returns true if the property can be deleted. */
  virtual bool defaults (bool can_reset);

protected:
  /** Properties where this property is from. */
  KPlayerGenericProperties* m_properties;
};

/** The appendable string property.
  * @author kiriuja
  */
class KPlayerAppendableProperty : public KPlayerStringProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerAppendableProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerAppendableProperty();

  /** Returns the property value based on the option and the given value. */
  QString appendableValue (const QString& current) const;
  /** Sets the property value and option. */
  void setAppendableValue (const QString& value, bool append);

  /** Returns the property option. */
  bool option (void) const
    { return m_option; }
  /** Sets the property option. */
  void setOption (bool option)
    { m_option = option; }

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

protected:
  /** Property option. */
  bool m_option;
};

/** The string list property.
  * @author kiriuja
  */
class KPlayerStringListProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerStringListProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerStringListProperty();

  /** Returns the property value. */
  const QStringList& value (void) const
    { return m_value; }
  /** Adds the given entry to the property value. */
  void addEntry (const QString& entry)
    { m_value.append (entry); }
  /** Sets the property value. */
  void setValue (const QStringList& value)
    { m_value = value; }

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup& config, const QString& name);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup& config, const QString& name) const;

  /** Resets the value to the default and returns true if the property can be deleted. */
  virtual bool defaults (bool can_reset);

protected:
  /** Property value. */
  QStringList m_value;
};

/** The integer string map property.
  * @author kiriuja
  */
class KPlayerIntegerStringMapProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerIntegerStringMapProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerIntegerStringMapProperty();

  /** Returns the property value. */
  const QMap<int, QString>& value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (int key, const QString& value)
    { m_value.insert (key, value); }

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup& config, const QString& name);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup& config, const QString& name) const;

protected:
  /** Property value. */
  QMap<int, QString> m_value;
};

/** The URL property.
  * @author kiriuja
  */
class KPlayerUrlProperty : public KPlayerProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerUrlProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerUrlProperty();

  /** Returns the property value. */
  const KUrl& value (void) const
    { return m_value; }
  /** Sets the property value. */
  void setValue (const KUrl& value)
    { m_value = value; }

  /** Returns the value as a string. */
  virtual QString asString (void) const;
  /** Compares the value with that of another property of the same type. */
  virtual int compare (KPlayerProperty*) const;

protected:
  /** Property value. */
  KUrl m_value;
};

/** The persistent URL property.
  * @author kiriuja
  */
class KPlayerPersistentUrlProperty : public KPlayerUrlProperty
{
public:
  /** Constructor. Initializes the property. */
  KPlayerPersistentUrlProperty (void) { }
  /** Destructor. */
  virtual ~KPlayerPersistentUrlProperty();

  /** Reads the value from the given config under the given name. */
  virtual void read (KConfigGroup&, const QString&);
  /** Saves the value to the given config under the given name. */
  virtual void save (KConfigGroup&, const QString&) const;

  /** Resets the value to the default and returns true if the property can be deleted. */
  virtual bool defaults (bool can_reset);
};

/** The KPlayer media property information.
  * @author kiriuja
  */
class KPlayerPropertyInfo
{
public:
  /** Default constructor. */
  KPlayerPropertyInfo (void);
  /** Destructor. */
  virtual ~KPlayerPropertyInfo();

  /** Returns the property caption. */
  const QString& caption (void) const
    { return m_caption; }
  /** Sets the property caption. */
  void setCaption (const QString& caption)
    { m_caption = caption; }

  /** Returns the property group. */
  int group (void) const
    { return m_group; }
  /** Sets the property group. */
  void setGroup (int group)
    { m_group = group; m_show = showByDefault(); }

  /** Returns whether the property can be shown. */
  bool canShow (void) const
    { return group() >= 0; }
  /** Returns whether the property should be shown by default. */
  bool showByDefault (void) const
    { return group() == 0 || group() == 1 || group() == 5; }

  /** Returns whether the property should be shown. */
  bool show (void) const
    { return m_show; }
  /** Sets whether the property should be shown. */
  void setShow (bool show)
    { m_show = show; }

  /** Returns whether the property can be edited. */
  bool canEdit (void) const
    { return m_can_edit; }
  /** Sets whether the property can be edited. */
  void setCanEdit (bool can_edit)
    { m_can_edit = can_edit; }

  /** Returns whether the property can be reset. */
  bool canReset (void) const
    { return m_can_reset; }
  /** Sets whether the property can be reset. */
  void setCanReset (bool can_reset)
    { m_can_reset = can_reset; }

  /** Returns the override flag. */
  bool override (void) const
    { return m_override; }
  /** Sets the override flag. */
  void setOverride (bool override)
    { m_override = override; }

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const = 0;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const = 0;

  /** Returns whether the property exists for the current URL. */
  virtual bool exists (KPlayerProperties* properties, const QString& name) const;

protected:
  /** Caption. */
  QString m_caption;
  /** Property group. */
  int m_group;
  /** Flag indicating whether the property should be shown. */
  bool m_show;
  /** Flag indicating whether the property can be edited. */
  bool m_can_edit;
  /** Flag indicating whether the property can be reset. */
  bool m_can_reset;
  /** Flag indicating whether an override is in effect. */
  bool m_override;
};

/** The KPlayer boolean property information.
  * @author kiriuja
  */
class KPlayerBooleanPropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerBooleanPropertyInfo (void);
  /** Destructor. */
  virtual ~KPlayerBooleanPropertyInfo();

  /** Returns the default property value. */
  bool defaultValue (void) const
    { return m_default; }
  /** Sets the default property value. */
  void setDefaultValue (bool value)
    { m_default = value; }

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

protected:
  /** Default value. */
  bool m_default;
};

/** The KPlayer integer property information.
  * @author kiriuja
  */
class KPlayerIntegerPropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerIntegerPropertyInfo (void);
  /** Destructor. */
  virtual ~KPlayerIntegerPropertyInfo();

  /** Returns the default property value. */
  int defaultValue (void) const
    { return m_default; }
  /** Sets the default property value. */
  void setDefaultValue (int value)
    { m_default = value; }

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

protected:
  /** Default value. */
  int m_default;
};

/** The KPlayer relative property information.
  * @author kiriuja
  */
class KPlayerRelativePropertyInfo : public KPlayerIntegerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerRelativePropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerRelativePropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer cache property information.
  * @author kiriuja
  */
class KPlayerCachePropertyInfo : public KPlayerIntegerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerCachePropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerCachePropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer frequency property information.
  * @author kiriuja
  */
class KPlayerFrequencyPropertyInfo : public KPlayerIntegerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerFrequencyPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerFrequencyPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

  /** Returns whether the property exists for the current URL. */
  virtual bool exists (KPlayerProperties* properties, const QString& name) const;
};

/** The KPlayer float property information.
  * @author kiriuja
  */
class KPlayerFloatPropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerFloatPropertyInfo (void);
  /** Destructor. */
  virtual ~KPlayerFloatPropertyInfo();

  /** Returns the default property value. */
  float defaultValue (void) const
    { return m_default; }
  /** Sets the default property value. */
  void setDefaultValue (float value)
    { m_default = value; }

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

protected:
  /** Default value. */
  float m_default;
};

/** The KPlayer length property information.
  * @author kiriuja
  */
class KPlayerLengthPropertyInfo : public KPlayerFloatPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerLengthPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerLengthPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer size property information.
  * @author kiriuja
  */
class KPlayerSizePropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerSizePropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerSizePropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer optional size property information.
  * @author kiriuja
  */
class KPlayerDisplaySizePropertyInfo : public KPlayerSizePropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerDisplaySizePropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerDisplaySizePropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer string property information.
  * @author kiriuja
  */
class KPlayerStringPropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerStringPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerStringPropertyInfo();

  /** Returns the default property value. */
  const QString& defaultValue (void) const
    { return m_default; }
  /** Sets the default property value. */
  void setDefaultValue (const QString& value)
    { m_default = value; }

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

protected:
  /** Default value. */
  QString m_default;
};

/** The KPlayer combo string property information.
  * @author kiriuja
  */
class KPlayerComboStringPropertyInfo : public KPlayerStringPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerComboStringPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerComboStringPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

  /** Returns whether the property exists for the current URL. */
  virtual bool exists (KPlayerProperties* properties, const QString& name) const;
};

/** The KPlayer translated string property information.
  * @author kiriuja
  */
class KPlayerTranslatedStringPropertyInfo : public KPlayerStringPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerTranslatedStringPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerTranslatedStringPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

#if 0
/** The KPlayer string history property information.
  * @author kiriuja
  */
class KPlayerStringHistoryPropertyInfo : public KPlayerStringPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerStringHistoryPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerStringHistoryPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

  /** Returns whether the property exists for the current URL. */
  virtual bool exists (KPlayerProperties* properties, const QString& name) const;
};
#endif

/** The KPlayer name property information.
  * @author kiriuja
  */
class KPlayerNamePropertyInfo : public KPlayerStringPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerNamePropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerNamePropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

  /** Returns whether the property exists for the current URL. */
  virtual bool exists (KPlayerProperties* properties, const QString& name) const;
};

/** The KPlayer appendable property information.
  * @author kiriuja
  */
class KPlayerAppendablePropertyInfo : public KPlayerStringPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerAppendablePropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerAppendablePropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

  /** Returns whether the property exists for the current URL. */
  virtual bool exists (KPlayerProperties* properties, const QString& name) const;
};

/** The KPlayer string list property information.
  * @author kiriuja
  */
class KPlayerStringListPropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerStringListPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerStringListPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer integer string map property information.
  * @author kiriuja
  */
class KPlayerIntegerStringMapPropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerIntegerStringMapPropertyInfo (bool multi)
    { m_multi = multi; }
  /** Destructor. */
  virtual ~KPlayerIntegerStringMapPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;

  /** Returns whether multiple entries are required. */
  bool multipleEntriesRequired (void) const
    { return m_multi; }

protected:
  /** Indicates whether multiple entries are required. */
  bool m_multi;
};

/** The KPlayer URL property information.
  * @author kiriuja
  */
class KPlayerUrlPropertyInfo : public KPlayerPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerUrlPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerUrlPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer persistent URL property information.
  * @author kiriuja
  */
class KPlayerPersistentUrlPropertyInfo : public KPlayerUrlPropertyInfo
{
public:
  /** Constructor. Sets up the property information. */
  KPlayerPersistentUrlPropertyInfo (void) { }
  /** Destructor. */
  virtual ~KPlayerPersistentUrlPropertyInfo();

  /** Creates and returns a property of the corresponding type. */
  virtual KPlayerProperty* create (KPlayerProperties*) const;
  /** Makes a copy of the given property. */
  virtual KPlayerProperty* copy (const KPlayerProperty*) const;
};

/** The KPlayer property counts by name.
  * @author kiriuja
  */
class KPlayerPropertyCounts : public QMap<QString, int>
{
public:
  /** Default constructor. Creates an empty map. */
  KPlayerPropertyCounts (void) { }

  /** Returns the count for the given key. */
  int count (const QString& key) const;
  /** Returns the number of properties. */

  /** Adds the given counts. */
  void add (const KPlayerPropertyCounts& counts);
  /** Subtracts the given counts. */
  void subtract (const KPlayerPropertyCounts& counts);
};

/** The KPlayer properties.
  * @author kiriuja
  */
class KPlayerProperties : public QObject
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerProperties (void);
  /** Destructor. */
  virtual ~KPlayerProperties();

  /** Sets up media properties. */
  void setup (void);

  /** Initializes property information. */
  virtual void setupInfo (void);
  /** Initializes meta properties. */
  virtual void setupMeta (void);

  void defaults (void);
  void commit (void);

  /** Returns the config storage. */
  virtual KConfig* config (void) const = 0;
  /** Returns the config group name. */
  virtual QString configGroupName (void) const = 0;
  /** Returns the config group. */
  KConfigGroup& configGroup (void)
    { return m_config_group; }

  // Property access

  /** Returns property information map. */
  static const KPlayerPropertyInfoMap& info (void)
    { return m_info; }

  /** Returns property information for the given key. */
  static KPlayerPropertyInfo* info (const QString& key);
  /** Returns boolean property information for the given key. */
  static KPlayerBooleanPropertyInfo* booleanInfo (const QString& key)
    { return (KPlayerBooleanPropertyInfo*) info (key); }
  /** Returns integer property information for the given key. */
  static KPlayerIntegerPropertyInfo* integerInfo (const QString& key)
    { return (KPlayerIntegerPropertyInfo*) info (key); }
  /** Returns float property information for the given key. */
  static KPlayerFloatPropertyInfo* floatInfo (const QString& key)
    { return (KPlayerFloatPropertyInfo*) info (key); }
  /** Returns string property information for the given key. */
  static KPlayerStringPropertyInfo* stringInfo (const QString& key)
    { return (KPlayerStringPropertyInfo*) info (key); }

  /** Returns default order of attributes. */
  static QStringList defaultOrder (void);
  /** Returns the list of default meta attributes. */
  static const QStringList& defaultAttributes (void)
    { return m_meta_attributes; }

  /** Returns whether the property for the given key can be edited. */
  bool canEdit (const QString& key)
    { return info (key) -> canEdit(); }
  /** Returns whether the property for the given key can be reset. */
  bool canReset (const QString& key)
    { return info (key) -> canReset(); }

  /** Returns the list of property keys. */
  QStringList keys (void) const;

  /** Returns whether meta information for the given key is available. */
  bool has (const QString& key) const
    { return m_properties.contains (key); }

  /** Returns whether meta information for the given key was available prior to the change. */
  bool had (const QString& key) const
    { return m_previous.contains (key); }

  /** Returns the property with the given name if it exists. */
  KPlayerProperty* property (const QString& key) const;
  /** Returns the property map. */
  const KPlayerPropertyMap& properties (void) const
    { return m_properties; }

  /** Returns the property with the given name, creating it if necessary. */
  KPlayerProperty* get (const QString& key);
  /** Removes the property with the given name. */
  void reset (const QString& key);
  /** Removes all properties from the store. */
  void purge (void)
    { config() -> deleteGroup (configGroupName()); }

  /** Compares the properties to the given properties and emits the updated signal. */
  void diff (KPlayerProperties* media);
  /** Compares the property with the given name to that from the given properties. */
  int compare (KPlayerProperties* properties, const QString& key) const;

  /** Increments the given counts for existing properties. */
  void count (KPlayerPropertyCounts& counts) const;

  /** Returns a map of added properties. */
  const KPlayerPropertyCounts& added (void) const
    { return m_added; }
  /** Returns a map of changed properties. */
  const KPlayerPropertyCounts& changed (void) const
    { return m_changed; }
  /** Returns a map of removed properties. */
  const KPlayerPropertyCounts& removed (void) const
    { return m_removed; }

  virtual bool getBoolean (const QString& key) const = 0;
  virtual void setBoolean (const QString& key, bool value) = 0;
  int getBooleanOption (const QString& key) const;
  void setBooleanOption (const QString& key, int value);

  virtual int getInteger (const QString& key) const = 0;
  virtual void setInteger (const QString& key, int value) = 0;
  int getIntegerOption (const QString& key) const;
  void setIntegerOption (const QString& key, int value);
  void set (const QString& key, int value);

  virtual int getRelativeValue (const QString& key) const = 0;
  int getRelative (const QString& key) const;
  virtual void setRelative (const QString& key, int value) = 0;
  virtual void adjustRelative (const QString& key, int value) = 0;
  int getRelativeOption (const QString& key) const;
  void setRelativeOption (const QString& key, int value, int option);

  virtual int getCache (const QString& key) const = 0;
  virtual int getCacheSize (const QString& key) const = 0;
  int getCacheOption (const QString& key) const;

  virtual float getFloat (const QString& key) const = 0;
  virtual void setFloat (const QString& key, float value) = 0;
  void set (const QString& key, float value);

  const QSize& getSize (const QString& key) const;
  virtual QSize getDisplaySize (const QString& key) const;
  int getSizeOption (const QString& key) const;
  void setSize (const QString& key, const QSize& value);
  void setSize (const QString& key, const QSize& value, int option);

  /** Returns the value of the given key as a string. */
  QString asString (const QString& key) const;
  /** Returns the integer value of the given key as a string. */
  QString asIntegerString (const QString& key) const;
  /** Sets the value for the given key from the given string. */
  void fromString (const QString& key, const QString& value);

  virtual const QString& getString (const QString& key) const = 0;
  void setString (const QString& key, const QString& value);
  void set (const QString& key, const QString& value);

  const QString& getStringValue (const QString& key) const;

  void setComboValue (const QString& key, const QString& value);
  bool hasComboValue (const QString& key) const;

  virtual const QString& getComboString (const QString& key) const = 0;
  bool hasComboString (const QString& key) const;
  const QString& getStringOption (const QString& key) const;
  void setStringOption (const QString& key, const QString& value);

  //const QStringList& getHistory (const QString& key) const;

  virtual QString getAppendable (const QString& key) const = 0;
  int getAppendableOption (const QString& key) const;
  void setAppendable (const QString& key, const QString& value, int option);

  const QStringList& getStringList (const QString& key) const;
  void addStringListEntry (const QString& key, const QString& entry);
  void setStringList (const QString& key, const QStringList& value);

  const QMap<int, QString>& getIntegerStringMap (const QString& key) const;
  void setIntegerStringMapKey (const QString& key, int id);
  void setIntegerStringMapKeyValue (const QString& key, int id, const QString& lang);
  bool hasIntegerStringMapKey (const QString& key, int id) const;
  bool hasIntegerStringMapValue (const QString& key, int id) const;

  const KUrl& getUrl (const QString& key) const;
  void setUrl (const QString& key, const KUrl& value);

  virtual bool autoloadSubtitles (const QString& key) const;
  virtual bool getVobsubSubtitles (const QString& key, const KUrl& url) const;
  virtual bool getPlaylist (const QString& key, const KUrl& url) const;
  virtual bool getUseKioslave (const QString& key, const KUrl& url) const;
  /** Returns whether this media needs frequency. */
  virtual bool needsFrequency (void) const;

  /** Initializes the class. */
  static void initialize (void);
  /** Terminates the class. */
  static void terminate (void);

  /** Configuration properties. */
  KPlayerConfiguration* configuration (void) const
    { return KPlayerEngine::engine() -> configuration(); }

  // General properties

  bool fullScreen (void) const
    { return getBoolean ("Full Screen"); }
  void setFullScreen (bool full_screen)
    { setBoolean ("Full Screen", full_screen); }

  bool maximized (void) const
    { return getBoolean ("Maximized"); }
  void setMaximized (bool maximized)
    { setBoolean ("Maximized", maximized); }

  bool maintainAspect (void) const
    { return getBoolean ("Maintain Aspect"); }
  void setMaintainAspect (bool value)
    { return setBoolean ("Maintain Aspect", value); }

  // Subtitle properties

  bool subtitleAutoload (void) const
    { return autoloadSubtitles ("Autoload Subtitles"); }
  void setSubtitleAutoload (bool value)
    { return setBoolean ("Autoload Subtitles", value); }

  bool subtitleClosedCaption (void) const
    { return getBoolean ("Closed Caption"); }
  void setSubtitleClosedCaption (bool enable)
    { setBoolean ("Closed Caption", enable); }

  const QString& subtitleEncoding (void) const
    { return getString ("Subtitle Encoding"); }
  void setSubtitleEncoding (const QString& encoding)
    { set ("Subtitle Encoding", encoding); }
  bool hasSubtitleEncoding (void) const
    { return has ("Subtitle Encoding"); }
  void resetSubtitleEncoding (void)
    { reset ("Subtitle Encoding"); }

  int subtitlePosition (void) const
    { return getInteger ("Subtitle Position"); }
  void setSubtitlePosition (int value)
    { return setInteger ("Subtitle Position", limit (value, 0, 100)); }

  float subtitleDelay (void) const
    { return getFloat ("Subtitle Delay"); }
  void setSubtitleDelay (float value)
    { return setFloat ("Subtitle Delay", value); }

  // Audio properties

  int volume (void) const
    { return getRelative ("Volume"); }
  void setVolume (int volume)
    { setRelative ("Volume", volume); }
  void adjustVolume (int volume)
    { adjustRelative ("Volume", volume); }

  float audioDelay (void) const
    { return getFloat ("Audio Delay"); }
  void setAudioDelay (float value)
    { return setFloat ("Audio Delay", value); }

  const QString& audioDriver (void) const
    { return getString ("Audio Driver"); }
  const QString& audioDevice (void) const
    { return getString ("Audio Device"); }

  bool softwareVolume (void) const
    { return getBoolean ("Software Volume"); }
  int maximumSoftwareVolume (void) const
    { return getInteger ("Maximum Software Volume"); }

  const QString& mixerDevice (void) const
    { return getString ("Mixer Device"); }
  const QString& mixerChannel (void) const
    { return getString ("Mixer Channel"); }

  const QString& audioCodec (void) const
    { return getStringValue ("Audio Codec"); }
  void setAudioCodec (const QString& codec)
    { setComboValue ("Audio Codec", codec); }

  // Video properties

  int contrast (void) const
    { return getRelative ("Contrast"); }
  void setContrast (int contrast)
    { setRelative ("Contrast", contrast); }
  void adjustContrast (int contrast)
    { adjustRelative ("Contrast", contrast); }

  int brightness (void) const
    { return getRelative ("Brightness"); }
  void setBrightness (int brightness)
    { setRelative ("Brightness", brightness); }
  void adjustBrightness (int brightness)
    { adjustRelative ("Brightness", brightness); }

  int hue (void) const
    { return getRelative ("Hue"); }
  void setHue (int hue)
    { setRelative ("Hue", hue); }
  void adjustHue (int hue)
    { adjustRelative ("Hue", hue); }

  int saturation (void) const
    { return getRelative ("Saturation"); }
  void setSaturation (int saturation)
    { setRelative ("Saturation", saturation); }
  void adjustSaturation (int saturation)
    { adjustRelative ("Saturation", saturation); }

  const QString& videoDriver (void) const
    { return getString ("Video Driver"); }
  const QString& videoDevice (void) const
    { return getString ("Video Device"); }

  const QString& videoCodec (void) const
    { return getStringValue ("Video Codec"); }
  void setVideoCodec (const QString& codec)
    { setComboValue ("Video Codec", codec); }

  int videoScaler (void) const
    { return getInteger ("Video Scaler"); }
  void setVideoScaler (int value)
    { setInteger ("Video Scaler", value); }

  bool videoDoubleBuffering (void) const
    { return getBoolean ("Double Buffering"); }
  void setVideoDoubleBuffering (bool value)
    { setBoolean ("Double Buffering", value); }

  bool videoDirectRendering (void) const
    { return getBoolean ("Direct Rendering"); }
  void setVideoDirectRendering (bool value)
    { setBoolean ("Direct Rendering", value); }

  // Advanced properties

  const QString& executablePath (void) const
    { return getString ("Executable Path"); }
  void setExecutablePath (const QString& path)
    { setString ("Executable Path", path); }

  QString commandLine (void) const
    { return getAppendable ("Command Line"); }
  void setCommandLine (const QString& line)
    { setString ("Command Line", line); }
  //const QStringList& commandLineHistory (void) const
  //  { return getHistory ("Command Line"); }

  const QString& demuxer (void) const
    { return getStringValue ("Demuxer"); }
  void setDemuxer (const QString& codec)
    { setComboValue ("Demuxer", codec); }

  int frameDrop (void) const
    { return getInteger ("Frame Dropping"); }
  void setFrameDrop (int frame_drop)
    { setInteger ("Frame Dropping", frame_drop); }

  int cache (void) const
    { return getCache ("Cache"); }
  int cacheSize (void) const
    { return getCacheSize ("Cache"); }
  void setCache (int cache, int size);
  void setCacheOption (int cache, int size);

  int buildNewIndex (void) const
    { return getInteger ("Build New Index"); }
  void setBuildNewIndex (int build)
    { setInteger ("Build New Index", build); }

  bool useTemporaryFile (void) const
    { return getBoolean ("Use Temporary File For KIOSlave"); }
  void setUseTemporaryFile (bool value)
    { setBoolean ("Use Temporary File For KIOSlave", value); }

  int osdLevel (void) const
    { return getInteger ("OSD Level"); }
  void setOsdLevel (int level)
    { setInteger ("OSD Level", level); }

protected:
  /** Properties, including meta information. */
  KPlayerPropertyMap m_properties;
  /** Previous properties. */
  KPlayerPropertyMap m_previous;
  /** Properties that have been added. */
  KPlayerPropertyCounts m_added;
  /** Properties that have been changed. */
  KPlayerPropertyCounts m_changed;
  /** Properties that have been removed. */
  KPlayerPropertyCounts m_removed;

  KConfigGroup m_config_group;

  /** Information about configuration properties. */
  static KPlayerPropertyInfoMap m_info;

  /** Information about meta properties. */
  static KPlayerStringPropertyInfo m_meta_info;
  /** List of default meta attributes. */
  static QStringList m_meta_attributes;
  /** Null URL. */
  static const KUrl nullUrl;
  /** Null size. */
  static const QSize nullSize;
  /** Null string. */
  static const QString nullString;
  /** Null string list. */
  static const QStringList nullStringList;
  /** Null integer string map. */
  static const QMap<int, QString> nullIntegerStringMap;

  void load (void);
  void save (void);
  void update (void);
  void cleanup (void);

  void beginUpdate (void);
  void updated (const QString& key);

signals:
  /** Emitted when the properties are updated. */
  void updated (void);
};

/** The KPlayer configuration.
  * @author kiriuja
  */
class KPlayerConfiguration : public KPlayerProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerConfiguration (void);
  /** Destructor. */
  virtual ~KPlayerConfiguration();

  /** Returns the config storage. */
  virtual KConfig* config (void) const;
  /** Returns the config group. */
  virtual QString configGroupName (void) const;

  virtual bool getBoolean (const QString& key) const;
  virtual void setBoolean (const QString& key, bool value);
  virtual int getInteger (const QString& key) const;
  virtual void setInteger (const QString& key, int value);
  virtual int getRelativeValue (const QString& key) const;
  virtual void setRelative (const QString& key, int value);
  virtual void adjustRelative (const QString& key, int value);
  virtual int getCache (const QString& key) const;
  virtual int getCacheSize (const QString& key) const;
  virtual float getFloat (const QString& key) const;
  virtual void setFloat (const QString& key, float value);
  virtual const QString& getString (const QString& key) const;
  virtual const QString& getComboString (const QString& key) const;
  virtual QString getAppendable (const QString& key) const;
  virtual bool autoloadSubtitles (const QString& key) const;
  virtual bool getVobsubSubtitles (const QString& key, const KUrl& url) const;
  virtual bool getPlaylist (const QString& key, const KUrl& url) const;
  virtual bool getUseKioslave (const QString& key, const KUrl& url) const;

  /** Returns the override flag for the given key. */
  bool override (const QString& key)
    { return info (key) -> override(); }
  /** Sets the override flag for the given key. */
  void setOverride (const QString& key, bool override)
    { info (key) -> setOverride (override); }

  /** Resets configuration for a new item. */
  void itemReset (void);

  // General configuration

  bool resizeAutomatically (void) const
    { return getBoolean ("Resize Main Window Automatically"); }
  void setResizeAutomatically (bool resize)
    { setBoolean ("Resize Main Window Automatically", resize); }

  int preferredVideoWidth (void) const
    { return getInteger ("Minimum Initial Width"); }
  void setPreferredVideoWidth (int width)
    { setInteger ("Minimum Initial Width", limit (width, 200)); }

  QPoint preferredWindowPosition (void) const
    { return QPoint (getInteger ("Position X"), getInteger ("Position Y")); }
  void setPreferredWindowPosition (const QPoint& position)
    { setInteger ("Position X", position.x()); setInteger ("Position Y", position.y()); }

  int recentMenuSize (void) const
    { return getInteger ("Recent File List Size"); }
  void setRecentMenuSize (int size)
    { setInteger ("Recent File List Size", limit (size, 0)); }

  int recentListSize (void) const
    { return getInteger ("Playlist Size Limit"); }
  void setRecentListSize (int size)
    { setInteger ("Playlist Size Limit", limit (size, 0)); }

  // Playlist configuration

  /** Returns whether the playlist loops. */
  bool loop (void) const
    { return getBoolean ("Playlist Loop"); }
  /** Sets the playlist loop option. */
  void setLoop (bool loop)
    { setBoolean ("Playlist Loop", loop); }

  /** Returns whether the playlist is shuffled. */
  bool shuffle (void) const
    { return getBoolean ("Playlist Shuffle"); }
  /** Sets the playlist shuffle option and shuffles the playlist if the option is on. */
  void setShuffle (bool shuffle)
    { setBoolean ("Playlist Shuffle", shuffle); }

  bool allowDuplicateEntries (void) const
    { return getBoolean ("Allow Duplicate Entries"); }
  void setAllowDuplicateEntries (bool allow)
    { setBoolean ("Allow Duplicate Entries", allow); }

  int playlistMenuSize (void) const
    { return getInteger ("Playlist Menu Size"); }
  void setPlaylistMenuSize (int size)
    { setInteger ("Playlist Menu Size", limit (size, 0)); }

  int cacheSizeLimit (void) const
    { return getInteger ("Cache Size Limit"); }
  void setCacheSizeLimit (int size)
    { setInteger ("Cache Size Limit", limit (size, 10, 1000000)); }

  // Control configuration

  bool rememberWithShift (void) const
    { return getBoolean ("Remember With Shift"); }
  void setRememberWithShift (bool remember)
    { setBoolean ("Remember With Shift", remember); }

  bool rememberSize (void) const
    { return getBoolean ("Remember Size"); }
  bool rememberSize (bool shift) const
    { return rememberSize() || (shift && rememberWithShift()); }
  void setRememberSize (bool remember)
    { setBoolean ("Remember Size", remember); }

  bool rememberAspect (void) const
    { return getBoolean ("Remember Aspect"); }
  bool rememberAspect (bool shift) const
    { return rememberAspect() || (shift && rememberWithShift()); }
  void setRememberAspect (bool remember)
    { setBoolean ("Remember Aspect", remember); }

  bool rememberFullScreen (void) const
    { return getBoolean ("Remember Full Screen"); }
  bool rememberFullScreen (bool shift) const
    { return rememberFullScreen() || (shift && rememberWithShift()); }
  void setRememberFullScreen (bool remember)
    { setBoolean ("Remember Full Screen", remember); }

  bool rememberMaximized (void) const
    { return getBoolean ("Remember Maximized"); }
  bool rememberMaximized (bool shift) const
    { return rememberMaximized() || (shift && rememberWithShift()); }
  void setRememberMaximized (bool remember)
    { setBoolean ("Remember Maximized", remember); }

  bool rememberMaintainAspect (void) const
    { return getBoolean ("Remember Maintain Aspect"); }
  bool rememberMaintainAspect (bool shift) const
    { return rememberMaintainAspect() || (shift && rememberWithShift()); }
  void setRememberMaintainAspect (bool remember)
    { setBoolean ("Remember Maintain Aspect", remember); }

  bool rememberVolume (void) const
    { return getBoolean ("Remember Volume"); }
  bool rememberVolume (bool shift) const
    { return rememberVolume() || (shift && rememberWithShift()); }
  void setRememberVolume (bool remember)
    { setBoolean ("Remember Volume", remember); }

  bool rememberAudioDelay (void) const
    { return getBoolean ("Remember Audio Delay"); }
  bool rememberAudioDelay (bool shift) const
    { return rememberAudioDelay() || (shift && rememberWithShift()); }
  void setRememberAudioDelay (bool remember)
    { setBoolean ("Remember Audio Delay", remember); }

  bool rememberFrameDrop (void) const
    { return getBoolean ("Remember Frame Drop"); }
  bool rememberFrameDrop (bool shift) const
    { return rememberFrameDrop() || (shift && rememberWithShift()); }
  void setRememberFrameDrop (bool remember)
    { setBoolean ("Remember Frame Drop", remember); }

  bool rememberContrast (void) const
    { return getBoolean ("Remember Contrast"); }
  bool rememberContrast (bool shift) const
    { return rememberContrast() || (shift && rememberWithShift()); }
  void setRememberContrast (bool remember)
    { setBoolean ("Remember Contrast", remember); }

  bool rememberBrightness (void) const
    { return getBoolean ("Remember Brightness"); }
  bool rememberBrightness (bool shift) const
    { return rememberBrightness() || (shift && rememberWithShift()); }
  void setRememberBrightness (bool remember)
    { setBoolean ("Remember Brightness", remember); }

  bool rememberHue (void) const
    { return getBoolean ("Remember Hue"); }
  bool rememberHue (bool shift) const
    { return rememberHue() || (shift && rememberWithShift()); }
  void setRememberHue (bool remember)
    { setBoolean ("Remember Hue", remember); }

  bool rememberSaturation (void) const
    { return getBoolean ("Remember Saturation"); }
  bool rememberSaturation (bool shift) const
    { return rememberSaturation() || (shift && rememberWithShift()); }
  void setRememberSaturation (bool remember)
    { setBoolean ("Remember Saturation", remember); }

  bool rememberSubtitlePosition (void) const
    { return getBoolean ("Remember Subtitle Position"); }
  bool rememberSubtitlePosition (bool shift) const
    { return rememberSubtitlePosition() || (shift && rememberWithShift()); }
  void setRememberSubtitlePosition (bool remember)
    { setBoolean ("Remember Subtitle Position", remember); }

  bool rememberSubtitleDelay (void) const
    { return getBoolean ("Remember Subtitle Delay"); }
  bool rememberSubtitleDelay (bool shift) const
    { return rememberSubtitleDelay() || (shift && rememberWithShift()); }
  void setRememberSubtitleDelay (bool remember)
    { setBoolean ("Remember Subtitle Delay", remember); }

  // Volume configuration

  int volumeMinimum (void) const
    { return getInteger ("Volume Minimum"); }
  void setVolumeMinimum (int volume)
    { setInteger ("Volume Minimum", limit (volume, 0, 100)); }

  int volumeMaximum (void) const
    { return getInteger ("Volume Maximum"); }
  void setVolumeMaximum (int volume)
    { setInteger ("Volume Maximum", limit (volume, 0, 100)); }

  void setVolumeMinimumMaximum (int minimum, int maximum)
  {
    limit4 (minimum, maximum, 0, 100);
    setVolumeMinimum (minimum);
    setVolumeMaximum (maximum);
    //setVolume (limit (volume(), volumeMinimum(), volumeMaximum()));
  }

  int volumeStep (void) const
    { return getInteger ("Volume Step"); }
  void setVolumeStep (int volume)
    { setInteger ("Volume Step", limit (volume, 1, volumeMaximum() - volumeMinimum())); }

  bool volumeReset (void) const
    { return getBoolean ("Volume Reset"); }
  void setVolumeReset (bool volume)
    { setBoolean ("Volume Reset", volume); }

  int volumeEvery (void) const
    { return getInteger ("Volume Every"); }
  void setVolumeEvery (int volume)
    { setInteger ("Volume Every", limit (volume, 0, 1)); }
  bool resetVolumeEveryFile (void) const
    { return volumeReset() && volumeEvery() == 0; }

  int initialVolume (void) const
    { return getInteger ("Volume Default"); }
  void setInitialVolume (int volume)
    { setInteger ("Volume Default", limit (volume, volumeMinimum(), volumeMaximum())); }

  // Contrast configuration

  int contrastMinimum (void) const
    { return getInteger ("Contrast Minimum"); }
  void setContrastMinimum (int contrast)
    { setInteger ("Contrast Minimum", limit (contrast, -100, 100)); }

  int contrastMaximum (void) const
    { return getInteger ("Contrast Maximum"); }
  void setContrastMaximum (int contrast)
    { setInteger ("Contrast Maximum", limit (contrast, -100, 100)); }

  void setContrastMinimumMaximum (int minimum, int maximum)
  {
    limit4 (minimum, maximum, -100, 100);
    setContrastMinimum (minimum);
    setContrastMaximum (maximum);
    //setContrast (limit (contrast(), contrastMinimum(), contrastMaximum()));
  }

  int contrastStep (void) const
    { return getInteger ("Contrast Step"); }
  void setContrastStep (int contrast)
    { setInteger ("Contrast Step", limit (contrast, 1, contrastMaximum() - contrastMinimum())); }

  bool contrastReset (void) const
    { return getBoolean ("Contrast Reset"); }
  void setContrastReset (bool contrast)
    { setBoolean ("Contrast Reset", contrast); }

  int contrastEvery (void) const
    { return getInteger ("Contrast Every"); }
  void setContrastEvery (int contrast)
    { setInteger ("Contrast Every", limit (contrast, 0, 1)); }
  bool resetContrastEveryFile (void) const
    { return contrastReset() && contrastEvery() == 0; }

  int initialContrast (void) const
    { return getInteger ("Contrast Default"); }
  void setInitialContrast (int contrast)
    { setInteger ("Contrast Default", limit (contrast, contrastMinimum(), contrastMaximum())); }

  // Brightness configuration

  int brightnessMinimum (void) const
    { return getInteger ("Brightness Minimum"); }
  void setBrightnessMinimum (int brightness)
    { setInteger ("Brightness Minimum", limit (brightness, -50, 50)); }

  int brightnessMaximum (void) const
    { return getInteger ("Brightness Maximum"); }
  void setBrightnessMaximum (int brightness)
    { setInteger ("Brightness Maximum", limit (brightness, -50, 50)); }

  void setBrightnessMinimumMaximum (int minimum, int maximum)
  {
    limit4 (minimum, maximum, -50, 50);
    setBrightnessMinimum (minimum);
    setBrightnessMaximum (maximum);
    //setBrightness (limit (brightness(), brightnessMinimum(), brightnessMaximum()));
  }

  int brightnessStep (void) const
    { return getInteger ("Brightness Step"); }
  void setBrightnessStep (int brightness)
    { setInteger ("Brightness Step", limit (brightness, 1, brightnessMaximum() - brightnessMinimum())); }

  bool brightnessReset (void) const
    { return getBoolean ("Brightness Reset"); }
  void setBrightnessReset (bool brightness)
    { setBoolean ("Brightness Reset", brightness); }

  int brightnessEvery (void) const
    { return getInteger ("Brightness Every"); }
  void setBrightnessEvery (int brightness)
    { setInteger ("Brightness Every", limit (brightness, 0, 1)); }
  bool resetBrightnessEveryFile (void) const
    { return brightnessReset() && brightnessEvery() == 0; }

  int initialBrightness (void) const
    { return getInteger ("Brightness Default"); }
  void setInitialBrightness (int brightness)
    { setInteger ("Brightness Default", limit (brightness, brightnessMinimum(), brightnessMaximum())); }

  // Hue configuration

  int hueMinimum (void) const
    { return getInteger ("Hue Minimum"); }
  void setHueMinimum (int hue)
    { setInteger ("Hue Minimum", limit (hue, -100, 100)); }

  int hueMaximum (void) const
    { return getInteger ("Hue Maximum"); }
  void setHueMaximum (int hue)
    { setInteger ("Hue Maximum", limit (hue, -100, 100)); }

  void setHueMinimumMaximum (int minimum, int maximum)
  {
    limit4 (minimum, maximum, -100, 100);
    setHueMinimum (minimum);
    setHueMaximum (maximum);
    //setHue (limit (hue(), hueMinimum(), hueMaximum()));
  }

  int hueStep (void) const
    { return getInteger ("Hue Step"); }
  void setHueStep (int hue)
    { setInteger ("Hue Step", limit (hue, 1, hueMaximum() - hueMinimum())); }

  bool hueReset (void) const
    { return getBoolean ("Hue Reset"); }
  void setHueReset (bool hue)
    { setBoolean ("Hue Reset", hue); }

  int hueEvery (void) const
    { return getInteger ("Hue Every"); }
  void setHueEvery (int hue)
    { setInteger ("Hue Every", limit (hue, 0, 1)); }
  bool resetHueEveryFile (void) const
    { return hueReset() && hueEvery() == 0; }

  int initialHue (void) const
    { return getInteger ("Hue Default"); }
  void setInitialHue (int hue)
    { setInteger ("Hue Default", limit (hue, hueMinimum(), hueMaximum())); }

  // Saturation configuration

  int saturationMinimum (void) const
    { return getInteger ("Saturation Minimum"); }
  void setSaturationMinimum (int saturation)
    { setInteger ("Saturation Minimum", limit (saturation, -100, 100)); }

  int saturationMaximum (void) const
    { return getInteger ("Saturation Maximum"); }
  void setSaturationMaximum (int saturation)
    { setInteger ("Saturation Maximum", limit (saturation, -100, 100)); }

  void setSaturationMinimumMaximum (int minimum, int maximum)
  {
    limit4 (minimum, maximum, -100, 100);
    setSaturationMinimum (minimum);
    setSaturationMaximum (maximum);
    //setSaturation (limit (saturation(), saturationMinimum(), saturationMaximum()));
  }

  int saturationStep (void) const
    { return getInteger ("Saturation Step"); }
  void setSaturationStep (int saturation)
    { setInteger ("Saturation Step", limit (saturation, 1, saturationMaximum() - saturationMinimum())); }

  bool saturationReset (void) const
    { return getBoolean ("Saturation Reset"); }
  void setSaturationReset (bool saturation)
    { setBoolean ("Saturation Reset", saturation); }

  int saturationEvery (void) const
    { return getInteger ("Saturation Every"); }
  void setSaturationEvery (int saturation)
    { setInteger ("Saturation Every", limit (saturation, 0, 1)); }
  bool resetSaturationEveryFile (void) const
    { return saturationReset() && saturationEvery() == 0; }

  int initialSaturation (void) const
    { return getInteger ("Saturation Default"); }
  void setInitialSaturation (int saturation)
    { setInteger ("Saturation Default", limit (saturation, saturationMinimum(), saturationMaximum())); }

  // Progress and seeking configuration

  int progressNormalSeek (void) const
    { return getInteger ("Normal Seek"); }
  void setProgressNormalSeek (int seek)
    { setInteger ("Normal Seek", limit (seek, 1,
        progressFastSeekUnits() == progressNormalSeekUnits() ? progressFastSeek() :
        progressNormalSeekUnits() ? INT_MAX : 100)); }

  int progressNormalSeekUnits (void) const
    { return getInteger ("Normal Seek Units"); }
  void setProgressNormalSeekUnits (int units)
    { setInteger ("Normal Seek Units", limit (units, 0, 1)); }

  int progressFastSeek (void) const
    { return getInteger ("Fast Seek"); }
  void setProgressFastSeek (int seek)
    { setInteger ("Fast Seek", limit (seek,
        progressFastSeekUnits() == progressNormalSeekUnits() ? progressNormalSeek() : 1,
        progressFastSeekUnits() ? INT_MAX : 100)); }

  int progressFastSeekUnits (void) const
    { return getInteger ("Fast Seek Units"); }
  void setProgressFastSeekUnits (int units)
    { setInteger ("Fast Seek Units", limit (units, 0, 1)); }

  // Slider configuration

  int preferredSliderLength (void) const
    { return getInteger ("Preferred Slider Length"); }
  void setPreferredSliderLength (int length)
    { setInteger ("Preferred Slider Length", limit (length, 50)); }

  int minimumSliderLength (void) const
    { return getInteger ("Minimum Slider Length"); }
  void setMinimumSliderLength (int length)
    { setInteger ("Minimum Slider Length", limit (length, 50)); }

  bool showSliderMarks (void) const
    { return getBoolean ("Show Slider Marks"); }
  void setShowSliderMarks (bool show)
    { setBoolean ("Show Slider Marks", show); }

  int sliderMarksInterval (int span) const;
  int sliderMarks (void) const
    { return getInteger ("Slider Marks"); }
  void setSliderMarks (int marks)
    { setInteger ("Slider Marks", limit (marks, 1, 100)); }

  // Message configuration

  bool showMessagesOnError (void) const
    { return getBoolean ("Show Messages On Error"); }
  void setShowMessagesOnError (bool show)
    { setBoolean ("Show Messages On Error", show); }

  // Audio configuration

  bool mute (void) const
    { return getBoolean ("Mute"); }
  void setMute (bool mute)
    { setBoolean ("Mute", mute); }

  float audioDelayStep (void) const
    { return getFloat ("Audio Delay Step"); }
  void setAudioDelayStep (float step)
    { setFloat ("Audio Delay Step", flimit (step, 0.01)); }
  void resetAudioDelayStep (void)
    { reset ("Audio Delay Step"); }

  void setAudioDriver (const QString& driver)
    { setString ("Audio Driver", driver); }
  void setAudioDevice (const QString& device)
    { setString ("Audio Device", device); }

  void setSoftwareVolume (bool value)
    { return setBoolean ("Software Volume", value); }
  void setMaximumSoftwareVolume (int maximum)
    { setInteger ("Maximum Software Volume", maximum); }

  void setMixerDevice (const QString& device)
    { setString ("Mixer Device", device); }
  bool hasMixerDevice (void) const
    { return has ("Mixer Device"); }
  void setMixerChannel (const QString& channel)
    { setString ("Mixer Channel", channel); }

  const QString& switchAudioDemuxers (void) const
    { return getString ("Switch Audio Demuxers"); }

  // Video configuration

  void setVideoDriver (const QString& driver)
    { setString ("Video Driver", driver); }
  void setVideoDevice (const QString& device)
    { setString ("Video Device", device); }

  // Subtitle configuration

  const QString& subtitleFontName (void) const
    { return getString ("Subtitle Font Name"); }
  void setSubtitleFontName (const QString& name)
    { setString ("Subtitle Font Name", name); }

  bool subtitleFontBold (void) const
    { return getBoolean ("Subtitle Font Bold"); }
  void setSubtitleFontBold (bool bold)
    { setBoolean ("Subtitle Font Bold", bold); }

  bool subtitleFontItalic (void) const
    { return getBoolean ("Subtitle Font Italic"); }
  void setSubtitleFontItalic (bool italic)
    { setBoolean ("Subtitle Font Italic", italic); }

  float subtitleTextSize (void) const
    { return getFloat ("Subtitle Text Size"); }
  void setSubtitleTextSize (float size)
    { setFloat ("Subtitle Text Size", flimit (size, 0.1, 100)); }
  void resetSubtitleTextSize (void)
    { reset ("Subtitle Text Size"); }

  bool subtitleAutoscale (void) const
    { return getBoolean ("Subtitle Autoscale"); }
  void setSubtitleAutoscale (bool scale)
    { setBoolean ("Subtitle Autoscale", scale); }

  float subtitleFontOutline (void) const
    { return getFloat ("Subtitle Font Outline"); }
  void setSubtitleFontOutline (float outline)
    { setFloat ("Subtitle Font Outline", flimit (outline, 0, 10)); }
  bool hasSubtitleFontOutline (void) const
    { return has ("Subtitle Font Outline"); }
  QString subtitleFontOutlineString (void) const
    { return asString ("Subtitle Font Outline"); }

  int subtitleTextWidth (void) const
    { return getInteger ("Subtitle Text Width"); }
  void setSubtitleTextWidth (int width)
    { setInteger ("Subtitle Text Width", limit (width, 10, 100)); }
  bool hasSubtitleTextWidth (void) const
    { return has ("Subtitle Text Width"); }
  QString subtitleTextWidthString (void) const
    { return asString ("Subtitle Text Width"); }

  int subtitlePositionStep (void) const
    { return getInteger ("Subtitle Position Step"); }
  void setSubtitlePositionStep (int step)
    { setInteger ("Subtitle Position Step", limit (step, 1)); }

  float subtitleDelayStep (void) const
    { return getFloat ("Subtitle Delay Step"); }
  void setSubtitleDelayStep (float step)
    { setFloat ("Subtitle Delay Step", flimit (step, 0.01)); }
  void resetSubtitleDelayStep (void)
    { reset ("Subtitle Delay Step"); }

  int subtitleAutoexpand (void) const
    { return getInteger ("Subtitle Autoexpand"); }
  void setSubtitleAutoexpand (int expand)
    { setInteger ("Subtitle Autoexpand", limit (expand, 0, 3)); }
  void resetSubtitleAutoexpand (void)
    { reset ("Subtitle Autoexpand"); }
  bool hasSubtitleAutoexpand (void) const
    { return has ("Subtitle Autoexpand"); }
  QSize autoexpandAspect (void) const;

  const QString& autoloadExtensionList (void) const
    { return getString ("Autoload Extension List"); }
  void setAutoloadExtensionList (const QString& autoload)
    { setString ("Autoload Extension List", autoload); }

  QStringList subtitleExtensions (void) const;

  // Advanced configuration

  bool useKioslaveForHttp (void) const
    { return getBoolean ("Use KIOSlave For HTTP"); }
  void setUseKioslaveForHttp (bool use)
    { setBoolean ("Use KIOSlave For HTTP", use); }

  bool useKioslaveForFtp (void) const
    { return getBoolean ("Use KIOSlave For FTP"); }
  void setUseKioslaveForFtp (bool use)
    { setBoolean ("Use KIOSlave For FTP", use); }

  bool useKioslaveForSmb (void) const
    { return getBoolean ("Use KIOSlave For SMB"); }
  void setUseKioslaveForSmb (bool use)
    { setBoolean ("Use KIOSlave For SMB", use); }
};

/** The KPlayer media.
  * @author kiriuja
  */
class KPlayerMedia : public KPlayerProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerMedia (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerMedia();

  /** Parent properties. */
  KPlayerProperties* parent (void) const
    { return m_parent; }
  /** Sets the parent. */
  void setParent (KPlayerMedia* media);

  const KUrl& url (void) const
    { return m_url; }

  /** Increments reference count. */
  void reference (void)
    { ++ m_references; }
  /** Returns reference count. */
  int references (void)
    { return m_references; }

  /** Returns the config group. */
  virtual QString configGroupName (void) const;

  virtual bool getBoolean (const QString& key) const;
  virtual void setBoolean (const QString& key, bool value);
  virtual int getInteger (const QString& key) const;
  virtual void setInteger (const QString& key, int value);
  virtual int getRelativeValue (const QString& key) const;
  virtual void setRelative (const QString& key, int value);
  virtual void adjustRelative (const QString& key, int value);
  virtual int getCache (const QString& key) const;
  virtual int getCacheSize (const QString& key) const;
  virtual float getFloat (const QString& key) const;
  virtual void setFloat (const QString& key, float value);
  virtual const QString& getString (const QString& key) const;
  virtual const QString& getComboString (const QString& key) const;
  virtual QString getAppendable (const QString& key) const;

  /** Creates generic properties for the given URL based on configuration if needed and adds a reference. */
  static KPlayerGenericProperties* genericProperties (const KUrl& url);
  /** Creates device properties for the given URL based on configuration if needed and adds a reference. */
  static KPlayerDeviceProperties* deviceProperties (const KUrl& url);
  /** Creates disk properties for the given URL based on configuration if needed and adds a reference. */
  static KPlayerDiskProperties* diskProperties (const KUrl& url);
  /** Creates disk properties for the given URL if needed and adds a reference. */
  static KPlayerDiskProperties* diskProperties (KPlayerDeviceProperties* parent, const KUrl& url);
  /** Creates TV device properties for the given URL based on configuration if needed and adds a reference. */
  static KPlayerTVProperties* tvProperties (const KUrl& url);
  /** Creates DVB device properties for the given URL based on configuration if needed and adds a reference. */
  static KPlayerDVBProperties* dvbProperties (const KUrl& url);
  /** Creates track properties for the given URL if needed and adds a reference. */
  static KPlayerTrackProperties* trackProperties (const KUrl& url);
  /** Adds a reference and returns the media for the given URL if it is already loaded. */
  static KPlayerMedia* reference (const QString& urls);
  /** Releases the given media properties and removes them from the map when no references remain. */
  static void release (KPlayerMedia* media);

protected:
  /** Parent properties. */
  KPlayerProperties* m_parent;
  /** Media URL. */
  KUrl m_url;
  /** Reference count. */
  int m_references;

  /** Media map by URL string. */
  static KPlayerMediaMap m_media_map;

  /** Decrements reference count and deletes the object when no references remain. */
  bool release (void);
};

/** The KPlayer generic properties.
  * @author kiriuja
  */
class KPlayerGenericProperties : public KPlayerMedia
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerGenericProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerGenericProperties();

  /** Returns the config storage. */
  virtual KConfig* config (void) const;

  QString name (void) const
    { return asString ("Name"); }
  void setName (const QString& name)
    { set ("Name", name); }
  bool hasName (void) const
    { return ! getString ("Name").isEmpty(); }
  //const QStringList& nameHistory (void) const
  //  { return getHistory ("Name"); }

  QString defaultName (void) const;
  void setDefaultName (const QString& name)
    { m_default_name = name; }

  const QString& temporaryName (void) const
    { return m_temporary_name; }
  void setTemporaryName (const QString& name)
    { m_temporary_name = name; }
  QString currentName (void) const
    { return temporaryName().isEmpty() ? name() : temporaryName(); }

  QString caption (void) const;

  /** Returns icon name. */
  virtual QString icon (void) const;
  void setIcon (const QString& icon)
    { setString ("Icon", icon); }
  bool hasIcon (void) const
    { return has ("Icon"); }

  const QStringList& children (void) const
    { return getStringList ("Children"); }
  void setChildren (const QStringList& children)
    { setStringList ("Children", children); }
  void resetChildren (void)
    { reset ("Children"); }
  bool hasChildren (void) const
    { return has ("Children"); }

  const KUrl& origin (void) const
    { return getUrl ("Origin"); }
  void setOrigin (const KUrl& url)
    { setUrl ("Origin", url); }
  void resetOrigin (void)
    { reset ("Origin"); }

  bool customOrder (void) const
    { return getBoolean ("Custom Order"); }
  void setCustomOrder (bool custom)
    { setBooleanOption ("Custom Order", custom ? 1 : 2); }
  void resetCustomOrder (void)
    { reset ("Custom Order"); }
  bool hasCustomOrder (void) const
    { return has ("Custom Order"); }

  const QString& groupingKey (void) const
    { return getString ("Group By"); }
  void setGroupingKey (const QString& key)
    { setString ("Group By", key); }

  /** Returns the type property for the given ID. */
  QString type (const QString& id) const;

  /** Returns the MSF property for the given ID. */
  float msf (const QString& id) const;

  /** Returns the hidden property for the given ID. */
  bool hidden (const QString& id) const;
  /** Sets the hidden property for the given ID. */
  void setHidden (const QString& id, bool hidden);

protected:
  /** Default name. */
  QString m_default_name;
  /** Temporary name. */
  QString m_temporary_name;
};

/** The KPlayer media properties.
  * @author kiriuja
  */
class KPlayerMediaProperties : public KPlayerGenericProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerMediaProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerMediaProperties();

  // General properties

  const KUrl& path (void) const
    { return getUrl ("Path"); }
  void setPath (const KUrl& url)
    { setUrl ("Path", url); }
  bool hasPath (void) const
    { return has ("Path"); }
  QString pathString (void) const
    { return asString ("Path"); }

  const QString& type (void) const
    { return getString ("Type"); }
  void setType (const QString& type)
    { setString ("Type", type); }
  void resetType (void)
    { return reset ("Type"); }
  bool hasType (void) const
    { return has ("Type"); }
  /** Translated type. */
  QString typeString (void) const
    { return asString ("Type"); }

  QSize displaySize (void) const
    { return getDisplaySize ("Display Size"); }
  int displaySizeOption (void) const
    { return getSizeOption ("Display Size"); }
  bool hasDisplaySize (void) const
    { return has ("Display Size"); }
  virtual void setDisplaySize (const QSize& size, int option);
  void resetDisplaySize (void)
    { reset ("Display Size"); }
  /** Display size as string. */
  QString displaySizeString (void) const
    { return asString ("Display Size"); }
  /** Display width as string. */
  QString displayWidthString (void) const
    { return hasDisplaySize() ? QString::number (getSize ("Display Size").width()) : QString::null; }
  /** Display height as string. */
  QString displayHeightString (void) const
    { return hasDisplaySize() ? QString::number (getSize ("Display Size").height()) : QString::null; }

  const QString& channelList (void) const
    { return getString ("Channel List"); }
  bool hasChannelList (void) const
    { return has ("Channel List"); }

  const QString& inputDriver (void) const
    { return getString ("Input Driver"); }

  // Size properties

  int fullScreenOption (void) const
    { return getBooleanOption ("Full Screen"); }
  void setFullScreenOption (int full_screen)
    { setBooleanOption ("Full Screen", full_screen); }
  bool hasFullScreen (void) const
    { return has ("Full Screen"); }

  int maximizedOption (void) const
    { return getBooleanOption ("Maximized"); }
  void setMaximizedOption (int maximized)
    { setBooleanOption ("Maximized", maximized); }

  int maintainAspectOption (void) const
    { return getBooleanOption ("Maintain Aspect"); }
  void setMaintainAspectOption (int value)
    { return setBooleanOption ("Maintain Aspect", value); }

  // Subtitle properties

  void setSubtitlePositionValue (int value)
    { return set ("Subtitle Position", limit (value, 0, 100)); }
  QString subtitlePositionString (void) const
    { return asString ("Subtitle Position"); }
  bool hasSubtitlePosition (void) const
    { return has ("Subtitle Position"); }
  void resetSubtitlePosition (void)
    { reset ("Subtitle Position"); }

  void setSubtitleDelayValue (float value)
    { return set ("Subtitle Delay", value); }
  QString subtitleDelayString (void) const
    { return asString ("Subtitle Delay"); }
  bool hasSubtitleDelay (void) const
    { return has ("Subtitle Delay"); }
  void resetSubtitleDelay (void)
    { reset ("Subtitle Delay"); }

  int subtitleClosedCaptionOption (void) const
    { return getBooleanOption ("Closed Caption"); }
  void setSubtitleClosedCaptionOption (int value)
    { return setBooleanOption ("Closed Caption", value); }

  // Audio properties

  int volumeOption (void) const
    { return getRelativeOption ("Volume"); }
  void setVolumeOption (int volume, int option)
    { setRelativeOption ("Volume", volume, option); }
  QString volumeString (void) const
    { return asIntegerString ("Volume"); }
  void resetVolume (void)
    { reset ("Volume"); }

  void setAudioDelayValue (float value)
    { return set ("Audio Delay", value); }
  QString audioDelayString (void) const
    { return asString ("Audio Delay"); }
  bool hasAudioDelay (void) const
    { return has ("Audio Delay"); }
  void resetAudioDelay (void)
    { reset ("Audio Delay"); }

  const QString& audioCodecValue (void) const
    { return getString ("Audio Codec"); }
  const QString& audioCodecOption (void) const
    { return getStringOption ("Audio Codec"); }
  void setAudioCodecOption (const QString& codec)
    { setStringOption ("Audio Codec", codec); }

  QString audioDriverString (void) const;
  QString mixerChannelString (void) const;
  QString audioCodecString (void) const;

  int audioMode (void) const
    { return getInteger ("Audio Mode"); }
  QString audioModeString (void) const
    { return asString ("Audio Mode"); }
  bool hasAudioMode (void) const
    { return has ("Audio Mode"); }

  int audioInput (void) const
    { return getInteger ("Audio Input"); }
  QString audioInputString (void) const
    { return asString ("Audio Input"); }
  bool hasAudioInput (void) const
    { return has ("Audio Input"); }

  bool immediateMode (void) const
    { return getBoolean ("Immediate Mode"); }
  bool alsaCapture (void) const
    { return getBoolean ("ALSA Capture"); }

  const QString& captureDevice (void) const
    { return getString ("Capture Device"); }
  bool hasCaptureDevice (void) const
    { return has ("Capture Device"); }

  // Video properties

  int contrastOption (void) const
    { return getRelativeOption ("Contrast"); }
  void setContrastOption (int contrast, int option)
    { setRelativeOption ("Contrast", contrast, option); }
  QString contrastString (void) const
    { return asIntegerString ("Contrast"); }
  void resetContrast (void)
    { reset ("Contrast"); }

  int brightnessOption (void) const
    { return getRelativeOption ("Brightness"); }
  void setBrightnessOption (int brightness, int option)
    { setRelativeOption ("Brightness", brightness, option); }
  QString brightnessString (void) const
    { return asIntegerString ("Brightness"); }
  void resetBrightness (void)
    { reset ("Brightness"); }

  int hueOption (void) const
    { return getRelativeOption ("Hue"); }
  void setHueOption (int hue, int option)
    { setRelativeOption ("Hue", hue, option); }
  QString hueString (void) const
    { return asIntegerString ("Hue"); }
  void resetHue (void)
    { reset ("Hue"); }

  int saturationOption (void) const
    { return getRelativeOption ("Saturation"); }
  void setSaturationOption (int saturation, int option)
    { setRelativeOption ("Saturation", saturation, option); }
  QString saturationString (void) const
    { return asIntegerString ("Saturation"); }
  void resetSaturation (void)
    { reset ("Saturation"); }

  const QString& videoCodecValue (void) const
    { return getString ("Video Codec"); }
  const QString& videoCodecOption (void) const
    { return getStringOption ("Video Codec"); }
  void setVideoCodecOption (const QString& codec)
    { setStringOption ("Video Codec", codec); }

  QString videoDriverString (void) const;
  QString videoCodecString (void) const;

  int videoInput (void) const
    { return getInteger ("Video Input"); }
  QString videoInputString (void) const
    { return asString ("Video Input"); }
  bool hasVideoInput (void) const
    { return has ("Video Input"); }

  const QString& videoFormat (void) const
    { return getString ("Video Format"); }
  bool hasVideoFormat (void) const
    { return has ("Video Format"); }

  int videoNorm (void) const
    { return getInteger ("Video Norm"); }
  QString videoNormString (void) const
    { return asString ("Video Norm"); }
  bool hasVideoNorm (void) const
    { return has ("Video Norm"); }

  // Advanced properties

  bool hasExecutablePath (void) const
    { return has ("Executable Path"); }

  int commandLineOption (void) const
    { return getAppendableOption ("Command Line"); }
  const QString& commandLineValue (void) const
    { return getStringValue ("Command Line"); }
  void setCommandLineOption (const QString& value, int option)
    { return setAppendable ("Command Line", value, option); }
  bool hasCommandLine (void) const
    { return has ("Command Line"); }

  const QString& demuxerValue (void) const
    { return getString ("Demuxer"); }
  const QString& demuxerOption (void) const
    { return getStringOption ("Demuxer"); }
  void setDemuxerOption (const QString& codec)
    { setStringOption ("Demuxer", codec); }
  QString demuxerString (void) const;

  int frameDropOption (void) const
    { return getIntegerOption ("Frame Dropping"); }
  void setFrameDropOption (int frame_drop)
    { setIntegerOption ("Frame Dropping", frame_drop); }

  int cacheOption (void) const
    { return getCacheOption ("Cache"); }
  QString cacheSizeString (void) const
    { return asString ("Cache"); }

  int buildNewIndexOption (void) const
    { return getIntegerOption ("Build New Index"); }
  void setBuildNewIndexOption (int build)
    { setIntegerOption ("Build New Index", build); }

  int osdLevelOption (void) const
    { return getIntegerOption ("OSD Level"); }
  void setOsdLevelOption (int level)
    { setIntegerOption ("OSD Level", level); }

  int mjpegDecimation (void) const
    { return getInteger ("Decimation"); }
  QString mjpegDecimationString (void) const
    { return asString ("Decimation"); }
  bool hasMjpegDecimation (void) const
    { return has ("Decimation"); }

  int mjpegQuality (void) const
    { return getInteger ("Compression"); }
  QString mjpegQualityString (void) const
    { return asString ("Compression"); }
  bool hasMjpegQuality (void) const
    { return has ("Compression"); }
};

/** The KPlayer device properties.
  * @author kiriuja
  */
class KPlayerDeviceProperties : public KPlayerMediaProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDeviceProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerDeviceProperties();

  /** Initializes property information. */
  virtual void setupInfo (void);

  int digits (void) const
    { return m_digits; }

protected:
  /** Number of digits in track or channel names. */
  int m_digits;
};

/** The KPlayer tuner device properties.
  * @author kiriuja
  */
class KPlayerTunerProperties : public KPlayerDeviceProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerTunerProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerTunerProperties();

  /** Returns the list of channels. */
  virtual QStringList channels (void) = 0;
  /** Returns the frequency of the given channel. */
  int channelFrequency (const QString& id) const;

  void setVideoInput (int input)
    { setInteger ("Video Input", input); }
  void resetVideoInput (void)
    { reset ("Video Input"); }

  void setAudioInput (int input)
    { setInteger ("Audio Input", input); }
  void resetAudioInput (void)
    { reset ("Audio Input"); }

  void setChannelList (const QString& list)
    { setString ("Channel List", list); }

protected:
  /** Default frequencies. */
  QMap<QString, int> m_frequencies;
};

struct KPlayerChannelGroup
{
  /** Number of the first channel. */
  int first_channel;
  /** Number of channels. */
  uint channels;
  /** Number of digits. */
  uint digits;
  /** Name prefix. */
  const char* prefix;
  /** Frequency of the first channel. */
  int first_freq;
  /** Frequency step. */
  int freq_step;
};

struct KPlayerChannelList
{
  /** List ID. */
  const char* id;
  /** List name. */
  const char* name;
  /** Channel groups. */
  struct KPlayerChannelGroup* groups;
  /** Group count. */
  uint groupcount;
};

extern struct KPlayerChannelList channellists[];
extern const uint channellistcount;

/** The KPlayer TV device properties.
  * @author kiriuja
  */
class KPlayerTVProperties : public KPlayerTunerProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerTVProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerTVProperties();

  /** Initializes meta properties. */
  virtual void setupMeta (void);

  /** Returns the list of TV channels. */
  virtual QStringList channels (void);

  void setInputDriver (const QString& driver)
    { setString ("Input Driver", driver); }

  void setVideoFormat (const QString& format)
    { setString ("Video Format", format); }

  void setVideoNorm (int norm)
    { setInteger ("Video Norm", norm); }

  int audioModeOption (void) const
    { return getIntegerOption ("Audio Mode"); }
  void setAudioModeOption (int mode)
    { setIntegerOption ("Audio Mode", mode); }

  void setImmediateMode (bool enable)
    { setBoolean ("Immediate Mode", enable); }

  void setAlsaCapture (bool alsa)
    { setBoolean ("ALSA Capture", alsa); }

  void setCaptureDevice (const QString& device)
    { setString ("Capture Device", device); }

  void setMjpegDecimation (int decimation)
    { setInteger ("Decimation", decimation); }

  void setMjpegQuality (int quality)
    { setInteger ("Compression", quality); }

  /** Returns a channel list based on the KDE country setting. */
  static QString channelListFromCountry (void);
};

/** The KPlayer DVB device properties.
  * @author kiriuja
  */
class KPlayerDVBProperties : public KPlayerTunerProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDVBProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerDVBProperties();

  /** Initializes meta properties. */
  virtual void setupMeta (void);

  /** Returns the list of DVB channels. */
  QStringList channels (void);
  /** Returns the name of the given channel. */
  QString channelName (const QString& id) const
    { return m_names [id]; }

protected:
  /** Default names. */
  QMap<QString, QString> m_names;
};

/** The KPlayer disk properties.
  * @author kiriuja
  */
class KPlayerDiskProperties : public KPlayerDeviceProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDiskProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerDiskProperties();

  /** Initializes property information. */
  virtual void setupInfo (void);
  /** Initializes meta properties. */
  virtual void setupMeta (void);

  /** Returns the config storage. */
  virtual KConfig* config (void) const;

  int tracks (void) const
    { return getInteger ("Tracks"); }
  void setTracks (int tracks)
  {
    setInteger ("Tracks", tracks);
    setupMeta();
  }
  bool hasTracks (void) const
    { return has ("Tracks"); }
};

/** The KPlayer track properties.
  * @author kiriuja
  */
class KPlayerTrackProperties : public KPlayerMediaProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerTrackProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerTrackProperties();

  /** Initializes meta properties. */
  virtual void setupMeta (void);

  /** Returns the config storage. */
  virtual KConfig* config (void) const;

  /** Returns device option. */
  virtual QString deviceOption (void) const;
  /** Returns device setting. */
  virtual QString deviceSetting (void) const;
  /** Returns URL string. */
  virtual QString urlString (void) const = 0;

  /** Returns the display size based on the option and the original size. */
  virtual QSize getDisplaySize (const QString& key) const;
  /** Sets the display size and option. */
  virtual void setDisplaySize (const QSize& size, int option);

  /** Returns track ID option. */
  int getTrackOption (const QString& key) const;
  /** Sets track ID option. */
  void setTrackOption (const QString& key, int value);

  /** Imports the given value under the given key. */
  void importMeta (QString key, QString value);
  /** Extracts meta information from the given string and stores it in the properties,
      updating it if the update flag is set. */
  void extractMeta (const QString& str, bool update);

  QString extension (void) const;

  float length (void) const
    { return getFloat ("Length"); }
  void setLength (float length)
    { setFloat ("Length", length); }
  bool hasLength (void) const
    { return has ("Length"); }
  /** Time length as string. */
  QString lengthString (void) const
    { return asString ("Length"); }

  int normalSeek (void) const;
  int fastSeek (void) const;

  int track (void) const
    { return getInteger ("Track"); }
  void setTrack (int track)
    { setInteger ("Track", track); }
  bool hasTrack (void) const
    { return has ("Track"); }

  const QSize& resolution (void) const
    { return getSize ("Resolution"); }
  void setResolution (const QSize& size)
    { setSize ("Resolution", size); }
  bool hasResolution (void) const
    { return has ("Resolution"); }
  /** Resolution size as string. */
  QString resolutionString (void) const
    { return asString ("Resolution"); }
  /** Resolution width as string. */
  QString resolutionWidthString (void) const
  {
    const QSize& size (resolution());
    return size.isEmpty() ? QString::null : QString::number (size.width());
  }
  /** Resolution height as string. */
  QString resolutionHeightString (void) const
  {
    const QSize& size (resolution());
    return size.isEmpty() ? QString::null : QString::number (size.height());
  }

  const QSize& originalSize (void) const
    { return getSize ("Video Size"); }
  void setOriginalSize (const QSize& size)
    { setSize ("Video Size", size); }
  bool hasOriginalSize (void) const
    { return has ("Video Size"); }
  /** Original size as string. */
  QString originalSizeString (void) const
    { return asString ("Video Size"); }
  /** Original width as string. */
  QString originalWidthString (void) const
  {
    const QSize& size (originalSize());
    return size.isEmpty() ? QString::null : QString::number (size.width());
  }
  /** Original height as string. */
  QString originalHeightString (void) const
  {
    const QSize& size (originalSize());
    return size.isEmpty() ? QString::null : QString::number (size.height());
  }

  const QSize& currentResolution (void) const
    { return hasCurrentResolution() ? getSize ("Current Resolution") : resolution(); }
  void setCurrentResolution (const QSize& size)
    { setSize ("Current Resolution", size); }
  void resetCurrentResolution (void)
    { reset ("Current Resolution"); }
  bool hasCurrentResolution (void) const
    { return has ("Current Resolution"); }

  const QSize& currentSize (void) const
    { return hasCurrentSize() ? getSize ("Current Size") : originalSize(); }
  void setCurrentSize (const QSize& size)
    { setSize ("Current Size", size); }
  void resetCurrentSize (void)
    { reset ("Current Size"); }
  bool hasCurrentSize (void) const
    { return has ("Current Size"); }
  /** Current size as string. */
  QString currentSizeString (void) const
    { return asString ("Current Size"); }
  /** Current width as string. */
  QString currentWidthString (void) const
  {
    const QSize& size (currentSize());
    return size.isEmpty() ? QString::null : QString::number (size.width());
  }
  /** Current height as string. */
  QString currentHeightString (void) const
  {
    const QSize& size (currentSize());
    return size.isEmpty() ? QString::null : QString::number (size.height());
  }

  QSize originalAspect (void) const
    { return hasDisplaySize() ? displaySize() : originalSize(); }
  QSize currentAspect (void) const
    { return hasDisplaySize() ? displaySize() : currentSize(); }

  bool hasVideo (void) const
    { return hasOriginalSize() || hasDisplaySize(); }
  bool hasNoVideo (void) const
    { return ! hasOriginalSize() && ! getBoolean ("Has Video"); }
  void setHasVideo (bool value)
    { return setBoolean ("Has Video", value); }

  /** Returns whether the original video size is known. */
  bool originalSizeKnown (void) const
    { return hasVideo() || hasNoVideo(); }

  /** Returns whether the video area needs to be expanded to accommodate subtitles. */
  bool needsExpanding (void) const;
  /** Expands the video area to accommodate subtitles. */
  void autoexpand (void);

  bool hasDemuxer (void) const
    { return hasComboValue ("Demuxer"); }
  bool hasDemuxerOption (void) const
    { return hasComboString ("Demuxer"); }

  /* bool autoexpanded (void) const
    { return getBoolean ("Autoexpanded"); }
  void setAutoexpanded (bool autoexpanded)
    { setBoolean ("Autoexpanded", autoexpanded); } */

  const KUrl& subtitleUrl (void) const
    { return getUrl ("Subtitle URL"); }
  void setSubtitleUrl (const KUrl& url)
    { setUrl ("Subtitle URL", url); }
  void setSubtitleUrl (const QString& url)
    { setSubtitleUrl (url); }
  bool hasSubtitleUrl (void) const
    { return has ("Subtitle URL"); }
  QString subtitleUrlString (void) const
  {
    const KUrl& u (subtitleUrl());
    return u.isLocalFile() ? u.path() : u.url();
  }
  QString subtitlePath (void) const
    { return asString ("Subtitle URL"); }
  void resetSubtitleUrl (void)
    { reset ("Subtitle URL"); }

  void showSubtitleUrl (const KUrl& url);

  bool showSubtitles (void) const
    { return getBoolean ("Subtitle Visibility"); }
  void setShowSubtitles (bool show)
    { return setBoolean ("Subtitle Visibility", show); }

  bool showInternalSubtitles (void) const
    { return hasSubtitleID() || hasVobsubID(); }
  bool showExternalSubtitles (void) const
    { return hasSubtitleUrl() && showSubtitles() && ! showInternalSubtitles(); }

  bool vobsubSubtitles (void) const
    { return getVobsubSubtitles ("Vobsub", subtitleUrl()); }
  int vobsubSubtitlesOption (void) const
    { return getBooleanOption ("Vobsub"); }
  void setVobsubSubtitlesOption (int vobsub)
    { setBooleanOption ("Vobsub", vobsub); }

  int subtitleIndex (void) const;
  int subtitleOption (void) const;
  void setSubtitleOption (int option);

  int subtitleID (void) const
    { return getInteger ("Subtitle ID"); }
  void setSubtitleID (int id)
    { setInteger ("Subtitle ID", id); }
  QString subtitleIDString (void) const
    { return asString ("Subtitle ID"); }
  bool hasSubtitleID (void) const
    { return has ("Subtitle ID"); }
  void resetSubtitleID (void)
    { reset ("Subtitle ID"); }

  const QMap<int, QString>& subtitleIDs (void) const
    { return getIntegerStringMap ("Subtitle IDs"); }
  void addSubtitleID (int sid)
    { setIntegerStringMapKey ("Subtitle IDs", sid); }
  void setSubtitleLanguage (int sid, const QString& slang)
    { setIntegerStringMapKeyValue ("Subtitle IDs", sid, slang); }
  bool hasSubtitleIDs (void) const
    { return has ("Subtitle IDs"); }
  bool hasSubtitleID (int sid) const
    { return hasIntegerStringMapKey ("Subtitle IDs", sid); }
  bool hasSubtitleLanguage (int sid) const
    { return hasIntegerStringMapValue ("Subtitle IDs", sid); }

  int vobsubID (void) const
    { return getInteger ("Vobsub ID"); }
  void setVobsubID (int id)
    { setInteger ("Vobsub ID", id); }
  QString vobsubIDString (void) const
    { return asString ("Vobsub ID"); }
  bool hasVobsubID (void) const
    { return has ("Vobsub ID"); }
  void resetVobsubID (void)
    { reset ("Vobsub ID"); }

  const QMap<int, QString>& vobsubIDs (void) const
    { return getIntegerStringMap ("Vobsub IDs"); }
  void addVobsubID (int sid)
    { setIntegerStringMapKey ("Vobsub IDs", sid); }
  void setVobsubLanguage (int sid, const QString& slang)
    { setIntegerStringMapKeyValue ("Vobsub IDs", sid, slang); }
  bool hasVobsubIDs (void) const
    { return has ("Vobsub IDs"); }
  bool hasVobsubID (int sid) const
    { return hasIntegerStringMapKey ("Vobsub IDs", sid); }
  bool hasVobsubLanguage (int sid) const
    { return hasIntegerStringMapValue ("Vobsub IDs", sid); }
  void resetVobsubIDs (void)
    { return reset ("Vobsub IDs"); }

  float subtitleFramerate (void) const
    { return getFloat ("Subtitle Framerate"); }
  void setSubtitleFramerate (float framerate)
    { setFloat ("Subtitle Framerate", framerate); }
  bool hasSubtitleFramerate (void) const
    { return has ("Subtitle Framerate"); }
  QString subtitleFramerateString (void) const
    { return asString ("Subtitle Framerate"); }
  void resetSubtitleFramerate (void)
    { reset ("Subtitle Framerate"); }

  int audioBitrate (void) const
    { return getInteger ("Audio Bitrate"); }
  void setAudioBitrate (int bitrate)
    { setInteger ("Audio Bitrate", bitrate); }
  bool hasAudioBitrate (void) const
    { return has ("Audio Bitrate"); }
  QString audioBitrateString (void) const
    { return asString ("Audio Bitrate"); }

  int samplerate (void) const
    { return getInteger ("Samplerate"); }
  void setSamplerate (int samplerate)
    { setInteger ("Samplerate", samplerate); }
  bool hasSamplerate (void) const
    { return has ("Samplerate"); }
  /** Samplerate as string. */
  QString samplerateString (void) const
    { return asString ("Samplerate"); }

  int channels (void) const
    { return getInteger ("Channels"); }
  void setChannels (int channels)
    { setInteger ("Channels", channels); }
  bool hasChannels (void) const
    { return has ("Channels"); }
  /** Channels as string. */
  QString channelsString (void) const
    { return asString ("Channels"); }

  bool hasAudioCodec (void) const
    { return hasComboValue ("Audio Codec"); }
  bool hasAudioCodecOption (void) const
    { return hasComboString ("Audio Codec"); }

  int audioID (void) const
    { return getInteger ("Audio ID"); }
  void setAudioID (int id)
    { setInteger ("Audio ID", id); }
  int audioIDOption (void) const
    { return getTrackOption ("Audio ID"); }
  void setAudioIDOption (int option)
    { setTrackOption ("Audio ID", option); }
  QString audioIDString (void) const
    { return asString ("Audio ID"); }
  bool hasAudioID (void) const
    { return has ("Audio ID"); }

  const QMap<int, QString>& audioIDs (void) const
    { return getIntegerStringMap ("Audio IDs"); }
  void addAudioID (int aid)
    { setIntegerStringMapKey ("Audio IDs", aid); }
  void setAudioLanguage (int aid, const QString& alang)
    { setIntegerStringMapKeyValue ("Audio IDs", aid, alang); }
  bool hasAudioID (int aid) const
    { return hasIntegerStringMapKey ("Audio IDs", aid); }
  bool hasAudioLanguage (int aid) const
    { return hasIntegerStringMapValue ("Audio IDs", aid); }

  int videoBitrate (void) const
    { return getInteger ("Video Bitrate"); }
  void setVideoBitrate (int bitrate)
    { setInteger ("Video Bitrate", bitrate); }
  bool hasVideoBitrate (void) const
    { return has ("Video Bitrate"); }
  QString videoBitrateString (void) const
    { return asString ("Video Bitrate"); }

  float framerate (void) const
    { return getFloat ("Framerate"); }
  void setFramerate (float framerate)
    { setFloat ("Framerate", framerate); }
  bool hasFramerate (void) const
    { return has ("Framerate"); }
  /** Framerate as string. */
  QString framerateString (void) const
    { return asString ("Framerate"); }

  bool hasVideoCodec (void) const
    { return hasComboValue ("Video Codec"); }
  bool hasVideoCodecOption (void) const
    { return hasComboString ("Video Codec"); }

  int videoID (void) const
    { return getInteger ("Video ID"); }
  void setVideoID (int id)
    { setInteger ("Video ID", id); }
  int videoIDOption (void) const
    { return getTrackOption ("Video ID"); }
  void setVideoIDOption (int option)
    { setTrackOption ("Video ID", option); }
  QString videoIDString (void) const
    { return asString ("Video ID"); }
  bool hasVideoID (void) const
    { return has ("Video ID"); }

  const QMap<int, QString>& videoIDs (void) const
    { return getIntegerStringMap ("Video IDs"); }
  void addVideoID (int vid)
    { setIntegerStringMapKey ("Video IDs", vid); }
  bool hasVideoID (int vid) const
    { return hasIntegerStringMapKey ("Video IDs", vid); }

  bool playlist (void) const
    { return getPlaylist ("Playlist", url()); }
  int playlistOption (void) const
    { return getBooleanOption ("Playlist"); }
  void setPlaylistOption (int playlist)
    { setBooleanOption ("Playlist", playlist); }

  bool useKioslave (void) const
    { return getUseKioslave ("Use KIOSlave", url()); }
  int useKioslaveOption (void) const
    { return getIntegerOption ("Use KIOSlave"); }
  void setUseKioslaveOption (int use)
    { setIntegerOption ("Use KIOSlave", use); }

  int useTemporaryFileOption (void) const
    { return getBooleanOption ("Use Temporary File For KIOSlave"); }
  void setUseTemporaryFileOption (int value)
    { return setBooleanOption ("Use Temporary File For KIOSlave", value); }

  /** Returns whether the height has been adjusted to aspect. */
  bool heightAdjusted (void) const
    { return m_height_adjusted; }

protected:
  /** Flag indicating whether the height has been adjusted to aspect. */
  bool m_height_adjusted;
};

/** The KPlayer disk track properties.
  * @author kiriuja
  */
class KPlayerDiskTrackProperties : public KPlayerTrackProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDiskTrackProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerDiskTrackProperties();

  /** Initializes property information. */
  virtual void setupInfo (void);

  /** Parent properties. */
  KPlayerDiskProperties* parent (void) const
    { return (KPlayerDiskProperties*) m_parent; }

  /** Returns icon name. */
  virtual QString icon (void) const;

  /** Returns device option. */
  virtual QString deviceOption (void) const;
  /** Returns device setting. */
  virtual QString deviceSetting (void) const;
  /** Returns URL string. */
  virtual QString urlString (void) const;
};

/** The KPlayer channel properties.
  * @author kiriuja
  */
class KPlayerChannelProperties : public KPlayerTrackProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerChannelProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerChannelProperties();

  /** Parent properties. */
  KPlayerDeviceProperties* parent (void) const
    { return (KPlayerDeviceProperties*) m_parent; }

  /** Returns icon name. */
  virtual QString icon (void) const;

  /** Returns whether this media needs frequency. */
  virtual bool needsFrequency (void) const;
  /** Returns whether the frequency can be changed. */
  virtual bool canChangeFrequency (void) const = 0;

  /** Returns the channel frequency. */
  int frequency (void) const;
  /** Sets the channel frequency. */
  void setFrequency (int frequency);
  /** Frequency as string. */
  QString frequencyString (void) const
    { return asString ("Frequency"); }

  /** Returns the default frequency. */
  int defaultFrequency (void) const
    { return m_default_frequency; }
  /** Sets the default frequency. */
  void setDefaultFrequency (int frequency)
    { m_default_frequency = frequency; }

protected:
  int m_default_frequency;
};

/** The KPlayer TV channel properties.
  * @author kiriuja
  */
class KPlayerTVChannelProperties : public KPlayerChannelProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerTVChannelProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerTVChannelProperties();

  /** Initializes property information. */
  virtual void setupInfo (void);

  /** Parent properties. */
  KPlayerTVProperties* parent (void) const
    { return (KPlayerTVProperties*) m_parent; }

  /** Returns whether the frequency can be changed. */
  virtual bool canChangeFrequency (void) const;

  /** Returns device option. */
  virtual QString deviceOption (void) const;
  /** Returns device setting. */
  virtual QString deviceSetting (void) const;
  /** Returns URL string. */
  virtual QString urlString (void) const;
};

/** The KPlayer DVB channel properties.
  * @author kiriuja
  */
class KPlayerDVBChannelProperties : public KPlayerChannelProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerDVBChannelProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerDVBChannelProperties();

  /** Parent properties. */
  KPlayerDVBProperties* parent (void) const
    { return (KPlayerDVBProperties*) m_parent; }

  /** Initializes property information. */
  virtual void setupInfo (void);

  /** Returns whether the frequency can be changed. */
  virtual bool canChangeFrequency (void) const;

  /** Returns device option. */
  virtual QString deviceOption (void) const;
  /** Returns device setting. */
  virtual QString deviceSetting (void) const;
  /** Returns URL string. */
  virtual QString urlString (void) const;
};

inline int KPlayerFrequencyProperty::value (void) const
{
  return m_value ? m_value : m_properties -> defaultFrequency();
}

inline void KPlayerFrequencyProperty::setValue (int value)
{
  m_value = value == m_properties -> defaultFrequency() ? 0 : value;
}

/** The KPlayer item properties.
  * @author kiriuja
  */
class KPlayerItemProperties : public KPlayerTrackProperties
{
  Q_OBJECT

public:
  /** Constructor. */
  KPlayerItemProperties (KPlayerProperties* parent, const KUrl& url);
  /** Destructor. */
  virtual ~KPlayerItemProperties();

  /** Initializes property information. */
  virtual void setupInfo (void);
  /** Initializes meta properties. */
  virtual void setupMeta (void);

  virtual bool autoloadSubtitles (const QString& key) const;
  virtual bool getVobsubSubtitles (const QString& key, const KUrl& url) const;
  virtual bool getPlaylist (const QString& key, const KUrl& url) const;
  virtual bool getUseKioslave (const QString& key, const KUrl& url) const;

  /** Returns URL string. */
  virtual QString urlString (void) const;

  int subtitleAutoloadOption (void) const
    { return getBooleanOption ("Autoload Subtitles"); }
  void setSubtitleAutoloadOption (int value)
    { return setBooleanOption ("Autoload Subtitles", value); }

  /** Resets the meta info timer. */
  static void resetMetaInfoTimer (void)
    { m_meta_info_timer = 0; }

protected:
  /** Timer that prevents wasting too much time on getting meta information. */
  static int m_meta_info_timer;
};

#ifdef DEBUG
kdbgstream kdDebugTime (void);
#endif

#endif
