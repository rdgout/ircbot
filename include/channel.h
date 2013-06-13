#ifndef CHANNEL_H
#define CHANNEL_H

/* We read this out from the IRCd too */
#define CHANPREFIX         "#"

/* Channel settings */
#define CHANNELLEN         250
#define KEYLEN             23

/* Channel user status modes */
#define CUSTAT_OP          0x0001     /* User is currently opped */
#define CUSTAT_VOICE       0x0002     /* User is currently voiced */
#define CUSTAT_SENTOP      0x0004     /* We put a mode +o in the queue */
#define CUSTAT_SENTVOICE   0x0008     /* We put a mode +v in the queue */
#define CUSTAT_SENTKICK    0x0010     /* We put a kick in the queue for this user */

/* Channel status modes */
#define CHSTAT_REFRESH     0x0001     /* Refresh the whole channel */
#define CHSTAT_INACTIVE    0x0002     /* Don't bother doing any updates on this channel such as WHO */
#define CHSTAT_SENTREFRESH 0x0004     /* We put a WHO #channel in the queue */
#define CHSTAT_TOPICUPDATE 0x0008     /* We need to update the topic */
#define CHSTAT_NEEDJOIN    0x0010     /* We need to rejoin the channel */
#define CHSTAT_SENTJOIN    0x0020     /* We put a JOIN #channel in the queue */
/* More to come */

typedef struct chan_t {
    char                *name;        /* Read out from the file at first, changed into the IRCds version on join */
    int                  onchan;      /* Wether or not the bot is currently on the channel */
    int                  isopped;     /* Wether or not the bot is currently opped on the channel */
    time_t               lastkicked;  /* When the bot was last kicked (important!) */
    struct chanmode_t   *modes;       /* Channel modes */
    int                  status;
    timer               *timer;       /* This is the timer that checks the channel every so many seconds */
                                      /* Registered channel stuff comes here */
    time_t               createdat;   /* Timestamp we first joined this channel at (may be set by the bot if non-existant) */
    time_t               lastjoin;    /* When we last saw a user joining (note: not updated if the bot joins) */
    int                  flags;       /* Channel flags */

    /* Channel levels */
    struct chanlevel_t  *chanlevels;  /* Chanlevel access entries */
    
    /* Basic stuff again */
    int                  usercount;   /* How many users there are on the channel */
    int                  bancount;    /* How many bans there are on the channel */
    int                  clevcount;   /* How many chanlevel entries there are on the channel */
    struct chanuser_t   *users;       /* What users are on the channel (chanuser* pointers) */
    struct chanban_t    *bans;        /* What channel bans are currently set on the channel */
    struct chan_t       *next;        /* The next channel in the list */
} channel;

typedef struct chanuser_t {
    nick                *nick;        /* The users nick pointer (getting a readable nickname would be chanuser->nick->name) */
    channel             *channel;     /* The channel the user is on (getting a readable channelname would be chanuser->channel->name) */
    int                  status;      /* The status modes for that user */
    time_t               lastmessage; /* Last time we saw a message from that user (will be 0 until we see a message) */
    time_t               chanjoin;    /* When the user joined the channel (will be 0 if we didn't see him join) */
    struct chanuser_t   *nextbynick;
    struct chanuser_t   *nextbychan;
} chanuser;

typedef struct chanmode_t {
    modes               *mode;        /* The mode param (n, t, C, N etc) [in mode->str] */
    char                *parameter;   /* The possible parameter, like a key or a limit */
    int                  set;         /* Wether or not the mode is currently set */
    struct chanmode_t   *nextbychan;
} chanmode;

typedef struct chanban_t {
    channel             *channel;     /* The channel the ban is present in */
    char                *mask;        /* The banmask */
    char                *setby;       /* Who set the ban, will be NULL if set by a server */
    int                  isactive;    /* Wether or not it is currently active or not */
    time_t               timestamp;   /* The time the ban was set (is probably the server boot time due to netsplits) */
    struct chanban_t    *nextbychan;
} chanban;

extern channel *channels;

channel *findchannel(char *name);
channel *addchannel(char *name);
void delchannel(channel *cp);
chanuser *findchanuser(channel *cp, nick *np);
chanuser *findchanuserbychan(channel *cp, nick *np);
chanuser *findchanuserbynick(channel *cp, nick *np);
chanuser *addchanuser(channel *cp, nick *np, int modes, time_t joined);
void delchanuser(chanuser *cup);
chanmode *getchanmode(channel *cp, char mode);
chanmode *setchanmode(channel *cp, char mode, char *parameter, int set);
chanban *findchanban(channel *cp, char *mask);
chanban *addchanban(channel *cp, char *mask, char *setby, time_t setdate);
void delchanban(chanban *cbp);
int matchban(chanban *cbp, chanuser *cup);
char *spewchanmodes(channel *cp);
void checkchannel(void *pointer);

#endif /* CHANNEL_H */
