/*
Copyright (c) 2020 Victor Yodaiken - all rights reserved except as
granted specifically.


 Doubly linked list of structures dlist_t linked
 in a circle. The list anchor is  pointer of type dlist_t * and it points
 to the first element.

 anchor-----> head (next direction)
 	 tail/    \second
	  ^         |
	  |         \/
	  e         e
	  |         |
	  e         e
	  |         |
	  e<------- e
 The user must define 
 typedef dlist_t
 to some structure which has
 at least the elements dlist_t *n and dlist_t *p (next and previous).
 The other contents of this structure are up to the user/application.

 dlist_init(dlist_t **anchor);  initializes to empty
 int dlist_isempty(dlist_t **anchor); 1 true, 0 false
 dlist_t *dlist_next(dlist_t ** anchor, dlist_t * x); iterator
 int dlist_enq(dlist_t **anchor);  returns 0 on fail, 1 on success
 dlist_t dlist_deq(dlist_t **anchor); returns NULL on fail
 dlist_t dlist_pop(dlist_t **anchor) ;   (using the list as a stack)
 int dlist_insert(dlist **anchor, dlist_t *element);  (inserts after element)
 int dlist_preinsert(dlist_t *element);   (inserts before a given element)
 dlist_t *dlist_search(dlist_t ** anchor, dlist_t * last, dlist_key_t k)
 	user must define dlist_compare(x,y) and dlist_key_t to make it work.
	if last== NULL then searches for first match
	else it will search for first match after last
 	so you can iterate looking for all matching elements.
 dlist_msort is a merge sort - only compiled if dlist_leq(dlist_t *x,dlist_t *y) is defined
 	which returns 1 if x <= y and 0 otherwise.
dlist_join - not done yet



 How to use

 1. declare a struct with the n and p pointers and your payload
 2. define or typedef dlist_t to this struct
 3. #include "dlinklist.h" which will create all the routines to
     operate on linked lists of dlist_t 
 4. for each list of this type declare a pointer to dlist_t ito be anchor
     and dlist_init it
 5. malloc or otherwise create structs of the right type and do stuff 
    with them

    Question: What if I want to use e.g. lists of ints and floats?
    Answer: either one type per file (suggested)
    or void * pointers *    in the list
    or sed s/dlist_/mylist_/g for example s/dlist_/floatlist_/
    to creat new header files via make
 */

#ifndef INLINE
#define INLINE  static inline
#endif

INLINE void dlist_init(dlist_t ** x)
{
	*x = (dlist_t *) x;
}

INLINE int dlist_isempty(dlist_t **x){ return *x == (dlist_t *) x;}

INLINE dlist_t *dlist_next(dlist_t ** anchor, dlist_t * x)
{
	//remove this test to speed up - live dangerously
	if (!anchor || !*anchor || (*anchor == (void *)anchor)) {
		return (0);
	}
	return (!x ? *anchor : (x->n == *anchor ? (dlist_t *) NULL : x->n));
}

INLINE int dlist_enq(dlist_t ** anchor, dlist_t * x)
{
	dlist_t *head = *anchor;
	if ((void *)head == (void *)anchor) {	//empty
		x->n = x;
		x->p = x;
		*anchor = x;
	} else if (head) {	// should be a unnecessary test
		x->n = head;
		x->p = head->p;
		head->p = x;
		x->p->n = x;
	} else
		return 0;
	return 1;
}

INLINE dlist_t *dlist_deq(dlist_t ** anchor)
{
	dlist_t *x;
	if (!anchor || !(*anchor) || (*anchor == (dlist_t *) anchor))
		return (dlist_t *) NULL;
	x = *anchor;
	if (x->n == x)
		*anchor = (void *)anchor;	//empty
	else {
		*anchor = x->n;
		x->n->p = x->p;
		x->p->n = *anchor;
	}
	return x;
}

INLINE dlist_t *dlist_pop(dlist_t ** anchor)
{
	dlist_t *x;
	if (!anchor || !(*anchor) || (*anchor == (dlist_t *) anchor)
	    || !((*anchor)->p))
		return (dlist_t *) NULL;
	x = (*anchor)->p;
	if (x->p == x)
		*anchor = (void *)anchor;	//empty
	else {
		(*anchor)->p = x->p;
		x->p->n = (*anchor);
	}
	return x;
}

INLINE int dlist_insert(dlist_t ** anchor, dlist_t * prev, dlist_t * x)
{				// insert after prev
	// if the list is empty (2cd condition) prev must be erroneous
	if (!anchor || (*anchor == (void *)anchor) || !prev || !x)
		return 0;
	x->n = prev->n;
	x->p = prev;
	prev->n = x;
	(prev->n)->p = x;
	return 1;
}
#if 0 //needs thinking
INLINE int dlist_join(dlist_t **a, dlist_t **b){
	if (!a || !b || (*b == (void *)b))
		return 0;
	if((*a == (void *)a)){
			*a= *b;
	} else {
#endif


INLINE int dlist_preinsert(dlist_t ** anchor, dlist_t * prev, dlist_t * x)
{				// insert before prev
	if ((*anchor == (void *)anchor) || !prev || !x)
		return 0;
	x->n = prev;
	x->p = prev->p;
	(x->p)->n = x;
	prev->p = x;
	if (*anchor == prev)
		*anchor = x;
	return 1;
}

#if defined(dlist_compare) && defined(dlist_key_t)

INLINE dlist_t *dlist_search(dlist_t ** anchor, dlist_t * last, dlist_key_t k)
{
	dlist_t *x;
	if (!anchor || (*anchor == (void *)anchor) ||
	    (last && (x = last->n) == (*anchor)))
		return 0;
	if (!last)
		x = *anchor;
	do {
		if (dlist_compare(x, k) == 0)
			return x;
	} while ((x = x->n) != *anchor);
	return NULL;
}

#endif
#if defined( DLIST_MERGE)

INLINE int dlist_merge(dlist_t ** a, int l);
INLINE void dlist_msort(dlist_t ** anchor)
{
	int sublistlen = 1;	//minimal for merging
	int notdone = 1;
	if (!anchor || !(*anchor) || (*anchor == (void *)anchor)
	    || (((*anchor)->n) == (*anchor)))
		return;
	//so at least 2 elements;
	for (sublistlen = 1; notdone; sublistlen *= 2) {
		notdone = dlist_merge(anchor, sublistlen);
	}

}

static inline int dlist_merge(dlist_t ** a, int l)
{
// left:right,left:right .... 
	
	dlist_t *left;		// left part to be merged
	dlist_t *right;	// right part to be merged
	dlist_t *nleft;	//the start of the next pair of lists
	int q, r;	//count unmerged elements in left and right lists
	int notdone = 1;
	int firstmerge = 1;

	nleft = *a;
	do {
		int i = 0;
		left = nleft;
		right = NULL; 
		while (i < 2 * l) {//find right,nleft, q,r 
			i++;
			if ((nleft = nleft->n) == *a) {
				if (firstmerge){
					notdone = 0;
				}
				break;
			}
			if (i == l)//gone through all of left list
				right = nleft;
		};
		firstmerge = 0;
		q = (i >= l ? l : i);
		r = (i > l ? i - l : 0);
		if(r>0 && !right){
			fprintf(stderr,"Second list error in merge\n");
			exit(0);
		}
		while (q > 0 && r > 0) {
			if (!dlist_leq(left, right)) {	//swap them, decrement r, advance y
				dlist_t *n = right->n;
				(right->p)->n = n;
				n->p = right->p;
				right->p = left->p;
				right->n = left;
				(left->p)->n = right;
				left->p = right;
				if (*a == left)
					*a = right;
				right = n;
				r--;
			} else {	//advance x
				left = left->n;
				q--;
			}
		}
	} while (nleft != *a);
	return notdone;
}
#endif
