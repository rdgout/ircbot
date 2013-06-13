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
