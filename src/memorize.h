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


#ifndef __WEEWM_MEMORIZE_H
#define __WEEWM_MEMORIZE_H 1

#define APPLI_LINE_BEGIN    "*** "

#define MEMO_NOT_SET        -999999

typedef struct t_memo_win t_memo_win;
    
struct t_memo_win
{
    t_memo_win *next;
    char *application_name;
    int x, y;
    int width, height;
    int desktop;
};

extern t_memo_win *memo_win;

extern void memo_init ();
extern int read_memo_file (char *);
extern int save_memo_file (char *);
extern void memorize_client_position (t_client *);
extern void memorize_client_desktop (t_client *);
extern void forget_client_position (t_client *);
extern void forget_client_desktop (t_client *);

#endif /* memorize.h */
