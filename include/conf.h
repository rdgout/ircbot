#ifndef CONF_H
#define CONF_H

typedef struct conf_t {
    char *key;
    char *setting;
    
    struct conf_t *next;
} conf;

extern conf *confsettings;

int loadconf(char *filename);
void freeconf();
void rehashconf();
char *getconf(char *key, char *defaultvalue);

#endif /* CONF_H */
