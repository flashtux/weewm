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


#ifndef __WEEWM_CLIENT_H
#define __WEEWM_CLIENT_H    1

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define NO_DOCK     -1

typedef struct t_client t_client;

struct t_client
{
    t_client    *next;
    Window      window;
    Window      parent;
    XSizeHints  *size;
    char        *application_name;
    int         ignore_unmap;
    int         x, y, width, height;
    int         oldx, oldy, oldw, oldh, oldmx, oldmy, oldmw, oldmh;
    int         border;
    int         vdesk[256];
    int         screen;
    int         dock_position;
};

typedef struct t_motif_wm_hints t_motif_wm_hints;
    
struct t_motif_wm_hints
{
    unsigned long   flags;
    unsigned long   functions;
    unsigned long   decorations;
};

extern int      dock_pos_free;
extern int      dock_size;


extern void set_wm_state(const Window, const Atom, int, long);
extern void send_wm_delete(t_client *);
extern void set_smart_position(t_client *, char);
extern void recalculate_dock_positions();
extern void change_gravity(t_client *, int);
extern void send_config(t_client *);
extern void create_new_client(Window, int);
extern t_client *find_client(Window);
extern t_client *get_first_client_desktop(int, int);
extern void remove_client(t_client *, int);

#endif /* client.h */
