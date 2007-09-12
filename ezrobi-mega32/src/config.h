/* -*- Mode: C -*-
 *
 * $Id: config.h,v 1.3 2007/09/12 16:26:34 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: CONFIG.H: basic configuration of the application
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * This include file contains global definitions important for all
 * modules. It should "configure" the application. The adaption to
 * the hardware is not done here. Look inside platform.h to see the
 * hardware configuration!
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
 */


#ifndef __CONFIG_H__
#define __CONFIG_H__


/*+=========================================================================+*/
/*|                          GLOBAL COMPILER-FLAGS                          |*/
/*`========================================================================='*/

/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/


/*             .-----------------------------------------------.             */
/* ___________/  Configuration of the Application               \___________ */
/*            `-------------------------------------------------'            */

#define PRJNAME "ezROBI Mega32"

#define __V_MAJOR 0
#define __V_MINOR 1
#define __V_BUILD 1
#define __BUILDLEVEL "1"

#ifdef BETA
#define __VERSION "0.1-BLD" __BUILDLEVEL
#else
#define __VERSION "0.1"
#endif


/*             .-----------------------------------------------.             */
/* ___________/  Configuration depending on the MCU / Design    \___________ */
/*            `-------------------------------------------------'            */

/* CPU clock speed in Hz -- must be defined in the Makefile
 */
#ifndef F_CPU
#  error "FATAL: configure the CPU with F_CPU in Makefile!"
#endif

/* CYCLES_PER_US is used by some short delay loops. It specifies
 * the number of CPU cycles per usec.
 */
#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	

/* If there is an external watchdog timer designed in, set this compiler flag
 * to true, and define the output pin named out_XWATCHDOG. Don't forget to set
 * the corresponding direction bit to 1.
 *
 * Example:     #define out_XWATCHDOG __PIN(PORTC,6)
 */
#define CFG_USE_XWATCHDOG 0		     /* 1: use extern watchdog */


/*             .-----------------------------------------------.             */
/* ___________/  Configuration of the Modules                   \___________ */
/*            `-------------------------------------------------'            */

/* ===== THE SERIAL PORT =====
 */

/* select one of the predefined baudrates. Not all baudrate are available, and
 * there may be differences for the different CPU speed.
 */
#define CFG_V24_BAUD 57600

/* Set this to 1 means, that the TxD functions waits until the data has been
 * transmitted. 0 means, that the functiosn returns immediately after the data
 * has been copied to the transmittion register.
 */
#define CFG_V24_WAIT_FOR_TX 0

/* Define the allocation of the receive buffer. We use a simple and fast
 * calculation scheme here. The size must be a value (1<<n) and the mask is
 * ((1<<n)-1). So with n=4 we get size=8 and mask=7. With n=5 we will get
 * size=16(=0x10) and mask=15(=0x0F).
 */
#define V24_BUFFSIZE 0x08
#define V24_BUFFMASK 0x07

/* Enable some additional functions -- use a 0 to disable and a 1 to enable
 * the function.
 */
#define V24_NEED_PUT_UNSIGNED  0
#define V24_NEED_PUT_WORD      1
#define V24_NEED_PUT_BYTE      0


/* ===== THE ANALOG INPUTS (ADC) =====
 */

/* The ADC is has a resolution of 10bits. Nevertheless, in most cases,
 * this precision is not needed. So to restict the ADC to 8bit, set 
 * the compiler flag below to 1. This make the ADC function return
 * a BYTE (as T_ADC).
 */
#define CFG_USE_8BIT_ADC 1


/* ===== THE SOFTWARE TIMER =====
 */

/* Allocate the event counters. We must define the max. number of concurrent
 * timers here. If you can't determine this number, simply use the max. number
 * of defined timers. If you have not enough RAM, then think a little bit
 * harder. ;-)
 */
#define SWT_MAX_EVENTS 8


/* ===== THE REMOTE PROTOCOL PARSER =====
 */

/* Define the maximal number of numeric parameters the remote protocol
 * should support. The internal commands needs at least 3, so don't define
 * a number less than 3!
 */
#define RP_MAX_PARAMETERS 3


/* ===== THE MOTOR CONTROLER =====
 */

/* Here we configure the base clock the the PWM generator. For now, 
 * possible values are 1 to 3 where 1 is the fastest, 2 is normal and
 * 3 means slow.
 * 
 * For the M32-Rob we have to choose 3! This is because the motor
 * driver is not (really) designed for PWM usage. So we must use
 * a really slow clock to get the best power out of the driver. 
 */
#define CFG_PWM_SPEED 3



#include "platform.h"

#endif
/* ==[End of file]========================================================== */
