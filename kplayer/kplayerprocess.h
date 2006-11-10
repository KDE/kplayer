/***************************************************************************
                          kplayerprocess.h
                          ----------------
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

#ifndef KPLAYERPROCESS_H
#define KPLAYERPROCESS_H

#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kprocess.h>
#include <kurl.h>
#include <qfile.h>
#include <qobject.h>
#include <qptrlist.h>

class KPlayerTrackProperties;
class KPlayerSettings;
class KTempFile;

/**KProcess extension, handles line output.
  *@author kiriuja
  */
class KPlayerLineOutputProcess : public KProcess
{
  Q_OBJECT

public:
  /** Constructs the objects and sets the initial property values.
    */
  KPlayerLineOutputProcess (void);
  /** Frees up the allocated memory.
    */
  virtual ~KPlayerLineOutputProcess();

  /** Returns the merge setting that indicates whether stdout and stderr output are handled
      separately or merged. If they are merged, receivedStderrLine will not be emitted. */
  bool merge (void)
    { return m_merge; }
  /** Sets the merge option that specifies whether stdout and stderr output are handled
      separately or merged. If they are merged, receivedStderrLine will not be emitted. */
  void setMerge (bool merge)
    { m_merge = merge; }

protected slots:
  /** Handles the process stdout output. Emits the receivedStdoutLine signal.
    */
  void slotReceivedStdout (KProcess*, char*, int);
  /** Handles the process stderr output. Emits the receivedStderrLine signal.
    */
  void slotReceivedStderr (KProcess*, char*, int);
  /** Reimplemented from KProcess. Emits receivedStd*Line signals
      if the respective buffers are not empty. */
  virtual void processHasExited (int);

signals:
  /** Emitted when a CR and/or LF terminated line is received on stdout
      or the process finishes and the stdout buffer is not empty. */
  void receivedStdoutLine (KPlayerLineOutputProcess*, char*, int);
  /** Emitted when a CR and/or LF terminated line is received on stderr
      or the process finishes and the stderr buffer is not empty. */
  void receivedStderrLine (KPlayerLineOutputProcess*, char*, int);

protected:
  /** Handles the process output, splits it into lines and emits the appropriate signals.
   */
  void receivedOutput (KProcess* proc, char* str, int len, char* buf, int blen, int llen, bool bstdout);

  /** Current stdout line or part thereof.
    */
  char* m_stdout_buffer;
  /** Current stdout buffer length.
    */
  int m_stdout_buffer_length;
  /** Current stdout line length.
    */
  int m_stdout_line_length;
  /** Current stderr line or part thereof.
    */
  char* m_stderr_buffer;
  /** Current stderr buffer length.
    */
  int m_stderr_buffer_length;
  /** Current stderr line length.
    */
  int m_stderr_line_length;
  /** Specifies whether stdout and stderr output are handled separately or merged.
      If they are merged, all output will be reported via receivedStdoutLine, and
      receivedStderrLine will not be emitted. */
  bool m_merge;
};

/**Handles the MPlayer process invocation, input and output.
  *@author kiriuja
  */
class KPlayerProcess : public QObject
{
  Q_OBJECT

public: 
  /** The process states. */
  enum State
  {
    /** mplayer process is not running */
    Idle,
    /** mplayer process is running but has not started playing yet */
    Running,
    /** mplayer is playing the current file */
    Playing,
    /** mplayer is paused */
    Paused
  };

  /** The progress type values. */
  enum ProgressType
  {
    /** position progress */
    Position,
    /** cache fill progress */
    CacheFill,
    /** index generation progress */
    IndexGeneration,
    /** file transfer progress */
    FileTransfer
  };

  /** Constructs the objects and sets the initial property values. */
  KPlayerProcess (void);
  /** Frees up the allocated memory. */
  virtual ~KPlayerProcess();

  /** Starts temporary file transfer. */
  void load (KURL);
  /** Retrieves information on the current file. */
  void get_info (void);
  /** Starts playback from the beginning. */
  void play (void);

  /** Runs mplayer on the current file. */
  void start (void);
  /** Restarts playback of the current file at the current position. */
  void restart (void);

  /** Pauses playback. */
  void pause (void);
  /** Stops playback. */
  void stop (void);
  /** Detaches the mplayer process. */
  void kill (void);

  /** The current process state. */
  KPlayerProcess::State state (void) const
    { return m_state; }

  /** Move to the time position given. */
  void absoluteSeek (int);
  /** Move by the time difference given. */
  void relativeSeek (int);

  /** Current position in seconds. */
  float position (void) const
    { return m_position; }
  /** Current position as string. */
  QString positionString (void) const;

  /** Changes volume level. */
  void volume (int);
  /** Adjusts audio delay. */
  void audioDelay (float, bool = false);
  /** Sets audio ID. */
  void audioID (int id);

  /** Changes brightness level. */
  void brightness (int);
  /** Changes contrast level. */
  void contrast (int);
  /** Changes hue level. */
  void hue (int);
  /** Changes saturation level. */
  void saturation (int);

  /** Moves subtitles vertically. */
  void subtitleMove (int, bool = false);
  /** Adjusts subtitle delay. */
  void subtitleDelay (float, bool = false);
  /** Selects subtitle index. */
  void subtitleIndex (int index);
  /** Toggles subtitle visibility. */
  void subtitleVisibility (void);
  /** Loads subtitles if necessary and selects them. */
  void subtitles (void);

  /** Changes the frame dropping setting. */
  void frameDrop (int);

  /** Returns whether the file information is available. */
  bool isInfoAvailable (void)
    { return m_info_available; }
  /** Returns whether a 0.9x version of MPlayer was detected. */
  bool is09Version (void)
    { return m_09_version; }
  /** Returns true unless playing directly from KIOSlave. */
  bool isSeekable (void)
    { return m_seekable; }

public slots:
  void progressSliderReleased (void);

protected:
  /** Settings. */
  KPlayerSettings* settings (void) const;
  /** Properties. */
  KPlayerTrackProperties* properties (void) const;

  /** Sends the given command to the MPlayer process. */
  void sendPlayerCommand (QCString&);
  /** Sends the given command to the MPlayer helper process. */
  void sendHelperCommand (QCString&);
  /** Closes and unlinks the named pipe. */
  void removeDataFifo (void);

  /** The pointer to the mplayer process object. */
  KPlayerLineOutputProcess* m_player;
  /** The pointer to the mplayer process used to get file info. */
  KPlayerLineOutputProcess* m_helper;

  /** Current position. */
  float m_position;
  /** Current state. */
  KPlayerProcess::State m_state;
  /** Indicates whether the file information is available. */
  bool m_info_available;
  /** Indicates if a 0.9x version of MPlayer was detected. */
  bool m_09_version;

  /** Prepares and runs the given process. */
  bool run (KPlayerLineOutputProcess* player);
  /** Stops the given process. */
  void stop (KPlayerLineOutputProcess** player, bool* quit, bool send_quit = false);
  /** Sets the process state and emits the stateChanged signal. */
  void setState (KPlayerProcess::State);
  /** Starts temporary file transfer. */
  void transferTemporaryFile (void);

  /** Name of named pipe used to send data to MPlayer. */
  QCString m_fifo_name;
  /** Handle of named pipe used to send data to MPlayer. */
  int m_fifo_handle;
  /** Offset of data to send. */
  uint m_fifo_offset;
  /** Fifo notifier object. */
  QSocketNotifier* m_fifo_notifier;
  /** Fifo timer object. */
  QTimer* m_fifo_timer;

  /** Transfer job. */
  KIO::TransferJob* m_slave_job;
  /** Temporary file transfer job. */
  KIO::TransferJob* m_temp_job;
  /** Cached transfer data. */
  QPtrList<QByteArray> m_cache;
  /** Temporary file from KIOSlave. */
  KTempFile* m_temporary_file;

  /** Current subtitle position. */
  int m_subtitle_position;
  /** Current subtitle delay. */
  float m_subtitle_delay;
  /** Current subtitle index. */
  int m_subtitle_index;
  /** Current list of external subtitles. */
  QStringList m_subtitles;

  /** Current audio delay. */
  float m_audio_delay;
  /** Current audio ID. */
  int m_audio_id;

  // Following should really be private
  /** Cache size for transfer job data. */
  uint m_cache_size;
  /** Only cache the first chunk of data. */
  bool m_first_chunk;
  int m_helper_seek, m_helper_seek_count, m_absolute_seek, m_seek_count;
  float m_helper_position, m_max_position, m_seek_origin;
  bool m_pausing, m_paused, m_quit, m_kill, m_size_sent, m_sent, m_send_seek;
  bool m_seek, m_seekable, m_success, m_delayed_player, m_delayed_helper;
  bool m_send_volume, m_send_contrast, m_send_brightness, m_send_hue, m_send_saturation;
  bool m_send_frame_drop, m_send_audio_id, m_send_subtitle_load;
  bool m_subtitle_visibility, m_send_subtitle_visibility;
  float m_send_audio_delay, m_send_subtitle_delay;
  int m_send_subtitle_position, m_send_subtitle_index, m_sent_count;

protected slots:
  /** Receives notification when the mplayer process exits. */
  void playerProcessExited (KProcess*);
  /** Receives notification when mplayer sends something to stdout. */
  void receivedStdoutLine (KPlayerLineOutputProcess*, char*, int);
  /** Receives notification when the mplayer helper process sends something to stdout. */
  void receivedHelperLine (KPlayerLineOutputProcess*, char*, int);
  /** Receives notification when mplayer sends something to stderr. */
//void receivedStderrLine (KPlayerLineOutputProcess*, char*, int);
  /** Receives notification when the data has been written to the fifo. */
  void playerDataWritten (int);
  /** Transfers data from a KIOSlave to an MPlayer process. */
  void transferData (KIO::Job*, const QByteArray&);
  /** Transfers data from a KIOSlave to a temporary file. */
  void transferTempData (KIO::Job*, const QByteArray&);
  /** Emits file transfer progress signal. */
  void transferProgress (KIO::Job*, unsigned long);
  /** Emits message received signal. */
  void transferInfoMessage (KIO::Job*, const QString&);
  /** Processes the result of a slave transfer job. */
  void transferDone (KIO::Job*);
  /** Processes the result of a temporary file transfer job. */
  void transferTempDone (KIO::Job*);
  /** Receives the refresh signal from KPlayerSettings. Stops temporary file transfer if needed. */
  //void refreshSettings (void);
  /** Sends data to MPlayer through named pipe. */
  void sendFifoData (void);

signals:
  /** Emitted when KPlayer state has changed. */
  void stateChanged (KPlayerProcess::State, KPlayerProcess::State);
  /** Emitted when mplayer progress needs to be updated. */
  void progressChanged (float, KPlayerProcess::ProgressType);
  /** Emitted when mplayer process used to get file information has finished. */
  void infoAvailable (void);
  /** Emitted when the video size becomes known. */
  void sizeAvailable (void);
  /** Emitted when a message is received from the mplayer process. */
  void messageReceived (QString);
  /** Emitted when MPlayer fails to play a file. */
  void errorDetected (void);
};

#endif
