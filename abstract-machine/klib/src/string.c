#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/* strlen() function calculates the length of the string pointed to by s,
** excluding the terminating null byte ('\0').
*/
size_t strlen(const char *s) {
	size_t i = 0;
	uint8_t *ptr = (uint8_t *)s;
	for ( ; *ptr != '\0'; i++)
		ptr++;
	return i;
}


/* strcpy() copies the string pointed to by src (including the terminating null
** character) into the array pointed to by dst. 
** Returns the value of dst.
*/
char *strcpy(char *dst, const char *src) {
	assert(dst != NULL);
	size_t src_len = strlen(src);
	size_t i;
	
	for( i = 0; i <= src_len; i++)
		dst[i] = src[i];

	return dst;
}

/* strncpy() copies not more than n characters (characters that
** follow a null character are not copied) from the string 
** pointed to by src into the array pointed to by dst. 
** If the array pointed by src is a string that is short than n
** characters, null characters are appended to the copy in 
** the array pointed to by dst, until n characters in all
** have been writted. 
** Returns the value of dst.
*/
char *strncpy(char *dst, const char *src, size_t n) {
	assert(dst != NULL);
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dst[i] = src[i];
	for ( ; i < n; i++)
		dst[i] = '\0';

	return dst; 	
}

/* strcat() appends the src string to the dest string, overwriting
** the terminating null byte ('\0') at the end of dest, 
** and then adds a terminating null byte.
** Return a pointer to the resulting string dest.
*/
char *strcat(char *dst, const char *src) {
	assert(dst != NULL);

	size_t dst_len = strlen(dst);
	size_t i;
	
	for( i = 0; src[i] != '\0'; i++)
		dst[dst_len + i] = src[i];
	dst[dst_len + i] = '\0';

	return dst;	
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
/* strncmp() is similar with strcmp(), except is compares only the first (at most) n bytes of s1 and s2 */
int strncmp(const char *s1, const char *s2, size_t n) {
	char *ptr1 = (char *) s1;
	char *ptr2 = (char *) s2;
	int i = 0;
	while (*ptr1 != '\0' && *ptr2 != '\0' && i < n) {
		if (*ptr1 == *ptr2) { 
			ptr1++;
			ptr2++;
		} else if (*ptr1 > *ptr2) { 
			return 1;	
		} else {
			return -1;
		}
		i++;
	}	
	if (i == n){
		return 0;
	} 
	if (*ptr1 == '\0' && *ptr2 == '\0') 
		return 0;
	else if (*ptr1 != '\0') 
		return 1;
	else 
		return -1;
}

/* memset() function fills the first n bytes of the memory area
** pointed to by s with the constant byte c,
** Return a pointer to the memory area s.
*/
void *memset(void *s, int c, size_t n) {
	assert( s != NULL);
	size_t i;
	
	uint8_t *p = (uint8_t *)s;
	for (i = 0; i < n; i++)
		p[i] = c;
	return s;
}

/* The memmove() function copies n characters from the object pointed to by src 
 * into the object pointed to by dst. Copying takes place as if the n characters
 * from the object pointed to by src are first copied into a temporary array 
 * of n characters that does not overlap the objects pointed to by dst and src,
 * and then the n characters from the temporary array are copied into the 
 * object pointed to by dst.
 * Return - memmove() returns the value of dst.
 */
void *memmove(void *dst, const void *src, size_t n) {
	assert(dst != NULL && src != NULL && n != 0);
	uint8_t buf[n];
	uint8_t *p1 = (uint8_t *)dst;
	const uint8_t *p2 = (uint8_t *)src;
	for(int i = 0; i < n; i++){
		buf[i] = p2[i]; 
	}
	for(int i = 0; i < n; i++){
		p1[i] = buf[i]; 
	}
	return dst;
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

/* memcmp() function compares the first n bytes (each interpreted
** as unsigned char) of the memory areas s1 and s2.
** Rerutn 1, 0, -1 if s1 is greater than, equal to, or
** less than s2 respectively. 
*/
int memcmp(const void *s1, const void *s2, size_t n) {
	assert( s1 != NULL && s2 != NULL);
	unsigned char *ptr1 = (unsigned char *)s1;
	unsigned char *ptr2 = (unsigned char *)s2;
	size_t i;
	
	for( i = 0; i < n && *ptr1 != '\0' && *ptr2 != '\0'; i++) {	
		if (*ptr1 == *ptr2) { 
			ptr1++;
			ptr2++;
		} else if (*ptr1 > *ptr2) { 
			return 1;	
		} else {
			return -1;
		}
	}	

	if ( i == n)
		return 0;
	if (*ptr1 == '\0' && *ptr2 == '\0') 
		return 0;
	else if (*ptr1 != '\0') 
		return 1;
	else 
		return -1;
}

#endif
