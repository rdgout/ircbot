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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "hash.h"
#include "user.h"
#include "channel.h"

#define ACCOUNTHASHSIZE  128

#define DBF_OWNER        0x01
#define DBF_MASTER       0x02
#define DBF_OP           0x04
#define DBF_VOICE        0x08
#define DBF_KNOWN        0x10

#define DBF_ALL          (DBF_OWNER | DBF_MASTER | DBF_OP | DBF_VOICE | DBF_KNOWN)

typedef struct account_t {
    char               *name;
    struct nicklist_t  *nicks;
    int                 loggedin;
    int                 status;
    struct dbuser_t    *dbuser;
} account;

typedef struct dbuser_t {
    account            *account;
    int                 flags;
    time_t              lastseen;
    int                 trigger;
    int                 clevcount;
    struct chanlevel_t *chanlevels;
} dbuser;

typedef struct chanlevel_t {
    struct chan_t      *channel;
    struct account_t   *account;
    time_t              lastchanged;
    time_t              lastseen;
    int                 flags;
    struct chanlevel_t *nextbychan;
    struct chanlevel_t *nextbyaccount;
} chanlevel;

extern htable *accounts;

account *findaccount(char *name);
account *addaccount(char *name);
void addnicktoaccount(account *ap, nick *np);
void removenickfromaccount(nick *np);
void delaccount(account *ap);
void addaccounttohash(account *ap);
void removeaccountfromhash(account *ap);
dbuser *newdbuser();
chanlevel *findchanlevel(channel *cp, account *ap);
chanlevel *findchanlevelbyaccount(channel *cp, account *ap);
chanlevel *findchanlevelbychan(channel *cp, account *ap);
chanlevel *addchanlevel(channel *cp, account *ap);
void delchanlevel(chanlevel *clp);

#endif /* ACCOUNT_H */
