/* -*- Mode: C -*-
 *
 * $Id: cpu_avr.h,v 1.3 2007/12/17 12:27:38 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: CPU_AVR.H: CPU specific declarations and functions
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * This module defines and declares all the AVR related stuff. This means,
 * alle the code here is independent from the application code.
 * 
 * All the low level code that is part of the application is put into
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

/* Definitions for the delay passed to cpuDelay()
 */
#define DEL_10MS            1
#define DEL_50MS            5
#define DEL_100MS          10
#define DEL_250MS          25
#define DEL_500MS          50
#define DEL_1000MS        100


/* Some constants for timer configuration...
 */
// Timer-1, ...
#define TIMER_CLK_STOP		0x00	// Timer Stopped
#define TIMER_CLK_DIV1		0x01	// Timer clocked at F_CPU
#define TIMER_CLK_DIV8		0x02	// Timer clocked at F_CPU/8
#define TIMER_CLK_DIV64		0x03	// Timer clocked at F_CPU/64
#define TIMER_CLK_DIV256	0x04	// Timer clocked at F_CPU/256
#define TIMER_CLK_DIV1024	0x05	// Timer clocked at F_CPU/1024
#define TIMER_CLK_T_FALL	0x06	// Timer clocked at T falling edge
#define TIMER_CLK_T_RISE	0x07	// Timer clocked at T rising edge
#define TIMER_PRESCALE_MASK	0x07	// Timer Prescaler Bit-Mask
// Timer-0
#define TIMER0_CLK_STOP		0x00	// Timer Stopped
#define TIMER0_CLK_DIV1		0x01	// Timer clocked at F_CPU
#define TIMER0_CLK_DIV8		0x02	// Timer clocked at F_CPU/8
#define TIMER0_CLK_DIV32	0x03	// Timer clocked at F_CPU/32
#define TIMER0_CLK_DIV64	0x04	// Timer clocked at F_CPU/64
#define TIMER0_CLK_DIV128	0x05	// Timer clocked at F_CPU/128
#define TIMER0_CLK_DIV256	0x06	// Timer clocked at F_CPU/256
#define TIMER0_CLK_DIV1024	0x07	// Timer clocked at F_CPU/1024
#define TIMER0_PRESCALE_MASK	0x07	// Timer Prescaler Bit-Mask


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

void cpuDelay (unsigned int time);
void cpuDelay_us (unsigned short time_us);

void cpuInitADC (void);
T_ADC cpuReadADC (BYTE Channel);

void cpuInitPWM (void );
void cpuSetPWM1 (BYTE Speed);
void cpuSetPWM2 (BYTE Speed);
void cpuSetPWM3 (BYTE Speed);

#endif

/* ==[End of file]========================================================== */

