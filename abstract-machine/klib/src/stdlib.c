#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

static uint8_t *heap_space = NULL;
/* malloc() function allocates space for an object whose size is specified by 
 * size and whose value is indeterminate.
 * Return - returns either a null pointer or a pointer to the allocated space.
 */
void *malloc(size_t size) {
	/*
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  panic("Not implemented");
#endif
  return NULL;
	*/
	/* refer nemu/src/device/io/map.c */
	// start
	if (heap_space == NULL) {
		heap_space = heap.start;
	}	
	//printf("use klib's malloc\n");
	// 4 bytes aligned
	size = (size_t)ROUNDUP(size, 4);
	uint8_t *p = heap_space;
	uint8_t *old = heap_space;
	heap_space += size;
	assert( IN_RANGE((void*)heap_space, heap) );	
	for( ; old != heap_space; old ++) {
		*old = 0;
	}
	return (void *)p;
}

void free(void *ptr) {
}

<<<<<<< HEAD
=======
/*
>>>>>>> tracer-ysyx
static void reverse(char s[]) {
	char c;
	int i, j;
	for(i = 0, j = strlen(s) - 1; i < j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}	
}

void itoa(int n, char s[]) {
	int i = 0; 
	int sign = 0;
	
	if ( (sign = n) < 0) 
		n = -n;
	
	do{ 
		s[i++] = n % 10 + '0';
	}while((n /= 10) > 0);
	
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}
<<<<<<< HEAD
=======
*/
>>>>>>> tracer-ysyx
#endif
