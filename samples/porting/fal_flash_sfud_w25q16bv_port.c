/*
 * File      : fal_flash_sfud_port.c
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

#include <sfud.h>

#include <rtdevice.h>
#include "spi_flash.h"

static sfud_flash *sfud_norflash0 = RT_NULL;

static int init(void)
{
    static rt_spi_flash_device_t rtt_dev = RT_NULL;

    rtt_dev = (rt_spi_flash_device_t)rt_sfud_flash_probe("flash0", "spi10");
    if (rtt_dev == RT_NULL)
    {
        rt_kprintf("rt_sfud_flash_probe failed! \n");
        return 0;
    }

    sfud_norflash0 = (sfud_flash_t)rtt_dev->user_data;
    if (sfud_norflash0->chip.capacity < 1024 * 1024)
    {
        rt_kprintf("%d KB %s is current selected device.\n", \
          sfud_norflash0->chip.capacity / 1024, sfud_norflash0->name);
    } 
    else
    {
        rt_kprintf("%d MB %s is current selected device.\n", \
          sfud_norflash0->chip.capacity / 1024 / 1024, sfud_norflash0->name);
    }

    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    assert(sfud_norflash0->init_ok);
    sfud_read(sfud_norflash0, nor_flash0.addr + offset, size, buf);

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    assert(sfud_norflash0->init_ok);
    if (sfud_write(sfud_norflash0, nor_flash0.addr + offset, size, buf) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

static int erase(long offset, size_t size)
{
    assert(sfud_norflash0->init_ok);
    if (sfud_erase(sfud_norflash0, nor_flash0.addr + offset, size) != SFUD_SUCCESS)
    {
        return -1;
    }

    return size;
}

const struct fal_flash_dev nor_flash0 = { "w25q16bv", 0, 2*1024*1024, 4096, {init, read, write, erase} };
