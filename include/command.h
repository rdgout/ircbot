#ifndef COMMAND_H
#define COMMAND_H

/* This should be increased to the closest amount of commands you have to make lookup fastest */
#define COMMANDHASHSIZE 64

#define CTYPE_SERVER    0x01
#define CTYPE_PRIVMSG   0x02
#define CTYPE_PUBLIC    0x04

/* Channel Permissions _UNSAFE_ */
#define CLEVEL_ALL      0x0001
#define CLEVEL_KNOWN    0x0002
#define CLEVEL_VOICE    0x0004
#define CLEVEL_OP       0x0008
#define CLEVEL_MASTER   0x0010
#define CLEVEL_OWNER    0x0020
/* Global permissions _UNSAFE_ */
#define GLEVEL_ALL      0x0040
#define GLEVEL_ACCOUNT  0x0080
#define GLEVEL_KNOWN    0x0100
#define GLEVEL_VOICE    0x0200
#define GLEVEL_OP       0x0400
#define GLEVEL_MASTER   0x0800
#define GLEVEL_OWNER    0x1000
#define GLEVEL_UNKNOWN  0x2000

/* These are used for the commands */
/* Channel permission masks */
#define CMASK_KNOWN     (GLEVEL_ACCOUNT | CLEVEL_KNOWN)
#define CMASK_VOICE     (GLEVEL_ACCOUNT | CLEVEL_VOICE)
#define CMASK_OP        (GLEVEL_ACCOUNT | CLEVEL_OP)
#define CMASK_MASTER    (GLEVEL_ACCOUNT | CLEVEL_MASTER)
#define CMASK_OWNER     (GLEVEL_ACCOUNT | CLEVEL_OWNER)
/* Global permission masks */
#define GMASK_UNKNOWN   GLEVEL_UNKNOWN
#define GMASK_ALL       (CLEVEL_ALL | GLEVEL_ALL)
#define GMASK_ACCOUNT   GLEVEL_ACCOUNT
#define GMASK_KNOWN     (GLEVEL_ACCOUNT | GLEVEL_KNOWN)
#define GMASK_VOICE     (GLEVEL_ACCOUNT | GLEVEL_VOICE)
#define GMASK_OP        (GLEVEL_ACCOUNT | GLEVEL_OP)
#define GMASK_MASTER    (GLEVEL_ACCOUNT | GLEVEL_MASTER)
#define GMASK_OWNER     (GLEVEL_ACCOUNT | GLEVEL_OWNER)

/* These can be used for both dbuser and chanlevel pointers */
#define CIsKnown(x)        ((x)->flags & DBF_KNOWN)
#define CIsVoice(x)        ((x)->flags & DBF_VOICE)
#define CIsOp(x)           ((x)->flags & DBF_OP)
#define CIsMaster(x)       ((x)->flags & DBF_MASTER)
#define CIsOwner(x)        ((x)->flags & DBF_OWNER)

#define CAtleastKnown(x)   ((x)->flags & (DBF_KNOWN | DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER))
#define CAtleastVoice(x)   ((x)->flags & (DBF_VOICE | DBF_OP | DBF_MASTER | DBF_OWNER))
#define CAtleastOp(x)      ((x)->flags & (DBF_OP | DBF_MASTER | DBF_OWNER))
#define CAtleastMaster(x)  ((x)->flags & (DBF_MASTER | DBF_OWNER))

typedef int (*srvcmdhandler_t)(char *source, int argc, char **argv);
typedef int (*cmdhandler_t)(nick *np, channel *cp, int argc, char **argv);

typedef struct command_t {
    char *commandname;
    int   level;
    int   type;
    srvcmdhandler_t srvhandler; /* Used for server messages */
    cmdhandler_t handler; /* Used for public/privmsg messages */
} command;

extern htable *commands;

command *findcommand(char *name, int type);
command *addcommand(char *name, int level, int type, srvcmdhandler_t srvhandler, cmdhandler_t handler);
void delcommand(char *name, int type, srvcmdhandler_t srvhandler, cmdhandler_t handler);
int haspermissions(nick *np, int permissions);
int haschanpermissions(nick *np, channel *cp, int permissions);

#endif /* COMMAND_H */
