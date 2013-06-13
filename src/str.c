#include "common.h"

int linetoarray(char *line, char **outline, int split, int max) {
    char *charp;
    int count = 0, iw = 0;
    
    if (*line == ':')
        line++;
    
    for (charp = line; *charp; charp++) {
        if (iw) {
            if (*charp == ' ') {
                *charp = '\0';
                iw = 0;
            }
        } else {
            if (*charp == ' ') {
                *charp = '\0';
            } else {
                if (count && split && *charp == ':') {
                    outline[count++] = charp + 1;
                    break;
                } else if ((count + 1) == max) {
                    outline[count++] = charp;
                    break;
                } else {
                    outline[count++] = charp;
                    iw = 1;
                }
            }
        }
    }
    
    return count;
}

/* Hash function taken from http://www.cse.yorku.ca/~oz/hash.html */
unsigned long hash(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/* Hash function made case insensitive */
unsigned long hashi(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = tolower(*str++)))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

int match(const char *mask, const char *name) {
    const char *m = mask, *n = name;
    const char *m_tmp = mask, *n_tmp = name;
    int star_p;

    for (;;) switch (*m) {
                 case '\0':
                     if (!*n)
                         return 0;
backtrack:
                     if (m_tmp == mask)
                         return 1;
                     m = m_tmp;
                     n = ++n_tmp;
                     break;
                 case '\\':
                     m++;
                     /* allow escaping to force capitalization */
                     if (*m++ != *n++)
                         goto backtrack;
                     break;
                 case '*': case '?':
                     for (star_p = 0; ; m++) {
                         if (*m == '*')
                             star_p = 1;
                         else if (*m == '?') {
                             if (!*n++)
                                 goto backtrack;
                         } else break;
                     }
                     if (star_p) {
                         if (!*m)
                             return 0;
                         else if (*m == '\\') {
                             m_tmp = ++m;
                             if (!*m)
                                 return 1;
                             for (n_tmp = n; *n && *n != *m; n++) ;
                         } else {
                             m_tmp = m;
                             for (n_tmp = n; *n && tolower(*n) != tolower(*m); n++) ;
                         }
                     }
                     /* and fall through */
                 default:
                     if (!*n)
                         return *m != '\0';
                     if (tolower(*m) != tolower(*n))
                         goto backtrack;
                     m++;
                     n++;
                     break;
                 }
}

/*
 * mmatch()
 *
 * Written by Run (carlo@runaway.xs4all.nl), 25-10-96
 *
 *
 * From: Carlo Wood <carlo@runaway.xs4all.nl>
 * Message-Id: <199609021026.MAA02393@runaway.xs4all.nl>
 * Subject: [C-Com] Analysis for `mmatch' (was: gline4 problem)
 * To: coder-com@mail.undernet.org (coder committee)
 * Date: Mon, 2 Sep 1996 12:26:01 +0200 (MET DST)
 *
 * We need a new function `mmatch(const char *old_mask, const char *new_mask)'
 * which returns `true' likewise the current `match' (start with copying it),
 * but which treats '*' and '?' in `new_mask' differently (not "\*" and "\?" !)
 * as follows:  a '*' in `new_mask' does not match a '?' in `old_mask' and
 * a '?' in `new_mask' does not match a '\?' in `old_mask'.
 * And ofcourse... a '*' in `new_mask' does not match a '\*' in `old_mask'...
 * And last but not least, '\?' and '\*' in `new_mask' now become one character.
 */

/** Compares one mask against another.
 * One wildcard mask may be said to be a superset of another if the
 * set of strings matched by the first is a proper superset of the set
 * of strings matched by the second.  In practical terms, this means
 * that the second is made redundant by the first.
 *
 * The logic for this test is similar to that in match(), but a
 * backslash in old_mask only matches a backslash in new_mask (and
 * requires the next character to match exactly), and -- after
 * contiguous runs of wildcards are logically collapsed -- a '?' in
 * old_mask does not match a '*' in new_mask.
 *
 * @param[in] old_mask One wildcard mask.
 * @param[in] new_mask Another wildcard mask.
 * @return Zero if a old_mask is a superset of a new_mask, non-zero otherwise.
 */
int mmatch(const char *old_mask, const char *new_mask) {
    const char *m = old_mask;
    const char *n = new_mask;
    const char *ma = m;
    const char *na = n;
    int wild = 0;
    int mq = 0, nq = 0;

    while (1) {
        if (*m == '*') {
            while (*m == '*')
                m++;
            wild = 1;
            ma = m;
            na = n;
        }

        if (!*m) {
            if (!*n)
                return 0;
            for (m--; (m > old_mask) && (*m == '?'); m--)
                ;
            if ((*m == '*') && (m > old_mask) && (m[-1] != '\\'))
                return 0;
            if (!wild)
                return 1;
            m = ma;

            /* Added to `mmatch' : Because '\?' and '\*' now is one character: */
            if ((*na == '\\') && ((na[1] == '*') || (na[1] == '?')))
                ++na;

            n = ++na;
        } else if (!*n) {
            while (*m == '*')
                m++;
            return (*m != 0);
        }
        if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?'))) {
            m++;
            mq = 1;
        }
        else
            mq = 0;

        /* Added to `mmatch' : Because '\?' and '\*' now is one character: */
        if ((*n == '\\') && ((n[1] == '*') || (n[1] == '?'))) {
            n++;
            nq = 1;
        }
        else
            nq = 0;

/*
 * This `if' has been changed compared to match() to do the following:
 * Match when:
 *   old (m)         new (n)         boolean expression
 *    *               any             (*m == '*' && !mq) ||
 *    ?               any except '*'  (*m == '?' && !mq && (*n != '*' || nq)) ||
 * any except * or ?  same as m       (!((*m == '*' || *m == '?') && !mq) &&
 *                                      ToLower(*m) == ToLower(*n) &&
 *                                        !((mq && !nq) || (!mq && nq)))
 *
 * Here `any' also includes \* and \? !
 *
 * After reworking the boolean expressions, we get:
 * (Optimized to use boolean short-circuits, with most frequently occurring
 *  cases upfront (which took 2 hours!)).
 */
        if ((*m == '*' && !mq) ||
            ((!mq || nq) && tolower(*m) == tolower(*n)) ||
            (*m == '?' && !mq && (*n != '*' || nq))) {
            if (*m)
                m++;
            if (*n)
                n++;
        } else {
            if (!wild)
                return 1;
            m = ma;

            /* Added to `mmatch' : Because '\?' and '\*' now is one character: */
            if ((*na == '\\') && ((na[1] == '*') || (na[1] == '?')))
                ++na;

            n = ++na;
        }
    }
}

char *findtype(int type) {
    if (type & DBF_OWNER) {
        return "owner";
    } else if (type & DBF_MASTER) {
        return "master";
    } else if (type & DBF_OP) {
        return "operator";
    } else if (type & DBF_VOICE) {
        return "voice";
    } else if (type & DBF_KNOWN) {
        return "friend";
    }
    
    return "Unknown";
}

void tolowerstr(char *p) {

    for (; *p != '\0'; p++)
        *p = tolower(*p);
}
