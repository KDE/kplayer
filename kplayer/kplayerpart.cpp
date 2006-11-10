/***************************************************************************
                           kplayerpart.cpp
                           ---------------
    begin                : Wed Aug 20 2003
    copyright            : (C) 2003-2004 by kiriuja
    email                : kplayer dash developer at en dash directo dot net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <kapplication.h>
#include <ktoolbar.h>
#include <kxmlguifactory.h>
#include <kparts/genericfactory.h>
#include <qpopupmenu.h>
#include <dcopclient.h>

#ifdef DEBUG
#define DEBUG_KPLAYER_KPART
#endif

#include "kplayerpart.h"
#include "kplayerpart.moc"
#include "kplayerengine.h"
#include "kplayersettings.h"
#include "kplayerwidget.h"

static const char* description = I18N_NOOP("KPlayerPart, an embeddable KDE media player based on MPlayer");

typedef KParts::GenericFactory<KPlayerPart> KPlayerPartFactory;
K_EXPORT_COMPONENT_FACTORY (libkplayerpart, KPlayerPartFactory)

/*KPlayerBrowserExtension::KPlayerBrowserExtension (KPlayerPart* parent)
  : KParts::BrowserExtension (parent, "KPlayerBrowserExtension")
{
}

KPlayerBrowserExtension::~KPlayerBrowserExtension()
{
}*/

KPlayerPart::KPlayerPart (QWidget* wparent, const char* wname, QObject* parent, const char* name, const QStringList&)
  : KParts::ReadOnlyPart (parent, name ? name : "kplayerpart")
{
/*m_toolbar_visible [0] = true;
  m_toolbar_visible [1] = false;
  m_toolbar_names << "progressToolBar" << "volumeToolBar";
  m_toolbar_actions << "settings_progress_toolbar" << "settings_volume_toolbar";*/
  KPlayerEngine::initialize (actionCollection(), wparent, wname);
//KPlayerEngine::engine() -> configuration() -> setResizeAutomatically (false);
  setInstance (KPlayerPartFactory::instance());
//connect (kPlayerProcess(), SIGNAL (stateChanged(KPlayerProcess::State)), this, SLOT (playerStateChanged(KPlayerProcess::State)));
//connect (kPlayerProcess(), SIGNAL (infoAvailable()), this, SLOT (playerInfoAvailable()));
  connect (kPlayerWorkspace(), SIGNAL (contextMenu(const QPoint&)), this, SLOT (widgetContextMenu(const QPoint&)));
  setWidget (kPlayerWorkspace());
  initActions();
  setXMLFile ("kplayerpartui.rc");
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "XML File: '" << xmlFile() << "'\n";
  //m_extension = new KPlayerBrowserExtension (this);
  kdDebugTime() << "KPlayerPart: creating popup menu\n";
#endif
  m_popup_menu = new QPopupMenu (wparent);
  action ("player_launch") -> plug (m_popup_menu);
  m_popup_menu -> insertSeparator();
  action ("player_play") -> plug (m_popup_menu);
  action ("player_pause") -> plug (m_popup_menu);
  action ("player_stop") -> plug (m_popup_menu);
  m_popup_menu -> insertSeparator();
  action ("view_maintain_aspect") -> plug (m_popup_menu);
  m_popup_menu -> insertSeparator();
  action ("file_properties") -> plug (m_popup_menu);
  //if ( KGlobalSettings::insertTearOffHandle() )
  //  m_popup_menu -> insertTearOffHandle();
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "KPlayerPart: created popup menu with " << m_popup_menu -> count() << "items\n";
#endif
/*readOptions();
  KToolBar* toolbar = toolBar (PROGRESS_TOOLBAR);
  if ( toolbar )
  {
    toolbar -> setCaption (i18n("Progress"));
    sliderAction (m_toolbar_actions [0]) -> slider() -> setOrientation (toolbar -> orientation());
  }
  toolbar = toolBar (VOLUME_TOOLBAR);
  if ( toolbar )
  {
    toolbar -> setCaption (i18n("Volume"));
    sliderAction (m_toolbar_actions [1]) -> slider() -> setOrientation (toolbar -> orientation());
  }
  enablePlayerActions();
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "Constructor OK\n";
#endif*/
}

KPlayerPart::~KPlayerPart()
{
//closeURL();
//if ( m_popup_menu )
//  delete m_popup_menu;
  KPlayerEngine::terminate();
}

KAboutData* KPlayerPart::createAboutData (void)
{
  KAboutData* aboutData = new KAboutData ("kplayer", "KPlayerPart",
    VERSION, description, KAboutData::License_GPL,
    "(C) 2002-2004, kiriuja", 0, 0, "kplayer dash developer at en dash directo dot net");
  aboutData -> addAuthor ("kiriuja", 0, "kplayer dash developer at en dash directo dot net");
  return aboutData;
}

bool KPlayerPart::openURL (const KURL& url)
{
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "Open URL\n";
#endif
  emit setWindowCaption (url.prettyURL());
  kPlayerEngine() -> load (url);
  return true;
}

/*
bool KPlayerPart::closeURL (void)
{
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "Close URL\n";
#endif
//saveOptions();
  return true;
}
*/

//KToolBar* KPlayerPart::toolBar (int index)
//{
//index -= PROGRESS_TOOLBAR;
/*QObject* toolbar = sliderAction (m_toolbar_actions [index]) -> slider() -> parent();
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "Name: '" << toolbar -> name() << "'\n";
  if ( toolbar && toolbar -> inherits ("KToolBar") )
    kdDebugTime() << "Got parent\n";
  if ( toolbar )
    kdDebugTime() << "KPlayerSlider parent class name: " << toolbar -> className() << "\n";
#endif
  if ( toolbar && toolbar -> inherits ("KToolBar") )
    return (KToolBar*) toolbar;*/
#ifdef DEBUG_KPLAYER_KPART
  /*if ( qApp )
    kdDebugTime() << "App OK\n";
  if ( qApp && qApp -> mainWidget() )
    kdDebugTime() << "Main widget OK\n";*/
#endif
//if ( ! qApp || ! qApp -> mainWidget() )
//  return 0;
#ifdef DEBUG_KPLAYER_KPART
  /*if ( qApp -> mainWidget() -> child (m_toolbar_names [index]) )
    kdDebugTime() << "Child OK\n";
  if ( qApp -> mainWidget() -> child (m_toolbar_names [index], "KToolBar") )
    kdDebugTime() << "KToolBar OK\n";*/
#endif
//return (KToolBar*) qApp -> mainWidget() -> child (m_toolbar_names [index], "KToolBar");
//}

void KPlayerPart::initActions (void)
{
  KAction* action = new KAction (i18n("Start &KPlayer"), 0, 0, this, SLOT (launchKPlayer()), actionCollection(), "player_launch");
  action -> setStatusText (i18n("Stops playback and starts KPlayer with the current URL"));
  action -> setWhatsThis (i18n("Start KPlayer command stops playback, opens the full KPlayer, puts the multimedia file or URL on the playlist and starts playing it. It is always recommended that you choose this command, since it will give you better interface and more options when playing the multimedia."));
  kPlayerEngine() -> setActionCollection (actionCollection());
  kPlayerEngine() -> setupActions();
}

/*
void KPlayerPart::saveOptions (void)
{
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "Saving options\n";
#endif
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Part Options");
  config -> writeEntry (QString (m_toolbar_names [0]) + " Part Visible", m_toolbar_visible [0]);
  config -> writeEntry (QString (m_toolbar_names [1]) + " Part Visible", m_toolbar_visible [1]);
}

void KPlayerPart::readOptions (void)
{
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "Reading options\n";
#endif
  KPlayerSettings* settings = kPlayerSettings();
  KConfig* config = kPlayerConfig();
  config -> setGroup ("Part Options");
  m_toolbar_visible[0] = config -> readBoolEntry (QString (m_toolbar_names[0]) + " Part Visible", m_toolbar_visible[0]);
  toggleAction (m_toolbar_actions [0]) -> setChecked (m_toolbar_visible [0]);
#ifdef DEBUG_KPLAYER_KPART
  if ( toolBar (PROGRESS_TOOLBAR) )
    kdDebugTime() << "Toolbar OK\n";
#endif
  KToolBar* toolbar = toolBar (PROGRESS_TOOLBAR);
  if ( toolbar )
  {
    if ( m_toolbar_visible[0] )
      toolbar -> show();
    else
      toolbar -> hide();
  }
  m_toolbar_visible[1] = config -> readBoolEntry (QString (m_toolbar_names[1]) + " Part Visible", m_toolbar_visible[1]);
  toggleAction (m_toolbar_actions[1]) -> setChecked (m_toolbar_visible[1]);
  toolbar = toolBar (VOLUME_TOOLBAR);
  if ( toolbar )
  {
    if ( m_toolbar_visible [1] )
      toolbar -> show();
    else
      toolbar -> hide();
  }
}

void KPlayerPart::enableToolbar (int index)
{
  index -= PROGRESS_TOOLBAR;
  KToggleAction* action = toggleAction (m_toolbar_actions [index]);
  action -> setEnabled (true);
  if ( m_toolbar_visible [index] )
  {
    action -> setChecked (true);
    KToolBar* toolbar = toolBar (index + PROGRESS_TOOLBAR);
    if ( toolbar )
      toolbar -> show();
  }
}

void KPlayerPart::disableToolbar (int index)
{
  index -= PROGRESS_TOOLBAR;
  KToggleAction* action = toggleAction (m_toolbar_actions [index]);
  if ( action -> isChecked() )
  {
    action -> setChecked (false);
    KToolBar* toolbar = toolBar (index + PROGRESS_TOOLBAR);
    if ( toolbar )
      toolbar -> hide();
  }
  action -> setEnabled (false);
}

void KPlayerPart::enablePlayerActions (void)
{
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "Enable Toolbar\n";
#endif
  if ( kPlayerSettings() -> hasLength() )
    enableToolbar (PROGRESS_TOOLBAR);
  else
    disableToolbar (PROGRESS_TOOLBAR);
}
*/

void KPlayerPart::launchKPlayer (void)
{
  kPlayerEngine() -> stop();
  KProcess process;
  process << "kplayer" << KPlayerEngine::engine() -> properties() -> url().url();
  process.start (KProcess::DontCare);
  process.detach();
}

void KPlayerPart::widgetContextMenu (const QPoint& global_position)
{
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "KPlayerPart context menu\n";
#endif
  QPopupMenu* popup = 0;
  if ( factory() )
    popup = (QPopupMenu*) factory() -> container ("player_popup", this);
  if ( ! popup )
    popup = m_popup_menu;
  if ( popup )
  {
#ifdef DEBUG_KPLAYER_KPART
    kdDebugTime() << "KPlayerPart: displaying popup menu\n";
#endif
    popup -> popup (global_position);
  }
}

/*
void KPlayerPart::viewProgressToolbar (void)
{
  KToolBar* toolbar = toolBar (PROGRESS_TOOLBAR);
  if ( ! toolbar )
    return;
  bool visible = toggleAction ("settings_progress_toolbar") -> isChecked();
  m_toolbar_visible [0] = visible;
  if ( visible )
    toolbar -> show();
  else
    toolbar -> hide();
}

void KPlayerPart::viewVolumeToolbar (void)
{
  KToolBar* toolbar = toolBar (VOLUME_TOOLBAR);
  if ( ! toolbar )
    return;
  bool visible = toggleAction ("settings_volume_toolbar") -> isChecked();
  m_toolbar_visible [1] = visible;
  if ( visible )
    toolbar -> show();
  else
    toolbar -> hide();
}

void KPlayerPart::playerStateChanged (KPlayerProcess::State state)
{
#ifdef DEBUG_KPLAYER_KPART
  kdDebugTime() << "State change received: " << state << "\n";
#endif
//enablePlayerActions();
}

void KPlayerPart::playerInfoAvailable (void)
{
  enablePlayerActions();
}*/
