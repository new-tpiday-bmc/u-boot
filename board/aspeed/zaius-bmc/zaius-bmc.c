/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * (C) Copyright 2003
 * Texas Instruments, <www.ti.com>
 * Kshitij Gupta <Kshitij@ti.com>
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <i2c.h>
#include <netdev.h>
#include <net.h>

#include <asm/arch/ast_scu.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/io.h>

/* TODO: Move this to GPIO specific file */
#define GPIO_DVR			(0x0)
#define GPIO_DDR			(0x4)
#define GPIO_D3			(1 << 27)

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif

int board_init(void)
{
#if defined(CONFIG_SET_PWM_DUTY)
	unsigned long reg;
        unsigned long duty = 0x00200020;
        printf("init duty = %x\n",reg);

        /* adress of boot parameters */
        gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

        //The system boot status LED  lit up by u-boot#GPIOD5
        //set GPIOD5 data low
        reg = readl(AST_GPIO_BASE | 0x00);
        reg = reg & ~(1<<29);
        writel(reg, AST_GPIO_BASE | 0x00);
        //set GPIOD5 direction output
        reg = readl(AST_GPIO_BASE | 0x04);
        reg = reg | (1<<29);
        writel(reg, AST_GPIO_BASE | 0x04);

        //disable watchdog2 to bump to linux
        writel(0x0, 0x1e78502c);
        
        printf("start set PWM duty\n");
        /* Set PWM duty to 19% */
        reg = *((volatile ulong*) 0x1e6e2088);
        reg |= 0x3e;  // enable PWM1~6 function pin
        *((volatile ulong*) 0x1e6e2088) = reg;
        printf("0x1e6e2088 = %x\n",reg);

        // reset PWM
        reg = *((volatile ulong*) 0x1e6e2004);
        reg &= ~(0x200); /* stop the reset */
        *((volatile ulong*) 0x1e6e2004) = reg;
        printf("0x1e6e2004 = %x\n",reg);

        // enable clock and and set all tacho/pwm to type M
        *((volatile ulong*) 0x1e786000) = 1;
        *((volatile ulong*) 0x1e786040) = 1;
        /* set clock division and period of type M/N */
        /* 0xFF11 --> 24000000 / (2 * 2 * 256) = 23437.5 Hz */
        *((volatile ulong*) 0x1e786004) = 0xFF11FF11;
        *((volatile ulong*) 0x1e786044) = 0xFF11FF11;

        /* 256*19% =50pwm =0x0030 || 0x30FF */
        //PWM0-1
        *((volatile ulong*) 0x1e786008) = duty;
        printf("0x1e786008 = %x\n",duty);
        //PWM2-3
        *((volatile ulong*) 0x1e78600c) = duty;
        printf("0x1e78600c = %x\n",duty);
        //PWM4-5
        *((volatile ulong*) 0x1e786048) = duty;
        printf("0x1e786048 = %x\n",duty);
        //PWM6-7
        *((volatile ulong*) 0x1e78604C) = duty;
        printf("0x1e78604C = %x\n",duty);

        *((volatile ulong*) 0x1e786010) = 0x10000001;
        *((volatile ulong*) 0x1e786018) = 0x10000001;
        *((volatile ulong*) 0x1e786014) = 0x10000000;
        *((volatile ulong*) 0x1e78601c) = 0x10000000;
        *((volatile ulong*) 0x1e786020) = 0;
        *((volatile ulong*) 0x1e786000) = 0xf01;
        *((volatile ulong*) 0x1e786040) = 0xf01;

#else

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
    
	//The system boot status LED  lit up by u-boot#GPIOD5
	//set GPIOD5 data low
	reg = readl(AST_GPIO_BASE | 0x00);
	reg = reg & ~(1<<29);
	writel(reg, AST_GPIO_BASE | 0x00);
	//set GPIOD5 direction output
	reg = readl(AST_GPIO_BASE | 0x04);
	reg = reg | (1<<29);
	writel(reg, AST_GPIO_BASE | 0x04);

	//disable watchdog2 to bump to linux
	writel(0x0, 0x1e78502c);
#endif
	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	u32 vga = ast_scu_get_vga_memsize();
	u32 dram = ast_sdmc_get_mem_size();
	gd->ram_size = (dram - vga);

	return 0;
}

void reset_phy(void)
{
	setbits_le32(AST_GPIO_BASE + GPIO_DDR, GPIO_D3);
	clrbits_le32(AST_GPIO_BASE + GPIO_DVR, GPIO_D3);
	mdelay(2);
	setbits_le32(AST_GPIO_BASE + GPIO_DVR, GPIO_D3);
}

int board_eth_init(bd_t *bd)
{
	debug("Board ETH init\n");
#ifdef CONFIG_FTGMAC100
	return ftgmac100_initialize(bd);
#elif defined(CONFIG_ASPEEDNIC)
	return aspeednic_initialize(bd);
#endif
}
