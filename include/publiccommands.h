#ifndef PUBLICCOMMANDS_H
#define PUBLICCOMMANDS_H

void initregcmds();

/* PRIVMSG (only) commands */
int private_hellocmd(nick *np, channel *cp, int argc, char **argv);

/* PUBLIC and PRIVMSG commands */
int public_statuscmd(nick *np, channel *cp, int argc, char **argv);
int public_helpcmd(nick *np, channel *cp, int argc, char **argv);
int public_userlistcmd(nick *np, channel *cp, int argc, char **argv);
int public_whoiscmd(nick *np, channel *cp, int argc, char **argv);
int public_kickcmd(nick *np, channel *cp, int argc, char **argv);
int public_addchancmd(nick *np, channel *cp, int argc, char **argv);
int public_delchancmd(nick *np, channel *cp, int argc, char **argv);
int public_savedbcmd(nick *np, channel *cp, int argc, char **argv);
int public_clearqueuecmd(nick *np, channel *cp, int argc, char **argv);
int public_rehashcmd(nick *np, channel *cp, int argc, char **argv);
int public_triggercmd(nick *np, channel *cp, int argc, char **argv);
int public_moocmd(nick *np, channel *cp, int argc, char **argv);

#endif /* PUBLICCOMMANDS_H */
