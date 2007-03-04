/***************************************************************************
                          x11.cpp
                          -------
    begin                : Wed Feb 26 2003
    copyright            : (C) 2003-2007 by kiriuja
    email                : http://kplayer.sourceforge.net/email.html
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <X11/Xlib.h>
#include <kdebug.h>

#ifdef DEBUG
#include <kdebug.h>
kdbgstream kdDebugTime (void);
#define DEBUG_KPLAYER_GRAB
#define DEBUG_KPLAYER_PROPERTY
//#define DEBUG_KPLAYER_X11
//#define DEBUG_KPLAYER_FOCUS
//#define DEBUG_KPLAYER_KEY
#define DEBUG_KPLAYER_RESIZE
//#define DEBUG_KPLAYER_CLIENT
#endif

#ifdef DEBUG_KPLAYER_CLIENT
extern Atom qt_xdnd_position;
extern Atom qt_xdnd_status;
#endif

/*bool KPlayerX11TestGrab (Display* display, int winid)
{
  if ( int status = XGrabPointer (display, winid, False,
        (uint)(ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask),
        GrabModeAsync, GrabModeAsync, None, None, CurrentTime) )
  {
    const char *s =
      status == GrabNotViewable ? "GrabNotViewable" :
      status == AlreadyGrabbed  ? "AlreadyGrabbed" :
      status == GrabFrozen      ? "GrabFrozen" :
      status == GrabInvalidTime ? "GrabInvalidTime" : "Unknown";
#ifdef DEBUG_KPLAYER_GRAB
    kdDebugTime() << "Grab failed, status: " << s << "\n";
#endif
    return false;
  }
#define DEBUG_KPLAYER_GRAB
  kdDebugTime() << "Grab succeeded\n";
#endif
  XUngrabPointer (display, CurrentTime);
  return true;
}*/

extern void KPlayerSetControlShiftState (bool control, bool shift);
extern void KPlayerWidgetResizeHandler (bool);
extern void KPlayerWidgetMapHandler (uint);
extern void KPlayerWidgetUnmapHandler (uint);
extern void KPlayerWindowStateChanged (uint);

typedef int (*QX11EventFilter) (XEvent*);

static QX11EventFilter PreviousX11EventFilter = 0;

#ifdef DEBUG_KPLAYER_RESIZE

const char* KPlayerX11EventTypeNames [LASTEvent] = {
  "EVENT0",
  "EVENT1",
  "KeyPress",
  "KeyRelease",
  "ButtonPress",
  "ButtonRelease",
  "MotionNotify",
  "EnterNotify",
  "LeaveNotify",
  "FocusIn",
  "FocusOut",
  "KeymapNotify",
  "Expose",
  "GraphicsExpose",
  "NoExpose",
  "VisibilityNotify",
  "CreateNotify",
  "DestroyNotify",
  "UnmapNotify",
  "MapNotify",
  "MapRequest",
  "ReparentNotify",
  "ConfigureNotify",
  "ConfigureRequest",
  "GravityNotify",
  "ResizeRequest",
  "CirculateNotify",
  "CirculateRequest",
  "PropertyNotify",
  "SelectionClear",
  "SelectionRequest",
  "SelectionNotify",
  "ColormapNotify",
  "ClientMessage",
  "MappingNotify"
};

#endif

#ifdef DEBUG_KPLAYER_RESIZE

const char* KPlayerX11EventModeNames [4] = {
  "NotifyNormal",
  "NotifyGrab",
  "NotifyUngrab",
  "NotifyWhileGrabbed"
};

const char* KPlayerX11EventDetailNames [8] = {
  "NotifyAncestor",
  "NotifyVirtual",
  "NotifyInferior",
  "NotifyNonlinear",
  "NotifyNonlinearVirtual",
  "NotifyPointer",
  "NotifyPointerRoot",
  "NotifyDetailNone"
};

#endif

int KPlayerX11EventFilter (XEvent* event)
{
#ifdef DEBUG_KPLAYER_X11
  XAnyEvent* anyev = (XAnyEvent*) event;
  kdDebugTime() << "X11 event " << KPlayerX11EventTypeNames [event -> type] << " " << anyev -> window
    << " " << anyev -> send_event << " " << anyev -> serial << "\n";
#endif
  if ( event -> type == FocusIn || event -> type == FocusOut )
  {
    XFocusChangeEvent* ev = (XFocusChangeEvent*) event;
#ifdef DEBUG_KPLAYER_FOCUS
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> window << " " 
      << ev -> send_event << " " << ev -> serial << " mode " << KPlayerX11EventModeNames [ev -> mode]
      << " detail " << KPlayerX11EventDetailNames [ev -> detail] << "\n";
#endif
    if ( event -> type == FocusIn && ev -> mode == NotifyUngrab
        || event -> type == FocusOut && ev -> mode == NotifyGrab && ev -> detail == NotifyAncestor )
    {
#ifdef DEBUG_KPLAYER_GRAB
      kdDebugTime() << "Calling KPlayerWidgetResizeHandler (" << (ev -> mode == NotifyGrab) << ")\n";
#endif
      KPlayerWidgetResizeHandler (ev -> mode == NotifyGrab);
    }
  }
  else if ( event -> type == KeyPress || event -> type == KeyRelease )
  {
    XKeyEvent* ev = (XKeyEvent*) event;
#ifdef DEBUG_KPLAYER_KEY
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> window << " " 
      << ev -> send_event << " " << ev -> serial << " root " << ev -> root << " subwindow "
      << ev -> subwindow << " " << ev -> x << "x" << ev -> y << " " << ev -> x_root << "x" << ev -> y_root
      << " keycode " << ev -> keycode << " state " << ev -> state << " same " << ev -> same_screen << "\n";
#endif
    KPlayerSetControlShiftState ((ev -> state & ControlMask) == ControlMask, (ev -> state & ShiftMask) == ShiftMask);
    if ( (ev -> state & (ControlMask | ShiftMask)) == (ControlMask | ShiftMask) )
      ev -> state &= ~ ShiftMask;
  }
#ifdef DEBUG_KPLAYER_CLIENT
  else if ( event -> type == ClientMessage && (event -> xclient.message_type == qt_xdnd_position
    || event -> xclient.message_type == qt_xdnd_status) )
  {
    XClientMessageEvent* ev = (XClientMessageEvent*) event;
    kdDebugTime() << "X11 ClientMessage " << ev -> message_type << " " << ev -> window << " " 
      << ev -> send_event << " " << ev -> serial << " format " << ev -> format << " window " << ev -> data.l[0]
      << " flags " << ev -> data.l[1] << " position " << ((ev -> data.l[2] & 0xffff0000) >> 16)
      << "x" << (ev -> data.l[2] & 0x0000ffff) << " size " << ((ev -> data.l[3] & 0xffff0000) >> 16)
      << "x" << (ev -> data.l[3] & 0x0000ffff) << " action " << ev -> data.l[4] << "\n";
  }
#endif
#ifdef DEBUG_KPLAYER_RESIZE
  else if ( event -> type == ConfigureNotify )
  {
    XConfigureEvent* ev = (XConfigureEvent*) event;
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> event
      << " " << ev -> window << " " << ev -> send_event << " " << ev -> serial
      << " " << ev -> x << "x" << ev -> y << " " << ev -> width << "x" << ev -> height
      << " border " << ev -> border_width << " above " << ev -> above << " override " << ev -> override_redirect << "\n";
  }
  else if ( event -> type == ConfigureRequest )
  {
    XConfigureRequestEvent* ev = (XConfigureRequestEvent*) event;
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> parent
      << " " << ev -> window << " " << ev -> send_event << " " << ev -> serial
      << " " << ev -> x << "x" << ev -> y << " " << ev -> width << "x" << ev -> height
      << " border " << ev -> border_width << " above " << ev -> above
      << " detail " << ev -> detail << " mask " << ev -> value_mask << "\n";
  }
  else if ( event -> type == ResizeRequest )
  {
    XResizeRequestEvent* ev = (XResizeRequestEvent*) event;
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type]
      << " " << ev -> window << " " << ev -> send_event << " " << ev -> serial
      << " " << ev -> width << "x" << ev -> height << "\n";
  }
  else if ( event -> type == ReparentNotify )
  {
    XReparentEvent* ev = (XReparentEvent*) event;
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> event
      << " " << ev -> window << " " << ev -> parent << " " << ev -> send_event << " " << ev -> serial
      << " " << ev -> x << "x" << ev -> y << " override " << ev -> override_redirect << "\n";
  }
  else if ( event -> type == MapNotify )
  {
    XMapEvent* ev = (XMapEvent*) event;
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> event << " " << ev -> window
      << " " << ev -> send_event << " " << ev -> serial << " override " << ev -> override_redirect << "\n";
  }
#endif
  else if ( event -> type == MapRequest )
  {
    XMapRequestEvent* ev = (XMapRequestEvent*) event;
#ifdef DEBUG_KPLAYER_RESIZE
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> parent
      << " " << ev -> window << " " << ev -> send_event << " " << ev -> serial << "\n";
#endif
    KPlayerWidgetMapHandler (ev -> window);
  }
  else if ( event -> type == UnmapNotify )
  {
    XUnmapEvent* ev = (XUnmapEvent*) event;
#ifdef DEBUG_KPLAYER_RESIZE
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> event << " " << ev -> window
      << " " << ev -> send_event << " " << ev -> serial << " from configure " << ev -> from_configure << "\n";
#endif
    KPlayerWidgetUnmapHandler (ev -> window);
  }
  else if ( event -> type == PropertyNotify )
  {
    XPropertyEvent* ev = (XPropertyEvent*) event;
#ifdef DEBUG_KPLAYER_PROPERTY
    kdDebugTime() << "X11 " << KPlayerX11EventTypeNames [event -> type] << " " << ev -> send_event
      << " " << ev -> window << " " << ev -> atom << " " << ev -> time << " " << ev -> state << "\n";
#endif
    char* propname = XGetAtomName (ev -> display, ev -> atom);
    if ( propname )
    {
      if ( strcmp (propname, "_NET_WM_STATE") == 0 )
        KPlayerWindowStateChanged (ev -> window);
#ifdef DEBUG_KPLAYER_PROPERTY
      kdDebugTime() << "X11 property name " << propname << "\n";
#endif
    }
#ifdef DEBUG_KPLAYER_PROPERTY
    if ( ev -> state == PropertyNewValue )
    {
      Atom type;
      int format;
      unsigned long items, bytes;
      unsigned char* data;
      if ( XGetWindowProperty (ev -> display, ev -> window, ev -> atom, 0, 32, false, AnyPropertyType,
        &type, &format, &items, &bytes, &data) == Success && type != None )
      {
        kdDebugTime() << "X11 property type " << type << " format " << format
          << " items " << items << " bytes " << bytes << "\n";
        if ( data )
        {
          if ( format == 8 )
            kdDebugTime() << "X11 property value " << data << "\n";
          else if ( format == 16 )
          {
            short* sdata = (short*) data;
            for ( unsigned long i = 0; i < items; ++ i )
              kdDebugTime() << "X11 property value " << sdata[i] << "\n";
          }
          else if ( format == 32 )
            if ( strcmp (propname, "_NET_SUPPORTED") == 0
              || strcmp (propname, "_NET_WM_WINDOW_TYPE") == 0
              || strcmp (propname, "_NET_WM_STATE") == 0
              || strcmp (propname, "_NET_WM_ALLOWED_ACTIONS") == 0 )
            {
              Atom* adata = (Atom*) data;
              for ( unsigned long i = 0; i < items; ++ i )
              {
                char* atomname = XGetAtomName (ev -> display, adata[i]);
                if ( atomname )
                {
                  kdDebugTime() << "X11 property name " << atomname << "\n";
                  XFree (atomname);
                }
                else
                  kdDebugTime() << "X11 property value " << adata[i] << "\n";
              }
            }
            else
            {
              long* ldata = (long*) data;
              for ( unsigned long i = 0; i < items; ++ i )
                kdDebugTime() << "X11 property value " << ldata[i] << "\n";
            }
          XFree (data);
        }
      }
    }
#endif
    if ( propname )
      XFree (propname);
  }
  if ( PreviousX11EventFilter )
    return PreviousX11EventFilter (event);
  return 0;
}

Display* qt_xdisplay (void);
extern Time qt_x_time;

void KPlayerX11SetInputFocus (uint id)
{
  XSetInputFocus (qt_xdisplay(), id, RevertToNone, qt_x_time);
  XFlush (qt_xdisplay());
}

void KPlayerX11MapWindow (uint id)
{
  XMapWindow (qt_xdisplay(), id);
  XFlush (qt_xdisplay());
}

void KPlayerX11UnmapWindow (uint id)
{
  XUnmapWindow (qt_xdisplay(), id);
  XFlush (qt_xdisplay());
}

void KPlayerX11ClearExposeWindow (uint id)
{
  XClearArea (qt_xdisplay(), id, 0, 0, 0, 0, True);
  XFlush (qt_xdisplay());
}

void KPlayerX11SendConfigureEvent (uint id, int w, int h)
{
#ifdef DEBUG_KPLAYER_RESIZE
  kdDebugTime() << "KPlayerX11SendConfigureEvent " << id << " " << w << "x" << h << "\n";
#endif
  XConfigureEvent event = { ConfigureNotify, 0, True, qt_xdisplay(), id, id, 0, 0, w, h, 0, None, False };
  XSendEvent (qt_xdisplay(), event.event, True, StructureNotifyMask, (XEvent*) &event);
  XFlush (qt_xdisplay());
}

void KPlayerX11SendConfigureEvent (uint id, int x, int y, int w, int h)
{
#ifdef DEBUG_KPLAYER_RESIZE
  kdDebugTime() << "KPlayerX11SendConfigureEvent " << id << " " << x << "x" << y << " " << w << "x" << h << "\n";
#endif
  XConfigureEvent event = { ConfigureNotify, 0, True, qt_xdisplay(), id, id, x, y, w, h, 0, None, False };
  XSendEvent (qt_xdisplay(), event.event, True, StructureNotifyMask, (XEvent*) &event);
  XFlush (qt_xdisplay());
}

void KPlayerX11DiscardConfigureEvents (uint id)
{
  XEvent event;
  while ( XCheckTypedWindowEvent (qt_xdisplay(), id, ConfigureNotify, &event) )
#ifdef DEBUG_KPLAYER_RESIZE
    kdDebugTime() << "Discarded ConfigureEvent " << event.xconfigure.event << " " << event.xconfigure.window
      << " " << event.xconfigure.send_event << " " << event.xconfigure.serial
      << " " << event.xconfigure.x << "x" << event.xconfigure.y
      << " " << event.xconfigure.width << "x" << event.xconfigure.height
      << " border " << event.xconfigure.border_width << " above " << event.xconfigure.above
      << " override " << event.xconfigure.override_redirect << "\n"
#endif
  ;
}

void KPlayerX11GetKeyboardMouseState (uint id)
{
#ifdef DEBUG_KPLAYER_KEY
  kdDebugTime() << "KPlayerX11GetKeyboardMouseState " << id << "\n";
#endif
  Window root, child;
  int root_x, root_y, win_x, win_y;
  uint state;
  if ( XQueryPointer (qt_xdisplay(), id, &root, &child, &root_x, &root_y, &win_x, &win_y, &state) )
  {
#ifdef DEBUG_KPLAYER_KEY
    kdDebugTime() << " root " << root << " " << root_x << "x" << root_y << " child " << child
      << " " << win_x << "x" << win_y << " state " << state << "\n";
#endif
    KPlayerSetControlShiftState ((state & ControlMask) == ControlMask, (state & ShiftMask) == ShiftMask);
  }
}

extern QX11EventFilter qt_set_x11_event_filter (QX11EventFilter);

static int KPlayerX11EventFilterCount = 0;

void KPlayerSetX11EventFilter (void)
{
  if ( ! KPlayerX11EventFilterCount ++ )
    PreviousX11EventFilter = qt_set_x11_event_filter (KPlayerX11EventFilter);
}

void KPlayerResetX11EventFilter (void)
{
  if ( -- KPlayerX11EventFilterCount )
    return;
  qt_set_x11_event_filter (PreviousX11EventFilter);
  PreviousX11EventFilter = 0;
}
