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


#ifndef __WEEWM_CLIENT_H
#define __WEEWM_CLIENT_H 1

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define NO_DOCK -1

typedef struct t_client t_client;

struct t_client
{
    t_client *next;
    Window window;
    Window parent;
    XSizeHints *size;
    char *application_name;
    int ignore_unmap;
    int x, y, width, height;
    int oldx, oldy, oldw, oldh;
    int oldmx, oldmy, oldmw, oldmh;
    int border;
    int vdesk[256];
    int screen;
    int dock_position;
};

typedef struct t_motif_wm_hints t_motif_wm_hints;
    
struct t_motif_wm_hints
{
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
};

extern t_client *head_client;
extern t_client *current_client;
extern int dock_pos_free;
extern int dock_size;

extern void client_new (Window, int);
extern void set_wm_state (const Window, const Atom, int, long);
extern void client_send_wm_delete (t_client *);
extern void client_set_smart_position (t_client *, char);
extern void client_recalculate_dock_positions ();
extern void client_change_gravity (t_client *, int);
extern void client_send_config (t_client *);
extern void client_draw_outline (t_client *);
extern void client_hide (t_client *);
extern void client_unhide (t_client *, int);
extern void client_recalculate_sweep (t_client *, int, int, int, int);
extern void client_drag (t_client *);
extern void client_move (t_client *, int);
extern void client_resize (t_client *, int);
extern t_client *client_search (Window);
extern t_client *client_get_first_for_desktop (int, int);
extern void client_remove (t_client *, int);

#endif /* client.h */
