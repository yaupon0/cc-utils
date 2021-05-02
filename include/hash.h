/* djb */
static inline unsigned int xhash(unsigned char *w)
{
    unsigned long hash = 0;
    int c;

    while ((c = *w++))
	hash = c + (hash << 6) + (hash << 16) - hash;

    return hash % HLISTSIZE;
}

unsigned int hash(unsigned char *str)
    {
        unsigned long hash = 5381;
        int c;

        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash % HLISTSIZE;
    }

unsigned int hash2strings(unsigned char *str1, unsigned char *str2)
    {
        unsigned long hash = 5381;
        int c;

        while ((c = *str1++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        while ((c = *str2++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash % HLISTSIZE;
    }

