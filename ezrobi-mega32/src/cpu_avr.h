/* -*- Mode: C -*-
 *
 * $Id: cpu_avr.h,v 1.1 2007/08/29 12:42:24 jdesch Exp $
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

