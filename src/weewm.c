/* ############################################################################
 * ###           ___       __            ___       ________  ___            ###
 * ###           __ |     / /_____ _____ __ |     / /___   |/  /            ###
 * ###           __ | /| / / _  _ \_  _ \__ | /| / / __  /|_/ /             ###
 * ###           __ |/ |/ /  /  __//  __/__ |/ |/ /  _  /  / /              ###
 * ###           ____/|__/   \___/ \___/ ____/|__/   /_/  /_/               ###
 * ###                                                                      ###
 * ###                  WeeWM - Wee Enhanced Environment                    ###
 * ###            Fast & light window manager manager for XFree             ###
 * ###                                                                      ###
 * ###                By: FlashCode <flashcode@flashtux.org>                ###
 * ###                    Xahlexx <xahlexx@tuxisland.org>                   ###
 * ###                    Bounga <bounga@altern.org>                        ###
 * ###                                                                      ###
 * ###                        http://www.weewm.org                          ###
 * ###                                                                      ###
 * ############################################################################
 *
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


/* weewm.c: core functions for WeeWM */


#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include "weewm.h"
#include "config.h"
#include "client.h"
#include "events.h"
#include "info.h"
#include "memorize.h"


/* commande line parameters or readed from config file */

char            *display_name;
char            *font_name;
char            *config_file;
char            *keys_file;

/* other variables */

Display         *display;               /* display                      */
XFontStruct     *font;                  /* font                         */
int             num_heads;              /* number of heads              */
t_client        *head_client;           /* first client                 */
t_client        *current_client;        /* current client               */
int             vdesk[256];             /* virtual desktops             */
GC              invert_gc[256];         /* allocated gc                 */
Atom		    atom_wm_state;          /* atom wm_state                */
Atom		    atom_wm_change_state;   /* atom wm_change_state         */
Atom		    atom_wm_protos;         /* atom wm_protos               */
Atom		    atom_wm_delete;         /* atom wm_delete               */
Atom		    atom_wm_cmapwins;       /* atom wm_cmapwins             */
Atom            atom_motif_wm_hints;    /* atom wm_hints                */
XColor          color_border1;          /* color of active border       */
XColor          color_border0;          /* color of inactive border     */
XColor          color_sticky;           /* color of sticky windows      */
Cursor          cursor_arrow;           /* cursor when idle (arrow)     */
Cursor          cursor_move;            /* cursor when moving window    */
Cursor          cursor_resize;          /* cursor when resizing window  */

int             NumLockMask, CapsLockMask, ScrollLockMask;
int             ignore_modifiers[NUM_IGNORE_MODIFIERS];

int             default_config_name;    /* 1 if ~/.weewm/config.rc used */
int             default_keys_name;      /* 1 if ~/.weewm/keys.rc   used */

char            *memo_file;             /* file with memorized windows  */

/*
 * wee_parse_args: parse command line args
 */

void wee_parse_args(int argc, char *argv[])
{
    int     i;
    
    display_name = NULL;
    font_name = NULL;
    config_file = NULL;
    keys_file = NULL;
    for (i = 1; i < argc; i++)
    {
        if ( (strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--config") == 0) )
        {
            if (i == argc-1)
                fprintf(stderr, "WeeWM: warning: no config file specified (parameter '%s'), ignored\n", argv[i]);
            else
            {
                config_file = argv[i+1];
                i++;
            }
        }
        else if ( (strcmp(argv[i], "-k") == 0) || (strcmp(argv[i], "--keys") == 0) )
        {
            if (i == argc-1)
                fprintf(stderr, "WeeWM: warning: no keys file specified (parameter '%s'), ignored\n", argv[i]);
            else
            {
                keys_file = argv[i+1];
                i++;
            }
        }
        else if ( (strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--font") == 0) )
        {
            if (i == argc-1)
                fprintf(stderr, "WeeWM: warning: no font specified (parameter '%s'), ignored\n", argv[i]);
            else
            {
                font_name = argv[i+1];
                i++;
            }
        }
        else if ( (strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0) )
        {
            printf("\n%s%s", WEE_USAGE);
            exit(0);
        }
        else if ( (strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--licence") == 0) )
        {
            printf("\n%s%s%s%s%s%s%s%s%s", WEE_LICENCE);
            exit(0);
        }
        else if ( (strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--list-commands") == 0) )
        {
            wee_print_commands();
            exit(0);
        }
        else if ( (strcmp(argv[i], "-o") == 0) || (strcmp(argv[i], "--list-options") == 0) )
        {
            wee_print_options();
            exit(0);
        }
        else if ( (strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0) )
        {
            printf("WeeWM " VERSION "\n");
            exit(0);
        }
        else if ( (strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--display") == 0) )
        {
            if (i == argc-1)
                fprintf(stderr, "WeeWM: warning: no display specified (parameter '%s'), ignored\n", argv[i]);
            else
            {
                display_name = argv[i+1];
                i++;
            }
        }
        else
        {
            fprintf(stderr, "WeeWM: warning: unknown parameter '%s', ignored\n", argv[i]);
        }
    }
    
    if (config_file == NULL)
    {
        config_file = (char *)malloc((strlen(getenv("HOME"))+64)*sizeof(char));
        sprintf(config_file, "%s/.weewm/config.rc", getenv("HOME"));
        default_config_name = 1;
    }
    else
        default_config_name = 0;
    
    if (keys_file == NULL)
    {
        keys_file = (char *)malloc((strlen(getenv("HOME"))+64)*sizeof(char));
        sprintf(keys_file, "%s/.weewm/keys.rc", getenv("HOME"));
        default_keys_name = 1;
    }
    else
        default_keys_name = 0;
    
    memo_file = (char *)malloc((strlen(getenv("HOME"))+64)*sizeof(char));
    sprintf(memo_file, "%s/.weewm/memo_app", getenv("HOME"));
}

/*
 * wee_create_home_dir: create weewm home directory
 */

void wee_create_home_dir()
{
    char    *weewm_home_dir;
    int     return_code;
    
    weewm_home_dir = (char *)malloc((strlen(getenv("HOME"))+64)*sizeof(char));
    sprintf(weewm_home_dir, "%s/.weewm", getenv("HOME"));
    return_code = mkdir(weewm_home_dir, 0755);
    if (return_code < 0)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "WeeWM: can't create directory '%s'.\n", weewm_home_dir);
            free(weewm_home_dir);
            exit(1);
        }
    }
    free(weewm_home_dir);
}

/*
 * handle_xerror: handles an X error
 */

int handle_xerror(Display *display, XErrorEvent *event)
{
    t_client    *client;
    
    client = find_client(event->resourceid);
  
    /* prevent compiler warning */
    (void)display;
  
    if (event->error_code == BadAccess && event->request_code == X_ChangeWindowAttributes)
    {
        fprintf(stderr, "WeeWM: root window unavailable (maybe another wm is running ?)\n");
        exit(1);
    }
    if (client)
    {
        remove_client(client, 0);
    }
    return 0;
}

/*
 * ignore_xerror: function called when X errors should be ignored
 */

int ignore_xerror(Display *display, XErrorEvent *event)
{
    /* prevent compiler warning */
    (void)display;
    (void)event;
    return 0;
}

/*
 * wee_init_display: init display for WeeWM
 */

void wee_init_display()
{
    Window                  root, parent, *children;
    int                     i, k;
    unsigned int            j, nchildren;
    XWindowAttributes       attr;
    XSetWindowAttributes    sattr;
    XGCValues               gv;
    XColor                  dummy_color;
    t_keymap                *ptr_keymap;
    XModifierKeymap         *xmk;
    KeyCode                 *map;
    
    /* init display */
    display = XOpenDisplay(display_name);
    if (!display)
    {
        fprintf(stderr, "WeeWM: can't open display '%s'\n", display_name);
        exit(1);
    }
    
    XSetErrorHandler(handle_xerror);
    
    atom_wm_state = XInternAtom(display, "WM_STATE", False);
    atom_wm_change_state = XInternAtom(display, "WM_CHANGE_STATE", False);
    atom_wm_protos = XInternAtom(display, "WM_PROTOCOLS", False);
    atom_wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
    atom_motif_wm_hints = XInternAtom(display, "_MOTIF_WM_HINTS", False);
    
    /* load font */
    font = NULL;
    if (font_name)
        font = XLoadQueryFont(display, font_name);
    if (!font)
        font = XLoadQueryFont(display, DEFAULT_FONT);
    if (!font)
    {
        fprintf(stderr, "error loading font\n");
        exit(1);
    }
    
    sattr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | PropertyChangeMask | ButtonPressMask | ButtonReleaseMask;
    
    gv.function = GXinvert;
    gv.subwindow_mode = IncludeInferiors;
    gv.line_width = config_border;
    gv.font = font->fid;
    
    dock_pos_free = 0;
    dock_size = 0;
    
    /* create cursors */
    cursor_arrow = XCreateFontCursor(display, XC_left_ptr);
    cursor_move = XCreateFontCursor(display, XC_hand2);
    cursor_resize = XCreateFontCursor(display, XC_plus);
    
    num_heads = ScreenCount(display);
    for (i = 0; i < num_heads; i++)
    {
        XQueryTree(display, RootWindow(display, i), &root, &parent, &children, &nchildren);
        for (j = 0; j < nchildren; j++)
        {
            XGetWindowAttributes(display, children[j], &attr);
            if (!attr.override_redirect && attr.map_state == IsViewable)
                create_new_client(children[j], i);
        }
        XFree(children);
        XChangeWindowAttributes(display, RootWindow(display, i), CWEventMask, &sattr);
        invert_gc[i] = XCreateGC(display, RootWindow(display, i),
            GCFunction | GCSubwindowMode | GCLineWidth | GCFont, &gv);
        
        XDefineCursor(display, RootWindow(display, i), cursor_arrow);
        
        XAllocNamedColor(display, DefaultColormap(display, i),
            config_color_border1, &color_border1, &dummy_color);
        XAllocNamedColor(display, DefaultColormap(display, i),
            config_color_border0, &color_border0, &dummy_color);
        XAllocNamedColor(display, DefaultColormap(display, i),
            config_color_sticky, &color_sticky, &dummy_color);
        
        xmk = XGetModifierMapping(display);
        if (xmk)
        {
            map = xmk->modifiermap;
            for (j = 0; j < 8; j++)
                for (k = 0; k < xmk->max_keypermod; k++, map++)
                {
                    if (*map == XKeysymToKeycode(display, XK_Num_Lock))
                        NumLockMask = (1 << j);
                    if (*map == XKeysymToKeycode (display, XK_Caps_Lock))
                        CapsLockMask = (1 << j);
                    if (*map == XKeysymToKeycode (display, XK_Scroll_Lock))
                        ScrollLockMask = (1 << j);
                }
            XFreeModifiermap(xmk);
        }
        
        ignore_modifiers[0] = 0;
        ignore_modifiers[1] = CapsLockMask;
        ignore_modifiers[2] = NumLockMask;
        ignore_modifiers[3] = ScrollLockMask;
        ignore_modifiers[4] = CapsLockMask | NumLockMask;
        ignore_modifiers[5] = CapsLockMask | ScrollLockMask;
        ignore_modifiers[6] = NumLockMask | ScrollLockMask;
        ignore_modifiers[7] = CapsLockMask | NumLockMask | ScrollLockMask;
        
        for (ptr_keymap = keymap; ptr_keymap != NULL; ptr_keymap = ptr_keymap->next)
        {
            for (j = 0; j < NUM_IGNORE_MODIFIERS; j++)
            {
                XGrabKey(display, XKeysymToKeycode(display, ptr_keymap->keysym),
                    ignore_modifiers[j] | ptr_keymap->modifier_mask,
                    RootWindow(display, i), True, GrabModeAsync, GrabModeAsync);
            }
        }
    }
}

/*
 * wee_main_loop: WeeWM main loop
 */

void wee_main_loop()
{
    XEvent  event;
  
    for (;;)
    {
        XNextEvent(display, &event);
        switch (event.type)
        {
            case KeyPress:
                handle_key_event(&event.xkey);
                break;
            case ButtonPress:
                handle_button_event(&event.xbutton);
                break;
            case ConfigureRequest:
                handle_configure_request(&event.xconfigurerequest);
                break;
            case MapRequest:
                handle_map_request(&event.xmaprequest); 
                break;
            case EnterNotify:
                handle_enter_event(&event.xcrossing);
                break;
            case PropertyNotify:
                handle_property_change(&event.xproperty); 
                break;
            case UnmapNotify:
                handle_unmap_event(&event.xunmap);
                break;
            /* case Expose:
                handle_expose_event(&event.xexpose);
                break; */
        }
    }
}

/*
 * wee_shutdown: shut down WeeWM
 */

void wee_shutdown()
{
    XCloseDisplay(display);
    printf("WeeWM: quit request, exiting.\n");
    exit(0);
}

/*
 * WeeWM entry point
 */

int main(int argc, char *argv[])
{
    /* parse command line args */
    wee_parse_args(argc, argv);
    
    /* create weewm home directory */
    wee_create_home_dir();
    
    /* read config & keys files */
    if (wee_read_config(config_file, CONFIG_OPTIONS) < 0)
    {
        wee_create_default_config(config_file, CONFIG_OPTIONS);
        wee_read_config(config_file, CONFIG_OPTIONS);
    }
    if (wee_read_config(keys_file, CONFIG_KEYS) < 0)
    {
        wee_create_default_config(keys_file, CONFIG_KEYS);
        wee_read_config(keys_file, CONFIG_KEYS);
    }
    
    /* read memorized windows */
    memo_init();
    read_memo_file(memo_file);
    
    /* init display & setup environment */
    wee_init_display();
    info_init();
    
    /* WeeWM main loop (wait for events) */
    wee_main_loop();
    
    /* statement never executed */
    return 0;
}
