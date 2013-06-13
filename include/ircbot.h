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
