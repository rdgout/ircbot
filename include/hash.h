#ifndef HASH_H
#define HASH_H

typedef struct htable_t {
    int size;
    int initsize;
    int count;
    int rehash;
    int bucketcount;
    struct bucket_t **buckets;
} htable;

typedef struct bucket_t {
    char *key;
    void *pointer;
    struct bucket_t *next;
} bucket;

htable *newhtable(int size, int rehash);
void insertintohtable(htable *htp, char *key, void *pointer);
void removefromhtable(htable *htp, char *key, void *pointer);
void freehtable(htable *htp);
void rehashhtable(htable *htp, int newhashsize);

#endif /* HASH_H */
