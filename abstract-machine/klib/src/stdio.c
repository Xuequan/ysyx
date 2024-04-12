#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// max char number to be print by printf();
#define MAX_CHAR_CNT 512

/* write output to stdout */
int printf(const char *fmt, ...) {
	char buf[MAX_CHAR_CNT];
	/* cannot use __VA_ARGS__ ?
	if (sprintf(buf, fmt, __VA_ARGS__) < 0){
		return -1;
	}
	*/
	// copy all __VA_ARGS__ 
	//void va_copy(va_list dest, va_list src);
	va_list ap;
	va_list aq;
	va_start(ap, fmt);
	va_copy(aq, ap);
	va_end(ap);
	if (sprintf(buf, fmt, aq) < 0){
		putstr("printf() error!\n");
		return -1;
	}
	// from klib-macros.h
	putstr(buf);
	return 0;
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
	int ival;
	char *sval;
	// index 指向out[] 最后的 '\0'
	int index = 0;

	va_start(ap, fmt);

	char c;
	while ((c = *fmt)) {
		switch (c) {
			case '%':
				break;

      case 's':
        sval = va_arg(ap, char *); 
        //printf("sval = %s\n", sval);
        memcpy(out + index, sval, strlen(sval));
        index += strlen(sval);
        break;

      case 'd':   
        ival = va_arg(ap, int);
        char buf2[5] = {0};
        itoa(ival, buf2); 
        //printf("ival = %d, buf2 = %s\n", ival, buf2);
        memcpy(out+index, buf2, strlen(buf2));
        index += strlen(buf2);
        break;

      default:
        //printf("default = %c\n", c); 
        out[index++] = c;
        out[index] = '\0';
        break;
    }   
    fmt++;
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
