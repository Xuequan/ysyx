#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
//#if defined(__NATIVE_USE_KLIB__)

// internal flag definitions   
#define FLAGS_ZEROPAD   (1U <<  0U)
#define FLAGS_LEFT      (1U <<  1U)
#define FLAGS_PLUS      (1U <<  2U)
#define FLAGS_SPACE     (1U <<  3U)
#define FLAGS_HASH      (1U <<  4U)
#define FLAGS_UPPERCASE (1U <<  5U)
#define FLAGS_CHAR      (1U <<  6U)
#define FLAGS_SHORT     (1U <<  7U)
#define FLAGS_LONG      (1U <<  8U)
#define FLAGS_LONG_LONG (1U <<  9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

#define PRINTF_NTOA_BUFFER_SIZE 32U

// output function type
typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);

static inline void _out_null(char character, void* buffer, size_t idx, size_t maxlen)
{
	(void)character; (void)buffer; (void)idx; (void)maxlen;
}

// internal _putchar wrapper
static inline void _out_char(char character, void* buffer, size_t idx, size_t maxlen)
{
	(void)buffer; (void)idx; (void)maxlen;
	if (character){
		putch(character);
	}
}

// internal buffer wrapper
static inline void _out_buffer(char character, void* buffer, size_t idx, size_t maxlen)
{
	if (idx < maxlen) {
		((char* )buffer)[idx] = character;
	}
}

static inline bool _is_digit(char ch)
{
	return (ch >= '0') && (ch <= '9');
}

static unsigned int _atoi(const char** str)
{
	unsigned int i = 0U;
	while(_is_digit(**str)) {
		i = i * 10U + (unsigned int)(*((*str)++) - '0');
	}
	return i;
}

// output the spceified string in revise, taking case of any zero-padding
static size_t _out_rev(out_fct_type out, char* buffer, size_t idx, size_t maxlen,
		const char* buf, size_t len, unsigned int width, unsigned int flags) 
{
	const size_t start_idx = idx;
	
	// pad spaces up to given width
	if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD)) {
		for (size_t i = len; i < width; i++) {
			out(' ', buffer, idx++, maxlen);
		}
	}

	// reverse string
	while (len) {
		out(buf[--len], buffer, idx++, maxlen);
	}
	
	// operand pad spaces up to given width
	if (flags & FLAGS_LEFT) {
		while (idx - start_idx < width) {
			out(' ', buffer, idx++, maxlen);
		}
	}
	
	return idx;
}	
// internal itoa format
static size_t _ntoa_format(out_fct_type out, char *buffer, size_t idx, size_t maxlen,
					char* buf, size_t len, bool negative, unsigned int base, unsigned int prec,
					unsigned int width, unsigned int flags)
{
	// pad leading zeros
	if (!(flags & FLAGS_LEFT)) {
		if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
			width--;
		}
		while((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			buf[len++] = '0';
		}
		while((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			buf[len++] = '0';
		}
	}
	// handle hash
	if (flags & FLAGS_HASH) {
		if (!(flags & FLAGS_PRECISION) && len && ((len == prec) || (len == width))) {
			len--;
			if (len && (base == 16U)) {
				len--;
			}
		}
		if ((base == 16U) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			buf[len++] = 'x';
		}
		else if ((base == 16U) && (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
			buf[len++] = 'X';
		}
		if (len < PRINTF_NTOA_BUFFER_SIZE) {
			buf[len++] = '0';
		}
	}
	
	if (len < PRINTF_NTOA_BUFFER_SIZE) {
		if (negative) {
			buf[len++] = '-';
		}
		else if (flags & FLAGS_PLUS) {
			buf[len++] = '+';
		}
		else if (flags & FLAGS_SPACE) {
			buf[len++] = ' ';
		}
	}
	
	return _out_rev(out, buffer, idx, maxlen, buf, len, width, flags);
}
		
static size_t _ntoa_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, 
					unsigned long value, bool negative, unsigned long base, unsigned int prec,
					unsigned int width, unsigned int flags)
{
	char buf[PRINTF_NTOA_BUFFER_SIZE];
	size_t len = 0U;

	// no hash for 0 values;
	if (!value) {
		flags &= ~FLAGS_HASH;
	}
	
	// write if precision != 0 and value is != 0
	if (!(flags & FLAGS_PRECISION) || value) {
		do {
			const char digit = (char)(value % base);
			buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
			value /= base;
		} while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
	}
	return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width,
		flags);
}

static size_t _ntoa_long_long(out_fct_type out, char* buffer, size_t idx, size_t maxlen, 
					unsigned long long value, bool negative, unsigned long long base, unsigned int prec,
					unsigned int width, unsigned int flags)
{
	char buf[PRINTF_NTOA_BUFFER_SIZE];
	size_t len = 0U;

	// no hash for 0 values;
	if (!value) {
		flags &= ~FLAGS_HASH;
	}
	
	// write if precision != 0 and value is != 0
	if (!(flags & FLAGS_PRECISION) || value) {
		do {
			const char digit = (char)(value % base);
			buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
			value /= base;
		} while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
	}
	return _ntoa_format(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width,
		flags);
}

// internal secure strlen
// return the length of the string (excluding the terminating 0) limited by maxsize
static inline unsigned int _strnlen_s(const char* str, size_t maxsize) 
{
	const char* s;
	for(s = str; *s && maxsize--; ++s);
	return (unsigned int)(s - str);
}

// format specifier: %[flags][width][.precision][length]
static int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format,
											va_list va)
{
	unsigned int flags, width, precision, n;
	size_t idx = 0U;

	if (!buffer) {
		// use null output function
		out = _out_null;
	}

	while(*format)
	{
		if (*format != '%') {
			out(*format, buffer, idx++, maxlen);
			format++;
			continue;
		}
		else {
			format++;
		}
		
		// evaluate flags
		flags = 0U;
		do {
			switch (*format) {
				case '0': flags |= FLAGS_ZEROPAD; format++; n = 1U; break;
				case '-': flags |= FLAGS_LEFT; 		format++; n = 1U; break;
				case '+': flags |= FLAGS_PLUS;    format++; n = 1U; break;
				case ' ': flags |= FLAGS_SPACE;   format++; n = 1U; break;
				case '#': flags |= FLAGS_HASH;		format++; n = 1U; break;
				default:																		n = 0U; break;
			}
		} while(n);

		// evaluate width field
		width = 0U;
		if (_is_digit(*format)) {
			width = _atoi(&format);
		} else if (*format == '*') {
			const int w = va_arg(va, int);
			if (w < 0) {
				flags |= FLAGS_LEFT;
				width = (unsigned int)-w;
			}else {
				width = (unsigned int) w;
			}
			format++;
		}

		// evaluate precision field
		precision = 0U;
		if (*format == '.'){
			flags |= FLAGS_PRECISION;
			format++;
			if(_is_digit(*format)) {
				precision = _atoi(&format);
			}else if (*format == '*') {
				const int prec = (int)va_arg(va, int);
				precision = prec > 0 ? (unsigned int)prec : 0U;
				format++;
			}
		}
				
		// evaluate length field
		// 暂时只支持 'l', 'll'
		switch(*format) {
			case 'l':
				flags |= FLAGS_LONG;
				format++;
				if (*format == 'l'){
					flags |= FLAGS_LONG_LONG;
					format++;
				}
				break;

			default:
				break;
		}
	
		// evaluate specifier
		// 暂时只支持 正数 d, i, x, X
		switch (*format) {
			case 'd':
			case 'i':
			case 'x':
			case 'X':{
				// set the base
				unsigned int base;
				if (*format == 'x' || *format == 'X'){
					base = 16U;
				}else{
					base = 10U;
					flags &= ~FLAGS_HASH;
				}	

			  if (*format == 'X'){
					flags |= FLAGS_UPPERCASE;
				}
				
				if ((*format != 'i') && (*format != 'd')){
					flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
				}
				
				// ignore '0' flag when precision is given
				if (flags & FLAGS_PRECISION) {
					flags &= ~FLAGS_ZEROPAD;
				}
			
				// convert the intger
				if ((*format == 'i') || (*format == 'd')) {
					// signed
					if (flags & FLAGS_LONG_LONG){
						const long long value = va_arg(va, long long);
						idx = _ntoa_long_long(out, buffer, idx, maxlen, 
							(unsigned long long)(value > 0 ? value : 0 - value), 
							value < 0, base, precision, width, flags);
					} else if (flags & FLAGS_LONG) {
						const long value = va_arg(va, long);
						idx = _ntoa_long(out, buffer, idx, maxlen,
							(unsigned long)(value > 0 ? value : 0 - value),
							value < 0, base, precision, width, flags);
					} else {
						const int value = va_arg(va, int);
						idx = _ntoa_long(out, buffer, idx, maxlen,
							(unsigned long)(value > 0 ? value : 0 - value),
							value < 0, base, precision, width, flags);
					} 
				}
				else {
						// unsigned
					if (flags & FLAGS_LONG_LONG){
						const unsigned long long value = va_arg(va, unsigned long long);
						idx = _ntoa_long_long(out, buffer, idx, maxlen, 
								value, 
								false, base, precision, width, flags);
					} else if (flags & FLAGS_LONG) {
						const unsigned long value = va_arg(va, unsigned long);
						idx = _ntoa_long(out, buffer, idx, maxlen,
								value,
								false, base, precision, width, flags);
					} else {
						const unsigned int value = va_arg(va, unsigned int);
							idx = _ntoa_long(out, buffer, idx, maxlen,
								value,
								false, base, precision, width, flags);
					}
				}

				format++;
				break;
			} // end case 'X':{
					
			case 'c': {
					unsigned int l = 1U;
					// pre padding
					if (!(flags && FLAGS_LEFT)) {
						while (l++ < width) {
							out(' ', buffer, idx++, maxlen);
						}
					}
					// char output
					out((char)va_arg(va, int), buffer, idx++, maxlen);
					// past padding
					if (flags && FLAGS_LEFT) {
						while (l++ < width) {
							out(' ', buffer, idx++, maxlen);
						}
					}
					format++;
					break;
			} // end case 'c'

			case 's': {
					const char *p = va_arg(va, char *);
					unsigned int l = _strnlen_s(p, precision ? precision : (size_t)-1);
					// pre padding
					if (flags && FLAGS_PRECISION) {
						l = ( l < precision ? l : precision);
					}
					if (!(flags && FLAGS_LEFT)) {
						while (l++ < width) {
							out(' ', buffer, idx++, maxlen);
						}
					}
					// string output
					while ((*p != 0) && (!(flags & FLAGS_PRECISION) || precision--)) {
						out(*(p++), buffer, idx++, maxlen);
					}
					// past padding
					if (flags && FLAGS_LEFT) {
						while (l++ < width) {
							out(' ', buffer, idx++, maxlen);
						}
					}
					format++;
					break;
			} // end case 's'
					
			case '%':
					out('%', buffer, idx++, maxlen);
					format++;
					break;

				default:
					out(*format, buffer, idx++, maxlen);
					format++;
					break;
			} // end switch (*format) {
	
	}	// end while(*format)

		// termination
	out((char)0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);
		
	// return written chars without terminating \0
	return (int)idx;
}		
			
/* =============================================================== */
/* write output to stdout */
int printf(const char* fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char buffer[1];
	const int ret = _vsnprintf(_out_char, buffer, (size_t)-1, fmt, va);
	va_end(va);
	return ret;
}					

/* write to the character string *out  */
int vsprintf(char *out, const char *fmt, va_list ap) {
	const int ret = _vsnprintf(_out_buffer, out, (size_t)-1, fmt, ap);
	return ret;
}

/* write to the character string *out;
** upon successful return, these functions return the number of characters 
** printed (excluding the null byte used to end output to string);
** A negative value is returned if an output error encountered;
*/
int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	const int ret = _vsnprintf(_out_buffer, out, (size_t)-1, fmt, ap);
	va_end(ap);
	return ret;
}

/* write to the character string *out  */
int snprintf(char *out, size_t n, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	const int ret = _vsnprintf(_out_buffer, out, n, fmt, va);
	va_end(va);
	return ret;
}					

/* write to the character string *out  */
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
	return _vsnprintf(_out_buffer, out, n, fmt, ap);
}

#endif

