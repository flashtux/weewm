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


/* memorize.c: memorize windows position and/or desktop */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "weewm.h"
#include "memorize.h"
#include "config.h"


t_memo_win  *memo_win;          /* memorized windows        */


/*
 * memo_init: initialize memorization stuff
 */

void memo_init()
{
    memo_win = NULL;
}

/*
 * read_memo_file: read file with memorized clients
 */

int read_memo_file(char *filename)
{
    FILE        *file;
    char        **array_items;
    char        *separators = " \t";
    char        line[LINE_MAX_LENGTH+1], appli_name[LINE_MAX_LENGTH+1];
    char        *ptr_line, *pos;
    int         num_items;
    t_memo_win  *new_memo_win;
    int         i;
    
    
    if ( (file = fopen(filename, "rt")) == NULL)
        return -1;
    
    while (!feof(file))
    {
        ptr_line = fgets(line, LINE_MAX_LENGTH, file);
        if (ptr_line)
        {
            while (ptr_line[0] == ' ')
                ptr_line++;
            
            /* not a comment and not an empty line */
            if ( (ptr_line[0] != '#') && (ptr_line[0] != '\n') )
            {
                if (strncmp(ptr_line, APPLI_LINE_BEGIN, strlen(APPLI_LINE_BEGIN)) == 0)
                {
                    strcpy(appli_name, ptr_line + strlen(APPLI_LINE_BEGIN));
                    pos = strchr(appli_name, '\n');
                    if (pos)
                        pos[0] = '\0';
                }
                else
                {
                    array_items = explode_string(ptr_line, separators, 0,
                        &num_items);
                    if (num_items != 5)
                        fprintf(stderr,
                            "WeeWM: line ignored from '%s' (wrong arguments)\n",
                            filename);
                    else
                    {
                        new_memo_win = (t_memo_win *)malloc(sizeof(t_memo_win));
                        new_memo_win->next = memo_win;
                        memo_win = new_memo_win;
                        new_memo_win->application_name =
                            (char *)malloc((strlen(appli_name)+1)*(sizeof(char)));
                        strcpy(new_memo_win->application_name, appli_name);
                        new_memo_win->x = atoi(array_items[0]);
                        new_memo_win->y = atoi(array_items[1]);
                        new_memo_win->width = atoi(array_items[2]);
                        new_memo_win->height = atoi(array_items[3]);
                        new_memo_win->desktop = atoi(array_items[4]);
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
    }
    fclose(file);
    return 0;
}

/*
 * save_memo_file: save file with memorized clients
 */

int save_memo_file(char *filename)
{
    char        line[1024];
    FILE        *file;
    t_memo_win  *ptr_memo_win;
    time_t      current_time;
    
    if ( (file = fopen(filename, "wt")) == NULL)
    {
        fprintf(stderr, "WeeWM: unable to create file '%s'.\n", filename);
        return -1;
    }
    
    fputs("# WeeWM - saved position, width & desktop\n", file);
    current_time = time(NULL);
    sprintf(line, "# Created by WeeWM on %s\n", ctime(&current_time));
    fputs(line, file);
    fputs("# DO NOT EDIT while WeeWM is running !\n\n", file);
    
    for (ptr_memo_win = memo_win; ptr_memo_win; ptr_memo_win = ptr_memo_win->next)
    {
        sprintf(line, "%s%s\n",
            APPLI_LINE_BEGIN,
            ptr_memo_win->application_name);
        fputs(line, file);
        sprintf(line, "%d %d %d %d %d\n",
            ptr_memo_win->x, ptr_memo_win->y,
            ptr_memo_win->width, ptr_memo_win->height,
            ptr_memo_win->desktop);
        fputs(line, file);
    }
    fclose(file);
    return 0;
}

/*
 * memorize_client_position: memorize position & width of client
 */

void memorize_client_position(t_client *client)
{
    t_memo_win  *ptr_memo_win, *new_memo_win;
    
    for (ptr_memo_win = memo_win; ptr_memo_win; ptr_memo_win = ptr_memo_win->next)
        if (strcmp(ptr_memo_win->application_name, client->application_name) == 0)
            break;
    
    if (ptr_memo_win)
    {
        /* client already memorized => update memo */
        ptr_memo_win->x = client->x;
        ptr_memo_win->y = client->y;
        ptr_memo_win->width = client->width;
        ptr_memo_win->height = client->height;
    }
    else
    {
        /* client not yet memorized => create new memo */
        new_memo_win = (t_memo_win *)malloc(sizeof(t_memo_win));
        new_memo_win->next = memo_win;
        new_memo_win->application_name =
            (char *)malloc((strlen(client->application_name)+1)*(sizeof(char)));
        strcpy(new_memo_win->application_name, client->application_name);
        new_memo_win->x = client->x;
        new_memo_win->y = client->y;
        new_memo_win->width = client->width;
        new_memo_win->height = client->height;
        new_memo_win->desktop = MEMO_NOT_SET;
        memo_win = new_memo_win;
    }
    
    /* save memorized clients to disk */
    save_memo_file(memo_file);
}

/*
 * memorize_client_desktop: memorize desktop of client
 */

void memorize_client_desktop(t_client *client)
{
    t_memo_win  *ptr_memo_win, *new_memo_win;
    
    for (ptr_memo_win = memo_win; ptr_memo_win; ptr_memo_win = ptr_memo_win->next)
        if (strcmp(ptr_memo_win->application_name, client->application_name) == 0)
            break;
    
    if (ptr_memo_win)
    {
        /* client already memorized => update memo */
        ptr_memo_win->desktop = client->vdesk[client->screen];
    }
    else
    {
        /* client not yet memorized => create new memo */
        new_memo_win = (t_memo_win *)malloc(sizeof(t_memo_win));
        new_memo_win->next = memo_win;
        new_memo_win->application_name = (char *)malloc((strlen(client->application_name)+1)*(sizeof(char)));
        strcpy(new_memo_win->application_name, client->application_name);
        new_memo_win->x = MEMO_NOT_SET;
        new_memo_win->y = MEMO_NOT_SET;
        new_memo_win->width = MEMO_NOT_SET;
        new_memo_win->height = MEMO_NOT_SET;
        new_memo_win->desktop = client->vdesk[client->screen];
        memo_win = new_memo_win;
    }
    
    /* save memorized clients to disk */
    save_memo_file(memo_file);
}

/*
 * remove_memo_win: remove memo for a window
 */

void remove_memo_win(t_memo_win *ptr_memo_win)
{
    t_memo_win  *previous;
    
    if (ptr_memo_win == memo_win)
        memo_win = ptr_memo_win->next;
    else
    {
        for (previous = memo_win; previous && previous->next; previous = previous->next)
            if (previous->next == ptr_memo_win)
                previous->next = ptr_memo_win->next;
    }
    free(ptr_memo_win->application_name);
    free(ptr_memo_win);
}

/*
 * forget_client_position: forget position & width of client
 */

void forget_client_position(t_client *client)
{
    t_memo_win  *ptr_memo_win;
    
    for (ptr_memo_win = memo_win; ptr_memo_win; ptr_memo_win = ptr_memo_win->next)
        if (strcmp(ptr_memo_win->application_name, client->application_name) == 0)
            break;
    
    if (ptr_memo_win)
    {
        if (ptr_memo_win->desktop == MEMO_NOT_SET)
            remove_memo_win(ptr_memo_win);
        else
        {
            ptr_memo_win->x = MEMO_NOT_SET;
            ptr_memo_win->y = MEMO_NOT_SET;
            ptr_memo_win->width = MEMO_NOT_SET;
            ptr_memo_win->height = MEMO_NOT_SET;
        }
        save_memo_file(memo_file);
    }
}

/*
 * forget_client_desktop: forget desktop of client
 */

void forget_client_desktop(t_client *client)
{
    t_memo_win  *ptr_memo_win;
    
    for (ptr_memo_win = memo_win; ptr_memo_win; ptr_memo_win = ptr_memo_win->next)
        if (strcmp(ptr_memo_win->application_name, client->application_name) == 0)
            break;
    
    if (ptr_memo_win)
    {
        if (ptr_memo_win->x == MEMO_NOT_SET)
            remove_memo_win(ptr_memo_win);
        else
            ptr_memo_win->desktop = MEMO_NOT_SET;
        save_memo_file(memo_file);
    }
}
