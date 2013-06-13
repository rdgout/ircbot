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
