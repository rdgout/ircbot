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

int sock;
int connected;
queue *botqueue = NULL;

int irc_connect(char *hostname, int port) {
    struct sockaddr_in  saddr;
    struct in_addr     *peer;
    struct hostent     *hent;
    
    printlog("Attempting to connect to %s:%d", hostname, port);

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printlog("Failed to open socket.");
        exit(1);
    }

    if (!(hent = gethostbyname(hostname))) {
        printlog("Failed to resolve hostname.");
        return 0;
    }

    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(port);

    peer = (struct in_addr *)hent->h_addr_list[0];

    saddr.sin_addr.s_addr = peer->s_addr;

    if (connect(sock, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
        printlog("Failed to connect to %s:%d.", inet_ntoa(saddr.sin_addr), port);
        return 0;
    }

    return 1;
}

int establish_connection() {
    /* For connecting */
    char *hostname, *port;
    /* For introduction purposes */
    char *nickname, *username, *realname;

    hostname = getconf("server", NULL);
    port     = getconf("port", "6667");

    nickname = getconf("nickname", NULL);
    username = getconf("username", NULL);
    realname = getconf("realname", NULL);

    if (!hostname || !nickname || !username || !realname) {
        printlog("You are missing some variables from the configuration file, "
               "please check the configuration file for the missing values of: %s%s%s%s%s%s%s",
               !hostname ? "server" : "", !hostname ? " " : "", !nickname ? "nickname" : "", 
               !nickname ? " " : "", !username ? "username" : "", !username ? " " : "", 
               !realname ? "realname" : "");
        exit(1);
    }

    if (irc_connect(hostname, atoi(port))) {
        /* If we connected, lets introduce ourselves to the server */
        
        /* Only copy the nickname until we use USERHOST on the server to find out our real host */
        bi.botuser = addtouser(NULL, nickname, NULL, NULL);
        
        irc_write(QUEUE_SERVER, "USER %s 127.0.0.1 localhost :%s", username, realname);
        irc_write(QUEUE_SERVER, "NICK %s", nickname);

        return 1;
    }

    return 0;
}

int irc_write(int type, char *format, ...) {
    queue *qp, *qp2;
    va_list args;
    char    buf[512];

    va_start(args, format);
    vsnprintf(buf, 509, format, args);
    va_end(args);

    strcat(buf, "\r\n");

    printlog("adding queue entry (%d): %s", type, buf);

    /* Make a queue entry */
    qp = (queue *)malloc(sizeof(queue));
    qp->type = type;
    qp->message = strdup(buf);
    qp->next = NULL;
    
    if (!botqueue)
        botqueue = qp;

    /* Place at the back of the queue */        
    for (qp2 = botqueue; qp2; qp2 = qp2->next) { 
        if (!qp2->next && qp2 != qp) {
            qp2->next = qp;
            break;
        }
    }

    return 1;
}

void processbuffer() {
    int i;
    char *copy, *buf = curbuffer;

    for (i = 0; i < cursize; i++) {
        if (curbuffer[i] == '\n' || curbuffer[i] == '\r') {
            curbuffer[i] = '\0';

            if (*buf != '\0')
                parseline(buf);

            buf = &(curbuffer[i + 1]);
        }
    }

    copy = (char *)malloc(curbuffer + cursize - buf);
    memcpy(copy, buf, curbuffer + cursize - buf);
    cursize = curbuffer + cursize - buf;
    
    free(curbuffer);
    curbuffer = copy;
}

int parseline(char *buf) {
    int argc;
    char *argv[30];
    char *buf2;
    command *cmd;

    buf2 = strdup(buf);

    argc = linetoarray(buf, argv, 1, 30);
    
//    if (strcmp(argv[1], "372"))
//        printlog("<< %s", buf2);
    free(buf2);

    if (!strcmp(argv[0], "PING")) {
        irc_write(QUEUE_SERVER, "PONG %s", argv[1]);
        return 1;
    }

    if ((cmd = findcommand(argv[1], CTYPE_SERVER)) && cmd->srvhandler)
        cmd->srvhandler(argv[0], argc - 2, argv + 2);

    return 0;
}

void initservercmds() {
    /* Numeric "commands" here */
    addcommand("001", 0, CTYPE_SERVER, bot_initserver, NULL); /* Connected to server */
    addcommand("005", 0, CTYPE_SERVER, bot_initsettings, NULL); /* Known settings on IRCd */
    addcommand("302", 0, CTYPE_SERVER, bot_userhost, NULL); /* Our host */
    addcommand("324", 0, CTYPE_SERVER, bot_getmodes, NULL); /* /MODE */
    addcommand("353", 0, CTYPE_SERVER, bot_names, NULL); /* /NAMES */
    addcommand("354", 0, CTYPE_SERVER, bot_whoreply, NULL); /* custom /WHO reply */
    addcommand("366", 0, CTYPE_SERVER, bot_endnames, NULL); /* End of /NAMES */
    addcommand("367", 0, CTYPE_SERVER, bot_getban, NULL); /* /MODE +b */
    addcommand("433", 0, CTYPE_SERVER, bot_nicktaken, NULL); /* nick "..." has been taken */

    /* Word "commands" here */
    addcommand("PRIVMSG", 0, CTYPE_SERVER, bot_privmsg, NULL);
    addcommand("MODE", 0, CTYPE_SERVER, bot_modemsg, NULL);
    addcommand("JOIN", 0, CTYPE_SERVER, bot_joinmsg, NULL);
    addcommand("PART", 0, CTYPE_SERVER, bot_partmsg, NULL);
    addcommand("QUIT", 0, CTYPE_SERVER, bot_quitmsg, NULL);
    addcommand("KICK", 0, CTYPE_SERVER, bot_kickmsg, NULL);
    addcommand("INVITE", 0, CTYPE_SERVER, bot_invitemsg, NULL);
    addcommand("NICK", 0, CTYPE_SERVER, bot_nickmsg, NULL);

    /* All done! */
}

int clear_queue(int type) {
    int i = 0;
    queue *qp, *next;

    for (qp = botqueue; qp; qp = next) {
        next = qp->next;
        
        if (qp->type & type) {
            i++;
            remove_queue(qp);
        }
    }

    return i;
}

void remove_queue(queue *qp) {
    queue *qp2;

    if (qp) {
        if (qp->message)
            free(qp->message);
        qp->message = NULL;
        qp->type = 0;

        if (qp == botqueue)
            botqueue = qp->next;
        else {            
            for (qp2 = botqueue; qp2; qp2 = qp2->next) {
                if (qp2->next == qp) {
                    qp2->next = qp->next;
                    break;
                }
            }
        }

        free(qp);
    }
}

modes *addmode(char mode, int type) {
    modes *nmp, *emp; /* newmodepointer, existingmodepointer */

    for (emp = bi.modes; emp; emp = emp->next)
        /* This is case sensitive */
        if (emp->str == mode)
            return emp;

    /* Didn't find this mode before, so lets create a pointer for it */
    nmp = (modes *)malloc(sizeof(modes));

    nmp->str  = mode;
    nmp->type = type;
    nmp->next = bi.modes;
    bi.modes  = nmp;

    return nmp;
}

void clearmodes() {
    modes *mode, *next;

    for (mode = bi.modes; mode; mode = next) {
        next = mode->next;

        free(mode);
    }
}

