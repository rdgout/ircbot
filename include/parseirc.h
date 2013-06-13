#ifndef PARSEIRC_H
#define PARSEIRC_H

int bot_initserver(char *source, int argc, char **argv);
int bot_initsettings(char *source, int argc, char **argv);
int bot_userhost(char *source, int argc, char **argv);
int bot_getmodes(char *source, int argc, char **argv);
int bot_nickmsg(char *source, int argc, char **argv);
int bot_nicktaken(char *source, int argc, char **argv);
int bot_names(char *source, int argc, char **argv);
int bot_endnames(char *source, int argc, char **argv);
int bot_getban(char *source, int argc, char **argv);
int bot_whoreply(char *source, int argc, char **argv);
int bot_privmsg(char *source, int argc, char **argv);
int bot_modemsg(char *source, int argc, char **argv);
int bot_joinmsg(char *source, int argc, char **argv);
int bot_partmsg(char *source, int argc, char **argv);
int bot_quitmsg(char *source, int argc, char **argv);
int bot_kickmsg(char *source, int argc, char **argv);
int bot_invitemsg(char *source, int argc, char **argv);

#endif /* PARSEIRC_H */
