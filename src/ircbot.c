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

char *curbuffer;
int cursize;
timer *saveDB = NULL;

/* Not working in the current version */
int debuglevel = 0;

int main(int argc, char **argv) {
    int allowed = 0, i, daemonize = 0;
    int res, size;
    fd_set readfd, writefd;
    time_t now, lastwrite = 0, connect = 0;
    queue *qp, *next;
    struct timeval tv;
    char op = 0;
    char tmpbuf[BUFSIZE + 1];

    while((op = getopt(argc, argv, "dhv")) != EOF) {
        switch (op) {
            case 'd':
                daemonize = !daemonize;
                break;
            case 'v':
                debuglevel++;
                break;
            case 'h':
            default:
                spew_usage(argv[0]);
                break;
        }
    }

    /* Seed rand */
    srand((unsigned int)time(NULL));

    /* Puts us in the background if it's required */
    if (daemonize)
        daemon(1, 0);

    /* Set up the hashtables */
    commands = newhtable(COMMANDHASHSIZE, 0);
    nicks = newhtable(NICKHASHSIZE, 1);
    accounts = newhtable(ACCOUNTHASHSIZE, 1);

    loadconf("etc/ircbot.conf");

    /* Load our channel data */
    loadchandb(getconf("chanfile", "etc/ircbot.chanfile"));
    /* Load our user data here */
    loaduserdb(getconf("userfile", "etc/ircbot.userfile"));

    /* Now set up the servercommands */
    initservercmds();
    /* Set up the regular user commands here */
    initregcmds();

    /* Save our databases hourly */
    saveDB = addtimer(time(NULL) + (60 * 60), savedatabases, NULL, TIMER_RECURRING, 60 * 60);

    while(1) {
        now = time(NULL);

        /* This makes sure we try to connect right away, it will also make sure we stay connected */
        if (!connected) {
            if (!connect)
                connect = now;

            if (connect <= now) {

                /* Clear the botinfo struct */
                clear_botinfo();

                connected = establish_connection();

                if (!connected)
                    /* If we were not able to connect, try again in 60 seconds */
                    connect += 60;
            }
        }

        /* Check if we need to execute any timers, which incidently might add some */
        /* queue data which is why we do this before sending anything */
        checktimers(now);

        FD_ZERO(&readfd);
        FD_SET(sock, &readfd);
        FD_ZERO(&writefd);
        FD_SET(sock, &writefd);
        
        tv.tv_sec = getnexttimer();
        tv.tv_usec = 0;

        /* Check if we need to write anything from the queue */
        /* Work out how many messages we should send this round */
        if (!lastwrite || (now - lastwrite) >= 10)
            allowed += 4; /* Send 4 (more) messages from the queue to the ircd */
        else if ((now - lastwrite) >= 2)
            allowed++; /* Our last message was 2 or more seconds ago, allow 1 more */

         /* If we have some space left over from last time, make sure we don't use more than 4 (EXCESS FLOOD is _bad_) */
        if (allowed > 4)
            allowed = 4;

        res = select(FD_SETSIZE, &readfd, (allowed && botqueue) ? &writefd : NULL, NULL, &tv);

        if (res != -1) {
            if (FD_ISSET(sock, &readfd)) {
                /* Read out the socket */
                size = read(sock, tmpbuf, BUFSIZE - 1);
                bi.bytesin += size;

                curbuffer = (char *)realloc(curbuffer, cursize + size);

                memcpy(curbuffer + cursize, tmpbuf, size);
                cursize += size;

                /* Process the data in the buffer */
                processbuffer();
            }

            if (FD_ISSET(sock, &writefd)) {
                /* Start with the most important type and work our way down to the less important messages */
                for (i = QUEUE_SERVER; i && allowed; i >>= 1) {
                    for (qp = botqueue; qp && allowed; qp = next) {
                        next = qp->next;

                        if (qp->type == i) {
                            allowed--;
                            lastwrite = now;

                            printlog(">> %s", qp->message);
                            bi.bytesout += write(sock, qp->message, strlen(qp->message));

                            remove_queue(qp);
                        }
                    }
                }
            }
        } else {
            /* socket died */
            connected = 0;
            connect = now + 5;
            printlog("Disconnected from the IRC server.");
        }
    }
    
    close(sock);
    
    exit(1);
}

void spew_usage(char *name) {
    printf("Usage: %s [-dhv]\n"
           "Where the following switches are valid:\n"
           "  d: do not daemonize and run in the foreground\n"
           "  h: display this help\n"
           "  v: increase the debug level (may be stacked)\n"
           , name);
    /* Exit out so the program doesn't run with a false switch */
    exit(1);
}
                       

void clear_botinfo() {
    int dbload = 0;
    
    if (bi.dbload)
        dbload = bi.dbload;
        
    if (bi.botuser)
        deluser(bi.botuser);
        
    if (bi.servername)
        free(bi.servername);
                
    memset(&bi, 0, sizeof(bi));
    bi.botuser = NULL;
    bi.servername = NULL;
    
    bi.bytesin = 0;
    bi.bytesout = 0;
    bi.connected = 0;
    bi.dbload = dbload;
    
    /* Set the various settings to some silly defaults */
    bi.maxmodes = 2;
    bi.maxbans = 30;
    bi.maxchanlen = 100;
    bi.maxtopiclen = 100;
    
    if (bi.modes)
        clearmodes();
    
    bi.modes = NULL;
}

void printlog(char *format, ...) {
    struct tm *tm;
    time_t timestamp = time(NULL);
    static char timebuf[30];
    va_list args;
    char    buf[1024];

    va_start(args, format);
    vsnprintf(buf, 1021, format, args);
    va_end(args);
    
    if (buf[strlen(buf) - 1] != '\n')   
        strcat(buf, "\r\n");

    tm = gmtime(&timestamp);
    strftime(timebuf, 29, "%d-%m-%Y %H:%M:%S", tm);
    
    printf("%s %s", timebuf, buf);
}

void die() {
    int i;
    bucket *bp, *next;
    command *cmdp;
    channel *cp, *nextcp;
    timer *timer, *nexttimer;

    char *reason = "QUIT :Request.\r\n";

    /* HACK: We can't use irc_write because we also use clear_queue here */
    write(sock, reason, strlen(reason));

    /* Free all conf settings */
    freeconf();

    /* Kill all timers */
    for (timer = timers; timer; timer = nexttimer) {
        nexttimer = timer->next;
        remove_timer(timer);
    }

    /* Clear the botinfo, including our own nick * pointer */
    clear_botinfo();
    clearmodes();

    /* Delete all users */
    for (i = 0; i < nicks->size; i++) {
        bp = nicks->buckets[i];

        for (;bp; bp = next) {
            next = bp->next;
            deluser((nick *)bp->pointer);
        }
    }
    
    /* Delete all left over accounts (shouldn't be any except for dbusers) */
    for (i = 0; i < accounts->size; i++) {
        bp = accounts->buckets[i];

        for (;bp; bp = next) {
            next = bp->next;
            delaccount((account *)bp->pointer);
        }
    }
    
    /* Delete all commands */
    for (i = 0; i < commands->size; i++) {
        bp = commands->buckets[i];

        for (;bp; bp = next) {
            next = bp->next;
            cmdp = (command *)bp->pointer;
            delcommand(cmdp->commandname, cmdp->type, cmdp->srvhandler, cmdp->handler);
        }
    }
    
    /* Free the hashtables */
    freehtable(nicks);
    freehtable(accounts);
    freehtable(commands);
    
    /* Delete all channels */
    for (cp = channels; cp; cp = nextcp) {
        nextcp = cp->next;
        delchannel(cp);
    }
    /* Clear all queue messages */
    clear_queue(QUEUE_NORMAL|QUEUE_SLOW|QUEUE_WHO);
    
    /* Close the connection socket and kill the process */
    close(sock);
    exit(1);
}
