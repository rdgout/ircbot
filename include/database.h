#ifndef DATABASE_H
#define DATABASE_H

/* Loading and saving channel data */
int loadchandb(char *filename);
int savechandb(char *filename);
/* Loading and saving user data */
int loaduserdb(char *filename);
int saveuserdb(char *filename);
/* Save databases by timer */
void savedatabases(void *p);

#endif /* DATABASE_H */
