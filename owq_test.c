/* (c) Victor Yodaiken. All rights reserved.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Example code for using one-way queues
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// start with integer queues
typedef int owq_element_t;
#include "owq.h"
typedef struct owq_struct owq_t;

//add a second set of queue routines for doubles
// owq2.h is created using sed in the Makefile
typedef double owq2_element_t;
#include "owq2.h"
typedef struct owq2_struct owq2_t;

// thread function types
void *iproducer(void *p);
void *iconsumer(void *p);
void *dproducer(void *p);
void *dconsumer(void *p);

#define SHORTQ  10
#define LONGQ  (1024*1024)
int shorti[SHORTQ];
int longi[LONGQ];
double shortd[SHORTQ];
double longd[LONGQ];

 // declare the queue control structures
owq_t iq1 = {.t = 0,.h = 0,.v = shorti,.z = SHORTQ };
owq_t iq2 = {.t = 0,.h = 0,.v = longi,.z = LONGQ };
owq2_t dq1 = {.t = 0,.h = 0,.v = shortd,.z = SHORTQ };
owq2_t dq2 = {.t = 0,.h = 0,.v = longd,.z = LONGQ };

struct pinfo {
	int *poison;
	int test;
	char *m;
	owq_t *q;
};
struct dpinfo {
	int *poison;
	int test;
	char *m;
	owq2_t *q;
};
void twothreads(char *m, owq_t * q);
void twodthreads(char *m, owq2_t * q);
#define REPETITIONS (1024*1024*1020)
#define MAXSLEEP 100000

int main(int argc, char **argv)
{
	int repeat_count = 1;
	int test_number = 1;
	if (argc > 1) {
		if ((repeat_count = atoi(argv[1])) <= 0) {
			fprintf(stderr, "Bad repetition count\n");
			exit(1);
		}
	}
	printf
	    ("Owq test with %d enqs. Short queue = %d elements. Long queue = %d elements\n",REPETITIONS, SHORTQ, LONGQ);

	while (repeat_count-- > 0) {
		fprintf(stdout, "Run %d\n", test_number++);
		iq1.h = iq2.h = iq1.t = iq2.t = 0;
		dq1.h = dq2.h = dq1.t = dq2.t = 0;
		twothreads("Shortq int", &iq1);
		twothreads("Longq int", &iq2);
		twodthreads("Shortq double", &dq1);
		twodthreads("Longq double", &dq2);
	}

}

void *iproducer(void *p)
{
	int n = 0;
	int sleeps = 0;
	struct pinfo pi = *(struct pinfo *)p;
	owq_t *q = pi.q;

	do {
		if (owq_enq(q, n) == 0) {
			n++;
			sleeps = 0;
		} else {
			if (sleeps++ > 10000) {
				usleep(1);
			}
		}
	} while (n < REPETITIONS && sleeps < MAXSLEEP);

	if (sleeps >= MAXSLEEP) {
		fprintf(stderr, "  Int Producer oversleeps\n");
		exit(0);
	}
	if(n != REPETITIONS)
		fprintf(stdout, "  Int Producer %s exits after %d enqs\n", pi.m, n);
	*(pi.poison) = 1;
	return 0;
}

void *iconsumer(void *p)
{
	int n = 0;
	int j;
	int sleeps = 0;
	struct pinfo pi = *(struct pinfo *)p;
	owq_t *q = pi.q;

	do {
		if (owq_deq(q, &j) == 0) {
			if (j != n) {
				fprintf(stderr,
					"  Int Consumer %s sequence error %d != %d\n",
					pi.m, n, j);
				exit(0);
			}
			n++;
			sleeps = 0;
		} else {
			if (sleeps > 1000)
				usleep(1);
			sleeps++;
			if (*(pi.poison))
				*(pi.poison) += 1;
		}
	}
	while (sleeps < 2 * MAXSLEEP && (*(pi.poison) < 5));

	if (sleeps >= MAXSLEEP) {
		fprintf(stderr, "  Int Consumer %s oversleeps\n", pi.m);
	} else if(n != REPETITIONS){
		fprintf(stdout, "  Int Consumer %s exits after %d deqs\
           Producer was %s\n", pi.m, n, (*(pi.poison) ? "done" : "not done"));
	}
	return 0;
}

void *dproducer(void *p)
{
	double n = 0;
	int sleeps = 0;
	int count = 0;
	struct dpinfo pi = *(struct dpinfo *)p;
	owq2_t *q = pi.q;

	do {
		if (owq2_enq(q, n) == 0) {
			n += 1.1;
			count++;
			sleeps = 0;
		} else {
			if (sleeps++ > 10000) {
				usleep(1);
			}
		}
	} while (count < REPETITIONS && sleeps < MAXSLEEP);

	if (sleeps >= MAXSLEEP) {
		fprintf(stderr, "  Dproducer %s oversleeps\n", pi.m);
		exit(0);
	}
	if(count != REPETITIONS)
		fprintf(stdout, "  DProducer %s exits after %d enqs\n", pi.m, count);
	*(pi.poison) = 1;
	return 0;
}

void *dconsumer(void *p)
{
	double n = 0;
	double j;
	unsigned int count = 0;
	int sleeps = 0;
	struct dpinfo pi = *(struct dpinfo *)p;
	owq2_t *q = pi.q;

	do {
		if (owq2_deq(q, &j) == 0) {
			if (j != n) {
				fprintf(stderr,
					"  DConsumer %s sequence error %f != %f\n",
					pi.m, n, j);
				exit(0);
			}
			n += 1.1;
			count++;
			sleeps = 0;
		} else {
			if (sleeps > 1000)
				usleep(1);
			sleeps++;
			if (*(pi.poison))
				*(pi.poison) += 1;
		}
	}
	while (sleeps < 2 * MAXSLEEP && (*(pi.poison) < 5));

	if (sleeps >= MAXSLEEP) {
		fprintf(stderr, "  DConsumer %s oversleeps\n", pi.m);
	} else if(count != REPETITIONS) {
		fprintf(stdout, "  Dconsumer exits after %d deqs\
           Producer was %s\n", count, (*(pi.poison) ? "done" : "not done"));
	}
	return 0;
}

unsigned long millisec(void);

void twothreads(char *m, owq_t * q)
{
	int r1, r2;
	int poison = 0;
	unsigned long elapsed = 0;
	struct pinfo pi = {.poison = &poison,.q = q,.m = m };
	pthread_t thread1, thread2;

	r1 = pthread_create(&thread1, NULL, iproducer, (void *)&pi);
	r2 = pthread_create(&thread2, NULL, iconsumer, (void *)&pi);

	elapsed = millisec();

	if (r1 || r2) {
		fprintf(stdout,
			"  %s thread create returns: %d %d\n", m, r1, r2);
		exit(0);
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	if (r1 || r2) {
		fprintf(stdout,
			"  %s Thread 1 returns: %d Thread 2 returns %d\n",
			m, r1, r2);
	}
	fprintf(stdout, "  %s took %ld milliseconds\n",
		m, millisec() - elapsed);
}

void twodthreads(char *m, owq2_t * q)
{
	int r1, r2;
	int poison = 0;
	unsigned long elapsed = 0;
	struct dpinfo pi = {.poison = &poison,.q = q,.m = m };
	pthread_t thread1, thread2;

	r1 = pthread_create(&thread1, NULL, dproducer, (void *)&pi);
	r2 = pthread_create(&thread2, NULL, dconsumer, (void *)&pi);

	elapsed = millisec();

	if (r1 || r2) {
		fprintf(stdout,
			"  %s thread create returns: %d %d\n", m, r1, r2);
		exit(0);
	}

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	if (r1 || r2) {
		fprintf(stdout,
			"  %s Thread 1 returns: %d Thread 2 returns %d\n",
			m, r1, r2);
	}
	fprintf(stdout, "  %s took %ld milliseconds\n",
		m, millisec() - elapsed);
}

#include <time.h>
unsigned long millisec(void)
{
	struct timespec t;
	if (clock_gettime(CLOCK_REALTIME, &t)) {
		fprintf(stdout, "Can't read time\n");
	}

	return t.tv_sec * 1000 + ((unsigned long)t.tv_nsec) / (1000 * 1000);
}

void *mmalloc(size_t n, char *message)
{
	void *v = malloc(n);
	if (!v) {
		fprintf(stderr, "Malloc fails for %s\n", message);
		exit(-1);
	}
	return v;
}
