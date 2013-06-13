#ifndef HANDLERS_H
#define HANDLERS_H

int handlechanmsg(channel *cp, nick *np, char *arg);
int handleprivmsg(nick *np, char *arg);

#endif /* HANDLERS_H */
