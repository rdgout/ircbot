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
