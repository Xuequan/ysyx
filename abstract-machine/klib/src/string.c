#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  panic("Not implemented");
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

int strcmp(const char *s1, const char *s2) {
  panic("Not implemented");
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
