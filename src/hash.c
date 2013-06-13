#include "common.h"

htable *newhtable(int size, int rehash) {
    htable *htp;
    int i;
    
    htp = (htable *)malloc(sizeof(htable));
    
    if (!htp) {
        printlog("Failed to allocate memory to create hashtable!");
        exit(0);
    }
    
    htp->size = size;
    htp->initsize = size;
    htp->count = 0;
    htp->rehash = rehash;
    htp->bucketcount = 0;
    htp->buckets = (bucket **)malloc(htp->size * sizeof(bucket *));
    
    if (!htp->buckets) {
        printlog("Failed to allocate memory to create hashtable buckets!");
        exit(1);
    }
    
    for (i = 0; i < htp->size; i++)
        htp->buckets[i] = NULL;
        
    return htp;
}

void insertintohtable(htable *htp, char *key, void *pointer) {
    int i = hashi(key) % htp->size;
    bucket *bp;
    
    bp = (bucket *)malloc(sizeof(bucket));
    
    bp->key = key;
    bp->pointer = pointer;
    
    htp->count++;
    if (!htp->buckets[i])
        htp->bucketcount++;
    
    bp->next = htp->buckets[i];
    htp->buckets[i] = bp;
      
    /* Check if we need to make the hashtable bigger */
    if (htp->rehash && (((htp->bucketcount * 100) / htp->size) >= 80))
        rehashhtable(htp, htp->size * 2);
}

void removefromhtable(htable *htp, char *key, void *pointer) {
    int i = hashi(key) % htp->size;
    bucket *bp, *prev = NULL;
    
    for (bp = htp->buckets[i]; bp; prev = bp, bp = bp->next) {
        
        /* Compare by pointer incase we use more than one pointer for the same key */
        /* Or if the hashsize requires us to place more than 1 thing in a bucket */
        if (bp->pointer == pointer) {
            if (!prev)
                htp->buckets[i] = bp->next;
            else
                prev->next = bp->next;
                
            if (!htp->buckets[i])
                htp->bucketcount--;
            
            htp->count--;
            free(bp);
            break;
        }
    }
    
    /* Check if we need to make the hashtable smaller */
    if (htp->rehash && htp->size > htp->initsize && (((htp->bucketcount * 100) / htp->size) <= 30))
        rehashhtable(htp, htp->size / 2);
}

void freehtable(htable *htp) {
    int i;
    bucket *bp, *next;

    if (!htp)
        return;

    for (i = 0; i < htp->size; i++) {
        bp = htp->buckets[i];
        
        for (; bp; bp = next) {
            next = bp->next;

            free(bp->key);
            free(bp);
        }
    }

    /* Free the buckets */
    free(htp->buckets);
    /* Free the pointer */
    free(htp);

    /* Done */
}

void rehashhtable(htable *htp, int newhashsize) {
    bucket **newbuckets;
    bucket *p, *next;
    int i, j, oldbucket;

    if (!htp)
        return;

    /* Allocate some new buckets */
    newbuckets  = (bucket **)malloc(newhashsize * sizeof(bucket *));

    if (!newbuckets) {
        printlog("Failed to allocate memory to rehash!");
        exit(0);
    }

    for (i = 0; i < newhashsize; i++)
        newbuckets[i] = NULL;
        
    /* Since a new hash will probably bring up more or less buckets, reset it */
    oldbucket = htp->bucketcount;
    htp->bucketcount = 0;

    /* Rehash everything into the new hashtable */
    for (i = 0; i < htp->size; i++) {
        p = htp->buckets[i];
        
        for (; p; p = next) {
            next = p->next;
            
            j = hashi(p->key) % newhashsize;
            
            if (!newbuckets[j])
                htp->bucketcount++;
            
            p->next = newbuckets[j];
            newbuckets[j] = p;
        }
    }
    
    printlog("Rehashing hashtable from %d to %d (%d to %d)", htp->size, newhashsize, oldbucket, htp->bucketcount);

    free(htp->buckets);
    htp->buckets = newbuckets;
    htp->size = newhashsize;
}

