/* -*- Mode: C -*-
 *
 * $Id: cpu_avr.h,v 1.5 2008/04/14 04:24:35 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: CPU_AVR.H: CPU specific declarations and functions
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * This module defines and declares all the AVR related stuff. This means, all
 * the code here is independent from the application code.
 * 
 * All the low level code that is part of the application has to be put into
 * system.c!
 * 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * --------------------------------------------------------------------------
 * COMPILER-FLAGS:
 *
 * --------------------------------------------------------------------------
 */

#ifndef __CPU_AVR_H__
#define __CPU_AVR_H__
#include "config.h"


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/


/* Some constants for timer configuration...
 */
/* Prescaler Values
 * ATMega128x/256x: Timer0/1/3/4
 * ATMega128: Timer1/2/3
 * ATMega32:  Timer0/1/3
 * ATMega8:   Timer0/1/3
 */
#define TIMER_CLK_STOP	        0x00	// Timer1/2/3 Stopped
#define TIMER_CLK_DIV1	        0x01	// clocked at F_CPU
#define TIMER_CLK_DIV8	        0x02	// clocked at F_CPU/8
#define TIMER_CLK_DIV64	        0x03	// clocked at F_CPU/64
#define TIMER_CLK_DIV256	0x04	// clocked at F_CPU/256
#define TIMER_CLK_DIV1024	0x05	// clocked at F_CPU/1024
#define TIMER_CLK_T_FALL	0x06	// clocked at T falling edge
#define TIMER_CLK_T_RISE	0x07	// clocked at T rising edge
#define TIMER_PRESCALE_MASK	0x07	// Timer Prescaler Bit-Mask

/* (Extended) Prescaler Values:
 * ATMega128x/256x: Timer2
 * ATMega128: Timer0
 * ATMega32:  Timer2
 * ATMega8:   Timer2
 */
#define TIMERX_CLK_STOP	        0x00	// Timer0 Stopped
#define TIMERX_CLK_DIV1	        0x01	// clocked at F_CPU
#define TIMERX_CLK_DIV8	        0x02	// clocked at F_CPU/8
#define TIMERX_CLK_DIV32	0x03	// clocked at F_CPU/32
#define TIMERX_CLK_DIV64	0x04	// clocked at F_CPU/64
#define TIMERX_CLK_DIV128	0x05	// clocked at F_CPU/128
#define TIMERX_CLK_DIV256	0x06	// clocked at F_CPU/256
#define TIMERX_CLK_DIV1024	0x07	// clocked at F_CPU/1024
#define TIMERX_PRESCALE_MASK	0x07	// Timer0 Prescaler Bit-Mask


/*+=========================================================================+*/
/*|                            TYPEDECLARATIONS                             |*/
/*`========================================================================='*/

#if CFG_USE_8BIT_ADC
typedef BYTE T_ADC;
#else
typedef WORD T_ADC;
#endif


/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                     PROTOTYPES OF GLOBAL FUNCTIONS                      |*/
/*`========================================================================='*/

void cpuInitWatchDog (void);
void cpuResetWatchDog (void);

void cpuDelay_ms (BYTE time_ms);
void cpuDelay_us (WORD time_us);

void cpuInitADC (void);
T_ADC cpuReadADC (BYTE Channel);

void cpuInitPWM (void );
void cpuSetPWM1 (BYTE Speed);
void cpuSetPWM2 (BYTE Speed);
void cpuSetPWM3 (BYTE Speed);

#endif

/* ==[End of file]========================================================== */

