/******************************************************************************

    @file    StateOS: osport.c
    @author  Rajmund Szymanski
    @date    02.12.2016
    @brief   StateOS port file for STM8S uC.

 ******************************************************************************

    StateOS - Copyright (C) 2013 Rajmund Szymanski.

    This file is part of StateOS distribution.

    StateOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation; either version 3 of the License,
    or (at your option) any later version.

    StateOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

 ******************************************************************************/

#include <oskernel.h>

/* -------------------------------------------------------------------------- */

void port_sys_init( void )
{
/******************************************************************************
 Put here configuration of system timer for non-tick-less mode
*******************************************************************************/

	CLK->CKDIVR = 0;
	CLK->ECKR  |= CLK_ECKR_HSEEN; while ((CLK->ECKR & CLK_ECKR_HSERDY) == 0);
	CLK->SWCR  |= CLK_SWCR_SWEN;
	CLK->SWR    = 0xB4; /* HSE */ while ((CLK->SWCR & CLK_SWCR_SWBSY)  == 1);

#define LN1_(X)   (X)
#define LN2_(X)  ((X)>>1?1+LN1_((X)>>1):LN1_(X))
#define LN4_(X)  ((X)>>2?2+LN2_((X)>>2):LN2_(X))
#define LN8_(X)  ((X)>>4?4+LN4_((X)>>4):LN4_(X))
#define LEN_(X)  ((X)>>8?8+LN8_((X)>>8):LN8_(X))
#define PSC_ LEN_((CPU_FREQUENCY/OS_FREQUENCY-1)>>16)
#define ARR_    (((CPU_FREQUENCY/OS_FREQUENCY)>>PSC_)-1)

	TIM3->PSCR  = PSC_;
	TIM3->ARRH  = ARR_ >> 8;
	TIM3->ARRL  = ARR_;
	TIM3->IER  |= TIM3_IER_UIE;
	TIM3->IER  |= TIM3_IER_CC1IE;
	TIM3->CR1  |= TIM3_CR1_CEN;

	enableInterrupts();

/******************************************************************************
 End of configuration
*******************************************************************************/
}

/* -------------------------------------------------------------------------- */

#if OS_TIMER == 0

/******************************************************************************
 Put here the procedure of interrupt handler of system timer for non-tick-less mode
*******************************************************************************/

@interrupt
void TIM3_UPD_OVF_BRK_IRQHandler( void )
{
	TIM3->SR1= ~TIM3_SR1_UIF;

	System.cnt++;
#if OS_ROBIN
	core_tmr_handler();
	System.dly++;
	if (System.dly >= OS_FREQUENCY/OS_ROBIN)
	port_ctx_switch();
#endif
}

/******************************************************************************
 End of the procedure of interrupt handler
*******************************************************************************/

#endif//OS_TIMER

/* -------------------------------------------------------------------------- */
