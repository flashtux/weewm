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


/* events.c: events management */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#include "weewm.h"
#include "events.h"
#include "weeconfig.h"
#include "client.h"
#include "screen.h"
#include "memorize.h"


int switch_to_desktop;


/*
 * launch_command: creates a new process and launch a command with (optional) parameters 
 */

void launch_command (char *cmd, char *const argv[], int screen)
{
    pid_t pid;
    char *buffer, *display;
    int buffer_length, display_length;

    if ( (pid = fork()) == 0 )
    {
        /* child process, execute command */
        
        buffer_length = strlen (display_name ? display_name : DEFAULT_DISPLAY) + 10;
        buffer = (char *) malloc (buffer_length * sizeof (char));
        snprintf (buffer, buffer_length, "%s.%i",
                  display_name ? display_name : DEFAULT_DISPLAY, screen);

        #ifdef DEBUG
        fprintf (stderr, "WeeWM debug: starting command '%s', on display %i - %s\n",
                 cmd, screen, buffer);
        #endif
        
        display_length = strlen (buffer) + 8 + 1;
        display = (char *) malloc (display_length * sizeof (char));
        snprintf (display, display_length, "DISPLAY=%s", buffer);
        putenv (display);
        execvp (cmd, argv);
    }
}

/*
 * action_key: do the right action correspondig to the key pressed
 */

void action_key (t_keymap *km, KeySym key, KeySym mask, XKeyEvent *event)
{
    t_keymap *ptr_keymap;
    t_client *ptr_client;
    
    #ifdef DEBUG
    fprintf (stderr,
             "WeeWM debug: action_key called with key = %d, mask = %d\n",
             (int) key, (int) mask);
    #endif
    switch_to_desktop = -1;
    for (ptr_keymap = km; ptr_keymap != NULL; ptr_keymap = ptr_keymap->next)
    {
        if ( (ptr_keymap->keysym == (int) key)
            && (ptr_keymap->modifier_mask == (int) mask) )
        {
            if (ptr_keymap->command_file == NULL)
            {
                if (ptr_keymap->handler == NULL)
                    return;
                else
                {
                    switch_to_desktop = -1;
                    if (ptr_keymap->key_switch_desktop >= 0)
                        switch_to_desktop = ptr_keymap->key_switch_desktop;
                    ptr_keymap->handler (event);
                    return;
                }
            }
            else
            {
                if (ptr_keymap->only_one)
                {
                    for (ptr_client = head_client; ptr_client;
                         ptr_client = ptr_client->next)
                    {
                        if (strcmp (ptr_client->application_name,
                                    ptr_keymap->application_name) == 0)
                        {
                            switch_desktop (ptr_client->vdesk[ScreenOfRootWindow(event->root)],
                                            ScreenOfRootWindow (event->root));
                            return;
                        }
                    }
                    return;
                }
                if (ptr_keymap->initial_desktop > 0)
                {                    
                    if  (ptr_keymap->initial_desktop <= config_num_desktops)
                        switch_desktop (ptr_keymap->initial_desktop - 1,
                                        ScreenOfRootWindow (event->root));
                    else
                        fprintf (stderr,
                                 "WeeWM warning: can't switch to desktop "
                                 "%d for command %s\n",
                                 ptr_keymap->initial_desktop,
                                 ptr_keymap->application_name);
                }
                launch_command (ptr_keymap->command_file,
                                ptr_keymap->command_args,
                                ScreenOfRootWindow (event->root));
            }
        }
    }
}

/*
 * handle_key_event: handler for key event
 */

void handle_key_event (XKeyEvent *event)
{
    t_client *client;
    KeySym key, state;

    client = client_search (event->window);
    key = XKeycodeToKeysym (display, event->keycode, 0);
    state = event->state;
    
    if (!client)
        client = current_client;
    action_key (keymap, key, state, event);
}

/*
 * handle_button_event: handler for button event
 */

void handle_button_event (XButtonEvent *event)
{
    t_client *client;

    client = client_search (event->window);
    
    if (client && event->window != RootWindow (display, client->screen))
    {
        switch (event->button)
        {
            case Button1:
                XRaiseWindow (display, client->parent);
                client_move (client, 0);
                break;
            case Button3:
                XRaiseWindow (display, client->parent);
                client_resize (client, 0);
                break;
            case Button2:
                XLowerWindow (display, client->parent);
                break;
        }
    }
}

/*
 * handle_convfigure_request: handler for configure request event
 */

void handle_configure_request (XConfigureRequestEvent *event)
{
    t_client *client;
    XWindowChanges wc;
    
    client = client_search (event->window);
    
    wc.sibling = event->above;
    wc.stack_mode = event->detail;
    if (client)
    {
        client_change_gravity (client, -1);
        if (event->value_mask & CWX)
            client->x = event->x;
        if (event->value_mask & CWY)
            client->y = event->y;
        if (event->value_mask & CWWidth)
            client->width = event->width;
        if (event->value_mask & CWHeight)
            client->height = event->height;
        client_change_gravity (client, 1);
    
        wc.x = client->x - client->border;
        wc.y = client->y - client->border;
        wc.width = client->width + (client->border * 2);
        wc.height = client->height + (client->border * 2);
        wc.border_width = 0;
        XConfigureWindow (display, client->parent, event->value_mask, &wc);
        client_send_config (client);
        
        if (client->dock_position != NO_DOCK)
            client_recalculate_dock_positions ();
    }
  
    wc.x = client ? client->border : event->x;
    wc.y = client ? client->border : event->y;
    wc.width = event->width;
    wc.height = event->height;
    XConfigureWindow (display, event->window, event->value_mask, &wc);
}

/*
 * handle_map_request: handler for map request event
 */

void handle_map_request (XMapRequestEvent *event)
{
    t_client *client;

    client = client_search (event->window);
    
    if (client)
    {
        if (client->vdesk[client->screen] != vdesk[client->screen])
            switch_desktop(client->vdesk[client->screen], client->screen);
        client_unhide (client, 1);
    }
    else
        client_new (event->window, ScreenOfWindow(event->window));
}

/*
 * handle_unmap_event: handler for unmap event
 */

void handle_unmap_event (XUnmapEvent *event)
{
    t_client *client;

    client = client_search (event->window);
  
    if (client)
    {
        if (client->ignore_unmap)
            client->ignore_unmap--;
        else
            client_remove (client, 0);
    }
}

/*
 * handle_property_change: handler for property change event
 */

void handle_property_change (XPropertyEvent *event)
{
    t_client *client;
    long dummy;
    
    client = client_search (event->window);
    
    if (client && (event->atom == XA_WM_NORMAL_HINTS))
        XGetWMNormalHints (display, client->window, client->size, &dummy);
}

/*
 * handle_enter_event: handler for enter event
 */

void handle_enter_event (XCrossingEvent *event)
{
    int         screen, wdesk, i;
    t_client    *client;
    
    screen = ScreenOfRootWindow (event->root);
    client = client_search (event->window);
    
    if (client)
    {
        wdesk = client->vdesk[screen];
        if ( (wdesk != vdesk[screen]) && (wdesk != -1) )
            return;
    
        if (current_client && (client != current_client))
        {
            XSetWindowBackground (display, current_client->parent,
                                  color_border0.pixel);
            XClearWindow (display, current_client->parent);
        }
        XSetWindowBackground (display, client->parent,
                              (wdesk == FIXED_WINDOW) ?
                                  color_sticky.pixel : color_border1.pixel);
        XClearWindow (display, client->parent);
        
        /*for (i = 0; i < 15; i++)
            XDrawLine (display, client->parent, invert_gc[client->screen], 0, i, i*4, i);*/
        
        current_client = client;
        XSetInputFocus (display, client->window, RevertToPointerRoot,
                        CurrentTime);

        for (i = 0; i < NUM_IGNORE_MODIFIERS; i++)
        {
            XGrabButton (display, AnyButton, ignore_modifiers[i] | mouse_mask,
                         client->parent, False,
                         ButtonPressMask | ButtonReleaseMask,
                         GrabModeAsync, GrabModeSync, None, None);
        } 
    }
}

/*
 * action_next_win: focuses the next window
 */

void action_next_win (XKeyEvent *event)
{
    t_client *client, *ptr_client, *client_found;
    int screen;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    screen = ScreenOfRootWindow (event->root);
    
    if (!client)
        client_found = client_get_first_for_desktop (vdesk[screen], screen);
    else
    {
        ptr_client = (client->next) ? client->next : head_client;
        client_found = NULL;
        while (ptr_client != client)
        {
            if ( ( (ptr_client->vdesk[ptr_client->screen] == FIXED_WINDOW) ||
                (ptr_client->vdesk[ptr_client->screen] == client->vdesk[client->screen]) )
                && (ptr_client->dock_position == NO_DOCK)
                && (ptr_client->vdesk[ptr_client->screen] != -1) )
                client_found = ptr_client;
            ptr_client = (ptr_client->next) ? ptr_client->next : head_client;
        }
    }
    if (client_found)
    {
        client_unhide (client_found, 1);
        XWarpPointer (display, None, client_found->window, 0, 0, 0, 0,
            client_found->width - 1, client_found->height - 1);
    }
}

/*
 * action_previous_win: focuses the previous window
 */

void action_previous_win (XKeyEvent *event)
{
    t_client *client, *ptr_client;
    int screen;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    screen = ScreenOfRootWindow (event->root);
    if (!client)
        ptr_client = client_get_first_for_desktop (vdesk[screen], screen);
    else
    {
        ptr_client = (client->next) ? client->next : head_client;
        while (ptr_client != client)
        {
            if ( ( (ptr_client->vdesk[ptr_client->screen] == FIXED_WINDOW) ||
                (ptr_client->vdesk[ptr_client->screen] == client->vdesk[client->screen]) )
                && (ptr_client->dock_position == NO_DOCK)
                && (ptr_client->vdesk[ptr_client->screen] != -1) )
                break;
            ptr_client = (ptr_client->next) ? ptr_client->next : head_client;
        }
    }
    if (ptr_client)
    {
        client_unhide (ptr_client, 1);
        XWarpPointer (display, None, ptr_client->window, 0, 0, 0, 0,
                      ptr_client->width - 1, ptr_client->height - 1);
    }
}

/*
 * action_pos_top_left: moves the window to the top left corner
 */

void action_pos_top_left (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '7');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_top: moves the window to the top edge of the screen
 */

void action_pos_top (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '8');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_top_right: moves the window to the top right corner
 */

void action_pos_top_right (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '9');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_left: moves the window to the left edge of the screen
 */

void action_pos_left (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '4');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_center: centers the window on the screen
 */

void action_pos_center (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '5');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_right: moves the window to the right edge of the screen
 */

void action_pos_right (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '6');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_bottom_left: moves the window to the bottom left corner of the screen
 */

void action_pos_bottom_left (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '1');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_bottom: moves the window to the bottom edge of the screen
 */

void action_pos_bottom (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '2');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_pos_bottom_right: moves the window to the bottom right corner of the screen
 */

void action_pos_bottom_right (XKeyEvent *event)
{
    t_client *client;

    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    client_set_smart_position (client, '3');
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_move_left: moves the window to the left (of <offset> pixels)
 */

void action_move_left (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;

    if (client->dock_position != NO_DOCK)
        return;
    
    client->x -= config_move_offset; 
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_move_right: moves the window to the right (of <offset> pixels)
 */

void action_move_right (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;

    if (client->dock_position != NO_DOCK)
        return;
    
    client->x += config_move_offset; 
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_move_up: moves the window up (of <offset> pixels)
 */

void action_move_up (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;

    if (client->dock_position != NO_DOCK)
        return;
    
    client->y -= config_move_offset; 
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
                  client->width - 1, client->height - 1);
}

/*
 * action_move_down: moves the window down (of <offset> pixels)
 */

void action_move_down (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;

    if (client->dock_position != NO_DOCK)
        return;
    
    client->y += config_move_offset; 
    client_move (client, 1);
    XWarpPointer (display, None, client->window, 0, 0, 0, 0,
        client->width - 1, client->height - 1);
}

/*
 * action_previous_desktop: displays previous desktop
 */

void action_previous_desktop (XKeyEvent *event)
{
    int screen;
    t_client *client;
    
    screen = ScreenOfRootWindow (event->root);
    if (vdesk[screen] > 0)
    {
        client = client_search (event->window);
        if (client == NULL)
            client = current_client;
        switch_desktop (vdesk[screen]-1, screen);
    }
    else
        switch_desktop (config_num_desktops-1, screen);
}

/*
 * action_next_desktop: displays next desktop
 */

void action_next_desktop (XKeyEvent *event)
{
    int screen;
    t_client *client;
    
    screen = ScreenOfRootWindow (event->root);
    if (vdesk[screen] < (config_num_desktops-1))
    {
        client = client_search (event->window);
        if (client == NULL)
            client = current_client;
        switch_desktop (vdesk[screen]+1, screen);
    }
    else
        switch_desktop (0, screen);
}

/*
 * action_go_to_desktop: go to desktop N
 */

void action_go_to_desktop (XKeyEvent *event)
{
    int screen;
    t_client *client;
    
    screen = ScreenOfRootWindow (event->root);
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if ( (switch_to_desktop >= 0) && (switch_to_desktop < config_num_desktops) )
        switch_desktop (switch_to_desktop, screen);
}

/*
 * action_send_previous_desktop: sends the window to the previous desktop
 */

void action_send_previous_desktop (XKeyEvent *event)
{
    int screen;
    t_client *client;
    
    if (head_client == NULL)
        return;
    screen = ScreenOfRootWindow (event->root);
    if (vdesk[screen] > 0)
    {
        client = client_search (event->window);
        if (client == NULL)
            client = current_client;
        if (client == NULL)
            return;
        if (client->dock_position != NO_DOCK)
            return;
        if (client->vdesk[screen] != FIXED_WINDOW)
            client->vdesk[screen]--;
        switch_desktop (vdesk[screen] - 1, screen);
    }
}

/*
 * action_send_next_desktop: sends the window to the next desktop
 */

void action_send_next_desktop (XKeyEvent *event)
{
    int screen;
    t_client *client;
    
    if (head_client == NULL)
        return;
    screen = ScreenOfRootWindow (event->root);
    if (vdesk[screen] < config_num_desktops)
    {
        client = client_search (event->window);
        if (client == NULL)
            client = current_client;
        if (client == NULL)
            return;
        if (client->dock_position != NO_DOCK)
            return;
        if (client->vdesk[screen] != FIXED_WINDOW)
            client->vdesk[screen]++;
        switch_desktop (vdesk[screen] + 1, screen);
    }
}

/*
 * action_max_horizontal: maximizes window horizontally
 */

void action_max_horizontal (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    if (client->oldw)
    {
        client->x = client->oldx;
        client->width = client->oldw;
        client->oldw = 0;
    } 
	else
    {
        client->oldx = client->x;
        client->oldw = client->width;
        client_recalculate_sweep (client,
                                  get_x_min (client->screen),
                                  client->y,
                                  get_x_max (client->screen),
                                  client->y + client->height);
    }
    
    client_resize (client, 1);
}

/*
 * action_max_vertical: maximizes window vertically
 */

void action_max_vertical (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    if (client->oldh)
    {
        client->y = client->oldy;
        client->height = client->oldh;
        client->oldh = 0;
    } 
    else
    {
        client->oldy = client->y;
        client->oldh = client->height;
        client_recalculate_sweep (client,
                                  client->x, get_y_min(client->screen),
                                  client->x + client->width,
                                  get_y_max (client->screen));
    }
    
    client_resize (client, 1);
}

/*
 * action_maximize: maximizes window
 */

void action_maximize (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    if (client->dock_position != NO_DOCK)
        return;
    
    if (client->oldmh && client->oldmw)
    {
        client->y = client->oldmy;
        client->height = client->oldmh;
        client->oldmh = 0;
        client->x = client->oldmx;
        client->width = client->oldmw;
        client->oldmw = 0;
    }
    else
    {
        client->oldmy = client->y;
        client->oldmh = client->height;
        client->oldmx = client->x;
        client->oldmw = client->width;

        client_recalculate_sweep (client,
                                  get_x_min (client->screen) + client->border,
                                  get_y_min (client->screen) + client->border,
                                  get_x_max (client->screen) - client->border,
                                  get_y_max (client->screen) - client->border);
    }
    
    client_resize (client, 1);
}

/*
 * action_smart_maximize: maximizes window in free space
 *                        => looks for bigger free space
 *                        available on current desktop
 */

void action_smart_maximize (XKeyEvent *event)
{
    t_client *client;
    int num_clients;
    
    /* TODO: write this function */
}

/*
 * action_fix_window: fix/unfix window
 */

void action_fix_window (XKeyEvent *event)
{
    t_client *client;
    int screen;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    screen = ScreenOfRootWindow (event->root);
    
    if (client->vdesk[client->screen] == FIXED_WINDOW)
        client->vdesk[client->screen] = vdesk[screen];
    else
        client->vdesk[client->screen] = FIXED_WINDOW;
    
    XSetWindowBackground (display, client->parent,
                          (client->vdesk[client->screen] == FIXED_WINDOW) ?
                              color_sticky.pixel : color_border1.pixel);
    XClearWindow (display, client->parent);
}

/*
 * action_memo_position: memorize position & size of window
 */

void action_memo_position (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    memorize_client_position (client);
}

/*
 * action_memo_desktop: memorize desktop of window
 */

void action_memo_desktop (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    memorize_client_desktop (client);
}

/*
 * action_forget_position: forget position & size of window
 */

void action_forget_position (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    forget_client_position (client);
}

/*
 * action_forget_desktop: forget desktop of window
 */

void action_forget_desktop (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    forget_client_desktop (client);
}

/*
 * action_win_kill: kills the window
 */

void action_win_kill (XKeyEvent *event)
{
    t_client *client;
    
    if (head_client == NULL)
        return;
    client = client_search (event->window);
    if (client == NULL)
        client = current_client;
    if (client == NULL)
        return;
    
    client_send_wm_delete (client);
}

/*
 * action_reconfigure: reloads WeeWM configuration (config & bindings)
 */

void action_reconfigure (XKeyEvent *event)
{
    /* NOT DEVELOPED ! */
    (void)event;
}

/*
 * action_quit: quit WeeWM (oh no!)
 */

void action_quit (XKeyEvent *event)
{
    (void)event;
    while (head_client)
        client_remove (head_client, 1);
    free_keymap ();
    wee_shutdown ();
}
