/* -*- Mode: C -*-
 *
 * $Id: platform.h,v 1.4 2008/04/14 04:24:35 jdesch Exp $
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
 * This is the implementation for the platform named "M32-Rob".
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

/* This section defines all input / output pins of the CPU. The macros
 * #DIR_PORT_*# defines the 'data direction' and the macros #VAL_PORT_*#
 * the 'default power-up values.
 * 
 * DataDirection (DDR) flags of all ports: if the MCU run with an external bus
 * enabled, these definitions have no effect! Define only these ports your MCU
 * really have. AVR's define an output with an bit set to 1.
 *
 * 
 * The default values of the PORT register have different meanings.
 *
 * OUTPUTS: For outputs, this is the level which will be set for the MCU pin.
 *
 * INPUTS: For inputs, this has a different meaning. If there is a 1 in the
 * port register, the input uses a internal pull-up resistor. If the bit is
 * set to 0, the input pin is tristate!
 */



/* Port A  -- declare the alias for pin's here. Use PORT* for outputs and
 *            PIN* for inputs.
 */
/* {{{ */
/* These are the 8 ADC inputs. It's up to you to used it as ADC or
 * as normal IO. For now, they are defines as inputs without pull-up!
 */
//                           ,--------[7]  *all pins* ADC inputs
//                           |,-------[6]  
//                           ||,------[5]  
//                           |||,-----[4]  
//                           ||||,----[3]  
//                           |||||,---[2]  
//                           ||||||,--[1]  
//                           |||||||,-[0]  
#define DIR_PORT_A  BIN2BYTE(00000000)
#define VAL_PORT_A  BIN2BYTE(00000000)

/* }}} */

/* Port B  -- declare the alias for pin's here. Use PORT* for outputs and
 *            PIN* for inputs.
 */
/* {{{ */
//                           ,--------[7]  out: green led  / in: sw3
//                           |,-------[6]  out: red led    / in: sw2
//                           ||,------[5]  out: yellow led / in: sw1
//                           |||,-----[4]  out: MP3
//                           ||||,----[3]  out: unused
//                           |||||,---[2]  out: unused
//                           ||||||,--[1]  out: unused
//                           |||||||,-[0]  out: unused
#define DIR_PORT_B  BIN2BYTE(11111111)
#define VAL_PORT_B  BIN2BYTE(11111111)

#define in_out_UNUSED_PB0      __PIN(PORTB,0)
#define in_out_UNUSED_PB1      __PIN(PORTB,1)
#define in_out_UNUSED_PB2      __PIN(PORTB,2)
#define in_out_UNUSED_PB3      __PIN(PORTB,3)
#define out_MP3                __PIN(PORTB,4)
#define out_YELLOW_LED         __PIN(PORTB,5)
#define out_RED_LED            __PIN(PORTB,6)
#define out_GREEN_LED          __PIN(PORTB,7)
#define in_SW1                 __PIN(PINB,5)
#define in_SW2                 __PIN(PINB,6)
#define in_SW3                 __PIN(PINB,7)

/* Because we use the pins 5..7 as input and as output, we need the following
 * macros to switch the direction.
 */
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

//                           ,--------[7]  out: MP2
//                           |,-------[6]  out: MP1
//                           ||,------[5]  
//                           |||,-----[4]  
//                           ||||,----[3]  
//                           |||||,---[2]  
//                           ||||||,--[1]  
//                           |||||||,-[0]  
#define DIR_PORT_C  BIN2BYTE(11000000)
#define VAL_PORT_C  BIN2BYTE(11000000)

#define out_MP1                __PIN(PORTC,6)
#define out_MP2                __PIN(PORTC,7)

/* }}} */

/* Port D  -- declare the alias for pin's here. Use PORT* for outputs and
 *            PIN* for inputs.
 */
/* {{{ */
//                           ,--------[7]  out: motor-1 direction
//                           |,-------[6]  out: motor-1 direction
//                           ||,------[5]  out: motor-2 enable
//                           |||,-----[4]  out: motor-1 enable
//                           ||||,----[3]  in: IO or INT  
//                           |||||,---[2]  in: IO or INT
//                           ||||||,--[1]  
//                           |||||||,-[0]  
#define DIR_PORT_D  BIN2BYTE(11110010)
#define VAL_PORT_D  BIN2BYTE(11001111)

#define in_IO_OR_INT0          __PIN(PIND,2)    /* unused: either IO or INT */
#define in_IO_OR_INT1          __PIN(PIND,3)    /* unused: either IO or INT */
#define out_MOTOR1_ENAB        __PIN(PORTD,4)   /* 1=active 0=stand-by */
#define out_MOTOR2_ENAB        __PIN(PORTD,5)   /* 1=active 0=stand-by */
#define out_MOTOR1_DIR         __PIN(PORTD,6)
#define out_MOTOR2_DIR         __PIN(PORTD,7)

/* }}} */

#endif
/* ==[End of file]========================================================== */

