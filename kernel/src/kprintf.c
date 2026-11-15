
#include "kprintf.h"
#include "terminal.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

static void print_string(const char *str)
{
    if (str == NULL) {
        str = "(null)";
    }

    while (*str != '\0') {
        terminal_putchar(*str);
        str++;
    }
}

static void print_unsigned(unsigned int value, unsigned int base)
{
    char buffer[32];
    size_t index = 0;

    const char digits[] = "0123456789abcdef";

    if (base < 2 || base > 16) {
        return;
    }

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0) {
        buffer[index++] = digits[value % base];
        value /= base;
    }

    while (index > 0) {
        terminal_putchar(buffer[--index]);
    }
}

static void print_integer(int value)
{
    unsigned int magnitude;

    if (value < 0) {
        terminal_putchar('-');
        magnitude = (unsigned int)(-(value + 1)) + 1U;
    } else {
        magnitude = (unsigned int)value;
    }

    print_unsigned(magnitude, 10);
}

void kprintf(const char *format, ...)
{
    if (format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);

    while (*format != '\0') {

        if (*format != '%') {
            terminal_putchar(*format++);
            continue;
        }

        format++;

        if (*format == '\0') {
            break;
        }

        switch (*format) {

            case 'c':
                terminal_putchar((char)va_arg(args, int));
                break;

            case 's':
                print_string(va_arg(args, const char *));
                break;

            case 'd':
                print_integer(va_arg(args, int));
                break;

            case 'u':
                print_unsigned(va_arg(args, unsigned int), 10);
                break;

            case 'x':
                print_unsigned(va_arg(args, unsigned int), 16);
                break;

            case '%':
                terminal_putchar('%');
                break;

            default:
                terminal_putchar('%');
                terminal_putchar(*format);
                break;
        }

        format++;
    }

    va_end(args);
}
