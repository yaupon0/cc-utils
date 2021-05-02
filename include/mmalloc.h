// malloc utility - malloc fails are not recoverable
static inline char *mmalloc(int n, char *message)
{
	char *r = malloc(n);
	if (!r) {
		fprintf(stderr, "FAIL MALLOC: Cannot allocate %s\n", message);
		exit(-1);
	}
	return r;
}

