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


int handlechanmsg(channel *cp, nick *np, char *arg) {
    int i;
    nick *np2;
    chanuser *cup;
    chanban *cbp;
    command *cmdp;
    int argc, botnick = 0;
    char *argv[20];

    if (!(cup = findchanuser(cp, np)))
        return 0;

    argc = linetoarray(arg, argv, 1, 19);

    cup->lastmessage = time(NULL);

    /* To be removed later */
    if (np->account && np->account->dbuser)
        if (!np->account->dbuser->trigger)
            np->account->dbuser->trigger = '$';
    
    if (!strcasecmp(argv[0], bi.botuser->name) || 
        (np->account && np->account->dbuser && np->account->dbuser->trigger && argv[0][0] == np->account->dbuser->trigger)) {

        /*if (!np->account || (np->account && !np->account->dbuser)) {
            irc_write(QUEUE_WHO, "NOTICE %s :Sorry, you don't have a registered account with me.", np->name);
            return 0;
        }*/

        if (!strcasecmp(argv[0], bi.botuser->name)) {
            botnick = 1;
            cmdp = findcommand(argv[1], CTYPE_PUBLIC);
        } else {
            cmdp = findcommand(argv[0] + 1, CTYPE_PUBLIC);
        }

        if (cmdp) {
            /* Channel and global access checking */
            if (!haspermissions(np, cmdp->level) && !haschanpermissions(np, cp, cmdp->level)) {
                irc_write(QUEUE_SLOW, "NOTICE %s :Sorry, You don't have access on %s to do that.", np->name, cp->name);
                return 0;
            }

            cmdp->handler(np, cp, botnick ? argc - 2 : argc - 1, botnick ? argv + 2 : argv + 1);
        }
    }
    
    if (!strcasecmp(argv[0], "!modes") && np->account && !strcmp("metroid", np->account->name)) {
        irc_write(QUEUE_NORMAL, "NOTICE %s :Modes on %s: %s %s%s%s.", np->name, cp->name, spewchanmodes(cp),
                  getchanmode(cp, 'l')->set ? getchanmode(cp, 'l')->parameter : "",
                  (getchanmode(cp, 'l')->set && getchanmode(cp, 'k')->set) ? " " : "",
                  getchanmode(cp, 'k')->set ? getchanmode(cp, 'k')->parameter : "");
    } else if (!strcasecmp(argv[0], "!op") && np->account && np->account->dbuser) {
        /* First check if we're opped */
        if (!cp->isopped)
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :I am not opped here!", cp->name);
        else if (argc < 2)
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :Specify the victim please.. >:)", cp->name);
        else if (!(np2 = finduser(argv[1])))
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :I don't know that person :(", cp->name);
        else if (!(cup = findchanuser(cp, np2)))
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :%s is not on this channel!", cp->name, np2->name);
        else if (cup->status & (CUSTAT_OP | CUSTAT_SENTOP))
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :%s is already opped on this channel!", cp->name, np2->name);
        else {
            irc_write(QUEUE_NORMAL, "MODE %s +o %s", cp->name, np2->name);
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :Executed.", cp->name, np2->name);
        }
    } else if (!strcasecmp(argv[0], "!bans") && np->account && np->account->dbuser) {
        irc_write(QUEUE_NORMAL, "PRIVMSG %s :%d bans in %s%s", cp->name, cp->bancount, cp->name, cp->bancount ? ":" : ".");
        for (cbp = cp->bans; cbp; cbp = cbp->nextbychan)
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :%s set by %s at %lu", cp->name, cbp->mask, cbp->setby ? cbp->setby : "[unknown]", cbp->timestamp);
    } else if (!strcasecmp(argv[0], "!chanusers") && np->account && np->account->dbuser) {
        char buffer[450];
        char nickspace[NICKLEN + 3];
        
        irc_write(QUEUE_NORMAL, "NOTICE %s :Chanusers in %s:", np->name, cp->name);
        
        for (i = 0, buffer[0] = '\0', cup = cp->users; cup; cup = cup->nextbychan) {

            snprintf(nickspace, sizeof(nickspace), "%s%s%-15s", cup->status & CUSTAT_OP ? "@" : " ",
                     cup->status & CUSTAT_VOICE ? "+" : " ",
                     cup->nick->name);
            
            strcat(buffer, nickspace);
            i++;
            
            if (!cup->nextbychan || i >= 5) {
                irc_write(QUEUE_NORMAL, "NOTICE %s :%s", np->name, buffer);
                buffer[0] = '\0';
                i = 0;
            }
        }
        
    } else if (!strcasecmp(argv[0], "!whois") && argc > 1 && np->account && np->account->dbuser) {
        if (!(np2 = finduser(argv[1]))) {
            irc_write(QUEUE_NORMAL, "NOTICE %s :I don't know %s, are you sure that %s is on a common channel with me?", np->name, argv[1], argv[1]);
        } else if (!np2->account) {
            irc_write(QUEUE_NORMAL, "NOTICE %s :That user does not appear to have an account.", np->name);
        } else {
            nicklist *nlp;
            for (nlp = np2->account->nicks; nlp; nlp = nlp->next)
                irc_write(QUEUE_NORMAL, "NOTICE %s :%s is logged in as %s", np->name, nlp->nick->name, nlp->nick->account->name);
            if (np2->account->dbuser)
                irc_write(QUEUE_NORMAL, "NOTICE %s :Flags: %d, Last seen at %lu", np->name, np2->account->dbuser->flags, np2->account->dbuser->lastseen);
            else
                irc_write(QUEUE_NORMAL, "NOTICE %s :User doesn't have a user record.", np->name);
        }
    } else if (!strcasecmp(argv[0], "!die") && np->account && !strcmp("metroid", np->account->name)) {
        die();        
    } else if (!strcasecmp(argv[0], "!cqueue") && np->account && !strcmp("metroid", np->account->name)) {
        i = clear_queue(QUEUE_NORMAL|QUEUE_SLOW|QUEUE_WHO);
        irc_write(QUEUE_NORMAL, "PRIVMSG %s :%d queue messages cleared.", cp->name, i);
    } else if (!strcasecmp(argv[0], "!adduser") && np->account && !strcmp("metroid", np->account->name)) {
        if (argc < 2)
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :!adduser <nickname>", cp->name);
        else if (!(np2 = finduser(argv[1])))
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :I don't know that person :(", cp->name);
        else if (!np2->account)
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :That user is not authed.", cp->name);
        else if (np2->account->dbuser)
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :That user is already known.", cp->name);
        else if (!(cup = findchanuser(cp, np2)))
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :%s is not on this channel!", cp->name, np2->name);
        else {
            dbuser *dup;
            dup = newdbuser();
            
            np2->account->dbuser = dup;
            dup->account = np2->account;
            
            irc_write(QUEUE_NORMAL, "PRIVMSG %s :%s was added with accountname %s! Try !whoami", cp->name, np2->name, np2->account->name);
        }
    }
    
    return 1;
}

int handleprivmsg(nick *np, char *arg) {
    channel *cp = NULL;
    command *cmdp;
    int argc;
    char *argv[20];

    argc = linetoarray(arg, argv, 1, 19);

    cmdp = findcommand(argv[0], CTYPE_PRIVMSG);

    if (!cmdp)
        return 0;

    if (cmdp && !np->channels) {
        irc_write(QUEUE_WHO, "NOTICE %s :Sorry, I don't know you.", np->name);
        return 0;
    }

    if (argc > 1 && *argv[1] == '#')
        cp = findchannel(argv[1]);

    /* Check if user has access to the channel he's trying to use the command on */
    if (cp) {
        if (!haspermissions(np, cmdp->level) && !haschanpermissions(np, cp, cmdp->level)) {
            irc_write(QUEUE_NORMAL, "NOTICE %s :Sorry, You don't have access on %s to do that.", np->name, cp->name);
            return 0;
        }
    } else {
        if (!haspermissions(np, cmdp->level)) {
            irc_write(QUEUE_NORMAL, "NOTICE %s :Sorry, You don't have access to do that.", np->name);
            return 0;
        }
    }

    if (cmdp)
        cmdp->handler(np, cp, argc - 1, argv + 1);

    return 1;
}

