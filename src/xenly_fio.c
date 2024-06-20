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

// print
void print(const char *format, ...) {
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
        }
        
        else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            putchar(*format);
        }
        format++;
    }

    va_end(args);
}

// sprint
void sprint(char *str, const char *format, ...) {
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
        }
        
        else {
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

// input
void input(const char *format, ...) {
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


// println
void println(const char *format, ...) {
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
        }
        
        else {
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

// scanln
void scanln(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0'; // Remove newline character
        }
    }
}

// sprintln
void sprintln(char *str, const char *format, ...) {
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
        }
        
        else {
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

// error
void error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

// errorln
void errorln(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    va_end(args);
}

// write
void write(FILE *stream, const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    fprintf(stream, "%d", i);
                    break;
                }
                case 'f': {
                    double f = va_arg(args, double);
                    fprintf(stream, "%f", f);
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    fputc(c, stream);
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    fprintf(stream, "%s", s);
                    break;
                }
                // Add more cases as needed
                default:
                    fputc(*format, stream);
                    break;
            }
        }
        
        else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            fputc(*format, stream);
        }
        format++;
    }

    va_end(args);
}

// writeln
void writeln(FILE *stream, const char *format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    fprintf(stream, "%d", i);
                    break;
                }
                case 'f': {
                    double f = va_arg(args, double);
                    fprintf(stream, "%f", f);
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    fputc(c, stream);
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    fprintf(stream, "%s", s);
                    break;
                }
                // Add more cases as needed
                default:
                    fputc(*format, stream);
                    break;
            }
        }
        
        else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            fputc(*format, stream);
        }
        format++;
    }
    fputc('\n', stream);

    va_end(args);
}

// vfprint
void vfprint(FILE *stream, const char *format, va_list args) {
    while (*format) {
        if (*format == '%' && *(format + 1) != '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int i = va_arg(args, int);
                    fprintf(stream, "%d", i);
                    break;
                }
                case 'u': {
                    unsigned int u = va_arg(args, unsigned int);
                    fprintf(stream, "%u", u);
                    break;
                }
                case 'o': {
                    unsigned int o = va_arg(args, unsigned int);
                    fprintf(stream, "%o", o);
                    break;
                }
                case 'x': {
                    unsigned int x = va_arg(args, unsigned int);
                    fprintf(stream, "%x", x);
                    break;
                }
                case 'X': {
                    unsigned int X = va_arg(args, unsigned int);
                    fprintf(stream, "%X", X);
                    break;
                }
                case 'f': {
                    double f = va_arg(args, double);
                    fprintf(stream, "%f", f);
                    break;
                }
                case 'e': {
                    double e = va_arg(args, double);
                    fprintf(stream, "%e", e);
                    break;
                }
                case 'E': {
                    double E = va_arg(args, double);
                    fprintf(stream, "%E", E);
                    break;
                }
                case 'g': {
                    double g = va_arg(args, double);
                    fprintf(stream, "%g", g);
                    break;
                }
                case 'G': {
                    double G = va_arg(args, double);
                    fprintf(stream, "%G", G);
                    break;
                }
                case 'c': {
                    int c = va_arg(args, int);
                    fputc(c, stream);
                    break;
                }
                case 's': {
                    char *s = va_arg(args, char *);
                    fprintf(stream, "%s", s);
                    break;
                }
                case 'p': {
                    void *p = va_arg(args, void *);
                    fprintf(stream, "%p", p);
                    break;
                }
                case 'n': {
                    int *n = va_arg(args, int *);
                    *n = ftell(stream);
                    break;
                }
                default:
                    fputc(*format, stream);
                    break;
            }
        }
        
        else {
            if (*format == '%' && *(format + 1) == '%') {
                format++;
            }
            fputc(*format, stream);
        }
        format++;
    }
}

// vfprintln
void vfprintln(FILE *stream, const char *format, va_list args) {
    vfprint(stream, format, args);
    fputc('\n', stream);
}
