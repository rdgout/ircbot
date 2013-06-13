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

#ifndef TIMER_H
#define TIMER_H

#define TIMER_ONCE       0x01
#define TIMER_RECURRING  0x02

typedef void (*tmrhandler_t)(void *cookie);

typedef struct timer_t {
    int ID;
    time_t exectime;
    int type;
    int interval;
    void *arg;
    tmrhandler_t handler;
    struct timer_t *next;
} timer;

extern int timerID;
extern timer *timers;

timer *addtimer(time_t time, tmrhandler_t handler, void *arg, int type, int interval);
void remove_timer(timer *tp);
int getnexttimer();
void checktimers();

#endif /* TIMER_H */
