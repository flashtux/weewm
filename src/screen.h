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


#ifndef __WEEWM_SCREEN_H
#define __WEEWM_SCREEN_H 1

#include <X11/Xlib.h>

extern int ScreenOfWindow (Window);
extern int ScreenOfRootWindow (Window);
extern int get_x_min (int);
extern int get_x_max (int);
extern int get_y_min (int);
extern int get_y_max (int);
extern void get_mouse_position (int *, int *, int);
extern void switch_desktop (int, int);

#endif /* screen.h */
