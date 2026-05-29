#include <string.h> // memmove
#include "autoconf.h" // CONFIG_SERIAL_BAUD
#include "board/io.h" // readb
//#include "board/irq.h" // irq_save
#include "board/misc.h" // console_sendf
#include "board/pgm.h" // READP
//#include "command.h" // DECL_CONSTANT
#include "sched.h" // sched_wake_tasks
#include "uart_debug.h"
#include "stm32g0b0xx.h"

#define RX_BUFFER_SIZE 192

static uint8_t receive_buf[RX_BUFFER_SIZE], receive_pos;
uint8_t cprf = 0;

void debug_utoa_hex(unsigned int value, char *buf)
{
    const char hex_chars[] = "0123456789ABCDEF";
    int i = 0;
    char tmp[9];  // 최대 8자리 + '\0'

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value && i < 8) {
        tmp[i++] = hex_chars[value & 0xF];
        value >>= 4;
    }

    // reverse
    int j = 0;
    while (i > 0)
        buf[j++] = tmp[--i];

    buf[j] = '\0';
}

void debug_itoa(int value, char *buf)
{
    char tmp[16];
    int i = 0, j = 0;
    int is_negative = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    while (value && i < (int)sizeof(tmp) - 1) {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative)
        tmp[i++] = '-';

    while (i > 0)
        buf[j++] = tmp[--i];

    buf[j] = '\0';
}

void debug_print(const char *str)
{
    while (*str)
    {
        while (!(USART1->ISR & USART_ISR_TXE_TXFNF))
            ;

        USART1->TDR = *str++;
         while (!(USART1->ISR & USART_ISR_TC));
    }

}

void debug_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char ch;
    while ((ch = *fmt++) != '\0') {
        if (ch == '%') {
            char next = *fmt++;
            if (next == 'd')
            {
                int val = va_arg(args, int);
                char numbuf[16];
                debug_itoa(val, numbuf);
                debug_print(numbuf);
            }
            else if (next == 'x')
            {
                unsigned int val = va_arg(args, unsigned int);
                char numbuf[16];
                debug_utoa_hex(val, numbuf);
                debug_print(numbuf);
            }
            else if (next == 'c')
            {
                char val = (char)va_arg(args, int);
                char tmp[2] = { val, '\0' };
                debug_print(tmp);
            }
            else if (next == 's')
            {
                char *str = va_arg(args, char *);
                debug_print(str);
            }
            else
            {
                debug_print("%");
                if (next)
                    debug_print((char[]){next, 0});
            }
        } else {
            debug_print((char[]){ch, 0});
        }
    }

    va_end(args);
}

void debug_task(void)
{
    while (USART1->ISR & USART_ISR_RXNE_RXFNE)
    {
        uint8_t c = USART1->RDR;

        if (c == '\b' || c == 0x7F)
        {
            if (receive_pos > 0)
            {
                receive_pos--;
                debug_print("\b \b");
            }
        }
        else if (c == '\r' || c == '\n')
        {
            receive_buf[receive_pos] = '\0';
            debug_print("\r\n");
            if (!strcmp((char *)receive_buf, "help"))
            {
                debug_print("help\r\n");
            }
            else if(!strcmp((char *)receive_buf, "cprf"))
            {
                if(cprf == 0)
                {
                    cprf = 1;
                    debug_print("Debug ON\r\n");
                }
                else
                {
                    cprf = 0;
                    debug_print("Debug OFF\r\n");
                }
            }
            else if(!strcmp((char *)receive_buf, "reset"))
            {
                __disable_irq(); // 인터럽트 비활성화
                NVIC_SystemReset();  // CMSIS 제공 함수, 내부적으로 SCB->AIRCR 제어
                while (1);
            }
            else
            {
                debug_print("unknown command!!!!\r\n");
            }

            receive_pos = 0;
            memset(receive_buf, 0, sizeof(receive_buf));
            debug_printf(PROMPT);
        }
        else
        {
            if (receive_pos < RX_BUFFER_SIZE - 1)
            {
                receive_buf[receive_pos++] = c;
                // echo back (사용자 입력이 보이게)
                char echo[2] = { c, '\0' };
                debug_print(echo);
            }
        }
    }
}
DECL_TASK(debug_task);
