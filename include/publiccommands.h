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
