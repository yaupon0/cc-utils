/* (c) Victor Yodaiken 2016-2021 All rights reserved.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Use:
Defines  lock free producer/consumer fifo queues  (one producer, one consumer).
To use with multiple producers add a lock for producers -same with consumers
DEPENDS ON X86 STRONG MEMORY MODEL!! WARNING. 

The queues are implemented on arrays of owq_element_t which must be defined
by the user.

In C source:
#define owq_element_t qtype // this is the type of element to be queued
#include "owq.h"
//declare or allocate some array
owq_element_t A[ACOUNT];
//Initialize a header
struct owq_struct myqueue = {.h =0, .t =0, .v = A, z= ACOUNT};

//Now use the functions 

owq_enq(owq_struct *q, owq_element t x);
  // return 0 on success
owq_deq(owq_struct *q, owq_element *x);
  // return 0 on success


A head and tail index are maintained so that enq only increments tail and
deq only increments head which allows producer and consumer to operate
in parallel without write conflicts.

Way too much work was put into distinguishing 2 cases where head==tail using
either the high order bit in head/tail or the low order bit (see end of file)

The high order bit is discarded when head or tail
is used as an index. A deq that fills the queue causes the bit to be
set in head to complement of the bit setting in tail
(which may concurrently change). 
A deq that empties the queue causes the two bits to be set the same.

An alternative, but logically equivalent method is used in the ifdef 0 below.

See the owq_test.c file for use.
*/



#ifndef OWQ_BIT_OFFSET
#define OWQ_BIT_OFFSET ( (sizeof(unsigned int)*8) -1 )
#define OWQ_SETBIT ( (unsigned int)1 << OWQ_BIT_OFFSET ) 
#define OWQ_OFFBIT (~( (unsigned int)1 << OWQ_BIT_OFFSET )) 
#endif

struct owq_struct { unsigned int h; owq_element_t *v; unsigned int z;  unsigned int t;};


inline static int owq_deq(struct owq_struct * q, owq_element_t  *i)
{
	unsigned int next;
	unsigned int h = q->h;
	unsigned int t = q->t;
	if (t==h) return -1;
	else {
		h= h& OWQ_OFFBIT;
	*i = ((owq_element_t *)q->v)[h];
	next  = (h + 1) % q->z;
	if(next  == (t &  OWQ_OFFBIT) ) q->h = t; //empty
	else q->h = next;
	return 0;
    }
}

inline static int owq_enq(struct owq_struct * q, owq_element_t i)
{
	unsigned int next;
	unsigned int t = q->t & OWQ_OFFBIT;
	unsigned int h = q->h;
    if ((t == (h& OWQ_OFFBIT))&& (h != q->t)) return -1;
    ((owq_element_t *)q->v)[t] = i;
    next = (t + 1) % q->z;
    if(next == (h & OWQ_OFFBIT) && ((h & OWQ_SETBIT)== 0))q->t = h| OWQ_SETBIT;
    else q->t = next;

    return 0;

}
#if 0

#ifndef OWQ_STRUCT_T
typedef struct  {
    unsigned int h, t; void *v; const int z; } struct owq_struct;
#define owqfull(q) ((q->t>>1  == q->h>>1  ) && (q->h != q->t))
#define owqempty(q) ((q->t == q->h))
#define OWQ_STRUCT_T
#endif



static void owq_init( struct owq_struct * q, owq_element_t *a, unsigned int n){

    q->h=q->t = 0;
    *(int *)&q->z = n;
    q->v = (void *) a;
}


inline static int owq_deq(struct owq_struct * q, owq_element_t  *i)
{
	int next;
	int h = q->h;
	int t = q->t;
	if (t==h) return -1;
	else {
		h=h>>1;
	*i = ((owq_element_t *)q->v)[h];
	next  = (h + 1) % q->z;
	if(next  == (t>>1) ) q->h = t;
	else q->h = next*2;
	return 0;
    }
}

inline static int owq_enq(struct owq_struct * q, owq_element_t i)
{
	int next;
	int t = q->t>>1;
	int h = q->h;
    if ((t == (h>>1))&& (h != q->t)) return -1;
    ((owq_element_t *)q->v)[t] = i;
    next = (t + 1) % q->z;
    if(next == (h >> 1) && ((h & 1)== 0))q->t = h+1;
    else q->t = next*2;

    return 0;

}
#endif




