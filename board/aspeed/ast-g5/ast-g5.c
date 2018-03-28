/*
<<<<<<< HEAD
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
=======
 * Copyright 2016 IBM Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
>>>>>>> f9b2a49f57c153c7358f9769234c0cefdf4de8d4
 */

#include <common.h>
#include <netdev.h>

#include <asm/arch/ast_scu.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/io.h>

#include <asm/arch/platform.h>
#include <asm/arch/regs-ahbc.h>
#include <asm/arch/regs-scu.h>


DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int misc_init_r(void)
{
        u32 reg;
        unsigned long duty = 0x00200020;
        printf("init duty = %x\n",reg);

        /* Unlock AHB controller */
        writel(AHBC_PROTECT_UNLOCK, AST_AHBC_BASE);

        /* Map DRAM to 0x00000000 */
        reg = readl(AST_AHBC_BASE + AST_AHBC_ADDR_REMAP);
        writel(reg | BIT(0), AST_AHBC_BASE + AST_AHBC_ADDR_REMAP);

        /* Unlock SCU */
        writel(SCU_PROTECT_UNLOCK, AST_SCU_BASE);

        printf("Start set pwm duty\n");
        /* Set PWM dytu to 100% */
         reg = *((volatile ulong*) 0x1e6e2088);
         reg |= 0x3e;  // enable PWM1~6 function pin
         *((volatile ulong*) 0x1e6e2088) = reg;
        printf("enable pwm1~6 duty 0x1e6e2088 = %x\n",reg);

        // reset PWM
        reg = *((volatile ulong*) 0x1e6e2004);
        reg &= ~(0x200); /* stop the reset */
        *((volatile ulong*) 0x1e6e2004) = reg;
        printf("reset pwm duty 0x1e6e2088 = %x\n",reg);

       // enable clock and and set all tacho/pwm to type M
        *((volatile ulong*) 0x1e786000) = 1;
        *((volatile ulong*) 0x1e786040) = 1;
       /* set clock division and period of type M/N */
       /* 0xFF11 --> 24000000 / (2 * 2 * 256) = 23437.5 Hz */
       *((volatile ulong*) 0x1e786004) = 0xFF11FF11;
       *((volatile ulong*) 0x1e786044) = 0xFF11FF11;
       //PWM0-1
       *((volatile ulong*) 0x1e786008) = duty;
        printf("pwm0~1 duty 0x1e786008 = %x\n",reg);
       //PWM2-3
       *((volatile ulong*) 0x1e78600c) = duty;
        printf("pwm2~3 duty 0x1e78600c = %x\n",reg);
       //PWM4-5
       *((volatile ulong*) 0x1e786048) = duty;
        printf("pwm4~5 duty 0x1e786048 = %x\n",reg);
       //PWM6-7
       *((volatile ulong*) 0x1e78604C) = duty;
        printf("pwm6~7 duty 0x1e78604C = %x\n",reg);

       *((volatile ulong*) 0x1e786010) = 0x10000001;
       *((volatile ulong*) 0x1e786018) = 0x10000001;
       *((volatile ulong*) 0x1e786014) = 0x10000000;
       *((volatile ulong*) 0x1e78601c) = 0x10000000;
       *((volatile ulong*) 0x1e786020) = 0;
       *((volatile ulong*) 0x1e786000) = 0xf01;
       *((volatile ulong*) 0x1e786040) = 0xf01;

        /*
         * The original file contained these comments.
         * TODO: verify the register write does what it claims
         *
         * LHCLK = HPLL/8
         * PCLK  = HPLL/8
         * BHCLK = HPLL/8
         */
        reg = readl(AST_SCU_BASE + AST_SCU_CLK_SEL);
        reg &= 0x1c0fffff;
        reg |= 0x61800000;
        writel(reg, AST_SCU_BASE + AST_SCU_CLK_SEL);

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

#ifdef CONFIG_FTGMAC100
int board_eth_init(bd_t *bd)
{
        return ftgmac100_initialize(bd);
}
#endif

#ifdef CONFIG_ASPEEDNIC
int board_eth_init(bd_t *bd)
{
        return aspeednic_initialize(bd);
}
#endif

/* Called by macro WATCHDOG_RESET */
#if defined(CONFIG_HW_WATCHDOG)
void hw_watchdog_reset(void)
{
	/* Restart WD2 timer */
	writel(0x4755, AST_WDT2_BASE + 0x08);
}
#endif /* CONFIG_WATCHDOG */
