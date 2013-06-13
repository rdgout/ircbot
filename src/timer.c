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

int timerID = 0;
timer *timers = NULL;

timer *addtimer(time_t time, tmrhandler_t handler, void *arg, int type, int interval) {
    timer *tp, *tp2;

    tp = (timer *)malloc(sizeof(timer));

    tp->ID = ++timerID;
    tp->exectime = time;
    tp->type = type;
    tp->interval = interval;
    tp->arg = arg;
    tp->handler = handler;
    tp->next = NULL;

    if (timers) {
        for (tp2 = timers; tp2; tp2 = tp2->next)
            if (!tp2->next) {
                tp2->next = tp;
                break;
            }
    } else
        timers = tp;

    return tp;
}

void remove_timer(timer *tp) {
    timer *tp2;

    if (tp) {
        tp->arg = NULL;
        tp->type = 0;

        if (tp == timers)
            timers = tp->next;
        else {            
            for (tp2 = timers; tp2; tp2 = tp2->next) {
                if (tp2->next == tp) {
                    tp2->next = tp->next;
                    break;
                }
            }
        }

        free(tp);
    }

}

int getnexttimer() {
    int lowest, current;
    time_t now = time(NULL);
    timer *timer;

    /* By default, assume the first one is 60 seconds */
    lowest = 60;
    /* However, if we have any queue items, we need to change this to 2 seconds */
    if (botqueue)
        lowest = 2;

    for (timer = timers; timer; timer = timer->next) {
        if (timer->exectime > now)
            current = (timer->exectime - now);
        else
            current = (now - timer->exectime);

        if (current < lowest)
            lowest = current;
    }

    return lowest;
}

void checktimers(time_t time) {
    timer *timer, *next;
    void *arg;
    tmrhandler_t func;

    for (timer = timers; timer; timer = next) {
        next = timer->next;

        if (timer->exectime > 0 && timer->exectime <= time) {
            arg = timer->arg;
            func = timer->handler;

            func(arg);

            switch (timer->type) {
                case TIMER_ONCE:
                    remove_timer(timer);
                    break;
                case TIMER_RECURRING:
                    /* Extend the time */
                    timer->exectime += timer->interval;
                    break;
            }
        }

        if (!timer->exectime)
            remove_timer(timer);
    }

}
