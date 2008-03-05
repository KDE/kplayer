/***************************************************************************
                           kplayerpart.h
                           -------------
    begin                : Wed Aug 20 2003
    copyright            : (C) 2003-2008 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KPLAYERPART_H
#define KPLAYERPART_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kaction.h>
#include <kparts/browserextension.h>

#include "kplayerprocess.h"

class KPlayerPart;

#define PROGRESS_TOOLBAR    1
#define VOLUME_TOOLBAR      2

/** The KPlayer BrowserExtension class.
  *@author kiriuja
  */
/*class KPlayerBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT

public:
  KPlayerBrowserExtension (KPlayerPart*);
  virtual ~KPlayerBrowserExtension();
};*/

/** The KPlayer KPart class.
  *@author kiriuja
  */
class KPlayerPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

public:
  KPlayerPart (QWidget*, QObject*, const QVariantList&);
  virtual ~KPlayerPart();

  /** Returns the data for the About dialog. **/
  static KAboutData* createAboutData (void);

  virtual bool openURL (const KUrl&);

  /** Enables or disables player actions. */
//void enablePlayerActions (void);

  /** Enables a toolbar.
   * @param index the toolbar number
   */
//void enableToolbar (int index);
  /** Disables a toolbar.
   * @param index the toolbar number
   */
//void disableToolbar (int index);

  /** Returns a pointer to the toolbar with the given index. */
  //KToolBar* toolBar (int);

public slots:
  /** Stops playback, starts KPlayer and gives it the URL. */
  void launchKPlayer (void);

  /** Stops the KPlayerProcess and closes the KPlayerWidget. */
  //virtual bool closeURL (void);

  /** Toggles the progress toolbar. */
//void viewProgressToolbar (void);
  /** Toggles the volume toolbar. */
//void viewVolumeToolbar (void);

  /** Receives the stateChanged signal from KPlayerProcess. */
//void playerStateChanged (KPlayerProcess::State);
  /** Receives the infoAvailable signal from KPlayerProcess. */
//void playerInfoAvailable (void);
  /** Displays the right click popup menu at the given position. */
  void widgetContextMenu (const QPoint& global_position);

protected:
  /** Initializes the KActions of the KPart. */
  void initActions (void);
  /** Dummy openFile required because of KParts::ReadOnlyPart design flaw. */
  virtual bool openFile (void)
    { return false; }

  /** Saves options to the configuration file. */
//void saveOptions (void);
  /** Reads options from the configuration file. */
//void readOptions (void);

  /** The browser extension. */
  //KPlayerBrowserExtension* m_extension;

  QMenu* m_popup_menu;

  // Following should really be private
//QStringList m_toolbar_names;
//QStringList m_toolbar_actions;
//bool m_toolbar_visible [2];
};

#endif
