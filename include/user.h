/*******************************************************************************
 * ircbot                                                                      *
 * Copyright (C) 2013 Rick Gout                                                *
 *                                                                             *
 * This program is free software; you can redistribute it and/or               *
 * modify it under the terms of the GNU General Public License                 *
 * as published by the Free Software Foundation; either version 2              *
 * of the License, or (at your option) any later version.                      *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *******************************************************************************/

#ifndef USER_H
#define USER_H

/* User settings */
#define NICKHASHSIZE  128
#define IGNOHASHSIZE  16

#define NICKLEN       15
#define USERLEN       12
#define HOSTLEN       63
#define USERHOSTLEN   USERLEN + HOSTLEN + 1
#define ACCOUNTLEN    15

#define USTAT_REFRESH     0x01 /* User needs to be refreshed with WHO */
#define USTAT_NOREFRESH   0x04 /* User doesn't need to be refreshed with WHO */
#define USTAT_SENTREFRESH 0x08 /* We put a WHO refresh in the queue */
#define USTAT_ISDEAD      0x10 /* User is marked for deletion */
#define USTAT_ISMARKED    0x20 /* User is marked for various purposes */

#define IGFL_PRIVATE  0x01
#define IGFL_NOTICE   0x02
#define IGFL_CHANPUB  0x04
#define IGFL_CHANNOT  0x08

typedef struct nick_t {
  char              *name;
  char              *userhost;
  struct account_t  *account;
  int                channelcount;
  int                status;
  struct chanuser_t *channels;
} nick;

typedef struct nicklist_t {
  nick              *nick;
  struct nicklist_t *next;
} nicklist;

typedef struct ignore_t {
	char            *mask;
	time_t           added;
    time_t           expiration;
    int              flags;
    struct ignore_t *next;
} ignore;

extern htable *nicks;
extern ignore *ignores;

/* nick functions */
nick *finduser(char *nickname);
nick *addtouser(nick *np, char *nickname, char *userhost, char *account);
void deluser(nick *np);
char *getnickfromhostmask(char *hostmask);
char *gethostfromhostmask(char *hostmask);
void addnicktohash(nick *np);
void removenickfromhash(nick *np);

/* ignore functions */
ignore *findignore(char *mask);
ignore *addignore(char *mask, time_t expiration);
void delignore(ignore *ip);
int isignored(nick *np);

/* nicklist function(s) */
nicklist *newnicklist();

#endif /* USER_H */
