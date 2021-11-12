#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

 //gcc needs this volatile with O3 to force load but clang does not
volatile unsigned int shared = 0;
volatile int gate = 0;
int die = 0;

void *mythread(void *whoami)
{
	int id = (int)whoami;
	unsigned int last = id;

	printf("thread %d\n", id);

	while (!die) {
		while (gate != id) ;
		if (last != (volatile int)shared) {
			fprintf(stderr,
				"Out of sync for thread %d shared is %d\n", id,
				shared);
			exit(0);
		} else {
			shared++;
			last += 2;
			//if the compiler misbehaves we might need this
			//asm("" : /* no output */ : /* no input */ : "memory");
			gate = (id ? 0 : 1);

		}
	}
	fprintf(stderr, "ended thread %d normally with shared = %d\n", id,
		shared);
	return whoami;
}

int main()
{
	pthread_t a, b;
	int i, j;

	i = pthread_create(&a, NULL, mythread, (void *)0);
	j = pthread_create(&b, NULL, mythread, (void *)1);
	if (i < 0 || j < 0) {
		fprintf(stderr, "Failed to create both threads. Giving up\n");
		exit(1);
	}
//proof of life during the long boring test
	for (int k = 0; k < 9000; k++) {
		sleep(10);
		printf("Up to %d\n", shared);
	}

//tell threads to give it up
	die = 1;

//clean up 
	pthread_join(a, NULL);
	pthread_join(b, NULL);

	fprintf(stderr, "done");
}
