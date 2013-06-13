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

#include "common.h"

char *confname = NULL;
conf *confsettings = NULL;

int loadconf(char *filename) {
    FILE *fp;
    char  line[1024]; /* Large buffer, just incase */
    char *charp;
    conf *cop, *cop2, *newcop;
    int loaded = 0;

    /* On initial load, define the configuration filename */
    if (filename)
        confname = filename;

    /* On rehash, get the filename from the initial load */
    if (!filename)
        filename = confname;

    if (!(fp = fopen(filename, "r"))) {
        printlog("Could not open configuration file %s.", filename);
        exit(1);
    }
    
    while((fgets(line, 1023, fp))) {
        /* Read out the line and skip any comments and empty lines too. */
        /* Valid comments are: # and // */
        if (line[0] == '#' || (line[0] == '/' && line[1] == '/') || line[0] == '\n' || line[0] == '\r')
            continue;

        /* Remove the end-of-line chars here */
        for (charp = line; *charp; charp++) {
            if (*charp == '\n' || *charp == '\r') {
                *charp = '\0';
                break;
            }
        }

        /* Is the line long enough? */
        /* Required is atleast key=setting (3 chars) */
        if (strlen(line) < 3)
            continue;

        /* root out the '=' char now from the line. */
        for (charp = line; *charp; charp++) {
            if (*charp == '=') {
                *charp = '\0';
                
                /* Make sure the setting isn't empty */
                if (!strlen(charp + 1))
                    break;

                /* Check if this key already exists, if it does, free it completely and add a new one from scratch. */
                /* Yes this does look somewhat useless, maybe we should just renew the setting instead (ponder ponder ponder) */
                for (cop = confsettings; cop; cop = cop->next) {
                    if (!strcasecmp(cop->key, line)) {
                        /* Should be safe as we only set these if we find a valid line */
                        free(cop->key);
                        free(cop->setting);
                        
                        if (cop == confsettings)
                            confsettings = cop->next;
                        else {            
                            for (cop2 = confsettings; cop2; cop2 = cop2->next) {
                                if (cop2->next == cop) {
                                    cop2->next = cop->next;
                                    break;
                                }
                            }
                        }
                            
                        free(cop);
                    }
                }
                
                newcop = (conf *)malloc(sizeof(conf));
                newcop->next = NULL;
                
                /* Copy the key */
                newcop->key = strdup(line);
                
                /* Copy the setting */
                newcop->setting = strdup(charp + 1);
                
                newcop->next = confsettings;
                confsettings = newcop;
                
                loaded++;
            }
        }
    }

    fclose(fp);
    printlog("Configuration file %s loaded.", filename);
    return loaded;
}

void freeconf() {
    conf *cp, *next;
    
    for (cp = confsettings; cp; cp = next) {
        next = cp->next;
        
        free(cp->key);
        free(cp->setting);
        free(cp);
    }
}

void rehashconf() {
    freeconf();
    loadconf(NULL);
}

char *getconf(char *key, char *defaultvalue) {
    conf *cp;
    
    for (cp = confsettings; cp; cp = cp->next) {
        if (!strcasecmp(cp->key, key))
            return (char *)cp->setting;
    }
    
    return (char *)defaultvalue;
}
