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


#ifndef __WEEWM_EVENT_H
#define __WEEWM_EVENT_H 1

#include <X11/Xlib.h>

/* t_zone and t_free_zone structs: used for smart_maximize action */

typedef struct t_zone t_zone;
    
struct t_zone
{
    int x1, y1, x2, y2;
};

typedef struct t_free_zone t_free_zone;
    
struct t_free_zone
{
    int x1, y1, x2, y2;
    char used;
    t_free_zone *next;
};

extern void handle_key_event (XKeyEvent *);
extern void handle_button_event (XButtonEvent *);
extern void handle_configure_request (XConfigureRequestEvent *);
extern void handle_map_request (XMapRequestEvent *e);
extern void handle_unmap_event (XUnmapEvent *);
extern void handle_property_change (XPropertyEvent *);
extern void handle_enter_event (XCrossingEvent *);

extern void action_next_win (XKeyEvent *);
extern void action_previous_win (XKeyEvent *);
extern void action_pos_top_left (XKeyEvent *);
extern void action_pos_top (XKeyEvent *);
extern void action_pos_top_right (XKeyEvent *);
extern void action_pos_left (XKeyEvent *);
extern void action_pos_center (XKeyEvent *);
extern void action_pos_right (XKeyEvent *);
extern void action_pos_bottom_left (XKeyEvent *);
extern void action_pos_bottom (XKeyEvent *);
extern void action_pos_bottom_right (XKeyEvent *);
extern void action_move_left (XKeyEvent *);
extern void action_move_right (XKeyEvent *);
extern void action_move_up (XKeyEvent *);
extern void action_move_down (XKeyEvent *);
extern void action_previous_desktop (XKeyEvent *);
extern void action_next_desktop (XKeyEvent *);
extern void action_go_to_desktop (XKeyEvent *);
extern void action_send_previous_desktop (XKeyEvent *);
extern void action_send_next_desktop (XKeyEvent *);
extern void action_max_horizontal (XKeyEvent *);
extern void action_max_vertical (XKeyEvent *);
extern void action_maximize (XKeyEvent *);
extern void action_smart_maximize (XKeyEvent *);
extern void action_fix_window (XKeyEvent *);
extern void action_memo_position (XKeyEvent *);
extern void action_memo_desktop (XKeyEvent *);
extern void action_forget_position (XKeyEvent *);
extern void action_forget_desktop (XKeyEvent *);
extern void action_win_kill (XKeyEvent *);
extern void action_reconfigure (XKeyEvent *);
extern void action_quit (XKeyEvent *);

#endif /* events.h */
