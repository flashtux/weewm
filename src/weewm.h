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


#ifndef __WEEWM_H
#define __WEEWM_H   1

#include <X11/Xlib.h>
#include "client.h"

/* #define DEBUG */

#define VERSION     "0.0.2"

#define WEE_LICENCE \
    "WeeWM " VERSION " (c) Copyright 2003 by FlashCode <flashcode@flashtux.org>\n" \
    "                                  Xahlexx  <xahlexx@tuxfamily.org>\n" \
    "                                  Bounga <bounga@altern.org>\n\n" \
    "This program is free software; you can redistribute it and/or modify\n" \
    "it under the terms of the GNU General Public License as published by\n" \
    "the Free Software Foundation; either version 2 of the License, or\n" \
    "(at your option) any later version.\n" \
    "\n", \
    \
    "This program is distributed in the hope that it will be useful,\n" \
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
    "GNU General Public License for more details.\n" \
    "\n" \
    "You should have received a copy of the GNU General Public License\n" \
    "along with this program; if not, write to the Free Software\n" \
    "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\n" \
    "Portions of the code were based on cwwm, which contains this license:\n\n", \
    \
    "* CWWM LICENSE :\n\n" \
    "cwwm-1.2.0\n" \
    "Copyright (C) 1999-2002 ChezWam <dev@cwwm.chezwam.org>\n\n" \
    "Developpers:\n\n" \
    "Sebastien GROSS: <seb@chezwam.org>\n" \
    "Dimitri FONTAINE: <dim@tapoueh.org>\n" \
    "Julien DUCROS: <jul@chezwam.org>\n\n" \
    "THIS SOFTWARE IS PROVIDED BY THE AUTHOR \"AS IS\", WITHOUT ANY EXPRESS\n" \
    "OR IMPLIED WARRANTIES OF ANY KIND. IN NO EVENT SHALL THE AUTHOR BE\n" \
    "HELD LIABLE FOR ANY DAMAGES CONNECTED WITH THE USE OF THIS PROGRAM.\n\n", \
    \
    "You are granted permission to copy, publish, distribute, and/or sell\n" \
    "copies of this program and any modified versions or derived works,\n" \
    "provided that this copyright and notice are not removed or altered.\n\n" \
    "Portions of the code were based on evilwm, which contains this license:\n\n", \
    \
    "* EVILWM LICENCE :\n\n" \
    "evilwm-0.3\n" \
    "Copyright (C) 1999-2002  Ciaran Anscomb <evilwm@6809.org.uk>\n\n" \
    "This is free software.  You can do what you want to it, but if it\n" \
    "breaks something, you get to pay for the counselling.  The code was\n" \
    "originally based on aewm, so this is distributed under the same terms,\n" \
    "which follow.\n\n", \
    \
    "* AEWM LICENCE :\n\n" \
    "Copyright (c) 1998-2000 Decklin Foster.\n\n" \
    "THIS SOFTWARE IS PROVIDED BY THE AUTHOR \"AS IS\", WITHOUT ANY EXPRESS\n" \
    "OR IMPLIED WARRANTIES OF ANY KIND. IN NO EVENT SHALL THE AUTHOR BE\n" \
    "HELD LIABLE FOR ANY DAMAGES CONNECTED WITH THE USE OF THIS PROGRAM.\n\n", \
    \
    "You are granted permission to copy, publish, distribute, and/or sell\n" \
    "copies of this program and any modified versions or derived works,\n" \
    "provided that this copyright and notice are not removed or altered.\n\n" \
    "Portions of the code were based on 9wm, which contains this license:\n\n", \
    \
    "> 9wm is free software, and is Copyright (c) 1994 by David Hogan.\n" \
    "> Permission is granted to all sentient beings to use this software,\n" \
    "> to make copies of it, and to distribute those copies, provided\n" \
    "> that:\n" \
    ">\n" \
    ">     (1) the copyright and licence notices are left intact\n" \
    ">     (2) the recipients are aware that it is free software\n" \
    ">     (3) any unapproved changes in functionality are either\n" \
    ">           (i) only distributed as patches\n", \
    \
    ">       or (ii) distributed as a new program which is not called 9wm\n" \
    ">               and whose documentation gives credit where it is due\n" \
    ">     (4) the author is not held responsible for any defects\n" \
    ">         or shortcomings in the software, or damages caused by it.\n" \
    ">\n" \
    "> There is no warranty for this software.  Have a nice day.\n\n"
    


#define WEE_USAGE \
    "WeeWM " VERSION " (c) Copyright 2003 by FlashCode <flashcode@flashtux.org>\n" \
    "                                  Xahlexx  <xahlexx@tuxfamily.org>\n" \
    "                                  Bounga <bounga@altern.org>\n\n" \
    "  -c, --config        config file to load (default: $HOME/.weewm/config.rc)\n" \
    "  -d, --display       choose X display\n" \
    "  -f, --font          font name\n" \
    "  -h, --help          this help screen\n", \
    "  -k, --keys          key bindings file to load (default: $HOME/.weewm/keys.rc)\n" \
    "  -l, --licence       display program licence\n" \
    "  -m, --list-commands display WeeWM commands (for keys.rc)\n" \
    "  -o, --list-options  display WeeWM options (for config.rc)\n" \
    "  -v, --version       display WeeWM version\n\n"


#define DEFAULT_DISPLAY         ":0"
#define DEFAULT_FONT            "variable"

#define NUM_IGNORE_MODIFIERS    8

#define MAX_DESKTOPS            256

#define FIXED_WINDOW            -1

#define DOCK_TOP                1
#define DOCK_LEFT               2
#define DOCK_RIGHT              3
#define DOCK_BOTTOM             4

#define DOCK_ALIGN_TOP_LEFT     0
#define DOCK_ALIGN_CENTER       1
#define DOCK_ALIGN_BOTTOM_RIGHT 2


extern char         *display_name;
extern char         *font_name;

extern Display      *display;
extern XFontStruct  *font;
extern int          num_heads;
extern t_client     *head_client;
extern t_client     *current_client;
extern int          vdesk[256];
extern GC           invert_gc[256];
extern Atom         atom_wm_state;
extern Atom         atom_wm_change_state;
extern Atom         atom_wm_protos;
extern Atom         atom_wm_delete;
extern Atom         atom_wm_cmapwins;
extern Atom         atom_motif_wm_hints;
extern XColor       color_border1;
extern XColor       color_border0;
extern XColor       color_sticky;
extern Cursor       cursor_arrow;
extern Cursor       cursor_move;
extern Cursor       cursor_resize;

extern int          NumLockMask, CapsLockMask, ScrollLockMask;
extern int          ignore_modifiers[NUM_IGNORE_MODIFIERS];

extern int          default_config_name;
extern int          default_keys_name;

extern char         *memo_file;


extern int handle_xerror(Display *, XErrorEvent *);
extern int ignore_xerror(Display *, XErrorEvent *);


extern void wee_shutdown();

#endif /* weewm.h */
