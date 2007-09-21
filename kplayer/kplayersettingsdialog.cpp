/***************************************************************************
                          kplayersettingsdialog.cpp
                          -------------------------
    begin                : Tue Apr 22 2003
    copyright            : (C) 2003-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <klocale.h>
#include <kmessagebox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfontdatabase.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qslider.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_SETTINGS_DIALOG
#endif

#include "kplayersettingsdialog.h"
#include "kplayersettingsdialog.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"

static QString s_default_entry (i18n("%1 (%2)"));

KPlayerSettingsDialog::KPlayerSettingsDialog (QWidget* parent)
  : KPageDialog (parent)
{
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
  setButtonGuiItem (KDialog::Cancel, KStandardGuiItem::close());
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
  configuration() -> setCache (c_use_cache -> currentIndex(), labs (c_cache_size -> text().toInt()));
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
  if ( cache == 2 )
    c_cache_size -> setText (QString::number (configuration() -> cacheSize()));
  else
    c_cache_size -> setText ("");
  c_cache_size -> setEnabled (cache == 2);
  l_cache_size_kb -> setEnabled (cache == 2);
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
  setupUi (this);
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
  c_delay_step -> setText (QString::number (configuration() -> audioDelayStep()));
}

void KPlayerSettingsAudio::save (void)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::save\n";
#endif
  configuration() -> setAudioDriver (listEntry (c_driver));
  if ( c_driver -> currentIndex() > 0 )
  {
    configuration() -> setAudioDevice (c_device -> text());
    configuration() -> setMixerDevice (c_mixer -> text());
    configuration() -> setMixerChannel (c_channel -> currentText());
  }
  configuration() -> setSoftwareVolume (c_softvol -> isChecked());
  if ( configuration() -> softwareVolume() )
  {
    int maximum = labs (c_maximum -> text().toInt());
    if ( maximum < 10 )
      configuration() -> resetMaximumSoftwareVolume();
    else
      configuration() -> setMaximumSoftwareVolume (maximum);
  }
  configuration() -> setAudioCodec (listEntry (c_codec));
  float value = fabs (c_delay_step -> text().toFloat());
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
  QString mixer = c_mixer -> text();
  if ( ! mixer.isEmpty() )
  {
    *amixer << "-D" << mixer;
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
    kdDebugTime() << " Device " << mixer << "\n";
#endif
  }
  *amixer << "scontents";
  connect (amixer, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*, int)),
    SLOT (amixerOutput (KPlayerLineOutputProcess*, char*, int)));
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
  bool mixer = device && ! softvol && (driver == "alsa" || driver == "oss" || driver == "sun");
  bool channel = mixer && driver != "sun";
  if ( driver != m_driver )
  {
    bool original = driver == configuration() -> audioDriver();
    c_device -> setText (original && device ? configuration() -> audioDevice() : "");
    m_device = c_device -> text();
    c_mixer -> setText (original && mixer ? configuration() -> mixerDevice() : "");
    if ( channel )
      if ( driver == "oss" )
      {
        c_channel -> clear();
        c_channel -> addItem ("vol");
        c_channel -> addItem ("pcm");
        c_channel -> addItem ("line");
      }
      else if ( ! m_amixer_running )
        runAmixer();
    c_channel -> setEditText (original && channel ? configuration() -> mixerChannel() : "");
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
  kdDebugTime() << " Device " << device << "\n";
#endif
  if ( c_mixer -> isEnabled() && c_mixer -> text() == m_device && listEntry (c_driver) == "alsa" )
    c_mixer -> setText (device);
  m_device = device;
}

void KPlayerSettingsAudio::mixerChanged (const QString&)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::mixerChanged\n";
#endif
  if ( ! c_softvol -> isChecked() && listEntry (c_driver) == "alsa" )
    runAmixer();
}

void KPlayerSettingsAudio::softvolChanged (bool checked)
{
#ifdef DEBUG_KPLAYER_SETTINGS_DIALOG
  kdDebugTime() << "KPlayerSettingsAudio::softvolChanged\n";
  kdDebugTime() << " Checked " << checked << "\n";
#endif
  c_maximum -> setText (checked ? QString::number (configuration() -> maximumSoftwareVolume()) : "");
  c_maximum -> setEnabled (checked);
  QString driver (listEntry (c_driver));
  bool empty = checked || driver != configuration() -> audioDriver()
    || driver != "alsa" && driver != "oss" && driver != "sun";
  c_mixer -> setText (empty ? "" : configuration() -> hasMixerDevice()
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
  resizeAutomaticallyChanged (configuration() -> resizeAutomatically());
  c_playlist_menu_size -> setText (QString::number (configuration() -> playlistMenuSize()));
  c_recent_menu_size -> setText (QString::number (configuration() -> recentMenuSize()));
  c_recent_list_size -> setText (QString::number (configuration() -> recentListSize()));
  c_cache_size_limit -> setText (QString::number (configuration() -> cacheSizeLimit()));
  c_allow_duplicate_entries -> setChecked (configuration() -> allowDuplicateEntries());
  c_show_messages_on_error -> setChecked (configuration() -> showMessagesOnError());
}

void KPlayerSettingsGeneral::save (void)
{
  configuration() -> setResizeAutomatically (c_resize_automatically -> isChecked());
  if ( configuration() -> resizeAutomatically() )
    configuration() -> setMinimumInitialWidth (labs (c_minimum_initial_width -> text().toInt()));
  configuration() -> setPlaylistMenuSize (labs (c_playlist_menu_size -> text().toInt()));
  configuration() -> setRecentMenuSize (labs (c_recent_menu_size -> text().toInt()));
  configuration() -> setRecentListSize (labs (c_recent_list_size -> text().toInt()));
  configuration() -> setCacheSizeLimit (labs (c_cache_size_limit -> text().toInt()));
  configuration() -> setAllowDuplicateEntries (c_allow_duplicate_entries -> isChecked());
  configuration() -> setShowMessagesOnError (c_show_messages_on_error -> isChecked());
}

void KPlayerSettingsGeneral::resizeAutomaticallyChanged (bool resizeAutomaticallyChecked)
{
  if ( resizeAutomaticallyChecked )
    c_minimum_initial_width -> setText (QString::number (configuration() -> minimumInitialWidth()));
  else
    c_minimum_initial_width -> setText ("");
  c_minimum_initial_width -> setEnabled (resizeAutomaticallyChecked);
  l_minimum_initial_width -> setEnabled (resizeAutomaticallyChecked);
}

KPlayerSettingsSliders::KPlayerSettingsSliders (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsSliders::load (void)
{
  c_preferred_slider_length -> setText (QString::number (configuration() -> preferredSliderLength()));
  c_minimum_slider_length -> setText (QString::number (configuration() -> minimumSliderLength()));
  c_show_slider_marks -> setChecked (configuration() -> showSliderMarks());
  showMarksChanged (c_show_slider_marks -> isChecked());
}

void KPlayerSettingsSliders::save (void)
{
  int value = labs (c_preferred_slider_length -> text().toInt());
  if ( value )
    configuration() -> setPreferredSliderLength (value);
  else
    configuration() -> resetPreferredSliderLength();
  value = labs (c_minimum_slider_length -> text().toInt());
  if ( value )
    configuration() -> setMinimumSliderLength (value);
  else
    configuration() -> resetMinimumSliderLength();
  configuration() -> setShowSliderMarks (c_show_slider_marks -> isChecked());
  value = labs (c_slider_marks -> text().toInt());
  if ( value )
    configuration() -> setSliderMarks (value);
  else
    configuration() -> resetSliderMarks();
}

void KPlayerSettingsSliders::showMarksChanged (bool showMarksChecked)
{
  if ( showMarksChecked )
    c_slider_marks -> setText (QString::number (configuration() -> sliderMarks()));
  else
    c_slider_marks -> setText ("");
  l_slider_marks -> setEnabled (showMarksChecked);
  c_slider_marks -> setEnabled (showMarksChecked);
  l_slider_marks_percent -> setEnabled (showMarksChecked);
}

KPlayerSettingsSubtitles::KPlayerSettingsSubtitles (QWidget* parent)
  : QFrame (parent)
{
  m_initialized = m_recursion = false;
  setupUi (this);
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
  while ( it != sizes.end() )
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
  c_outline -> setText (QString::number (configuration() -> subtitleFontOutline()));
  outlineEditChanged (c_outline -> text());
  c_width -> setText (QString::number (configuration() -> subtitleTextWidth()));
  widthEditChanged (c_width -> text());
  c_position_step -> setText (QString::number (configuration() -> subtitlePositionStep()));
  c_delay_step -> setText (QString::number (configuration() -> subtitleDelayStep()));
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
  c_embedded_fonts -> setChecked (configuration() -> subtitleEmbeddedFonts());
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
  configuration() -> setSubtitleFontOutline (c_outline -> text().toFloat());
  configuration() -> setSubtitleTextWidth (c_width -> text().toInt());
  configuration() -> setSubtitlePositionStep (labs (c_position_step -> text().toInt()));
  value = fabs (c_delay_step -> text().toFloat());
  if ( value )
    configuration() -> setSubtitleDelayStep (value);
  else
    configuration() -> resetSubtitleDelayStep();
  if ( c_encoding -> currentIndex() )
    configuration() -> setSubtitleEncoding (c_encoding -> currentText().section (':', 0, 0));
  else
    configuration() -> resetSubtitleEncoding();
  configuration() -> setSubtitleEmbeddedFonts (c_embedded_fonts -> isChecked());
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
    c_width -> setText (QString::number (value));
}

void KPlayerSettingsSubtitles::widthEditChanged (const QString& value)
{
  int number = value.toInt();
  if ( number < 10 )
    number = 10;
  else if ( number > 100 )
    number = 100;
  m_recursion = true;
  c_width_slider -> setValue (number);
  m_recursion = false;
}

void KPlayerSettingsSubtitles::outlineSliderChanged (int value)
{
  if ( ! m_recursion )
    c_outline -> setText (QString::number (float (value) / 20));
}

void KPlayerSettingsSubtitles::outlineEditChanged (const QString& value)
{
  int number = int (value.toFloat() * 20 + 0.5);
  if ( number < 0 )
    number = 0;
  else if ( number > 200 )
    number = 200;
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
    c_aspect -> addItem ("", 0);
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
  load();
}

void KPlayerSettingsProgress::load (void)
{
  c_progress_seek -> setText (QString::number (configuration() -> progressNormalSeek()));
  c_progress_seek_units -> setCurrentIndex (configuration() -> progressNormalSeekUnits());
  c_progress_fast -> setText (QString::number (configuration() -> progressFastSeek()));
  c_progress_fast_units -> setCurrentIndex (configuration() -> progressFastSeekUnits());
}

void KPlayerSettingsProgress::save (void)
{
  configuration() -> setProgressNormalSeekUnits (c_progress_seek_units -> currentIndex());
  configuration() -> setProgressFastSeekUnits (c_progress_fast_units -> currentIndex());
  configuration() -> setProgressNormalSeek (labs (c_progress_seek -> text().toInt()));
  configuration() -> setProgressFastSeek (labs (c_progress_fast -> text().toInt()));
}

KPlayerSettingsVolume::KPlayerSettingsVolume (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsVolume::load (void)
{
  c_volume_minimum -> setText (QString::number (configuration() -> volumeMinimum()));
  c_volume_maximum -> setText (QString::number (configuration() -> volumeMaximum()));
  c_volume_step -> setText (QString::number (configuration() -> volumeStep()));
  c_volume_reset -> setChecked (configuration() -> volumeReset());
  resetChanged (configuration() -> volumeReset());
}

void KPlayerSettingsVolume::save (void)
{
  configuration() -> setVolumeMinimumMaximum (labs (c_volume_minimum -> text().toInt()), labs (c_volume_maximum -> text().toInt()));
  configuration() -> setVolumeStep (labs (c_volume_step -> text().toInt()));
  configuration() -> setVolumeReset (c_volume_reset -> isChecked());
  if ( configuration() -> volumeReset() )
  {
    configuration() -> setInitialVolume (labs (c_volume_default -> text().toInt()));
    configuration() -> setVolumeEvery (c_volume_every -> currentIndex());
  }
}

void KPlayerSettingsVolume::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_volume_default -> setText (QString::number (configuration() -> initialVolume()));
    c_volume_every -> setCurrentIndex (0);
    c_volume_every -> setItemText (c_volume_every -> currentIndex(), i18n("file"));
    c_volume_every -> setCurrentIndex (configuration() -> volumeEvery());
  }
  else
  {
    c_volume_default -> setText ("");
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

KPlayerSettingsContrast::KPlayerSettingsContrast (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsContrast::load (void)
{
  c_contrast_minimum -> setText (QString::number (configuration() -> contrastMinimum()));
  c_contrast_maximum -> setText (QString::number (configuration() -> contrastMaximum()));
  c_contrast_step -> setText (QString::number (configuration() -> contrastStep()));
  c_contrast_reset -> setChecked (configuration() -> contrastReset());
  resetChanged (configuration() -> contrastReset());
}

void KPlayerSettingsContrast::save (void)
{
  configuration() -> setContrastMinimumMaximum (c_contrast_minimum -> text().toInt(), c_contrast_maximum -> text().toInt());
  configuration() -> setContrastStep (labs (c_contrast_step -> text().toInt()));
  configuration() -> setContrastReset (c_contrast_reset -> isChecked());
  if ( configuration() -> contrastReset() )
  {
    configuration() -> setInitialContrast (c_contrast_default -> text().toInt());
    configuration() -> setContrastEvery (c_contrast_every -> currentIndex());
  }
}

void KPlayerSettingsContrast::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_contrast_default -> setText (QString::number (configuration() -> initialContrast()));
    c_contrast_every -> setCurrentIndex (0);
    c_contrast_every -> setItemText (c_contrast_every -> currentIndex(), i18n("file"));
    c_contrast_every -> setCurrentIndex (configuration() -> contrastEvery());
  }
  else
  {
    c_contrast_default -> setText ("");
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

KPlayerSettingsBrightness::KPlayerSettingsBrightness (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsBrightness::load (void)
{
  c_brightness_minimum -> setText (QString::number (configuration() -> brightnessMinimum()));
  c_brightness_maximum -> setText (QString::number (configuration() -> brightnessMaximum()));
  c_brightness_step -> setText (QString::number (configuration() -> brightnessStep()));
  c_brightness_reset -> setChecked (configuration() -> brightnessReset());
  resetChanged (configuration() -> brightnessReset());
}

void KPlayerSettingsBrightness::save (void)
{
  configuration() -> setBrightnessMinimumMaximum (c_brightness_minimum -> text().toInt(), c_brightness_maximum -> text().toInt());
  configuration() -> setBrightnessStep (labs (c_brightness_step -> text().toInt()));
  configuration() -> setBrightnessReset (c_brightness_reset -> isChecked());
  if ( configuration() -> brightnessReset() )
  {
    configuration() -> setInitialBrightness (c_brightness_default -> text().toInt());
    configuration() -> setBrightnessEvery (c_brightness_every -> currentIndex());
  }
}

void KPlayerSettingsBrightness::resetChanged (bool resetChecked)
{
  if ( resetChecked )
  {
    c_brightness_default -> setText (QString::number (configuration() -> initialBrightness()));
    c_brightness_every -> setCurrentIndex (0);
    c_brightness_every -> setItemText (c_brightness_every -> currentIndex(), i18n("file"));
    c_brightness_every -> setCurrentIndex (configuration() -> brightnessEvery());
  }
  else
  {
    c_brightness_default -> setText ("");
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

KPlayerSettingsHue::KPlayerSettingsHue (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsHue::load (void)
{
  c_hue_minimum -> setText (QString::number (configuration() -> hueMinimum()));
  c_hue_maximum -> setText (QString::number (configuration() -> hueMaximum()));
  c_hue_step -> setText (QString::number (configuration() -> hueStep()));
  c_hue_reset -> setChecked (configuration() -> hueReset());
  resetChanged (configuration() -> hueReset());
}

void KPlayerSettingsHue::save (void)
{
  configuration() -> setHueMinimumMaximum (c_hue_minimum -> text().toInt(), c_hue_maximum -> text().toInt());
  configuration() -> setHueStep (labs (c_hue_step -> text().toInt()));
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
    c_hue_default -> setText (QString::number (configuration() -> initialHue()));
    c_hue_every -> setCurrentIndex (0);
    c_hue_every -> setItemText (c_hue_every -> currentIndex(), i18n("file"));
    c_hue_every -> setCurrentIndex (configuration() -> hueEvery());
  }
  else
  {
    c_hue_default -> setText ("");
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

KPlayerSettingsSaturation::KPlayerSettingsSaturation (QWidget* parent)
  : QFrame (parent)
{
  setupUi (this);
  load();
}

void KPlayerSettingsSaturation::load (void)
{
  c_saturation_minimum -> setText (QString::number (configuration() -> saturationMinimum()));
  c_saturation_maximum -> setText (QString::number (configuration() -> saturationMaximum()));
  c_saturation_step -> setText (QString::number (configuration() -> saturationStep()));
  c_saturation_reset -> setChecked (configuration() -> saturationReset());
  resetChanged (configuration() -> saturationReset());
}

void KPlayerSettingsSaturation::save (void)
{
  configuration() -> setSaturationMinimumMaximum (c_saturation_minimum -> text().toInt(), c_saturation_maximum -> text().toInt());
  configuration() -> setSaturationStep (labs (c_saturation_step -> text().toInt()));
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
    c_saturation_default -> setText (QString::number (configuration() -> initialSaturation()));
    c_saturation_every -> setCurrentIndex (0);
    c_saturation_every -> setItemText (c_saturation_every -> currentIndex(), i18n("file"));
    c_saturation_every -> setCurrentIndex (configuration() -> saturationEvery());
  }
  else
  {
    c_saturation_default -> setText ("");
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
