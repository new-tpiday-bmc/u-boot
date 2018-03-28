/*
 * (C) Copyright 2002 Ryan Chen
 * Copyright 2016 IBM Corporation
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/platform.h>
#include <asm/arch/ast-sdmc.h>
#include <asm/arch/regs-ahbc.h>
#include <asm/arch/regs-scu.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
    printf("Boot reached stage %d\n", progress);
}
#endif

int board_init(void)
{
	/* The BSP did this in the cpu code */
	icache_enable();
	dcache_enable();

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gd->flags = 0;
        printf("May test\n");
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
	gd->ram_size = ast_sdmc_get_mem_size();

	return 0;
}

int board_eth_init(bd_t *bd)
{
	return aspeednic_initialize(bd);
}
