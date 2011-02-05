/***************************************************************************
                          kplayersettingsdialog.cpp
                          -------------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "kplayersettingsdialog.h"
#include "kplayersettingsdialog.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

#include <KLocale>
#include <KMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QSlider>
#include <Solid/AudioInterface>
#include <Solid/Device>
#include <Solid/DeviceInterface>

#ifdef DEBUG
#define DEBUG_KPLAYER_SETTINGS_DIALOG
#endif

QString comboEntry (QComboBox* combo)
{
  return combo -> currentIndex() < 0 ? combo -> currentText() : combo -> itemData (combo -> currentIndex()).toString();
}

KPlayerSettingsDialog::KPlayerSettingsDialog (QWidget* parent)
  : KPageDialog (parent)
{
  m_advanced = 0;
  setFaceType (KPageDialog::Tree);
  setCaption (i18n("KPlayer Preferences"));
  setButtons (KDialog::Help | KDialog::Default | KDialog::Ok | KDialog::Apply | KDialog::Cancel);
  connect (this, SIGNAL (applyClicked()), SLOT (apply()));
  connect (this, SIGNAL (okClicked()), SLOT (apply()));
  connect (this, SIGNAL (defaultClicked()), SLOT (defaults()));
  connect (this, SIGNAL (currentPageChanged (KPageWidgetItem*, KPageWidgetItem*)),
    SLOT (pageAboutToShow (KPageWidgetItem*, KPageWidgetItem*)));
//m_initial_move = false;
  m_general = new KPlayerSettingsGeneral;
  KPageWidgetItem* item = new KPageWidgetItem (m_general);
  item -> setName (i18n("General"));
  item -> setHeader (i18n("General Settings"));
  addPage (item);
  m_page_names.insert (item, "general");
  m_controls = new KPlayerSettingsControls;
  KPageWidgetItem* controls = new KPageWidgetItem (m_controls);
  controls -> setName (i18n("Controls"));
  controls -> setHeader (i18n("Control Settings"));
  addPage (controls);
  m_page_names.insert (controls, "controls");
  m_video = new KPlayerSettingsVideo;
  item = new KPageWidgetItem (m_video);
  item -> setName (i18n("Video"));
  item -> setHeader (i18n("Video Settings"));
  addPage (item);
  m_page_names.insert (item, "video");
  m_audio = new KPlayerSettingsAudio;
  item = new KPageWidgetItem (m_audio);
  item -> setName (i18n("Audio"));
  item -> setHeader (i18n("Audio Settings"));
  addPage (item);
  m_page_names.insert (item, "audio");
  m_subtitles = new KPlayerSettingsSubtitles;
  item = new KPageWidgetItem (m_subtitles);
  item -> setName (i18n("Subtitles"));
  item -> setHeader (i18n("Subtitle Settings"));
  addPage (item);
  m_page_names.insert (item, "subtitles");
  m_advanced = new KPlayerSettingsAdvanced;
  item = new KPageWidgetItem (m_advanced);
  item -> setName (i18n("Advanced"));
  item -> setHeader (i18n("Advanced Settings"));
  addPage (item);
  m_page_names.insert (item, "advanced");
  m_progress = new KPlayerSettingsProgress;
  item = new KPageWidgetItem (m_progress);
  item -> setName (i18n("Progress"));
  item -> setHeader (i18n("Progress Control Settings"));
  addSubPage (controls, item);
  m_page_names.insert (item, "progress");
  m_volume = new KPlayerSettingsVolume;
  item = new KPageWidgetItem (m_volume);
  item -> setName (i18n("Volume"));
  item -> setHeader (i18n("Volume Control Settings"));
  addSubPage (controls, item);
  m_page_names.insert (item, "volume");
  m_contrast = new KPlayerSettingsContrast;
  item = new KPageWidgetItem (m_contrast);
  item -> setName (i18n("Contrast"));
  item -> setHeader (i18n("Contrast Control Settings"));
  addSubPage (controls, item);
  m_page_names.insert (item, "contrast");
  m_brightness = new KPlayerSettingsBrightness;
  item = new KPageWidgetItem (m_brightness);
  item -> setName (i18n("Brightness"));
  item -> setHeader (i18n("Brightness Control Settings"));
  addSubPage (controls, item);
  m_page_names.insert (item, "brightness");
  m_hue = new KPlayerSettingsHue;
  item = new KPageWidgetItem (m_hue);
  item -> setName (i18n("Hue"));
  item -> setHeader (i18n("Hue Control Settings"));
  addSubPage (controls, item);
  m_page_names.insert (item, "hue");
  m_saturation = new KPlayerSettingsSaturation;
  item = new KPageWidgetItem (m_saturation);
  item -> setName (i18n("Saturation"));
  item -> setHeader (i18n("Saturation Control Settings"));
  addSubPage (controls, item);
  m_page_names.insert (item, "saturation");
  m_sliders = new KPlayerSettingsSliders;
  item = new KPageWidgetItem (m_sliders);
  item -> setName (i18n("Sliders"));
  item -> setHeader (i18n("Slider Control Settings"));
  addSubPage (controls, item);
  m_page_names.insert (item, "sliders");
  setHelp ("settings");
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
  QString name (group.readEntry ("Settings Dialog Page"));
  for ( QHash<QObject*, QString>::ConstIterator it (m_page_names.constBegin()); it != m_page_names.constEnd(); ++ it )
    if ( it.value() == name )
      setCurrentPage ((KPageWidgetItem*) it.key());
/*int x = group.readEntry ("Settings Dialog Left", -1);
  int y = group.readEntry ("Settings Dialog Top", -1);*/
  int w = group.readEntry ("Settings Dialog Width", 0);
  int h = group.readEntry ("Settings Dialog Height", 0);
//QSize size (group.readEntry ("Settings Dialog Width", 0),
//  group.readEntry ("Settings Dialog Height", 0));
//QSize hint = minimumSizeHint();
//if ( size.width() < hint.width() || size.height() < hint.height() )
//  size = sizeHint();
//setGeometry (x, y, size.width(), size.height());
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
//kdDebugTime() << "KPlayerSettingsDialog position " << x << "x" << y << " size " << w << "x" << h << "\n";
#endif
/*if ( x >= 0 && y >= 0 )
  {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << "KPlayerSettingsDialog moving to " << x << "x" << y << "\n";
#endif
    move (x, y);
  }*/
  if ( w > 0 && h > 0 )
  {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << "KPlayerSettingsDialog resizing to " << w << "x" << h << "\n";
#endif
    resize (w, h);
  }
}

KPlayerSettingsDialog::~KPlayerSettingsDialog (void)
{
  kPlayerEngine() -> getLists();
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog " << x() << "x" << y() << " " << width() << "x" << height() << " Hint " << sizeHint().width() << "x" << sizeHint().height() << "\n";
#endif
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
/*if ( size() == sizeHint() )
  {
    group.deleteEntry ("Settings Dialog Width");
    group.deleteEntry ("Settings Dialog Height");
  }
  else
  {*/
/*group.writeEntry ("Settings Dialog Left", frameGeometry().x());
  group.writeEntry ("Settings Dialog Top", frameGeometry().y());*/
  group.writeEntry ("Settings Dialog Width", width());
  group.writeEntry ("Settings Dialog Height", height());
  if ( m_page_names.contains (currentPage()) )
    group.writeEntry ("Settings Dialog Page", m_page_names [currentPage()]);
  else
    group.deleteEntry ("Settings Dialog Page");
}

/*void KPlayerSettingsDialog::moveEvent (QMoveEvent* event)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog move " << event -> oldPos().x() << "x" << event -> oldPos().y()
    << " => " << event -> pos().x() << "x" << event -> pos().y() << "\n";
#endif
  KDialogBase::moveEvent (event);
  if ( m_initial_move )
    return;
  m_initial_move = true;
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
  int x = group.readEntry ("Settings Dialog Left", -1);
  int y = group.readEntry ("Settings Dialog Top", -1);
  if ( x >= 0 && y >= 0 )
  {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << "KPlayerSettingsDialog moving to " << x << "x" << y << "\n";
#endif
    move (x + frameGeometry().x() - geometry().x(), y + frameGeometry().y() - geometry().y());
  }
  m_initial_move = false;
}

void KPlayerSettingsDialog::resizeEvent (QResizeEvent* event)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog resize " << event -> oldSize(). width() << "x" << event -> oldSize(). height()
    << " => " << event -> size(). width() << "x" << event -> size(). height() << "\n";
#endif
  KDialogBase::resizeEvent (event);
}

void KPlayerSettingsDialog::showEvent (QShowEvent* event)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog::showEvent\n";
#endif
  KDialogBase::showEvent (event);
}

void KPlayerSettingsDialog::windowActivationChange (bool old)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog::windowActivationChange " << old << "\n";
#endif
  m_initial_move = true;
  KDialogBase::windowActivationChange (old);
}*/

void KPlayerSettingsDialog::defaults (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog::defaults\n";
#endif
  if ( KMessageBox::warningYesNo (this, i18n("All configuration settings will be reset.\n\nAre you sure?"))
      != KMessageBox::Yes )
    return;
  configuration() -> defaults();
  configuration() -> commit();
  m_general -> load();
  m_controls -> load();
  m_video -> load();
  m_audio -> load();
  m_sliders -> load();
  m_subtitles -> load();
  m_advanced -> load();
  m_progress -> load();
  m_volume -> load();
  m_contrast -> load();
  m_brightness -> load();
  m_hue -> load();
  m_saturation -> load();
  setButtonGuiItem (KDialog::Cancel, KStandardGuiItem::close());
}

void KPlayerSettingsDialog::pageAboutToShow (KPageWidgetItem* current, KPageWidgetItem*)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog::pageAboutToShow\n";
#endif
  if ( m_advanced )
    m_advanced -> refreshLists();
  setHelp (m_page_names.contains (current) ? "settings-" + m_page_names [current] : "settings");
}

//void dumpObject (const QObject* object, int indent, int depth = 20);

void KPlayerSettingsDialog::apply (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsDialog::apply\n";
  //dumpObject (this, 0);
#endif
  m_advanced -> save();
  m_audio -> save();
  m_controls -> save();
  m_general -> save();
  m_sliders -> save();
  m_subtitles -> save();
  m_video -> save();
  m_progress -> save();
  m_volume -> save();
  m_contrast -> save();
  m_brightness -> save();
  m_hue -> save();
  m_saturation -> save();
  configuration() -> commit();
}

KPlayerSettingsAdvanced::KPlayerSettingsAdvanced (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  loadLists();
  load();
  QApplication::connect (kPlayerEngine(), SIGNAL (updated()), this, SLOT (refresh()));
}

void KPlayerSettingsAdvanced::refresh (void)
{
  int index = c_demuxer -> currentIndex();
  if ( index > 0 )
    index = engine() -> demuxerIndex (listEntry (c_demuxer)) + 1;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "Demuxer: " << c_demuxer -> currentText() << " => " << index << " / " << engine() -> demuxerCount() << "\n";
#endif
  loadLists();
  c_demuxer -> setCurrentIndex (index);
}

void KPlayerSettingsAdvanced::loadLists (void)
{
  if ( engine() -> demuxerCount() )
  {
    c_demuxer -> clear();
    c_demuxer -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> demuxerCount(); i ++ )
      c_demuxer -> addItem (engine() -> demuxerName (i));
  }
}

void KPlayerSettingsAdvanced::load (void)
{
  c_executable_path -> setText (configuration() -> executablePath());
  c_command_line -> setText (configuration() -> commandLine());
  c_demuxer -> setCurrentIndex (engine() -> demuxerIndex (configuration() -> demuxer()) + 1);
  c_frame_drop -> setCurrentIndex (configuration() -> frameDrop());
  c_use_cache -> setCurrentIndex (configuration() -> cache());
  cacheChanged (c_use_cache -> currentIndex());
  c_build_index -> setCurrentIndex (configuration() -> buildNewIndex());
  c_osd_level -> setCurrentIndex (configuration() -> osdLevel());
  c_use_temporary_file -> setChecked (configuration() -> useTemporaryFile());
  c_use_kioslave_for_http -> setChecked (configuration() -> useKioslaveForHttp());
  c_use_kioslave_for_ftp -> setChecked (configuration() -> useKioslaveForFtp());
  c_use_kioslave_for_smb -> setChecked (configuration() -> useKioslaveForSmb());
}

void KPlayerSettingsAdvanced::save (void)
{
  configuration() -> setExecutablePath (c_executable_path -> text());
  configuration() -> setCommandLine (c_command_line -> text());
  configuration() -> setDemuxer (listEntry (c_demuxer));
  configuration() -> setFrameDrop (c_frame_drop -> currentIndex());
  configuration() -> setCache (c_use_cache -> currentIndex(), c_cache_size -> value());
  configuration() -> setBuildNewIndex (c_build_index -> currentIndex());
  configuration() -> setOsdLevel (c_osd_level -> currentIndex());
  configuration() -> setUseTemporaryFile (c_use_temporary_file -> isChecked());
  configuration() -> setUseKioslaveForHttp (c_use_kioslave_for_http -> isChecked());
  configuration() -> setUseKioslaveForFtp (c_use_kioslave_for_ftp -> isChecked());
  configuration() -> setUseKioslaveForSmb (c_use_kioslave_for_smb -> isChecked());
}

void KPlayerSettingsAdvanced::refreshLists (void)
{
  engine() -> getLists (c_executable_path -> text());
}

void KPlayerSettingsAdvanced::cacheChanged (int cache)
{
  c_cache_size -> setValue (configuration() -> cacheSize());
  c_cache_size -> setEnabled (cache == 2);
  if ( cache == 2 && sender() )
  {
    c_cache_size -> setFocus();
    c_cache_size -> selectAll();
  }
}

KPlayerSettingsAudio::KPlayerSettingsAudio (QWidget* parent)
  : QFrame (parent)
{
  m_amixer_running = false;
  m_devices_listed = true;
  setupUi (this);
  c_maximum -> setSuffix (ki18np(" percent", " percent"));
  loadLists();
  load();
  QApplication::connect (kPlayerEngine(), SIGNAL (updated()), this, SLOT (refresh()));
}

void KPlayerSettingsAudio::refresh (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::refresh\n";
#endif
  int dindex = c_driver -> currentIndex();
  if ( dindex > 0 )
    dindex = engine() -> audioDriverIndex (listEntry (c_driver)) + 1;
  int cindex = c_codec -> currentIndex();
  if ( cindex > 0 )
    cindex = engine() -> audioCodecIndex (listEntry (c_codec)) + 1;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "Audio Driver: " << c_driver -> currentText() << " => " << dindex << " / " << engine() -> audioDriverCount() << "\n";
  kdDebugTime() << "Audio Codec: " << c_codec -> currentText() << " => " << cindex << " / " << engine() -> audioCodecCount() << "\n";
#endif
  loadLists();
  c_driver -> setCurrentIndex (dindex);
  driverChanged (dindex);
  c_codec -> setCurrentIndex (cindex);
}

void KPlayerSettingsAudio::loadLists (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::loadLists\n";
#endif
  if ( engine() -> audioCodecCount() )
  {
    c_codec -> clear();
    c_codec -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> audioCodecCount(); i ++ )
      c_codec -> addItem (engine() -> audioCodecName (i));
  }
  if ( engine() -> audioDriverCount() )
  {
    c_driver -> clear();
    c_driver -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> audioDriverCount(); i ++ )
      c_driver -> addItem (engine() -> audioDriverName (i));
  }
}

void KPlayerSettingsAudio::load (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::load\n";
#endif
  c_driver -> setCurrentIndex (engine() -> audioDriverIndex (configuration() -> audioDriver()) + 1);
  m_softvol = configuration() -> softwareVolume();
  c_softvol -> setChecked (m_softvol);
  softvolChanged (m_softvol);
  c_codec -> setCurrentIndex (engine() -> audioCodecIndex (configuration() -> audioCodec()) + 1);
  c_delay_step -> setValue (configuration() -> audioDelayStep());
}

void KPlayerSettingsAudio::save (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::save\n";
#endif
  configuration() -> setAudioDriver (listEntry (c_driver));
  if ( c_driver -> currentIndex() > 0 )
  {
    configuration() -> setAudioDevice (comboEntry (c_device));
    configuration() -> setMixerDevice (comboEntry (c_mixer));
    configuration() -> setMixerChannel (c_channel -> currentText());
  }
  configuration() -> setSoftwareVolume (c_softvol -> isChecked());
  if ( configuration() -> softwareVolume() )
  {
    int maximum = c_maximum -> value();
    configuration() -> setMaximumSoftwareVolume (maximum);
  }
  configuration() -> setAudioCodec (listEntry (c_codec));
  float value = c_delay_step -> value();
  if ( value )
    configuration() -> setAudioDelayStep (value);
  else
    configuration() -> resetAudioDelayStep();
}

void KPlayerSettingsAudio::defaultAlsaChannels (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::defaultAlsaChannels\n";
#endif
  QString text = c_channel -> currentText();
  c_channel -> addItem ("Master");
  c_channel -> addItem ("PCM");
  c_channel -> addItem ("Line");
  c_channel -> setEditText (text);
}

void KPlayerSettingsAudio::runAmixer (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::runAmixer\n";
#endif
  m_rerun_amixer = m_amixer_running;
  if ( m_amixer_running )
    return;
  c_channel -> clear();
  KPlayerLineOutputProcess* amixer = new KPlayerLineOutputProcess;
  *amixer << "amixer";
  QString mixer = comboEntry (c_mixer);
  if ( ! mixer.isEmpty() )
  {
    *amixer << "-D" << mixer;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << " Device " << mixer << "\n";
#endif
  }
  *amixer << "scontents";
  connect (amixer, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*)),
    SLOT (amixerOutput (KPlayerLineOutputProcess*, char*)));
  connect (amixer, SIGNAL (processFinished (KPlayerLineOutputProcess*)),
    SLOT (amixerFinished (KPlayerLineOutputProcess*)));
  amixer -> start();
  m_amixer_running = true;
  if ( ! m_amixer_running )
    defaultAlsaChannels();
}

void KPlayerSettingsAudio::amixerOutput (KPlayerLineOutputProcess*, char* str)
{
  static QString control;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << " amixer: " << str << "\n";
#endif
  static QRegExp re_control ("^Simple mixer control '(.*)'");
  static QRegExp re_pvolume ("^ +Capabilities:.* pvolume");
  if ( re_control.indexIn (str) >= 0 )
  {
    control = re_control.cap(1);
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << " Control " << control << "\n";
#endif
  }
  else if ( re_pvolume.indexIn (str) >= 0 )
  {
    QString text = c_channel -> currentText();
    c_channel -> addItem (control);
    c_channel -> setEditText (text);
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << " Volume " << control << "\n";
#endif
  }
}

void KPlayerSettingsAudio::amixerFinished (KPlayerLineOutputProcess* proc)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::amixerExited\n";
#endif
  delete proc;
  m_amixer_running = false;
  if ( m_rerun_amixer )
    runAmixer();
  else if ( c_channel -> count() <= 0 )
    defaultAlsaChannels();
}

void KPlayerSettingsAudio::driverChanged (int index)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::driverChanged\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  QString driver (listEntry (c_driver));
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << " Driver " << driver << "\n";
#endif
  bool device = index > 0;
  bool softvol = c_softvol -> isChecked();
  bool alsa = driver == "alsa";
  bool oss = driver == "oss";
  bool sun = driver == "sun";
  bool mixer = device && ! softvol && (alsa || oss || sun);
  bool channel = mixer && ! sun;
  if ( driver != m_driver )
  {
    m_devices_listed = false;
    c_device -> clear();
    c_mixer -> clear();
    if ( alsa || oss )
    {
      QStringList cards;
      foreach ( const Solid::Device &device, Solid::Device::listFromType (Solid::DeviceInterface::AudioInterface) )
        if ( const Solid::AudioInterface* interface = device.as<Solid::AudioInterface>() )
        {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
          kdDebugTime() << " Device type " << interface -> deviceType() << "\n";
          kdDebugTime() << " Card type " << interface -> soundcardType() << "\n";
          kdDebugTime() << " Driver " << interface -> driver() << "\n";
          if ( ! interface -> name().isEmpty() )
            kdDebugTime() << " Name   " << interface -> name().toLatin1().constData() << "\n";
#endif
          if ( interface -> deviceType() & (Solid::AudioInterface::AudioControl | Solid::AudioInterface::AudioOutput) )
          {
            QString name;
            if ( alsa && interface -> driver() == Solid::AudioInterface::Alsa )
            {
              bool ok;
              QVariantList list = interface -> driverHandle().toList();
              if ( ! list.isEmpty() )
              {
                name = list[0].toString();
                index = list[0].toInt (&ok);
                if ( ok && index >= 0 )
                {
                  QFile info ("/proc/asound/card" + QString::number (index) + "/id");
                  if ( info.open (QIODevice::ReadOnly) )
                  {
                    QByteArray id (info.readLine());
                    info.close();
                    if ( ! id.isEmpty() && id.endsWith ('\n') )
                      id.chop (1);
                    if ( ! id.isEmpty() )
                      name = id;
                  }
                }
              }
              int device = -1;
              if ( list.count() > 1 )
              {
                index = list[1].toInt (&ok);
                if ( ok )
                  device = index;
              }
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
              if ( ! name.isEmpty() )
                kdDebugTime() << " Card   " << name.toLatin1().constData() << "\n";
              if ( device >= 0 )
                kdDebugTime() << " Device " << device << "\n";
#endif
              name = "hw:" + name;
              if ( interface -> deviceType() & Solid::AudioInterface::AudioOutput )
                cards.append (name);
              if ( device >= 0 )
                name += "," + QString::number (device);
            }
            else if ( oss && interface -> driver() == Solid::AudioInterface::OpenSoundSystem )
            {
              name = interface -> driverHandle().toString();
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
              kdDebugTime() << " Device " << name << "\n";
#endif
            }
            else
              continue;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
            kdDebugTime() << " Adding " << name.toLatin1().constData() << "\n";
#endif
            QString entry = interface -> name().isEmpty() ? name : i18n ("%1: %2", name, interface -> name());
            if ( entry.endsWith (" ()") )
              entry.remove (entry.length() - 3, 3);
            if ( interface -> deviceType() & Solid::AudioInterface::AudioOutput )
              c_device -> addItem (entry, name);
            if ( interface -> deviceType() & Solid::AudioInterface::AudioControl )
              c_mixer -> addItem (entry, name);
          }
        }
      if ( alsa )
        for ( int i = c_mixer -> count() - 1; i >= 0; i -- )
          if ( ! cards.contains (c_mixer -> itemData (i).toString().section (',', 0, 0)) )
          {
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
            kdDebugTime() << " Removing " << c_mixer -> itemText (i) << "\n";
#endif
            c_mixer -> removeItem (i);
          }
    }
    bool original = driver == configuration() -> audioDriver();
    c_device -> model() -> sort (0);
    c_device -> setEditText (original && device ? configuration() -> audioDevice() : "");
    c_device -> setCurrentIndex (c_device -> findData (c_device -> currentText()));
    m_device = c_device -> currentText();
    c_mixer -> model() -> sort (0);
    c_mixer -> setEditText (original && mixer ? configuration() -> mixerDevice() : "");
    c_mixer -> setCurrentIndex (c_mixer -> findData (c_mixer -> currentText()));
    if ( channel )
    {
      if ( driver == "oss" )
      {
        c_channel -> clear();
        c_channel -> addItem ("vol");
        c_channel -> addItem ("pcm");
        c_channel -> addItem ("line");
      }
      else if ( ! m_amixer_running )
        runAmixer();
    }
    c_channel -> setEditText (original && channel ? configuration() -> mixerChannel() : "");
    m_devices_listed = true;
  }
  c_device -> setEnabled (device);
  c_mixer -> setEnabled (mixer);
  c_channel -> setEnabled (channel);
  m_driver = driver;
}

void KPlayerSettingsAudio::deviceChanged (const QString& device)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::deviceChanged\n";
#endif
  if ( ! m_devices_listed )
    return;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << " Device " << device << "\n";
#endif
  if ( c_mixer -> isEnabled() && listEntry (c_driver) == "alsa" )
  {
    if ( c_device -> currentIndex() < 0 )
    {
      if ( c_mixer -> currentText() == m_device )
        c_mixer -> setEditText (device);
    }
    else if ( c_mixer -> currentIndex() >= 0 || c_mixer -> currentText().isEmpty() )
      c_mixer -> setCurrentIndex (c_mixer -> findData (comboEntry (c_device).section (',', 0, 0)));
  }
  m_device = device;
}

void KPlayerSettingsAudio::mixerChanged (const QString&)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::mixerChanged\n";
#endif
  if ( m_devices_listed && ! c_softvol -> isChecked() && listEntry (c_driver) == "alsa" )
    runAmixer();
}

void KPlayerSettingsAudio::softvolChanged (bool checked)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::softvolChanged\n";
  kdDebugTime() << " Checked " << checked << "\n";
#endif
  c_maximum -> setValue (configuration() -> maximumSoftwareVolume());
  c_maximum -> setEnabled (checked);
  QString driver (listEntry (c_driver));
  bool empty = checked || driver != configuration() -> audioDriver()
    || (driver != "alsa" && driver != "oss" && driver != "sun");
  c_mixer -> setEditText (empty ? "" : configuration() -> hasMixerDevice()
    || m_softvol == checked ? configuration() -> mixerDevice() : configuration() -> audioDevice());
  c_channel -> setEditText (empty || driver == "sun" ? "" : configuration() -> mixerChannel());
  driverChanged (c_driver -> currentIndex());
  m_softvol = checked;
}

KPlayerSettingsControls::KPlayerSettingsControls (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsControls::load (void)
{
  c_shift -> setChecked (configuration() -> rememberWithShift());
  c_size -> setChecked (configuration() -> rememberSize());
  rememberSizeChanged (configuration() -> rememberSize());
  c_full_screen -> setChecked (configuration() -> rememberFullScreen());
  c_maximized -> setChecked (configuration() -> rememberMaximized());
  c_maintain_aspect -> setChecked (configuration() -> rememberMaintainAspect());
  c_volume -> setChecked (configuration() -> rememberVolume());
  c_audio_delay -> setChecked (configuration() -> rememberAudioDelay());
  c_frame_drop -> setChecked (configuration() -> rememberFrameDrop());
  c_contrast -> setChecked (configuration() -> rememberContrast());
  c_brightness -> setChecked (configuration() -> rememberBrightness());
  c_hue -> setChecked (configuration() -> rememberHue());
  c_saturation -> setChecked (configuration() -> rememberSaturation());
  c_subtitle_position -> setChecked (configuration() -> rememberSubtitlePosition());
  c_subtitle_delay -> setChecked (configuration() -> rememberSubtitleDelay());
}

void KPlayerSettingsControls::save (void)
{
  configuration() -> setRememberWithShift (c_shift -> isChecked());
  configuration() -> setRememberSize (c_size -> isChecked());
  if ( ! configuration() -> rememberSize() )
    configuration() -> setRememberAspect (c_aspect -> isChecked());
  configuration() -> setRememberFullScreen (c_full_screen -> isChecked());
  configuration() -> setRememberMaximized (c_maximized -> isChecked());
  configuration() -> setRememberMaintainAspect (c_maintain_aspect -> isChecked());
  configuration() -> setRememberVolume (c_volume -> isChecked());
  configuration() -> setRememberAudioDelay (c_audio_delay -> isChecked());
  configuration() -> setRememberFrameDrop (c_frame_drop -> isChecked());
  configuration() -> setRememberContrast (c_contrast -> isChecked());
  configuration() -> setRememberBrightness (c_brightness -> isChecked());
  configuration() -> setRememberHue (c_hue -> isChecked());
  configuration() -> setRememberSaturation (c_saturation -> isChecked());
  configuration() -> setRememberSubtitlePosition (c_subtitle_position -> isChecked());
  configuration() -> setRememberSubtitleDelay (c_subtitle_delay -> isChecked());
}

void KPlayerSettingsControls::rememberSizeChanged (bool checked)
{
  c_aspect -> setChecked (checked || configuration() -> rememberAspect());
  c_aspect -> setEnabled (! checked);
}

KPlayerSettingsGeneral::KPlayerSettingsGeneral (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsGeneral::load (void)
{
  c_resize_automatically -> setChecked (configuration() -> resizeAutomatically());
  c_playlist_menu_size -> setValue (configuration() -> playlistMenuSize());
  c_recent_menu_size -> setValue (configuration() -> recentMenuSize());
  c_recent_list_size -> setValue (configuration() -> recentListSize());
  c_cache_size_limit -> setValue (configuration() -> cacheSizeLimit());
  c_allow_duplicate_entries -> setChecked (configuration() -> allowDuplicateEntries());
  c_show_messages_on_error -> setChecked (configuration() -> showMessagesOnError());
}

void KPlayerSettingsGeneral::save (void)
{
  configuration() -> setResizeAutomatically (c_resize_automatically -> isChecked());
  configuration() -> setPlaylistMenuSize (c_playlist_menu_size -> value());
  configuration() -> setRecentMenuSize (c_recent_menu_size -> value());
  configuration() -> setRecentListSize (c_recent_list_size -> value());
  configuration() -> setCacheSizeLimit (c_cache_size_limit -> value());
  configuration() -> setAllowDuplicateEntries (c_allow_duplicate_entries -> isChecked());
  configuration() -> setShowMessagesOnError (c_show_messages_on_error -> isChecked());
}

KPlayerSettingsSliders::KPlayerSettingsSliders (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  c_minimum_slider_length -> setSuffix(ki18np(" pixel", " pixels"));
  c_preferred_slider_length -> setSuffix(ki18np(" pixel", " pixels"));
  c_slider_marks -> setSuffix(ki18np(" percent", " percent"));
  load();
}

void KPlayerSettingsSliders::load (void)
{
  c_minimum_slider_length -> setValue (configuration() -> minimumSliderLength());
  c_preferred_slider_length -> setValue (configuration() -> preferredSliderLength());
  c_slider_marks -> setValue (configuration() -> sliderMarks());
  c_show_slider_marks -> setChecked (configuration() -> showSliderMarks());
  showMarksChanged (c_show_slider_marks -> isChecked());
}

void KPlayerSettingsSliders::save (void)
{
  configuration() -> setPreferredSliderLength (c_preferred_slider_length -> value());
  configuration() -> setMinimumSliderLength (c_minimum_slider_length -> value());
  configuration() -> setShowSliderMarks (c_show_slider_marks -> isChecked());
  configuration() -> setSliderMarks (c_slider_marks -> value());
}

void KPlayerSettingsSliders::showMarksChanged (bool showMarksChecked)
{
  l_slider_marks -> setEnabled (showMarksChecked);
  c_slider_marks -> setEnabled (showMarksChecked);
}

void KPlayerSettingsSliders::minimumSliderLengthChanged (int minimumSliderLength)
{
  c_preferred_slider_length -> setMinimum(minimumSliderLength);
}

KPlayerSettingsSubtitles::KPlayerSettingsSubtitles (QWidget* parent)
  : QFrame (parent)
{
  m_initialized = m_recursion = false;
  setupUi (this);
  c_width -> setSuffix (ki18np(" percent", " percent"));
  c_position_step -> setSuffix (ki18np(" percent", " percent"));
  loadLists();
  load();
  m_initialized = true;
}

void fillEncodingCombobox (QComboBox* combobox);

void KPlayerSettingsSubtitles::loadLists (void)
{
  QList<int> sizes (QFontDatabase::standardSizes());
  QList<int>::ConstIterator it (sizes.begin());
  for ( int i = 1; i < *it; ++ i )
    c_text_size -> addItem (QString::number (i));
  while ( it != sizes.constEnd() )
  {
    c_text_size -> addItem (QString::number (*it));
    ++ it;
  }
  fillEncodingCombobox (c_encoding);
}

void KPlayerSettingsSubtitles::load (void)
{
  c_font_name -> setEditText (configuration() -> subtitleFontName());
  c_bold -> setChecked (configuration() -> subtitleFontBold());
  c_italic -> setChecked (configuration() -> subtitleFontItalic());
  if ( configuration() -> subtitleTextSize() )
    c_text_size -> setEditText (QString::number (configuration() -> subtitleTextSize()));
  else
    c_text_size -> setCurrentIndex (0);
  c_autoscale -> setChecked (configuration() -> subtitleAutoscale());
  c_outline -> setValue (configuration() -> subtitleFontOutline());
  outlineEditChanged (c_outline -> value());
  c_width -> setValue (configuration() -> subtitleTextWidth());
  widthEditChanged (c_width -> value());
  c_position_step -> setValue (configuration() -> subtitlePositionStep());
  c_delay_step -> setValue (configuration() -> subtitleDelayStep());
  if ( configuration() -> hasSubtitleEncoding() )
  {
    QString encoding = configuration() -> subtitleEncoding();
    c_encoding -> setEditText (encoding);
    encoding += ": ";
    for ( int i = 1; i < c_encoding -> count(); ++ i )
      if ( c_encoding -> itemText (i).startsWith (encoding) )
      {
        c_encoding -> setCurrentIndex (i);
        break;
      }
  }
  else
    c_encoding -> setCurrentIndex (0);
  c_closed_caption -> setChecked (configuration() -> subtitleClosedCaption());
  c_subtitles_autoexpand -> setChecked (configuration() -> hasSubtitleAutoexpand());
  autoexpandChanged (c_subtitles_autoexpand -> isChecked());
  c_subtitles_autoload -> setChecked (configuration() -> subtitleAutoload());
  autoloadSubtitlesChanged (c_subtitles_autoload -> isChecked());
}

void KPlayerSettingsSubtitles::save (void)
{
  configuration() -> setSubtitleFontName (c_font_name -> currentText());
  configuration() -> setSubtitleFontBold (c_bold -> isChecked());
  configuration() -> setSubtitleFontItalic (c_italic -> isChecked());
  float value = fabs (c_text_size -> currentText().toFloat());
  if ( value )
    configuration() -> setSubtitleTextSize (value);
  else
    configuration() -> resetSubtitleTextSize();
  configuration() -> setSubtitleAutoscale (c_autoscale -> isChecked());
  configuration() -> setSubtitleFontOutline (c_outline -> value());
  configuration() -> setSubtitleTextWidth (c_width -> value());
  configuration() -> setSubtitlePositionStep (c_position_step -> value());
  value = c_delay_step -> value();
  if ( value )
    configuration() -> setSubtitleDelayStep (value);
  else
    configuration() -> resetSubtitleDelayStep();
  if ( c_encoding -> currentIndex() )
    configuration() -> setSubtitleEncoding (c_encoding -> currentText().section (':', 0, 0));
  else
    configuration() -> resetSubtitleEncoding();
  configuration() -> setSubtitleClosedCaption (c_closed_caption -> isChecked());
  if ( c_subtitles_autoexpand -> isChecked() )
    configuration() -> setSubtitleAutoexpand (c_aspect -> currentIndex() + 1);
  else
    configuration() -> resetSubtitleAutoexpand();
  configuration() -> setSubtitleAutoload (c_subtitles_autoload -> isChecked());
  if ( c_subtitles_autoload -> isChecked() )
    configuration() -> setAutoloadExtensionList (c_subtitles_list -> text());
}

void KPlayerSettingsSubtitles::widthSliderChanged (int value)
{
  if ( ! m_recursion )
    c_width -> setValue (value);
}

void KPlayerSettingsSubtitles::widthEditChanged (const int number)
{
  m_recursion = true;
  c_width_slider -> setValue (number);
  m_recursion = false;
}

void KPlayerSettingsSubtitles::outlineSliderChanged (int value)
{
  if ( ! m_recursion )
    c_outline -> setValue (value / 20.0);
}

void KPlayerSettingsSubtitles::outlineEditChanged (const double value)
{
  int number = int (value * 20 + 0.5);
  m_recursion = true;
  c_outline_slider -> setValue (number);
  m_recursion = false;
}

void KPlayerSettingsSubtitles::autoexpandChanged (bool autoexpandChecked)
{
  if ( autoexpandChecked )
  {
    if ( c_aspect -> itemText (0).isEmpty() )
      c_aspect -> removeItem (0);
    int value = configuration() -> subtitleAutoexpand();
    c_aspect -> setCurrentIndex (value ? value - 1 : 1);
  }
  else
  {
    c_aspect -> insertItem (0, "");
    c_aspect -> setCurrentIndex (0);
  }
  c_aspect -> setEnabled (autoexpandChecked);
}

void KPlayerSettingsSubtitles::autoloadSubtitlesChanged (bool autoloadSubtitlesChecked)
{
  if ( autoloadSubtitlesChecked )
  {
    c_subtitles_list -> setText (configuration() -> autoloadExtensionList());
    c_subtitles_list -> setCursorPosition (0);
  }
  else
    c_subtitles_list -> setText ("");
  l_extensions -> setEnabled (autoloadSubtitlesChecked);
  c_subtitles_list -> setEnabled (autoloadSubtitlesChecked);
  if ( autoloadSubtitlesChecked && m_initialized )
  {
    c_subtitles_list -> setFocus();
    c_subtitles_list -> selectAll();
  }
}

KPlayerSettingsVideo::KPlayerSettingsVideo (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  loadLists();
  load();
  QApplication::connect (engine(), SIGNAL (updated()), this, SLOT (refresh()));
}

void KPlayerSettingsVideo::refresh (void)
{
  int dindex = c_driver -> currentIndex();
  if ( dindex > 0 )
    dindex = engine() -> videoDriverIndex (listEntry (c_driver)) + 1;
  int cindex = c_codec -> currentIndex();
  if ( cindex > 0 )
    cindex = engine() -> videoCodecIndex (listEntry (c_codec)) + 1;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "Video Driver: " << c_driver -> currentText()
    << " => " << dindex << " / " << engine() -> videoDriverCount() << "\n";
  kdDebugTime() << "Video Codec: " << c_codec -> currentText()
    << " => " << cindex << " / " << engine() -> videoCodecCount() << "\n";
#endif
  loadLists();
  c_driver -> setCurrentIndex (dindex);
  driverChanged (dindex);
  c_codec -> setCurrentIndex (cindex);
}

void KPlayerSettingsVideo::loadLists (void)
{
  if ( engine() -> videoCodecCount() )
  {
    c_codec -> clear();
    c_codec -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> videoCodecCount(); i ++ )
      c_codec -> addItem (engine() -> videoCodecName (i));
  }
  if ( engine() -> videoDriverCount() )
  {
    c_driver -> clear();
    c_driver -> addItem (i18n("auto"));
    for ( int i = 0; i < engine() -> videoDriverCount(); i ++ )
      c_driver -> addItem (engine() -> videoDriverName (i));
  }
}

void KPlayerSettingsVideo::load (void)
{
  c_driver -> setCurrentIndex (engine() -> videoDriverIndex (configuration() -> videoDriver()) + 1);
  driverChanged (c_driver -> currentIndex());
  c_codec -> setCurrentIndex (engine() -> videoCodecIndex (configuration() -> videoCodec()) + 1);
  c_scaler -> setCurrentIndex (configuration() -> videoScaler());
  c_doublebuffering -> setChecked (configuration() -> videoDoubleBuffering());
  c_direct_rendering -> setChecked (configuration() -> videoDirectRendering());
}

void KPlayerSettingsVideo::save (void)
{
  configuration() -> setVideoDriver (listEntry (c_driver));
  if ( c_driver -> currentIndex() > 0 )
    configuration() -> setVideoDevice (c_device -> text());
  configuration() -> setVideoCodec (listEntry (c_codec));
  configuration() -> setVideoScaler (c_scaler -> currentIndex());
  configuration() -> setVideoDoubleBuffering (c_doublebuffering -> isChecked());
  configuration() -> setVideoDirectRendering (c_direct_rendering -> isChecked());
}

void KPlayerSettingsVideo::driverChanged (int index)
{
  c_device -> setText (index > 0 ? configuration() -> videoDevice() : "");
  c_device -> setEnabled (index > 0);
}

KPlayerSettingsProgress::KPlayerSettingsProgress (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  c_progress_seek_units -> insertItem(KPlayerSettingsProgress::percent, i18np(" percent", " percent", 0));
  c_progress_seek_units -> insertItem(KPlayerSettingsProgress::seconds, i18np(" second", " seconds", 0));
  c_progress_fast_units -> insertItem(KPlayerSettingsProgress::percent, i18np(" percent", " percent", 0));
  c_progress_fast_units -> insertItem(KPlayerSettingsProgress::seconds, i18np(" second", " seconds", 0));
  load();
}

void KPlayerSettingsProgress::load (void)
{
  c_progress_seek -> setValue (configuration() -> progressNormalSeek());
  normalSeekAmountChanged (configuration() -> progressNormalSeek());
  c_progress_seek_units -> setCurrentIndex (configuration() -> progressNormalSeekUnits());
  c_progress_fast -> setValue (configuration() -> progressFastSeek());
  fastSeekAmountChanged (configuration() -> progressFastSeek());
  c_progress_fast_units -> setCurrentIndex (configuration() -> progressFastSeekUnits());
}

void KPlayerSettingsProgress::save (void)
{
  configuration() -> setProgressNormalSeekUnits (c_progress_seek_units -> currentIndex());
  configuration() -> setProgressFastSeekUnits (c_progress_fast_units -> currentIndex());
  configuration() -> setProgressNormalSeek (c_progress_seek -> value());
  configuration() -> setProgressFastSeek (c_progress_fast -> value());
}

void KPlayerSettingsProgress::normalSeekAmountChanged (int value)
{
  c_progress_seek_units -> setItemText(KPlayerSettingsProgress::percent, i18np(" percent", " percent", value));
  c_progress_seek_units -> setItemText(KPlayerSettingsProgress::seconds, i18np(" second", " seconds", value));
}

void KPlayerSettingsProgress::fastSeekAmountChanged (int value)
{
  c_progress_fast_units -> setItemText(KPlayerSettingsProgress::percent, i18np(" percent", " percent", value));
  c_progress_fast_units -> setItemText(KPlayerSettingsProgress::seconds, i18np(" second", " seconds", value));
}

void KPlayerSettingsProgress::normalSeekUnitsChanged ()
{
  if ( c_progress_seek_units -> currentIndex() ==  KPlayerSettingsProgress::percent)
    c_progress_seek -> setMaximum (100);
  if ( c_progress_seek_units -> currentIndex() ==  KPlayerSettingsProgress::seconds)
    c_progress_seek -> setMaximum (9999);
}

void KPlayerSettingsProgress::fastSeekUnitsChanged ()
{
  if ( c_progress_fast_units -> currentIndex() ==  KPlayerSettingsProgress::percent)
    c_progress_fast -> setMaximum (100);
  if ( c_progress_fast_units -> currentIndex() ==  KPlayerSettingsProgress::seconds)
    c_progress_fast -> setMaximum (9999);
}

KPlayerSettingsVolume::KPlayerSettingsVolume (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsVolume::load (void)
{
  c_volume_minimum -> setValue (configuration() -> volumeMinimum());
  c_volume_maximum -> setValue (configuration() -> volumeMaximum());
  c_volume_step -> setValue (configuration() -> volumeStep());
  c_volume_reset -> setChecked (configuration() -> volumeReset());
  resetChanged (configuration() -> volumeReset());
}

void KPlayerSettingsVolume::save (void)
{
  configuration() -> setVolumeMinimumMaximum (c_volume_minimum -> value(), c_volume_maximum -> value());
  configuration() -> setVolumeStep (c_volume_step -> value());
  configuration() -> setVolumeReset (c_volume_reset -> isChecked());
  if ( configuration() -> volumeReset() )
  {
    configuration() -> setInitialVolume (c_volume_default -> value());
    configuration() -> setVolumeEvery (c_volume_every -> currentIndex());
  }
}

void KPlayerSettingsVolume::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_volume_default -> setValue (configuration() -> initialVolume());
    c_volume_every -> setCurrentIndex (0);
    c_volume_every -> setItemText (c_volume_every -> currentIndex(), i18n("file"));
    c_volume_every -> setCurrentIndex (configuration() -> volumeEvery());
  }
  else
  {
    c_volume_every -> setCurrentIndex (0);
    c_volume_every -> setItemText (c_volume_every -> currentIndex(), "");
  }
  c_volume_default -> setEnabled (resetChecked);
  c_volume_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_volume_default -> setFocus();
    c_volume_default -> selectAll();
  }
}

void KPlayerSettingsVolume::minimumVolumeValueChanged (int value)
{
  c_volume_default -> setMinimum (value);
  if ( value >= c_volume_maximum -> value() )
    c_volume_maximum -> setValue (value+1);
  c_volume_step -> setMaximum (c_volume_maximum -> value() - value);
}

void KPlayerSettingsVolume::maximumVolumeValueChanged (int value)
{
  c_volume_default -> setMaximum (value);
  if ( value <= c_volume_minimum -> value() )
    c_volume_minimum -> setValue (value-1);
  c_volume_step -> setMaximum (value - c_volume_minimum -> value());
}

KPlayerSettingsContrast::KPlayerSettingsContrast (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsContrast::load (void)
{
  c_contrast_minimum -> setValue (configuration() -> contrastMinimum());
  c_contrast_maximum -> setValue (configuration() -> contrastMaximum());
  c_contrast_step -> setValue (configuration() -> contrastStep());
  c_contrast_reset -> setChecked (configuration() -> contrastReset());
  resetChanged (configuration() -> contrastReset());
}

void KPlayerSettingsContrast::save (void)
{
  configuration() -> setContrastMinimumMaximum (c_contrast_minimum -> value(), c_contrast_maximum -> value());
  configuration() -> setContrastStep (c_contrast_step -> value());
  configuration() -> setContrastReset (c_contrast_reset -> isChecked());
  if ( configuration() -> contrastReset() )
  {
    configuration() -> setInitialContrast (c_contrast_default -> value());
    configuration() -> setContrastEvery (c_contrast_every -> currentIndex());
  }
}

void KPlayerSettingsContrast::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_contrast_default -> setValue (configuration() -> initialContrast());
    c_contrast_every -> setCurrentIndex (0);
    c_contrast_every -> setItemText (c_contrast_every -> currentIndex(), i18n("file"));
    c_contrast_every -> setCurrentIndex (configuration() -> contrastEvery());
  }
  else
  {
    c_contrast_every -> setCurrentIndex (0);
    c_contrast_every -> setItemText (c_contrast_every -> currentIndex(), "");
  }
  c_contrast_default -> setEnabled (resetChecked);
  c_contrast_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_contrast_default -> setFocus();
    c_contrast_default -> selectAll();
  }
}

void KPlayerSettingsContrast::minimumContrastValueChanged (int value)
{
  c_contrast_default -> setMinimum (value);
  if ( value >= c_contrast_maximum -> value() )
    c_contrast_maximum -> setValue (value+1);
  c_contrast_step -> setMaximum (c_contrast_maximum -> value() - value);
}

void KPlayerSettingsContrast::maximumContrastValueChanged (int value)
{
  c_contrast_default -> setMaximum (value);
  if ( value <= c_contrast_minimum -> value() )
    c_contrast_minimum -> setValue (value-1);
  c_contrast_step -> setMaximum (value - c_contrast_minimum -> value());
}

KPlayerSettingsBrightness::KPlayerSettingsBrightness (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsBrightness::load (void)
{
  c_brightness_minimum -> setValue (configuration() -> brightnessMinimum());
  c_brightness_maximum -> setValue (configuration() -> brightnessMaximum());
  c_brightness_step -> setValue (configuration() -> brightnessStep());
  c_brightness_reset -> setChecked (configuration() -> brightnessReset());
  resetChanged (configuration() -> brightnessReset());
}

void KPlayerSettingsBrightness::save (void)
{
  configuration() -> setBrightnessMinimumMaximum (c_brightness_minimum -> value(), c_brightness_maximum -> value());
  configuration() -> setBrightnessStep (c_brightness_step -> value());
  configuration() -> setBrightnessReset (c_brightness_reset -> isChecked());
  if ( configuration() -> brightnessReset() )
  {
    configuration() -> setInitialBrightness (c_brightness_default -> value());
    configuration() -> setBrightnessEvery (c_brightness_every -> currentIndex());
  }
}

void KPlayerSettingsBrightness::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_brightness_default -> setValue (configuration() -> initialBrightness());
    c_brightness_every -> setCurrentIndex (0);
    c_brightness_every -> setItemText (c_brightness_every -> currentIndex(), i18n("file"));
    c_brightness_every -> setCurrentIndex (configuration() -> brightnessEvery());
  }
  else
  {
    c_brightness_every -> setCurrentIndex (0);
    c_brightness_every -> setItemText (c_brightness_every -> currentIndex(), "");
  }
  c_brightness_default -> setEnabled (resetChecked);
  c_brightness_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_brightness_default -> setFocus();
    c_brightness_default -> selectAll();
  }
}

void KPlayerSettingsBrightness::minimumBrightnessValueChanged (int value)
{
  c_brightness_default -> setMinimum (value);
  if ( value >= c_brightness_maximum -> value() )
    c_brightness_maximum -> setValue (value+1);
  c_brightness_step -> setMaximum (c_brightness_maximum -> value() - value);
}

void KPlayerSettingsBrightness::maximumBrightnessValueChanged (int value)
{
  c_brightness_default -> setMaximum (value);
  if ( value <= c_brightness_minimum -> value() )
    c_brightness_minimum -> setValue (value-1);
  c_brightness_step -> setMaximum (value - c_brightness_minimum -> value());
}

KPlayerSettingsHue::KPlayerSettingsHue (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsHue::load (void)
{
  c_hue_minimum -> setValue (configuration() -> hueMinimum());
  c_hue_maximum -> setValue (configuration() -> hueMaximum());
  c_hue_step -> setValue (configuration() -> hueStep());
  c_hue_reset -> setChecked (configuration() -> hueReset());
  resetChanged (configuration() -> hueReset());
}

void KPlayerSettingsHue::save (void)
{
  configuration() -> setHueMinimumMaximum (c_hue_minimum -> value(), c_hue_maximum -> value());
  configuration() -> setHueStep (c_hue_step -> value());
  configuration() -> setHueReset (c_hue_reset -> isChecked());
  if ( configuration() -> hueReset() )
  {
    configuration() -> setInitialHue (c_hue_default -> text().toInt());
    configuration() -> setHueEvery (c_hue_every -> currentIndex());
  }
}

void KPlayerSettingsHue::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_hue_default -> setValue (configuration() -> initialHue());
    c_hue_every -> setCurrentIndex (0);
    c_hue_every -> setItemText (c_hue_every -> currentIndex(), i18n("file"));
    c_hue_every -> setCurrentIndex (configuration() -> hueEvery());
  }
  else
  {
    c_hue_every -> setCurrentIndex (0);
    c_hue_every -> setItemText (c_hue_every -> currentIndex(), "");
  }
  c_hue_default -> setEnabled (resetChecked);
  c_hue_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_hue_default -> setFocus();
    c_hue_default -> selectAll();
  }
}

void KPlayerSettingsHue::minimumHueValueChanged (int value)
{
  c_hue_default -> setMinimum (value);
  if ( value >= c_hue_maximum -> value() )
    c_hue_maximum -> setValue (value+1);
  c_hue_step -> setMaximum (c_hue_maximum -> value() - value);
}

void KPlayerSettingsHue::maximumHueValueChanged (int value)
{
  c_hue_default -> setMaximum (value);
  if ( value <= c_hue_minimum -> value() )
    c_hue_minimum -> setValue (value-1);
  c_hue_step -> setMaximum (value - c_hue_minimum -> value());
}

KPlayerSettingsSaturation::KPlayerSettingsSaturation (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsSaturation::load (void)
{
  c_saturation_minimum -> setValue (configuration() -> saturationMinimum());
  c_saturation_maximum -> setValue (configuration() -> saturationMaximum());
  c_saturation_step -> setValue (configuration() -> saturationStep());
  c_saturation_reset -> setChecked (configuration() -> saturationReset());
  resetChanged (configuration() -> saturationReset());
}

void KPlayerSettingsSaturation::save (void)
{
  configuration() -> setSaturationMinimumMaximum (c_saturation_minimum -> value(), c_saturation_maximum -> value());
  configuration() -> setSaturationStep (c_saturation_step -> value());
  configuration() -> setSaturationReset (c_saturation_reset -> isChecked());
  if ( configuration() -> saturationReset() )
  {
    configuration() -> setInitialSaturation (c_saturation_default -> text().toInt());
    configuration() -> setSaturationEvery (c_saturation_every -> currentIndex());
  }
}

void KPlayerSettingsSaturation::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_saturation_default -> setValue (configuration() -> initialSaturation());
    c_saturation_every -> setCurrentIndex (0);
    c_saturation_every -> setItemText (c_saturation_every -> currentIndex(), i18n("file"));
    c_saturation_every -> setCurrentIndex (configuration() -> saturationEvery());
  }
  else
  {
    c_saturation_every -> setCurrentIndex (0);
    c_saturation_every -> setItemText (c_saturation_every -> currentIndex(), "");
  }
  c_saturation_default -> setEnabled (resetChecked);
  c_saturation_every -> setEnabled (resetChecked);
  if ( resetChecked && sender() )
  {
    c_saturation_default -> setFocus();
    c_saturation_default -> selectAll();
  }
}

void KPlayerSettingsSaturation::minimumSaturationValueChanged (int value)
{
  c_saturation_default -> setMinimum (value);
  if ( value >= c_saturation_maximum -> value() )
    c_saturation_maximum -> setValue (value+1);
  c_saturation_step -> setMaximum (c_saturation_maximum -> value() - value);
}

void KPlayerSettingsSaturation::maximumSaturationValueChanged (int value)
{
  c_saturation_default -> setMaximum (value);
  if ( value <= c_saturation_minimum -> value() )
    c_saturation_minimum -> setValue (value-1);
  c_saturation_step -> setMaximum (value - c_saturation_minimum -> value());
}
