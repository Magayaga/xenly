/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * It is available for Linux and Windows operating systems.
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void xenly_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    printf("%d", i);
                    break;
                }
                case 'f': {
                    double f = va_arg(args, double);
                    printf("%f", f);
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    putchar(c);
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    printf("%s", s);
                    break;
                }
                // Add more cases as needed
                default:
                    putchar(*format);
                    break;
            }
        } else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            putchar(*format);
        }
        format++;
    }

    va_end(args);
}

void xenly_sprintf(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *buffer = str;

    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    buffer += sprintf(buffer, "%d", i);
                    break;
                }
                case 'f': {
                    double f = va_arg(args, double);
                    buffer += sprintf(buffer, "%f", f);
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    *buffer++ = (char)c;
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    buffer += sprintf(buffer, "%s", s);
                    break;
                }
                // Add more cases as needed
                default:
                    *buffer++ = *format;
                    break;
            }
        } else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            *buffer++ = *format;
        }
        format++;
    }
    *buffer = '\0';

    va_end(args);
}

void xenly_scanf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int *i = va_arg(args, int *);
                    scanf("%d", i);
                    break;
                }
                case 'f': {
                    float *f = va_arg(args, float *);
                    scanf("%f", f);
                    break;
                }
                case 'c': {
                    char *c = va_arg(args, char *);
                    scanf(" %c", c);
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    scanf("%s", s);
                    break;
                }
                // Add more cases as needed
                default:
                    break;
            }
        }
        format++;
    }

    va_end(args);
}

void xenly_println(const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    printf("%d", i);
                    break;
                }
                case 'f': {
                    double f = va_arg(args, double);
                    printf("%f", f);
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    putchar(c);
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    printf("%s", s);
                    break;
                }
                default:
                    putchar(*format);
                    break;
            }
        } else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            putchar(*format);
        }
        format++;
    }
    putchar('\n');

    va_end(args);
}

// my_scanln
void xenly_scanln(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0'; // Remove newline character
        }
    }
}

// my_sprintln
void xenly_sprintln(char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *buffer = str;

    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    buffer += sprintf(buffer, "%d", i);
                    break;
                }
                case 'f': {
                    double f = va_arg(args, double);
                    buffer += sprintf(buffer, "%f", f);
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    *buffer++ = (char)c;
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    buffer += sprintf(buffer, "%s", s);
                    break;
                }
                default:
                    *buffer++ = *format;
                    break;
            }
        } else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            *buffer++ = *format;
        }
        format++;
    }
    *buffer++ = '\n';
    *buffer = '\0';

    va_end(args);
}

// my_errorf
void xenly_errorf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

// my_errorln
void xenly_errorln(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
}