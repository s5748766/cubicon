#ifndef __STM32_FLASH_H
#define __STM32_FLASH_H

#include <stdint.h>

int flash_write_block(uint32_t block_address, uint32_t *data);
int flash_complete(void);



void flash_write_page127_flag(void);
void flash_read_page127_flag(void);
void wake_flash_write_func(void);
void wake_flash_read_func(void);

#endif
