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


#ifndef __WEEWM_SCREEN_H
#define __WEEWM_SCREEN_H    1

#include <X11/Xlib.h>
#include "client.h"

extern int ScreenOfWindow(Window);
extern int ScreenOfRootWindow(Window);
extern int get_x_min(int);
extern int get_x_max(int);
extern int get_y_min(int);
extern int get_y_max(int);
extern void draw_outline(t_client *);
extern void get_mouse_position(int *, int *, int);
extern void hide(t_client *);
extern void unhide(t_client *, int);
extern void recalculate_sweep(t_client *, int, int, int, int);

extern void client_drag(t_client *);
extern void client_move(t_client *, int);
extern void client_resize(t_client *, int);
extern void switch_desktop(int, int);

#endif /* screen.h */
