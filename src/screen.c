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


/* screen.c: X windows management */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>

#include "weewm.h"
#include "screen.h"
#include "weeconfig.h"
#include "client.h"
#include "info.h"


/*
 * ScreenOfWindow: returns the screen of the window
 */

int ScreenOfWindow (Window window)
{
    XWindowAttributes   attr;
    int                 i;
    
    if (num_heads == 1)
        return DefaultScreen (display);
    
    XGetWindowAttributes (display, window, &attr);
    
    for (i = 0; i < num_heads; i++)
    {
        if (ScreenOfDisplay (display, i) == attr.screen)
            return i;
    }
    return DefaultScreen (display);
}

/*
 * ScreenOfRootWindow: returns the screen of the root window
 */

int ScreenOfRootWindow (Window window)
{
    int i;

    for (i = 0; i < num_heads; i++)
    {
        if (RootWindow (display, i) == window)
            return i;
    }
    return DefaultRootWindow (display);
}

/*
 * get_x_min: returns min X for windows, excluding dock
 */

int get_x_min (int screen)
{
    (void) screen;
    if ( (config_dock_use) && (config_dock_position == DOCK_LEFT) )
        return config_dock_size;
    else
        return 0;
}

/*
 * get_x_max: returns max X for windows, excluding dock
 */

int get_x_max (int screen)
{
    if ( (config_dock_use) && (config_dock_position == DOCK_RIGHT) )
        return DisplayWidth (display, screen) - config_dock_size;
    else
        return DisplayWidth (display, screen);
}

/*
 * get_y_min: returns min Y for windows, excluding dock
 */

int get_y_min (int screen)
{
    (void) screen;
    if ( (config_dock_use) && (config_dock_position == DOCK_TOP) )
        return config_dock_size;
    else
        return 0;
}

/*
 * get_y_max: returns max Y for windows, excluding dock
 */

int get_y_max (int screen)
{
    if ( (config_dock_use) && (config_dock_position == DOCK_BOTTOM) )
        return DisplayHeight (display, screen) - config_dock_size;
    else
        return DisplayHeight (display, screen);
}

/*
 * get_mouse_position: returns mouse position
 */

void get_mouse_position (int *x, int *y, int screen)
{
    Window root, child;
    int win_x, win_y;
    unsigned int mask;
  
    XQueryPointer(display, RootWindow(display, screen), &root, &child,
                  x, y, &win_x, &win_y, &mask);
}

/*
 * switch_desktop: switch from desktop to another
 */

void switch_desktop (int vdesk_target, int screen)
{
    t_client *client;
    int wdesk;
    char desktop_name[256];
    
    if (vdesk_target == vdesk[screen])
        return;
    for (client = head_client; client; client = client->next)
    {
        if (client->dock_position == NO_DOCK)
        {
            wdesk = client->vdesk[screen];
            if ( (wdesk == vdesk[screen]) && (wdesk != FIXED_WINDOW) )
                client_hide (client);
            else
                if (wdesk == vdesk_target)
                    client_unhide (client, 0);
        }
    }
    vdesk[screen] = vdesk_target;
    
    sprintf(desktop_name, "Desktop %d", vdesk_target + 1);
    info_add(desktop_name, screen);
}
