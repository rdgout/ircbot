#ifndef STR_H
#define STR_H

int linetoarray(char *line, char **outline, int split, int max);
unsigned long hash(char *str);
unsigned long hashi(char *str);
int match(const char *mask, const char *name);
int mmatch(const char *old_mask, const char *new_mask);
char *findtype(int type);
void tolowerstr(char *p);

#endif /* STR_H */
