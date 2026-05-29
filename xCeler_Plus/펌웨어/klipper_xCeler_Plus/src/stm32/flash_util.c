#include "internal.h"
#include "command.h"
#include "sched.h"  // irq_save, irq_restore 포함
#include "irq.h"
#include "serial_irq.h"
#include "flash_util.h"
#include "generic/irq.h"
#include "uart_debug.h"

static void flash_erase_page(uint32_t page_number);
static void flash_write_word(uint32_t address, uint64_t value);

#define CONFIG_BLOCK_SIZE 1

#define FLASH_PAGE_SIZE           0x800  // 2KB
#define FLASH_BASE_ADDR           0x08000000
#define FLASH_PAGE_63_ADDR       (FLASH_BASE_ADDR + (63 * FLASH_PAGE_SIZE))
#define FLASH_WRITE_VALUE         0x11111111

#define FLASH                 ((FLASH_TypeDef *) FLASH_R_BASE)
#define FLASH_KEY1            0x45670123U
/*!< Flash key1 */
#define FLASH_KEY2            0xCDEF89ABU
/*!< Flash key2: used with FLASH_KEY1*/


static struct task_wake wake_flash_write;
static struct task_wake wake_flash_read;

static uint32_t g_flash_flag = 0;

// Some chips have slightly different register names
#if CONFIG_MACH_STM32G0
#define FLASH_SR_BSY (FLASH_SR_BSY1 | FLASH_SR_BSY2)
#elif CONFIG_MACH_STM32H7
#define CR CR1
#define SR SR1
#define KEYR KEYR1
#endif

// Wait for flash hardware to report ready
static void
wait_flash(void)
{
    while (FLASH->SR & FLASH_SR_BSY)
        ;
}

#ifndef FLASH_KEY1 // Some stm32 headers don't define this
#define FLASH_KEY1 (0x45670123UL)
#define FLASH_KEY2 (0xCDEF89ABUL)
#endif

// Issue low-level flash hardware unlock sequence
static void
unlock_flash(void)
{
    if (FLASH->CR & FLASH_CR_LOCK) {
        // Unlock Flash Erase
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    wait_flash();
}

// Place low-level flash hardware into a locked state
static void
lock_flash(void)
{
    FLASH->CR = FLASH_CR_LOCK;
}

void command_flash_write(uint32_t *args)
{
    g_flash_flag = args[1];
    sched_wake_task(&wake_flash_write);
}
DECL_COMMAND(command_flash_write, "query_flash_write oid=%u flag=%u");

void command_flash_read(uint32_t *args)
{
    sched_wake_task(&wake_flash_read);
}
DECL_COMMAND(command_flash_read, "query_flash_read oid=%u");

static void flash_clear_flags(void)
{
    FLASH->SR = FLASH_SR_EOP     // End of operation
              | FLASH_SR_WRPERR  // Write protect error
              | FLASH_SR_PGAERR  // Programming alignment error
              | FLASH_SR_SIZERR  // Size error
              | FLASH_SR_OPTVERR // Option validity error
              | FLASH_SR_PROGERR // Programming error
              | FLASH_SR_OPERR;  // Operation error
}

static void flash_erase_page(uint32_t page_number)
{
    while (FLASH->SR & FLASH_SR_BSY1);  // wait for not busy

    flash_clear_flags();
    FLASH->CR = (FLASH_CR_STRT |
                (page_number <<  FLASH_CR_PNB_Pos) |
                FLASH_CR_PER);

    while (FLASH->SR & FLASH_SR_BSY1);  // wait until erase complete
    FLASH->CR &= ~FLASH_CR_PER;
}

static void flash_write_word(uint32_t address, uint64_t  value)
{
    volatile uint32_t *dst = (volatile uint32_t *)address;

    uint32_t low = (uint32_t)(value & 0xFFFFFFFF);
    uint32_t high = (uint32_t)((value >> 32U) & 0xFFFFFFFF);

    while (FLASH->SR & FLASH_SR_BSY1);

    flash_clear_flags();

    FLASH->CR |= FLASH_CR_PG;

    *(dst) = low;

    __ISB(); // 명령 순서 보장 (옵션)

    *(dst + 1) = high;

    while (FLASH->SR & FLASH_SR_BSY1);

    if (FLASH->SR & FLASH_SR_EOP)
    {
        debug_printf("EOP detected, clearing...\r\n");
        FLASH->SR |= FLASH_SR_EOP;
    }
    if (FLASH->SR & (FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGSERR))
    {
        debug_printf("Error detected in SR: 0x%x, clearing...\r\n", FLASH->SR);
        FLASH->SR |= FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGSERR;
    }

    FLASH->CR &= ~FLASH_CR_PG;
}

void flash_write_page127_flag(void)
{
    if (!sched_check_wake(&wake_flash_write))
        return;
    irqstatus_t flags = irq_save();
    unlock_flash();
    flash_erase_page(63);
    flash_write_word(FLASH_PAGE_63_ADDR, g_flash_flag);
    lock_flash();
    irq_restore(flags);
}
DECL_TASK(flash_write_page127_flag);

void flash_read_page127_flag(void)
{
    if (!sched_check_wake(&wake_flash_read))
        return;
    uint32_t flash_value = *(uint32_t*)FLASH_PAGE_63_ADDR;
    sendf("response_flash_read flag=%u", flash_value);

}
DECL_TASK(flash_read_page127_flag);
