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


/* config.c: WeeWM configuration (options & key bindings) */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "weewm.h"
#include "config.h"
#include "events.h"


/* WeeWM config options */

int         config_border;
int         config_space;
int         config_num_desktops;
int         config_min_width;
int         config_min_height;
int         config_move_offset;
char        *config_color_border1;
char        *config_color_border0;
char        *config_color_sticky;
int         config_info_width;
int         config_info_height;
int         config_dock_use;
int         config_dock_size;
int         config_dock_position;
int         config_dock_align;


t_config_option     weewm_options[] =
/*  name            description
    type               min          max  def.int  def.str    ptr to int             ptr to char */
{ { "border",       "width of border (in pixels)",
    OPTION_TYPE_INT,    1,            3,      1,   NULL,     &config_border,        NULL                       },
  { "space",        "space between window and border (in pixels)",
    OPTION_TYPE_INT,    0,            5,      1,   NULL,     &config_space,         NULL                       },
  { "desktops",     "number of desktops",
    OPTION_TYPE_INT,    1, MAX_DESKTOPS,      5,   NULL,     &config_num_desktops,  NULL                       },
  { "minwidth",     "minimum window width",
    OPTION_TYPE_INT,    1,         4096,     15,   NULL,     &config_min_width,     NULL                       },
  { "minheight",    "minimum window height",
    OPTION_TYPE_INT,    1,         4096,     15,   NULL,     &config_min_height,    NULL                       },
  { "move_offset",  "offset for moving windows (in pixels)",   
    OPTION_TYPE_INT,    1,          256,     16,   NULL,     &config_move_offset,   NULL                       },
  { "color.border.active", "color of active border",
    OPTION_TYPE_STRING, 0,            0,      0,   "green",  NULL,                  &config_color_border1      },
  { "color.border.inactive",  "color of inactive border",
    OPTION_TYPE_STRING, 0,            0,      0,   "black",  NULL,                  &config_color_border0      },
  { "color.sticky", "color of border for sticky windows",
    OPTION_TYPE_STRING, 0,            0,      0,   "blue",   NULL,                  &config_color_sticky       },
  { "info.width",   "width of info window - NOT DEVELOPED!",
    OPTION_TYPE_INT,    10,        4096,    100,   NULL,     &config_info_width,    NULL                       },
  { "info.height",  "height of info window - NOT DEVELOPED!",
    OPTION_TYPE_INT,    10,        4096,     50,   NULL,     &config_info_height,   NULL                       },
  { "dock.use",     "dock activation (1=use, 0=no dock)",
    OPTION_TYPE_INT,    0,            1,      1,   NULL,     &config_dock_use,      NULL                       },
  { "dock.size",    "dock size (in pixels)",
    OPTION_TYPE_INT,    8,          512,     64,   NULL,     &config_dock_size,     NULL                       },
  { "dock.position", "dock position (1=top, 2=left, 3=right, 4=bottom)",
    OPTION_TYPE_INT,    1,            4,      3,   NULL,     &config_dock_position, NULL                       },
  { "dock.align",    "dock alignment (0=top/left, 1=centered, 2=bottom/right)",
    OPTION_TYPE_INT,    0,            2,      0,   NULL,     &config_dock_align,    NULL                       },
  { NULL,            NULL,
    0,                  0,            0,      0,   NULL,     NULL,                  NULL                       } };


/* WeeWM key bindings */

t_keymap    *keymap;
int         mouse_mask;
    
    
t_modifier_list     modifier_list[] =
{ { "shift",   ShiftMask   },
  { "control", ControlMask },
  { "mod1",    Mod1Mask    },
  { "mod2",    Mod2Mask    },
  { "mod3",    Mod3Mask    },
  { "mod4",    Mod4Mask    },
  { "mod5",    Mod5Mask    },
  {  NULL,     0           } };

  
t_handler_list      weewm_commands[] =
{ { "next_win",           action_next_win,              "focus next window"                       },
  { "prev_win",           action_previous_win,          "focus previous window"                   },
  { "pos_top_left",       action_pos_top_left,          "move window to the top left corner"      },
  { "pos_top",            action_pos_top,               "move window to the top of the screen"    },
  { "pos_top_right",      action_pos_top_right,         "move window to the top right corner"     },
  { "pos_left",           action_pos_left,              "move window to the left of the screen"   },
  { "pos_center",         action_pos_center,            "center window on screen"                 },
  { "pos_right",          action_pos_right,             "move window to the right of the screen"  },
  { "pos_bottom_left",    action_pos_bottom_left,       "move window to the bottom left corner"   },
  { "pos_bottom",         action_pos_bottom,            "move window to the bottom of the screen" },
  { "pos_bottom_right",   action_pos_bottom_right,      "move window to the bottom right corner"  },
  { "move_win_left",      action_move_left,             "move window horizontally (-X pixels)"    },
  { "move_win_right",     action_move_right,            "move window horizontally (+X pixels)"    },
  { "move_win_up",        action_move_up,               "move window vertically (-X pixels)"      },
  { "move_win_down",      action_move_down,             "move window vertically (+X pixels)"      },
  { "prev_desktop",       action_previous_desktop,      "go to previous desktop"                  },
  { "next_desktop",       action_next_desktop,          "go to next desktop"                      },
  { "desktop[1-256]",     NULL,                         "go to specified desktop"                 },
  { "send_prev_desktop",  action_send_previous_desktop, "send window to previous desktop"         },
  { "send_next_desktop",  action_send_next_desktop,     "send window to next desktop"             },
  { "win_max_horizontal", action_max_horizontal,        "maximize window horizontally"            },
  { "win_max_vertical",   action_max_vertical,          "maximize window vertically"              },
  { "win_max",            action_maximize,              "maximize window"                         },
  { "win_fix",            action_fix_window,            "fix/unfix window"                        },
  { "win_memo_pos",       action_memo_position,         "memorize position and size of window"    },
  { "win_memo_desktop",   action_memo_desktop,          "memorize desktop of window"              },
  { "win_forget_pos",     action_forget_position,       "forget memorized position for window"    },
  { "win_forget_desktop", action_forget_desktop,        "forget memorized desktop for window"     },
  { "win_kill",           action_win_kill,              "kill current window"                     },
  { "reconfigure",        action_reconfigure,           "reload config - NOT DEVELOPED!"          },
  { "quit",               action_quit,                  "quit WeeWM"                              },
  { NULL,                 NULL,                         NULL                                      } };

  
t_default_keys  weewm_default_keys[] =
{ { "mouse_mask",   "Mod4",   NULL,                 NULL,    0, NULL,   NULL    },
  { "Mod1|Control", "Left",   "prev_desktop",       NULL,    0, NULL,   NULL    },
  { "Mod1|Control", "Right",  "next_desktop",       NULL,    0, NULL,   NULL    },
  { "Mod4",         "Left",   "send_prev_desktop",  NULL,    0, NULL,   NULL    },
  { "Mod4",         "Right",  "send_next_desktop",  NULL,    0, NULL,   NULL    },
  { "Mod4",         "F1",     "desktop_1",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F2",     "desktop_2",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F3",     "desktop_3",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F4",     "desktop_4",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F5",     "desktop_5",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F6",     "desktop_6",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F7",     "desktop_7",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F8",     "desktop_8",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F9",     "desktop_9",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "F10",    "desktop_10",         NULL,    0, NULL,   NULL    },
  { "Mod4",         "F11",    "desktop_11",         NULL,    0, NULL,   NULL    },
  { "Mod4",         "F12",    "desktop_12",         NULL,    0, NULL,   NULL    },
  { "Mod4",         "Home",   "pos_top_left",       NULL,    0, NULL,   NULL    },
  { "Mod4",         "End",    "pos_bottom_left",    NULL,    0, NULL,   NULL    },
  { "Mod4",         "Prior",  "pos_top_right",      NULL,    0, NULL,   NULL    },
  { "Mod4",         "Next",   "pos_bottom_right",   NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "Up",     "pos_top",            NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "Down",   "pos_bottom",         NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "Left",   "pos_left",           NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "Right",  "pos_right",          NULL,    0, NULL,   NULL    },
  { "Mod4",         "i",      "move_win_up",        NULL,    0, NULL,   NULL    },
  { "Mod4",         "k",      "move_win_down",      NULL,    0, NULL,   NULL    },
  { "Mod4",         "j",      "move_win_left",      NULL,    0, NULL,   NULL    },
  { "Mod4",         "l",      "move_win_right",     NULL,    0, NULL,   NULL    },
  { "Mod1",         "Tab",    "next_win",           NULL,    0, NULL,   NULL    },
  { "Mod1|Shift",   "Tab",    "prev_win",           NULL,    0, NULL,   NULL    },
  { "Mod4",         "space",  "win_max",            NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "h",      "win_max_horizontal", NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "v",      "win_max_vertical",   NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "f",      "win_fix",            NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "p",      "win_memo_pos",       NULL,    0, NULL,   NULL    },
  { "Mod4|Control", "d",      "win_memo_desktop",   NULL,    0, NULL,   NULL    },
  { "Mod4|Mod1",    "p",      "win_forget_pos",     NULL,    0, NULL,   NULL    },
  { "Mod4|Mod1",    "d",      "win_forget_desktop", NULL,    0, NULL,   NULL    },
  { "Mod4",         "Escape", "win_kill",           NULL,    0, NULL,   NULL    },
  { "Mod1|Control", "Escape", "quit",               NULL,    0, NULL,   NULL    },
  { "Mod4",         "Return", "run",                "xterm", 0, "7913", "xterm" },
  { NULL,           NULL,     NULL,                 NULL,    0, NULL,   NULL    } };


/*
 * wee_print_options: print all weewm options
 */

void wee_print_options()
{
    int     i;
    
    i = 0;
    while (weewm_options[i].option_name)
    {
        printf("%15s: %s\n", weewm_options[i].option_name, weewm_options[i].description);
        if (weewm_options[i].option_type == OPTION_TYPE_INT)
            printf("                 min: %d, max: %d, default: %d\n",
                weewm_options[i].min, weewm_options[i].max, weewm_options[i].default_int);
        else
            printf("                 default: '%s'\n", weewm_options[i].default_string);
        i++;
    }
}

/*
 * wee_print_commands: print all weewm commands
 */

void wee_print_commands()
{
    int     i;
    
    i = 0;
    while (weewm_commands[i].command_name)
    {
        printf("%20s: %s\n", weewm_commands[i].command_name, weewm_commands[i].description);
        i++;
    }
}

/*
 * explode_string: explode a string according to separators
 */
          
char **explode_string(char *string, char *separators, int num_items_max, int *num_items)
{
    int     i, n_items;
    char    **array;
    char    *ptr, *ptr1, *ptr2;

    if (num_items != NULL)
        *num_items = 0;
    
    n_items = num_items_max;
    
    if (string == NULL)
        return NULL;
    
    if (num_items_max == 0)
    {
        /* calculate number of items */
        ptr = string;
        i = 1;
        while ((ptr = strpbrk(ptr, separators)))
        {
            while (strchr(separators, ptr[0]) != NULL)
                ptr++;
            i++;
        }
        n_items = i;
    }
    
    array = (char **)malloc((num_items_max ? n_items : n_items+1) * sizeof(char *));
    
    ptr1 = string;
    ptr2 = string;
    
    for (i = 0; i < n_items; i++)
    {
        while (strchr(separators, ptr1[0]) != NULL)
            ptr1++;
        if (i == (n_items - 1) || (ptr2 = strpbrk(ptr1, separators)) == NULL)
            if ( (ptr2 = strchr(ptr1, '\n')) == NULL)
                ptr2 = strchr(ptr1, '\0');
        
        if ( (ptr1 == NULL) || (ptr2 == NULL) )
        {
            array[i] = NULL;
        }
        else
        {
            if (ptr2 - ptr1 > 0)
            {
                array[i] = (char *)malloc((ptr2-ptr1+1)*sizeof(char));
                array[i] = strncpy(array[i], ptr1, ptr2-ptr1);
                array[i][ptr2-ptr1] = '\0';
                ptr1 = ++ptr2;
            }
            else
            {
                array[i] = NULL;
            }
        }
    }
    if (num_items_max == 0)
    {
        array[i] = NULL;
        if (num_items != NULL)
            *num_items = i;
    }
    else
    {
        if (num_items != NULL)
            *num_items = num_items_max;
    }
    
    return array;
}

/*
 * config_set_option: sets an option read from config file
 */

void config_set_option(char **items)
{
    int     i;
    
    if (items[1] == NULL)
    {
        fprintf(stderr, "WeeWM: value missing for option '%s', line discarded.\n", items[0]);
        return;
    }
    i = 0;
    while (weewm_options[i].option_name)
    {
        if (strcasecmp(weewm_options[i].option_name, items[0]) == 0)
        {
            /* option found, setting value */
            switch (weewm_options[i].option_type)
            {
                case OPTION_TYPE_INT:
                    *weewm_options[i].ptr_int = atoi(items[1]);
                    if ( (*weewm_options[i].ptr_int < weewm_options[i].min) ||
                        (*weewm_options[i].ptr_int > weewm_options[i].max) )
                    {
                        fprintf(stderr,
                            "WeeWM: option '%s' out of range. Setting it to %d (default value).\n",
                            items[0], weewm_options[i].default_int);
                        *weewm_options[i].ptr_int = weewm_options[i].default_int;
                    }
                    break;
                case OPTION_TYPE_STRING:
                    free(*weewm_options[i].ptr_string);
                    *weewm_options[i].ptr_string = (char *)(malloc((strlen(items[1])+1)*sizeof(char)));
                    strcpy(*weewm_options[i].ptr_string, items[1]);
                    break;
            }
            return;
        }
        i++;
    }
        fprintf(stderr, "WeeWM: unknown option '%s', line discarded.\n", items[0]);
}

/*
 * get_modifier_mask: get modifier mask from a modifier name (with | for logical or)
 */

int get_modifier_mask(char *modifier_name)
{
    int     i, j;
    char    *separators = "|";
    char    **array;
    int     mask;

    array = explode_string(modifier_name, separators, NUM_MODIFIERS, NULL);
    mask = 0;
    for (i = 0; (array[i] != NULL) && (i < NUM_MODIFIERS); i++)
    {
        for (j = 0; modifier_list[j].modifier_name != NULL; j++)
        {
            if (strcasecmp(modifier_list[j].modifier_name, array[i]) == 0 )
                mask |= modifier_list[j].modifier_mask;
        }
    }
    return mask;
}

/*
 * create_new_key_binding: create a new key binding from a config line
 *                         beginning with "key" word
 */

t_keymap *create_new_key_binding(t_keymap *km, int num_items, char **items)
{
    t_keymap    *key;
    int         i;
    
    if ( ( (items[1] == NULL) || (items[2] == NULL) )
        || ( (strncasecmp(items[2], "run", 3) == 0) &&
            ( (items[3] == NULL) || (items[4] == NULL) 
            || (items[5] == NULL) || (items[6] == NULL) ) ) )
    {
        fprintf(stderr, "WeeWM: error parsing config file\n");
        return keymap;
    }
    
    key = (t_keymap *)malloc(sizeof(t_keymap));
    
    /* modifier */
    key->modifier_name = (char *)malloc((strlen(items[0])+1)*sizeof(char));
    strcpy(key->modifier_name, items[0]);
    key->modifier_mask = get_modifier_mask(key->modifier_name);
    
    /* keysym */
    key->key_name = (char *)malloc((strlen(items[1])+1)*sizeof(char));
    strcpy(key->key_name, items[1]);
    key->keysym = (int)XStringToKeysym(key->key_name);
    
    if (strncasecmp(items[2], "run", 3) == 0)
    {
        /* external command */
        
        key->application_name = (char *)malloc((strlen(items[3]) + 1) * sizeof(char));
        strcpy(key->application_name, items[3]);
        
        key->initial_desktop = atoi(items[4]);
        
        key->smart_position = (char *)malloc((strlen(items[5]) + 1) * sizeof(char));
        strcpy(key->smart_position, items[5]);
        
        key->only_one = (strcasecmp(items[2], "run1") == 0) ? 1 : 0;
        
        key->command_file = (char *)malloc((strlen(items[6]) + 1) * sizeof(char));
        strcpy(key->command_file, items[6]);
        
        key->command_args = (char **)malloc((num_items - 7 + 1 + 1) * sizeof(char *));
        
        key->command_args[0] = (char *)malloc((strlen(items[6]) + 1) * sizeof(char));
        strcpy(key->command_args[0], items[6]);
        
        i = 7;
        while (items[i] != NULL)
        {
            key->command_args[i-7+1] = (char *)malloc((strlen(items[i]) + 1) * sizeof(char));
            strcpy(key->command_args[i-7+1], items[i]);
            i++;
        }
        key->command_args[i-7+1] = NULL;
        key->key_switch_desktop = -1;
    }
    else
    {
        /* internal command */
        
        key->initial_desktop = 0;
        key->smart_position = NULL;
        key->only_one = 0;
        key->command_file = NULL;
        key->command_args = NULL;
        key->handler = NULL;
        key->key_switch_desktop = -1;
        i = 0;
        if (strncasecmp(items[2], "desktop_", 8) == 0)
        {
            key->key_switch_desktop = atoi(items[2]+8)-1;
            if ( (key->key_switch_desktop < 0) || (key->key_switch_desktop > MAX_DESKTOPS-1) )
            {
                fprintf(stderr,
                    "WeeWM: invalid desktop number for desktop_xxx command (must be between 1 and %d).\n",
                    MAX_DESKTOPS);
                key->key_switch_desktop = -1;
            }
            key->handler = action_go_to_desktop;
        }
        else
        {
            while (weewm_commands[i].command_name != NULL)
            {
                if (strcasecmp(weewm_commands[i].command_name, items[2]) == 0)
                {
                    key->handler = weewm_commands[i].handler;
                    break;
                }
                i++;
            }
        }
        if (key->handler == NULL)
            fprintf(stderr,
                "WeeWM: internal command '%s' not found, line discarded.\n",
                items[2]);
    }
    key->next = km;
    return key;
}

/*
 * print_options: print options
 */

void print_options()
{
    int     i;
    
    i = 0;
    while (weewm_options[i].option_name)
    {
        printf("option '%s' = ", weewm_options[i].option_name);
        if (weewm_options[i].option_type == OPTION_TYPE_INT)
            printf("%d", *weewm_options[i].ptr_int);
        if (weewm_options[i].option_type == OPTION_TYPE_STRING)
            printf("%s", *weewm_options[i].ptr_string);
        printf("\n");
        i++;
    }
}

/*
 * print_keymap: print keymap
 */

void print_keymap(t_keymap *km)
{
    int     i;
    
    while (km != NULL)
    {
        printf("WeeWM: %s(%d),%s(%d),dsk:%d,smart:%s,cmd:%s, args:",
            km->modifier_name, km->modifier_mask, km->key_name, km->keysym,
            km->initial_desktop, km->smart_position, km->command_file);
        for (i = 0; km->command_args[i]; i++)
            printf("%s ", km->command_args[i]);
        printf(", hnd:%X\n", (unsigned int)km->handler);
        km = km->next;
    }
}

/*
 * wee_read_config: reads WeeWM config file (config or keys bindings)
 */

int wee_read_config(char *filename, int config_type)
{
    FILE    *file;
    char    line[LINE_MAX_LENGTH+1], *ptr_line;
    char    *separators = ":= \t";
    char    **array_items;
    int     i, num_items;
    
    /* initialize keymap when reading key bindings */
    if (config_type == CONFIG_KEYS)
    {
        keymap = NULL;
        mouse_mask = 0;
    }
    
    /* initialize variables when reading options file */
    if (config_type == CONFIG_OPTIONS)
    {
        i = 0;
        while (weewm_options[i].option_name)
        {
            if (weewm_options[i].option_type == OPTION_TYPE_INT)
                *weewm_options[i].ptr_int = weewm_options[i].default_int;
            if (weewm_options[i].option_type == OPTION_TYPE_STRING)
            {
                *weewm_options[i].ptr_string =
                    (char *)malloc((strlen(weewm_options[i].default_string)+1)*(sizeof(char)));
                strcpy(*weewm_options[i].ptr_string, weewm_options[i].default_string);
            }
            i++;
        }
    }
    
    /* open config/keys file */
    if ( (file = fopen(filename, "rt")) == NULL)
    {
        if ( ( (config_type == CONFIG_OPTIONS) && (default_config_name) ) ||
            ( (config_type == CONFIG_KEYS) && (default_keys_name) ) )
            return -1;
        else
        {
            fprintf(stderr, "WeeWM: Fatal: config file '%s' not found !\n", filename);
            exit(1);
        }
    }
    
    /* read each line of the file */
    while (!feof(file))
    {
        ptr_line = fgets(line, LINE_MAX_LENGTH, file);
        if (ptr_line)
        {
            /* skip spaces */
            while (ptr_line[0] == ' ')
                ptr_line++;
            
            /* not a comment and not an empty line */
            if ( (ptr_line[0] != '#') && (ptr_line[0] != '\n') )
            {
                array_items = explode_string(ptr_line, separators, 0, &num_items);
                
                /* at least one item found */
                if (array_items != NULL)
                {
                    if (config_type == CONFIG_OPTIONS)
                        config_set_option(array_items);
                    if (config_type == CONFIG_KEYS)
                    {
                        if (strcasecmp(array_items[0], "mouse_mask") == 0)
                            mouse_mask = get_modifier_mask(array_items[1]);
                        else
                            keymap = create_new_key_binding(keymap, num_items, array_items);
                    }
                }
                i = 0;
                while (array_items[i])
                {
                    free(array_items[i]);
                    i++;
                }
                free(array_items);
            }
        }
    }
    #ifdef DEBUG
    /*if (config_type == CONFIG_OPTIONS)
        print_options();
    if (config_type == CONFIG_KEYS)
        print_keymap(keymap);*/
    #endif
    fclose(file);
    return 0;
}

/*
 * wee_create_default_config: create default config (config.rc or keys.rc)
 */

void wee_create_default_config(char *filename, int config_type)
{
    char    line[1024];
    FILE    *file;
    int     i;
    
    if ( (file = fopen(filename, "wt")) == NULL)
    {
        fprintf(stderr, "WeeWM: unable to create file '%s'.\n", filename);
        exit(1);
    }
    
    switch (config_type)
    {
        case CONFIG_OPTIONS:
            fputs("# WeeWM configuration\n", file);
            fputs("# See README for syntax.\n\n", file);
            fputs("# ===== WeeWM configuration =====\n\n", file);
            i = 0;
            while (weewm_options[i].option_name)
            {
                sprintf(line, "# %s\n", weewm_options[i].description);
                fputs(line, file);
                if (weewm_options[i].option_type == OPTION_TYPE_INT)
                    sprintf(line, "%-20s %d\n",
                        weewm_options[i].option_name,
                        weewm_options[i].default_int);
                if (weewm_options[i].option_type == OPTION_TYPE_STRING)
                    sprintf(line, "%-20s %s\n",
                        weewm_options[i].option_name,
                        weewm_options[i].default_string);
                fputs(line, file);
                fputs("\n", file);
                i++;
            }
            break;
        case CONFIG_KEYS:
            fputs("# WeeWM key bindings\n", file);
            fputs("# See README for syntax.\n\n", file);
            fputs("# ===== WeeWM key bindings =====\n\n", file);
            i = 0;
            while (weewm_default_keys[i].key_name)
            {
                if (weewm_default_keys[i].application_name)
                    sprintf(line, "%-25s %-10s %-5s %s %d %s %s\n",
                        weewm_default_keys[i].modifier_name,
                        weewm_default_keys[i].key_name,
                        weewm_default_keys[i].command,
                        weewm_default_keys[i].application_name,
                        weewm_default_keys[i].initial_desktop,
                        weewm_default_keys[i].smart_position,
                        weewm_default_keys[i].command_file_and_args);
                else
                {
                    if (weewm_default_keys[i].command)
                        sprintf(line, "%-25s %-10s %s\n",
                            weewm_default_keys[i].modifier_name,
                            weewm_default_keys[i].key_name,
                            weewm_default_keys[i].command);
                    else
                        sprintf(line, "%-25s %-10s\n",
                            weewm_default_keys[i].modifier_name,
                            weewm_default_keys[i].key_name);
                }
                fputs(line, file);
                i++;
            }
            break;
    }
    fclose(file);
}

/*
 * free_keymap: free the allocated keymap
 */

void free_keymap()
{
    t_keymap    *ptr_next_keymap;
    int         i;
    
    while (keymap)
    {
        if (keymap->modifier_name)
            free(keymap->modifier_name);
        if (keymap->key_name)
            free(keymap->key_name);
        if (keymap->smart_position)
            free(keymap->smart_position);
        if (keymap->application_name)
            free(keymap->application_name);
        if (keymap->command_file)
            free(keymap->command_file);
        if (keymap->command_args)
        {
            i = 0;
            while (keymap->command_args[i])
            {
                free(keymap->command_args[i]);
                i++;
            }
            free(keymap->command_args);
        }
        ptr_next_keymap = keymap->next;
        free(keymap);
        keymap = ptr_next_keymap;
    }
}
