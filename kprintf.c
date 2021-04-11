#include "kprintf.h"
#include "log.h"
#include <assert.h>
#include <stdarg.h>

void kprintf(char const *fmt, ...) {
    uint16_t cursor = 0;
    va_list ap;
    char buf[2] = { 0, '\0' };

    va_start(ap, fmt);

    while(cursor < 65535 && fmt[cursor] != '\0') {
        buf[0] = fmt[cursor];

        if (buf[0] == '%') {
            cursor++;

            assert(fmt[cursor] != '\0');
            if (fmt[cursor] == '\0') {
                return;
            }

            char directive = fmt[cursor];

            switch (directive) {
            case 's':
                l_str(va_arg(ap, char const *));
                break;
            case 'b':
                l_num((uint8_t)va_arg(ap, int));
                break;
            case 'w':
                l_num((uint16_t)va_arg(ap, int));
                break;
            case '%':
                l_str("%");
                break;
            }
        }
        else {
            l_str(buf);
        }

        cursor++;
    }

    va_end(ap);
}