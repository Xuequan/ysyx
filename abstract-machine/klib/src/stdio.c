#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

/* write output to stdout */
int printf(const char *fmt, ...) {
  panic("Not implemented");
}

/* write to the character string *out  */
int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

/* write to the character string *out;
** upon successful return, these functions return the number of characters 
** printed (excluding the null byte used to end output to string);
** A negative value is returned if an output error encountered;
*/
int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	int d;
	char *s;
	char *ptr = out;
	va_start(ap, fmt);
	while (*fmt) {
		switch (*fmt++) {
			case 's':
				s = va_arg(ap, char *);
				memcpy(ptr, s, sizeof(s));
				ptr = ptr + sizeof(s);
				break;
			case 'd':
				d = va_arg(ap, int);
				memcpy(ptr, &d, sizeof(d));
				ptr = ptr + sizeof(d);
				break;
			default:
				break;	
		}
	} // end while
	va_end(ap);
	return 0;
}

/* write to the character string *out  */
int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

/* write to the character string *out  */
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
