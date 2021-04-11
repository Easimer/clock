#ifndef CLOCK_KPRINTF_H
#define CLOCK_KPRINTF_H

#define LOG_SUCCESS "[+] "
#define LOG_FAILURE "[-] "
#define LOG_ERROR   "[!] "

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A printf function that produces an output using a format string
 * and several arguments.
 * 
 * This function only supports a strict subset of directives:
 * - %b - unsigned 8-bit number
 * - %w - unsigned 16-bit number
 * - %s - pointer to a zero-terminated string
 * - %% - percentage sign literal
 * 
 * The generated output is sent to the serial port.
 */
void kprintf(char const *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_KPRINTF_H */