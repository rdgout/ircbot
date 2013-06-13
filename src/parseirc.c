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

/* :demon1.uk.quakenet.org 001 ircbot :Welcome to the QuakeNet IRC Network, ircbot */
int bot_initserver(char *source, int argc, char **argv) {
    channel *cp;

    /* Store the servername + connect-time */
    bi.servername = strdup(source);
    bi.connected = time(NULL);
        
    printlog("Connection was accepted by %s.", bi.servername);
        
    irc_write(QUEUE_SERVER, "USERHOST %s", bi.botuser->name);
        
    /* Join our channels */
    for (cp = channels; cp; cp = cp->next)
        irc_write(QUEUE_NORMAL, "JOIN %s", cp->name);

    return 1;
}

/* :demon1.uk.quakenet.org 005 ircbot WHOX WALLCHOPS WALLVOICES USERIP CPRIVMSG CNOTICE SILENCE=15 MODES=6 MAXCHANNELS=20 MAXBANS=45 NICKLEN=15 :are supported by this server */
/* :demon1.uk.quakenet.org 005 ircbot MAXNICKLEN=15 TOPICLEN=250 AWAYLEN=160 KICKLEN=250 CHANNELLEN=200 MAXCHANNELLEN=200 CHANTYPES=#& PREFIX=(ov)@+ STATUSMSG=@+ CHANMODES=b,k,l,imnpstrDducCNMT CASEMAPPING=rfc1459 NETWORK=QuakeNet :are supported by this server */
int bot_initsettings(char *source, int argc, char **argv) {
    int i, type = MTYPE_LIST;
    char *pos, *chp;
    
    for (i = 0; i < argc; i++) {
        if ((pos = strchr(argv[i], '='))) {
            *pos = '\0';
            
            printlog("Found setting: %s", argv[i]);
            
            if (!strcmp("MODES", argv[i]))
                bi.maxmodes = atoi(pos + 1);
            else if (!strcmp("MAXBANS", argv[i]))
                bi.maxbans = atoi(pos + 1);
            else if (!strcmp("NICKLEN", argv[i]))
                bi.maxnicklen = atoi(pos + 1);
            else if (!strcmp("TOPICLEN", argv[i]))
                bi.maxtopiclen = atoi(pos + 1);
            else if (!strcmp("CHANNELLEN", argv[i]))
                bi.maxchanlen = atoi(pos + 1);
            else if (!strcmp("CHANMODES", argv[i])) {
                for (chp = (pos + 1); *chp; chp++) {
                    if (*chp == ',') {
                        type <<= 1;
                        continue;
                    }
                    
                    addmode(*chp, type);
                }
            }
        }
    }
    
    return 1;
}

/* :demon1.uk.quakenet.org 302 ircbot :ircbot=+metroid@pluto.shroudbox.net */
int bot_userhost(char *source, int argc, char **argv) {
    char *charp;
    char *keyline, *name, *pass, *modex;

    if ((charp = strchr(argv[1], '=')))
        /* This adds the userhost to our bot entry */
        addtouser(bi.botuser, NULL, charp + 1, NULL);

    printlog("I am %s!%s", bi.botuser->name, bi.botuser->userhost);

    /* Attempt to authenticate to network service now */
    keyline = getconf("keyline", NULL);

    if (keyline) {
        name = getconf("authname", NULL);
        pass = getconf("authpass", NULL);

        if (name && pass)
            irc_write(QUEUE_SERVER, keyline, name, pass);

        /* Temporary hack */
        modex = getconf("modex", NULL);
        if (modex && atoi(modex) == 1)
            irc_write(QUEUE_SERVER, "MODE %s +x", bi.botuser->name);
    }

    return 1;
}

/* :demon1.uk.quakenet.org 324 m00bot #v1per +stnCNTl 30 */
int bot_getmodes(char *source, int argc, char **argv) {
    channel *cp;
    modes *emp;
    chanmode *cmp;
    char *chp;
    int index = 3;
    
    if (!(cp = findchannel(argv[1])))
        return 0;

    /* This should be the very first message we recieve after joining a channel for the first time
     * or after having been out of a channel for a long time..
     * In any case, we create a full list of modes for the channel and set them to unset */
     if (cp->modes)
         for (cmp = cp->modes; cmp; cmp = cmp->nextbychan)
             setchanmode(cp, cmp->mode->str, NULL, 0);

    for (chp = argv[2]; *chp; chp++) {
        /* Lots of looping here, go through the list of valid modes as defined by the ircd */
        for (emp = bi.modes; emp; emp = emp->next) {
            if (emp->str == *chp) {
                //printlog("Time to deal with %c on %s (%d)", *chp, cp->name, emp->type & (MTYPE_PARAMALWAYS | MTYPE_PARAMSET));
                if (emp->type & (MTYPE_PARAMALWAYS | MTYPE_PARAMSET)) {
                    setchanmode(cp, emp->str, argv[index++], 1);
                } else {
                    setchanmode(cp, emp->str, NULL, 1);
                }
            }
        }
    }

    /* Should have all modes now */
    return 1;
}

/* :metroiD!metroid@metroid.users.quakenet.org NICK :metroid */
int bot_nickmsg(char *source, int argc, char **argv) {
    char *unick = getnickfromhostmask(source);
    nick *np;

    if (!unick)
        return 0;

    np = finduser(unick); /* this can't go wrong really */
    free(unick);

    addtouser(np, argv[0], NULL, NULL);
    return 1;
}

/* :demon1.uk.quakenet.org 433 * ircbot :Nickname is already in use. */
int bot_nicktaken(char *source, int argc, char **argv) {
    /* Nick already taken, go with the altnick */
    char *altnick = getconf("altnickname", NULL);

    if (!altnick) {
        altnick = argv[1];
    }

    if (!strcasecmp(argv[1], altnick)) {
        /* Generate a new nickname (because there was either no alternate nickname
           or because the alternate nickname was taken too) */
        if ((bi.maxnicklen && strlen(altnick) < bi.maxnicklen) || (strlen(altnick) < NICKLEN))
            sprintf(altnick, "%s%d", altnick, rand()%10);
    }

    irc_write(QUEUE_SERVER, "NICK %s", altnick);
    addtouser(bi.botuser, altnick, NULL, NULL);

    return 1;
}

/* :demon1.uk.quakenet.org 353 m00bot @ #v1per :m00bot @NeoBlaster @bingos metroid @Witchblade @Arie- @cib @Scaffo @evo_ @_David @Q */
int bot_names(char *source, int argc, char **argv) {
    channel *cp;
    nick *np;
    int nicks, chmode, i;
    char *nline[512], *charp;

    if (!(cp = findchannel(argv[2])))
        return 0;

    nicks = linetoarray(argv[3], nline, 0, 512);

    for (i = 0; i < nicks; i++) {
        chmode = 0;
        for (charp = nline[i]; *charp; charp++) {
            if (*charp == '@') {
                chmode |= CUSTAT_OP;
                nline[i]++;
            } else if (*charp == '+') {
                chmode |= CUSTAT_VOICE;
                nline[i]++;
            }
        }

        if (!(np = finduser(nline[i])))
            np = addtouser(NULL, nline[i], NULL, NULL);
            
        if (!(np->account || np->userhost) && !(np == bi.botuser))
            np->status |= USTAT_REFRESH;

        /* Add the chanuser + modes to the user here */
        addchanuser(cp, np, chmode, 0);
    }

    return 1;
}

/* :demon1.uk.quakenet.org 366 m00bot #v1per :End of /NAMES list. */
int bot_endnames(char *source, int argc, char **argv) {
    channel *cp;

    if (!(cp = findchannel(argv[1])))
        return 0;

    //irc_write(QUEUE_NORMAL, "PRIVMSG %s :I joined %s and there are %d users.", "#v1per", cp->name, cp->usercount);

    return 1;
}

/* :demon1.uk.quakenet.org 367 ircbot #v1per *!~admin@202.56.7.166 * 1221671612 */
/* :servername             367 botnick #channel hostmask setby timestamp */
int bot_getban(char *source, int argc, char **argv) {
    channel *cp;
    chanban *cbp;

    if (!(cp = findchannel(argv[1])))
        return 0;

    cbp = addchanban(cp, argv[2], !strcmp(argv[3], "*") ? NULL : argv[3], atol(argv[4]));
    printlog("Added ban %s to %s (%d)", cbp->mask, cbp->channel->name, cp->bancount);

    return 1;
}

/* :demon1.uk.quakenet.org 354 m00bot #v1per metroid pluto.shroudbox.net m00bot H 0 */
int bot_whoreply(char *source, int argc, char **argv) {
    char userhost[USERHOSTLEN + 1], *charp;
    int mode = 0;
    nick *np;
    account *ap;
    channel *cp = NULL;
    chanuser *cup = NULL;
    
    if (strcmp(argv[1], "472") && strcmp(argv[1], "268"))
        return 0;

    if (!strcmp(argv[1], "472") && !(cp = findchannel(argv[2])))
        return 0;

    for (charp = argv[6]; *charp; charp++) {
        if (*charp == '@')
            mode |= CUSTAT_OP;
        else if (*charp == '+')
            mode |= CUSTAT_VOICE;
    }

    if (!(np = finduser(argv[5]))) {
        sprintf(userhost, "%s@%s", argv[3], argv[4]);
        /* This generally shouldn't happen so it doesn't matter too much that we search for the nick twice here */
        np = addtouser(NULL, argv[5], userhost, NULL);
        /* This means that we don't have a chanuser either */
        printlog("Added user %s (%s) because we didn't see him before.", np->name, np->userhost);
        if (cp)
            cup = addchanuser(cp, np, mode, 0);
    }

    if (!np->userhost) {
        sprintf(userhost, "%s@%s", argv[3], argv[4]);
        /* We don't have a host stored for this user yet, add it now */
        addtouser(np, NULL, userhost, NULL);
    }

    np->status &= ~USTAT_SENTREFRESH;

    if (cp && (cup = findchanuser(cp, np)) && mode)
        cup->status |= mode;

    /* Check for account here */
    if (!np->account) {
        if (!strcmp(argv[7], "0")) {
            /* Mark the client as needing to be refreshed */
            np->status |= USTAT_REFRESH;
            if (cp)
                cp->status |= CHSTAT_REFRESH;
        } else {
            ap = addaccount(argv[7]);
            addnicktoaccount(ap, np);
        }
    }

    if (np->status & USTAT_REFRESH) {
        for (cup = np->channels; cup; cup = cup->nextbynick) {
            if (!(cup->channel->status & CHSTAT_REFRESH)) {
                /* Lets mark the channel as needing to be refreshed */
                cup->channel->status |= CHSTAT_REFRESH;
            }
        }
    }

    return 1;
}

/* :metroid!metroid@securenet.eu.org PRIVMSG #v1per :won't work for you :p */
int bot_privmsg(char *source, int argc, char **argv) {
    char *unick = getnickfromhostmask(source);
    nick *np;
    channel *cp;
    int newuser = 0;
    
    if (!unick)
        return 0;

    np = finduser(unick);
    cp = findchannel(argv[0]);

    if (!cp) {
        /* This is a private message to me */
        if (!np) {
            /* Add this user temporarily and delete him afterwards */
            np = addtouser(NULL, unick, NULL, NULL);
            newuser = 1;
        }
        handleprivmsg(np, argv[1]);
        /* Remove that unknown user! */
        if (newuser)
            deluser(np);
    } else {
        handlechanmsg(cp, np, argv[1]);
    }

    free(unick);
    return 1;
}

/* :metroid!metroid@securenet.eu.org MODE #v1per -o metroid */
int bot_modemsg(char *source, int argc, char **argv) {
    char *unick = getnickfromhostmask(source);
    nick *np = NULL, *tnp;
    chanuser *cup;
    channel *cp;
    chanmode *cmp;
    chanban *cbp;
    int index = 2, dir = 0;
    char *chp;

    if (unick) {
        np = finduser(unick);
        free(unick);
    }

    if (!(cp = findchannel(argv[0])))
        return 0;

    for (chp = argv[1]; *chp; chp++) {
        if (*chp == '+') {
            dir = 1;
            continue;
        }

        if (*chp == '-') {
            dir = 0;
            continue;
        }

        if (*chp == 'o') {
            tnp = finduser(argv[index++]);

            if (!tnp)
                continue;

            if (!(cup = findchanuser(cp, tnp)))
                continue;

            if (dir)
                cup->status |= CUSTAT_OP;
            else
                cup->status &= ~CUSTAT_OP;

            if (tnp == bi.botuser)
                cp->isopped = dir;
        }

        if (*chp == 'v') {
            tnp = finduser(argv[index++]);

            if (!tnp)
                continue;

            if (!(cup = findchanuser(cp, tnp)))
                continue;

            if (dir)
                cup->status |= CUSTAT_VOICE;
            else
                cup->status &= ~CUSTAT_VOICE;
        }

        if (*chp == 'b') {
            cbp = findchanban(cp, argv[index]);

            if (!cbp && dir) {
                cbp = addchanban(cp, argv[index], np ? np->name : NULL, time(NULL));

                for (cup = cp->users; cup; cup = cup->nextbychan)
                    if (!matchban(cbp, cup))
                        /* Ban enforcing code here later */
                        irc_write(QUEUE_NORMAL, "PRIVMSG #v1per :[%s] %s is matched by the ban %s", cp->name, cup->nick->name,
                                  cbp->mask);
            } else if (!cbp && !dir)
                printlog("We just got an unban for a ban (%s) I didn't know! :(", argv[index]);
            else if (cbp && !dir)
                delchanban(cbp);

            index++;
            continue;
        }

        /* else */
        if ((cmp = getchanmode(cp, *chp))) {
            if ((dir && cmp->mode->type == MTYPE_PARAMSET) || (cmp->mode->type == MTYPE_PARAMALWAYS))
                setchanmode(cp, *chp, argv[index++], dir);
            else
                setchanmode(cp, *chp, NULL, dir);
        }
    }

    return 1;
}

/* :PROMI!~ULIX@93.81.137.148 JOIN #v1per */
int bot_joinmsg(char *source, int argc, char **argv) {
    char *unick = getnickfromhostmask(source);
    nick *np;
    channel *cp = findchannel(argv[0]);
    chanmode *cmp;
    modes *emp;
    int i;
    
    np = finduser(unick);
    
    if (np) {
        if (np == bi.botuser) {
            /* I joined a channel */
            printlog("I joined %s", cp->name);
            cp->onchan = 1;
            cp->status &= ~CHSTAT_SENTJOIN; /* Recieved join! */
            
            if (cp->timer)
                cp->timer = addtimer(time(NULL) + 1, checkchannel, (void *)cp->name, TIMER_RECURRING, 90);

            if (!cp->modes) {
                /* Create a new complete list */
                for (i = MTYPE_PARAMALWAYS; i <= MTYPE_PARAMNEVER; i <<= 1) {
                    for (emp = bi.modes; emp; emp = emp->next) {
                        if (emp->type == i) {
                            cmp = (chanmode *)malloc(sizeof(chanmode));

                            cmp->mode = emp;
                            cmp->parameter = NULL;
                            cmp->set = 0;
                            cmp->nextbychan = cp->modes;
                            cp->modes = cmp;
                        }
                    }
                }
            }

            /* Check wether or not we should do a WHO, MODE+(list) */
            if (cp->lastkicked && (time(NULL) - cp->lastkicked) < 30) {
                /* If we were kicked less than 30 seconds ago, don't refresh the entire channel */
                /* Do something to punish the kicker? */
            } else {
                cp->status |= CHSTAT_REFRESH;
                /* HACK */
                irc_write(QUEUE_SLOW, "MODE %s", cp->name);
                modes *mp;
                for (mp = bi.modes; mp; mp = mp->next) {
                    if (mp->type == MTYPE_LIST)
                        irc_write(QUEUE_SLOW, "MODE %s +%c", cp->name, mp->str);
                }
            }

            /* Check if we need to do anything else here */
        }
    } else
        np = addtouser(NULL, unick, gethostfromhostmask(source), NULL);
    
    /* Mark both the user and the channel as needing a WHO refresh */
    if (!np->account) {
        if (cp)
            cp->status |= CHSTAT_REFRESH;
        np->status |= USTAT_REFRESH;
    }
    
    addchanuser(cp, np, 0, time(NULL));
    
    free(unick);
    return 1;
}

/* :sawVector!~gydani92@92-249-222-249.pool.digikabel.hu PART #cod4.wars */
int bot_partmsg(char *source, int argc, char **argv) {
    char *unick = getnickfromhostmask(source);
    nick *np    = finduser(unick);
    channel *cp = findchannel(argv[0]);
    chanuser *cup;
    
    if (!cp || !np)
        return 0;
    
    if (np == bi.botuser) {
        /* I parted a channel, wait what, why?! */
        printlog("I parted %s", cp->name);
        /* Check wether or not we should do rejoin or something else */
        cp->onchan = 0;
        
        /* If no rejoin, kill the timer */
        if (cp->timer)
            remove_timer(cp->timer);
        cp->timer = NULL;
    }
    
    if ((cup = findchanuser(cp, np)))
        delchanuser(cup);
    else
        printlog("Couldn't find chanuser %s on %s", np->name, cp->name);
    
    free(unick);
    return 1;
}

/* :warm0p-nInja!~johan_edb@i116z6cm6.cable.soderhamn-net.com QUIT :Signed off */
int bot_quitmsg(char *source, int argc, char **argv) {
    char *unick = getnickfromhostmask(source);
    nick *np;

    np = finduser(unick);
    
    if (!np)
        return 0;

    deluser(np);

    free(unick);
    return 1;
}

/* :metroid!metroid@securenet.eu.org KICK #v1per m00bot :metroid */
int bot_kickmsg(char *source, int argc, char **argv) {
    channel *cp = findchannel(argv[0]);
    nick *np = finduser(argv[1]);
    chanuser *cup;
    
    if (!cp || !np)
        return 0;
        
    if (np == bi.botuser) {
        printlog("I was just kicked from %s: %s", cp->name, argv[2]);
        cp->lastkicked = time(NULL);
        cp->status |= CHSTAT_NEEDJOIN;
        cp->onchan = 0;

        /* Schedule so we checkchannel in 1 second */
        if (cp->timer)
            remove_timer(cp->timer);
        cp->timer = addtimer(time(NULL) + 1, checkchannel, (void *)cp->name, TIMER_RECURRING, 90);
    }

    if ((cup = findchanuser(cp, np)))
        delchanuser(cup);
    else
        printlog("Couldn't find chanuser %s on %s", np->name, cp->name);

    return 1;
}

int bot_invitemsg(char *source, int argc, char **argv) {
    /* char *unick = getnickfromhostmask(source);
    nick *np = finduser(unick); */
    nick *np2;
    channel *cp;
    
    /* If I'm not the target, ignore it */
    /* Is this even possible? */
    /* Do something with the nickpointer if this guy bothers us too much -- future */
    if (!(np2 = finduser(argv[0])) || !(np2 == bi.botuser))
        return 0;
        
    if ((cp = findchannel(argv[1])) && !cp->onchan) {
        cp->status |= CHSTAT_SENTJOIN;
        irc_write(QUEUE_NORMAL, "JOIN %s", cp->name);
    }
    
    /* free(unick); */
    return 1;
}
