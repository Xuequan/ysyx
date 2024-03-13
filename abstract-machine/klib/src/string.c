#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/* strlen() function calculates the length of the string pointed to by s,
** excluding the terminating null byte ('\0').
*/
size_t strlen(const char *s) {
	assert(s != NULL);
	size_t i = 0;
	char *ptr = (char *)s;
	for ( ; *ptr != '\0'; i++)
		ptr++;
	return i;
}


char *strcpy(char *dst, const char *src) {
  panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  panic("Not implemented");
}

/* return 0 if s1 and s2 are equal;
** return 1 if s1 is greater than s2;
** return -1 if s1 is less than s2;
*/
int strcmp(const char *s1, const char *s2) {
	char *ptr1 = (char *) s1;
	char *ptr2 = (char *) s2;
	while (*ptr1 != '\0' && *ptr2 != '\0') {
		if (*ptr1 == *ptr2) { 
			ptr1++;
			ptr2++;
		} else if (*ptr1 > *ptr2) { 
			return 1;	
		} else {
			return -1;
		}
	}	
	if (*ptr1 == '\0' && *ptr2 == '\0') 
		return 0;
	else if (*ptr1 != '\0') 
		return 1;
	else 
		return -1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

/* memcpy() copies n bytes from memory area src to memory area dest.
** The memory area must not overloap.
** Return a pointer to dest
*/
void *memcpy(void *out, const void *in, size_t n) {
	if ( n == 0 || in == NULL || out == NULL) 
		return out;

	uint8_t *p1 = (uint8_t *) out;
	uint8_t *p2 = (uint8_t *) in;
	for ( ; n != 0; n--) {
		*p1 = *p2;
		p1++;
		p2++;
	}
	*p1 = '\0';
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  panic("Not implemented");
}

#endif
