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

#ifndef IRCBOT_H
#define IRCBOT_H

extern char  *curbuffer;
extern int    cursize;
extern timer *saveDB;

void spew_usage(char *name);
void printlog(char *format, ...);
void clear_botinfo();
void die();

/* BotInfo struct, contains local info important to the bot */
struct {
    nick   *botuser;
    char   *servername;
    time_t  connected;
    long    bytesin;
    long    bytesout;
    int     dbload;
    int     maxmodes;
    int     maxnicklen;
    int     maxbans;
    int     maxchanlen;
    int     maxtopiclen;
    modes  *modes;
} bi;

#endif /* IRCBOT_H */
