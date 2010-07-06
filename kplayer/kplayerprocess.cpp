/***************************************************************************
                          kplayerprocess.cpp
                          ------------------
    begin                : Sat Jan 11 2003
    copyright            : (C) 2002-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kio/jobuidelegate.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <unistd.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_PROCESS
//#define DEBUG_KPLAYER_PROGRESS
#define DEBUG_KPLAYER_HELPER
//#define DEBUG_KPLAYER_LINEOUT
//#define DEBUG_KPLAYER_KIOSLAVE
//#define DEBUG_KPLAYER_DUMP
#endif

#include "kplayerprocess.h"
#include "kplayerengine.h"
#include "kplayersettings.h"
#include "kplayerwidget.h"

#define MIN_VIDEO_LENGTH 5
#define NO_SEEK_ORIGIN -5

#ifdef DEBUG_KPLAYER_DUMP
static QFile s_dump (QDir::homePath() + "/kioslave.dump");
#endif

static QRegExp re_ext ("^[A-Za-z0-9]+$");
static QRegExp re_a_or_v ("^[AV]: *([0-9,:.-]+)");
static QRegExp re_a_and_v ("^A: *([0-9,:.-]+) +V: *([0-9,:.-]+)");
static QRegExp re_start ("^(?:Start playing|Starting playback|ZaËÌn·m p¯ehr·vat|Starte Wiedergabe|PÂbegynder afspilning|≈ÍÍﬂÌÁÛÁ ·Ì··Ò·„˘„ﬁÚ|Empezando reproducciÛn|DÈmarre la lecture|Lej·tsz·s indÌt·sa|Inizio la riproduzione|∫∆¿∏≥´ªœ|Ïû¨ÏÉùÏùÑ ÏãúÏûëÌï©ÎãàÎã§|–ü–æ—á–Ω—É–≤–∞ –ø–ª–µ—ò–±–µ–∫–æ—Ç|Start afspelen|Starter avspilling|Zaczynam odtwarzanie|InÌciando reproduÁ„o|Rulez|Ó¡ﬁ¡Ãœ ◊œc–“œ…⁄◊≈ƒ≈Œ…—|ZaËÌnam prehr·vaª|«almaya ba˛lan˝yor|œﬁ¡‘œÀ –“œ«“¡◊¡ŒŒ—|ø™ º≤•∑≈|∂\\}©lºΩ©Ò)\\.\\.\\.", Qt::CaseInsensitive);
//static QRegExp re_playing ("(?:^(?:Playing|P¯ehr·v·m|Spiele|Afspiller|¡Ì··Ò·„˘„ﬁ ÙÔı|Reproduciendo|Joue|In riproduzione|–ü—É—à—Ç–µ–Ω–æ|Bezig met het afspelen van|Spiller|Odtwarzam|Reproduzindo|Rulez|“œ…«“Ÿ◊¡Œ…≈|Prehr·vam|“œ«“¡◊¡ŒŒ—|≤•∑≈|•ø¶bºΩ©Ò) | (?:lej·tsz·sa|§Ú∫∆¿∏√Ê|Ïû¨ÏÉù Ï§ë|«al˝n˝yor)\\.*$)", Qt::CaseInsensitive);
static QRegExp re_exiting ("^(?:Exiting|»ÁÎËÁ‡Ï|KonËÌm|Beende| ?Afslutter| ?∏ÓÔ‰ÔÚ|Saliendo|Sortie|KilÈpek|In uscita|Ω™Œª§∑§∆§§§ﬁ§π|Ï¢ÖÎ£åÌï©ÎãàÎã§.|–ò–∑–ª–µ–≥—É–|Bezig met afsluiten|Avslutter|WychodzÍ|Saindo|Ie∫ire|˜Ÿ»œƒ…Õ|KonËÌm|«˝k˝l˝yor|˜…»œƒ…Õœ|’˝‘⁄ÕÀ≥ˆ|•ø¶b∞h•X)", Qt::CaseInsensitive);
static QRegExp re_quit ("^(?:Exiting|»ÁÎËÁ‡Ï|KonËÌm|Beende| ?Afslutter| ?∏ÓÔ‰ÔÚ|Saliendo|Sortie|KilÈpek|In uscita|Ω™Œª§∑§∆§§§ﬁ§π|Ï¢ÖÎ£åÌï©ÎãàÎã§.|–ò–∑–ª–µ–≥—É–|Bezig met afsluiten|Avslutter|WychodzÍ|Saindo|Ie∫ire|˜Ÿ»œƒ…Õ|KonËÌm|«˝k˝l˝yor|˜…»œƒ…Õœ|’˝‘⁄ÕÀ≥ˆ|•ø¶b∞h•X)\\.\\.\\. \\((?:Quit| ‡È|Konec|Ende|Afslut| ÎÂﬂÛÈÏÔ|Salida\\.?|Fin|KilÈpÈs|Uscita|Ω™Œª|Ï¢ÖÎ£å|–û—Ç–∫–∞–∂–∏|Stop|Avslutt|Wyj∂cie|Sair|Ie∫ire|˜Ÿ»œƒ|Koniec|«˝k˝˛|˜…»¶ƒ|ÕÀ≥ˆ|¬˜∂\\})\\)", Qt::CaseInsensitive);
static QRegExp re_success ("^(?:Exiting|»ÁÎËÁ‡Ï|KonËÌm|Beende| ?Afslutter| ?∏ÓÔ‰ÔÚ|Saliendo|Sortie|KilÈpek|In uscita|Ω™Œª§∑§∆§§§ﬁ§π|¡æ∑·«’¥œ¥Ÿ|––∑–ª–µ–≥—–≤–∞|Bezig met afsluiten|Avslutter|WychodzÍ|Saindo|Ie∫ire|˜Ÿ»œƒ…Õ|KonËÌm|«˝k˝l˝yor|˜…»œƒ…Õœ|’˝‘⁄ÕÀ≥ˆ|•ø¶b∞h•X)\\.\\.\\. \\((?:End of file| ‡È Ì‡ Ù‡ÈÎ‡|Konec souboru|Ende der Datei|Slut pÂ filen|‘›ÎÔÚ ÙÔı ·Ò˜ÂﬂÔı|Fin del archivo\\.?|Fin du fichier|VÈge a file-nak|Fine del file|•’•°•§•Î§ŒÀˆ√º§«§π|ÌååÏùºÏùò ÎÅù|–ö—Ä–∞—ò –Ω–∞ –¥–∞—Ç–æ—Ç–µ–∫–∞—Ç–∞|Einde van bestand|Slutt pÂ filen|Koniec pliku|Fim do arquivo|Sf‚r∫it fi∫ier|ÎœŒ≈√ ∆¡ Ã¡|Koniec s˙boru|Dosyan˝n Sonu|Î¶Œ≈√ÿ ∆¡ Ã’|Œƒº˛Ω· ¯|¿…Æ◊•Ω∫›)\\)", Qt::CaseInsensitive);
static QRegExp re_cache_fill ("^Cache fill: *([0-9]+[.,]?[0-9]*) *%", Qt::CaseInsensitive);
static QRegExp re_generating_index ("^Generating Index: *([0-9]+[.,]?[0-9]*) *%", Qt::CaseInsensitive);
static QRegExp re_mpeg12 ("mpeg[12]", Qt::CaseInsensitive);
static QRegExp re_version ("^MPlayer *0\\.9.* \\(C\\) ");
static QRegExp re_crash ("^ID_SIGNAL=([0-9]+)$");
static QRegExp re_paused ("^ID_PAUSED$");

static QByteArray command_quit ("quit\n");
static QByteArray command_pause ("pause\n");
static QByteArray command_visibility ("sub_visibility\n");
static QByteArray command_seek_100 ("seek 100 1\n");
static QByteArray command_seek_99 ("seek 99 1\n");
static QByteArray command_seek_95 ("seek 95 1\n");
static QByteArray command_seek_90 ("seek 90 1\n");
static QByteArray command_seek_50 ("seek 50 1\n");

KPlayerLineOutputProcess::KPlayerLineOutputProcess (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Creating MPlayer process\n";
#endif
  m_stdout_line_length = m_stderr_line_length = 0;
  m_stdout_buffer_length = m_stderr_buffer_length = 129;
  m_stdout_buffer = new char [m_stdout_buffer_length];
  m_stderr_buffer = new char [m_stderr_buffer_length];
#if 0
  m_merge = false;
#endif
  setOutputChannelMode (KProcess::SeparateChannels);
  connect (this, SIGNAL (readyReadStandardOutput()), SLOT (readStandardOutput()));
  connect (this, SIGNAL (readyReadStandardError()), SLOT (readStandardError()));
  connect (this, SIGNAL (finished (int, QProcess::ExitStatus)), SLOT (processHasExited (int, QProcess::ExitStatus)));
  connect (this, SIGNAL (error (QProcess::ProcessError)), SLOT (processHasErrored (QProcess::ProcessError)));
}

KPlayerLineOutputProcess::~KPlayerLineOutputProcess()
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Destroying MPlayer process\n";
#endif
  delete m_stdout_buffer;
  delete m_stderr_buffer;
}

void KPlayerLineOutputProcess::processHasExited (int, QProcess::ExitStatus)
{
  if ( m_stdout_line_length )
    emit receivedStdoutLine (this, m_stdout_buffer);
  if ( m_stderr_line_length )
    emit receivedStderrLine (this, m_stderr_buffer);
  m_stdout_line_length = m_stderr_line_length = 0;
  emit processFinished (this);
}

void KPlayerLineOutputProcess::processHasErrored (QProcess::ProcessError error)
{
  if ( error == QProcess::FailedToStart )
    emit processFinished (this);
}

void KPlayerLineOutputProcess::readStandardOutput (void)
{
  QByteArray ba (readAllStandardOutput());
#ifdef DEBUG_KPLAYER_LINEOUT
  kdDebugTime() << "StdOut: " << ba.length() << " '" << ba.data() << "'\n";
#endif
  receivedOutput (ba.data(), ba.length(), m_stdout_buffer, m_stdout_buffer_length, m_stdout_line_length, true);
}

void KPlayerLineOutputProcess::readStandardError (void)
{
#if 0
  if ( m_merge )
    slotReceivedStdout (proc, str, len);
  else
#endif
  QByteArray ba (readAllStandardOutput());
  receivedOutput (ba.data(), ba.length(), m_stderr_buffer, m_stderr_buffer_length, m_stderr_line_length, false);
}

void KPlayerLineOutputProcess::receivedOutput (char* str, int len, char* buf, int blen, int llen, bool bstdout)
{
  static int avlen = 0;
  static char* av = 0;
#ifdef DEBUG_KPLAYER_LINEOUT
  kdDebugTime() << "stdout received length: " << len << "\n";
  kdDebugTime() << llen << "/" << blen << ": " << buf << "\n";
#endif
  while ( len > 0 && ! str [len - 1] )
    len --;
  while ( len > 0 )
  {
    char* lf = (char*) memchr (str, '\n', len);
    if ( ! lf )
      lf = str + len;
    char* eol = (char*) memchr (str, '\r', lf - str);
    if ( ! eol )
      eol = lf;
    if ( eol - str + llen >= blen )
    {
      char* old_buffer = buf;
      blen = eol - str + llen + 10;
#ifdef DEBUG_KPLAYER_LINEOUT
      kdDebugTime() << "new buffer length: " << blen << "\n";
#endif
      buf = new char [blen];
      if ( bstdout )
      {
        m_stdout_buffer = buf;
        m_stdout_buffer_length = blen;
      }
      else
      {
        m_stderr_buffer = buf;
        m_stderr_buffer_length = blen;
      }
      if ( llen )
        memcpy (buf, old_buffer, llen);
      delete[] old_buffer;
    }
    if ( eol > str )
    {
      memcpy (buf + llen, str, eol - str);
      llen += eol - str;
      if ( bstdout )
        m_stdout_line_length = llen;
      else
        m_stderr_line_length = llen;
    }
    buf [llen] = 0;
    if ( eol - str == len )
      break;
    if ( av && *av && re_paused.indexIn (buf) >= 0 )
    {
#ifdef DEBUG_KPLAYER_LINEOUT
      kdDebugTime() << "Sending AV Buffer On Pause: '" << av << "'\n";
#endif
      if ( bstdout )
        emit receivedStdoutLine (this, av);
      else
        emit receivedStderrLine (this, av);
      *av = 0;
    }
    if ( re_a_or_v.indexIn (buf) >= 0 || re_cache_fill.indexIn (buf) >= 0 || re_generating_index.indexIn (buf) >= 0 )
    {
      if ( avlen <= llen )
      {
        if ( av )
          delete[] av;
        avlen = llen + 10;
        av = new char [avlen];
#ifdef DEBUG_KPLAYER_LINEOUT
        kdDebugTime() << "new av buffer length: " << avlen << "\n";
#endif
      }
      memcpy (av, buf, llen + 1);
#ifdef DEBUG_KPLAYER_LINEOUT
      kdDebugTime() << "AV Buffer: '" << av << "'\n";
#endif
    }
    else if ( bstdout )
      emit receivedStdoutLine (this, buf);
    else
      emit receivedStderrLine (this, buf);
#ifdef DEBUG_KPLAYER_LINEOUT
    kdDebugTime() << "Buffer: '" << buf << "'\n";
#endif
    llen = 0;
    if ( bstdout )
      m_stdout_line_length = llen;
    else
      m_stderr_line_length = llen;
    len -= eol - str + 1;
    str = eol + 1;
  }
  if ( av && *av )
  {
#ifdef DEBUG_KPLAYER_LINEOUT
    kdDebugTime() << "Sending AV Buffer: '" << av << "'\n";
#endif
    if ( bstdout )
      emit receivedStdoutLine (this, av);
    else
      emit receivedStderrLine (this, av);
    *av = 0;
  }
//kdDebugTime() << "normal return\n";
}

inline KPlayerSettings* KPlayerProcess::settings (void) const
{
  return KPlayerEngine::engine() -> settings();
}

inline KPlayerTrackProperties* KPlayerProcess::properties (void) const
{
  return settings() -> properties();
}

inline KPlayerConfiguration* KPlayerProcess::configuration (void) const
{
  return KPlayerEngine::engine() -> configuration();
}

KPlayerProcess::KPlayerProcess (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Creating process\n";
#endif
  m_player = m_helper = 0;
  m_temporary_file = 0;
  m_state = Idle;
  m_pausing = m_paused = m_quit = m_kill = m_seek = m_success = m_size_sent = m_info_available = false;
  m_delayed_player = m_delayed_helper = m_sent = m_send_seek = false;
  m_seekable = m_09_version = m_first_chunk = false;
  m_position = m_max_position = m_helper_position = 0;
  m_seek_origin = NO_SEEK_ORIGIN;
  m_helper_seek = m_helper_seek_count = m_absolute_seek = m_seek_count = m_sent_count = m_cache_size = 0;
  m_slave_job = m_temp_job = 0;
  m_send_volume = m_send_contrast = m_send_brightness = m_send_hue = m_send_saturation = false;
  m_send_frame_drop = m_send_audio_id = m_send_subtitle_load = m_send_subtitle_visibility = false;
  m_send_audio_delay = m_send_subtitle_delay = m_send_subtitle_position = 0;
  m_audio_delay = m_subtitle_delay = m_subtitle_position = 0;
  m_audio_id = m_subtitle_index = -1;
  m_send_subtitle_index = -2;
  m_subtitle_visibility = true;
  m_fifo_handle = -1;
  m_fifo_offset = 0;
  m_fifo_notifier = 0;
  m_fifo_timer = 0;
  QString home (QDir::homePath());
  QDir (home).mkdir (".mplayer");
}

KPlayerProcess::~KPlayerProcess()
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Destroying process\n";
#endif
  if ( m_player )
    delete m_player;
  if ( m_helper )
    delete m_helper;
  if ( m_slave_job )
    m_slave_job -> kill (KJob::Quietly);
  if ( m_temp_job )
    m_temp_job -> kill (KJob::Quietly);
  if ( m_temporary_file )
  {
    m_temporary_file -> close();
    delete m_temporary_file;
  }
  removeDataFifo();
}

void KPlayerProcess::transferTemporaryFile (void)
{
  if ( properties() -> useKioslave() && properties() -> useTemporaryFile() && ! m_temporary_file )
  {
    QFileInfo fi (properties() -> url().fileName());
    QString extension (fi.suffix().toLower());
    m_temporary_file = new KTemporaryFile;
    if ( ! extension.isEmpty() )
      m_temporary_file -> setSuffix ("." + extension);
    m_temporary_file -> open();
#ifdef DEBUG_KPLAYER_PROCESS
    if ( m_temporary_file )
      kdDebugTime() << "Temporary file " << m_temporary_file -> handle() << " " << m_temporary_file -> fileName() << "\n";
    kdDebugTime() << "Process: Creating temp job\n";
#endif
    m_temp_job = KIO::get (properties() -> url(), KIO::NoReload, KIO::HideProgressInfo);
    m_temp_job -> ui() -> setWindow (kPlayerWorkspace());
    m_temp_job -> addMetaData ("PropagateHttpHeader", "true");
    connect (m_temp_job, SIGNAL (data (KIO::Job*, const QByteArray&)), SLOT (transferTempData (KIO::Job*, const QByteArray&)));
    connect (m_temp_job, SIGNAL (result (KIO::Job*)), SLOT (transferTempDone (KIO::Job*)));
    connect (m_temp_job, SIGNAL (percent (KIO::Job*, unsigned long)), SLOT (transferProgress (KIO::Job*, unsigned long)));
    connect (m_temp_job, SIGNAL (infoMessage (KIO::Job*, const QString&)), SLOT (transferInfoMessage (KIO::Job*, const QString&)));
    transferProgress (m_temp_job, 0);
    m_delayed_helper = true;
  }
}

void KPlayerProcess::load (KUrl)
{
  m_position = 0;
  m_delayed_player = m_delayed_helper = false;
  m_size_sent = properties() -> hasVideo() || properties() -> hasNoVideo();
  m_info_available = properties() -> hasLength();
  if ( m_temp_job )
    m_temp_job -> kill (KJob::EmitResult);
  if ( m_temporary_file )
  {
    m_temporary_file -> close();
    delete m_temporary_file;
    m_temporary_file = 0;
  }
  transferTemporaryFile();
}

void KPlayerProcess::setState (State state)
{
  if ( m_state == state && state != Paused )
    return;
  State previous = m_state;
  m_state = state;
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: New state: " << state << ", previous state: " << previous << ", position: " << m_position << "\n";
#endif
  if ( previous == Running && state == Idle && ! m_quit )
    emit errorDetected();
  if ( ! m_quit || state == Idle )
    emit stateChanged (state, previous);
}

QString KPlayerProcess::positionString (void) const
{
  QString l (properties() -> lengthString()), p (timeString (position(), true));
  return l.isEmpty() ? p : p + " / " + l;
}

void KPlayerProcess::sendHelperCommand (const QByteArray& command)
{
  if ( ! m_helper )
    return;
  m_helper -> write (command);
#ifdef DEBUG_KPLAYER_HELPER
  kdDebugTime() << "helper << " << command;
#endif
}

void KPlayerProcess::sendPlayerCommand (const QByteArray& command)
{
  if ( ! m_player )
    return;
  m_player -> write (command);
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "process << " << command;
#endif
  m_sent = true;
  m_sent_count = 0;
}

void KPlayerProcess::get_info (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Get info\n";
  kdDebugTime() << " Widget " << kPlayerWorkspace() -> hiddenWidget() -> x()
    << "x" << kPlayerWorkspace() -> hiddenWidget() -> y()
    << " " << kPlayerWorkspace() -> hiddenWidget() -> width()
    << "x" << kPlayerWorkspace() -> hiddenWidget() -> height() << "\n";
#endif
  m_delayed_helper = m_kill = false;
  m_helper_seek = m_helper_seek_count = 0;
  m_helper_position = 0;
  if ( properties() -> url().isEmpty() || ! properties() -> deviceOption().isEmpty() )
    return;
  if ( properties() -> useKioslave() )
  {
    if ( ! properties() -> useTemporaryFile() )
      return;
    if ( m_temporary_file && m_temporary_file -> handle() >= 0 )
    {
      m_delayed_helper = true;
      return;
    }
  }
  m_helper = new KPlayerLineOutputProcess;
  *m_helper << properties() -> executablePath() << "-slave" << "-ao" << "null" << "-vo" << "x11"
    << "-wid" << QString::number (kPlayerWorkspace() -> hiddenWidget() -> winId());
  if ( properties() -> cache() == 1 || ! properties() -> url().isLocalFile() && ! properties() -> useKioslave() )
    *m_helper << "-nocache";
  else if ( properties() -> cache() == 2 )
    *m_helper << "-cache" << QString::number (properties() -> cacheSize());
  connect (m_helper, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*)),
    SLOT (receivedHelperLine (KPlayerLineOutputProcess*, char*)));
  connect (m_helper, SIGNAL (processFinished (KPlayerLineOutputProcess*)),
    SLOT (helperProcessFinished (KPlayerLineOutputProcess*)));
  run (m_helper);
}

void KPlayerProcess::play (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Play\n";
#endif
  if ( properties() -> url().isEmpty() )
    return;
  m_position = 0;
  emit progressChanged (m_position, Position);
  start();
}

QString resourcePath (const QString& filename)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Looking for " << filename << "\n";
#endif
  QString path (KStandardDirs::locate ("appdata", filename));
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << " appdata '" << path << "'\n";
#endif
  if ( path.isEmpty() )
    path = KStandardDirs::locate ("data", "kplayer/" + filename);
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << " found '" << path << "'\n";
#endif
  return path;
}

void KPlayerProcess::start (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Start\n";
#endif
  if ( m_slave_job )
    m_slave_job -> kill (KJob::EmitResult);
  m_position = m_max_position = 0;
  m_seek_count = m_cache_size = m_sent_count = 0;
  m_pausing = m_paused = m_quit = m_kill = m_09_version = m_delayed_player = m_first_chunk = false;
  m_seek = m_success = m_send_seek = m_sent = false;
  m_send_volume = m_send_contrast = m_send_brightness = m_send_hue = m_send_saturation = false;
  m_send_frame_drop = m_send_audio_id = m_send_subtitle_load = m_send_subtitle_visibility = false;
  m_send_audio_delay = m_send_subtitle_delay = m_send_subtitle_position = 0;
  m_send_subtitle_index = -2;
  m_seekable = m_subtitle_visibility = true;
  m_cache.clear();
  setState (Running);
  transferTemporaryFile();
  if ( properties() -> useKioslave() && properties() -> useTemporaryFile()
    && m_temporary_file && m_temporary_file -> handle() >= 0 )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Delaying play...\n";
#endif
    m_delayed_player = true;
    return;
  }
/*if ( m_helper && re_dvd_vcd.indexIn (settings() -> url().url()) >= 0 )
  {
    m_delayed_play = true;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Delaying play...\n";
#endif
    return;
  }*/
  m_player = new KPlayerLineOutputProcess;
  *m_player << properties() -> executablePath() << "-zoom" << "-noautosub" << "-slave"
    << "-wid" << QString::number (kPlayerWidget() -> winId()) << "-stop-xscreensaver";
  QString driver (properties() -> videoDriverString());
  if ( ! driver.isEmpty() )
  {
    if ( driver.startsWith ("xvmc") )
      driver = "xvmc:ck=set" + driver.mid (4);
    else if ( driver.startsWith ("xv,") || driver.startsWith ("xv:") )
      driver = "xv:ck=set" + driver.mid (2);
    *m_player << "-vo" << driver << "-colorkey" << "0x020202";
  }
  driver = properties() -> audioDriverString();
  if ( ! driver.isEmpty() )
    *m_player << "-ao" << driver;
  if ( properties() -> softwareVolume() )
    *m_player << "-softvol" << "-softvol-max" << QString::number (properties() -> maximumSoftwareVolume());
  else if ( driver.startsWith ("alsa") || driver.startsWith ("oss") )
  {
    driver = properties() -> mixerDevice();
    if ( ! driver.isEmpty() )
      *m_player << "-mixer" << driver;
    driver = properties() -> mixerChannel();
    if ( ! driver.isEmpty() )
      *m_player << "-mixer-channel" << driver;
  }
  *m_player << "-osdlevel" << QString::number (properties() -> osdLevel());
  *m_player << "-contrast" << QString::number (settings() -> contrast());
  *m_player << "-brightness" << QString::number (settings() -> brightness());
  *m_player << "-hue" << QString::number (settings() -> hue());
  *m_player << "-saturation" << QString::number (settings() -> saturation());
  if ( settings() -> frameDrop() == 0 )
    *m_player << "-noframedrop";
  else if ( settings() -> frameDrop() == 1 )
    *m_player << "-framedrop";
  else if ( settings() -> frameDrop() == 2 )
    *m_player << "-hardframedrop";
  int cache = properties() -> cache();
  if ( cache == 0 && properties() -> useKioslave() && (! properties() -> useTemporaryFile() || ! m_temporary_file) )
    *m_player << "-cache" << "1024";
  else if ( cache == 2 )
    *m_player << "-cache" << QString().setNum (properties() -> cacheSize());
  else if ( cache == 1 )
    *m_player << "-nocache";
  if ( properties() -> videoScaler() > 0 )
    *m_player << "-sws" << QString::number (properties() -> videoScaler());
  m_audio_delay = settings() -> audioDelay();
  if ( m_audio_delay != 0 )
    *m_player << "-delay" << QString::number (m_audio_delay);
  if ( properties() -> hasVideoID() )
    *m_player << "-vid" << QString::number (properties() -> videoID());
  m_audio_id = properties() -> audioID();
  if ( m_audio_id > -1 )
    *m_player << "-aid" << QString::number (m_audio_id);
  m_subtitles.clear();
  m_vobsub = QString::null;
  m_subtitle_index = properties() -> subtitleIndex();
  if ( settings() -> hasSubtitles() )
  {
    if ( settings() -> showVobsubSubtitles() || settings() -> hasVobsubSubtitles() && ! settings() -> showSubtitles() )
    {
      m_vobsub = settings() -> vobsubSubtitles();
      *m_player << "-vobsub" << m_vobsub;
      if ( properties() -> hasVobsubID() )
        *m_player << "-vobsubid" << QString::number (properties() -> vobsubID());
      else
        m_send_subtitle_index = m_subtitle_index;
    }
    else if ( settings() -> showSubtitles() )
    {
      if ( properties() -> hasSubtitleID() )
        *m_player << "-sid" << QString::number (properties() -> subtitleID());
      else if ( settings() -> hasExternalSubtitles() )
      {
        QString urls (settings() -> currentSubtitles());
        if ( urls.indexOf (',') < 0 )
          *m_player << "-sub" << urls;
        else
        {
          m_subtitle_index = -1;
          m_send_subtitle_load = true;
        }
      }
    }
  }
  m_subtitle_delay = settings() -> subtitleDelay();
  if ( m_subtitle_delay != 0 )
    *m_player << "-subdelay" << QString::number (m_subtitle_delay);
  m_subtitle_position = settings() -> subtitlePosition();
  if ( m_subtitle_position != 100 )
    *m_player << "-subpos" << QString::number (m_subtitle_position);
  QString font (configuration() -> subtitleFontName());
  if ( configuration() -> subtitleFontBold() )
    font += ":bold";
  if ( configuration() -> subtitleFontItalic() )
    font += ":italic";
  *m_player << "-fontconfig" << "-font" << font;
  *m_player << "-subfont-autoscale" << (configuration() -> subtitleAutoscale() ? "3" : "0");
  if ( configuration() -> subtitleTextSize() )
    *m_player << "-subfont-text-scale" << QString::number (configuration() -> subtitleTextSize());
  if ( configuration() -> hasSubtitleFontOutline() )
    *m_player << "-ffactor" << configuration() -> subtitleFontOutlineString();
  if ( configuration() -> hasSubtitleTextWidth() )
    *m_player << "-subwidth" << configuration() -> subtitleTextWidthString();
  const QString& encoding (properties() -> subtitleEncoding());
  if ( encoding == "UTF-8" )
    *m_player << "-utf8";
  else if ( ! encoding.isEmpty() )
    *m_player << "-subcp" << encoding;
  if ( properties() -> hasSubtitleFramerate() )
    *m_player << "-subfps" << properties() -> subtitleFramerateString();
  if ( properties() -> subtitleClosedCaption() )
    *m_player << "-subcc";
  if ( properties() -> videoDoubleBuffering() )
    *m_player << "-double";
  if ( properties() -> videoDirectRendering() && ! settings() -> showSubtitles() )
    *m_player << "-dr";
  if ( ! properties() -> videoDriverString().startsWith ("sdl")
    && ! properties() -> videoDriverString().startsWith ("svga") )
  {
    QString path = resourcePath ("input.conf");
    if ( ! path.isEmpty() )
      *m_player << "-input" << "conf=" + path;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Input.conf: '" << path << "'\n";
#endif
  }
  if ( properties() -> useKioslave() && (! properties() -> useTemporaryFile() || ! m_temporary_file) )
  {
    if ( m_temporary_file )
    {
      m_temporary_file -> close();
      delete m_temporary_file;
      m_temporary_file = 0;
    }
    QString ext (properties() -> extension());
    if ( re_ext.indexIn (ext) >= 0 )
      ext.prepend ('.');
    else
      ext = "";
    m_fifo_name = QFile::encodeName (QDir::homePath() + "/.mplayer/kpstream" + ext);
    removeDataFifo();
#ifdef HAVE_MKFIFO
#ifdef DEBUG_KPLAYER_PROCESS
    int rv =
#endif
    ::mkfifo (m_fifo_name, S_IRUSR | S_IWUSR);
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: mkfifo " << m_fifo_name << " returned " << rv << "\n";
#endif
#else
#ifdef DEBUG_KPLAYER_PROCESS
    int rv =
#endif
    ::mknod (m_fifo_name, S_IFIFO | S_IRUSR | S_IWUSR, 0);
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: mknod " << m_fifo_name << " returned " << rv << "\n";
#endif
#endif
  }
  else
    m_fifo_name.clear();
  connect (m_player, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*)),
    SLOT (receivedOutputLine (KPlayerLineOutputProcess*, char*)));
  connect (m_player, SIGNAL (receivedStderrLine (KPlayerLineOutputProcess*, char*)),
    SLOT (receivedOutputLine (KPlayerLineOutputProcess*, char*)));
  connect (m_player, SIGNAL (processFinished (KPlayerLineOutputProcess*)),
    SLOT (playerProcessFinished (KPlayerLineOutputProcess*)));
  run (m_player);
  if ( properties() -> useKioslave() && (! properties() -> useTemporaryFile() || ! m_temporary_file) )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Will send get_time_length for '" << properties() -> url().url() << "'\n";
    kdDebugTime() << "Process: Creating slave job\n";
#endif
    m_slave_job = KIO::get (properties() -> url(), KIO::NoReload, KIO::HideProgressInfo);
    m_slave_job -> ui() -> setWindow (kPlayerWorkspace());
    m_slave_job -> addMetaData ("PropagateHttpHeader", "true");
    connect (m_slave_job, SIGNAL (data (KIO::Job*, const QByteArray&)), SLOT (transferData (KIO::Job*, const QByteArray&)));
    connect (m_slave_job, SIGNAL (result (KIO::Job*)), SLOT (transferDone (KIO::Job*)));
    connect (m_slave_job, SIGNAL (infoMessage (KIO::Job*, const QString&)), SLOT (transferInfoMessage (KIO::Job*, const QString&)));
    m_cache_size = properties() -> cache() == 2 ? properties() -> cacheSize() * 1024 : 1048576;
    m_first_chunk = true;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Cache size: " << m_cache_size << "\n";
#endif
    m_seekable = properties() -> playlist();
  }
  properties() -> resetVobsubIDs();
}

void KPlayerProcess::restart (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Restart\n";
#endif
  if ( m_temp_job || ! m_player || properties() -> url().isEmpty() || state() == Idle )
    return;
  m_quit = true;
  m_cache.clear();
  if ( m_slave_job )
    m_slave_job -> kill (KJob::EmitResult);
  m_absolute_seek = int (m_position);
  sendPlayerCommand (command_quit);
  stop (&m_player, &m_quit, m_state != Paused);
  start();
  m_send_seek = true;
}

void KPlayerProcess::run (KPlayerLineOutputProcess* player)
{
  static QRegExp re_space (" +");
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Run\n";
#endif
  QString codec (properties() -> videoCodecString());
  if ( ! codec.isEmpty() )
    *player << "-vc" << codec;
  codec = properties() -> audioCodecString();
  if ( ! codec.isEmpty() )
    *player << "-ac" << codec;
  codec = properties() -> demuxerString();
  if ( ! codec.isEmpty() )
    *player << "-demuxer" << codec;
  if ( properties() -> buildNewIndex() == 0 )
    *player << "-idx";
  else if ( properties() -> buildNewIndex() == 2 )
    *player << "-forceidx";
  *player << "-noquiet" << "-msglevel" << "identify=4";
  QString commandline = properties() -> commandLine();
  if ( ! commandline.isEmpty() )
    *player << commandline.split (re_space);
  codec = properties() -> deviceSetting();
  if ( ! codec.isEmpty() )
    *player << properties() -> deviceOption() << codec;
  if ( properties() -> playlist() )
    *player << "-playlist";
  else
    *player << "--";
  if ( properties() -> useKioslave() )
    *player << (properties() -> useTemporaryFile() && m_temporary_file ? QFile::encodeName (m_temporary_file -> fileName()) : m_fifo_name);
  else
    *player << properties() -> urlString();
#if 0
  player -> setMerge (true);
#endif
  player -> start();
}

void KPlayerProcess::pause (void)
{
  if ( ! m_player || m_quit )
    return;
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process::Pause: state " << m_state << " sent " << m_sent << " count " << m_sent_count << " pausing " << m_pausing << " paused " << m_paused << "\n";
#endif
  if ( m_sent || m_pausing || state() == Running )
  {
    m_pausing = ! m_pausing;
    return;
  }
  sendPlayerCommand (command_pause);
  setState (m_state == Paused ? Playing : Paused);
  m_pausing = m_paused = false;
}

void KPlayerProcess::stop (KPlayerLineOutputProcess** player, bool* quit, bool send_quit)
{
  if ( *player )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Stopping MPlayer process\n";
#endif
    *quit = true;
    if ( send_quit )
    {
      if ( (*player) -> state() == KProcess::Running )
      {
#ifdef DEBUG_KPLAYER_PROCESS
        kdDebugTime() << "Process: MPlayer is running. Waiting...\n";
#endif
        (*player) -> waitForFinished (1000);
      }
    }
    if ( *quit && *player && (*player) -> state() == KProcess::Running )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Closing MPlayer...\n";
#endif
      (*player) -> terminate();
      (*player) -> waitForFinished (1000);
      if ( *quit && *player && (*player) -> state() == KProcess::Running )
      {
#ifdef DEBUG_KPLAYER_PROCESS
        kdDebugTime() << "Process: Killing MPlayer...\n";
#endif
        (*player) -> kill();
        (*player) -> waitForFinished (1000);
#ifdef DEBUG_KPLAYER_PROCESS
        if ( *quit && *player && (*player) -> state() == KProcess::Running )
          kdDebugTime() << "Process: Could not shut down MPlayer\n";
#endif
      }
    }
    if ( *quit && *player )
    {
      delete *player;
      *player = 0;
    }
  }
}

void KPlayerProcess::stop (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Stop\n";
#endif
  m_delayed_helper = m_delayed_player = false;
  m_quit = true;
  if ( m_temp_job )
  {
    m_temp_job -> kill (KJob::EmitResult);
    if ( m_temporary_file )
    {
      m_temporary_file -> close();
      delete m_temporary_file;
      m_temporary_file = 0;
    }
  }
  m_cache.clear();
  if ( m_slave_job )
    m_slave_job -> kill (KJob::EmitResult);
  if ( m_player )
    sendPlayerCommand (command_quit);
  stop (&m_player, &m_quit, m_state != Paused);
  setState (Idle);
}

void KPlayerProcess::kill (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Kill\n";
#endif
  m_delayed_helper = m_delayed_player = false;
  m_quit = m_kill = true;
  if ( m_temp_job )
  {
    m_temp_job -> kill (KJob::EmitResult);
    if ( m_temporary_file )
    {
      m_temporary_file -> close();
      delete m_temporary_file;
      m_temporary_file = 0;
    }
  }
  m_cache.clear();
  if ( m_slave_job )
    m_slave_job -> kill (KJob::EmitResult);
  if ( m_player )
    sendPlayerCommand (command_quit);
  if ( m_helper )
    sendHelperCommand (command_quit);
  stop (&m_player, &m_quit, m_state != Paused);
  stop (&m_helper, &m_kill);
  //setState (Idle);
}

void KPlayerProcess::progressSliderReleased (void)
{
  m_seek_count = 1;
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Slider released. Position " << position() << " origin " << m_seek_origin << " sent " << m_sent << " count " << m_seek_count << "\n";
#endif
}

void KPlayerProcess::absoluteSeek (int seconds)
{
  if ( ! m_player || m_quit )
    return;
  if ( seconds < 0 )
    seconds = 0;
  if ( m_sent || m_position - m_seek_origin < 0.65 && m_seek_origin - m_position < 0.25 )
  {
    m_send_seek = true;
    m_absolute_seek = seconds;
    return;
  }
  if ( m_position - float (seconds) < 0.95 && float (seconds) - m_position < 0.45 )
  {
    if ( float (seconds) > m_position )
      seconds ++;
    else
      seconds --;
  }
  QByteArray s ("seek ");
  // broken codec workaround
  if ( properties() -> length() >= MIN_VIDEO_LENGTH
    && re_mpeg12.indexIn (properties() -> videoCodecString()) >= 0
    && properties() -> deviceOption().isEmpty() )
  {
    seconds = limit (int (float (seconds) / properties() -> length() * 100 + 0.5), 0, 100);
    s += QByteArray::number (seconds) + " 1\n";
  }
  else
    s += QByteArray::number (seconds) + " 2\n";
  sendPlayerCommand (s);
  m_seek = true;
  m_seek_origin = position();
  m_send_seek = false;
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Sent seek. Position " << position() << " origin " << m_seek_origin << " sent " << m_sent << " count " << m_seek_count << "\n";
#endif
}

void KPlayerProcess::relativeSeek (int seconds)
{
  if ( ! m_player || m_quit || seconds == 0 )
    return;
  QByteArray s ("seek ");
  // broken codec workaround
  if ( (seconds > 4 || seconds < -4) && properties() -> length() >= MIN_VIDEO_LENGTH
    && re_mpeg12.indexIn (properties() -> videoCodecString()) >= 0
    && properties() -> deviceOption().isEmpty() )
  {
    absoluteSeek (int (m_position + seconds + 0.5));
    return;
  }
  else
    s += QByteArray::number (seconds) + "\n";
  sendPlayerCommand (s);
  m_seek = true;
}

void KPlayerProcess::volume (int volume)
{
  if ( ! m_player || m_quit || state() != Playing )
    return;
  if ( m_sent )
  {
    m_send_volume = true;
    return;
  }
  volume = limit (volume, 0, 100);
  sendPlayerCommand ("volume " + QByteArray::number (volume) + " 1\n");
  m_send_volume = false;
}

void KPlayerProcess::frameDrop (int frame_drop)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running )
    return;
  if ( m_sent || state() == Running )
  {
    m_send_frame_drop = true;
    return;
  }
  sendPlayerCommand ("frame_drop " + QByteArray::number (frame_drop) + "\n");
  m_send_frame_drop = false;
}

void KPlayerProcess::contrast (int contrast)
{
  if ( ! m_player || m_quit || state() != Playing )
    return;
  if ( m_sent )
  {
    m_send_contrast = true;
    return;
  }
  contrast = limit (contrast, -100, 100);
  sendPlayerCommand ("contrast " + QByteArray::number (contrast) + " 1\n");
  m_send_contrast = false;
}

void KPlayerProcess::brightness (int brightness)
{
  if ( ! m_player || m_quit || state() != Playing )
    return;
  if ( m_sent )
  {
    m_send_brightness = true;
    return;
  }
  brightness = limit (brightness, -100, 100);
  sendPlayerCommand ("brightness " + QByteArray::number (brightness) + " 1\n");
  m_send_brightness = false;
}

void KPlayerProcess::hue (int hue)
{
  if ( ! m_player || m_quit || state() != Playing )
    return;
  if ( m_sent )
  {
    m_send_hue = true;
    return;
  }
  hue = limit (hue, -100, 100);
  sendPlayerCommand ("hue " + QByteArray::number (hue) + " 1\n");
  m_send_hue = false;
}

void KPlayerProcess::saturation (int saturation)
{
  if ( ! m_player || m_quit || state() != Playing )
    return;
  if ( m_sent )
  {
    m_send_saturation = true;
    return;
  }
  saturation = limit (saturation, -100, 100);
  sendPlayerCommand ("saturation " + QByteArray::number (saturation) + " 1\n");
  m_send_saturation = false;
}

void KPlayerProcess::subtitleMove (int position, bool absolute)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running )
    return;
  if ( absolute )
    position -= m_subtitle_position;
  if ( position == 0 )
    return;
  m_subtitle_position += position;
  if ( m_sent || state() == Running )
  {
    m_send_subtitle_position += position;
    return;
  }
  position += m_send_subtitle_position;
  if ( position == 0 )
    return;
  sendPlayerCommand ("sub_pos " + QByteArray::number (position) + "\n");
  m_send_subtitle_position = 0;
}

void KPlayerProcess::subtitleDelay (float delay, bool absolute)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running )
    return;
  if ( absolute )
    delay -= m_subtitle_delay;
  if ( delay < 0.001 && delay > - 0.001 )
    return;
  m_subtitle_delay += delay;
  if ( m_sent || state() == Running )
  {
    m_send_subtitle_delay += delay;
    return;
  }
  delay += m_send_subtitle_delay;
  if ( delay < 0.001 && delay > - 0.001 )
    return;
  sendPlayerCommand ("sub_delay " + QByteArray::number (- delay) + "\n");
  m_send_subtitle_delay = 0;
}

void KPlayerProcess::subtitleIndex (int index)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running )
    return;
  if ( m_sent || state() == Running )
  {
    m_send_subtitle_index = index;
    return;
  }
  sendPlayerCommand ("sub_select " + QByteArray::number (index) + "\n");
  m_subtitle_index = index;
  m_send_subtitle_index = -2;
  if ( index == -1 == m_subtitle_visibility )
    subtitleVisibility();
  else
    m_send_subtitle_visibility = false;
}

void KPlayerProcess::subtitleVisibility (void)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running )
    return;
  if ( m_sent || state() == Running )
  {
    m_send_subtitle_visibility = true;
    return;
  }
  sendPlayerCommand (command_visibility);
  m_subtitle_visibility = ! m_subtitle_visibility;
  m_send_subtitle_visibility = false;
}

void KPlayerProcess::subtitles (void)
{
  if ( ! m_player || m_quit || state() == Idle )
    return;
  if ( m_vobsub != settings() -> vobsubSubtitles() && settings() -> showVobsubSubtitles() )
  {
    restart();
    return;
  }
  int index = properties() -> subtitleIndex();
  int count = properties() -> subtitleIDs().count() + properties() -> vobsubIDs().count();
  if ( index < count )
  {
    subtitleIndex (index);
    m_send_subtitle_load = false;
    return;
  }
  QString subtitle (settings() -> currentSubtitles());
  index = m_subtitles.indexOf (subtitle);
  if ( index >= 0 )
  {
    subtitleIndex (index + count);
    m_send_subtitle_load = false;
    return;
  }
  if ( m_sent || state() == Running )
  {
    m_send_subtitle_load = true;
    return;
  }
  sendPlayerCommand ("sub_load \"" + subtitle.toUtf8() + "\"\n");
  m_send_subtitle_load = false;
}

void KPlayerProcess::audioDelay (float delay, bool absolute)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running )
    return;
  if ( absolute )
    delay -= m_audio_delay;
  if ( delay < 0.001 && delay > - 0.001 )
    return;
  m_audio_delay += delay;
  if ( m_sent || state() == Running )
  {
    m_send_audio_delay += delay;
    return;
  }
  delay += m_send_audio_delay;
  if ( delay < 0.001 && delay > - 0.001 )
    return;
  sendPlayerCommand ("audio_delay " + QByteArray::number (- delay) + "\n");
  m_send_audio_delay = 0;
}

void KPlayerProcess::audioID (int id)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running )
    return;
  if ( m_sent || state() == Running )
  {
    m_send_audio_id = true;
    return;
  }
  if ( id != m_audio_id )
  {
    QRegExp demuxers (configuration() -> switchAudioDemuxers());
    if ( demuxers.indexIn (properties() -> demuxerString()) >= 0 )
    {
      sendPlayerCommand ("switch_audio " + QByteArray::number (id) + "\n");
      m_audio_id = id;
    }
    else
      restart();
  }
  m_send_audio_id = false;
}

void KPlayerProcess::transferData (KIO::Job* job, const QByteArray& data)
{
  if ( job && job == m_slave_job && m_player )
  {
    if ( data.size() == 0 )
      return;
    if ( m_cache.count() == 0 || m_cache.count() == 1 && ! m_first_chunk )
    {
#ifdef DEBUG_KPLAYER_KIOSLAVE
      kdDebugTime() << "Process: Cache: Creating new chunk, size " << data.size() << "\n";
#endif
      m_cache.append (QByteArray (data.constData(), data.size()));
    }
    else
    {
      QByteArray& array = m_cache.last();
      int size = array.size();
      array.resize (size + data.size());
#ifdef DEBUG_KPLAYER_KIOSLAVE
      if ( array.size() != size + data.size() )
        kdDebugTime() << "Process: Cache: Size mismatch: " << size << " + " << data.size() << " = " << array.size() << "\n";
      else
        kdDebugTime() << "Process: Cache: Appended to chunk " << m_cache.count() << " size " << size << " + " << data.size() << " = " << array.size() << "\n";
#endif
      memcpy (array.data() + size, data.data(), data.size());
    }
    if ( m_cache.count() > 1 && ! m_slave_job -> isSuspended() && m_cache.last().size() >= m_cache_size )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Suspending transfer job\n";
#endif
      m_slave_job -> suspend();
    }
    if ( m_cache.count() == 1 && (! m_first_chunk || m_cache.first().size() >= m_cache_size) )
    {
      if ( m_first_chunk && ! m_quit )
        emit progressChanged (100, CacheFill);
      sendFifoData();
    }
    else if ( m_first_chunk && ! m_quit )
      emit progressChanged (limit (int ((m_cache.first().size() * 100 + m_cache_size / 2) / m_cache_size), 0, 100), CacheFill);
  }
  else
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Stray transfer job\n";
#endif
    m_cache.clear();
    if ( job )
      job -> kill (KJob::Quietly);
  }
}

void KPlayerProcess::transferTempData (KIO::Job* job, const QByteArray& data)
{
  if ( job && job == m_temp_job && m_temporary_file )
  {
#ifdef DEBUG_KPLAYER_KIOSLAVE
    int rv =
#endif
    m_temporary_file -> write (data);
#ifdef DEBUG_KPLAYER_KIOSLAVE
    kdDebugTime() << "Process: Write call returned " << rv << "\n";
#endif
  }
  else
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Stray temporary file TransferJob\n";
#endif
    if ( job )
      job -> kill (KJob::Quietly);
  }
}

void KPlayerProcess::transferProgress (KIO::Job* job, unsigned long progress)
{
  if ( job && job == m_temp_job )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Temporary file transfer progress received: " << progress << "\n";
#endif
    emit progressChanged (progress, FileTransfer);
  }
#ifdef DEBUG_KPLAYER_PROCESS
  else
    kdDebugTime() << "Process: Stray temporary file progress received: " << progress << "\n";
#endif
}

void KPlayerProcess::transferInfoMessage (KIO::Job* job, const QString& message)
{
  if ( job && (job == m_slave_job || job == m_temp_job) )
    emit messageReceived (message);
}

void KPlayerProcess::transferDone (KIO::Job* job)
{
  if ( job && job == m_slave_job )
  {
    bool error_page = m_slave_job -> isErrorPage();
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Transfer job ended, result code: " << job -> error()
      << " error page " << error_page << "\n";
#endif
    if ( job -> error() != 0 && (job -> error() != 20 || ! m_quit) || error_page )
    {
      QString errorString;
      if ( job -> error() != 0 )
      {
        errorString = job -> errorString();
        if ( errorString.isEmpty() )
        {
          KUrl url (properties() -> url());
          errorString = job -> detailedErrorStrings (&url).first();
        }
      }
      else if ( error_page )
      {
        m_cache.clear();
/*#ifdef DEBUG_KPLAYER_PROCESS
        KIO::MetaData metadata (job -> metaData());
        for ( KIO::MetaData::Iterator it = metadata.begin(); it != metadata.end(); ++ it )
          kdDebugTime() << "Process: Error page metadata: key '" << it.key() << "' data '" << it.data() << "'\n";
#endif*/
        errorString = job -> queryMetaData ("HTTP-Headers");
      }
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Error string: '" << errorString << "'\n";
#endif
      if ( ! errorString.isEmpty() )
        emit messageReceived (errorString);
      emit errorDetected();
      error_page = (error_page || m_first_chunk) && ! m_quit;
    }
    else if ( m_cache.count() == 1 && m_first_chunk && m_cache.first().size() < m_cache_size && ! m_quit )
      sendFifoData();
    m_cache_size = 0;
    m_first_chunk = false;
    m_slave_job = 0;
    if ( m_player && m_cache.isEmpty() )
    {
      removeDataFifo();
      if ( error_page && m_player )
      {
        stop (&m_player, &m_quit);
        setState (Idle);
      }
    }
  }
#ifdef DEBUG_KPLAYER_PROCESS
  else
    kdDebugTime() << "Process: Stray transfer job ended\n";
#endif
}

void KPlayerProcess::transferTempDone (KIO::Job* job)
{
  if ( job && job == m_temp_job )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Temporary file transfer job ended, result code: " << job -> error()
      << " error page " << m_temp_job -> isErrorPage() << "\n";
#endif
    if ( job -> error() != 0 && (job -> error() != 20 || ! m_quit) || m_temp_job -> isErrorPage() )
    {
      QString errorString;
      if ( job -> error() != 0 )
        errorString = job -> errorString();
      else if ( m_temp_job -> isErrorPage() )
        errorString = job -> queryMetaData ("HTTP-Headers");
/*    {
#ifdef DEBUG_KPLAYER_PROCESS
        KIO::MetaData metadata (job -> metaData());
        for ( KIO::MetaData::Iterator it = metadata.begin(); it != metadata.end(); ++ it )
          kdDebugTime() << "Process: Error page metadata: key '" << it.key() << "' data '" << it.data() << "'\n";
#endif
      }*/
      if ( ! errorString.isEmpty() )
      {
        emit messageReceived (errorString);
#ifdef DEBUG_KPLAYER_PROCESS
        kdDebugTime() << "Process: Error string: " << errorString << "\n";
#endif
      }
      /*if ( m_temp_job -> isErrorPage() )
        emit messageReceived ("HTTP request returned an error");
      QString errorString (job -> errorString());
      if ( ! errorString.isEmpty() )
        emit messageReceived (errorString);
      KUrl url (settings() -> url());
      QStringList errors (job -> detailedErrorStrings (&url));
      for ( QStringList::Iterator it = errors.begin(); it != errors.end(); ++ it )
        if ( ! (*it).isEmpty() )
          emit messageReceived (*it);*/
      emit errorDetected();
      if ( m_temporary_file )
      {
        m_temporary_file -> close();
        delete m_temporary_file;
        m_temporary_file = 0;
      }
      m_temp_job = 0;
      m_delayed_player = m_delayed_helper = false;
      setState (Idle);
    }
    else if ( m_quit )
    {
      if ( m_temporary_file )
      {
        m_temporary_file -> close();
        delete m_temporary_file;
        m_temporary_file = 0;
      }
      m_temp_job = 0;
      m_delayed_player = m_delayed_helper = false;
    }
    else
    {
      emit progressChanged (100, FileTransfer);
      m_temp_job = 0;
      if ( m_temporary_file )
        m_temporary_file -> close();
      if ( m_delayed_helper )
        get_info();
      if ( m_delayed_player )
        play();
    }
  }
#ifdef DEBUG_KPLAYER_PROCESS
  else
    kdDebugTime() << "Process: Stray temporary file TransferJob ended\n";
#endif
}

void KPlayerProcess::removeDataFifo (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process::removeDataFifo\n";
#endif
  if ( m_fifo_notifier )
  {
    delete m_fifo_notifier;
    m_fifo_notifier = 0;
  }
  if ( m_fifo_handle >= 0 )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: closing fifo " << m_fifo_handle << "...\n";
#endif
    m_fifo_handle = ::close (m_fifo_handle);
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: fifo close returned " << m_fifo_handle << "\n";
#endif
    m_fifo_handle = -1;
    m_fifo_offset = 0;
  }
  if ( ! m_fifo_name.isEmpty() )
    ::unlink (m_fifo_name);
#ifdef DEBUG_KPLAYER_DUMP
  if ( s_dump.isOpen() )
    s_dump.close();
#endif
}

void KPlayerProcess::sendFifoData (void)
{
#ifdef DEBUG_KPLAYER_KIOSLAVE
  kdDebugTime() << "Process::sendFifoData\n";
#endif
  if ( m_fifo_handle < 0 )
  {
    m_fifo_handle = ::open (m_fifo_name, O_WRONLY | O_NONBLOCK, S_IRUSR | S_IWUSR);
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: fifo open returned " << m_fifo_handle << "\n";
#endif
    if ( m_fifo_handle >= 0 )
    {
      if ( m_fifo_timer )
      {
#ifdef DEBUG_KPLAYER_PROCESS
        kdDebugTime() << "Process: fifo open successful, deleting timer\n";
#endif
        delete m_fifo_timer;
        m_fifo_timer = 0;
      }
      m_fifo_notifier = new QSocketNotifier (m_fifo_handle, QSocketNotifier::Write);
      m_fifo_notifier -> setEnabled (false);
      connect (m_fifo_notifier, SIGNAL (activated (int)), SLOT (playerDataWritten (int)));
    }
    else if ( ! m_fifo_timer )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: fifo open failed, creating timer\n";
#endif
      m_fifo_timer = new QTimer (this);
      connect (m_fifo_timer, SIGNAL (timeout()), SLOT (sendFifoData()));
      m_fifo_timer -> start (100);
    }
  }
  if ( m_fifo_handle >= 0 )
  {
    QByteArray& array = m_cache.first();
    if ( ! m_cache.isEmpty() && array.size() > m_fifo_offset )
    {
#ifdef DEBUG_KPLAYER_KIOSLAVE
      kdDebugTime() << "Process: Cache: Writing " << array.size() << " - " << m_fifo_offset << " bytes to fifo\n";
#endif
      int rv = ::write (m_fifo_handle, array.data() + m_fifo_offset, array.size() - m_fifo_offset);
      if ( rv > 0 )
      {
#ifdef DEBUG_KPLAYER_DUMP
        if ( ! s_dump.isOpen() )
          s_dump.open (QIODevice::WriteOnly);
        s_dump.writeBlock (array.data() + m_fifo_offset, rv);
#endif
        m_fifo_offset += rv;
      }
#ifdef DEBUG_KPLAYER_KIOSLAVE
      kdDebugTime() << "Process: Cache: Write call returned " << rv << "\n";
#endif
      m_fifo_notifier -> setEnabled (true);
      m_first_chunk = false;
    }
  }
}

void KPlayerProcess::playerDataWritten (int fd)
{
  if ( fd == m_fifo_handle )
  {
#ifdef DEBUG_KPLAYER_KIOSLAVE
    kdDebugTime() << "Process: Cache: Data written\n";
#endif
    QByteArray& array = m_cache.first();
    if ( ! m_cache.isEmpty() && array.size() <= m_fifo_offset )
    {
#ifdef DEBUG_KPLAYER_KIOSLAVE
      kdDebugTime() << "Process: Cache: Wrote " << array -> size() << " byte chunk, offset " << m_fifo_offset << "\n";
#endif
      m_cache.removeFirst();
      m_fifo_offset = 0;
      m_fifo_notifier -> setEnabled (false);
      if ( m_slave_job && m_slave_job -> isSuspended() )
      {
#ifdef DEBUG_KPLAYER_PROCESS
        kdDebugTime() << "Process: Resuming transfer job\n";
#endif
        m_slave_job -> resume();
      }
    }
    if ( ! m_cache.isEmpty() )
      sendFifoData();
    else if ( ! m_slave_job )
      removeDataFifo();
  }
#ifdef DEBUG_KPLAYER_PROCESS
  else
    kdDebugTime() << "Process: Stray socket notifier signal\n";
#endif
}

void KPlayerProcess::playerProcessFinished (KPlayerLineOutputProcess* proc)
{
  if ( proc != m_player )
  {
    delete proc;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Stray MPlayer process exited\n";
#endif
    return;
  }
  if ( m_player -> error() == QProcess::FailedToStart )
  {
    emit messageReceived (i18n("Could not start MPlayer"));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Could not start MPlayer\n";
#endif
  }
#ifdef DEBUG_KPLAYER_PROCESS
  else
    kdDebugTime() << "Process: MPlayer process exited\n";
#endif
  delete m_player;
  m_player = 0;
  if ( m_success && ! m_seek && m_position > 0 && m_position > properties() -> length() / 40 )
  {
    properties() -> setLength (m_max_position);
    m_info_available = true;
    emit infoAvailable();
    properties() -> commit();
  }
  m_cache.clear();
  if ( m_slave_job )
    m_slave_job -> kill (KJob::EmitResult);
  removeDataFifo();
  m_fifo_name.clear();
  if ( ! m_quit )
    setState (Idle);
}

void KPlayerProcess::helperProcessFinished (KPlayerLineOutputProcess* proc)
{
  if ( proc != m_helper )
  {
    delete proc;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Stray MPlayer helper process finished\n";
#endif
    return;
  }
#ifdef DEBUG_KPLAYER_PROCESS
  if ( m_helper -> error() == QProcess::FailedToStart )
    kdDebugTime() << "Process: Could not start helper\n";
  else
    kdDebugTime() << "MPlayer helper process finished\n";
#endif
  delete m_helper;
  m_helper = 0;
  if ( m_helper_seek < 500 && m_helper_position >= MIN_VIDEO_LENGTH
      && m_helper_position > properties() -> length() / 40 )
    properties() -> setLength (m_helper_position);
  m_info_available = true;
  if ( ! m_kill )
    emit infoAvailable();
  if ( ! m_size_sent && ! m_kill && m_helper_seek > 0 )
  {
    emit sizeAvailable();
    m_size_sent = true;
  }
  if ( ! m_kill && properties() -> url().isValid() )
    properties() -> commit();
  /*if ( m_delayed_play && ! m_player )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Delayed play...\n";
#endif
    play();
  }*/
}

void KPlayerProcess::receivedOutputLine (KPlayerLineOutputProcess* proc, char* str)
{
  if ( proc != m_player )
  {
    if ( re_exiting.indexIn (str) < 0 )
      proc -> write (command_quit);
    return;
  }
  static float prev_position = -1;
  float ftime;
  //if ( state() == Running )
  //  kPlayerWidget() -> sendConfigureEvent();
  if ( re_version.indexIn (str) >= 0 )
  {
    m_09_version = true;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: MPlayer 0.9x detected\n";
#endif
  }
  if ( re_paused.indexIn (str) >= 0 )
  {
    m_paused = true;
    m_pausing = false;
    m_sent = false;
    setState (Paused);
  }
  if ( strncmp (str, "ID_FILE_SUB_FILENAME=", 21) == 0 && str[21] )
  {
    m_subtitles.append (str + 21);
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Subtitle file " << m_subtitles.last() << "\n";
#endif
    if ( settings() -> currentSubtitles() == m_subtitles.last() )
      subtitleIndex (properties() -> subtitleIDs().count() + properties() -> vobsubIDs().count() + m_subtitles.count() - 1);
  }
  else if ( m_state < Playing || strncmp (str, "ID_", 3) == 0
    || strncmp (str, "Name", 4) == 0 || strncmp (str, "ICY Info:", 9) == 0 )
  {
    QSize size (properties() -> originalSize());
    bool hadVideo = properties() -> hasVideo();
    bool hadLength = properties() -> hasLength();
    properties() -> extractMeta (str, true);
    if ( ! hadLength && properties() -> hasLength() )
    {
      m_info_available = true;
      if ( ! m_quit )
        emit infoAvailable();
    }
    if ( properties() -> hasVideo() && (! hadVideo || size != properties() -> originalSize()) )
      m_size_sent = false;
    if ( ! m_quit && ! m_size_sent && properties() -> heightAdjusted() )
    {
      emit sizeAvailable();
      m_size_sent = true;
    }
  }
  if ( m_state == Running && (m_pausing || m_send_seek) && ! m_sent && ! m_quit && re_start.indexIn (str) >= 0 )
  {
    if ( m_send_seek )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Initial seek to " << m_absolute_seek << ". Position " << position() << " origin " << m_seek_origin << " sent " << m_sent << " count " << m_seek_count << "\n";
#endif
      absoluteSeek (m_absolute_seek);
    }
    else
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Pausing: '" << str << "'\n";
#endif
      sendPlayerCommand (command_pause);
      setState (m_state == Paused ? Playing : Paused);
      m_pausing = m_paused = false;
    }
  }
  if ( re_success.indexIn (str) >= 0 && ! m_quit )
    m_success = true;
  if ( re_exiting.indexIn (str) >= 0 && re_quit.indexIn (str) < 0 && re_success.indexIn (str) < 0 && ! m_quit )
    emit errorDetected();
  if ( re_crash.indexIn (str) >= 0 )
  {
    int sig = re_crash.cap(1).toInt();
    if ( sig <= 15 && ! m_quit || sig < 9 || sig > 9 && sig < 15 )
      emit errorDetected();
  }
//kdDebugTime() << "matching a_or_v regex\n";
  if ( re_a_or_v.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_LINEOUT
    kdDebugTime() << "Received AV Buffer: '" << str << "'\n";
#endif
    if ( m_state < Playing )
    {
      if ( ! m_size_sent && ! m_quit )
      {
        emit sizeAvailable();
        m_size_sent = true;
      }
      if ( ! m_quit )
        properties() -> commit();
      setState (Playing);
      m_send_volume = m_send_contrast = m_send_brightness = m_send_hue = m_send_saturation = true;
    }
    if ( m_sent && ++ m_sent_count >= 5 )
      m_sent = false;
    if ( m_quit && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Resending quit command\n";
#endif
      sendPlayerCommand (command_quit);
    }
    if ( m_send_subtitle_load && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitles\n";
#endif
      subtitles();
    }
    if ( m_send_subtitle_index > -2 && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitle index\n";
#endif
      subtitleIndex (m_send_subtitle_index);
    }
    if ( m_send_subtitle_visibility && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitle visibility\n";
#endif
      subtitleVisibility();
    }
    if ( m_send_audio_id && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending audio ID\n";
#endif
      audioID (properties() -> audioID());
    }
    if ( (m_send_audio_delay >= 0.001 || m_send_audio_delay <= - 0.001) && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending audio delay\n";
#endif
      sendPlayerCommand ("audio_delay " + QByteArray::number (- m_send_audio_delay) + "\n");
      m_send_audio_delay = 0;
    }
    if ( (m_send_subtitle_delay >= 0.001 || m_send_subtitle_delay <= - 0.001) && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitle delay\n";
#endif
      sendPlayerCommand ("sub_delay " + QByteArray::number (- m_send_subtitle_delay) + "\n");
      m_send_subtitle_delay = 0;
    }
    if ( m_send_subtitle_position && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitle position\n";
#endif
      sendPlayerCommand ("sub_pos " + QByteArray::number (m_send_subtitle_position) + "\n");
      m_send_subtitle_position = 0;
    }
    if ( m_send_volume && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending volume\n";
#endif
      volume (settings() -> actualVolume());
    }
    if ( m_send_frame_drop && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending frame drop\n";
#endif
      frameDrop (settings() -> frameDrop());
    }
    if ( m_send_contrast && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending contrast\n";
#endif
      contrast (settings() -> contrast());
    }
    if ( m_send_brightness && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending brightness\n";
#endif
      brightness (settings() -> brightness());
    }
    if ( m_send_hue && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending hue\n";
#endif
      hue (settings() -> hue());
    }
    if ( m_send_saturation && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending saturation\n";
#endif
      saturation (settings() -> saturation());
    }
//  kdDebugTime() << "regex matched\n";
    if ( re_a_and_v.indexIn (str) >= 0 )
    {
      ftime = stringToFloat (re_a_and_v.cap (1));
      float ftime2 = stringToFloat (re_a_and_v.cap (2));
      if ( ftime2 > ftime )
        ftime = ftime2;
    }
    else
    {
      ftime = stringToFloat (re_a_or_v.cap (1));
//    kdDebugTime() << "match: " << re_a_or_v.cap (1) << "ftime: " << ftime << "\n";
    }
    if ( ftime > properties() -> length() && properties() -> length() >= MIN_VIDEO_LENGTH )
      properties() -> setLength (ftime);
    if ( ftime != m_position )
    {
      m_position = ftime;
      if ( m_position > m_max_position )
        m_max_position = m_position;
      float diff = m_position - prev_position;
      prev_position = m_position;
      if ( ! m_quit && (diff > 0 || diff < -0.15 || m_position == 0)
          && (m_position - m_seek_origin > 0.65 || m_seek_origin - m_position > 0.25)
          && ! m_send_seek && (m_seek_count == 0 || ++ m_seek_count > 5) )
      {
#ifdef DEBUG_KPLAYER_PROCESS
        if ( m_seek_origin >= 0 )
          kdDebugTime() << "Process: Reset seek origin. Position " << position() << " origin " << m_seek_origin << " sent " << m_sent << " count " << m_seek_count << "\n";
#endif
        m_seek_origin = NO_SEEK_ORIGIN;
        emit progressChanged (m_position, Position);
        m_seek_count = 0;
      }
//    kdDebugTime() << "position: " << m_position << " length: " << properties() -> length() << "\n";
    }
    if ( m_pausing && ! m_quit && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Pausing: '" << str << "'\n";
#endif
      m_pausing = m_paused = false;
      pause();
    }
    if ( m_paused && ! m_quit && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Paused: '" << str << "'\n";
#endif
      m_pausing = m_paused = false;
      sendPlayerCommand (command_pause);
    }
    if ( m_send_seek && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending seek to " << m_absolute_seek << ". Position " << position() << " origin " << m_seek_origin << " sent " << m_sent << " count " << m_seek_count << "\n";
#endif
      absoluteSeek (m_absolute_seek);
    }
  }
  else if ( re_cache_fill.indexIn (str) >= 0 )
  {
    ftime = stringToFloat (re_cache_fill.cap (1));
#ifdef DEBUG_KPLAYER_PROGRESS
    kdDebugTime() << "Received cache progress: '" << str << "' -> " << ftime << "\n";
#endif
    if ( ! m_quit && ! m_first_chunk )
      emit progressChanged (ftime, CacheFill);
  }
  else if ( re_generating_index.indexIn (str) >= 0 )
  {
    ftime = stringToFloat (re_generating_index.cap (1));
#ifdef DEBUG_KPLAYER_PROGRESS
    kdDebugTime() << "Received indexing progress: '" << str << "' -> " << ftime << "\n";
#endif
    if ( ! m_quit )
      emit progressChanged (ftime, IndexGeneration);
  }
  else if ( ! m_quit )
  {
    emit messageReceived (QString::fromLocal8Bit (str));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "process >> " << str << "\n";
#endif
  }
}

void KPlayerProcess::receivedHelperLine (KPlayerLineOutputProcess* proc, char* str)
{
  if ( proc != m_helper )
  {
    if ( re_exiting.indexIn (str) < 0 )
      proc -> write (command_quit);
    return;
  }
  bool sent = false;
  float ftime;
#ifdef DEBUG_KPLAYER_HELPER
  kdDebugTime() << "helper >> " << str << "\n";
#endif
  bool hadVideo = properties() -> hasVideo();
  bool hadLength = properties() -> hasLength();
  properties() -> extractMeta (str, false);
  if ( ! hadLength && properties() -> hasLength() )
  {
    m_info_available = true;
    if ( ! m_kill )
      emit infoAvailable();
  }
  if ( m_helper_seek == 1 && properties() -> hasLength() )
    m_helper_seek_count = 9;
  if ( ! hadVideo && properties() -> hasVideo() )
    m_size_sent = false;
  if ( ! m_kill && ! m_size_sent && properties() -> heightAdjusted() )
  {
    emit sizeAvailable();
    m_size_sent = true;
  }
//kdDebugTime() << "matching a_or_v regex\n";
  if ( re_a_or_v.indexIn (str) >= 0 )
  {
//  kdDebugTime() << "regex matched\n";
    if ( re_a_and_v.indexIn (str) >= 0 )
    {
      ftime = stringToFloat (re_a_and_v.cap (1));
      float ftime2 = stringToFloat (re_a_and_v.cap (2));
      if ( ftime2 > ftime )
        ftime = ftime2;
    }
    else
    {
      ftime = stringToFloat (re_a_or_v.cap (1));
//    kdDebugTime() << "match: " << re_a_or_v.cap (1) << "ftime: " << ftime << "\n";
    }
    if ( m_helper_seek > 0 && ! sent )
    {
      if ( ++ m_helper_seek_count < 10 )
        sent = true;
      else
        m_helper_seek_count = 0;
#ifdef DEBUG_KPLAYER_HELPER
      kdDebugTime() << "Helper: Seek count: " << m_helper_seek_count << "\n";
#endif
    }
    if ( m_helper_seek == 0 && ftime >= MIN_VIDEO_LENGTH )
      m_helper_seek = 100;
    else
    {
      if ( m_helper_seek > 0 && m_helper_seek < 500 && ftime >= MIN_VIDEO_LENGTH
          && properties() -> hasLength() && ftime > properties() -> length() )
        properties() -> setLength (ftime);
      if ( ftime != m_helper_position )
      {
        m_helper_position = ftime;
#ifdef DEBUG_KPLAYER_HELPER
        kdDebugTime() << "helper position: " << m_helper_position << " length: " << properties() -> length() << "\n";
#endif
      }
      if ( m_helper_seek > 0 && m_helper_seek < 500 && ftime >= MIN_VIDEO_LENGTH )
      {
        float estlength = ftime * 100 / m_helper_seek;
        if ( properties() -> length() < estlength )
        {
          properties() -> setLength (estlength);
          m_info_available = true;
          if ( ! m_kill )
            emit infoAvailable();
        }
#ifdef DEBUG_KPLAYER_HELPER
        kdDebugTime() << "estimated length: " << properties() -> length() << "\n";
#endif
      }
    }
    if ( m_helper_seek == 0 && ! sent )
    {
      sendHelperCommand (command_seek_99);
      m_helper_seek = 99;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek == 99 && ! sent && properties() -> length() < MIN_VIDEO_LENGTH )
    {
      sendHelperCommand (command_seek_95);
      m_helper_seek = 95;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek == 95 && ! sent && properties() -> length() < MIN_VIDEO_LENGTH )
    {
      sendHelperCommand (command_seek_90);
      m_helper_seek = 90;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek == 90 && ! sent && properties() -> length() < MIN_VIDEO_LENGTH )
    {
      sendHelperCommand (command_seek_50);
      m_helper_seek = 50;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek < 100 && properties() -> length() >= MIN_VIDEO_LENGTH )
    {
      sendHelperCommand (command_seek_100);
      m_helper_seek = 100;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( (m_helper_seek == 50 || m_helper_seek == 100) && ! sent )
    {
      sendHelperCommand (command_quit);
      m_helper_seek = 500;
      sent = true;
    }
  }
}
