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


/* screen.c: various actions on clients */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>

#include "weewm.h"
#include "screen.h"
#include "client.h"
#include "info.h"
#include "config.h"


/*
 * ScreenOfWindow: returns the screen of the window
 */

int ScreenOfWindow(Window window)
{
    XWindowAttributes   attr;
    int                 i;
    
    if (num_heads == 1)
        return DefaultScreen(display);
    
    XGetWindowAttributes(display, window, &attr);
    
    
    for (i = 0; i < num_heads; i++)
    {
        if (ScreenOfDisplay(display, i) == attr.screen)
            return i;
    }
    return DefaultScreen(display);
}

/*
 * ScreenOfRootWindow: returns the screen of the root window
 */

int ScreenOfRootWindow(Window window)
{
    int     i;

    for (i = 0; i < num_heads; i++)
    {
        if (RootWindow(display, i) == window)
            return i;
    }
    return DefaultRootWindow(display);
}

/*
 * get_x_min: returns min X for windows, excluding dock
 */

int get_x_min(int screen)
{
    (void)screen;
    if ( (config_dock_use) && (config_dock_position == DOCK_LEFT) )
        return config_dock_size;
    else
        return 0;
}

/*
 * get_x_max: returns max X for windows, excluding dock
 */

int get_x_max(int screen)
{
    if ( (config_dock_use) && (config_dock_position == DOCK_RIGHT) )
        return DisplayWidth(display, screen) - config_dock_size;
    else
        return DisplayWidth(display, screen);
}

/*
 * get_y_min: returns min Y for windows, excluding dock
 */

int get_y_min(int screen)
{
    (void)screen;
    if ( (config_dock_use) && (config_dock_position == DOCK_TOP) )
        return config_dock_size;
    else
        return 0;
}

/*
 * get_y_max: returns max Y for windows, excluding dock
 */

int get_y_max(int screen)
{
    if ( (config_dock_use) && (config_dock_position == DOCK_BOTTOM) )
        return DisplayHeight(display, screen) - config_dock_size;
    else
        return DisplayHeight(display, screen);
}

/*
 * draw_outline: draws a rectangle around a window
 */

void draw_outline(t_client *client)
{
    char    buf[20];
    int     text_size;
    int     width_inc, height_inc;
    
    width_inc = 1;
    height_inc = 1;

    XDrawRectangle(display, RootWindow(display, client->screen),
        invert_gc[client->screen], client->x - client->border,
        client->y - client->border, client->width + client->border,
        client->height + client->border);

    if (client->size->flags & PResizeInc)
    {
        width_inc = client->size->width_inc;
        height_inc = client->size->height_inc;
    }
    
    snprintf(buf, sizeof(buf), "%dx%d+%d+%d", client->width/width_inc,
        client->height/height_inc, client->x, client->y);
    
    text_size = XTextWidth(font, buf, strlen(buf));
    XDrawString(display, RootWindow(display, client->screen),
        invert_gc[client->screen],
        client->x + client->width - text_size - config_space,
        client->y + client->height - config_space,
        buf, strlen(buf));
}

/*
 * get_mouse_position: returns mouse position
 */

void get_mouse_position(int *x, int *y, int screen)
{
    Window          root, child;
    int             win_x, win_y;
    unsigned int    mask;
  
    XQueryPointer(display, RootWindow(display, screen), &root, &child,
        x, y, &win_x, &win_y, &mask);
}

/*
 * hide: hide client window
 */

void hide(t_client *client)
{
    if (client)
    {
        client->ignore_unmap = 2;
        XUnmapWindow(display, client->parent);
        XUnmapWindow(display, client->window);
        set_wm_state(client->window, atom_wm_state, IconicState, None);
    }
}

/*
 * unhide: unhide client window
 */

void unhide(t_client *client, int raise)
{
    client->ignore_unmap = 0;
    XMapWindow(display, client->window);
    (raise) ? XMapRaised(display, client->parent) : XMapWindow(display, client->parent);
    set_wm_state(client->window, atom_wm_state, NormalState, None);
}

/*
 * recalculate_sweep: recalculate sweep for a client
 */

void recalculate_sweep(t_client *client, int x1, int y1, int x2, int y2)
{
    int     basex, basey;
    
    client->width = abs(x1 - x2);
    client->height = abs(y1 - y2);
    
    if (client->size->flags & PResizeInc)
    {
        basex = (client->size->flags & PBaseSize) ? client->size->base_width : (client->size->flags & PMinSize) ? client->size->min_width : 0;
        basey = (client->size->flags & PBaseSize) ? client->size->base_height : (client->size->flags & PMinSize) ? client->size->min_height : 0;
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
 * sweep: sweep a client
 */

void sweep(t_client *client)
{
    XEvent  event;
    int     old_cx, old_cy;
    
    old_cx = client->x;
    old_cy = client->y;
    
    if (XGrabPointer(display, RootWindow(display, client->screen), False,
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
        GrabModeAsync, GrabModeAsync, None, cursor_resize, CurrentTime) != GrabSuccess)
        return;
    XGrabServer(display);
    
    draw_outline(client);
    
    XWarpPointer(display, None, client->window, 0, 0, 0, 0, client->width-1, client->height-1);
    for (;;)
    {
        XMaskEvent(display, ButtonPressMask | ButtonReleaseMask | PointerMotionMask, &event);
        switch (event.type)
        {
            case MotionNotify:
                draw_outline(client);
                recalculate_sweep(client, old_cx, old_cy, event.xmotion.x, event.xmotion.y);
                draw_outline(client);
                break;
            case ButtonRelease:
                draw_outline(client);
                XUngrabServer(display);
                XUngrabPointer(display, CurrentTime);
                return;
        }
    }
}

/*
 * client_drag: drag the client window
 */

void client_drag(t_client *client)
{
    XEvent  event;
    int     x1, y1;
    int     old_cx, old_cy;
    
    if (client)
    {
        if (client->dock_position != NO_DOCK)
            return;
        
        old_cx = client->x;
        old_cy = client->y;
        
        if (XGrabPointer(display, RootWindow(display, client->screen), False,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
            GrabModeAsync, GrabModeAsync, None,
            cursor_move, CurrentTime) != GrabSuccess)
            return;
        get_mouse_position(&x1, &y1, client->screen);
        for (;;)
        {
            XMaskEvent(display,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                &event);
            switch (event.type)
            {
                case MotionNotify:
                    client->x = old_cx + (event.xmotion.x - x1);
                    client->y = old_cy + (event.xmotion.y - y1);
                    XMoveWindow(display, client->parent,
                        client->x - client->border, client->y - client->border);
                    send_config(client);
                    break;
                case ButtonRelease:
                    XUngrabPointer(display, CurrentTime);
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

void client_move(t_client *client, int set)
{
    if (client)
    {
        if (!set)
            client_drag(client);
        XMoveWindow(display, client->parent, client->x - client->border, client->y - client->border);
        send_config(client);
        XRaiseWindow(display, client->parent);
    }
}

/*
 * client_resize: resize the client window
 */

void client_resize(t_client *client, int set)
{
    if (client)
    {
        if (client->dock_position != NO_DOCK)
            return;
        if (!set)
            sweep(client);
        XMoveResizeWindow(display, client->parent, client->x,
            client->y, client->width + (client->border*2),
            client->height + (client->border*2));
        XMoveResizeWindow(display, client->window, client->border, client->border,
            client->width, client->height);
        send_config(client);
        XRaiseWindow(display, client->parent);
    }
}

/*
 * switch_desktop: switch from desktop to another
 */

void switch_desktop(int vdesk_target, int screen)
{
    t_client    *client;
    int         wdesk;
    char        desktop_name[256];
    
    if (vdesk_target == vdesk[screen])
        return;
    for (client = head_client; client; client = client->next)
    {
        if (client->dock_position == NO_DOCK)
        {
            wdesk = client->vdesk[screen];
            if ( (wdesk == vdesk[screen]) && (wdesk != FIXED_WINDOW) )
                hide(client);
            else
                if (wdesk == vdesk_target)
                    unhide(client, 0);
        }
    }
    vdesk[screen] = vdesk_target;
    
    sprintf(desktop_name, "Desktop %d", vdesk_target+1);
    info_add(desktop_name, screen);
}
