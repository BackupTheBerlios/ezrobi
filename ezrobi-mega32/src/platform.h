/* -*- Mode: C -*-
 *
 * $Id: platform.h,v 1.2 2007/09/03 13:44:18 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: PLATFORM.H: declarations defining the hardware platform
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
 *
 * This file defines the platform specific binding of the microcontroller. 
 * This file is for the platform named "M32-Rob"
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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__ 1

#ifdef __GNUC__
#include <avr/io.h>
#endif
#ifdef __CODEVISIONAVR__
#include <io.h>				     /* include matching MCU header */
#endif
#include "config.h"
#include "defs.h"
#include "cpu_avr.h"



/*             .-----------------------------------------------.             */
/* ___________/  COMPILER-FLAGS                                 \___________ */
/*            `-------------------------------------------------'            */

/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/
/* {{{ */

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  CPU port declarations                          \___________ */
/*            `-------------------------------------------------'            */

/* DataDirection (DDR) flags of all ports: if the MCU run with an external bus
 * enabled, these definitions have no effect! Define only these ports your MCU
 * really have. AVR's define an output with an bit set to 1.
 */
/* {{{ */

#define DIR_PORT_A  BIN2BYTE(00000000)	     /*  */
#define DIR_PORT_B  BIN2BYTE(11111111)	     /*  */
#define DIR_PORT_C  BIN2BYTE(11000000)	     /*  */
#define DIR_PORT_D  BIN2BYTE(11110010)	     /* 0xF2 */

/* }}} */

/* Default values of the PORT register.
 *
 * OUTPUTS: For outputs, this is the level which will be set for the MCU pin.
 *
 * INPUTS: For inputs, this has a different meaning. If there is a 1 in the
 * port register, the input uses a internal pull-up resistor. If the bit is
 * set to 0, the input pin is tristate!
 */
/* {{{ */

#define VAL_PORT_A  BIN2BYTE(00000000)	     /*  */
#define VAL_PORT_B  BIN2BYTE(11111111)	     /*  */
#define VAL_PORT_C  BIN2BYTE(11000000)	     /* 0xC0 */
#define VAL_PORT_D  BIN2BYTE(11001111)	     /* 0xCF */

/* }}} */

/* Port A  -- declare the alias for pin's here. Use PORT* for outputs and
 *            PIN* for inputs.
 */
/* {{{ */
// COMPLETLY USED FOR THE KEYPAD
/* }}} */

/* Port B  -- declare the alias for pin's here. Use PORT* for outputs and
 *            PIN* for inputs.
 */
/* {{{ */

#define out_YELLOW_LED         __PIN(PORTB,5)
#define out_RED_LED            __PIN(PORTB,6)
#define out_GREEN_LED          __PIN(PORTB,7)
#define in_SW1                 __PIN(PINB,5)
#define in_SW2                 __PIN(PINB,6)
#define in_SW3                 __PIN(PINB,7)

/* }}} */

/* Because we use the pins 5..7 as input and as output, we need the following
 * macros to switch the direction.
 */
/* {{{ */

#define SET_SW1_IN()           cbi(DDRB,5)
#define SET_SW1_OUT()          sbi(DDRB,5)
#define SET_SW2_IN()           cbi(DDRB,6)
#define SET_SW2_OUT()          sbi(DDRB,6)
#define SET_SW3_IN()           cbi(DDRB,7)
#define SET_SW3_OUT()          sbi(DDRB,7)

/* }}} */

/* Port C  -- declare the alias for pin's here. Use PORT* for outputs and
 *            PIN* for inputs.
 */
/* {{{ */

#define out_MP1                __PIN(PORTC,6)
#define out_MP2                __PIN(PORTC,7)

/* }}} */

/* Port D  -- declare the alias for pin's here. Use PORT* for outputs and
 *            PIN* for inputs.
 */
/* {{{ */
#define out_MOTOR1_ENAB        __PIN(PORTD,4)   /* 1=active 0=stand-by */
#define out_MOTOR2_ENAB        __PIN(PORTD,5)   /* 1=active 0=stand-by */
#define out_MOTOR1_DIR         __PIN(PORTD,6)
#define out_MOTOR2_DIR         __PIN(PORTD,7)

/* }}} */

#endif
/* ==[End of file]========================================================== */

