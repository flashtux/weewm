/*
 * Copyright (c) 2003 FlashCode <flashcode@flashtux.org>
 *                    Xahlexx <xahlexx@tuxfamily.org>
 *                    Bounga <bounga@altern.org>
 * See README for Licence detail.
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "weewm.h"
#include "client.h"
#include "screen.h"
#include "config.h"
#include "memorize.h"


int     dock_pos_free;              /* next free position for dock applet */
int     dock_size;                  /* size of dock                       */


/*
 * set_wm_state: set wm state
 */

void set_wm_state(const Window window, const Atom a, int state, long vis)
{
    long    data[2];
  
    data[0] = (long)state;
    data[1] = vis;
  
    XChangeProperty(display, window, a, a, 32, PropModeReplace, (unsigned char *)data, 2);
}

/*
 * send_xmessage: send an event to a window
 */

static int send_xmessage(Window window, Atom atom, long x)
{
    XEvent  event;
    
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = atom;
    event.xclient.format = 32;
    event.xclient.data.l[0] = x;
    event.xclient.data.l[1] = CurrentTime;
    
    return XSendEvent(display, window, False, NoEventMask, &event);
}

/*
 * send_wm_delete: send delete request to a client
 */

void send_wm_delete(t_client *client)
{
    int     i, n, found;
    Atom    *protocols;
    
    found = 0;
    if (client)
    {
        if (XGetWMProtocols(display, client->window, &protocols, &n))
        {
            for (i = 0; i < n; i++)
                if (protocols[i] == atom_wm_delete)
                    found++;
            XFree(protocols);
        }
        if (found)
            send_xmessage(client->window, atom_wm_protos, atom_wm_delete);
        else
            XKillClient(display, client->window);
    }
}

/*
 * set_smart_position: set smart position for a client
 *                     (according to position like keypad, 0-9, 5=centered)
 */

void set_smart_position(t_client *client, char position)
{
    switch (position)
    {
        /* no position specified, we leave default position */
        case '0':
            break;
        /* bottom left */
        case '1':
            client->x = client->border;
            client->y = get_y_max(client->screen) - client->height - (client->border * 2);
            break;
        /* bottom centered */
        case '2':
            client->x = (get_x_max(client->screen) - client->width - client->border) / 2;
            client->y = get_y_max(client->screen) - client->height - (client->border * 2);
            break;
        /* bottom right */
        case '3':
            client->x = get_x_max(client->screen) - client->width - (client->border * 2);
            client->y = get_y_max(client->screen) - client->height - (client->border * 2);
            break;
        /* left centered */
        case '4':
            client->x = client->border;
            client->y = (get_y_max(client->screen) - client->height - client->border) / 2;
            break;
        /* centered on screen */
        case '5':
            client->x = (get_x_max(client->screen) - client->width - client->border) / 2;
            client->y = (get_y_max(client->screen) - client->height - client->border) / 2;
            break;
        /* right centered */
        case '6':
            client->x = get_x_max(client->screen) - client->width - (client->border * 2);
            client->y = (get_y_max(client->screen) - client->height - client->border) / 2;
            break;
        /* top left */
        case '7':
            client->x = client->border;
            client->y = client->border;
            break;
        /* top centered */
        case '8':
            client->x = (get_x_max(client->screen) - client->width - client->border) / 2;
            client->y = client->border;
            break;
        /* top right */
        case '9':
            client->x = get_x_max(client->screen) - client->width - (client->border * 2);
            client->y = client->border;
            break;
    }
}

/*
 * set_dock_position: set window position for dock
 */

void set_dock_position(t_client *client)
{
    int     pos, max, size;
    
    if (config_dock_use)
    {
        if ( (config_dock_position == DOCK_TOP) || (config_dock_position == DOCK_BOTTOM) )
        {
            max = DisplayWidth(display, client->screen);
            size = client->width;
        }
        else
        {
            max = DisplayHeight(display, client->screen);
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
                client->x = DisplayWidth(display, client->screen) - config_dock_size;
                client->y = pos;
                break;
            case DOCK_BOTTOM:
                client->x = pos;
                client->y = DisplayHeight(display, client->screen) - config_dock_size;
                break;
        }
        dock_pos_free += size;
    }
}

/*
 * get_dock_size: return dock size
 */

int get_dock_size()
{
    t_client    *ptr_client;
    int         size;
    
    size = 0;
    for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
    {
        if (ptr_client->dock_position != NO_DOCK)
        {
            if ( (config_dock_position == DOCK_TOP) || (config_dock_position == DOCK_BOTTOM) )
                size += ptr_client->width;
            else
                size += ptr_client->height;
        }
    }
    return size;
}

/*
 * recalculate_dock_positions: recalculate position of each window in the dock
 */

void recalculate_dock_positions()
{
    t_client    *ptr_client;
    
    if (config_dock_use)
    {
        dock_pos_free = 0;
        dock_size = get_dock_size();
        for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
        {
            if (ptr_client->dock_position != NO_DOCK)
            {
                ptr_client->dock_position = dock_pos_free;
                set_dock_position(ptr_client);
                client_move(ptr_client, 1);
            }
        }
    }
}

/*
 * init_position: init client position (according to smart position or memorized position)
 */

void init_position(t_client *client)
{
    int         x, y, xmax, ymax, num_clients;
    t_keymap    *ptr_keymap;
    t_client    *ptr_client;
    t_memo_win  *ptr_memo_win;

    xmax = DisplayWidth(display, client->screen);
    ymax = DisplayHeight(display, client->screen);
    
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
        set_dock_position(client);
        if (config_dock_align == DOCK_ALIGN_CENTER)
            recalculate_dock_positions();
        
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
        client->x = (x / (float)xmax) * (xmax - client->border - client->width);
        client->y = (y / (float)ymax) * (ymax - client->border - client->height);
        
        /* look for memorized position and/or desktop */
        for (ptr_memo_win = memo_win; ptr_memo_win; ptr_memo_win = ptr_memo_win->next)
            if (strcmp(ptr_memo_win->application_name, client->application_name) == 0)
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
                client_move(client, 1);
                return;
            }
        }
        
        /* look for smart position */
        for (ptr_keymap = keymap; ptr_keymap; ptr_keymap = ptr_keymap->next)
        {
            if ( (ptr_keymap->command_file != NULL) && (ptr_keymap->smart_position != NULL) )
            {
                if (strcasecmp(client->application_name, ptr_keymap->application_name) == 0)
                {
                    /* smart position found, count the number of clients opened with this name */
                    num_clients = 0;
                    for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
                    {
                        if ( (ptr_client != client) &&
                            (ptr_client->vdesk[client->screen] == client->vdesk[client->screen]) &&
                            (strcasecmp(ptr_client->application_name, client->application_name) == 0) )
                            num_clients++;
                    }
                    if (num_clients >= (int)strlen(ptr_keymap->smart_position))
                        return;
                    set_smart_position(client, ptr_keymap->smart_position[num_clients]);
                }
            }
        }
    }
}

/*
 * change_gravity: change gravity for client (multiplier: 1 = gravity, -1 = ungravity)
 */

void change_gravity(t_client *client, int multiplier)
{
    int     dx, dy;
    int     gravity;
    
    dx = 0;
    dy = 0;
    gravity = (client->size->flags & PWinGravity) ? client->size->win_gravity : NorthWestGravity;
  
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
 * send_config: send config to client
 */

void send_config(t_client *client)
{
    XConfigureEvent     config_event;
  
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
  
    XSendEvent(display, client->window, False, StructureNotifyMask, (XEvent *)&config_event);
}

/*
 * reparent: reparent a client
 */

void reparent(t_client *client)
{
    XSetWindowAttributes    p_attr;

    XSelectInput(display, client->window, EnterWindowMask | PropertyChangeMask);
  
    p_attr.override_redirect = True;
    p_attr.background_pixel = color_border0.pixel;
    p_attr.event_mask =
        SubstructureRedirectMask | SubstructureNotifyMask | FocusChangeMask |
        ButtonPressMask | ExposureMask | EnterWindowMask;
    
    if (client->dock_position == NO_DOCK)
        client->parent = XCreateWindow(display, RootWindow(display, client->screen),
            client->x-client->border, client->y-client->border,
            client->width+(client->border*2), client->height + (client->border*2)/* +15 */, 0,
            DefaultDepth(display, client->screen), CopyFromParent,
            DefaultVisual(display, client->screen),
            CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
    else
        client->parent = XCreateWindow(display, RootWindow(display, client->screen),
            client->x-client->border, client->y-client->border,
            client->width+(client->border*2), client->height+(client->border*2), 0,
            DefaultDepth(display, client->screen), CopyFromParent,
            DefaultVisual(display, client->screen),
            CWOverrideRedirect | CWBackPixel | CWEventMask, &p_attr);
    
    XAddToSaveSet(display, client->window);
    XSetWindowBorderWidth(display, client->window, 0);
    if (client->dock_position == NO_DOCK)
        XReparentWindow(display, client->window, client->parent,
            client->border, client->border/* +15 */);
    else
        XReparentWindow(display, client->window, client->parent,
            client->border, client->border);
    
    send_config(client);
}


/*
 * create_new_client: create a new client
 */

void create_new_client(Window window, int screen)
{
    t_client            *client;
    XWindowAttributes   attr;
    long                dummy;
    XWMHints            *hints;
    char                *name;
    int                 format;
    unsigned long       nitems, bytes_after;
    Atom                atom_return;
    /*t_motif_wm_hints    *motif_wm_hints;*/
    unsigned char       *motif_wm_hints;
    
    XGrabServer(display);
    hints = XGetWMHints(display, window);
    client = (t_client *)malloc(sizeof(t_client));
    client->next = head_client;
    client->screen = screen;
    
    head_client = client;
    
    client->window = window;
    client->ignore_unmap = 0;
    
    client->size = XAllocSizeHints();
    XGetWMNormalHints(display, client->window, client->size, &dummy);
    XFetchName(display, client->window, &name);
    XGetWindowAttributes(display, client->window, &attr);
    XGetWindowProperty(display, client->window, atom_motif_wm_hints, 0, 3,  False,
        atom_motif_wm_hints, &atom_return, &format, &nitems, &bytes_after,
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
    client->application_name = (char *)malloc((strlen(name)+1)*(sizeof(char)));
    strcpy(client->application_name, name);
    
    if ( ((t_motif_wm_hints *)motif_wm_hints) && (((t_motif_wm_hints *)(motif_wm_hints))->flags & 2) )
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
        init_position(client);
        if ( (hints) && (hints->flags & StateHint) )
            set_wm_state(client->window, atom_wm_state, hints->initial_state, None);
    }
    
    if (name)
        XFree(name);
    if (hints)
        XFree(hints);
    if (motif_wm_hints)
        XFree(motif_wm_hints);
    
    change_gravity(client, 1);
    reparent(client);
    XMapWindow(display, client->window);
    XMapRaised(display, client->parent);
    set_wm_state(client->window, atom_wm_state, NormalState, None);
  
    XSync(display, False);
    XUngrabServer(display);
    
    if (client->dock_position == NO_DOCK)
        XWarpPointer(display, None, client->window, 0, 0, 0, 0,
            client->width-1, client->height-1);
    
    if (client->vdesk[client->screen] != vdesk[client->screen])
        switch_desktop(client->vdesk[screen], client->screen);
}

/*
 * find_client: find a client corresponding to a window
 */

t_client *find_client(Window window)
{
    t_client    *client;
  
    for (client = head_client; client; client = client->next)
        if ( (window == client->parent) || (window == client->window) )
            return client;
    return NULL;
}

/*
 * get_first_client_desktop: return the first client found for a given desktop
 */

t_client *get_first_client_desktop(int desktop, int screen)
{
    t_client    *ptr_client;
  
    for (ptr_client = head_client; ptr_client; ptr_client = ptr_client->next)
        if ( (ptr_client->dock_position == NO_DOCK) && (ptr_client->vdesk[screen] == desktop) )
            return ptr_client;
    return NULL;
}

/*
 * remove_client: remove a client
 */

void remove_client(t_client *client, int from_cleanup)
{
    t_client    *parent;
    int         recalculate_dock;

    XGrabServer(display);
    XSetErrorHandler(ignore_xerror);
  
    if (!from_cleanup)
        set_wm_state(client->window, atom_wm_state, WithdrawnState, None);
  
    free(client->application_name);
    recalculate_dock = (client->dock_position != NO_DOCK);
    change_gravity(client, -1);
    XRemoveFromSaveSet(display, client->window);
    XReparentWindow(display, client->window,
        RootWindow(display, client->screen), client->x, client->y);
    XSetWindowBorderWidth(display, client->window, 1);
    XDestroyWindow(display, client->parent);
    
    if (head_client == client)
        head_client = client->next;
    else
    {
        for (parent = head_client; parent && parent->next; parent = parent->next)
            if (parent->next == client)
                parent->next = client->next;
    }
    
    if (client->size)
        XFree(client->size);
    current_client = NULL;
    free(client);
    
    if (recalculate_dock)
        recalculate_dock_positions();
    
    XSync(display, False);
    XSetErrorHandler(handle_xerror);
    XUngrabServer(display);
}
