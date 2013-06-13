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

#include "common.h"

htable *commands = NULL;

command *findcommand(char *name, int type) {
    bucket *bp;
    command *cdp;

    for (bp = commands->buckets[hashi(name) % commands->size]; bp; bp = bp->next) {
        if (!strcasecmp(bp->key, name)) {
            cdp = (command *)bp->pointer;
            
            if (cdp->type & type)
            	return (command *)bp->pointer;
        }
    }

    return NULL;
}

command *addcommand(char *name, int level, int type, srvcmdhandler_t srvhandler, cmdhandler_t handler) {
    command *cdp;

    if ((cdp = findcommand(name, type)))
        return cdp;

    cdp = (command *)malloc(sizeof(command));

    if (!cdp)
        return NULL;

    cdp->commandname = strdup(name);
    cdp->level = level;
    cdp->type = type;

    cdp->srvhandler = srvhandler;
    cdp->handler = handler;

    insertintohtable(commands, cdp->commandname, cdp);

    return cdp;
}

void delcommand(char *name, int type, srvcmdhandler_t srvhandler, cmdhandler_t handler) {
    command *cdp;
    
    if (!(cdp = findcommand(name, type)))
        return;

    if (((cdp->type & CTYPE_SERVER) && cdp->srvhandler == srvhandler) || (!(cdp->type & CTYPE_SERVER) &&  cdp->handler == handler)) {
        /* This appears to be command we want to delete :) */
        removefromhtable(commands, cdp->commandname, cdp);
        
        printlog("Deleting command %s.", cdp->commandname);

        if (cdp->commandname)
            free(cdp->commandname);
        free(cdp);
    }
}

int haspermissions(nick *np, int permissions) {
    int flags = 0;

    if (!permissions || (permissions & (CLEVEL_ALL | GLEVEL_ALL)))
        return 1;
    if ((permissions & GLEVEL_UNKNOWN) && (np->account && np->account->dbuser))
        return 0;

    if (permissions & GLEVEL_ACCOUNT) {
        if (!np->account || (np->account && !np->account->dbuser))
            return 0;
        flags = np->account->dbuser->flags;

        if ((permissions & GLEVEL_KNOWN) && !(flags & (DBF_KNOWN | DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & GLEVEL_VOICE) && !(flags & (DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & GLEVEL_OP) && !(flags & (DBF_OP | DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & GLEVEL_MASTER) && !(flags & (DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & GLEVEL_OWNER) && !(flags & DBF_OWNER))
            return 0;
    }

    return 1;
}

int haschanpermissions(nick *np, channel *cp, int permissions) {
    chanlevel *clp;
    int cflags = 0;
    int gflags = 0;

    if (!permissions || (permissions & (CLEVEL_ALL | GLEVEL_ALL)))
        return 1;
    if ((permissions & GLEVEL_UNKNOWN) && (np->account && np->account->dbuser))
        return 0;

    if (permissions & GLEVEL_ACCOUNT) {
        if (!np->account || (np->account && !np->account->dbuser))
            return 0;

        for (clp = np->account->dbuser->chanlevels; clp; clp = clp->nextbyaccount) {
            if (clp->channel == cp) {
                cflags = clp->flags;
                break;
            }
        }

        gflags = np->account->dbuser->flags;

        if ((permissions & (CLEVEL_KNOWN | GLEVEL_KNOWN)) && !(cflags & (DBF_KNOWN | DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER))
            && !(gflags & (DBF_KNOWN | DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & (CLEVEL_VOICE | GLEVEL_VOICE)) && !(cflags & (DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER))
            && !(gflags & (DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & (CLEVEL_OP | GLEVEL_OP)) && !(cflags & (DBF_OP | DBF_MASTER | DBF_OWNER))
            && !(gflags & (DBF_OP | DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & (CLEVEL_MASTER | GLEVEL_MASTER)) && !(cflags & (DBF_MASTER | DBF_OWNER))
            && !(gflags & (DBF_MASTER | DBF_OWNER)))
            return 0;

        if ((permissions & (CLEVEL_OWNER | GLEVEL_OWNER)) && !(cflags & DBF_OWNER)
            && !(gflags & DBF_OWNER))
            return 0;
    }

    return 1;
}

