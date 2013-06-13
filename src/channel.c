#include "common.h"

channel *channels = NULL;

channel *findchannel(char *name) {
    channel *cp;
    
    for (cp = channels; cp; cp = cp->next)
        if (!strcasecmp(name, cp->name))
            return cp;

    return NULL;
}

channel *addchannel(char *name) {
    channel *cp;

    if ((cp = findchannel(name)))
        return cp;

    /* Add a new channel */
    cp = (channel *)malloc(sizeof(channel));

    /* This should be perfectly safe */
    cp->name = strdup(name);
    
    cp->status = 0;
    cp->onchan = 0; /* This is set when we recieve the JOIN message from the IRCd */
    cp->isopped = 0; /* Set when we recieve an actual MODE +o botnick */
    cp->lastkicked = 0; /* Updated only if we are kicked */
    cp->usercount = 0; /* Kept uptodate by addchanuser/delchanuser */
    cp->bancount = 0; /* Kept uptodate by addchanban/delchanban */
    cp->clevcount = 0; /* Kept uptodate by addchanlevel/delchanlevel */
    cp->timer = NULL;
    cp->modes = NULL; /* Updated by addchanmode etc */
    cp->users = NULL; /* Also updated by addchanuser/delchanuser */
    cp->bans = NULL;
    // cp->chanlevels = NULL; /* Not sure yet */
    cp->createdat = 0;
    cp->lastjoin = 0;
    cp->flags = 0;

    /* Place the channel in our channel list */
    cp->next = channels;
    channels = cp;

    printlog("added new channel: %s", cp->name);

    return cp;
}

void delchannel(channel *cp) {
    channel *nextchan;
    chanmode *cmp, *nextcmp;
    chanban *cbp, *nextban;
    chanuser *cup, *nextcu;

    if (!cp)
        return;

    printlog("deleting channel: %s", cp->name);

    /* First cancel the timer we may have for this channel */
    if (cp->timer)
        remove_timer(cp->timer);   
    
    /* Remove the allocated modes */
    if (cp->modes) {
        for (cmp = cp->modes; cmp; cmp = nextcmp) {
            nextcmp = cmp->nextbychan;
            if (cmp->parameter)
                free(cmp->parameter);
            free(cmp);
        }
    }

    /* Remove any bans we may have */
    for (cbp = cp->bans; cbp; cbp = nextban) {
        nextban = cbp->nextbychan;

        delchanban(cbp);
    }

    /* Remove any chanusers that might be present */
    for (cup = cp->users; cup; cup = nextcu) {
        nextcu = cup->nextbychan;

        delchanuser(cup);
    }

    /* Clear the name */
    if (cp->name)
        free(cp->name);
        
    /* Remove the channel from the list */
    if (cp == channels) {
            channels = cp->next;
    } else {        
        for (nextchan = channels; nextchan; nextchan = nextchan->next) {
            if (nextchan->next == cp) {
                nextchan->next = cp->next;
                break;
            }
        }
    }

    free(cp);
}

chanuser *findchanuser(channel *cp, nick *np) {
    if (!cp || !np)
        return NULL;

    /* Use the check that's likely to be fastest */
    if (np->channelcount < cp->usercount)
        return findchanuserbynick(cp, np);
    else
        return findchanuserbychan(cp, np);
}

chanuser *findchanuserbychan(channel *cp, nick *np) {
    chanuser *cup;

    for (cup = cp->users; cup; cup = cup->nextbychan)
        if (cup->nick == np)
            return cup;

    return NULL;
}

chanuser *findchanuserbynick(channel *cp, nick *np) {
    chanuser *cup;

    for (cup = np->channels; cup; cup = cup->nextbynick)
        if (cup->channel == cp)
            return cup;

    return NULL;
}

chanuser *addchanuser(channel *cp, nick *np, int modes, time_t joined) {
    chanuser *cup;

    if (!cp || !np)
        return NULL;

    /* Check wether or not we don't already know this user */
    if ((cup = findchanuser(cp, np)))
        return cup;

    cup = (chanuser *)malloc(sizeof(chanuser));

    cup->channel = cp;
    cup->nick = np;
    cup->status = modes; /* Generally modes should only contain OP, VOICE or 0 (at this point) */
    cup->lastmessage = 0;
    cup->chanjoin = joined; /* 0 if we add the user from NAMES, non-zero is we add the user from JOIN */
    
    if (joined && !(np == bi.botuser))
        cp->lastjoin = joined;

    /* Link the channel user to the other ones */
    cp->usercount++;
    np->channelcount++;

    /* First link the user */
    cup->nextbynick = np->channels;
    np->channels = cup;
    /* Now link the channel */
    cup->nextbychan = cp->users;
    cp->users = cup;

    return cup;
}

void delchanuser(chanuser *cup) {
    chanuser **tmp;
    
    /* Find and remove the channel from the users channel list */
    for (tmp = &(cup->nick->channels); *tmp; tmp = &((*tmp)->nextbynick)) {
        if ((*tmp)->channel == cup->channel) {
            /* Change it into the next channel */
            *tmp = (*tmp)->nextbynick;
            break;
        }
    }
    
    /* Find and remove the user from the channels list */
    for (tmp = &(cup->channel->users); *tmp; tmp = &((*tmp)->nextbychan)) {
        if ((*tmp)->nick == cup->nick) {
            /* Change it into the next user */
            *tmp = (*tmp)->nextbychan;
            break;
        }
    }

    if (--cup->channel->usercount <= 1)
        /* No people left in this channel, mark it as inactive */
        cup->channel->status |= CHSTAT_INACTIVE;

    /* Now we check to see if the user is in any other channels else, we delete him! */
    if (!(bi.botuser == cup->nick) && !cup->nick->channels && !(cup->nick->status & USTAT_ISDEAD))
        deluser(cup->nick);

    free(cup);
}

chanmode *getchanmode(channel *cp, char mode) {
    chanmode *cmp;

    for (cmp = cp->modes; cmp; cmp = cmp->nextbychan)
        if (cmp->mode->str == mode)
            return cmp;

    return NULL;
}

chanmode *setchanmode(channel *cp, char mode, char *parameter, int set) {
    chanmode *cmp;

    for (cmp = cp->modes; cmp; cmp = cmp->nextbychan) {
        if (cmp->mode->str == mode) {

            cmp->set = set;
            
            if (cmp->parameter) {
                free(cmp->parameter);
                cmp->parameter = NULL;
            }

            if (parameter)
                cmp->parameter = strdup(parameter);

            return cmp;
        }
    }

    return NULL;
}

chanban *findchanban(channel *cp, char *mask) {
    chanban *cbp;

    for (cbp = cp->bans; cbp; cbp = cbp->nextbychan)
        if (!strcmp(cbp->mask, mask))
            return cbp;

    return NULL;
}

chanban *addchanban(channel *cp, char *mask, char *setby, time_t setdate) {
    chanban *cbp;

    if ((cbp = findchanban(cp, mask)))
        return cbp;

    cbp = (chanban *)malloc(sizeof(chanban));

    cbp->channel    = cp;
    cbp->mask       = strdup(mask);

    if (setby)
        cbp->setby  = strdup(setby);
    else
        cbp->setby  = NULL;

    cbp->timestamp  = setdate;
    cbp->isactive   = 1;

    /* Link the ban to the channel */
    cbp->nextbychan = cp->bans;
    cp->bans        = cbp;

    /* Increase the bancount */
    cp->bancount++;

    return cbp;
}

void delchanban(chanban *cbp) {
    chanban **tmp;

    /* Find and remove the ban from the channels ban list */
    for (tmp = &(cbp->channel->bans); *tmp; tmp = &((*tmp)->nextbychan)) {
        if ((*tmp) == cbp) {
            /* Change it into the next ban */
            *tmp = (*tmp)->nextbychan;
            break;
        }
    }

    /* Decrease the bancount */
    cbp->channel->bancount--;

    if (cbp->mask)
        free(cbp->mask);
    if (cbp->setby)
        free(cbp->setby);

    free(cbp);
}

int matchban(chanban *cbp, chanuser *cup) {
    char usermask[NICKLEN + USERHOSTLEN + 2]; /* NICKLEN + ! + USERHOSTLEN + \0 */

    sprintf(usermask, "%s!%s", cup->nick->name, cup->nick->userhost);

    return mmatch(cbp->mask, usermask);
}

char *spewchanmodes(channel *cp) {
    static char chanmodebuf[50];
    chanmode *cmp;
    int count = 0, i;

    chanmodebuf[count++] = '+';

    for (i = MTYPE_PARAMNEVER; i > MTYPE_LIST; i >>= 1)
        for (cmp = cp->modes; cmp; cmp = cmp->nextbychan)
            if (cmp->set && cmp->mode->type == i)
                chanmodebuf[count++] = cmp->mode->str;

    chanmodebuf[count++] = '\0';

    return chanmodebuf;
}

void checkchannel(void *channame) {
    channel *cp;
    chanuser *cup;
    int len;
    char buffer[451];

    if (!(cp = findchannel((char *)channame)))
        return;

    printlog("checking %s..", cp->name);

    if ((!cp->onchan || (cp->status & CHSTAT_NEEDJOIN)) && !(cp->status & CHSTAT_SENTJOIN)) {
        cp->status |= CHSTAT_SENTJOIN;
        irc_write(QUEUE_NORMAL, "JOIN %s", cp->name);
        return; /* Since we can't do anything on the channel when we're not on it, don't bother trying to do anything else */
    }

    if (cp->status & CHSTAT_REFRESH) {
        printlog("attempting to refresh %s.", cp->name);
        /* First, check what's probably the best way of doing it */
        for (len = 0, cup = cp->users; cup; cup = cup->nextbychan) {
            if (cup->nick->status & USTAT_REFRESH) {
                len += strlen(cup->nick->name) + 1;

                if (len >= 400)
                    break;
            }
        }

        if (len >= 400) {
            irc_write(QUEUE_WHO, "WHO %s cd%%cfnuhat,472", cp->name);
        } else {
            for (buffer[0] = '\0', len = 0, cup = cp->users; cup; cup = cup->nextbychan) {
                if (cup->nick->status & USTAT_REFRESH) {
                    if (buffer[0])
                        strcat(buffer, ",");

                    len += strlen(cup->nick->name) + 1;
                    strcat(buffer, cup->nick->name);
                    /* Remove the status and add the sentwho status */
                    cup->nick->status &= ~USTAT_REFRESH;
                    cup->nick->status |= USTAT_SENTREFRESH;
                }

                /* If we run out of chanusers, flush */
                if (!cup->nextbychan && len) {
                    irc_write(QUEUE_WHO, "WHO %s cd%%cfnuhat,268", buffer);
                    len = 0;
                    buffer[0] = '\0';
                }
            }
        }
        cp->status &= ~CHSTAT_REFRESH;
    }
    
    /* Do some more stuff here */
}

