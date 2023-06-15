/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2021 Chris Johns <chrisj@rtems.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Defines for Xilinx Versal ACAP SLCR registers.
 *
 * Reference:
 *  https://www.xilinx.com/html_docs/registers/am012/mod___crl.html
 */

#ifndef _VERSAL_SLCR_H_
#define _VERSAL_SLCR_H_

#define VERSAL_SLCR_CRF_OFFSET		0x01260000
#define VERSAL_SLCR_CRL_OFFSET		0x0f5e0000

/*
 * PLL controls
 *
 * P = PPLL = PMCPLL = PMCPLL_CTRL : PMC PLL Clock
 * N = NPLL = NOCPLL = NOCPLL_CTRL : NOC PLL Clock
 * R = RPLL = RPLL_CTRL            : Low Power Domain clock
 */
#define VERSAL_SLCR_P_PLL_CTRL		(VERSAL_SLCR_CRF_OFFSET + 0x40)
#define VERSAL_SLCR_N_PLL_CTRL		(VERSAL_SLCR_CRF_OFFSET + 0x50)
#define VERSAL_SLCR_R_PLL_CTRL		(VERSAL_SLCR_CRL_OFFSET + 0x40)
#define   VERSAL_SLCR_PLL_CTRL_RESET		(1<<0)
#define   VERSAL_SLCR_PLL_CTRL_BYPASS		(1<<3)
#define   VERSAL_SLCR_PLL_CTRL_FBDIV_SHIFT	8
#define   VERSAL_SLCR_PLL_CTRL_FBDIV_MAX	0xff
#define   VERSAL_SLCR_PLL_CTRL_FBDIV_MASK	(VERSAL_SLCR_PLL_CTRL_FBDIV_MAX<<8)
#define   VERSAL_SLCR_PLL_CTRL_DIV_SHIFT	(16)
#define   VERSAL_SLCR_PLL_CTRL_DIV_MASK	(0x3<<16)
#define   VERSAL_SLCR_PLL_CTRL_PRE_SRC_SHIFT	20
#define   VERSAL_SLCR_PLL_CTRL_PRE_SRC_MASK	(0x7<<20)
#define   VERSAL_SLCR_PLL_CTRL_POST_SRC_SHIFT	24
#define   VERSAL_SLCR_PLL_CTRL_POST_SRC_MASK	(0x7<<24)
#define     VERSAL_SLCR_PLL_CTRL_SRC_REF_CLK		0x0
#define     VERSAL_SLCR_PLL_CTRL_SRC_REF_CLK_MASK	0x2
#define     VERSAL_SLCR_PLL_CTRL_SRC_PL_PMC_ALT_REF_CLK_MASK	0x3

/* Clock controls. */
#define VERSAL_SLCR_GEM0_CLK_CTRL		(VERSAL_SLCR_CRL_OFFSET + 0x118)
#define VERSAL_SLCR_GEM1_CLK_CTRL		(VERSAL_SLCR_CRL_OFFSET + 0x11c)
#define   VERSAL_SLCR_GEM_CLK_CTRL_CLKACT_RX		(1<<27)
#define   VERSAL_SLCR_GEM_CLK_CTRL_CLKACT_TX		(1<<26)
#define   VERSAL_SLCR_GEM_CLK_CTRL_CLKACT		(1<<25)
#define   VERSAL_SLCR_GEM_CLK_CTRL_DIVISOR_MAX		0x3ff
#define   VERSAL_SLCR_GEM_CLK_CTRL_DIVISOR_MASK	(VERSAL_SLCR_GEM_CLK_CTRL_DIVISOR_MAX<<8)
#define   VERSAL_SLCR_GEM_CLK_CTRL_DIVISOR_SHIFT	8
#define   VERSAL_SLCR_GEM_CLK_CTRL_SRCSEL_MASK		(7<<0)
#define   VERSAL_SLCR_GEM_CLK_CTRL_SRCSEL_P_PLL		(0<<0)
#define   VERSAL_SLCR_GEM_CLK_CTRL_SRCSEL_R_PLL		(1<<0)
#define   VERSAL_SLCR_GEM_CLK_CTRL_SRCSEL_N_PLL		(3<<0)

#define VERSAL_SLCR_PPLL_TO_XPD_CTRL		(VERSAL_SLCR_CRF_OFFSET + 0x100)
#define VERSAL_SLCR_NPLL_TO_XPD_CTRL		(VERSAL_SLCR_CRF_OFFSET + 0x104)
#define   VERSAL_SLCR_XPD_CLK_CTRL_DIVISOR_MAX		0x3ff
#define   VERSAL_SLCR_XPD_CLK_CTRL_DIVISOR_MASK	(VERSAL_SLCR_XPD_CLK_CTRL_DIVISOR_MAX<<8)
#define   VERSAL_SLCR_XPD_CTRL_DIV_SHIFT		8

#define VERSAL_DEFAULT_PS_CLK_FREQUENCY 33333333

#ifdef _KERNEL
extern int cgem_set_ref_clk(int unit, int frequency);
#endif
#endif /* _VERSAL_SLCR_H_ */
