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


#ifndef __WEEWM_CONFIG_H
#define __WEEWM_CONFIG_H 1

#define LINE_MAX_LENGTH     1024
#define NUM_MODIFIERS       7

#define CONFIG_OPTIONS      1
#define CONFIG_KEYS         2

#define OPTION_TYPE_INT     1
#define OPTION_TYPE_STRING  2

typedef struct t_modifier_list t_modifier_list;

struct t_modifier_list
{
    char *modifier_name;
    int modifier_mask;
};

typedef struct t_handler_list t_handler_list;

struct t_handler_list
{
    char *command_name;
    void (*handler)(XKeyEvent *event);
    char *description;
};

typedef struct t_keymap t_keymap;

struct t_keymap
{
    char *modifier_name;
    int modifier_mask;
    char *key_name;
    int keysym;
    int initial_desktop;
    char *smart_position;
    char *application_name;
    int only_one;
    char *command_file;
    char **command_args;
    void (*handler)(XKeyEvent *event);
    int key_switch_desktop;
    t_keymap *next;
};

typedef struct t_default_keys t_default_keys;

struct t_default_keys
{
    char *modifier_name;
    char *key_name;
    char *command;
    char *application_name;
    int initial_desktop;
    char *smart_position;
    char *command_file_and_args;
};

typedef struct t_config_option t_config_option;

struct t_config_option
{
    char *option_name;
    char *description;
    int option_type;
    int min, max;
    int default_int;
    char *default_string;
    int *ptr_int;
    char **ptr_string;
};

extern int config_border;
extern int config_space;
extern int config_num_desktops;
extern int config_min_width;
extern int config_min_height;
extern int config_move_offset;
extern char *config_color_border1;
extern char *config_color_border0;
extern char *config_color_sticky;
extern int config_info_width;
extern int config_info_height;
extern int config_dock_use;
extern int config_dock_size;
extern int config_dock_position;
extern int config_dock_align;

extern t_keymap *keymap;
extern int mouse_mask;

extern void wee_print_options ();
extern void wee_print_commands ();
extern char **explode_string (char *, char *, int, int *);
extern int wee_read_config (char *, int);
extern void wee_create_default_config (char *, int);
extern void free_keymap ();

#endif /* config.h */
