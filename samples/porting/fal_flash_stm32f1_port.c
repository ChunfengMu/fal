/*
 * File      : fal_flash_stm32f2_port.c
 * This file is part of FAL (Flash Abstraction Layer) package
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <fal.h>

#include "stm32f1xx_hal_flash.h"

/* Note: must define FLASH_MAX_SIZE,which equal with chip size(K) */

#define MIN(a, b) (a < b ? a : b)

static int init(void)
{
    /* do nothing now */
}

static int read(long offset, uint8_t *buf, size_t size)
{
    uint32_t addr = stm32f1_onchip_flash.addr + offset;

    for (; size > 0; size -= 1, addr += 1, buf++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    uint32_t addr_base;
    uint32_t addr = stm32f1_onchip_flash.addr + offset;
    uint32_t data;
    uint32_t buf_offset = 0;
    size_t total_size = size, request_size;
    uint8_t i, tmp_buf[4];

    HAL_FLASH_Unlock();
    if(addr%0x04 != 0x00)
    {
        addr_base = addr&(~(0x04 - 1));
        for(i = 0; i < 0x04; i++)
        {
            tmp_buf[i] = *(uint8_t *) (addr_base+i);
        }

        request_size = MIN(0x04 - addr&(0x04 - 1), total_size);
        for(i = (0x04 - (0x04 - addr&(0x04 - 1))); \
            i < (request_size + (0x04 - (0x04 - addr&(0x04 - 1)))); i++)
        {
            tmp_buf[i] = *(buf + buf_offset);
            buf_offset++;
        }

        data = ((unsigned long) (tmp_buf)[0]) \
        | ((unsigned long) (tmp_buf)[1] << 8) \
        | ((unsigned long) (tmp_buf)[2] << 16) \
        | ((unsigned long) (tmp_buf)[3] << 24);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr_base, data);

        total_size -= request_size;
        addr_base += 0x04;
    }
    else
    {
        addr_base = addr;
    }

    while(total_size)
    {
        request_size = MIN(total_size, 0x04);
        switch(request_size)
        {
            case 4:
            {
                data = ((unsigned long) (buf)[(buf_offset)]) \
                | ((unsigned long) (buf)[(buf_offset) + 1] << 8) \
                | ((unsigned long) (buf)[(buf_offset) + 2] << 16) \
                | ((unsigned long) (buf)[(buf_offset) + 3] << 24);
            }
            break;

            case 3:
            {
                data = ((unsigned long) (buf)[(buf_offset)]) \
                | ((unsigned long) (buf)[(buf_offset) + 1] << 8) \
                | ((unsigned long) (buf)[(buf_offset) + 2] << 16) \
                | ((unsigned long) 0xFF << 24);
            }
            break;

            case 2:
            {
                data = ((unsigned long) (buf)[(buf_offset)]) \
                | ((unsigned long) (buf)[(buf_offset) + 1] << 8) \
                | ((unsigned long) 0xFF << 16) \
                | ((unsigned long) 0xFF << 24);
            }
            break;

            case 1:
            {
                data = ((unsigned long) (buf)[(buf_offset)]) \
                | ((unsigned long) 0xFF << 8) \
                | ((unsigned long) 0xFF << 16) \
                | ((unsigned long) 0xFF << 24);
            }
            break;
        }
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr_base, data);

        addr_base += request_size;
        buf_offset += request_size;
        total_size -= request_size;
    }
    HAL_FLASH_Lock();

    return size;
}

static int erase(long offset, size_t size)
{
    HAL_StatusTypeDef flash_status;
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError;

    uint32_t addr_base, addr_end;
    uint32_t addr = stm32f1_onchip_flash.addr + offset;

    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.NbPages = 1;

    addr_base = addr&(~(FLASH_PAGE_SIZE - 1));
    addr_end = (addr + size)&(~(FLASH_PAGE_SIZE - 1));

    HAL_FLASH_Unlock();
    while(addr_base <= addr_end)
    {
        eraseInit.PageAddress = addr_base;

        flash_status = HAL_FLASHEx_Erase(&eraseInit, &pageError);
        if(flash_status != HAL_OK)
        {
            return -1;
        }
        addr_base += FLASH_PAGE_SIZE;
    }
    HAL_FLASH_Lock();

    return size;
}

const struct fal_flash_dev stm32f1_onchip_flash = { "stm32f1_onchip", 0x08000000, FLASH_MAX_SIZE*1024, FLASH_PAGE_SIZE, {RT_NULL, read, write, erase} };
