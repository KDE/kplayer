/***************************************************************************
                          kplayerengine.cpp
                          -----------------
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

#include "kplayerengine.h"
#include "kplayeractionlist.h"
#include "kplayerpropertiesdialog.h"
#include "kplayersettings.h"
#include "kplayerslideraction.h"
#include "kplayerwidget.h"

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <krecentdocument.h>
#include <kurlrequester.h>
#include <kurlrequesterdialog.h>
#include <qaction.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qregexp.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_ENGINE
#endif

KPlayerEngine* KPlayerEngine::m_engine = 0;

static QRegExp re_extension ("\\.[^/.]+$");
static QRegExp re_driver ("^\\s+([A-Za-z0-9]+)\\s+(\\S.*)");
static QRegExp re_codec ("^([A-Za-z0-9]+)\\s+\\S+\\s+\\S+\\s+(\\S.*)");
static QRegExp re_demuxer ("^\\s*([A-Za-z0-9]+)\\s+\\d+\\s+(\\S.*)");
static QRegExp re_multiple ("\\s+"), re_trailing (" $");
static QRegExp re_brackets (" ?\\[.*\\]"), re_parentheses (" ?\\((?:..+|\\?)\\)");
static QRegExp re_audio ("[- ](?:audio[- ])?(?:decoder|codec|output|out)s?", Qt::CaseInsensitive);
static QRegExp re_video ("[- ](?:video[- ])?(?:decoder|codec|output|out)s?", Qt::CaseInsensitive);
static QRegExp re_s ("'s", Qt::CaseInsensitive), re_layer ("layer-", Qt::CaseInsensitive);
static QRegExp re_dash (" - ", Qt::CaseInsensitive);
static QRegExp re_writer (" writer", Qt::CaseInsensitive);
static QRegExp re_demux (" demuxer", Qt::CaseInsensitive);
static QRegExp re_sega (" for Sega Saturn CD-ROM games", Qt::CaseInsensitive);
static QRegExp re_smjpeg ("smjpeg", Qt::CaseInsensitive);
static QRegExp re_tv_card ("Tv card", Qt::CaseInsensitive);
static QRegExp re_lmlm (" Compression Card stream", Qt::CaseInsensitive);
static QRegExp re_xv ("/XV$", Qt::CaseInsensitive), re_vidix (" \\(VIDIX\\)", Qt::CaseInsensitive);
static QRegExp re_xover ("General X11 driver for overlay capable video output drivers", Qt::CaseInsensitive);
static QRegExp re_opengl ("\\(OpenGL\\)", Qt::CaseInsensitive);
static QRegExp re_matrox ("Matrox .* overlay in", Qt::CaseInsensitive);
static QRegExp re_matroxg (" G200/G400/G450/G550", Qt::CaseInsensitive);
static QRegExp re_macintosh ("Macintosh Audio Compression and Expansion", Qt::CaseInsensitive);
static QRegExp re_amu ("Avid Meridien Uncompressed", Qt::CaseInsensitive);
static QRegExp re_speech ("Windows Media Audio 9 Speech", Qt::CaseInsensitive);

void KPlayerSetResizing (bool resizing)
{
  if ( KPlayerEngine::engine() )
    KPlayerEngine::engine() -> setResizing (resizing);
}

void KPlayerWindowStateChanged (uint wid)
{
  if ( kPlayerEngine() )
    kPlayerEngine() -> emitWindowStateChanged (wid);
}

int listIndex (const QStringList& sl, const QString& str)
{
  QString entry, stru (str.toUpper()), strusc (stru + ":");
  int index = 0;
  for ( QStringList::ConstIterator sli = sl.begin(); sli != sl.end(); ++ sli )
  {
    entry = (*sli).toUpper();
    if ( entry == stru || entry.startsWith (strusc) )
      return index;
    index ++;
  }
  return -1;
}

KPlayerFileDialog::KPlayerFileDialog (const QString& dir, const QString& filter, QWidget* parent)
  : KFileDialog (dir, filter, parent ? parent : kPlayerWidget())
{
  //ops -> clearHistory();
}

KPlayerEngine::KPlayerEngine (KActionCollection* ac, QWidget* parent, KConfig* config)
  : m_timer (this)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Creating engine\n";
#endif
  m_engine = this;
  m_ac = ac;
  m_light = config == 0;
  m_progress_factor = m_timer_ticks = 0;
  m_stop = m_updating = m_zooming = m_resizing = false;
  m_pending_resize = m_dockwidget_resize = false;
  m_layout_user_interaction = false;
  m_play_pending = m_enable_screen_saver = m_amixer_running = false;
  m_config = light() ? new KConfig ("kplayerrc") : config;
  m_store = new KConfig ("kplayerlibraryrc");
  m_meta = new KConfig ("kplayerplaylistrc");
  m_configuration = new KPlayerConfiguration;
  configuration() -> setup();
  m_settings = new KPlayerSettings;
  m_process = new KPlayerProcess;
  m_workspace = new KPlayerWorkspace (parent);
  m_widget = workspace() -> widget();
  connect (&m_timer, SIGNAL (timeout()), SLOT (layoutTimerTick()));
  connect (workspace(), SIGNAL (resized()), SLOT (workspaceResize()));
  connect (process(), SIGNAL (stateChanged(KPlayerProcess::State, KPlayerProcess::State)), this, SLOT (playerStateChanged(KPlayerProcess::State, KPlayerProcess::State)));
  connect (process(), SIGNAL (progressChanged(float, KPlayerProcess::ProgressType)), this, SLOT (playerProgressChanged(float, KPlayerProcess::ProgressType)));
  connect (process(), SIGNAL (infoAvailable()), this, SLOT (playerInfoAvailable()));
  connect (process(), SIGNAL (sizeAvailable()), this, SLOT (playerSizeAvailable()));
  connect (configuration(), SIGNAL (updated()), this, SLOT (refreshSettings()));
  setupActions();
  m_audio_drivers
    << "mpegpes: DVB"
    << "oss: OSS/ioctl"
    << "alsa: ALSA"
    << "arts: aRts"
    << "esd: EsounD"
    << "nas: NAS"
    << "sdl: SDLlib"
    << "null: Null"
    << "pcm: RAW PCM/WAVE file"
    << "plugin: Plugin";
  m_audio_codecs
    << "wma9dmo: Windows Media Audio 9 DMO"
    << "wmadmo: Windows Media Audio DMO"
    << "qdmc: Quicktime QDMC/QDM2"
    << "qclp: Quicktime QCLP"
    << "qtmace3: Quicktime MACE3"
    << "qtmace6: Quicktime MACE6"
    << "racook: RealAudio COOK"
    << "rasipr: RealAudio Sipro"
    << "raatrc: RealAudio ATRAC3"
    << "imaadpcm: IMA ADPCM"
    << "msadpcm: MS ADPCM"
    << "dk4adpcm: Duck DK4 ADPCM"
    << "dk3adpcm: Duck DK3 ADPCM"
    << "roqaudio: Id RoQ File"
    << "libdv: raw DV"
    << "ffdv: FFmpeg DV"
    << "faad: AAC MPEG2/MPEG4 Audio"
    << "pcm: Uncompressed PCM"
    << "divx: DivX audio"
    << "msadpcmacm: MS ADPCM"
    << "mp3: mp3lib MPEG layer 2, layer 3"
    << "ffwmav1: DivX audio v1"
    << "ffwmav2: DivX audio v2"
    << "ffmac3: Macintosh Audio Comp. and Exp. 3:1"
    << "ffmac6: Macintosh Audio Comp. and Exp. 6:1"
    << "ffmp3: FFmpeg layer 3"
    << "ffmp2: FFmpeg layer 1,2"
    << "mad: libMAD MPEG layer 1-2-3"
    << "mp3acm: MPEG layer 3"
    << "imaadpcmacm: IMA ADPCM"
    << "msgsmacm: MS GSM"
    << "msnaudio: MSN AUDIO"
    << "msgsm: MS GSM"
    << "alaw: aLaw"
    << "ulaw: uLaw"
    << "dvdpcm: Uncompressed DVD/VOB LPCM"
    << "a52: AC3-liba52"
    << "ac3: AC3-libac3"
    << "voxware: VoxWare"
    << "acelp: ACELP.net Sipro Lab"
    << "imc: Intel Music Coder"
    << "hwac3: AC3 through SPDIF"
    << "vorbis: OggVorbis"
    << "vivoaudio: Vivo G.723/Siren"
    << "g72x: G.711/G.721/G.723"
    << "atrac3: Sony ATRAC3"
    << "ALF2: ALF2"
    << "truespeech: DSP Group TrueSpeech"
    << "voxwarert24: VoxWare RT24 speech"
    << "lhacm: Lernout & Hauspie CELP and SBC";
  m_video_drivers
    << "xmga: Matrox overlay in X11 window"
    << "mga: Matrox G200/G4x0/G550 overlay"
    << "3dfx: 3dfx"
    << "tdfxfb: 3Dfx Banshee/Voodoo3/Voodoo5"
    << "xv: X11/XVideo"
    << "xvmc: XVideo Motion Compensation"
    << "x11: X11"
    << "xover: X11 overlay"
    << "gl: X11 OpenGL"
    << "gl2: X11 OpenGL - multiple textures version"
    << "dga: DGA"
    << "sdl: SDL YUV/RGB/BGR renderer"
    << "ggi: General Graphics Interface"
    << "fbdev: Framebuffer Device"
    << "fbdev2: Framebuffer Device"
    << "svga: SVGAlib"
    << "aa: AAlib"
    << "caca: libcaca"
    << "dxr3: DXR3/H+ video out"
    << "png: PNG file"
    << "jpeg: JPEG file"
    << "gif89a: animated GIF"
    << "null: Null video"
    << "pgm: PGM file"
    << "md5: MD5 sum"
    << "mpegpes: Mpeg-PES to DVB card"
    << "yuv4mpeg: yuv4mpeg for mjpegtools"
    << "vesa: VESA VBE 2.0 video"
    << "directfb: Direct Framebuffer Device"
    << "dfbmga: DirectFB / Matrox"
    << "xvidix: X11/VIDIX"
    << "cvidix: console VIDIX"
    << "tga: Targa";
  m_video_codecs
    << "mpegpes: Mpeg PES"
    << "mpeg12: MPEG 1 or 2"
    << "ffmpeg12: FFmpeg MPEG 1/2"
    << "nuv: NuppelVideo"
    << "mpng: PNG images"
    << "mtga: TGA images"
    << "sgi: SGI images"
    << "fli: Autodesk FLI/FLC Animation"
    << "qtrle: Quicktime Animation"
    << "qtrpza: Quicktime Apple Video"
    << "qtsmc: Apple Graphics"
    << "cyuv: Creative YUV"
    << "msrle: Microsoft RLE"
    << "roqvideo: Id RoQ File Video"
    << "lzo: LZO compressed"
    << "cram: Microsoft Video 1"
    << "cvidvfw: Cinepak Video"
    << "huffyuv: HuffYUV"
    << "svq1: Sorenson v1"
    << "video1: Microsoft Video 1"
    << "cvid: Cinepak Video"
    << "mphuffyuv: HuffYUV"
    << "mpmszh: AVImszh"
    << "mpzlib: AVIzlib"
    << "cvidxa: XAnim Radius Cinepak Video"
    << "ffhuffyuv: FFHuffYUV"
    << "ffsvq1: FFmpeg Sorenson Video v1"
    << "ffdivx: FFmpeg DivX ;-)"
    << "ffmp42: FFmpeg M$ MPEG-4 v2"
    << "ffmp41: FFmpeg M$ MPEG-4 v1"
    << "ffwmv1: FFmpeg M$ WMV1/WMV7"
    << "ffwmv2: FFmpeg M$ WMV2/WMV8"
    << "ffodivx: FFmpeg MPEG-4"
    << "xvid: Xvid"
    << "odivx: OpenDivX API"
    << "divx4: DivX4Linux API"
    << "divx4vfw: DivX4Windows-VFW"
    << "divxds: DivX ;-)"
    << "divx: DivX ;-)"
    << "mpeg4ds: Microsoft MPEG-4 v1/v2"
    << "mpeg4: Microsoft MPEG-4 v1/v2"
    << "wmv8: Windows Media Video 8"
    << "wmv7: Windows Media Video 7"
    << "wmv9dmo: Windows Media Video 9 DMO"
    << "wmvdmo: Windows Media Video DMO"
    << "ubmp4: UB Video MPEG 4"
    << "ffmjpeg: FFmpeg MJPEG"
    << "ijpg: Independent JPEG Group"
    << "m3jpeg: Morgan Motion JPEG"
    << "mjpeg: MainConcept Motion JPEG"
    << "avid: AVID Motion JPEG"
    << "LEAD: LEAD (M)Jpeg"
    << "imagepower: ImagePower MJPEG2000"
    << "m3jpeg2k: Morgan MJPEG2000"
    << "m3jpegds: Morgan MJPEG"
    << "pegasusm: Pegasus Motion JPEG"
    << "pegasusl: Pegasus Lossless JPEG"
    << "pegasusmwv: Pegasus Motion Wavelet 2000"
    << "vivo: Vivo H.263"
    << "u263: UB Video H.263/H.263+/H.263++"
    << "i263: I263"
    << "ffi263: FFmpeg I263"
    << "ffh263: FFmpeg H263+"
    << "h263xa: XAnim CCITT H.263"
    << "h261xa: XAnim CCITT H.261"
    << "m261: M261"
    << "indeo5ds: Intel Indeo 5"
    << "indeo5: Intel Indeo 5"
    << "indeo4: Intel Indeo 4.1"
    << "indeo3: Intel Indeo 3.1/3.2"
    << "indeo5xa: XAnim Intel Indeo 5"
    << "indeo4xa: XAnim Intel Indeo 4.1"
    << "indeo3xa: XAnim Intel Indeo 3.1/3.2"
    << "qdv: Sony Digital Video"
    << "ffdv: FFmpeg DV"
    << "libdv: Raw DV"
    << "mcdv: MainConcept DV"
    << "3ivXxa: XAnim 3ivX Delta 3.5 plugin"
    << "3ivX: 3ivX Delta 3.5"
    << "rv40: Linux RealPlayer 9 RV40"
    << "rv40win: Win32 RealPlayer 9 RV40"
    << "rv30: Linux RealPlayer 8 RV30"
    << "rv30win: Win32 RealPlayer 8 RV30"
    << "rv20: Linux RealPlayer 8 RV20"
    << "rv20win: Win32 RealPlayer 8 RV20"
    << "ffrv10: FFmpeg RV10"
    << "vp3: On2 OpenSource VP3"
    << "mwv1: Motion Wavelets"
    << "asv2: ASUS V2"
    << "asv1: ASUS V1"
    << "mss1: Windows Screen Video"
    << "ucod: UCOD-ClearVideo"
    << "vcr2: ATI VCR-2"
    << "CJPG: CJPG"
    << "tm20: TrueMotion 2.0"
    << "sp5x: SP5x - used by Aiptek MegaCam"
    << "qt3ivx: win32/quicktime 3IV1"
    << "qth263: win32/quicktime H.263"
    << "qtrlerpza: win32/quicktime RLE/RPZA"
    << "qtvp3: win32/quicktime VP3"
    << "qtzygo: win32/quicktime ZyGo"
    << "qtbhiv: win32/quicktime BeHereiVideo"
    << "qtcvid: win32/quicktime Cinepak"
    << "qtindeo: win32/quicktime Indeo"
    << "qtmjpeg: win32/quicktime MJPEG"
    << "qtmpeg4: win32/quicktime MPEG-4"
    << "qtsvq3: win32/quicktime SVQ3"
    << "qtsvq1: win32/quicktime SVQ1"
    << "zlib: AVIzlib"
    << "mszh: AVImszh"
    << "alaris: Alaris VideoGramPiX"
    << "vcr1: ATI VCR-1"
    << "pim1: Pinnacle Hardware MPEG1"
    << "qpeg: Q-Team QPEG"
    << "rricm: rricm"
    << "camtasia: TechSmith Camtasia Screen"
    << "rawrgb32: RAW RGB32"
    << "rawrgb24: RAW RGB24"
    << "rawrgb16: RAW RGB16"
    << "rawbgr32flip: RAW BGR32"
    << "rawbgr32: RAW BGR32"
    << "rawbgr24flip: RAW BGR24"
    << "rawbgr24: RAW BGR24"
    << "rawbgr16flip: RAW BGR15"
    << "rawbgr16: RAW BGR15"
    << "rawbgr15flip: RAW BGR15"
    << "rawbgr15: RAW BGR15"
    << "rawbgr8flip: RAW BGR8"
    << "rawbgr8: RAW BGR8"
    << "rawbgr1: RAW BGR1"
    << "rawyuy2: RAW YUY2"
    << "rawuyvy: RAW UYVY"
    << "rawyv12: RAW YV12"
    << "rawi420: RAW I420"
    << "rawyvu9: RAW YVU9"
    << "null: Null";
  m_demuxers
    << "rawaudio: Raw audio"
    << "rawvideo: Raw video"
    << "tv: TV card"
    << "mf: Image files"
    << "avi: AVI"
    << "y4m: YUV4MPEG2"
    << "asv: ASF"
    << "nsv: NullsoftVideo"
    << "nuv: NuppelVideo"
    << "real: Realmedia"
    << "smjpeg: SMJPEG"
    << "mkv: Matroska"
    << "realaudio: Realaudio"
    << "vqf: TwinVQ"
    << "mov: Quicktime/MP4"
    << "vivo: Vivo"
    << "fli: Autodesk FLIC"
    << "film: FILM/CPK"
    << "roq: RoQ"
    << "gif: GIF"
    << "ogg: Ogg"
    << "avs: Avisynth"
    << "pva: PVA"
    << "mpegts: MPEG-TS"
    << "lmlm4: LMLM4 MPEG4"
    << "mpegps: MPEG PS"
    << "mpegpes: MPEG PES"
    << "mpeges: MPEG ES"
    << "mpeggxf: MPEG ES in GXF"
    << "mpeg4es: MPEG4 ES"
    << "h264es: H.264 ES"
    << "rawdv: Raw DV"
    << "mpc: Musepack"
    << "audio: Audio"
    << "tivo: TiVo"
    << "rtp: LIVE555 RTP"
    << "aac: AAC";
  getLists();
}

KPlayerEngine::~KPlayerEngine()
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Destroying engine\n";
#endif
  enableScreenSaver();
  kill();
  if ( process() )
    delete process();
  if ( meta() )
  {
    QStringList groups (meta() -> groupList());
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << "Cache has " << groups.count() << " entries\n";
#endif
    if ( int (groups.count()) > configuration() -> cacheSizeLimit() )
    {
      QMap<QString,QString> map;
      int i = 0;
      for ( QStringList::Iterator it = groups.begin(); it != groups.end(); ++ it )
      {
        QDateTime dt (meta() -> group (*it).readEntry ("Date", QDateTime()));
        if ( ! dt.isNull() )
          map.insert (dt.toString (Qt::ISODate) + QString().sprintf ("-%04u", i ++), *it);
      }
      i = groups.count() - configuration() -> cacheSizeLimit();
      for ( QMap<QString,QString>::Iterator mapit = map.begin(); i && mapit != map.end(); ++ mapit )
      {
#ifdef DEBUG_KPLAYER_ENGINE
        kdDebugTime() << "Deleting entry for " << mapit.value() << " from " << mapit.key() << "\n";
#endif
        meta() -> deleteGroup (mapit.value());
        i --;
      }
    }
  }
  if ( settings() )
  {
    disconnect (settings() -> properties(), SIGNAL (updated()), this, SLOT (refreshProperties()));
    delete settings();
  }
  if ( configuration() )
  {
    disconnect (configuration(), SIGNAL (updated()), this, SLOT (refreshSettings()));
    configuration() -> commit();
    delete configuration();
  }
  if ( light() && config() )
    delete m_config;
  if ( store() )
    delete store();
  if ( meta() )
    delete meta();
}

void KPlayerEngine::initialize (KActionCollection* ac, QWidget* parent, KConfig* config)
{
  if ( ! engine() )
  {
    KPlayerProperties::initialize();
    m_engine = new KPlayerEngine (ac, parent, config);
  }
}

void KPlayerEngine::terminate (void)
{
  if ( engine() )
  {
    delete engine();
    m_engine = 0;
    KPlayerProperties::terminate();
  }
}

void KPlayerEngine::setupActions (void)
{
  if ( ! actionCollection() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::setupActions\n";
#endif

  KAction* action = new KAction (actionCollection());
  actionCollection() -> addAction ("file_properties", action);
  connect (action, SIGNAL (triggered()), SLOT (fileProperties()));
  action -> setText (i18n("&Properties..."));
  action -> setIcon (KIcon ("document-properties"));
  action -> setShortcut (Qt::AltModifier + Qt::Key_Return);
  action -> setStatusTip (i18n("Opens the File Properties dialog"));
  action -> setWhatsThis (i18n("Properties command opens the File Properties dialog that lets you choose many options specific to the currently loaded file. See the File properties micro-HOWTO for details."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("player_play", action);
  connect (action, SIGNAL (triggered()), SLOT (play()));
  action -> setText (i18n("&Play"));
  action -> setIcon (KIcon ("media-playback-start"));
  action -> setShortcut (Qt::Key_Return);
  action -> setStatusTip (i18n("Plays the currently loaded file"));
  action -> setWhatsThis (i18n("Play command starts playback of the current file. If the player has been paused, this command resumes playback. This command is available when a file is loaded."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("player_pause", action);
  connect (action, SIGNAL (triggered()), SLOT (pause()));
  action -> setText (i18n("Paus&e"));
  action -> setIcon (KIcon ("media-playback-pause"));
  action -> setShortcut (Qt::Key_Space);
  action -> setStatusTip (i18n("Pauses the player"));
  action -> setWhatsThis (i18n("Pause command pauses or resumes playback of the current file. This command is available when the player is not idle."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("player_stop", action);
  connect (action, SIGNAL (triggered()), SLOT (stop()));
  action -> setText (i18n("&Stop"));
  action -> setIcon (KIcon ("media-playback-stop"));
  action -> setShortcut (Qt::Key_Escape);
  action -> setStatusTip (i18n("Stops the player"));
  action -> setWhatsThis (i18n("Stop command stops playback of the current file. This command is available when the player is not idle."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("player_forward", action);
  connect (action, SIGNAL (triggered()), SLOT (forward()));
  action -> setText (i18n("&Forward"));
  action -> setShortcut (Qt::Key_Right);
  action -> setStatusTip (i18n("Moves playback forward"));
  action -> setWhatsThis (i18n("Forward command moves playback forward by one percent of the time length of the current file. This command is available when playing a file."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("player_fast_forward", action);
  connect (action, SIGNAL (triggered()), SLOT (fastForward()));
  action -> setText (i18n("F&ast Forward"));
  action -> setIcon (KIcon ("media-seek-forward"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Right);
  action -> setStatusTip (i18n("Moves playback forward fast"));
  action -> setWhatsThis (i18n("Fast Forward command moves playback forward by ten percent of the time length of the current file. This command is available when playing a file."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("player_backward", action);
  connect (action, SIGNAL (triggered()), SLOT (backward()));
  action -> setText (i18n("&Backward"));
  action -> setShortcut (Qt::Key_Left);
  action -> setStatusTip (i18n("Moves playback backward"));
  action -> setWhatsThis (i18n("Backward command moves playback backward by one percent of the time length of the current file. This command is available when playing a file."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("player_fast_backward", action);
  connect (action, SIGNAL (triggered()), SLOT (fastBackward()));
  action -> setText (i18n("Fast Back&ward"));
  action -> setIcon (KIcon ("media-seek-backward"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Left);
  action -> setStatusTip (i18n("Moves playback backward fast"));
  action -> setWhatsThis (i18n("Fast Backward command moves playback backward by ten percent of the time length of the current file. This command is available when playing a file."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("player_start", action);
  connect (action, SIGNAL (triggered()), SLOT (start()));
  action -> setText (i18n("Back &to Start"));
  action -> setIcon (KIcon ("media-skip-backward"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Home);
  action -> setStatusTip (i18n("Moves playback to the beginning"));
  action -> setWhatsThis (i18n("Back to Start command moves playback to the beginning of the current file. This command is available when playing a file."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("audio_volume_up", action);
  connect (action, SIGNAL (triggered()), SLOT (volumeIncrease()));
  action -> setText (i18n("&Increase Volume"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Up);
  action -> setStatusTip (i18n("Increases the volume"));
  action -> setWhatsThis (i18n("Increase Volume command increases the sound volume."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("audio_volume_down", action);
  connect (action, SIGNAL (triggered()), SLOT (volumeDecrease()));
  action -> setText (i18n("&Decrease Volume"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Down);
  action -> setStatusTip (i18n("Decreases the volume"));
  action -> setWhatsThis (i18n("Decrease Volume command decreases the sound volume."));

  KToggleAction* toggle = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("audio_mute", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (mute()));
  toggle -> setText (i18n("&Mute"));
  toggle -> setIcon (KIcon ("audio-volume-muted"));
  toggle -> setShortcut (Qt::ControlModifier + Qt::Key_Backslash);
  toggle -> setStatusTip (i18n("Turns the sound on/off"));
  toggle -> setWhatsThis (i18n("Mute command turns the sound on or off."));
  if ( configuration() -> mute() )
    toggle -> setChecked (true);

  toggle = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("view_maintain_aspect", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (maintainAspect()));
  toggle -> setText (i18n("Maintain &Aspect"));
  toggle -> setIcon (KIcon ("zoom-fit-best"));
  toggle -> setShortcut (Qt::ControlModifier + Qt::Key_A);
  toggle -> setStatusTip (i18n("Maintains the video aspect ratio"));
  toggle -> setWhatsThis (i18n("Maintain Aspect command toggles the option to maintain the video aspect ratio."));
  if ( settings() -> maintainAspect() )
    toggle -> setChecked (true);

  m_updating = true;
  KPlayerSliderAction* sa = new KPlayerSliderAction (actionCollection());
  actionCollection() -> addAction ("player_progress", sa);
  connect (sa -> slider(), SIGNAL (valueChanged (int)), SLOT (progressChanged (int)));
  sa -> setText (i18n("Progress"));
  sa -> slider() -> setup (0, 0, 0, configuration() -> showSliderMarks(), 0, 0, 0);
  sa -> setStatusTip (i18n("Shows player progress and allows seeking"));
  sa -> setWhatsThis (i18n("Progress slider shows playback progress and allows seeking."));
  connect (sa -> slider(), SIGNAL (sliderReleased()), kPlayerProcess(), SLOT (progressSliderReleased()));

  sa = new KPlayerSliderAction (actionCollection());
  actionCollection() -> addAction ("audio_volume", sa);
  connect (sa -> slider(), SIGNAL (valueChanged (int)), SLOT (volumeChanged (int)));
  sa -> setText (i18n("Volume"));
  sa -> setStatusTip (i18n("Changes volume level"));
  sa -> setWhatsThis (i18n("Volume slider shows the current sound volume level and allows you to change it."));

  KPlayerPopupSliderAction* psa = new KPlayerPopupSliderAction (actionCollection());
  actionCollection() -> addAction ("popup_volume", psa);
  connect (psa -> slider(), SIGNAL (valueChanged (int)), SLOT (volumeChanged (int)));
  psa -> setText (i18n("Volume"));
  psa -> setIcon (KIcon ("text-speak"));
  psa -> setShortcut (Qt::Key_F9);
  psa -> setStatusTip (i18n("Shows the volume popup slider"));
  psa -> setWhatsThis (i18n("Volume button displays a slider that shows the current sound volume level and allows you to change it."));

  if ( light() )
  {
    refreshSettings();
    enablePlayerActions();
    enableVideoActions();
    return;
  }

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("subtitles_load", action);
  connect (action, SIGNAL (triggered()), SLOT (fileOpenSubtitles()));
  action -> setText (i18n("&Load Subtitles..."));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_S);
  action -> setStatusTip (i18n("Opens a subtitle file"));
  action -> setWhatsThis (i18n("The 'Load Subtitles' command displays the standard Open File dialog and allows you to choose a subtitle file to use with the current file or URL. If you load subtitles when a video is playing, KPlayer will display the subtitles immediately. By default it will also remember the subtitles you choose in the current file's properties. This command is available for video files."));

  action = new KToggleAction (actionCollection());
  actionCollection() -> addAction ("view_full_screen", action);
  connect (action, SIGNAL (triggered()), SLOT (fullScreen()));
  action -> setText (i18n("&Full Screen"));
  action -> setIcon (KIcon ("view-fullscreen"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_F);
  action -> setStatusTip (i18n("Switches to full screen mode"));
  action -> setWhatsThis (i18n("The 'Full Screen' command switches between full screen video display and normal mode."));

  action = KStandardAction::zoomIn (this, SLOT (zoomIn()), actionCollection());
  actionCollection() -> addAction ("view_zoom_in", action);
  action -> setStatusTip (i18n("Enlarges the video"));
  action -> setWhatsThis (i18n("The 'Zoom In' command enlarges the video area by one half of the original video size of the current file."));

  action = KStandardAction::zoomOut (this, SLOT (zoomOut()), actionCollection());
  actionCollection() -> addAction ("view_zoom_out", action);
  action -> setStatusTip (i18n("Shrinks the video"));
  action -> setWhatsThis (i18n("The 'Zoom Out' command reduces the video area by one half of the original video size of the current file."));

  QActionGroup* action_group = new QActionGroup (this);
  action_group -> setExclusive (true);

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_zoom_1_2", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (zoom12()));
  toggle -> setText (i18n("Zoom to 5&0%"));
  // xgettext:no-c-format
  toggle -> setStatusTip (i18n("Scales video to 50% of its original size"));
  // xgettext:no-c-format
  toggle -> setWhatsThis (i18n("The 'Zoom to 50%' command resizes the video area to one half of the original video size of the current file."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_zoom_1_1", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (zoom11()));
  toggle -> setText (i18n("Zoom to &100%"));
  toggle -> setShortcut (Qt::ControlModifier + Qt::Key_Backspace);
  toggle -> setStatusTip (i18n("Scales video to its original size"));
  // xgettext:no-c-format
  toggle -> setWhatsThis (i18n("The 'Zoom to 100%' command resizes the video area to the original video size of the current file."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_zoom_3_2", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (zoom32()));
  toggle -> setText (i18n("Zoo&m to 150%"));
  // xgettext:no-c-format
  toggle -> setStatusTip (i18n("Scales video to 150% of its original size"));
  // xgettext:no-c-format
  toggle -> setWhatsThis (i18n("The 'Zoom to 150%' command resizes the video area to 150% of the original video size of the current file."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_zoom_2_1", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (zoom21()));
  toggle -> setText (i18n("Zoom to &200%"));
  // xgettext:no-c-format
  toggle -> setStatusTip (i18n("Scales video to 200% of its original size"));
  // xgettext:no-c-format
  toggle -> setWhatsThis (i18n("The 'Zoom to 200%' command resizes the video area to twice the original video size of the current file."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_zoom_5_2", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (zoom52()));
  toggle -> setText (i18n("Zoom &to 250%"));
  // xgettext:no-c-format
  toggle -> setStatusTip (i18n("Scales video to 250% of its original size"));
  // xgettext:no-c-format
  toggle -> setWhatsThis (i18n("The 'Zoom to 250%' command resizes the video area to 250% of the original video size of the current file."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_zoom_3_1", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (zoom31()));
  toggle -> setText (i18n("Zoom to &300%"));
  // xgettext:no-c-format
  toggle -> setStatusTip (i18n("Scales video to 300% of its original size"));
  // xgettext:no-c-format
  toggle -> setWhatsThis (i18n("The 'Zoom to 300%' command resizes the video area to three times the original video size of the current file."));

  action_group = new QActionGroup (this);
  action_group -> setExclusive (true);

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_original_aspect", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (maintainOriginalAspect()));
  toggle -> setText (i18n("Maintain Original &Aspect"));
  toggle -> setStatusTip (i18n("Maintains the original video aspect ratio"));
  toggle -> setWhatsThis (i18n("The 'Maintain Original Aspect' command toggles the option to maintain the original video aspect ratio of the current file."));
  if ( settings() -> maintainAspect() )
    toggle -> setChecked (true);

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_current_aspect", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (maintainCurrentAspect()));
  toggle -> setText (i18n("Maintain &Current Aspect"));
  toggle -> setStatusTip (i18n("Maintains the current video aspect ratio"));
  toggle -> setWhatsThis (i18n("The 'Maintain Current Aspect' command toggles the option to maintain the current video aspect ratio."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_aspect_4_3", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (aspect43()));
  toggle -> setText (i18n("Force &4-to-3 Aspect"));
  toggle -> setStatusTip (i18n("Sets a 4-to-3 video aspect ratio"));
  toggle -> setWhatsThis (i18n("The 'Force 4-to-3 Aspect' command toggles the option to maintain a four-to-three video aspect ratio."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("view_aspect_16_9", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (aspect169()));
  toggle -> setText (i18n("Force 1&6-to-9 Aspect"));
  toggle -> setStatusTip (i18n("Sets a 16-to-9 video aspect ratio"));
  toggle -> setWhatsThis (i18n("The 'Force 16-to-9 Aspect' command toggles the option to maintain a sixteen-to-nine video aspect ratio."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("audio_delay_up", action);
  connect (action, SIGNAL (triggered()), SLOT (audioDelayIncrease()));
  action -> setText (i18n("I&ncrease Delay"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_BracketLeft);
  action -> setStatusTip (i18n("Increases audio delay"));
  action -> setWhatsThis (i18n("The 'Increase Delay' command increases the delay of the sound relative to the video."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("audio_delay_down", action);
  connect (action, SIGNAL (triggered()), SLOT (audioDelayDecrease()));
  action -> setText (i18n("D&ecrease Delay"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_BracketRight);
  action -> setStatusTip (i18n("Decreases audio delay"));
  action -> setWhatsThis (i18n("The 'Decrease Delay' command decreases the delay of the sound relative to the video."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_contrast_down", action);
  connect (action, SIGNAL (triggered()), SLOT (contrastDecrease()));
  action -> setText (i18n("Decrease C&ontrast"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_1);
  action -> setStatusTip (i18n("Decreases video contrast"));
  action -> setWhatsThis (i18n("The 'Decrease Contrast' command decreases the video contrast."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_contrast_up", action);
  connect (action, SIGNAL (triggered()), SLOT (contrastIncrease()));
  action -> setText (i18n("Increase &Contrast"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_2);
  action -> setStatusTip (i18n("Increases video contrast"));
  action -> setWhatsThis (i18n("The 'Increase Contrast' command increases the video contrast."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_brightness_down", action);
  connect (action, SIGNAL (triggered()), SLOT (brightnessDecrease()));
  action -> setText (i18n("Decrease B&rightness"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_3);
  action -> setStatusTip (i18n("Decreases video brightness"));
  action -> setWhatsThis (i18n("The 'Decrease Brightness' command decreases the video brightness."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_brightness_up", action);
  connect (action, SIGNAL (triggered()), SLOT (brightnessIncrease()));
  action -> setText (i18n("Increase &Brightness"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_4);
  action -> setStatusTip (i18n("Increases video brightness"));
  action -> setWhatsThis (i18n("The 'Increase Brightness' command increases the video brightness."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_hue_down", action);
  connect (action, SIGNAL (triggered()), SLOT (hueDecrease()));
  action -> setText (i18n("Decrease H&ue"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_5);
  action -> setStatusTip (i18n("Decreases video hue"));
  action -> setWhatsThis (i18n("The 'Decrease Hue' command decreases the video hue."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_hue_up", action);
  connect (action, SIGNAL (triggered()), SLOT (hueIncrease()));
  action -> setText (i18n("Increase &Hue"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_6);
  action -> setStatusTip (i18n("Increases video hue"));
  action -> setWhatsThis (i18n("The 'Increase Hue' command increases the video hue."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_saturation_down", action);
  connect (action, SIGNAL (triggered()), SLOT (saturationDecrease()));
  action -> setText (i18n("Decrease S&aturation"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_7);
  action -> setStatusTip (i18n("Decreases video saturation"));
  action -> setWhatsThis (i18n("The 'Decrease Saturation' command decreases the video saturation."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("video_saturation_up", action);
  connect (action, SIGNAL (triggered()), SLOT (saturationIncrease()));
  action -> setText (i18n("Increase &Saturation"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_8);
  action -> setStatusTip (i18n("Increases video saturation"));
  action -> setWhatsThis (i18n("The 'Increase Saturation' command increases the video saturation."));

  sa = new KPlayerSliderAction (actionCollection());
  actionCollection() -> addAction ("video_contrast", sa);
  connect (sa -> slider(), SIGNAL (valueChanged (int)), SLOT (contrastChanged (int)));
  sa -> setText (i18n("Contrast"));
  sa -> setStatusTip (i18n("Changes contrast level"));
  sa -> setWhatsThis (i18n("The Contrast slider shows the current video contrast level and allows you to change it."));

  sa = new KPlayerSliderAction (actionCollection());
  actionCollection() -> addAction ("video_brightness", sa);
  connect (sa -> slider(), SIGNAL (valueChanged (int)), SLOT (brightnessChanged (int)));
  sa -> setText (i18n("Brightness"));
  sa -> setStatusTip (i18n("Changes brightness level"));
  sa -> setWhatsThis (i18n("The Brightness slider shows the current video brightness level and allows you to change it."));

  sa = new KPlayerSliderAction (actionCollection());
  actionCollection() -> addAction ("video_hue", sa);
  connect (sa -> slider(), SIGNAL (valueChanged (int)), SLOT (hueChanged (int)));
  sa -> setText (i18n("Hue"));
  sa -> setStatusTip (i18n("Changes hue level"));
  sa -> setWhatsThis (i18n("The Hue slider shows the current video hue level and allows you to change it."));

  sa = new KPlayerSliderAction (actionCollection());
  actionCollection() -> addAction ("video_saturation", sa);
  connect (sa -> slider(), SIGNAL (valueChanged (int)), SLOT (saturationChanged (int)));
  sa -> setText (i18n("Saturation"));
  sa -> setStatusTip (i18n("Changes saturation level"));
  sa -> setWhatsThis (i18n("The Saturation slider shows the current video saturation level and allows you to change it."));

  psa = new KPlayerPopupSliderAction (actionCollection());
  actionCollection() -> addAction ("popup_contrast", psa);
  connect (psa -> slider(), SIGNAL (valueChanged (int)), SLOT (contrastChanged (int)));
  psa -> setText (i18n("Contrast"));
  psa -> setIcon (KIcon ("contrast"));
  psa -> setShortcut (Qt::Key_F5);
  psa -> setStatusTip (i18n("Shows the contrast popup slider"));
  psa -> setWhatsThis (i18n("The Contrast button displays a slider that shows the current video contrast level and allows you to change it."));

  psa = new KPlayerPopupSliderAction (actionCollection());
  actionCollection() -> addAction ("popup_brightness", psa);
  connect (psa -> slider(), SIGNAL (valueChanged (int)), SLOT (brightnessChanged (int)));
  psa -> setText (i18n("Brightness"));
  psa -> setIcon (KIcon ("brightness"));
  psa -> setShortcut (Qt::Key_F6);
  psa -> setStatusTip (i18n("Shows the brightness popup slider"));
  psa -> setWhatsThis (i18n("The Brightness button displays a slider that shows the current video brightness level and allows you to change it."));

  psa = new KPlayerPopupSliderAction (actionCollection());
  actionCollection() -> addAction ("popup_hue", psa);
  connect (psa -> slider(), SIGNAL (valueChanged (int)), SLOT (hueChanged (int)));
  psa -> setText (i18n("Hue"));
  psa -> setIcon (KIcon ("hue"));
  psa -> setShortcut (Qt::Key_F7);
  psa -> setStatusTip (i18n("Shows the hue popup slider"));
  psa -> setWhatsThis (i18n("The Hue button displays a slider that shows the current video hue level and allows you to change it."));

  psa = new KPlayerPopupSliderAction (actionCollection());
  actionCollection() -> addAction ("popup_saturation", psa);
  connect (psa -> slider(), SIGNAL (valueChanged (int)), SLOT (saturationChanged (int)));
  psa -> setText (i18n("Saturation"));
  psa -> setIcon (KIcon ("saturation"));
  psa -> setShortcut (Qt::Key_F8);
  psa -> setStatusTip (i18n("Shows the saturation popup slider"));
  psa -> setWhatsThis (i18n("The Saturation button displays a slider that shows the current video saturation level and allows you to change it."));

  action_group = new QActionGroup (this);
  action_group -> setExclusive (true);

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("player_soft_frame_drop", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (softFrameDrop()));
  toggle -> setText (i18n("S&oft Frame Dropping"));
  toggle -> setStatusTip (i18n("Turns soft frame dropping on/off"));
  toggle -> setWhatsThis (i18n("The 'Soft Frame Dropping' command toggles the soft frame dropping option. If your system is too slow to play a file, MPlayer can drop some frames so playback does not slow down. The soft option drops frames less aggressively than the hard one, and should not cause playback problems. The Frame drop option can also be set on the Advanced page either globally in KPlayer Settings or for a particular file in the File Properties."));

  toggle = new KToggleAction (action_group);
  actionCollection() -> addAction ("player_hard_frame_drop", toggle);
  connect (toggle, SIGNAL (triggered()), SLOT (hardFrameDrop()));
  toggle -> setText (i18n("&Hard Frame Dropping"));
  toggle -> setStatusTip (i18n("Turns hard frame dropping on/off"));
  toggle -> setWhatsThis (i18n("The 'Hard Frame Dropping' command toggles the hard frame dropping option. If your system is too slow to play a file, MPlayer can drop some frames so playback does not slow down. The hard option drops frames more aggressively than the soft one, and may sometimes break decoding. The Frame drop option can also be set on the Advanced page either globally in KPlayer Settings or for a particular file in the File Properties."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("subtitles_move_down", action);
  connect (action, SIGNAL (triggered()), SLOT (subtitlesMoveDown()));
  action -> setText (i18n("Move &Down"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Comma);
  action -> setStatusTip (i18n("Moves subtitles down"));
  action -> setWhatsThis (i18n("The 'Move Down' command moves the subtitles down."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("subtitles_move_up", action);
  connect (action, SIGNAL (triggered()), SLOT (subtitlesMoveUp()));
  action -> setText (i18n("Move &Up"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Period);
  action -> setStatusTip (i18n("Moves subtitles up"));
  action -> setWhatsThis (i18n("The 'Move Up' command moves the subtitles up."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("subtitles_delay_decrease", action);
  connect (action, SIGNAL (triggered()), SLOT (subtitlesDelayDecrease()));
  action -> setText (i18n("D&ecrease Delay"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Apostrophe);
  action -> setStatusTip (i18n("Decreases subtitle delay"));
  action -> setWhatsThis (i18n("The 'Decrease Delay' command decreases the delay of the subtitles relative to the video."));

  action = new KAction (actionCollection());
  actionCollection() -> addAction ("subtitles_delay_increase", action);
  connect (action, SIGNAL (triggered()), SLOT (subtitlesDelayIncrease()));
  action -> setText (i18n("I&ncrease Delay"));
  action -> setShortcut (Qt::ControlModifier + Qt::Key_Semicolon);
  action -> setStatusTip (i18n("Increases subtitle delay"));
  action -> setWhatsThis (i18n("The 'Increase Delay' command increases the delay of the subtitles relative to the video."));

  m_video_action_list = new KPlayerTrackActionList (ki18n("Track %1"), ki18n("Selects %1 video track"),
    ki18n("The 'Video %1' command switches to the selected video track."), this, "video_streams");
  connect (videoActionList(), SIGNAL (activated (int)), SLOT (videoStream (int)));
  m_audio_action_list = new KPlayerTrackActionList (ki18n("Track %1"), ki18n("Selects %1 audio track"),
    ki18n("The 'Audio %1' command switches to the selected audio track."), this, "audio_streams");
  connect (audioActionList(), SIGNAL (activated (int)), SLOT (audioStream (int)));
  m_subtitle_action_list = new KPlayerSubtitleTrackActionList (ki18n("Track %1"), ki18n("Selects %1 subtitle track"),
    ki18n("The 'Subtitles %1' command switches to the selected subtitle track."), this, "subtitle_streams");
  connect (subtitleActionList(), SIGNAL (activated (int)), SLOT (subtitleStream (int)));

  refreshSettings();
  enablePlayerActions();
  enableVideoActions();
}

void KPlayerEngine::refreshSettings (void)
{
  if ( ! actionCollection() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::refreshSettings\n";
#endif
  m_updating = true;
  int value = settings() -> volume();
  int minimum = configuration() -> volumeMinimum();
  int maximum = configuration() -> volumeMaximum();
  int interval = configuration() -> sliderMarksInterval (maximum - minimum);
  bool show = configuration() -> showSliderMarks();
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "  Volume " << value << ": " << minimum << " - " << maximum << " / " << interval << "\n";
#endif
  sliderAction ("audio_volume") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  popupAction ("popup_volume") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  m_last_volume = settings() -> volume();
  if ( properties() -> audioDriverString().startsWith ("alsa") )
    getAlsaVolume();
  process() -> volume (settings() -> actualVolume());
  if ( light() )
  {
    m_updating = false;
    return;
  }
  value = settings() -> contrast();
  minimum = configuration() -> contrastMinimum();
  maximum = configuration() -> contrastMaximum();
  interval = configuration() -> sliderMarksInterval (maximum - minimum);
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "  Contrast " << value << ": " << minimum << " - " << maximum << " / " << interval << "\n";
#endif
  sliderAction ("video_contrast") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  popupAction ("popup_contrast") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  process() -> contrast (value);
  value = settings() -> brightness();
  minimum = configuration() -> brightnessMinimum();
  maximum = configuration() -> brightnessMaximum();
  interval = configuration() -> sliderMarksInterval (maximum - minimum);
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "  Brightness " << value << ": " << minimum << " - " << maximum << " / " << interval << "\n";
#endif
  sliderAction ("video_brightness") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  popupAction ("popup_brightness") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  process() -> brightness (value);
  value = settings() -> hue();
  minimum = configuration() -> hueMinimum();
  maximum = configuration() -> hueMaximum();
  interval = configuration() -> sliderMarksInterval (maximum - minimum);
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "  Hue " << value << ": " << minimum << " - " << maximum << " / " << interval << "\n";
#endif
  sliderAction ("video_hue") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  popupAction ("popup_hue") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  process() -> hue (value);
  value = settings() -> saturation();
  minimum = configuration() -> saturationMinimum();
  maximum = configuration() -> saturationMaximum();
  interval = configuration() -> sliderMarksInterval (maximum - minimum);
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "  Saturation " << value << ": " << minimum << " - " << maximum << " / " << interval << "\n";
#endif
  sliderAction ("video_saturation") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  popupAction ("popup_saturation") -> slider() -> setup (minimum, maximum, value, show, interval, interval, 1);
  process() -> saturation (value);
  if ( properties() -> hasLength() )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << "  Length " << properties() -> length() << "\n";
#endif
    setupProgressSlider (sliderAction ("player_progress") -> slider() -> maximum());
  }
  m_updating = false;
  value = settings() -> frameDrop();
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "  Frame drop " << value << "\n";
#endif
  toggleAction ("player_soft_frame_drop") -> setChecked (value == 1);
  toggleAction ("player_hard_frame_drop") -> setChecked (value == 2);
  process() -> frameDrop (value);
  if ( settings() -> setInitialDisplaySize() )
  {
    //emit initialSize();
    handleLayout();
    refreshAspect();
  }
}

void KPlayerEngine::refreshProperties (void)
{
  if ( ! actionCollection() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::refreshProperties\n";
#endif
  setVolume();
  process() -> audioDelay (settings() -> audioDelay(), true);
  setContrast();
  setBrightness();
  setHue();
  setSaturation();
  process() -> subtitleMove (settings() -> subtitlePosition(), true);
  process() -> subtitleDelay (settings() -> subtitleDelay(), true);
  int value = settings() -> frameDrop();
  if ( ! light() )
  {
    videoActionList() -> update (properties() -> videoIDs(), properties() -> videoID());
    audioActionList() -> update (properties() -> audioIDs(), properties() -> audioID());
    subtitleActionList() -> update (settings() -> showSubtitles(), properties() -> subtitleIDs(),
      properties() -> subtitleID(), properties() -> vobsubIDs(), properties() -> vobsubID(),
      settings() -> subtitles(), settings() -> vobsubSubtitles(), settings() -> currentSubtitlePath());
    toggleAction ("player_soft_frame_drop") -> setChecked (value == 1);
    toggleAction ("player_hard_frame_drop") -> setChecked (value == 2);
  }
  process() -> frameDrop (value);
  if ( settings() -> setInitialDisplaySize() )
  {
    //emit initialSize();
    handleLayout();
  }
  enableVideoActions();
  if ( ! light() )
    toggleAction ("view_full_screen") -> setChecked (settings() -> fullScreen()
      && toggleAction ("view_full_screen") -> isEnabled());
  showSubtitles();
  refreshAspect();
}

void KPlayerEngine::refreshAspect (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine: Refreshing aspect: " << settings() -> aspect().width() << "x" << settings() -> aspect().height()
    << " " << settings() -> maintainAspect() << "\n";
#endif
  toggleAction ("view_maintain_aspect") -> setChecked (settings() -> maintainAspect());
  if ( light() )
    return;
  toggleAction ("view_original_aspect") -> setChecked (false);
  toggleAction ("view_current_aspect") -> setChecked (false);
  toggleAction ("view_aspect_4_3") -> setChecked (false);
  toggleAction ("view_aspect_16_9") -> setChecked (false);
  if ( ! settings() -> maintainAspect() )
    return;
  if ( settings() -> isAspect (properties() -> originalSize()) )
    toggleAction ("view_original_aspect") -> setChecked (true);
  else if ( settings() -> isAspect (QSize (4, 3)) )
    toggleAction ("view_aspect_4_3") -> setChecked (true);
  else if ( settings() -> isAspect (QSize (16, 9)) )
    toggleAction ("view_aspect_16_9") -> setChecked (true);
  else
    toggleAction ("view_current_aspect") -> setChecked (true);
}

void KPlayerEngine::setupProgressSlider (int maximum)
{
  int interval = maximum * configuration() -> sliderMarks() / 100;
  int pagestep = properties() -> fastSeek() * m_progress_factor;
  if ( pagestep == 0 )
    pagestep = interval;
  int linestep = properties() -> normalSeek() * m_progress_factor;
  if ( linestep == 0 )
    linestep = 1;
  KPlayerSlider* slider = sliderAction ("player_progress") -> slider();
  slider -> setup (0, maximum, slider -> value(), configuration() -> showSliderMarks(), interval, pagestep, linestep);
}

void KPlayerEngine::playerStateChanged (KPlayerProcess::State state, KPlayerProcess::State previous)
{
  if ( ! actionCollection() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine received state change: " << previous << " => " << state << "\n";
#endif
  toggleAction ("player_pause") -> setChecked (state == KPlayerProcess::Paused);
  enablePlayerActions();
  enableVideoActions();
  if ( state == KPlayerProcess::Playing )
    disableScreenSaver();
  else
    enableScreenSaver();
  if ( state < KPlayerProcess::Playing && previous >= KPlayerProcess::Playing
      && ! properties() -> temporaryName().isEmpty() )
  {
    properties() -> setTemporaryName (QString::null);
    properties() -> commit();
  }
}

void KPlayerEngine::playerProgressChanged (float progress, KPlayerProcess::ProgressType type)
{
  if ( ! actionCollection() || type != KPlayerProcess::Position )
    return;
  KPlayerSlider* slider = sliderAction ("player_progress") -> slider();
  if ( slider -> isSliderDown() )
    return;
  m_updating = true;
  int maximum = slider -> maximum();
  if ( maximum )
  {
    int value = int (progress * m_progress_factor + 0.5);
    if ( value > maximum )
      setupProgressSlider (value);
    slider -> setValue (value);
  }
  m_updating = false;
}

void KPlayerEngine::playerInfoAvailable (void)
{
  if ( ! actionCollection() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine: Info available. Detected length: " << properties() -> length() << "\n";
#endif
  m_updating = true;
  // QRangeControl breaks if its range is more than 524287
  if ( properties() -> length() > 50000 )
    m_progress_factor = 1;
  else if ( properties() -> length() > 5000 )
    m_progress_factor = 10;
  else
    m_progress_factor = 100;
  setupProgressSlider (int (properties() -> length() * m_progress_factor + 0.5));
  if ( properties() -> hasLength() )
    playerProgressChanged (process() -> position(), KPlayerProcess::Position);
  m_updating = false;
  enablePlayerActions();
}

void KPlayerEngine::playerSizeAvailable (void)
{
  if ( ! actionCollection() )
    return;
  if ( ! properties() -> hasDisplaySize() && ! properties() -> hasOriginalSize() )
    properties() -> setHasVideo (false);
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine: Size Available. Video size " << properties() -> currentSize().width() << "x" << properties() -> currentSize().height() << "\n";
#endif
  if ( settings() -> setInitialDisplaySize() )
  {
    //emit initialSize();
    handleLayout();
  }
  enableVideoActions();
  if ( ! light() )
    toggleAction ("view_full_screen") -> setChecked (settings() -> fullScreen()
      && toggleAction ("view_full_screen") -> isEnabled());
  refreshAspect();
  if ( m_play_pending )
  {
    m_play_pending = false;
    if ( ! m_stop )
      startPlaying();
  }
}

void KPlayerEngine::enablePlayerActions (void)
{
  if ( ! actionCollection() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine: Enabling player actions\n";
#endif
  KPlayerProcess::State state = kPlayerProcess() -> state();
  action ("file_properties") -> setEnabled (! properties() -> url().isEmpty());
  action ("player_play") -> setEnabled (! properties() -> url().isEmpty()
    && state != KPlayerProcess::Running && state != KPlayerProcess::Playing);
  bool busy = state != KPlayerProcess::Idle;
  action ("player_pause") -> setEnabled (busy);
  action ("player_stop") -> setEnabled (busy);
  bool unpaused = state != KPlayerProcess::Paused;
  busy = busy && unpaused && state != KPlayerProcess::Running && kPlayerProcess() -> isSeekable();
  action ("player_forward") -> setEnabled (busy);
  action ("player_fast_forward") -> setEnabled (busy);
  action ("player_backward") -> setEnabled (busy);
  action ("player_fast_backward") -> setEnabled (busy);
  action ("player_start") -> setEnabled (busy);
  m_updating = true;
  busy = busy && properties() -> hasLength();
  if ( ! busy )
  {
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    if ( settings() -> shift() )
      modifiers |= Qt::ShiftModifier;
    if ( settings() -> control() )
      modifiers |= Qt::ControlModifier;
    QMouseEvent me1 (QEvent::MouseButtonRelease, QPoint (0, 0), QPoint (0, 0), Qt::LeftButton, Qt::NoButton, modifiers);
    QApplication::sendEvent (sliderAction ("player_progress") -> slider(), &me1);
    QMouseEvent me2 (QEvent::MouseButtonRelease, QPoint (0, 0), QPoint (0, 0), Qt::MidButton, Qt::NoButton, modifiers);
    QApplication::sendEvent (sliderAction ("player_progress") -> slider(), &me2);
  }
  sliderAction ("player_progress") -> slider() -> setEnabled (busy);
  m_updating = false;
  action ("audio_volume_up") -> setEnabled (unpaused);
  action ("audio_volume_down") -> setEnabled (unpaused);
  action ("audio_mute") -> setEnabled (unpaused);
  if ( ! light() )
  {
    action ("player_soft_frame_drop") -> setEnabled (unpaused);
    action ("player_hard_frame_drop") -> setEnabled (unpaused);
  }
  action ("popup_volume") -> setEnabled (unpaused);
  sliderAction ("audio_volume") -> slider() -> setEnabled (unpaused);
}

void KPlayerEngine::enableVideoActions (void)
{
  if ( ! actionCollection() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine: Enabling video actions\n";
#endif
  bool video = properties() -> hasVideo();
  action ("view_maintain_aspect") -> setEnabled (video);
  if ( light() )
    return;
  action ("view_full_screen") -> setEnabled (video || settings() -> fullScreen());
  action ("view_original_aspect") -> setEnabled (video);
  action ("view_current_aspect") -> setEnabled (video);
  action ("view_aspect_4_3") -> setEnabled (video);
  action ("view_aspect_16_9") -> setEnabled (video);
  bool unpaused = video && kPlayerProcess() -> state() != KPlayerProcess::Paused;
  action ("audio_delay_up") -> setEnabled (unpaused);
  action ("audio_delay_down") -> setEnabled (unpaused);
  action ("video_contrast_up") -> setEnabled (unpaused);
  action ("video_contrast_down") -> setEnabled (unpaused);
  action ("video_brightness_up") -> setEnabled (unpaused);
  action ("video_brightness_down") -> setEnabled (unpaused);
  action ("video_hue_up") -> setEnabled (unpaused);
  action ("video_hue_down") -> setEnabled (unpaused);
  action ("video_saturation_up") -> setEnabled (unpaused);
  action ("video_saturation_down") -> setEnabled (unpaused);
  action ("popup_contrast") -> setEnabled (unpaused);
  action ("popup_brightness") -> setEnabled (unpaused);
  action ("popup_hue") -> setEnabled (unpaused);
  action ("popup_saturation") -> setEnabled (unpaused);
  sliderAction ("video_contrast") -> slider() -> setEnabled (unpaused);
  sliderAction ("video_brightness") -> slider() -> setEnabled (unpaused);
  sliderAction ("video_hue") -> slider() -> setEnabled (unpaused);
  sliderAction ("video_saturation") -> slider() -> setEnabled (unpaused);
  enableZoomActions();
  enableSubtitleActions();
}

void KPlayerEngine::enableSubtitleActions (void)
{
  if ( ! actionCollection() || light() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine: Enabling subtitle actions\n";
#endif
  bool video = properties() -> hasVideo();
  bool subtitles = video && settings() -> showSubtitles();
  bool playing = subtitles && kPlayerProcess() -> state() == KPlayerProcess::Playing;
  action ("subtitles_load") -> setEnabled (video);
  //action ("subtitles_load_url") -> setEnabled (video);
  //action ("subtitles_unload") -> setEnabled (subtitles);
  action ("subtitles_move_down") -> setEnabled (playing);
  action ("subtitles_move_up") -> setEnabled (playing);
  action ("subtitles_delay_decrease") -> setEnabled (playing);
  action ("subtitles_delay_increase") -> setEnabled (playing);
}

void KPlayerEngine::enableZoomActions (void)
{
  if ( ! actionCollection() || light() )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine: Enabling zoom actions\n";
#endif
  toggleAction ("view_zoom_1_2") -> setChecked (settings() -> isZoomFactor (1, 2));
  toggleAction ("view_zoom_1_1") -> setChecked (settings() -> isZoomFactor (1));
  toggleAction ("view_zoom_3_2") -> setChecked (settings() -> isZoomFactor (3, 2));
  toggleAction ("view_zoom_2_1") -> setChecked (settings() -> isZoomFactor (2));
  toggleAction ("view_zoom_5_2") -> setChecked (settings() -> isZoomFactor (5, 2));
  toggleAction ("view_zoom_3_1") -> setChecked (settings() -> isZoomFactor (3));
  bool canZoom = ! settings() -> fullScreen() && properties() -> hasVideo();
#ifdef DEBUG_KPLAYER_ENGINE
  if ( settings() -> fullScreen() )
    kdDebugTime() << "Full screen, no zoom\n";
  if ( ! properties() -> hasVideo() )
    kdDebugTime() << "Video size empty, no zoom\n";
#endif
  toggleAction ("view_zoom_in") -> setEnabled (canZoom);
  toggleAction ("view_zoom_out") -> setEnabled (canZoom);
  toggleAction ("view_zoom_1_2") -> setEnabled (canZoom);
  toggleAction ("view_zoom_1_1") -> setEnabled (canZoom);
  toggleAction ("view_zoom_3_2") -> setEnabled (canZoom);
  toggleAction ("view_zoom_2_1") -> setEnabled (canZoom);
  toggleAction ("view_zoom_5_2") -> setEnabled (canZoom);
  toggleAction ("view_zoom_3_1") -> setEnabled (canZoom);
}

void KPlayerEngine::disableScreenSaver (void)
{
  if ( ! properties() -> hasVideo() )
    return;
  /*
  QByteArray data, reply;
  Q3CString type;
  if ( ! kapp -> dcopClient() -> call ("kdesktop", "KScreensaverIface", "isEnabled()", data, type, reply)
      || type != "bool" )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << "Could not get screen saver status\n";
#endif
    return;
  }
  QDataStream replyStream (reply, QIODevice::ReadOnly);
  bool enabled = false;
  replyStream >> enabled;
  if ( ! enabled )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << "Screen saver is not enabled\n";
#endif
    return;
  }
  QDataStream argStream (data, QIODevice::WriteOnly);
  argStream << false;
#ifdef DEBUG_KPLAYER_ENGINE
  if ( kapp -> dcopClient() -> send ("kdesktop", "KScreensaverIface", "enable(bool)", data) )
    kdDebugTime() << "Screensaver successfully disabled\n";
  else
    kWarning() << "Could not disable screensaver\n";
#else
  kapp -> dcopClient() -> send ("kdesktop", "KScreensaverIface", "enable(bool)", data);
#endif
  */
  m_enable_screen_saver = true;
}

void KPlayerEngine::enableScreenSaver (void)
{
  if ( ! m_enable_screen_saver )
    return;
  /*
  QByteArray data;
  QDataStream argStream (data, QIODevice::WriteOnly);
  argStream << true;
#ifdef DEBUG_KPLAYER_ENGINE
  if ( kapp -> dcopClient() -> send ("kdesktop", "KScreensaverIface", "enable(bool)", data) )
    kdDebugTime() << "Screensaver successfully enabled\n";
  else
    kWarning() << "Could not enable screensaver\n";
#else
  kapp -> dcopClient() -> send ("kdesktop", "KScreensaverIface", "enable(bool)", data);
#endif
  */
  m_enable_screen_saver = false;
}

bool isReadableFile (const QString& path)
{
  QFileInfo info (path);
  return info.exists() && info.isReadable() && ! info.isDir();
}

void KPlayerEngine::startPlaying (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::startPlaying\n";
#endif
  m_play_pending = m_stop = false;
  m_last_volume = settings() -> volume();
  if ( properties() -> audioDriverString().startsWith ("alsa") )
    getAlsaVolume();
  if ( settings() -> showSubtitles() )
  {
    if ( ! properties() -> originalSizeKnown() && process() -> gettingInfo() )
    {
      m_play_pending = true;
      return;
    }
    else if ( properties() -> needsExpanding() )
      autoexpand();
  }
  if ( properties() -> originalSizeKnown() )
    handleLayout();
  process() -> play();
}

void KPlayerEngine::load (KUrl url)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::load " << url.prettyUrl() << "\n";
#endif
  if ( ! actionCollection() || (url.path().isEmpty() && url.host().isEmpty()) )
    return;
  m_last_volume = settings() -> volume();
  if ( url == properties() -> url() )
  {
    if ( properties() -> audioDriverString().startsWith ("alsa") )
      getAlsaVolume();
    play();
    return;
  }
  kill();
  if ( settings() -> shift() )
    stop();
  else
    m_stop = false;
  m_play_pending = false;
  if ( settings() -> properties() )
    disconnect (settings() -> properties(), SIGNAL (updated()), this, SLOT (refreshProperties()));
  settings() -> load (url);
  process() -> load (url);
  connect (settings() -> properties(), SIGNAL (updated()), this, SLOT (refreshProperties()));
  playerProgressChanged (0, KPlayerProcess::Position);
  settings() -> clearSubtitles();
  if ( properties() -> subtitleAutoload() )
    autoloadSubtitles();
  if ( properties() -> hasSubtitleUrl() && isReadableFile (properties() -> subtitleUrlString()) )
    settings() -> addSubtitlePath (properties() -> subtitleUrlString());
  refreshProperties();
  if ( properties() -> originalSizeKnown() )
    playerSizeAvailable();
  if ( properties() -> hasLength() )
    playerInfoAvailable();
  if ( ! properties() -> hasLength() || ! properties() -> originalSizeKnown() )
    process() -> get_info();
  if ( ! m_stop )
    startPlaying();
#ifdef DEBUG_KPLAYER_ENGINE
  else
    kdDebugTime() << "Engine::load: shift pressed, not starting playback\n";
#endif
}

void KPlayerEngine::autoloadSubtitles (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Autoloading subtitles\n";
#endif
  if ( ! properties() -> url().isLocalFile() )
    return;
  QString urls (properties() -> subtitleUrlString());
  QStringList exts (configuration() -> subtitleExtensions());
  QString filename (properties() -> url().fileName());
  QString basename (filename.section ('.', 0, -2));
  QDir dir (properties() -> url().directory(), QString::null, QDir::Name | QDir::IgnoreCase, QDir::Files);
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << " File name " << filename << "\n";
  kdDebugTime() << " Base name " << basename << "\n";
  kdDebugTime() << " Directory " << dir.path() << "\n";
#endif
  QFileInfoList list (dir.entryInfoList());
  QFileInfoList::ConstIterator it (list.constBegin());
  while ( it != list.constEnd() )
  {
    const QFileInfo& info = *it;
    QString name (info.fileName());
    if ( name != filename && info.filePath() != urls && name.startsWith (basename, Qt::CaseInsensitive)
      && info.exists() && info.isReadable() && ! info.isDir() )
    {
      QStringList::ConstIterator extiterator (exts.constBegin());
      while ( extiterator != exts.constEnd() )
      {
        if ( name.endsWith (*extiterator, Qt::CaseInsensitive) )
        {
#ifdef DEBUG_KPLAYER_ENGINE
          kdDebugTime() << " Autoloaded " << info.filePath() << "\n";
#endif
          settings() -> addSubtitlePath (info.filePath());
          break;
        }
        ++ extiterator;
      }
    }
    ++ it;
  }
}

void KPlayerEngine::autoexpand (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::autoexpand\n";
#endif
  properties() -> autoexpand();
  if ( settings() -> setInitialDisplaySize() )
  {
    //emit initialSize();
    handleLayout();
    enableVideoActions();
    refreshAspect();
  }
}

void KPlayerEngine::showSubtitles (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::showSubtitles\n";
#endif
  if ( settings() -> hasSubtitles() )
  {
    if ( settings() -> showSubtitles() && properties() -> needsExpanding() )
    {
      autoexpand();
      process() -> restart();
    }
    else
      process() -> subtitles();
    enableSubtitleActions();
  }
}

bool KPlayerEngine::loadSubtitles (const KUrl::List& urls, bool checkExtensions)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::loadSubtitles\n";
#endif
  if ( urls.isEmpty() || properties() -> url().isEmpty() )
    return false;
  QStringList exts (configuration() -> subtitleExtensions());
  KUrl::List::ConstIterator urliterator (urls.begin());
  while ( urliterator != urls.end() )
  {
    const KUrl& url (*urliterator);
    if ( ! url.isLocalFile() )
      return false;
    QString path (url.path());
    if ( path.isEmpty() )
      return false;
    QFileInfo info (path);
    if ( ! info.exists() || ! info.isReadable() || info.isDir() )
      return false;
    if ( checkExtensions )
    {
      QStringList::ConstIterator extiterator (exts.constBegin());
      while ( extiterator != exts.constEnd() )
      {
        if ( path.endsWith (*extiterator, Qt::CaseInsensitive) )
          break;
        ++ extiterator;
      }
      if ( extiterator == exts.constEnd() )
        return false;
    }
    ++ urliterator;
  }
  urliterator = urls.begin();
  while ( urliterator != urls.end() )
  {
    settings() -> addSubtitlePath ((*urliterator).path());
    ++ urliterator;
  }
  const KUrl& url = urls.first();
  if ( url != properties() -> subtitleUrl() )
  {
    properties() -> showSubtitleUrl (url);
    properties() -> commit();
    showSubtitles();
  }
  return true;
}

void KPlayerEngine::fileOpenSubtitles (void)
{
  loadSubtitles (openSubtitles());
}

void KPlayerEngine::fileProperties (void)
{
  if ( properties() -> url().isValid() )
  {
    KPlayerPropertiesDialog* dialog = KPlayerPropertiesDialog::createDialog (properties());
    dialog -> setup (properties() -> url());
    dialog -> exec();
    delete dialog;
  }
}

void KPlayerEngine::fullScreen (void)
{
  settings() -> setFullScreen (toggleAction ("view_full_screen") -> isChecked());
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::fullScreen " << settings() -> fullScreen() << "\n";
#endif
  handleLayout();
}

void KPlayerEngine::normal (void)
{
  settings() -> setMaximized (false);
  settings() -> setFullScreen (false);
  toggleAction ("view_full_screen") -> setChecked (false);
}

void KPlayerEngine::zoomIn (void)
{
  if ( ! properties() -> hasOriginalSize() )
    return;
  normal();
  settings() -> setDisplaySize (settings() -> displaySize() + properties() -> currentSize() / 2);
  handleLayout (true);
}

void KPlayerEngine::zoomOut (void)
{
  if ( ! properties() -> hasOriginalSize() )
    return;
  normal();
  settings() -> setDisplaySize (settings() -> displaySize() - properties() -> currentSize() / 2);
  handleLayout (true);
}

void KPlayerEngine::zoomTo (int m, int d)
{
  if ( ! properties() -> hasOriginalSize() )
    return;
  normal();
  settings() -> setDisplaySize (properties() -> currentSize() * m / d);
  handleLayout (true);
}

void KPlayerEngine::zoom12 (void)
{
  zoomTo (1, 2);
}

void KPlayerEngine::zoom11 (void)
{
  zoomTo (1);
}

void KPlayerEngine::zoom32 (void)
{
  zoomTo (3, 2);
}

void KPlayerEngine::zoom21 (void)
{
  zoomTo (2);
}

void KPlayerEngine::zoom52 (void)
{
  zoomTo (5, 2);
}

void KPlayerEngine::zoom31 (void)
{
  zoomTo (3);
}

void KPlayerEngine::wheel (int delta, int state)
{
  if ( ! settings() -> maximized() && ! settings() -> fullScreen() && properties() -> hasOriginalSize() )
  {
    settings() -> setDisplaySize (settings() -> displaySize() + properties() -> currentSize() * delta / 1200);
    handleLayout (true);
  }
  else if ( (state & Qt::ControlModifier) == Qt::ControlModifier )
  {
    if ( delta >= 0 )
      fastForward();
    else
      fastBackward();
  }
  else
  {
    if ( delta >= 0 )
      forward();
    else
      backward();
  }
}

void KPlayerEngine::doubleClick (void)
{
  if ( stopped() || ! properties() -> hasVideo() || light() )
    return;
  settings() -> setFullScreen (! settings() -> fullScreen());
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::doubleClick " << settings() -> fullScreen() << "\n";
#endif
  handleLayout();
}

void KPlayerEngine::emitWindowStateChanged (uint wid)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::emitWindowStateChanged " << wid << "\n";
#endif
  emit windowStateChanged (wid);
}

void KPlayerEngine::maintainAspect (void)
{
  maintainAspect (toggleAction ("view_maintain_aspect") -> isChecked(), properties() -> currentAspect());
}

void KPlayerEngine::maintainOriginalAspect (void)
{
  maintainAspect (toggleAction ("view_original_aspect") -> isChecked(), properties() -> originalSize());
}

void KPlayerEngine::maintainCurrentAspect (void)
{
  maintainAspect (toggleAction ("view_current_aspect") -> isChecked(), settings() -> displaySize());
  if ( settings() -> maintainAspect() )
    toggleAction ("view_current_aspect") -> setChecked (true);
}

void KPlayerEngine::aspect43 (void)
{
  maintainAspect (toggleAction ("view_aspect_4_3") -> isChecked(), QSize (4, 3));
  if ( settings() -> maintainAspect() )
    toggleAction ("view_aspect_4_3") -> setChecked (true);
}

void KPlayerEngine::aspect169 (void)
{
  maintainAspect (toggleAction ("view_aspect_16_9") -> isChecked(), QSize (16, 9));
  if ( settings() -> maintainAspect() )
    toggleAction ("view_aspect_16_9") -> setChecked (true);
}

void KPlayerEngine::play (void)
{
  if ( process() -> state() == KPlayerProcess::Paused )
    process() -> pause();
  else if ( process() -> state() == KPlayerProcess::Idle )
  {
    if ( settings() -> shift() )
      kill();
    startPlaying();
  }
}

void KPlayerEngine::pause (void)
{
  if ( process() -> state() != KPlayerProcess::Idle )
    process() -> pause();
  else
  {
    KToggleAction* action = toggleAction ("player_pause");
    if ( action -> isChecked() )
      action -> setChecked (false);
  }
}

void KPlayerEngine::stop (void)
{
  m_stop = true;
  m_play_pending = false;
  process() -> stop();
  handleLayout();
}

void KPlayerEngine::kill (void)
{
  m_stop = true;
  m_play_pending = false;
  process() -> kill();
}

void KPlayerEngine::forward (void)
{
  process() -> relativeSeek (properties() -> normalSeek());
}

void KPlayerEngine::fastForward (void)
{
  process() -> relativeSeek (properties() -> fastSeek());
}

void KPlayerEngine::backward (void)
{
  process() -> relativeSeek (- properties() -> normalSeek());
}

void KPlayerEngine::fastBackward (void)
{
  process() -> relativeSeek (- properties() -> fastSeek());
}

void KPlayerEngine::start (void)
{
  process() -> absoluteSeek (0);
}

void KPlayerEngine::progressChanged (int progress)
{
  if ( m_updating || ! m_progress_factor )
    return;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Seek to " << progress << ": " << process() -> position() << " => " << ((progress + m_progress_factor / 2) / m_progress_factor) << " / " << properties() -> length() << "\n";
#endif
  process() -> absoluteSeek ((progress + m_progress_factor / 2) / m_progress_factor);
}

void KPlayerEngine::volumeChanged (int volume)
{
  if ( m_updating )
    return;
  settings() -> setVolume (volume);
  setVolume();
}

void KPlayerEngine::contrastChanged (int contrast)
{
  if ( m_updating )
    return;
  settings() -> setContrast (contrast);
  setContrast();
}

void KPlayerEngine::brightnessChanged (int brightness)
{
  if ( m_updating )
    return;
  settings() -> setBrightness (brightness);
  setBrightness();
}

void KPlayerEngine::hueChanged (int hue)
{
  if ( m_updating )
    return;
  settings() -> setHue (hue);
  setHue();
}

void KPlayerEngine::saturationChanged (int saturation)
{
  if ( m_updating )
    return;
  settings() -> setSaturation (saturation);
  setSaturation();
}

void KPlayerEngine::setVolume (void)
{
  m_updating = true;
  int volume = settings() -> volume();
  sliderAction ("audio_volume") -> slider() -> setValue (volume);
  popupAction ("popup_volume") -> slider() -> setValue (volume);
  process() -> volume (settings() -> actualVolume());
  m_updating = false;
}

void KPlayerEngine::setContrast (void)
{
  m_updating = true;
  int contrast = settings() -> contrast();
  process() -> contrast (contrast);
  if ( ! light() )
  {
    sliderAction ("video_contrast") -> slider() -> setValue (contrast);
    popupAction ("popup_contrast") -> slider() -> setValue (contrast);
  }
  m_updating = false;
}

void KPlayerEngine::setBrightness (void)
{
  m_updating = true;
  int brightness = settings() -> brightness();
  process() -> brightness (brightness);
  if ( ! light() )
  {
    sliderAction ("video_brightness") -> slider() -> setValue (brightness);
    popupAction ("popup_brightness") -> slider() -> setValue (brightness);
  }
  m_updating = false;
}

void KPlayerEngine::setHue (void)
{
  m_updating = true;
  int hue = settings() -> hue();
  process() -> hue (hue);
  if ( ! light() )
  {
    sliderAction ("video_hue") -> slider() -> setValue (hue);
    popupAction ("popup_hue") -> slider() -> setValue (hue);
  }
  m_updating = false;
}

void KPlayerEngine::setSaturation (void)
{
  m_updating = true;
  int saturation = settings() -> saturation();
  process() -> saturation (saturation);
  if ( ! light() )
  {
    sliderAction ("video_saturation") -> slider() -> setValue (saturation);
    popupAction ("popup_saturation") -> slider() -> setValue (saturation);
  }
  m_updating = false;
}

void KPlayerEngine::volumeIncrease (void)
{
  settings() -> setVolume (settings() -> volume() + configuration() -> volumeStep());
  setVolume();
}

void KPlayerEngine::volumeDecrease (void)
{
  settings() -> setVolume (settings() -> volume() - configuration() -> volumeStep());
  setVolume();
}

void KPlayerEngine::mute (void)
{
  configuration() -> setMute (toggleAction ("audio_mute") -> isChecked());
  process() -> volume (settings() -> actualVolume());
}

void KPlayerEngine::audioDelayIncrease (void)
{
  settings() -> setAudioDelay (settings() -> audioDelay() + configuration() -> audioDelayStep());
  process() -> audioDelay (configuration() -> audioDelayStep());
}

void KPlayerEngine::audioDelayDecrease (void)
{
  settings() -> setAudioDelay (settings() -> audioDelay() - configuration() -> audioDelayStep());
  process() -> audioDelay (- configuration() -> audioDelayStep());
}

void KPlayerEngine::audioStream (int index)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::audioStream\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  properties() -> setAudioIDOption (index + 1);
  m_last_volume = settings() -> volume();
  if ( properties() -> audioDriverString().startsWith ("alsa") )
    getAlsaVolume();
  process() -> audioID (properties() -> audioID());
}

void KPlayerEngine::videoStream (int index)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::videoStream\n";
  kdDebugTime() << " Index  " << index << "\n";
#endif
  properties() -> setVideoIDOption (index + 1);
  m_last_volume = settings() -> volume();
  if ( properties() -> audioDriverString().startsWith ("alsa") )
    getAlsaVolume();
  process() -> restart();
}

void KPlayerEngine::subtitleStream (int index)
{
  if ( index || settings() -> showSubtitles() )
  {
    int count = properties() -> subtitleIDs().count() + properties() -> vobsubIDs().count();
    if ( index > count )
      properties() -> setSubtitleUrl (settings() -> subtitles() [index - count - 1]);
    properties() -> setSubtitleOption (index);
    showSubtitles();
  }
}

void KPlayerEngine::softFrameDrop (void)
{
  int drop = toggleAction ("player_soft_frame_drop") -> isChecked() ? 1 : 0;
  settings() -> setFrameDrop (drop);
  process() -> frameDrop (drop);
}

void KPlayerEngine::hardFrameDrop (void)
{
  int drop = toggleAction ("player_hard_frame_drop") -> isChecked() ? 2 : 0;
  settings() -> setFrameDrop (drop);
  process() -> frameDrop (drop);
}

void KPlayerEngine::subtitlesMoveDown (void)
{
  settings() -> setSubtitlePosition (settings() -> subtitlePosition() + configuration() -> subtitlePositionStep());
  process() -> subtitleMove (configuration() -> subtitlePositionStep());
}

void KPlayerEngine::subtitlesMoveUp (void)
{
  settings() -> setSubtitlePosition (settings() -> subtitlePosition() - configuration() -> subtitlePositionStep());
  process() -> subtitleMove (- configuration() -> subtitlePositionStep());
}

void KPlayerEngine::subtitlesDelayDecrease (void)
{
  settings() -> setSubtitleDelay (settings() -> subtitleDelay() - configuration() -> subtitleDelayStep());
  process() -> subtitleDelay (- configuration() -> subtitleDelayStep());
}

void KPlayerEngine::subtitlesDelayIncrease (void)
{
  settings() -> setSubtitleDelay (settings() -> subtitleDelay() + configuration() -> subtitleDelayStep());
  process() -> subtitleDelay (configuration() -> subtitleDelayStep());
}

void KPlayerEngine::contrastIncrease (void)
{
  settings() -> setContrast (settings() -> contrast() + configuration() -> contrastStep());
  setContrast();
}

void KPlayerEngine::contrastDecrease (void)
{
  settings() -> setContrast (settings() -> contrast() - configuration() -> contrastStep());
  setContrast();
}

void KPlayerEngine::brightnessIncrease (void)
{
  settings() -> setBrightness (settings() -> brightness() + configuration() -> brightnessStep());
  setBrightness();
}

void KPlayerEngine::brightnessDecrease (void)
{
  settings() -> setBrightness (settings() -> brightness() - configuration() -> brightnessStep());
  setBrightness();
}

void KPlayerEngine::hueIncrease (void)
{
  settings() -> setHue (settings() -> hue() + configuration() -> hueStep());
  setHue();
}

void KPlayerEngine::hueDecrease (void)
{
  settings() -> setHue (settings() -> hue() - configuration() -> hueStep());
  setHue();
}

void KPlayerEngine::saturationIncrease (void)
{
  settings() -> setSaturation (settings() -> saturation() + configuration() -> saturationStep());
  setSaturation();
}

void KPlayerEngine::saturationDecrease (void)
{
  settings() -> setSaturation (settings() -> saturation() - configuration() -> saturationStep());
  setSaturation();
}

KUrl::List KPlayerEngine::openFiles (const QString& title, QWidget* parent)
{
  static QString filter = i18n("*|All files\n*.avi *.AVI|AVI files\n*.mpg *.mpeg *.MPG *.MPEG|MPEG files\n*.ogg *.OGG|OGG files\n*.mp3 *.MP3|MP3 files");
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
  QString dir = group.readPathEntry ("Open File Directory", QString());
//Saving dialog position did not work: dlg.pos() returns wrong position in Qt 3.1.1
//int x = group.readEntry ("Open File Left", 0);
//int y = group.readEntry ("Open File Top", 0);
  int width = group.readEntry ("Open File Width", 0);
  int height = group.readEntry ("Open File Height", 0);
#ifdef DEBUG_KPLAYER_ENGINE
//kdDebugTime() << "Geometry " << x << "x" << y << " " << width << "x" << height << "\n";
#endif
  KPlayerFileDialog dlg (dir, filter, parent);
  dlg.setOperationMode (KFileDialog::Opening);
  dlg.setMode (KFile::Files | KFile::ExistingOnly);
  dlg.setCaption (title);
  if ( width > 0 && height > 0 )
    dlg.resize (width, height);
    //dlg.setGeometry (x, y, width, height);
    //dlg.move (x, y);
#ifdef DEBUG_KPLAYER_ENGINE
//kdDebugTime() << "         " << dlg.x() << "x" << dlg.y() << " " << dlg.width() << "x" << dlg.height() << "\n";
#endif
  dlg.exec();
  group.writePathEntry ("Open File Directory", dlg.directory());
//group.writeEntry ("Open File Left", dlg.x());
//group.writeEntry ("Open File Top", dlg.y());
  group.writeEntry ("Open File Width", dlg.width());
  group.writeEntry ("Open File Height", dlg.height());
#ifdef DEBUG_KPLAYER_ENGINE
//kdDebugTime() << "Geometry " << coord.x() << "x" << coord.y() << " " << dlg.width() << "x" << dlg.height() << "\n";
#endif
  return dlg.selectedUrls();
}

KUrl::List KPlayerEngine::openUrl (const QString& title, QWidget* parent)
{
  KUrl::List list;
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
  QString dir = group.readEntry ("Open URL");
//int x = group.readEntry ("Open URL Left", 0);
//int y = group.readEntry ("Open URL Top", 0);
  int width = group.readEntry ("Open URL Width", 0);
  int height = group.readEntry ("Open URL Height", 0);
  KUrlRequesterDialog dlg (dir, parent);
  dlg.setCaption (title);
  if ( width > 0 && height > 0 )
    dlg.resize (width, height);
    //dlg.setGeometry (x, y, width, height);
  dlg.exec();
  KUrl url (dlg.selectedUrl());
  if ( ! url.isEmpty() && url.isValid() )
  {
    list.append (url);
    KRecentDocument::add (url);
  }
  if ( dlg.result() == QDialog::Accepted )
    group.writeEntry ("Open URL", url.isLocalFile() ? url.path() : url.url());
//group.writeEntry ("Open URL Left", dlg.x());
//group.writeEntry ("Open URL Top", dlg.y());
  group.writeEntry ("Open URL Width", dlg.width());
  group.writeEntry ("Open URL Height", dlg.height());
  return list;
}

KUrl::List KPlayerEngine::openSubtitles (QWidget* parent)
{
  static QString filter = i18n("*|All files\n*.aqt *.AQT *.ass *.ASS *.js *.JS *.jss *.JSS *.rt *.RT *.smi *.SMI *.srt *.SRT *.ssa *.SSA *.sub *.SUB *.txt *.TXT *.utf *.UTF *.idx *.IDX *.ifo *.IFO|All subtitle files\n*.aqt *.AQT|AQT files\n*.ass *.ASS|ASS files\n*.js *.JS|JS files\n*.jss *.JSS|JSS files\n*.rt *.RT|RT files\n*.smi *.SMI|SMI files\n*.srt *.SRT|SRT files\n*.ssa *.SSA|SSA files\n*.sub *.SUB|SUB files\n*.txt *.TXT|TXT files\n*.utf *.UTF *.utf8 *.UTF8 *.utf-8 *.UTF-8|UTF files\n*.idx *.IDX *.ifo *.IFO|VobSub files");
  KConfigGroup group (kPlayerConfig() -> group ("Dialog Options"));
  QString dir = group.readPathEntry ("Open Subtitle Directory", QString());
//int x = group.readEntry ("Open Subtitle Left", 0);
//int y = group.readEntry ("Open Subtitle Top", 0);
  int width = group.readEntry ("Open Subtitle Width", 0);
  int height = group.readEntry ("Open Subtitle Height", 0);
  KPlayerFileDialog dlg (dir, filter, parent);
  dlg.setOperationMode (KFileDialog::Opening);
  dlg.setMode (KFile::Files | KFile::ExistingOnly);
  dlg.setCaption (i18n("Load Subtitles"));
  if ( width > 0 && height > 0 )
    dlg.resize (width, height);
    //dlg.setGeometry (x, y, width, height);
  dlg.exec();
  group.writeEntry ("Open Subtitle Directory", dlg.directory());
//group.writeEntry ("Open Subtitle Left", dlg.x());
//group.writeEntry ("Open Subtitle Top", dlg.y());
  group.writeEntry ("Open Subtitle Width", dlg.width());
  group.writeEntry ("Open Subtitle Height", dlg.height());
  return dlg.selectedUrls();
}

void KPlayerEngine::getLists (QString path)
{
  if ( path.isEmpty() )
    path = properties() -> executablePath();
  if ( path == m_path )
    return;
  m_path = path;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::getLists " << path << "\n";
#endif
  m_audio_codecs_ready = m_audio_drivers_ready = m_video_codecs_ready = m_video_drivers_ready = m_demuxers_ready = false;
  KPlayerLineOutputProcess* player = new KPlayerLineOutputProcess;
  *player << path << "-identify" << "-ac" << "help" << "-ao" << "help"
    << "-vc" << "help" << "-vo" << "help" << "-demuxer" << "help";
  connect (player, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*)),
    SLOT (receivedOutput (KPlayerLineOutputProcess*, char*)));
  connect (player, SIGNAL (processFinished (KPlayerLineOutputProcess*)),
    SLOT (processFinished (KPlayerLineOutputProcess*)));
  player -> start();
}

void KPlayerEngine::receivedOutput (KPlayerLineOutputProcess*, char* str)
{
  if ( strcmp (str, "ID_VIDEO_OUTPUTS") == 0 )
  {
    m_video_drivers.clear();
    m_video_drivers_ready = true;
    m_audio_codecs_ready = m_audio_drivers_ready = m_video_codecs_ready = m_demuxers_ready = false;
  }
  else if ( strcmp (str, "ID_VIDEO_CODECS") == 0 )
  {
    m_video_codecs.clear();
    m_video_codecs_ready = true;
    m_audio_codecs_ready = m_audio_drivers_ready = m_video_drivers_ready = m_demuxers_ready = false;
  }
  else if ( strcmp (str, "ID_AUDIO_OUTPUTS") == 0 )
  {
    m_audio_drivers.clear();
    m_audio_drivers_ready = true;
    m_audio_codecs_ready = m_video_codecs_ready = m_video_drivers_ready = m_demuxers_ready = false;
  }
  else if ( strcmp (str, "ID_AUDIO_CODECS") == 0 )
  {
    m_audio_codecs.clear();
    m_audio_codecs_ready = true;
    m_audio_drivers_ready = m_video_codecs_ready = m_video_drivers_ready = m_demuxers_ready = false;
  }
  else if ( strcmp (str, "ID_DEMUXERS") == 0 )
  {
    m_demuxers.clear();
    m_demuxers_ready = true;
    m_audio_codecs_ready = m_audio_drivers_ready = m_video_codecs_ready = m_video_drivers_ready = false;
  }
  else if ( m_audio_codecs_ready && re_codec.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kDebug() << str << "\n";
#endif
    QString desc (re_codec.cap(2));
    desc.replace (re_multiple, " ");
    desc.replace (re_trailing, "");
    desc.replace (re_brackets, "");
    desc.replace (re_parentheses, "");
    desc.replace (re_audio, "");
    desc.replace (re_s, "");
    desc.replace (re_layer, "layer ");
    desc.replace (re_speech, "WMA 9 Speech");
    desc.replace (re_macintosh, "Macintosh Audio Comp. and Exp.");
    m_audio_codecs += re_codec.cap(1) + ": " + desc;
  }
  else if ( m_audio_drivers_ready && re_driver.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kDebug() << str << "\n";
#endif
    QString desc (re_driver.cap(2));
    desc.replace (re_multiple, " ");
    desc.replace (re_trailing, "");
    desc.replace (re_brackets, "");
    desc.replace (re_parentheses, "");
    desc.replace (re_audio, "");
    desc.replace (re_s, "");
    desc.replace (re_layer, "layer ");
    desc.replace (re_writer, "");
    m_audio_drivers += re_driver.cap(1) + ": " + desc;
  }
  else if ( m_video_codecs_ready && re_codec.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kDebug() << str << "\n";
#endif
    QString desc (re_codec.cap(2));
    desc.replace (re_multiple, " ");
    desc.replace (re_trailing, "");
    desc.replace (re_brackets, "");
    desc.replace (re_parentheses, "");
    desc.replace (re_video, "");
    desc.replace (re_s, "");
    desc.replace (re_layer, "layer ");
    desc.replace (re_amu, "AMU");
    m_video_codecs += re_codec.cap(1) + ": " + desc;
  }
  else if ( m_video_drivers_ready && re_driver.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kDebug() << str << "\n";
#endif
    QString desc (re_driver.cap(2));
    desc.replace (re_multiple, " ");
    desc.replace (re_trailing, "");
    desc.replace (re_xv, "/XVideo");
    desc.replace (re_vidix, "/VIDIX");
    desc.replace (re_xover, "X11 overlay");
    desc.replace (re_opengl, "OpenGL");
    desc.replace (re_matrox, "Matrox overlay in");
    desc.replace (re_matroxg, "");
    desc.replace (re_brackets, "");
    desc.replace (re_parentheses, "");
    desc.replace (re_video, "");
    desc.replace (re_s, "");
    desc.replace (re_layer, "layer ");
    desc.replace (re_dash, " ");
    m_video_drivers += re_driver.cap(1) + ": " + desc;
  }
  else if ( m_demuxers_ready && re_demuxer.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kDebug() << str << "\n";
#endif
    QString desc (re_demuxer.cap(2));
    desc.replace (re_parentheses, "");
    desc.replace (re_demux, "");
    desc.replace (re_sega, "");
    desc.replace (re_smjpeg, "SMJPEG");
    desc.replace (re_tv_card, "TV card");
    desc.replace (re_lmlm, "");
    if ( desc == "mf" )
      desc = "Image files";
    m_demuxers += re_demuxer.cap(1) + ": " + desc;
  }
}

void KPlayerEngine::processFinished (KPlayerLineOutputProcess* proc)
{
  delete proc;
  m_audio_codecs.sort();
  m_audio_drivers.sort();
  m_video_codecs.sort();
  m_video_drivers.sort();
  m_demuxers.sort();
  if ( m_audio_codecs_ready || m_audio_drivers_ready || m_video_codecs_ready || m_video_drivers_ready || m_demuxers_ready )
    emit updated();
}

void KPlayerEngine::maintainAspect (bool maintain, QSize aspect)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::maintainAspect " << maintain << " " << aspect.width() << "x" << aspect.height() << "\n";
#endif
  if ( aspect.isEmpty() )
    maintain = false;
  settings() -> setMaintainAspect (maintain, aspect);
  refreshAspect();
  handleLayout();
}

void KPlayerEngine::handleLayout (bool user_zoom, bool user_resize)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Engine::handleLayout " << user_zoom << ", " << user_resize << "\n";
#endif
  if ( ! light() )
  {
    KToggleAction* action = toggleAction ("view_full_screen");
    action -> setChecked (settings() -> fullScreen() && action -> isEnabled());
  }
  if ( zooming() || resizing() || settings() -> anyButton() )
    return;
  m_zooming = true;
  bool pending;
  emit syncronizeState (&pending);
  if ( pending )
    return;
  emit syncronizeControls();
  QSize size (settings() -> adjustDisplaySize (user_zoom, user_resize));
  emit updateLayout (size);
  size = settings() -> adjustDisplaySize (user_zoom, user_resize);
  if ( user_zoom || ! settings() -> constrainedSize() )
    emit zoom();
  emit finalizeLayout();
  m_zooming = false;
  bool fixed_size = settings() -> fullScreen() || settings() -> maximized() || light();
  if ( ! fixed_size && (user_zoom || user_resize) )
    configuration() -> setPreferredVideoWidth (settings() -> displaySize().width());
  workspace() -> setDisplaySize (fixed_size ? size : settings() -> displaySize());
  enableZoomActions();
}

void KPlayerEngine::handleResize (bool user)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::handleResize " << user << " " << zooming() << "\n";
#endif
  if ( zooming() )
    return;
  if ( resizing() || settings() -> anyButton() )
  {
    m_pending_resize = true;
    return;
  }
  m_pending_resize = m_dockwidget_resize = false;
  if ( ! settings() -> constrainedSize() )
  {
    m_zooming = true;
    emit correctSize();
    m_zooming = false;
  }
  handleLayout (false, user);
}

void KPlayerEngine::layoutTimerTick (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Layout timer tick\n";
#endif
  if ( -- m_timer_ticks == 0 )
  {
    m_layout_user_interaction = false;
    m_timer.stop();
    handleResize (false);
  }
}

void KPlayerEngine::workspaceResize (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Workspace resize event\n";
#endif
  handleResize (false);
}

void KPlayerEngine::userResize (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "User resize event\n";
#endif
  handleResize (! light());
}

void KPlayerEngine::dockWidgetMove (bool docked)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Dock widget move event\n";
#endif
  if ( ! docked && settings() -> anyButton() )
    m_layout_user_interaction = true;
}

void KPlayerEngine::dockWidgetResize (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Dock widget resize event\n";
#endif
  m_dockwidget_resize = true;
}

void KPlayerEngine::dockWidgetVisibility (bool)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Dock widget visibility event\n";
#endif
  if ( settings() -> anyButton() )
    m_layout_user_interaction = true;
  else if ( m_layout_user_interaction )
  {
    m_timer_ticks = 5;
    m_timer.start (0);
  }
  else if ( ! zooming() )
  {
    emit dockWidgetVisibilityChanged();
    handleLayout();
  }
}

void KPlayerEngine::setResizing (bool resizing)
{
  if ( m_resizing == resizing )
    return;
  m_resizing = resizing;
  if ( ! resizing && m_pending_resize )
    userResize();
}

void KPlayerEngine::setModifiers (Qt::KeyboardModifiers modifiers)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Keyboard modifiers " << modifiers << "\n";
#endif
  settings() -> setModifiers (modifiers);
}

void KPlayerEngine::setButtons (Qt::MouseButtons buttons)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Mouse buttons " << buttons << "\n";
#endif
  Qt::MouseButtons previous = settings() -> buttons();
  settings() -> setButtons (buttons);
  if ( previous == Qt::NoButton )
    m_pending_resize = m_dockwidget_resize = false;
  else if ( buttons == Qt::NoButton )
  {
    if ( m_layout_user_interaction )
    {
      m_timer_ticks = 5;
      m_timer.start (0);
    }
    else if ( m_pending_resize )
    {
      if ( m_dockwidget_resize )
        userResize();
      else
        workspaceResize();
    }
  }
}

void KPlayerEngine::clearStoreSections (const QString& section)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "Clearing store section " << section << "\n";
#endif
  KConfigGroup group (store() -> group (section));
  int children = group.readEntry ("Children", 0);
  for ( int i = 0; i < children; ++ i )
  {
    QString entry (group.readEntry ("Child" + QString::number (i)));
    if ( entry.indexOf ('/') < 0 )
    {
      KUrl url (section);
      url.addPath (entry);
      clearStoreSections (url.url());
    }
  }
  store() -> deleteGroup (section);
}

void KPlayerEngine::getAlsaVolume (void)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::getAlsaVolume\n";
  kdDebugTime() << " Volume " << m_last_volume << "\n";
  kdDebugTime() << " Mute   " << configuration() -> mute() << "\n";
#endif
  if ( m_amixer_running )
    return;
  m_amixer_volume = -1;
  runAmixer ("get");
}

void KPlayerEngine::runAmixer (const QString& command, const QString& parameter)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::runAmixer\n";
#endif
  m_amixer_found_control = false;
  m_amixer_volume_first = m_amixer_volume_second = -1;
  KPlayerLineOutputProcess* amixer = new KPlayerLineOutputProcess;
  *amixer << "amixer";
  QString mixer = properties() -> mixerDevice();
  if ( ! mixer.isEmpty() )
  {
    *amixer << "-D" << mixer;
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << " Device " << mixer << "\n";
#endif
  }
  mixer = properties() -> mixerChannel();
  if ( mixer.isEmpty() )
    mixer = "PCM";
  *amixer << command << mixer;
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << " Command " << command << "\n";
  kdDebugTime() << " Channel " << mixer << "\n";
#endif
  if ( ! parameter.isEmpty() )
  {
    *amixer << parameter;
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << " Volume " << parameter << "\n";
#endif
  }
  connect (amixer, SIGNAL (receivedStdoutLine (KPlayerLineOutputProcess*, char*)),
    SLOT (amixerOutput (KPlayerLineOutputProcess*, char*)));
  connect (amixer, SIGNAL (processFinished (KPlayerLineOutputProcess*)),
    SLOT (amixerFinished (KPlayerLineOutputProcess*)));
  amixer -> start();
  m_amixer_running = true;
}

void KPlayerEngine::amixerOutput (KPlayerLineOutputProcess*, char* str)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << " amixer: " << str << "\n";
#endif
  static QRegExp re_control ("^Simple mixer control '(.*)'");
  static QRegExp re_volume ("^ +[^:]+: Playback \\d+ \\[(\\d+)%\\]");
  if ( re_control.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << " Control " << re_control.cap(1) << "\n";
#endif
    QString mixer = properties() -> mixerChannel();
    if ( mixer.isEmpty() )
      mixer = "PCM";
    m_amixer_found_control = re_control.cap(1) == mixer;
  }
  else if ( m_amixer_found_control && re_volume.indexIn (str) >= 0 )
  {
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << " Volume " << re_volume.cap(1) << "\n";
#endif
    (m_amixer_volume_first < 0 ? m_amixer_volume_first : m_amixer_volume_second) = re_volume.cap(1).toInt();
  }
}

void KPlayerEngine::amixerFinished (KPlayerLineOutputProcess* proc)
{
#ifdef DEBUG_KPLAYER_ENGINE
  kdDebugTime() << "KPlayerEngine::amixerExited\n";
#endif
  delete proc;
  m_amixer_running = false;
  bool set_volume = m_amixer_volume < 0;
  if ( m_amixer_volume_second >= 0 )
    m_amixer_volume = (m_amixer_volume_first + m_amixer_volume_second) >> 1;
  else if ( m_amixer_volume_first >= 0 )
    m_amixer_volume = m_amixer_volume_first;
  if ( m_amixer_volume >= 0 )
  {
    if ( configuration() -> mute() )
    {
      if ( m_amixer_volume == 0 )
        m_last_volume = 0;
      else
      {
        configuration() -> setMute (false);
        toggleAction ("audio_mute") -> setChecked (false);
      }
    }
    int volume = m_amixer_volume + settings() -> actualVolume() - m_last_volume;
#ifdef DEBUG_KPLAYER_ENGINE
    kdDebugTime() << " Volume " << m_amixer_volume << "\n";
    kdDebugTime() << " Target " << volume << "\n";
#endif
    if ( volume != m_amixer_volume && set_volume )
    {
      QString parameter;
      if ( m_amixer_volume_second < 0 )
        parameter = QString::number (volume) + "%";
      else
      {
        volume = (volume - m_amixer_volume) >> 1;
        parameter = QString::number (m_amixer_volume_first + volume) + "%,"
          + QString::number (m_amixer_volume_first + volume) + "%";
      }
      runAmixer ("set", parameter);
    }
    else if ( volume != settings() -> volume() && ! configuration() -> mute() )
    {
      properties() -> adjustVolume (volume);
      m_updating = true;
      sliderAction ("audio_volume") -> slider() -> setValue (volume);
      popupAction ("popup_volume") -> slider() -> setValue (volume);
      m_updating = false;
    }
  }
}
