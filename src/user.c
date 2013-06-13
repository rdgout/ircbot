#include "common.h"

htable *nicks = NULL;
ignore *ignores = NULL;

nick *finduser(char *nickname) {
    bucket *bp;

    for (bp = nicks->buckets[hashi(nickname) % nicks->size]; bp; bp = bp->next) {
        if (!strcasecmp(bp->key, nickname))
            return (nick *)bp->pointer;
    }

    return NULL;
}

nick *addtouser(nick *np, char *nickname, char *userhost, char *account) {
 
    if (!np && !(np = finduser(nickname))) {
        /* Add a new user */
        np = (nick *)malloc(sizeof(nick));
            
        np->name = NULL;
        np->userhost = NULL;
        np->account = NULL;
        np->channelcount = 0;
        np->status = 0;
        np->channels = NULL;
    }

    if (nickname) {
        if (np->name) {
            removenickfromhash(np);
            free(np->name);
            np->name = NULL;
        }

        np->name = strdup(nickname);
        addnicktohash(np);
    }

    if (userhost) {
        if (np->userhost)
            free(np->userhost);
        np->userhost = strdup(userhost);
    }

    if (account) {
        if (np->account) {
            /* This is not possible */
            printlog("What the hell are you doing!! We can't add 2 accounts to 1 user!");
            return NULL;
        }
        addnicktoaccount(addaccount(account), np);
    }

    return np;
}

void deluser(nick *np) {
    chanuser *cup, *next;

    if (!np)
        return;

    /* Remove the nickname from the hash */
    removenickfromhash(np);
    
    np->status |= USTAT_ISDEAD;
    
    /* Remove the user from the channels */
    if (np->channels) {
        for (cup = np->channels; cup; cup = next) {
            next = cup->nextbynick;
            
            delchanuser(cup);
        }
    }
    
    printlog("removing user: %s", np->name);

    if (np->name)
        free(np->name);
        
    if (np->userhost)
        free(np->userhost);
    
    if (np->account)
        removenickfromaccount(np);
        
    free(np);
}

char *getnickfromhostmask(char *hostmask) {
    char *nick;
    char *ex = strstr(hostmask, "!");

    if (!ex)
        return NULL;
    else {
        nick = strdup(hostmask);

        if (nick == NULL)
            return NULL;

        nick[ex - hostmask] = '\0';

        return nick;
    }
}

char *gethostfromhostmask(char *hostmask) {
    char *ex = strstr(hostmask, "!");

    if (ex)
        return ex + 1;
    
    return NULL;
}

void addnicktohash(nick *np) {
    insertintohtable(nicks, np->name, np);
}

void removenickfromhash(nick *np) {
    removefromhtable(nicks, np->name, np);
}

ignore *findignore(char *mask) {
    ignore *ip;

    for (ip = ignores; ip; ip = ip->next)
        if (!strcasecmp(ip->mask, mask))
            return ip;

    return NULL;
}

ignore *addignore(char *mask, time_t expiration) {
    ignore *ip;

    if (!(ip = findignore(mask))) {

        ip = (ignore *)malloc(sizeof(ignore));

        ip->mask       = strdup(mask);
        ip->added      = time(NULL);
        ip->expiration = expiration;
        ip->next = ignores;
        ignores = ip;
    }

    return ip;
}

void delignore(ignore *ip) {
    ignore **tmp;
    
    if (!ip)
        return;
    
    for (tmp = &(ignores); *tmp; tmp = &((*tmp)->next)) {
        if ((*tmp) == ip) {
            /* Change it into the next ignore */
            *tmp = (*tmp)->next;
            break;
        }
    }
}

int isignored(nick *np) {
    char usermask[NICKLEN + USERHOSTLEN + 2]; /* NICKLEN + ! + USERHOSTLEN + \0 */
    ignore *ip;

    sprintf(usermask, "%s!%s", np->name, np->userhost);

    for (ip = ignores; ip; ip = ip->next)
        if (!mmatch(ip->mask, usermask))
            return 1;
    
    return 0;
}

nicklist *newnicklist() {
    nicklist *nlp; /* _N_ick_L_ist_P_ointer */
    
    nlp = (nicklist *)malloc(sizeof(nicklist));
    nlp->nick = NULL;
    nlp->next = NULL;

    return nlp;
}

