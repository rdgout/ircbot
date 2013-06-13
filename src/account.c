#include "common.h"

htable *accounts = NULL;

account *findaccount(char *name) {
    bucket *bp;

    for (bp = accounts->buckets[hashi(name) % accounts->size]; bp; bp = bp->next)
        if (!strcasecmp(bp->key, name))
            return (account *)bp->pointer;

    return NULL;
}

account *addaccount(char *name) {
    account *ap;

    if ((ap = findaccount(name)))
        return ap;

    if (strlen(name) > ACCOUNTLEN)
        return NULL;

    ap = (account *)malloc(sizeof(account));

    ap->name = strdup(name);
    
    ap->nicks = NULL;
    ap->loggedin = 0;
    ap->dbuser = NULL;
    ap->status = 0;

    addaccounttohash(ap);
    return ap;
}

void addnicktoaccount(account *ap, nick *np) {
    nicklist *nlp;

    if (!ap)
        return;

    for (nlp = ap->nicks; nlp; nlp = nlp->next)
        if (nlp->nick == np)
            return;
    
    np->account = ap;

    nlp = newnicklist();
    nlp->nick = np;
    nlp->next = ap->nicks;
    ap->nicks = nlp;
    ap->loggedin++;
}

void removenickfromaccount(nick *np) {
    account *ap;
    nicklist **tmp, *nlp;

    if (np->account) {
        ap = np->account;

        /* Find and remove the nick from the account nicklist */
        for (tmp = &(ap->nicks); *tmp; tmp = &((*tmp)->next)) {
            if ((*tmp)->nick == np) {
                nlp = *tmp;
                *tmp = (*tmp)->next;
                free(nlp);
                break;
            }
        }

        /* Decrease loggedin and delete this account if it's no longer used */
        if (--ap->loggedin == 0 && !ap->dbuser)
            delaccount(ap);
    }
}

void delaccount(account *ap) {
    nicklist *tmp, *nlp;

    if (ap) {
        removeaccountfromhash(ap);

        if (ap->nicks) {
            for (nlp = ap->nicks; nlp; nlp = tmp) {
                tmp = nlp->next;
                free(nlp);
                break;
            }
        }

        if (ap->name)
            free(ap->name);
        if (ap->dbuser)
            free(ap->dbuser);
        free(ap);
    }
}

void addaccounttohash(account *ap) {
    insertintohtable(accounts, ap->name, ap);
}

void removeaccountfromhash(account *ap) {
    removefromhtable(accounts, ap->name, ap);
}

dbuser *newdbuser() {
    dbuser *dup;

    dup = (dbuser *)malloc(sizeof(dbuser));
    dup->account = NULL;
    dup->flags = 0;
    dup->lastseen = 0;
    dup->clevcount = 0;
    dup->chanlevels = NULL;
    dup->trigger = '$';

    return dup;
}

chanlevel *findchanlevel(channel *cp, account *ap) {
    if (!cp || !ap || !ap->dbuser)
        return NULL;

    if (ap->dbuser->clevcount < cp->clevcount)
        return findchanlevelbyaccount(cp, ap);
    else
        return findchanlevelbychan(cp, ap);
}

chanlevel *findchanlevelbyaccount(channel *cp, account *ap) {
    chanlevel *clp;

    for (clp = ap->dbuser->chanlevels; clp; clp = clp->nextbyaccount)
        if (clp->channel == cp)
            return clp;

    return NULL;
}

chanlevel *findchanlevelbychan(channel *cp, account *ap) {
    chanlevel *clp;

    for (clp = cp->chanlevels; clp; clp = clp->nextbychan)
        if (clp->account == ap)
            return clp;

    return NULL;
}

chanlevel *addchanlevel(channel *cp, account *ap) {
    chanlevel *clp;

    if ((clp = findchanlevel(cp, ap)))
        return clp;

    clp = (chanlevel *)malloc(sizeof(chanlevel));

    clp->channel     = cp;
    clp->account     = ap;

    clp->lastchanged = 0;
    clp->lastseen    = 0;
    clp->flags       = 0;

    clp->nextbychan  = cp->chanlevels;
    cp->chanlevels   = clp;

    cp->clevcount++;

    clp->nextbyaccount = ap->dbuser->chanlevels;
    ap->dbuser->chanlevels = clp;

    ap->dbuser->clevcount++;

    return clp;
}

void delchanlevel(chanlevel *clp) {
    chanlevel **tmp;
    
    /* Find and remove the channel from the users channel list */
    for (tmp = &(clp->account->dbuser->chanlevels); *tmp; tmp = &((*tmp)->nextbyaccount)) {
        if ((*tmp)->channel == clp->channel) {
            /* Change it into the next channel */
            *tmp = (*tmp)->nextbyaccount;
            break;
        }
    }
    
    /* Find and remove the user from the channels list */
    for (tmp = &(clp->channel->chanlevels); *tmp; tmp = &((*tmp)->nextbychan)) {
        if ((*tmp)->account == clp->account) {
            /* Change it into the next user */
            *tmp = (*tmp)->nextbychan;
            break;
        }
    }

    /* Check if the user has any other access flags, otherwise we might aswell delete him? */
    /* if (!clp->account->chanlevels && !clp->account->flags)
        delaccount(cup->nick); */

    clp->account->dbuser->clevcount--;
    clp->channel->clevcount--;

    free(clp);
}

