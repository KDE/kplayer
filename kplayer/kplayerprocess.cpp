/***************************************************************************
                          kplayerprocess.cpp
                          ------------------
    begin                : Sat Jan 11 2003
    copyright            : (C) 2002-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <klocale.h>
#include <kprocctrl.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <kdebug.h>

#include "kplayerprocess.h"
#include "kplayerprocess.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"
#include "kplayerwidget.h"

#define MIN_VIDEO_LENGTH 5

#define DEBUG_KPLAYER_PROCESS
//#define DEBUG_KPLAYER_PROGRESS
#define DEBUG_KPLAYER_HELPER
//#define DEBUG_KPLAYER_LINEOUT
//#define DEBUG_KPLAYER_KIOSLAVE
//#define DEBUG_KPLAYER_DUMP

#ifdef DEBUG_KPLAYER_DUMP
static QFile s_dump (QDir::homeDirPath() + "/kioslave.dump");
#endif

static QRegExp re_ext ("^[A-Za-z0-9]+$");
static QRegExp re_a_or_v ("^[AV]: *([0-9,:.-]+)");
static QRegExp re_a_and_v ("^A: *([0-9,:.-]+) +V: *([0-9,:.-]+)");
static QRegExp re_video ("^V(?:IDE)?O: *\\S+ +(\\d+)x(\\d+)");
static QRegExp re_vo ("^V(?:IDE)?O:.* => +(\\d+)x(\\d+)");
static QRegExp re_vbr ("^VIDEO:.* (\\d+,?\\d*) *fps *(\\d+),?\\d* *kbps");
static QRegExp re_abr ("^AUDIO:.*\\((\\d+),?\\d* *kbit\\)");
static QRegExp re_vc ("^Selected video codec: \\[([A-Za-z0-9,:.-]+)\\]");
static QRegExp re_ac ("^Selected audio codec: \\[([A-Za-z0-9,:.-]+)\\]");
static QRegExp re_start ("^(?:Start playing|Starting playback|ZaËÌn·m p¯ehr·vat|Starte Wiedergabe|PÂbegynder afspilning|≈ÍÍﬂÌÁÛÁ ·Ì··Ò·„˘„ﬁÚ|Empezando reproducciÛn|DÈmarre la lecture|Lej·tsz·s indÌt·sa|Inizio la riproduzione|∫∆¿∏≥´ªœ|Ïû¨ÏÉùÏùÑ ÏãúÏûëÌï©ÎãàÎã§|–ü–æ—á–Ω—É–≤–∞ –ø–ª–µ—ò–±–µ–∫–æ—Ç|Start afspelen|Starter avspilling|Zaczynam odtwarzanie|InÌciando reproduÁ„o|Rulez|Ó¡ﬁ¡Ãœ ◊œc–“œ…⁄◊≈ƒ≈Œ…—|ZaËÌnam prehr·vaª|«almaya ba˛lan˝yor|œﬁ¡‘œÀ –“œ«“¡◊¡ŒŒ—|ø™ º≤•∑≈|∂\\}©lºΩ©Ò)\\.\\.\\.", false);
//static QRegExp re_playing ("(?:^(?:Playing|P¯ehr·v·m|Spiele|Afspiller|¡Ì··Ò·„˘„ﬁ ÙÔı|Reproduciendo|Joue|In riproduzione|–ü—É—à—Ç–µ–Ω–æ|Bezig met het afspelen van|Spiller|Odtwarzam|Reproduzindo|Rulez|“œ…«“Ÿ◊¡Œ…≈|Prehr·vam|“œ«“¡◊¡ŒŒ—|≤•∑≈|•ø¶bºΩ©Ò) | (?:lej·tsz·sa|§Ú∫∆¿∏√Ê|Ïû¨ÏÉù Ï§ë|«al˝n˝yor)\\.*$)", false);
static QRegExp re_exiting ("^(?:Exiting|»ÁÎËÁ‡Ï|KonËÌm|Beende| ?Afslutter| ?∏ÓÔ‰ÔÚ|Saliendo|Sortie|KilÈpek|In uscita|Ω™Œª§∑§∆§§§ﬁ§π|Ï¢ÖÎ£åÌï©ÎãàÎã§.|–ò–∑–ª–µ–≥—É–|Bezig met afsluiten|Avslutter|WychodzÍ|Saindo|Ie∫ire|˜Ÿ»œƒ…Õ|KonËÌm|«˝k˝l˝yor|˜…»œƒ…Õœ|’˝‘⁄ÕÀ≥ˆ|•ø¶b∞h•X)", false);
static QRegExp re_quit ("^(?:Exiting|»ÁÎËÁ‡Ï|KonËÌm|Beende| ?Afslutter| ?∏ÓÔ‰ÔÚ|Saliendo|Sortie|KilÈpek|In uscita|Ω™Œª§∑§∆§§§ﬁ§π|Ï¢ÖÎ£åÌï©ÎãàÎã§.|–ò–∑–ª–µ–≥—É–|Bezig met afsluiten|Avslutter|WychodzÍ|Saindo|Ie∫ire|˜Ÿ»œƒ…Õ|KonËÌm|«˝k˝l˝yor|˜…»œƒ…Õœ|’˝‘⁄ÕÀ≥ˆ|•ø¶b∞h•X)\\.\\.\\. \\((?:Quit| ‡È|Konec|Ende|Afslut| ÎÂﬂÛÈÏÔ|Salida\\.?|Fin|KilÈpÈs|Uscita|Ω™Œª|Ï¢ÖÎ£å|–û—Ç–∫–∞–∂–∏|Stop|Avslutt|Wyj∂cie|Sair|Ie∫ire|˜Ÿ»œƒ|Koniec|«˝k˝˛|˜…»¶ƒ|ÕÀ≥ˆ|¬˜∂\\})\\)", false);
static QRegExp re_success ("^(?:Exiting|»ÁÎËÁ‡Ï|KonËÌm|Beende| ?Afslutter| ?∏ÓÔ‰ÔÚ|Saliendo|Sortie|KilÈpek|In uscita|Ω™Œª§∑§∆§§§ﬁ§π|¡æ∑·«’¥œ¥Ÿ|––∑–ª–µ–≥—–≤–∞|Bezig met afsluiten|Avslutter|WychodzÍ|Saindo|Ie∫ire|˜Ÿ»œƒ…Õ|KonËÌm|«˝k˝l˝yor|˜…»œƒ…Õœ|’˝‘⁄ÕÀ≥ˆ|•ø¶b∞h•X)\\.\\.\\. \\((?:End of file| ‡È Ì‡ Ù‡ÈÎ‡|Konec souboru|Ende der Datei|Slut pÂ filen|‘›ÎÔÚ ÙÔı ·Ò˜ÂﬂÔı|Fin del archivo\\.?|Fin du fichier|VÈge a file-nak|Fine del file|•’•°•§•Î§ŒÀˆ√º§«§π|ÌååÏùºÏùò ÎÅù|–ö—Ä–∞—ò –Ω–∞ –¥–∞—Ç–æ—Ç–µ–∫–∞—Ç–∞|Einde van bestand|Slutt pÂ filen|Koniec pliku|Fim do arquivo|Sf‚r∫it fi∫ier|ÎœŒ≈√ ∆¡ Ã¡|Koniec s˙boru|Dosyan˝n Sonu|Î¶Œ≈√ÿ ∆¡ Ã’|Œƒº˛Ω· ¯|¿…Æ◊•Ω∫›)\\)", false);
static QRegExp re_ans_length ("^ANS_LENGTH=(\\d+)$");
static QRegExp re_cache_fill ("^Cache fill: *([0-9]+[.,]?[0-9]*) *%", false);
static QRegExp re_generating_index ("^Generating Index: *([0-9]+[.,]?[0-9]*) *%", false);
static QRegExp re_mpeg12 ("mpeg[12]", false);
static QRegExp re_version ("^MPlayer *0\\.9.* \\(C\\) ");
static QRegExp re_crash ("MPlayer.*(?:crashed|havaroval|st¸rzte|ist abgest¸rzt|fik en alvorlig|Í·Ù›ÒÒÂıÛÂ|se detuvo|a plantÈ|egy 'illeg·lis utasÌt·st'|rˆpke|elcrashelt|Ë stato interrotto|Ë andato in crash|§œ…‘|§œ¡€ƒÍ|Ï¢ÖÎ£åÎêòÏóàÏäµÎãàÎã§|–ø–∞–¥–Ω–∞|crashte|zakoÒczy≥|zakoÒczy≥|falhou|a murit|”ÃœÕ¡Ã”—|zhavaroval|±¿¿£|∑Ì§F°C)");
static QRegExp re_dvd_vcd ("^(?:vcd|dvd|dvb|tv|cdda|cddb)://", false);
static QRegExp re_dvb_hack ("^((?:vcd|dvd|dvb|tv|cdda|cddb)://)kplayer/(.*)$", false);
static QRegExp re_paused ("^ *[=-]+ *(?:PAUSE|œ¿”«¿|POZASTAVENO|–¡’”«|PAUSA|SZ‹NET|ƒ‰ªﬂ|Ïû†ÏãúÎ©àÏ∂§|–ü–ê–£–ó–ê|GEPAUZEERD|PAUZA|PAUSADO|PAUZ√|ÚÈÔÛÙ·ÓÔ˜ÏÂÓÔ|DURAKLADI|·ı˙·|‘›Õ£|º»∞±) *[=-]+ *$");
//static QRegExp re_reading ("^Reading config file", false);

static QCString command_quit ("quit\n");
static QCString command_pause ("pause\n");
static QCString command_visibility ("sub_visibility\n");
static QCString command_length ("get_time_length\n");
static QCString command_seek_100 ("seek 100 1\n");
static QCString command_seek_99 ("seek 99 1\n");
static QCString command_seek_95 ("seek 95 1\n");
static QCString command_seek_90 ("seek 90 1\n");
static QCString command_seek_50 ("seek 50 1\n");

KPlayerLineOutputProcess::KPlayerLineOutputProcess (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Creating MPlayer process\n";
#endif
  m_stdout_line_length = m_stderr_line_length = 0;
  m_stdout_buffer_length = m_stderr_buffer_length = 129;
  m_stdout_buffer = new char [m_stdout_buffer_length];
  m_stderr_buffer = new char [m_stderr_buffer_length];
  m_merge = false;
  QApplication::connect (this, SIGNAL (receivedStdout (KProcess*, char*, int)),
    this, SLOT (slotReceivedStdout (KProcess*, char*, int)));
  QApplication::connect (this, SIGNAL (receivedStderr (KProcess*, char*, int)),
    this, SLOT (slotReceivedStderr (KProcess*, char*, int)));
}

KPlayerLineOutputProcess::~KPlayerLineOutputProcess()
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Destroying MPlayer process\n";
#endif
  delete [] m_stdout_buffer;
  delete [] m_stderr_buffer;
}

void KPlayerLineOutputProcess::processHasExited (int state)
{
  status = state;
  runs = false;
  commClose();
  if ( m_stdout_line_length )
    emit receivedStdoutLine (this, m_stdout_buffer, m_stdout_line_length); // , None
  if ( m_stderr_line_length )
    emit receivedStderrLine (this, m_stderr_buffer, m_stderr_line_length); // , None
  if ( run_mode != DontCare )
    emit processExited (this);
}

void KPlayerLineOutputProcess::slotReceivedStdout (KProcess* proc, char* str, int len)
{
#ifdef DEBUG_KPLAYER_LINEOUT
  kdDebugTime() << "StdOut: " << len << " '" << str << "'\n";
#endif
  receivedOutput (proc, str, len, m_stdout_buffer, m_stdout_buffer_length, m_stdout_line_length, true);
}

void KPlayerLineOutputProcess::slotReceivedStderr (KProcess* proc, char* str, int len)
{
  if ( m_merge )
    slotReceivedStdout (proc, str, len);
  else
    receivedOutput (proc, str, len, m_stderr_buffer, m_stderr_buffer_length, m_stderr_line_length, false);
}

void KPlayerLineOutputProcess::receivedOutput (KProcess* proc, char* str, int len, char* buf, int blen, int llen, bool stdout)
{
  static int avlen = 0;
  static char* av = 0;
  if ( proc != this )
    return;
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
      if ( stdout )
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
      if ( stdout )
        m_stdout_line_length = llen;
      else
        m_stderr_line_length = llen;
    }
    buf [llen] = 0;
    if ( eol - str == len )
      break;
    if ( av && *av && re_paused.search (buf) >= 0 )
    {
#ifdef DEBUG_KPLAYER_LINEOUT
      kdDebugTime() << "Sending AV Buffer On Pause: '" << av << "'\n";
#endif
      if ( stdout )
        emit receivedStdoutLine (this, av, strlen (av) - 1);
      else
        emit receivedStderrLine (this, av, strlen (av) - 1);
      *av = 0;
    }
    if ( re_a_or_v.search (buf) >= 0 || re_cache_fill.search (buf) >= 0 || re_generating_index.search (buf) >= 0 )
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
    else if ( stdout )
      emit receivedStdoutLine (this, buf, llen); // , *cr == '\r' ? CR : LF
    else
      emit receivedStderrLine (this, buf, llen); // , *cr == '\r' ? CR : LF
//  if ( *buf ) // && eol == lf
//  {
//    write (STDOUT_FILENO, buf, llen);
//    write (STDOUT_FILENO, "\n", 1);
//  }
#ifdef DEBUG_KPLAYER_LINEOUT
    kdDebugTime() << "Buffer: '" << buf << "'\n";
#endif
    llen = 0;
    if ( stdout )
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
    if ( stdout )
      emit receivedStdoutLine (this, av, strlen (av) - 1);
    else
      emit receivedStderrLine (this, av, strlen (av) - 1);
    *av = 0;
  }
//kdDebugTime() << "normal return\n";
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
  m_delayed_player = m_delayed_helper = m_sent = m_send_seek = m_send_length = false;
  m_seekable = m_09_version = m_first_chunk = false;
  m_position = m_max_position = m_helper_position = 0;
  m_seek_origin = - MIN_VIDEO_LENGTH;
  m_helper_seek = m_helper_seek_count = m_ans_length = m_absolute_seek = m_seek_count = m_sent_count = m_cache_size = 0;
  m_slave_job = m_temp_job = 0;
  m_send_volume = m_send_contrast = m_send_brightness = m_send_hue = m_send_saturation = false;
  m_send_frame_drop = m_send_subtitle_visibility = false;
  m_send_audio_delay = m_send_subtitle_delay = m_send_subtitle_position = 0;
  m_audio_delay = m_subtitle_delay = m_subtitle_position = 0;
  m_fifo_handle = -1;
  m_fifo_offset = 0;
  m_fifo_notifier = 0;
  m_fifo_timer = 0;
  QString home (QDir::homeDirPath());
  QDir (home).mkdir (".mplayer");
  m_cache.setAutoDelete (true);
  //connect (kPlayerSettings(), SIGNAL (refresh()), this, SLOT (refreshSettings()));
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
    m_slave_job -> kill (true);
  if ( m_temp_job )
    m_temp_job -> kill (true);
  if ( m_temporary_file )
  {
    m_temporary_file -> close();
    m_temporary_file -> unlink();
    delete m_temporary_file;
  }
  removeDataFifo();
}

/*void KPlayerProcess::refreshSettings (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process::refreshSettings\n";
#endif
  if ( ! kPlayerSettings() -> useKioslave() || kPlayerSettings() -> useTemporaryFile() )
    return;
  if ( m_temp_job )
    m_temp_job -> kill (false);
  if ( m_temporary_file )
  {
    m_temporary_file -> close();
    m_temporary_file -> unlink();
    delete m_temporary_file;
    m_temporary_file = 0;
  }
}*/

void KPlayerProcess::transferTemporaryFile (void)
{
  if ( kPlayerSettings() -> useKioslave() && kPlayerSettings() -> useTemporaryFile() && ! m_temporary_file )
  {
    QFileInfo fi (kPlayerSettings() -> url().fileName());
    QString extension (fi.extension(false).lower());
    if ( ! extension.isEmpty() )
      extension = "." + extension;
    m_temporary_file = new KTempFile (locateLocal ("tmp", "kpl"), extension);
#ifdef DEBUG_KPLAYER_PROCESS
    if ( m_temporary_file )
    {
      kdDebugTime() << "Temporary file: " << m_temporary_file -> name() << "\n";
      kdDebugTime() << "Temporary file creation status: " << m_temporary_file -> status() << "\n";
    }
    kdDebugTime() << "Process: Creating temp job\n";
#endif
    m_temp_job = KIO::get (kPlayerSettings() -> url(), false, false);
    m_temp_job -> setWindow (kPlayerWorkspace());
    m_temp_job -> addMetaData ("PropagateHttpHeader", "true");
    connect (m_temp_job, SIGNAL (data (KIO::Job*, const QByteArray&)), this, SLOT (transferTempData (KIO::Job*, const QByteArray&)));
    connect (m_temp_job, SIGNAL (result (KIO::Job*)), this, SLOT (transferTempDone (KIO::Job*)));
    connect (m_temp_job, SIGNAL (percent (KIO::Job*, unsigned long)), this, SLOT (transferProgress (KIO::Job*, unsigned long)));
    connect (m_temp_job, SIGNAL (infoMessage (KIO::Job*, const QString&)), this, SLOT (transferInfoMessage (KIO::Job*, const QString&)));
    transferProgress (m_temp_job, 0);
    m_delayed_helper = true;
  }
}

void KPlayerProcess::load (KURL)
{
  m_position = 0;
  m_delayed_player = m_delayed_helper = false;
  m_size_sent = kPlayerSettings() -> hasVideo() || kPlayerSettings() -> originalSize().isValid();
  m_info_available = kPlayerSettings() -> hasLength();
  if ( m_temp_job )
    m_temp_job -> kill (false);
  if ( m_temporary_file )
  {
    m_temporary_file -> close();
    m_temporary_file -> unlink();
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
  if ( previous == Running && state == Playing && ! kPlayerSettings() -> subtitleVisibility() )
    showSubtitles (false);
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: New state: " << state << ", previous state: " << previous << ", position: " << m_position << "\n";
#endif
  if ( (previous == Running && state != Playing && state != Paused || state == Idle && m_max_position < 0.45) && ! m_quit )
    emit errorDetected();
  if ( ! m_quit || state == Idle )
    emit stateChanged (state, previous);
}

QString KPlayerProcess::positionString (void) const
{
  QString l (kPlayerSettings() -> lengthString()), p (timeString (position(), true));
  return l.isEmpty() ? p : p + " / " + l;
}

void KPlayerProcess::sendHelperCommand (QCString& command)
{
  if ( ! m_helper )
    return;
  m_helper -> writeStdin (command, command.length());
#ifdef DEBUG_KPLAYER_HELPER
  kdDebugTime() << "helper << " << command;
#endif
}

void KPlayerProcess::sendPlayerCommand (QCString& command)
{
  if ( ! m_player )
    return;
  m_player -> writeStdin (command, command.length());
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
#endif
  m_info_available = m_delayed_helper = m_kill = false;
  m_helper_seek = m_helper_seek_count = m_ans_length = 0;
  m_helper_position = 0;
  KPlayerSettings* settings = kPlayerSettings();
  if ( settings -> url().isEmpty() || re_dvd_vcd.search (settings -> url().url()) >= 0 )
    return;
  if ( settings -> useKioslave() )
  {
    if ( ! settings -> useTemporaryFile() )
      return;
    if ( m_temporary_file && m_temporary_file -> handle() >= 0 )
    {
      m_delayed_helper = true;
      return;
    }
  }
  m_helper = new KPlayerLineOutputProcess;
  *m_helper << settings -> executablePath() << "-slave" << "-ao" << "null" << "-vo" << "null";
  if ( settings -> cache() == 1 || ! settings -> url().isLocalFile() && ! settings -> useKioslave() )
    *m_helper << "-nocache";
  else if ( settings -> cache() == 2 )
    *m_helper << "-cache" << QString().setNum (settings -> cacheSize());
  QApplication::connect (m_helper, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*, int)),
    this, SLOT (receivedHelperLine (KPlayerLineOutputProcess*, char*, int)));
  if ( ! run (m_helper) )
  {
    delete m_helper;
    m_helper = 0;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Could not start helper\n";
#endif
    return;
  }
}

void KPlayerProcess::play (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Play\n";
#endif
  KPlayerSettings* settings = kPlayerSettings();
  if ( settings -> url().isEmpty() || ! settings -> properties() )
    return;
  m_position = 0;
  emit progressChanged (m_position, Position);
  start();
}

void KPlayerProcess::start (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Start\n";
#endif
  if ( m_slave_job )
    m_slave_job -> kill (false);
  KPlayerSettings* settings = kPlayerSettings();
  m_position = m_max_position = 0;
  m_seek_count = m_cache_size = m_sent_count = 0;
  m_pausing = m_paused = m_quit = m_kill = m_09_version = m_delayed_player = m_first_chunk = false;
  m_seek = m_success = m_send_seek = m_sent = false;
  m_send_volume = m_send_contrast = m_send_brightness = m_send_hue = m_send_saturation = false;
  m_send_frame_drop = m_send_subtitle_visibility = false;
  m_send_audio_delay = m_send_subtitle_delay = m_send_subtitle_position = 0;
  m_show_subtitles = m_seekable = true;
  m_send_length = ! settings -> hasLength();
  m_cache.clear();
  setState (Running);
  transferTemporaryFile();
  if ( settings -> useKioslave() && settings -> useTemporaryFile()
    && m_temporary_file && m_temporary_file -> handle() >= 0 )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Delaying play...\n";
#endif
    m_delayed_player = true;
    return;
  }
/*if ( m_helper && re_dvd_vcd.search (settings -> url().url()) >= 0 )
  {
    m_delayed_play = true;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Delaying play...\n";
#endif
    return;
  }*/
  m_player = new KPlayerLineOutputProcess;
  *m_player << settings -> executablePath() << "-zoom" << "-noautosub" << "-wid" << QCString().setNum (kPlayerWidget() -> winId());
  QString driver (settings -> videoDriverString());
  if ( ! driver.isEmpty() )
    *m_player << "-vo" << driver;
  driver = settings -> audioDriverString();
  if ( ! driver.isEmpty() )
    *m_player << "-ao" << driver;
  *m_player << "-osdlevel" << QCString().setNum (settings -> osdLevel());
  *m_player << "-contrast" << QCString().setNum (settings -> contrast());
  *m_player << "-brightness" << QCString().setNum (settings -> brightness());
  *m_player << "-hue" << QCString().setNum (settings -> hue());
  *m_player << "-saturation" << QCString().setNum (settings -> saturation());
  if ( settings -> frameDrop() == 1 )
    *m_player << "-framedrop";
  else if ( settings -> frameDrop() == 2 )
    *m_player << "-hardframedrop";
  if ( settings -> useKioslave() && (! settings -> useTemporaryFile() || ! m_temporary_file)
      && settings -> properties() -> cacheOption() == -1 && settings -> cacheDefault() == 0 )
    *m_player << "-cache" << "1024";
  else if ( settings -> cache() == 2 )
    *m_player << "-cache" << QString().setNum (settings -> cacheSize());
  else if ( settings -> cache() == 1 )
    *m_player << "-nocache";
  if ( settings -> videoScaler() > 0 )
    *m_player << "-sws" << QCString().setNum (settings -> videoScaler());
  m_audio_delay = settings -> audioDelay();
  if ( m_audio_delay != 0 )
    *m_player << "-delay" << QCString().setNum (m_audio_delay);
  m_subtitle_delay = settings -> subtitleDelay();
  if ( m_subtitle_delay != 0 && ! settings -> subtitleUrl().isEmpty() )
    *m_player << "-subdelay" << QCString().setNum (m_subtitle_delay);
  m_subtitle_position = settings -> subtitlePosition();
  if ( m_subtitle_position != 100 && ! settings -> subtitleUrl().isEmpty() )
    *m_player << "-subpos" << QCString().setNum (m_subtitle_position);
  if ( settings -> videoDoubleBuffering() )
    *m_player << "-double";
  if ( settings -> videoDirectRendering() && settings -> subtitleUrl().isEmpty() )
    *m_player << "-dr";
  if ( settings -> videoDriver() != "sdl" && settings -> videoDriver() != "svga" )
  {
    QString path (KGlobal::dirs() -> findResource ("appdata", "input.conf"));
#ifdef DEBUG_KPLAYER_PROCESS
    if ( path.isEmpty() )
      kdDebugTime() << "Process: input.conf path is empty\n";
#endif
    if ( path.isEmpty() )
      path = KGlobal::dirs() -> findResource ("data", "kplayer/input.conf");
#ifdef DEBUG_KPLAYER_PROCESS
    if ( path.isEmpty() )
      kdDebugTime() << "Process: input.conf path is still empty\n";
#endif
    if ( ! path.isEmpty() )
      *m_player << "-input" << "conf=" + path;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Input.conf: '" << path << "'\n";
#endif
  }
  if ( ! settings -> subtitleUrl().isEmpty() )
    *m_player << "-sub" << (settings -> subtitleUrl().isLocalFile() ? settings -> subtitleUrl().path() : settings -> subtitleUrl().url());
  *m_player << "-slave";
  if ( settings -> useKioslave() && (! settings -> useTemporaryFile() || ! m_temporary_file) )
  {
    if ( m_temporary_file )
    {
      m_temporary_file -> close();
      m_temporary_file -> unlink();
      delete m_temporary_file;
      m_temporary_file = 0;
    }
    QString ext (settings -> properties() -> type());
    if ( re_ext.search (ext) >= 0 )
      ext.prepend ('.');
    else
      ext = "";
    m_fifo_name = QFile::encodeName (QDir::homeDirPath() + "/.mplayer/kpstream" + ext);
    removeDataFifo();
#ifdef HAVE_MKFIFO
    int rv = ::mkfifo (m_fifo_name, S_IRUSR | S_IWUSR);
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: mkfifo " << m_fifo_name << " returned " << rv << "\n";
#endif
#else
    int rv = ::mknod (m_fifo_name, S_IFIFO | S_IRUSR | S_IWUSR, 0);
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: mknod " << m_fifo_name << " returned " << rv << "\n";
#endif
#endif
  }
  else
    m_fifo_name = QCString();
  QApplication::connect (m_player, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*, int)),
    this, SLOT (receivedStdoutLine (KPlayerLineOutputProcess*, char*, int)));
  if ( ! run (m_player) )
  {
    delete m_player;
    m_player = 0;
    emit messageReceived (i18n("Could not start MPlayer"));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Could not start MPlayer\n";
#endif
    setState (Idle);
    return;
  }
//#ifdef DEBUG_KPLAYER_PROCESS
//if ( ! settings -> hasLength() && re_dvd_vcd.search (settings -> url().url()) >= 0 )
//  kdDebugTime() << "Process: Will send get_time_length for '" << settings -> url().url() << "'\n";
//#endif
  if ( settings -> useKioslave() && (! settings -> useTemporaryFile() || ! m_temporary_file) )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Will send get_time_length for '" << settings -> url().url() << "'\n";
    kdDebugTime() << "Process: Creating slave job\n";
#endif
    m_slave_job = KIO::get (settings -> url(), false, false);
    m_slave_job -> setWindow (kPlayerWorkspace());
    m_slave_job -> addMetaData ("PropagateHttpHeader", "true");
    connect (m_slave_job, SIGNAL (data (KIO::Job*, const QByteArray&)), this, SLOT (transferData (KIO::Job*, const QByteArray&)));
    connect (m_slave_job, SIGNAL (result (KIO::Job*)), this, SLOT (transferDone (KIO::Job*)));
    connect (m_slave_job, SIGNAL (infoMessage (KIO::Job*, const QString&)), this, SLOT (transferInfoMessage (KIO::Job*, const QString&)));
    m_cache_size = settings -> cache() == 2 ? settings -> cacheSize() * 1024 : 1048576;
    m_first_chunk = true;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Cache size: " << m_cache_size << "\n";
#endif
    m_seekable = settings -> playlist();
  }
}

void KPlayerProcess::restart (void)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Restart\n";
#endif
  if ( m_temp_job || ! m_player || ! kPlayerSettings() -> properties() )
    return;
  m_quit = true;
  m_cache.clear();
  if ( m_slave_job )
    m_slave_job -> kill (false);
  m_absolute_seek = int (m_position);
  sendPlayerCommand (command_quit);
  stop (&m_player, &m_quit, m_state != Paused);
  start();
  m_send_seek = true;
}

bool KPlayerProcess::run (KPlayerLineOutputProcess* player)
{
#ifdef DEBUG_KPLAYER_PROCESS
  kdDebugTime() << "Process: Run\n";
#endif
  KPlayerSettings* settings = kPlayerSettings();
  QString codec (settings -> videoCodecString());
  if ( ! codec.isEmpty() )
    *player << "-vc" << codec;
  codec = settings -> audioCodecString();
  if ( ! codec.isEmpty() )
    *player << "-ac" << codec;
  if ( settings -> buildNewIndex() == 0 )
    *player << "-idx";
  else if ( settings -> buildNewIndex() == 2 )
    *player << "-forceidx";
  *player << "-noquiet";
  if ( ! settings -> commandLine().isEmpty() )
    *player << QStringList::split (QChar (' '), settings -> commandLine());
  if ( settings -> playlist() )
    *player << "-playlist";
  else
    *player << "--";
  if ( settings -> useKioslave() )
    *player << (settings -> useTemporaryFile() && m_temporary_file ? QFile::encodeName (m_temporary_file -> name()) : m_fifo_name);
  else if ( settings -> url().isLocalFile() )
    *player << settings -> url().path();
  else if ( re_dvb_hack.search (settings -> url().url()) >= 0 )
    *player << (re_dvb_hack.cap (1) + KURL::decode_string (re_dvb_hack.cap (2)));
  else if ( re_dvd_vcd.search (settings -> url().url()) >= 0 )
    *player << KURL::decode_string (settings -> url().url());
  else
    *player << settings -> url().url();
  QApplication::connect (player, SIGNAL (processExited (KProcess*)),
    this, SLOT (playerProcessExited (KProcess*)));
//QApplication::connect (player, SIGNAL (receivedStderrLine (KPlayerLineOutputProcess*, char*, int)),
//  this, SLOT (receivedStderrLine (KPlayerLineOutputProcess*, char*, int)));
  player -> setMerge (true);
  return player -> start (KProcess::NotifyOnExit, KProcess::All);
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
      if ( (*player) -> isRunning() )
      {
#ifdef DEBUG_KPLAYER_PROCESS
        kdDebugTime() << "Process: MPlayer is running. Waiting...\n";
#endif
        KProcessController::theKProcessController -> waitForProcessExit (1);
      }
      //if ( *player && (*player) -> isRunning() )
      //  KProcessController::theKProcessController -> waitForProcessExit (1);
      //if ( *player && (*player) -> isRunning() )
      //  KProcessController::theKProcessController -> waitForProcessExit (1);
    }
    if ( *quit && *player && (*player) -> isRunning() )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Closing MPlayer...\n";
#endif
      (*player) -> kill();
      KProcessController::theKProcessController -> waitForProcessExit (1);
      if ( *quit && *player && (*player) -> isRunning() )
      {
#ifdef DEBUG_KPLAYER_PROCESS
        kdDebugTime() << "Process: Killing MPlayer...\n";
#endif
        (*player) -> kill (SIGKILL);
        KProcessController::theKProcessController -> waitForProcessExit (1);
        if ( *quit && *player && (*player) -> isRunning() )
        {
#ifdef DEBUG_KPLAYER_PROCESS
          kdDebugTime() << "Process: Could not shut down MPlayer\n";
#endif
          (*player) -> detach();
        }
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
    m_temp_job -> kill (false);
    if ( m_temporary_file )
    {
      m_temporary_file -> close();
      m_temporary_file -> unlink();
      delete m_temporary_file;
      m_temporary_file = 0;
    }
  }
  m_cache.clear();
  if ( m_slave_job )
    m_slave_job -> kill (false);
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
    m_temp_job -> kill (false);
    if ( m_temporary_file )
    {
      m_temporary_file -> close();
      m_temporary_file -> unlink();
      delete m_temporary_file;
      m_temporary_file = 0;
    }
  }
  m_cache.clear();
  if ( m_slave_job )
    m_slave_job -> kill (false);
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
  QCString s ("seek ");
  // broken codec workaround
  if ( kPlayerSettings() -> length() >= MIN_VIDEO_LENGTH
    && re_mpeg12.search (kPlayerSettings() -> properties() -> videoCodecValue()) >= 0
    && re_dvd_vcd.search (kPlayerSettings() -> url().url()) < 0 )
  {
    seconds = limit (int (float (seconds) / kPlayerSettings() -> properties() -> length() * 100 + 0.5), 0, 100);
    s += QCString().setNum (seconds) + " 1\n";
  }
  else
    s += QCString().setNum (seconds) + " 2\n";
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
  QCString s ("seek ");
  // broken codec workaround
  if ( (seconds > 4 || seconds < -4) && kPlayerSettings() -> length() >= MIN_VIDEO_LENGTH
    && re_mpeg12.search (kPlayerSettings() -> properties() -> videoCodecValue()) >= 0
    && re_dvd_vcd.search (kPlayerSettings() -> url().url()) < 0 )
  {
    //seconds = limit (int ((m_position + seconds) / kPlayerSettings() -> properties() -> length() * 100 + 0.5), 0, 100);
    //s += QCString().setNum (seconds) + " 1\n";
    //if ( m_send_seek )
    //  m_absolute_seek += seconds;
    //else
    absoluteSeek (int (m_position + seconds + 0.5));
    return;
  }
  else
    s += QCString().setNum (seconds) + "\n";
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
  QCString s ("volume ");
  s += QCString().setNum (volume) + " 1\n";
  sendPlayerCommand (s);
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
  QCString s ("frame_drop ");
  s += QCString().setNum (frame_drop) + "\n";
  sendPlayerCommand (s);
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
  QCString s ("contrast ");
  s += QCString().setNum (contrast) + " 1\n";
  sendPlayerCommand (s);
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
  QCString s ("brightness ");
  s += QCString().setNum (brightness) + " 1\n";
  sendPlayerCommand (s);
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
  QCString s ("hue ");
  s += QCString().setNum (hue) + " 1\n";
  sendPlayerCommand (s);
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
  QCString s ("saturation ");
  s += QCString().setNum (saturation) + " 1\n";
  sendPlayerCommand (s);
  m_send_saturation = false;
}

void KPlayerProcess::showSubtitles (bool show)
{
  if ( ! m_player || m_quit || state() != Playing && state() != Running || kPlayerSettings() -> subtitleUrl().isEmpty() || m_show_subtitles == show )
    return;
  if ( m_sent || state() == Running )
  {
    m_send_subtitle_visibility = ! m_send_subtitle_visibility;
    return;
  }
  if ( ! m_send_subtitle_visibility )
    sendPlayerCommand (command_visibility);
  m_send_subtitle_visibility = false;
  m_show_subtitles = show;
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
  QCString s ("sub_pos ");
  s += QCString().setNum (position) + "\n";
  sendPlayerCommand (s);
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
  QCString s ("sub_delay ");
  s += QCString().setNum (- delay) + "\n";
  sendPlayerCommand (s);
  m_send_subtitle_delay = 0;
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
  QCString s ("audio_delay ");
  s += QCString().setNum (- delay) + "\n";
  sendPlayerCommand (s);
  m_send_audio_delay = 0;
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
      m_cache.append (new QByteArray (data.copy()));
    }
    else
    {
      QByteArray* array = m_cache.last();
      int size = array -> size();
      array -> resize (size + data.size(), QGArray::SpeedOptim);
#ifdef DEBUG_KPLAYER_KIOSLAVE
      if ( array -> size() != size + data.size() )
        kdDebugTime() << "Process: Cache: Size mismatch: " << size << " + " << data.size() << " = " << array -> size() << "\n";
      else
        kdDebugTime() << "Process: Cache: Appended to chunk " << m_cache.count() << " size " << size << " + " << data.size() << " = " << array -> size() << "\n";
#endif
      memcpy (array -> data() + size, data.data(), data.size());
    }
    if ( m_cache.count() > 1 && ! m_slave_job -> isSuspended() && m_cache.last() -> size() >= m_cache_size )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Suspending transfer job\n";
#endif
      m_slave_job -> suspend();
    }
    if ( m_cache.count() == 1 && (! m_first_chunk || m_cache.first() -> size() >= m_cache_size) )
    {
      if ( m_first_chunk && ! m_quit )
        emit progressChanged (100, CacheFill);
      sendFifoData();
    }
    else if ( m_first_chunk && ! m_quit )
      emit progressChanged (limit (int ((m_cache.first() -> size() * 100 + m_cache_size / 2) / m_cache_size), 0, 100), CacheFill);
  }
  else
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Stray transfer job\n";
#endif
    m_cache.clear();
    if ( job )
      job -> kill (true);
  }
}

void KPlayerProcess::transferTempData (KIO::Job* job, const QByteArray& data)
{
  if ( job && job == m_temp_job && m_temporary_file )
  {
#ifdef DEBUG_KPLAYER_KIOSLAVE
    int rv =
#endif
    m_temporary_file -> file() -> writeBlock (data);
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
      job -> kill (true);
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
          KURL url (kPlayerSettings() -> url());
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
    else if ( m_cache.count() == 1 && m_first_chunk && m_cache.first() -> size() < m_cache_size && ! m_quit )
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
      KURL url (kPlayerSettings() -> url());
      QStringList errors (job -> detailedErrorStrings (&url));
      for ( QStringList::Iterator it = errors.begin(); it != errors.end(); ++ it )
        if ( ! (*it).isEmpty() )
          emit messageReceived (*it);*/
      emit errorDetected();
      if ( m_temporary_file )
      {
        m_temporary_file -> close();
        m_temporary_file -> unlink();
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
        m_temporary_file -> unlink();
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
      QObject::connect (m_fifo_notifier, SIGNAL (activated (int)), this, SLOT (playerDataWritten (int)));
    }
    else if ( ! m_fifo_timer )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: fifo open failed, creating timer\n";
#endif
      m_fifo_timer = new QTimer (this);
      connect (m_fifo_timer, SIGNAL (timeout()), this, SLOT (sendFifoData()));
      m_fifo_timer -> start (100);
    }
  }
  if ( m_fifo_handle >= 0 )
  {
    QByteArray* array = m_cache.first();
    if ( array && array -> size() > m_fifo_offset )
    {
#ifdef DEBUG_KPLAYER_KIOSLAVE
      kdDebugTime() << "Process: Cache: Writing " << array -> size() << " - " << m_fifo_offset << " bytes to fifo\n";
#endif
      int rv = ::write (m_fifo_handle, array -> data() + m_fifo_offset, array -> size() - m_fifo_offset);
      if ( rv > 0 )
      {
#ifdef DEBUG_KPLAYER_DUMP
        if ( ! s_dump.isOpen() )
          s_dump.open (IO_WriteOnly);
        s_dump.writeBlock (array -> data() + m_fifo_offset, rv);
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
    QByteArray* array = m_cache.first();
    if ( array && array -> size() <= m_fifo_offset )
    {
#ifdef DEBUG_KPLAYER_KIOSLAVE
      kdDebugTime() << "Process: Cache: Wrote " << array -> size() << " byte chunk, offset " << m_fifo_offset << "\n";
#endif
      m_cache.remove();
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

void KPlayerProcess::playerProcessExited (KProcess *proc)
{
  if ( proc == m_player )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: MPlayer process exited\n";
#endif
    delete m_player;
    m_player = 0;
    if ( kPlayerSettings() -> properties() && m_success && ! m_seek && m_position >= MIN_VIDEO_LENGTH )
    {
      kPlayerSettings() -> properties() -> setLength (m_position);
      m_info_available = true;
      emit infoAvailable();
      kPlayerSettings() -> properties() -> save();
    }
    m_cache.clear();
    if ( m_slave_job )
      m_slave_job -> kill (false);
    removeDataFifo();
    m_fifo_name = QCString();
    if ( ! m_quit )
      setState (Idle);
  }
  else if ( proc == m_helper )
  {
#ifdef DEBUG_KPLAYER_HELPER
    kdDebugTime() << "MPlayer helper process exited\n";
#endif
    delete m_helper;
    m_helper = 0;
    if ( kPlayerSettings() -> properties() && m_helper_seek < 500 && m_helper_position >= MIN_VIDEO_LENGTH )
      kPlayerSettings() -> properties() -> setLength (m_helper_position);
    if ( kPlayerSettings() -> properties()
        && (kPlayerSettings() -> properties() -> length() < MIN_VIDEO_LENGTH * 4 && m_ans_length > MIN_VIDEO_LENGTH * 10
        || kPlayerSettings() -> properties() -> length() == 0 && m_ans_length > 0) )
      kPlayerSettings() -> properties() -> setLength (m_ans_length);
    m_info_available = true;
    if ( ! m_kill )
      emit infoAvailable();
    if ( ! m_size_sent && ! m_kill && m_helper_seek > 0 )
    {
      emit sizeAvailable();
      m_size_sent = true;
    }
    if ( ! m_kill && kPlayerSettings() -> properties() )
      kPlayerSettings() -> properties() -> save();
    /*if ( m_delayed_play && ! m_player )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Delayed play...\n";
#endif
      play();
    }*/
  }
  else
  {
    delete proc;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Stray MPlayer process exited\n";
#endif
  }
}

float KPlayerProcess::stringToFloat (QString stime)
{
  int comma = stime.find (',');
  if ( comma >= 0 )
    stime [comma] = '.';
  QStringList sl = QStringList::split (':', stime);
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

void KPlayerProcess::receivedStdoutLine (KPlayerLineOutputProcess* proc, char* str, int len)
{
  if ( proc != m_player )
  {
    char buf [1025];
    if ( len > 1024 )
      len = 1024;
    memcpy (buf, str, len);
    buf [len] = 0;
    if ( re_exiting.search (buf) < 0 )
      proc -> writeStdin (command_quit, command_quit.length());
    return;
  }
  static float prev_position = -1;
  float ftime;
  if ( state() == Running )
    kPlayerWidget() -> sendConfigureEvent();
  if ( re_version.search (str) >= 0 )
  {
    m_09_version = true;
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: MPlayer 0.9x detected\n";
#endif
  }
  if ( re_vo.search (str) >= 0 )
  {
    if ( ! kPlayerSettings() -> hasVideo() )
      m_size_sent = false;
    if ( kPlayerSettings() -> properties() )
    {
      kPlayerSettings() -> properties() -> setOriginalSize (QSize (re_vo.cap(1).toInt(), re_vo.cap(2).toInt()));
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Adjusted Width " << kPlayerSettings() -> properties() -> originalSize().width() << " Height " << kPlayerSettings() -> properties() -> originalSize().height() << "\n";
#endif
    }
    if ( ! m_size_sent && ! m_quit )
    {
      emit sizeAvailable();
      m_size_sent = true;
    }
  }
  else if ( kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> originalSize().isEmpty() && re_video.search (str) >= 0 )
  {
    if ( ! kPlayerSettings() -> hasVideo() )
      m_size_sent = false;
    kPlayerSettings() -> properties() -> setOriginalSize (QSize (re_video.cap(1).toInt(), re_video.cap(2).toInt()));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Width " << kPlayerSettings() -> properties() -> originalSize().width() << " Height " << kPlayerSettings() -> properties() -> originalSize().height() << "\n";
#endif
  }
  if ( kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> length() <= 0
    && re_ans_length.search (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Player: Matched ans_length: " << re_ans_length.cap (1) << " in " << re_ans_length.cap (0) << "\n";
#endif
    kPlayerSettings() -> properties() -> setLength (re_ans_length.cap (1).toInt());
    m_info_available = true;
    if ( ! m_quit )
    {
      emit infoAvailable();
      kPlayerSettings() -> properties() -> save();
    }
  }
  if ( re_paused.search (str) >= 0 )
  {
    m_paused = true;
    m_pausing = false;
    m_sent = false;
    setState (Paused);
  }
//if ( m_state == Running && ! m_slave_job && m_fifo_handle < 0 && ! m_cache.isEmpty() && ! m_fifo_name.isEmpty()
//    && (re_reading.search (str) >= 0 || re_playing.search (str) >= 0 || re_start.search (str) >= 0) )
//  sendFifoData();
  if ( m_state == Running && (m_pausing || m_send_seek) && ! m_sent && ! m_quit && re_start.search (str) >= 0 )
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
  if ( re_success.search (str) >= 0 && ! m_quit )
    m_success = true;
  if ( re_exiting.search (str) >= 0 && re_quit.search (str) < 0 && re_success.search (str) < 0 && ! m_quit )
    emit errorDetected();
  if ( re_crash.search (str) >= 0 )
    emit errorDetected();
  if ( kPlayerSettings() -> properties() && re_vbr.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setFramerate (stringToFloat (re_vbr.cap(1)));
    kPlayerSettings() -> properties() -> setVideoBitrate (re_vbr.cap(2).toInt());
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Framerate " << kPlayerSettings() -> properties() -> framerate() << "\n";
    kdDebugTime() << "Process: Video bitrate " << kPlayerSettings() -> properties() -> videoBitrate() << "\n";
#endif
  }
  if ( kPlayerSettings() -> properties() && re_abr.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setAudioBitrate (re_abr.cap(1).toInt());
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Audio bitrate " << kPlayerSettings() -> properties() -> audioBitrate() << "\n";
#endif
  }
  if ( kPlayerSettings() -> properties() && re_vc.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setVideoCodecValue (re_vc.cap(1));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Video codec " << kPlayerSettings() -> properties() -> videoCodecValue() << "\n";
#endif
  }
  if ( kPlayerSettings() -> properties() && re_ac.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setAudioCodecValue (re_ac.cap(1));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Audio codec " << kPlayerSettings() -> properties() -> audioCodecValue() << "\n";
#endif
  }
//kdDebugTime() << "matching a_or_v regex\n";
  if ( re_a_or_v.search (str) >= 0 )
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
      if ( ! m_quit && kPlayerSettings() -> properties() )
        kPlayerSettings() -> properties() -> save();
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
    if ( m_send_volume && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending volume\n";
#endif
      volume (kPlayerSettings() -> volume());
    }
    if ( m_send_frame_drop && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending frame drop\n";
#endif
      frameDrop (kPlayerSettings() -> frameDrop());
    }
    if ( m_send_contrast && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending contrast\n";
#endif
      contrast (kPlayerSettings() -> contrast());
    }
    if ( m_send_brightness && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending brightness\n";
#endif
      brightness (kPlayerSettings() -> brightness());
    }
    if ( m_send_hue && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending hue\n";
#endif
      hue (kPlayerSettings() -> hue());
    }
    if ( m_send_saturation && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending saturation\n";
#endif
      saturation (kPlayerSettings() -> saturation());
    }
    if ( m_send_subtitle_visibility && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitle visibility\n";
#endif
      sendPlayerCommand (command_visibility);
      m_send_subtitle_visibility = false;
    }
    if ( (m_send_audio_delay >= 0.001 || m_send_audio_delay <= - 0.001) && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending audio delay\n";
#endif
      QCString s ("audio_delay ");
      s += QCString().setNum (- m_send_audio_delay) + "\n";
      sendPlayerCommand (s);
      m_send_audio_delay = 0;
    }
    if ( (m_send_subtitle_delay >= 0.001 || m_send_subtitle_delay <= - 0.001) && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitle delay\n";
#endif
      QCString s ("sub_delay ");
      s += QCString().setNum (- m_send_subtitle_delay) + "\n";
      sendPlayerCommand (s);
      m_send_subtitle_delay = 0;
    }
    if ( m_send_subtitle_position && ! m_sent )
    {
#ifdef DEBUG_KPLAYER_PROCESS
      kdDebugTime() << "Process: Sending subtitle position\n";
#endif
      QCString s ("sub_pos ");
      s += QCString().setNum (m_send_subtitle_position) + "\n";
      sendPlayerCommand (s);
      m_send_subtitle_position = 0;
    }
//  kdDebugTime() << "regex matched\n";
    if ( re_a_and_v.search (str) >= 0 )
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
//  if ( kPlayerSettings() -> properties() && ftime > kPlayerSettings() -> properties() -> length() && ftime >= MIN_VIDEO_LENGTH && kPlayerSettings() -> properties() -> length() > 0 )
    if ( kPlayerSettings() -> properties() && ftime > kPlayerSettings() -> properties() -> length() && kPlayerSettings() -> properties() -> length() >= MIN_VIDEO_LENGTH )
      kPlayerSettings() -> properties() -> setLength (ftime);
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
        m_seek_origin = - MIN_VIDEO_LENGTH;
        emit progressChanged (m_position, Position);
        m_seek_count = 0;
      }
//    kdDebugTime() << "position: " << m_position << " length: " << kPlayerSettings() -> properties() -> length() << "\n";
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
    if ( m_send_length && ! m_sent && ! kPlayerSettings() -> hasLength() )
    {
      sendPlayerCommand (command_length);
      m_send_length = false;
    }
  }
  else if ( re_cache_fill.search (str) >= 0 )
  {
    ftime = stringToFloat (re_cache_fill.cap (1));
#ifdef DEBUG_KPLAYER_PROGRESS
    kdDebugTime() << "Received cache progress: '" << str << "' -> " << ftime << "\n";
#endif
    if ( ! m_quit && ! m_first_chunk )
      emit progressChanged (ftime, CacheFill);
  }
  else if ( re_generating_index.search (str) >= 0 )
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

void KPlayerProcess::receivedHelperLine (KPlayerLineOutputProcess* proc, char* str, int len)
{
  if ( proc != m_helper )
  {
    char buf [1025];
    if ( len > 1024 )
      len = 1024;
    memcpy (buf, str, len);
    buf [len] = 0;
    if ( re_exiting.search (buf) < 0 )
      proc -> writeStdin (command_quit, command_quit.length());
    return;
  }
  bool sent = false;
  float ftime;
#ifdef DEBUG_KPLAYER_HELPER
  kdDebugTime() << "helper >> " << str << "\n";
#endif
//kdDebugTime() << "matching video regex\n";
  if ( kPlayerSettings() -> properties() && re_vo.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setOriginalSize (QSize (re_vo.cap(1).toInt(), re_vo.cap(2).toInt()));
#ifdef DEBUG_KPLAYER_HELPER
    kdDebugTime() << "Adjusted Width " << kPlayerSettings() -> properties() -> originalSize().width() << " Height " << kPlayerSettings() -> properties() -> originalSize().height() << "\n";
#endif
    if ( ! m_size_sent && ! m_kill )
    {
      emit sizeAvailable();
      m_size_sent = true;
    }
  }
  else if ( kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> originalSize().isEmpty() && re_video.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setOriginalSize (QSize (re_video.cap(1).toInt(), re_video.cap(2).toInt()));
#ifdef DEBUG_KPLAYER_HELPER
    kdDebugTime() << "Width " << kPlayerSettings() -> properties() -> originalSize().width() << " Height " << kPlayerSettings() -> properties() -> originalSize().height() << "\n";
#endif
  }
  if ( re_ans_length.search (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_HELPER
    kdDebugTime() << "Helper: Matched ans_length: " << re_ans_length.cap (1) << " in " << re_ans_length.cap (0) << "\n";
#endif
    m_ans_length = re_ans_length.cap (1).toInt();
    if ( m_helper_seek == 1 )
      m_helper_seek_count = 9;
  }
  if ( kPlayerSettings() -> properties() && ! kPlayerSettings() -> properties() -> videoBitrate() && re_vbr.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setFramerate (stringToFloat (re_vbr.cap(1)));
    kPlayerSettings() -> properties() -> setVideoBitrate (re_vbr.cap(2).toInt());
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Framerate " << kPlayerSettings() -> properties() -> framerate() << "\n";
    kdDebugTime() << "Process: Video bitrate " << kPlayerSettings() -> properties() -> videoBitrate() << "\n";
#endif
  }
  if ( kPlayerSettings() -> properties() && ! kPlayerSettings() -> properties() -> audioBitrate() && re_abr.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setAudioBitrate (re_abr.cap(1).toInt());
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Audio bitrate " << kPlayerSettings() -> properties() -> audioBitrate() << "\n";
#endif
  }
  if ( kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> videoCodecValue().isEmpty() && re_vc.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setVideoCodecValue (re_vc.cap(1));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Video codec " << kPlayerSettings() -> properties() -> videoCodecValue() << "\n";
#endif
  }
  if ( kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> audioCodecValue().isEmpty() && re_ac.search (str) >= 0 )
  {
    kPlayerSettings() -> properties() -> setAudioCodecValue (re_ac.cap(1));
#ifdef DEBUG_KPLAYER_PROCESS
    kdDebugTime() << "Process: Audio codec " << kPlayerSettings() -> properties() -> audioCodecValue() << "\n";
#endif
  }
//kdDebugTime() << "matching a_or_v regex\n";
  if ( re_a_or_v.search (str) >= 0 )
  {
//  kdDebugTime() << "regex matched\n";
    if ( re_a_and_v.search (str) >= 0 )
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
          && kPlayerSettings() -> properties() && ftime > kPlayerSettings() -> properties() -> length()
          && kPlayerSettings() -> properties() -> length() > 0 )
        kPlayerSettings() -> properties() -> setLength (ftime);
      if ( ftime != m_helper_position )
      {
        m_helper_position = ftime;
#ifdef DEBUG_KPLAYER_HELPER
        if ( kPlayerSettings() -> properties() )
          kdDebugTime() << "helper position: " << m_helper_position << " length: " << kPlayerSettings() -> properties() -> length() << "\n";
#endif
      }
      if ( m_helper_seek > 0 && m_helper_seek < 500 && ftime >= MIN_VIDEO_LENGTH )
      {
        float estlength = ftime * 100 / m_helper_seek;
        if ( kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> length() < estlength )
        {
          kPlayerSettings() -> properties() -> setLength (estlength);
          m_info_available = true;
          emit infoAvailable();
        }
#ifdef DEBUG_KPLAYER_HELPER
        if ( kPlayerSettings() -> properties() )
          kdDebugTime() << "estimated length: " << kPlayerSettings() -> properties() -> length() << "\n";
#endif
      }
    }
    if ( m_helper_seek == 0 && ! sent )
    {
      sendHelperCommand (command_length);
      m_helper_seek = 1;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek == 1 && ! sent )
    {
      sendHelperCommand (command_seek_99);
      m_helper_seek = 99;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek == 99 && ! sent && kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> length() < MIN_VIDEO_LENGTH )
    {
      sendHelperCommand (command_seek_95);
      m_helper_seek = 95;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek == 95 && ! sent && kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> length() < MIN_VIDEO_LENGTH )
    {
      sendHelperCommand (command_seek_90);
      m_helper_seek = 90;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek == 90 && ! sent && kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> length() < MIN_VIDEO_LENGTH )
    {
      sendHelperCommand (command_seek_50);
      m_helper_seek = 50;
      m_helper_seek_count = 0;
      sent = true;
    }
    if ( m_helper_seek < 100 && kPlayerSettings() -> properties() && kPlayerSettings() -> properties() -> length() >= MIN_VIDEO_LENGTH )
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

/*void KPlayerProcess::receivedStderrLine (KPlayerLineOutputProcess*, char* str, int len)
{
  write (STDERR_FILENO, str, len);
  write (STDERR_FILENO, "\n", 1);
}*/
