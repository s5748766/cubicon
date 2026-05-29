#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include <stdint.h> // uint32_t
#include <stdio.h>

void debug_print(const char *str);
void debug_printf(const char *fmt, ...);

#endif
