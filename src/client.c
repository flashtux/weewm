/*
 * Copyright (c) 2004 by FlashCode <flashcode@flashtux.org>
 * See README for Licence detail, AUTHORS for developers list.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/* client.c: clients management */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "weewm.h"
#include "client.h"
#include "weeconfig.h"
#include "screen.h"
#include "memorize.h"


t_client *head_client;          /* first client                       */
t_client *current_client;       /* current client                     */
int dock_pos_free;              /* next free position for dock applet */
int dock_size;                  /* size of dock                       */


/*
 * set_wm_state: set wm state
 */

void set_wm_state (const Window window, const Atom a, int state, long vis)
{
    long data[2];
  
    data[0] = (long) state;
    data[1] = vis;
  
    XChangeProperty (display, window, a, a, 32, PropModeReplace,
                     (unsigned char *) data, 2);
}

/*
 * send_xmessage: send an event to a window
 */

static int send_xmessage (Window window, Atom atom, long x)
{
    XEvent event;
    
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = atom;
    event.xclient.format = 32;
    event.xclient.data.l[0] = x;
    event.xclient.data.l[1] = CurrentTime;
    
    return XSendEvent (display, window, False, NoEventMask, &event);
}

/*
 * client_send_wm_delete: send delete request to a client
 */

void client_send_wm_delete (t_client *client)
{
    int i, n, found;
    Atom *protocols;
    
    found = 0;
    if (client)
    {
        if (XGetWMProtocols (display, client->window, &protocols, &n))
        {
            for (i = 0; i < n; i++)
                if (protocols[i] == atom_wm_delete)
                    found++;
            XFree (protocols);
        }
        if (found)
            send_xmessage (client->window, atom_wm_protos, atom_wm_delete);
        else
            XKillClient (display, client->window);
    }
}

/*
 * client_set_smart_position: set smart position for a client
 *                            (according to position like keypad, 0-9, 5=centered)
 */

void client_set_smart_position (t_client *client, char position)
{
    switch (position)
    {
        /* no position specified, we leave default position */
        case '0':
            break;
        /* bottom left */
        case '1':
            client->x = client->border;
            client->y = get_y_max (client->screen) - client->height -
                        (client->border * 2);
            break;
        /* bottom centered */
        case '2':
            client->x = (get_x_max (client->screen) - client->width -
                        client->border) / 2;
            client->y = get_y_max (client->screen) - client->height -
                        (client->border * 2);
            break;
        /* bottom right */
        case '3':
            client->x = get_x_max (client->screen) - client->width -
                        (client->border * 2);
            client->y = get_y_max (client->screen) - client->height -
                        (client->border * 2);
            break;
        /* left centered */
        case '4':
            client->x = client->border;
            client->y = (get_y_max (client->screen) - client->height -
                        client->border) / 2;
            break;
        /* centered on screen */
        case '5':
            client->x = (get_x_max (client->screen) - client->width -
                        client->border) / 2;
            client->y = (get_y_max (client->screen) - client->height -
                        client->border) / 2;
            break;
        /* right centered */
        case '6':
            client->x = get_x_max (client->screen) - client->width -
                        (client->border * 2);
            client->y = (get_y_max (client->screen) - client->height -
                        client->border) / 2;
            break;
        /* top left */
        case '7':
            client->x = client->border;
            client->y = client->border;
            break;
        /* top centered */
        case '8':
            client->x = (get_x_max (client->screen) - client->width -
                        client->border) / 2;
            client->y = client->border;
            break;
        /* top right */
        case '9':
            client->x = get_x_max (client->screen) - client->width -
                        (client->border * 2);
            client->y = client->border;
            break;
    }
}

/*
 * client_set_dock_position: set window position for dock
 */

void client_set_dock_position(t_client *client)
{
    int pos, max, size;
    
    if (config_dock_use)
    {
        if ( (config_dock_position == DOCK_TOP) || (config_dock_position == DOCK_BOTTOM) )
        {
            max = DisplayWidth (display, client->screen);
            size = client->width;
        }
        else
        {
            max = DisplayHeight (display, client->screen);
            size = client->height;
        }
        pos = 0;
        switch (config_dock_align)
        {
            case DOCK_ALIGN_TOP_LEFT:
                pos = client->dock_position;
                break;
            case DOCK_ALIGN_CENTER:
                pos = ( (max - dock_size) / 2) + client->dock_position;
                break;
            case DOCK_ALIGN_BOTTOM_RIGHT:
                pos = max - client->dock_position - size;
                break;
        }
        switch (config_dock_position)
        {
            case DOCK_TOP:
                client->x = pos;
                client->y = 0;
                break;
            case DOCK_LEFT:
                client->x = 0;
                client->y = pos;
                break;
            case DOCK_RIGHT:
                client->x = DisplayWidth (display, client->screen) -
                            config_dock_size;
                client->y = pos;
                break;
            case DOCK_BOTTOM:
                client->x = pos;
                client->y = DisplayHeight (display, client->screen) -
                            config_dock_size;
                break;
        }
        dock_pos_free += size;
    }
}

/*
 * client_get_dock_size: return dock size
 */

int client_get_dock_size()
{
    t_client *ptr_client;
    int size;
    
    size = 0;
    for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
    {
        if (ptr_client->dock_position != NO_DOCK)
        {
            if ( (config_dock_position == DOCK_TOP)
                || (config_dock_position == DOCK_BOTTOM) )
                size += ptr_client->width;
            else
                size += ptr_client->height;
        }
    }
    return size;
}

/*
 * client_recalculate_dock_positions: recalculate position of each window in the dock
 */

void client_recalculate_dock_positions()
{
    t_client *ptr_client;
    
    if (config_dock_use)
    {
        dock_pos_free = 0;
        dock_size = client_get_dock_size ();
        for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
        {
            if (ptr_client->dock_position != NO_DOCK)
            {
                ptr_client->dock_position = dock_pos_free;
                client_set_dock_position (ptr_client);
                client_move (ptr_client, 1);
            }
        }
    }
}

/*
 * client_init_position: init client position
 *                       (according to smart position or memorized position)
 */

void client_init_position (t_client *client)
{
    int x, y, xmax, ymax, num_clients;
    t_keymap *ptr_keymap;
    t_client *ptr_client;
    t_memo_win *ptr_memo_win;

    xmax = DisplayWidth (display, client->screen);
    ymax = DisplayHeight (display, client->screen);
    
    if (client->width < config_min_width)
        client->width = config_min_width;
    if (client->height < config_min_height)
        client->height = config_min_height;
    if (client->width > xmax)
        client->width = xmax;
    if (client->height > ymax)
        client->height = ymax;
    
    /* dock applet */
    if (client->dock_position != NO_DOCK)
    {
        client_set_dock_position (client);
        if (config_dock_align == DOCK_ALIGN_CENTER)
            client_recalculate_dock_positions ();
        return;
    }
    
    if (client->size->flags & (USSize))
    {
        client->width = client->size->width;
        client->height = client->size->height;
    }
    
    if (client->size->flags & (USPosition))
    {
        client->x = client->size->x;
        client->y = client->size->y;
        if ( (client->x < 0) || (client->y < 0)
            || (client->x > xmax) || (client->y > ymax) )
            client->x = client->y = client->border;
    }
    else
    {
        /* position by default */
        get_mouse_position(&x, &y, client->screen);
        client->x = (x / (float) xmax) * (xmax - client->border - client->width);
        client->y = (y / (float) ymax) * (ymax - client->border - client->height);
        
        /* look for memorized position and/or desktop */
        for (ptr_memo_win = memo_win; ptr_memo_win; ptr_memo_win = ptr_memo_win->next)
            if (strcmp (ptr_memo_win->application_name, client->application_name) == 0)
                break;
        
        if (ptr_memo_win)
        {
            if (ptr_memo_win->desktop != MEMO_NOT_SET)
                client->vdesk[client->screen] = ptr_memo_win->desktop;
            if (ptr_memo_win->x != MEMO_NOT_SET)
            {
                client->x = ptr_memo_win->x;
                client->y = ptr_memo_win->y;
                client->width = ptr_memo_win->width;
                client->height = ptr_memo_win->height;
                client_move (client, 1);
                return;
            }
        }
        
        /* look for smart position */
        for (ptr_keymap = keymap; ptr_keymap; ptr_keymap = ptr_keymap->next)
        {
            if ( (ptr_keymap->command_file != NULL) && (ptr_keymap->smart_position != NULL) )
            {
                if (strcasecmp (client->application_name, ptr_keymap->application_name) == 0)
                {
                    /* smart position found, count the number of clients opened with this name */
                    num_clients = 0;
                    for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
                    {
                        if ( (ptr_client != client) &&
                            (ptr_client->vdesk[client->screen] == client->vdesk[client->screen]) &&
                            (strcasecmp (ptr_client->application_name, client->application_name) == 0) )
                            num_clients++;
                    }
                    if (num_clients >= (int) strlen (ptr_keymap->smart_position))
                        return;
                    client_set_smart_position (client, ptr_keymap->smart_position[num_clients]);
                }
            }
        }
    }
}

/*
 * client_change_gravity: change gravity for client
 *                        (multiplier: 1 = gravity, -1 = ungravity)
 */

void client_change_gravity (t_client *client, int multiplier)
{
    int dx, dy;
    int gravity;
    
    dx = 0;
    dy = 0;
    gravity = (client->size->flags & PWinGravity) ?
              client->size->win_gravity : NorthWestGravity;
  
    switch (gravity)
    {
        case NorthWestGravity:
        case SouthWestGravity:
        case NorthEastGravity:
            dx = client->border;
        case NorthGravity:
            dy = client->border;
            break;
        /* case CenterGravity: dy = client->border; break; */
    }
    
    client->x += multiplier * dx;
    client->y += multiplier * dy;
}

/*
 * client_send_config: send config to client
 */

void client_send_config (t_client *client)
{
    XConfigureEvent config_event;
  
    config_event.type = ConfigureNotify;
    config_event.event = client->window;
    config_event.window = client->window;
    config_event.x = client->x;
    config_event.y = client->y;
    config_event.width = client->width;
    config_event.height = client->height;
    config_event.border_width = 0;
    config_event.above = None;
    config_event.override_redirect = 0;
  
    XSendEvent (display, client->window, False, StructureNotifyMask,
                (XEvent *)&config_event);
}

/*
 * client_reparent: reparent a client
 */

void client_reparent(t_client *client)
{
    XSetWindowAttributes p_attr;

    XSelectInput (display, client->window, EnterWindowMask | PropertyChangeMask);
  
    p_attr.override_redirect = True;
    p_attr.background_pixel = color_border0.pixel;
    p_attr.event_mask =
        SubstructureRedirectMask | SubstructureNotifyMask | FocusChangeMask |
        ButtonPressMask | ExposureMask | EnterWindowMask;
    
    if (client->dock_position == NO_DOCK)
        client->parent = XCreateWindow (display,
                                        RootWindow(display, client->screen),
                                        client->x-client->border,
                                        client->y-client->border,
                                        client->width+(client->border*2),
                                        client->height + (client->border*2)/* +15 */,
                                        0,
                                        DefaultDepth(display, client->screen),
                                        CopyFromParent,
                                        DefaultVisual(display, client->screen),
                                        CWOverrideRedirect | CWBackPixel | CWEventMask,
                                        &p_attr);
    else
        client->parent = XCreateWindow (display,
                                        RootWindow(display, client->screen),
                                        client->x-client->border,
                                        client->y-client->border,
                                        client->width+(client->border*2),
                                        client->height+(client->border*2),
                                        0,
                                        DefaultDepth(display, client->screen),
                                        CopyFromParent,
                                        DefaultVisual(display, client->screen),
                                        CWOverrideRedirect | CWBackPixel | CWEventMask,
                                        &p_attr);
    
    XAddToSaveSet (display, client->window);
    XSetWindowBorderWidth (display, client->window, 0);
    if (client->dock_position == NO_DOCK)
        XReparentWindow (display, client->window, client->parent,
                         client->border, client->border/* +15 */);
    else
        XReparentWindow (display, client->window, client->parent,
                         client->border, client->border);
    
    client_send_config (client);
}

/*
 * client_new: create a new client
 */

void client_new (Window window, int screen)
{
    t_client *client;
    XWindowAttributes attr;
    long dummy;
    XWMHints *hints;
    char *name;
    int format;
    unsigned long nitems, bytes_after;
    Atom atom_return;
    /*t_motif_wm_hints *motif_wm_hints;*/
    unsigned char *motif_wm_hints;
    
    client = (t_client *) malloc (sizeof (t_client));
    if (!client)
    {
        fprintf (stderr, "WeeWM warning: not enough memory for new client\n");
        return;
    }
    XGrabServer (display);
    hints = XGetWMHints (display, window);
    client->next = head_client;
    client->screen = screen;
    
    head_client = client;
    
    client->window = window;
    client->ignore_unmap = 0;
    
    client->size = XAllocSizeHints ();
    XGetWMNormalHints (display, client->window, client->size, &dummy);
    XFetchName (display, client->window, &name);
    XGetWindowAttributes (display, client->window, &attr);
    XGetWindowProperty (display, client->window, atom_motif_wm_hints, 0, 3,
                        False, atom_motif_wm_hints, &atom_return, &format,
                        &nitems, &bytes_after,
                        /*(unsigned char **)(&motif_wm_hints));*/
                        &motif_wm_hints);
    
    client->x = attr.x;
    client->y = attr.y;
    client->width = attr.width;
    client->height = attr.height;
    client->border = config_border;
    client->oldw = 0;
    client->oldh = 0;
    client->oldx = 0;
    client->oldy = 0;
    client->oldmx = 0;
    client->oldmy = 0;
    client->oldmw = 0;
    client->oldmh = 0;
    client->application_name = strdup (name);
    
    if ( ((t_motif_wm_hints *)motif_wm_hints)
        && (((t_motif_wm_hints *)(motif_wm_hints))->flags & 2) )
    {
        if (((t_motif_wm_hints *)motif_wm_hints)->decorations & 2)
            client->border = config_border;
        else
            client->border = 0;
    }
    /*if (strcasecmp(name, "XMMS") == 0)
        client->border = 0;*/
    
    client->vdesk[client->screen] = vdesk[screen];
    
    if ( (config_dock_use) && (hints) && (hints->flags & StateHint)
        && (hints->initial_state == WithdrawnState) )
    {
        client->dock_position = dock_pos_free;
        if ( (config_dock_position == DOCK_TOP) || (config_dock_position == DOCK_BOTTOM) )
            dock_size += client->width;
        else
            dock_size += client->height;
        client->border = 0;
    }
    else
        client->dock_position = NO_DOCK;
    
    if (attr.map_state == IsViewable)
        client->ignore_unmap++;
    else
    {
        client_init_position (client);
        if ( (hints) && (hints->flags & StateHint) )
            set_wm_state (client->window, atom_wm_state,
                          hints->initial_state, None);
    }
    
    if (name)
        XFree (name);
    if (hints)
        XFree (hints);
    if (motif_wm_hints)
        XFree (motif_wm_hints);
    
    client_change_gravity (client, 1);
    client_reparent (client);
    XMapWindow (display, client->window);
    XMapRaised (display, client->parent);
    set_wm_state (client->window, atom_wm_state, NormalState, None);
  
    XSync (display, False);
    XUngrabServer (display);
    
    if (client->dock_position == NO_DOCK)
        XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                      client->width-1, client->height-1);
    
    if (client->vdesk[client->screen] != vdesk[client->screen])
        switch_desktop (client->vdesk[screen], client->screen);
}

/*
 * client_draw_outline: draws a rectangle around a client's window
 */

void client_draw_outline (t_client *client)
{
    char buffer[32];
    int text_size;
    int width_inc, height_inc;
    
    width_inc = 1;
    height_inc = 1;

    XDrawRectangle (display, RootWindow(display, client->screen),
                    invert_gc[client->screen], client->x - client->border,
                    client->y - client->border, client->width + client->border,
                    client->height + client->border);

    if (client->size->flags & PResizeInc)
    {
        width_inc = client->size->width_inc;
        height_inc = client->size->height_inc;
    }
    
    snprintf (buffer, sizeof (buffer), "%dx%d+%d+%d",
              client->width/width_inc, client->height/height_inc,
              client->x, client->y);
    
    text_size = XTextWidth (font, buffer, strlen (buffer));
    XDrawString (display, RootWindow (display, client->screen),
                 invert_gc[client->screen],
                 client->x + client->width - text_size - config_space,
                 client->y + client->height - config_space,
                 buffer, strlen (buffer));
}

/*
 * client_hide: hide client window
 */

void client_hide (t_client *client)
{
    if (client)
    {
        client->ignore_unmap = 2;
        XUnmapWindow (display, client->parent);
        XUnmapWindow (display, client->window);
        set_wm_state (client->window, atom_wm_state, IconicState, None);
    }
}

/*
 * client_unhide: unhide client window
 */

void client_unhide (t_client *client, int raise)
{
    client->ignore_unmap = 0;
    XMapWindow (display, client->window);
    (raise) ? XMapRaised (display, client->parent) : XMapWindow(display, client->parent);
    set_wm_state (client->window, atom_wm_state, NormalState, None);
}

/*
 * client_recalculate_sweep: recalculate sweep for a client
 */

void client_recalculate_sweep (t_client *client, int x1, int y1, int x2, int y2)
{
    int basex, basey;
    
    client->width = abs (x1 - x2);
    client->height = abs (y1 - y2);
    
    if (client->size->flags & PResizeInc)
    {
        basex = (client->size->flags & PBaseSize) ?
                client->size->base_width :
                (client->size->flags & PMinSize) ? client->size->min_width : 0;
        basey = (client->size->flags & PBaseSize) ?
                client->size->base_height :
                (client->size->flags & PMinSize) ? client->size->min_height : 0;
        client->width -= (client->width - basex) % client->size->width_inc;
        client->height -= (client->height - basey) % client->size->height_inc;
    }
    
    if (client->size->flags & PMinSize)
    {
        if (client->width < client->size->min_width)
            client->width = client->size->min_width;
        if (client->height < client->size->min_height)
            client->height = client->size->min_height;
    }
    
    if (client->size->flags & PMaxSize)
    {
        if (client->width > client->size->max_width)
            client->width = client->size->max_width;
        if (client->height > client->size->max_height)
            client->height = client->size->max_height;
    }
    
    client->x = (x1 <= x2) ? x1 : x1 - client->width;
    client->y = (y1 <= y2) ? y1 : y1 - client->height;
}

/*
 * client_sweep: sweep a client
 */

void client_sweep (t_client *client)
{
    XEvent event;
    int old_cx, old_cy;
    
    old_cx = client->x;
    old_cy = client->y;
    
    if (XGrabPointer (display, RootWindow(display, client->screen), False,
                      ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                      GrabModeAsync, GrabModeAsync, None, cursor_resize, CurrentTime) != GrabSuccess)
        return;
    XGrabServer (display);
    
    client_draw_outline (client);
    
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width-1, client->height-1);
    for (;;)
    {
        XMaskEvent (display,
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                    &event);
        switch (event.type)
        {
            case MotionNotify:
                client_draw_outline (client);
                client_recalculate_sweep (client, old_cx, old_cy,
                                          event.xmotion.x, event.xmotion.y);
                client_draw_outline (client);
                break;
            case ButtonRelease:
                client_draw_outline (client);
                XUngrabServer (display);
                XUngrabPointer (display, CurrentTime);
                return;
        }
    }
}

/*
 * client_drag: drag the client window
 */

void client_drag (t_client *client)
{
    XEvent event;
    int x1, y1;
    int old_cx, old_cy;
    
    if (client)
    {
        if (client->dock_position != NO_DOCK)
            return;
        
        old_cx = client->x;
        old_cy = client->y;
        
        if (XGrabPointer (display, RootWindow (display, client->screen), False,
                          ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                          GrabModeAsync, GrabModeAsync, None,
                          cursor_move, CurrentTime) != GrabSuccess)
            return;
        get_mouse_position (&x1, &y1, client->screen);
        for (;;)
        {
            XMaskEvent (display,
                        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                        &event);
            switch (event.type)
            {
                case MotionNotify:
                    client->x = old_cx + (event.xmotion.x - x1);
                    client->y = old_cy + (event.xmotion.y - y1);
                    XMoveWindow (display, client->parent,
                                 client->x - client->border,
                                 client->y - client->border);
                    client_send_config (client);
                    break;
                case ButtonRelease:
                    XUngrabPointer (display, CurrentTime);
                    return;
                default:
                    break;
            }
        }
    }
}

/*
 * client_move: move the client window
 */

void client_move (t_client *client, int set)
{
    if (client)
    {
        if (!set)
            client_drag (client);
        XMoveWindow (display, client->parent,
                     client->x - client->border, client->y - client->border);
        client_send_config (client);
        XRaiseWindow (display, client->parent);
    }
}

/*
 * client_resize: resize the client window
 */

void client_resize (t_client *client, int set)
{
    if (client)
    {
        if (client->dock_position != NO_DOCK)
            return;
        if (!set)
            client_sweep (client);
        XMoveResizeWindow (display, client->parent, client->x,
                           client->y, client->width + (client->border * 2),
                           client->height + (client->border * 2));
        XMoveResizeWindow (display, client->window,
                           client->border, client->border,
                           client->width, client->height);
        client_send_config (client);
        XRaiseWindow (display, client->parent);
    }
}

/*
 * client_search: search for a client with a window pointer
 */

t_client *client_search (Window window)
{
    t_client *client;
  
    for (client = head_client; client; client = client->next)
        if ( (window == client->parent) || (window == client->window) )
            return client;
    return NULL;
}

/*
 * client_get_first_for_desktop: return the first client found for a given desktop
 */

t_client *client_get_first_for_desktop (int desktop, int screen)
{
    t_client *ptr_client;
  
    for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
        if ( (ptr_client->dock_position == NO_DOCK) && (ptr_client->vdesk[screen] == desktop) )
            return ptr_client;
    return NULL;
}

/*
 * client_remove: remove a client
 */

void client_remove (t_client *client, int from_cleanup)
{
    t_client *parent;
    int recalculate_dock;

    XGrabServer (display);
    XSetErrorHandler (ignore_xerror);
  
    if (!from_cleanup)
        set_wm_state (client->window, atom_wm_state, WithdrawnState, None);
  
    free (client->application_name);
    recalculate_dock = (client->dock_position != NO_DOCK);
    client_change_gravity (client, -1);
    XRemoveFromSaveSet (display, client->window);
    XReparentWindow (display, client->window,
                     RootWindow (display, client->screen),
                     client->x, client->y);
    XSetWindowBorderWidth (display, client->window, 1);
    XDestroyWindow (display, client->parent);
    
    if (head_client == client)
        head_client = client->next;
    else
    {
        for (parent = head_client; parent && parent->next; parent = parent->next)
            if (parent->next == client)
                parent->next = client->next;
    }
    
    if (client->size)
        XFree (client->size);
    current_client = NULL;
    free (client);
    
    if (recalculate_dock)
        client_recalculate_dock_positions();
    
    XSync (display, False);
    XSetErrorHandler (handle_xerror);
    XUngrabServer (display);
}
