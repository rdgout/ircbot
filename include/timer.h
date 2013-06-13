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
