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


/* info.c: info bar management (window with various infos) */


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

#include "weewm.h"
#include "info.h"
#include "config.h"
#include "screen.h"


int         info_exists;            /* 1 if info window exists, 0 otherwise */
Window      info_window;            /* window info                          */
char        *info_message;          /* last message displayed               */
GC          info_gc;                /* GC for window info                   */


/*
 * info_init: init info window system
 */

void info_init()
{
    info_exists = 0;
}

/*
 * info_add: adds a message to the info window (creates window if not exist)
 */

void info_add(char *message, int screen)
{
    /* ***** TO DO: modify info window to be transparent
             and automatically destroys himself after a delay ***** */
    
    /*XGCValues               gv;
    XSetWindowAttributes    attr;
    
    if (!info_exists)
    {
        info_exists = 1;
        attr.background_pixmap = None;
        info_window = XCreateWindow(display, RootWindow(display, screen),
            0, DisplayHeight(display, screen) - config_info_height,
            config_info_width, config_info_height,
            0, CopyFromParent, CopyFromParent, CopyFromParent, 0, NULL);
        gv.function = GXinvert;
        gv.subwindow_mode = IncludeInferiors;
        gv.line_width = config_border;
        gv.font = font->fid;
        info_gc = XCreateGC(display, info_window,
            GCFunction | GCSubwindowMode | GCLineWidth | GCFont, &gv);
    }
    else
        XClearWindow(display, info_window);
    XDrawString(display, info_window, info_gc, 0, 20, message, strlen(message));
    XMapWindow(display, info_window);*/
    (void)message;
    (void)screen;
    
    if (info_message)
    {
        XDrawString(display, RootWindow(display, screen),
            invert_gc[screen],
            10, get_y_max(screen)-20, info_message, strlen(info_message));
        free(info_message);
    }
    info_message = malloc(strlen(message)+1);
    strcpy(info_message, message);
    XDrawString(display, RootWindow(display, screen),
        invert_gc[screen],
        10, get_y_max(screen)-20, info_message, strlen(info_message));
}

/*
 * info_destroy: destroys info window
 */

void info_destroy()
{
    if (info_exists)
    {
        info_exists = 0;
        XDestroyWindow(display, info_window);
    }
}
