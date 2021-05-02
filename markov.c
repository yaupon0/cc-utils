/*
 * Markov text generator.
 *
 *  use:  markov < input.txt 
 *
 * This is a C language rewrite of the Lua book Markov program ( Roberto Ierusalimschy).
 * Only 3x as many lines of code but written for the serious purpose of, no serious purpose.
 *
 * reads text files from standard input and writes text using the vocabulary and probabilities
 * learned from the text.
 *
 * The program depends on C generic double linked list functions using sed to adapt them to
 * different types (the sed commands are in the Makefile).
 *
 * Build a dictionary of the unique pairs of words that appear in the text and associate
 * each entry with the list of words that immediately follow that use of that pair in the text 
 * (that list of followers may contain repetitions).
 *
 * Text generation starts somewhere with (w1,w2) and randomly selects an element w3 in the list
 * of followers for w2 to get (w2,w3) - printing w2. Then it does it again, this time printing
 * w2, selecting some w4 in the list of followers for w3 and so on. 
 *
 * The dictionary is a hash table (using a hash algorithm attributed to Daniel J. Bernstein) where
 * each element is a linked list of colliding pairs and each element in that linked list has a linked
 * list of followers.
 */

#define WORD_COUNT 500 //how many words to generate
#define HLISTSIZE 10000 //size of the hash table used in hash.h- make it bigger for bigger inputs
#define WORDSIZE 25		//max number of characters allowed per word
#define CHARBUFSIZE 2048 //chunk size for reads of the input file


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> //more includes in the text below
#include <mmalloc.h>

struct dictionary;		//defined below

struct dictionary *Build_Dictionary(int fd); //does what it says
void Write_Markov(struct dictionary *d, int count); //this is the creative writer

int main()
{
	struct dictionary *d = Build_Dictionary(0);
	if (d == NULL) {
		fprintf(stderr, "Can't build dictionary\n");
		exit(0);
	}
	Write_Markov(d, WORD_COUNT);
}

//the linked lists need structures with n (next) and p (previous) pointers plus arbitrary payload
struct pair;
struct follow {
	struct follow *n;
	struct follow *p;
	unsigned char *w;
};
struct pair {
	struct pair *n;
	struct pair *p;
	unsigned char *w1, *w2;
	int count;
	struct follow *f;
	int fcount;
};


struct dictionary {
	struct pair *H[HLISTSIZE];
} thed;

#define pair_t struct pair
#include "pair_ll.h"
struct pair *lookup(struct dictionary *d, unsigned char *, unsigned char *);

unsigned char *getword(int);
struct pair *add_pair(struct dictionary *, unsigned char *, unsigned char *);
void add_follower(struct pair *, unsigned char *);
struct dictionary *Build_Dictionary(int fd)
{
	unsigned char *w;
	struct pair *l = add_pair(&thed, (unsigned char *)"\n",(unsigned char *) "\n");

	while (l && (w = getword(fd))) {
		add_follower(l, w);
		l = add_pair(&thed, l->w2, w);
	}
	return &thed;

}

unsigned char *follower(struct follow *, int);
#include "hash.h"
struct pair *lookup(struct dictionary *d, unsigned char *w1, unsigned char *w2)
{

	struct pair *l= NULL;
	int hindex = hash2strings(w1, w2);
	struct pair **p = &d->H[hindex];
	if (!(*p) || pair_isempty(p))
		return NULL;
	while ( (l = pair_next(p, l))) {
		if (!strcmp((char *)w1, (char *)l->w1) && !strcmp((char *)w2,(char *) l->w2))
			return l;
	}
	return NULL;
}

void Write_Markov(struct dictionary *d, int count)	//pure side effect function 
{
	struct pair *l = lookup(d, (unsigned char *)"\n", (unsigned char *)"\n");

	srandom((int)time(0));

	if (!l || pair_isempty(&l)) {
		fprintf(stderr, "No first word in dictionary\n");
		exit(1);
	}
	if (!l->fcount) {
		fprintf(stderr, "First word has no followers\n");
		exit(1);
	}
	do {
		int index = 1 + (random()%l->fcount);
		l = lookup(d, l->w2, follower(l->f,index));
		fprintf(stdout, " %s", l->w1);
	}
	while (l && (--count > 0));
}

#define follower_t struct follow
#include "follower_ll.h"  //the follower linked list operates on types follower_t 

struct pair *add_pair(struct dictionary *d, unsigned char *w1, unsigned char *w2)
{
	struct pair *l = lookup(d, w1, w2);

	if (l == NULL) {	// not there
		int h = hash2strings(w1, w2);
		if (d->H[h] == NULL)
			pair_init(&d->H[h]);
		l = (struct pair *)mmalloc(sizeof(struct pair), "add pair");
		l->w1 = w1;
		l->w2 = w2;
		l->count = 1;
		l->fcount = 0;
		follower_init(&(l->f));
		pair_enq(&d->H[h], l);
	} else {
		l->count++;
	}
	return l;

}

void add_follower(struct pair *p, unsigned char *w)
{
	struct follow *f =
	    (struct follow *)mmalloc(sizeof(struct follow), "add follower");
	if(p->fcount++ == 0)follower_init(&p->f);
	f->w = w;
	follower_enq(&(p->f),f);
}

//this could be optimized by reversing order of search if k> fcount/2
unsigned char *follower(struct follow *fq, int k)
{
	int i = 0;
	struct follow *f= NULL;
	do {
		i++;
		f = follower_next(&fq, f);
	} while (f && (i < k));
	if (i != k) {
		fprintf(stderr, "follower sent past end of queue i=%d k=%d\n",
			i, k);
		exit(-1);	//not recoverable
	}
	return f->w;
}

//this should probably be fixed to not mangle words that cross buffer boundaries
unsigned char *getword(int fd)
{
	static unsigned char *buf = NULL;
	static int nextc = 0;
	static int n = 0;
	int i, j;
	if (!buf || ((CHARBUFSIZE - nextc) <= WORDSIZE)) {
		buf = (unsigned char *)mmalloc(CHARBUFSIZE, "character buffer");
		nextc = 0;
		n = read(fd, buf, CHARBUFSIZE);
		if (n < 1)
			return 0;
	}
	if (nextc > n)
		return 0;
	for (i = nextc; !isalpha(buf[i]); i++) ;
	for (j = i; !isspace(buf[j]) && (buf[j] != '\n') && j < n; j++) ;
	if (j < n)
		buf[j] = 0;
	else buf[n-1]=0;
	nextc = j;
	return &buf[i];
}
