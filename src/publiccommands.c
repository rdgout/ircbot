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


int private_hellocmd(nick *np, channel *cp, int argc, char **argv) {
    dbuser *dup = NULL;

    if (!np->account) {
        irc_write(QUEUE_SLOW, "NOTICE %s :sorry, I have no idea who you are.", np->name);
        return 0;
    }

    if (np->account->dbuser) {
        irc_write(QUEUE_SLOW, "NOTICE %s :we have already met. No need to introduce yourself again.", np->name);
        return 0;
    }

    /* Right so this user has an account, let's add him to the database then */
    dup = newdbuser();
    np->account->dbuser = dup;
    dup->account = np->account;

    /* Update lastseen to now */
    dup->lastseen = time(NULL);

    irc_write(QUEUE_SLOW, "NOTICE %s :hi %s, I have added you to my account list with the accountname %s.", np->name, np->name, np->account->name);
    return 1;
}

int public_statuscmd(nick *np, channel *cp, int argc, char **argv) {
    int i;
    bucket *bp;
    nick *np2;
    channel *cp2;
    chanuser *cup;
    int maxchain, curchain;
    int nohost, chans, opped, voiced, peon, bans, dbcount, noaccount;
            
    maxchain = curchain = 0;
    nohost = noaccount = dbcount = chans = opped = voiced = peon = bans = 0;

    for (i = 0; i < nicks->size; i++, curchain = 0) {
        bp = nicks->buckets[i];

        for (;bp; bp = bp->next) {
            np2 = (nick *)bp->pointer;
            if (!np2->userhost)
                nohost++;
                
            if (!np2->account)
                noaccount++;
            else if (np2->account->dbuser)
                dbcount++;

            chans += np2->channelcount;
            for (cup = np2->channels; cup; cup = cup->nextbynick) {
                if (cup->status & CUSTAT_OP)
                    opped++;
                else if (cup->status & CUSTAT_VOICE)
                    voiced++;
                else
                    peon++;
            }

            curchain++;
        }

        if (curchain > maxchain)
            maxchain = curchain;
   }

   irc_write(QUEUE_SLOW, "NOTICE %s :BytesIN: %.2f %s BytesOUT: %.2f %s", np->name,
             (float)(bi.bytesin / 1024.00) > 1024 ? (float)((bi.bytesin / 1024.00) / 1024.00) : (float)(bi.bytesin / 1024.00),
             (bi.bytesin / 1024) > 1024 ? "MB" : "KB",
             (float)(bi.bytesout / 1024.00) > 1024 ? (float)((bi.bytesout / 1024.00) / 1024.00) : (float)(bi.bytesout / 1024.00),
             (bi.bytesout / 1024) > 1024 ? "MB" : "KB");

   irc_write(QUEUE_SLOW, "NOTICE %s :%d nicks (HU: %d/%d (%.2f%%), mchain %d average %.1f) %d no-host, %d are not authed",
             np->name, nicks->count, nicks->bucketcount, nicks->size,
             (float)((nicks->bucketcount * 100.00) / nicks->size), maxchain,
             (float)nicks->count / nicks->bucketcount, nohost, noaccount);

   irc_write(QUEUE_SLOW, "NOTICE %s :%d accounts (HU: %d/%d (%.2f%%), average %.1f) %d db-users",
             np->name, accounts->count, accounts->bucketcount, accounts->size, (float)((accounts->bucketcount * 100.00) / accounts->size),
             (float)accounts->count / accounts->bucketcount, dbcount);
                                    
   for (i = 0, cp2 = channels; cp2; cp2 = cp2->next) {
       bans += cp2->bancount;
       i++;
   }

   irc_write(QUEUE_SLOW, "NOTICE %s :%d channels, %d chanusers (%d op, %d voice and %d peon), %d chanbans",
                                np->name, i, chans, opped, voiced, peon, bans);

    return 1;
}

int public_helpcmd(nick *np, channel *cp, int argc, char **argv) {
    command *tmp, **list;
    int i, j, count, header, state;
    bucket *bp;
    char buffer[512], commandspace[512];

    list = (command **)malloc(commands->count * sizeof(command *));

    for (i = 0, j = 0, buffer[0] = '\0'; i < commands->size; i++) {
        bp = commands->buckets[i];

        if (bp) {
            for (; bp; bp = bp->next) {
                tmp = (command *)bp->pointer;

                if (tmp->type & (CTYPE_PUBLIC | CTYPE_PRIVMSG))
                    if (haspermissions(np, tmp->level) || haschanpermissions(np, cp, tmp->level))
                        list[j++] = tmp;
            }
        }
    }

    count = header = state = 0;
    for (i = 0; i < j;) {
        if (!state && (list[i]->type & CTYPE_PRIVMSG) && !(list[i]->type & CTYPE_PUBLIC)) {
            snprintf(commandspace, sizeof(commandspace), "%s%-15s", count ? " " : "", list[i]->commandname);
            strcat(buffer, commandspace);
            count++;
        } else if (state && ((list[i]->type & CTYPE_PRIVMSG) || (list[i]->type & CTYPE_PUBLIC))) {
            snprintf(commandspace, sizeof(commandspace), "%s%-15s", count ? " " : "", list[i]->commandname);
            strcat(buffer, commandspace);
            count++;
        }

        if (++i == j || strlen(buffer) > 400 || count >= 7) {

            if (!header && count) {
                if (!state)
                    irc_write(QUEUE_SLOW, "NOTICE %s :available private commands: (to be used with /msg %s <command>)",
                              np->name, bi.botuser->name);
                else
                    irc_write(QUEUE_SLOW, "NOTICE %s :available public commands: (%s <command> or %c<command> or /msg %s <command>)",
                              np->name, bi.botuser->name, (np->account && np->account->dbuser && np->account->dbuser->trigger) ? np->account->dbuser->trigger : '$',
                              bi.botuser->name);
                header = 1;
            }

            if (strlen(buffer))
                irc_write(QUEUE_SLOW, "NOTICE %s :%s", np->name, buffer);
            buffer[0] = '\0';
            count = 0;

            if (i == j && !state) {
                i = header = 0; /* Reset the loop and run through with public commands now */
                state = 1;
            }
        }
    }

    free(list);
    return 1;
}

int public_userlistcmd(nick *np, channel *cp, int argc, char **argv) {
    account *tmp, **list;
    nicklist *nlp;
    bucket *bp;
    chanlevel *clp;
    char buffer[512], formatspace[512];
    int i, j, x, count, state, nicks;
    
    i = j = x = count = state = 0;
    
    if (!argc && CAtleastVoice(np->account->dbuser)) /* So the user can safely request the global userlist too */
        cp = NULL;
    
    if (!cp && !CAtleastVoice(np->account->dbuser)) {
        irc_write(QUEUE_NORMAL, "NOTICE %s :try userlist <#channel>", np->name);
        return 0;
    } else if (cp && (((clp = findchanlevel(cp, np->account)) && CAtleastKnown(clp)) || (CAtleastOp(np->account->dbuser)))) {
        /* View the userlist for a channel */
        printlog("Userlist - channel %s", cp->name);
        for (i = DBF_OWNER; i; i >>= 1) {

            buffer[0] = '\0';
            count = 0;
            for (clp = cp->chanlevels; clp; clp = clp->nextbychan) {
                if ((clp->flags & i) && !(clp->account->status & USTAT_ISMARKED)) {
                    if (clp->account->loggedin) {
                        nicks = 0;
                        for (nlp = clp->account->nicks; nlp; nlp = nlp->next) {
                            snprintf(formatspace, sizeof(formatspace), "%s%s%s%s%s", (count++ && !nicks) ? " " : "", !nicks ? "[" : "", nicks ? ", " : "", nlp->nick->name, (nlp->next == NULL) ? "]" : "");
                            strcat(buffer, formatspace);
                        }
                    } else {
                        snprintf(formatspace, sizeof(formatspace), "%s[#%s]", count++ ? " " : "", clp->account->name);
                        strcat(buffer, formatspace);
                    }
                    clp->account->status |= USTAT_ISMARKED;
                }
                
                if ((count > 10) || (count && strlen(buffer))) {
                    if (!state) {
                        irc_write(QUEUE_SLOW, "NOTICE %s :userlist for %s:", np->name, cp->name);
                        state = 1;
                    }
                    irc_write(QUEUE_SLOW, "NOTICE %s :channel %s%s: %s", np->name, findtype(i), (count == 1) ? "" : "s", buffer);
                    count = 0;
                }
            }

        }

        for (clp = cp->chanlevels; clp; clp = clp->nextbychan)
            clp->account->status &= ~USTAT_ISMARKED;
    } else {
        printlog("Userlist - global");
        /* View the global bot userlist */
    
        /* create a list to list global users incase the bot is swimming in accounts,
           first count the amount of accounts with dbusers attached (to avoid a bunch of realloc) */
        for (i = 0; i < accounts->size; i++) {
            bp = accounts->buckets[i];

            for (;bp; bp = bp->next) {
                tmp = (account *)bp->pointer;

                if (tmp->dbuser)
                    if ((CAtleastOp(np->account->dbuser) && CAtleastKnown(tmp->dbuser)) || (!CAtleastOp(np->account->dbuser) && CAtleastVoice(tmp->dbuser)))
                        count++;
            }
        }

        list = (account **)malloc(count * sizeof(account *));

        for (i = 0, j = 0; i < accounts->size; i++) {
            bp = accounts->buckets[i];

            for (;bp; bp = bp->next) {
                tmp = (account *)bp->pointer;

                if (tmp->dbuser)
                    /* Show all users if you're an operator or show 'voiced' people when you're below operator */
                    if ((CAtleastOp(np->account->dbuser) && CAtleastKnown(tmp->dbuser)) || (!CAtleastOp(np->account->dbuser) && CAtleastVoice(tmp->dbuser)))
                        list[j++] = tmp;
            }
        }

        for (i = DBF_OWNER; i < DBF_KNOWN; i <<= 1) {

            count = 0;
            buffer[0] = '\0';

            for (x = 0; x < j;) {
                tmp = list[x];
                if (tmp->dbuser && (tmp->dbuser->flags & i) && !(tmp->status & USTAT_ISMARKED)) {
                    if (tmp->loggedin) {
                        nicks = 0;
                        for (nlp = tmp->nicks; nlp; nlp = nlp->next, nicks++) {
                            snprintf(formatspace, sizeof(formatspace), "%s%s%s%s%s", (count++ && !nicks) ? " " : "", !nicks ? "[" : "", nicks ? ", " : "", nlp->nick->name, (nlp->next == NULL) ? "]" : "");
                            strcat(buffer, formatspace);
                        }
                    } else {
                        snprintf(formatspace, sizeof(formatspace), "%s[#%s]", count++ ? " " : "", tmp->name);
                        strcat(buffer, formatspace);
                    }
                    tmp->status |= USTAT_ISMARKED;
                    count++;
                }

                if (((++x == j) && strlen(buffer)) || strlen(buffer) > 400 || count > 10) {
                    if (!state) {
                        irc_write(QUEUE_SLOW, "NOTICE %s :global userlist:", np->name);
                        state = 1;
                    }
                    irc_write(QUEUE_SLOW, "NOTICE %s :%s%s: %s", np->name, findtype(i), (count == 1) ? "" : "s", buffer);
                }

            }
        }

        for (x = 0; x < j; x++)
            list[x]->status &= ~USTAT_ISMARKED;

        free(list);
    }
    
    
    return 1;
}

int public_whoiscmd(nick *np, channel *cp, int argc, char **argv) {
    nick *target;
    chanlevel *clp;

    if (!np->account || !np->account->dbuser) {
        irc_write(QUEUE_SLOW, "NOTICE %s :sorry, I do not know you yet. Try introducing yourself first.", np->name);
        return 0;
    }

    if (!argc) {
        irc_write(QUEUE_SLOW, "NOTICE %s :try whois <nickname>", np->name);
        return 0;
    }

    if (!(target = finduser(argv[0]))) {
        irc_write(QUEUE_SLOW, "NOTICE %s :I do not know %s.", np->name, argv[0]);
        return 0;
    }

    if (!target->account) {
        irc_write(QUEUE_SLOW, "NOTICE %s :%s is not logged in.", np->name, target->name);
        return 0;
    }

    if (target->account->loggedin > 1) {
        irc_write(QUEUE_SLOW, "NOTICE %s :%d users are logged in as %s", np->name, target->account->loggedin, target->account->name);
    } else {
        irc_write(QUEUE_SLOW, "NOTICE %s :%s is logged in as %s", np->name, target->name, target->account->name);
    }
    
    if (target->account->dbuser) {

        if (target->account->dbuser->flags)
            irc_write(QUEUE_SLOW, "NOTICE %s :global %-10s [Last seen: %lu]", np->name, findtype(target->account->dbuser->flags), target->account->dbuser->lastseen);
        else
            irc_write(QUEUE_SLOW, "NOTICE %s :last seen %lu", np->name, target->account->dbuser->lastseen);

        for (clp = target->account->dbuser->chanlevels; clp; clp = clp->nextbyaccount)
            irc_write(QUEUE_SLOW, "NOTICE %s :access on %s: %s [Last seen: %lu]", np->name, clp->channel->name, findtype(clp->flags), clp->lastseen);
    } else
        irc_write(QUEUE_SLOW, "NOTICE %s :%s is not registered user.", np->name, target->name);

    return 1;
}

int public_kickcmd(nick *np, channel *cp, int argc, char **argv) {
    nick *target;
    chanuser *cup;
    
    if (!cp)
        return 0;

    if (!cp->onchan || !cp->isopped) {
        irc_write(QUEUE_SLOW, "NOTICE %s :I am currently not on %s or I am not opped.", np->name, cp->name);
        return 0;
    }

    if (!argc) {
        irc_write(QUEUE_SLOW, "NOTICE %s :no target specified.", np->name);
        return 0;
    }

    if (!(target = finduser(argv[0])) || !(cup = findchanuser(cp, target))) {
        irc_write(QUEUE_SLOW, "NOTICE %s :%s is not on %s.", np->name, argv[0], cp->name);
        return 0;
    }

    if (target == bi.botuser) {
        irc_write(QUEUE_SLOW, "NOTICE %s :I am not going to remove myself from %s.", np->name, cp->name);
        return 0;
    }

    irc_write(QUEUE_NORMAL, "KICK %s %s :%s", cp->name, target->name, argc > 1 ? argv[1] : "Get lost.");
    irc_write(QUEUE_SLOW, "NOTICE %s :%s removed from %s.", np->name, target->name, cp->name);
    return 1;
}

int public_addchancmd(nick *np, channel *cp, int argc, char **argv) {
    channel *targetchan;

    if (*argv[0] != '#') {
        irc_write(QUEUE_NORMAL, "NOTICE %s :please use an '#' infront of the channel name.", np->name);
        return 0;
    } else if ((targetchan = findchannel(argv[0]))) {
        irc_write(QUEUE_NORMAL, "NOTICE %s :I'm already in %s.", np->name, targetchan->name);
            return 0;
    } else {
        targetchan = addchannel(argv[0]);
        targetchan->createdat = time(NULL);

        irc_write(QUEUE_NORMAL, "JOIN %s", targetchan->name);
        irc_write(QUEUE_NORMAL, "NOTICE %s :added %s to the channel list.", np->name, targetchan->name);
        
        /* save database for channels and users */
        savechandb(getconf("chanfile", "etc/ircbot.chanfile"));
        saveuserdb(getconf("userfile", "etc/ircbot.userfile"));
    }
    
    return 1;
}

int public_delchancmd(nick *np, channel *cp, int argc, char **argv) {
    channel *targetchan;

    if (*argv[0] != '#') {
        irc_write(QUEUE_NORMAL, "NOTICE %s :please use an '#' infront of the channel name.", np->name);
        return 0;
    } else if (!(targetchan = findchannel(argv[0]))) {
        irc_write(QUEUE_NORMAL, "NOTICE %s :I'm not in %s.", np->name, argv[0]);
        return 0;
    } else {

        if (targetchan->onchan)
            irc_write(QUEUE_NORMAL, "PART %s", targetchan->name);

        irc_write(QUEUE_NORMAL, "NOTICE %s :removed %s from the channel list.", np->name, targetchan->name);

        delchannel(targetchan);
        
        /* save database for channels and users */
        savechandb(getconf("chanfile", "etc/ircbot.chanfile"));
        saveuserdb(getconf("userfile", "etc/ircbot.userfile"));
    }

    return 1;
}

int public_savedbcmd(nick *np, channel *cp, int argc, char **argv) {
    int cfi, ufi;
    
    cfi = ufi = 0;
    
    cfi = savechandb(getconf("chanfile", "etc/ircbot.chanfile"));
    ufi = saveuserdb(getconf("userfile", "etc/ircbot.userfile"));
    irc_write(QUEUE_NORMAL, "NOTICE %s :%d channels, %d users saved.", np->name, cfi, ufi);
    
    return 1;
}

int public_clearqueuecmd(nick *np, channel *cp, int argc, char **argv) {
    int cleared = 0;
    
    cleared = clear_queue(QUEUE_NORMAL|QUEUE_SLOW|QUEUE_WHO);
    irc_write(QUEUE_NORMAL, "NOTICE %s :%d queue messages cleared.", np->name, cleared);

    return 1;
}

int public_rehashcmd(nick *np, channel *cp, int argc, char **argv) {
    int loaded = 0;

    irc_write(QUEUE_NORMAL, "NOTICE %s :reloading configuration file.", np->name);
    loaded = loadconf(NULL);
    irc_write(QUEUE_NORMAL, "NOTICE %s :loaded %d configuration items.", np->name, loaded);

    return 1;
}

int public_triggercmd(nick *np, channel *cp, int argc, char **argv) {

    if (!argc) {
        irc_write(QUEUE_SLOW, "NOTICE %s :try trigger <special char> (!, -, $)", np->name);
        irc_write(QUEUE_SLOW, "NOTICE %s :current trigger: %c", np->name, np->account->dbuser->trigger);
        return 0;
    } else if (argv[0][0] >= 33 && argv[0][0] <= 46) {
        np->account->dbuser->trigger = argv[0][0];
        irc_write(QUEUE_SLOW, "NOTICE %s :changed trigger to %c.", np->name, np->account->dbuser->trigger);

        /* save the dbs because we want to make sure we don't forget this trigger change */
        savechandb(getconf("chanfile", "etc/ircbot.chanfile"));
        saveuserdb(getconf("userfile", "etc/ircbot.userfile"));
    } else {
        irc_write(QUEUE_SLOW, "NOTICE %s :please use a special character (!, -, $)", np->name);
        return 0;
    }

    return 1;
}

int public_moocmd(nick *np, channel *cp, int argc, char **argv) {
    int i, maxchar = random()%30;
    short mx;
    char moo[31];

    /* make sure we have atleast 4 characters */
    if (maxchar < 7)
        maxchar = 7;

    for (i = 0; i <= maxchar; i++) {
    
        mx = random()%3;
        if (mx == 1)
            moo[i] = 'o';
        else if (mx == 2)
            moo[i] = 'O';
        else
            moo[i] = '0';
    }

    moo[i++] = '\0';

    irc_write(QUEUE_WHO, "PRIVMSG %s :%s: m%s", cp->name, np->name, moo);

    return 1;
}

void initregcmds() {
    /* PRIVMSG (only) commands */
    addcommand("hello", GMASK_UNKNOWN, CTYPE_PRIVMSG, NULL, private_hellocmd);
    /* PUBLIC and PRIVMSG commands */
    /* admin commands */
    addcommand("addchan", GMASK_MASTER, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_addchancmd);
    addcommand("delchan", GMASK_MASTER, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_delchancmd);
    addcommand("savedb", GMASK_OWNER, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_savedbcmd);
    addcommand("cqueue", GMASK_OWNER, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_clearqueuecmd);
    addcommand("rehash", GMASK_OWNER, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_rehashcmd);
    addcommand("status", GMASK_OWNER, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_statuscmd);
    
    /* peon commands */
    addcommand("help", GMASK_ALL, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_helpcmd);
    
    /* reguser commands */
    addcommand("moo", CMASK_KNOWN | GMASK_VOICE, CTYPE_PUBLIC, NULL, public_moocmd);
    addcommand("kick", CMASK_OP | GMASK_OP, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_kickcmd);
    addcommand("userlist", CMASK_VOICE | GMASK_VOICE, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_userlistcmd);
    addcommand("whois", GMASK_ALL, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_whoiscmd);
    addcommand("trigger", GMASK_ACCOUNT, CTYPE_PUBLIC | CTYPE_PRIVMSG, NULL, public_triggercmd);

}
