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

#ifndef IRC_H
#define IRC_H

/* Various IRC settings */
#define MAXLINELEN     512
#define BUFSIZE        20 * MAXLINELEN /* Read 20 lines at once max */

/* Queue definitions */
#define MAX_QUEUE      200

#define QUEUE_SERVER   0x08
#define QUEUE_NORMAL   0x04
#define QUEUE_SLOW     0x02
#define QUEUE_WHO      0x01

/* Channel mode types */
#define MTYPE_LIST         0x01
#define MTYPE_PARAMALWAYS  0x02
#define MTYPE_PARAMSET     0x04
#define MTYPE_PARAMNEVER   0x08

extern int sock;
extern int connected;

typedef struct queue_t {
    int type;
    char *message;
    struct queue_t *next;
} queue;

extern queue *botqueue;

/* The struct that contains the valid modes defined by the IRCd */
typedef struct modes_t {
    char                 str;
    int                  type;
    struct modes_t      *next;
} modes;

int irc_connect(char *hostname, int port);
int establish_connection();
int irc_write(int type, char *format, ...);
void processbuffer();
int parseline(char *buf);
void initservercmds();
int clear_queue(int type);
void remove_queue(queue *qp);
modes *addmode(char mode, int type);
void clearmodes();

#endif /* IRC_H */
