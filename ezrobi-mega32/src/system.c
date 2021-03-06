/* -*- Mode: C -*-
 *
 * $Id: system.c,v 1.5 2007/09/18 07:55:43 jdesch Exp $
 * --------------------------------------------------------------------------
 * Copyright  (c) Dipl.-Ing. Joerg Desch
 * --------------------------------------------------------------------------
 * PROJECT: ezROBI Mega32
 * MODULE.: SYSTEM.C: low level code
 * AUTHOR.: jdesch
 * --------------------------------------------------------------------------
 * DESCRIPTION:
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

#define __SYSTEM_C__
#include "system.h"
#include "cpu_avr.h"



#define DEBUG_KEY_IN 1


/*+=========================================================================+*/
/*|                      CONSTANT AND MACRO DEFINITIONS                     |*/
/*`========================================================================='*/
/* {{{ */

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  local macro declaration                        \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */


/* }}} */

/*+=========================================================================+*/
/*|                          LOCAL TYPEDECLARATIONS                         |*/
/*`========================================================================='*/
/* {{{ */

/* }}} */

/*+=========================================================================+*/
/*|                            PUBLIC VARIABLES                             |*/
/*`========================================================================='*/
/* {{{ */

/* }}} */

/*+=========================================================================+*/
/*|                             LOCAL VARIABLES                             |*/
/*`========================================================================='*/
/* {{{ */

static BYTE led_state = 0;	/* bitflags which remember the LED state */

/* }}} */

/*+=========================================================================+*/
/*|                      PROTOTYPES OF LOCAL FUNCTIONS                      |*/
/*`========================================================================='*/
/* {{{ */

static void setupCPU_Ports (void);
static T_Key mergedKeypadPins (void);

/* }}} */


/*+=========================================================================+*/
/*|                     IMPLEMENTATION OF THE FUNCTIONS                     |*/
/*`========================================================================='*/


/*             .-----------------------------------------------.             */
/* ___________/  Initialization                                 \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void sysInitHardware (void)
{
    cpuInitWatchDog();
    setupCPU_Ports();
 
    cli();			     /* don't disturbe our circles ;) */
    cpuInitADC();
    cpuInitPWM();

#if 0
    // External Interrupt(s) initialization
    // INT0: On
    // INT0 Mode: Falling Edge
    // INT1: On
    // INT1 Mode: Falling Edge
    // INT2: Off
    GICR|=0xC0;
    MCUCR=0x0A;
    MCUCSR=0x00;
    GIFR=0xC0;

    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK=0x41;
#endif

    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud rate: Double Speed Mode
    // USART RX-Interrupt On
    UCSRA=0x02;
    UCSRB=0x98;
    UCSRC=0x86;
#ifdef __CODEVISIONAVR__
#error "TODO: parsing the baudrate cfg with CodeVision ins't done yet!"
#endif    
#if (F_CPU == 16000000)
# if (CFG_V24_BAUD == 57600)
    UBRRH=0;  UBRRL=34;
# elif (CFG_V24_BAUD == 38400)
    UBRRH=0;  UBRRL=51;
# elif (CFG_V24_BAUD == 19200)
    UBRRH=0;  UBRRL=103;
# elif (CFG_V24_BAUD == 9600)
    UBRRH=0;  UBRRL=207;
# elif (CFG_V24_BAUD == 4800)
    UBRRH=1;  UBRRL=160;
# else
#  error "baudrate: illegal or undefined CFG_V24_BAUD" 
# endif
#else
#  error "baudrate: unsupported CPU speed (F_CPU)" 
#endif

    /* The IRQ Stuff
     */
    sei();
    
    /* init the local variables...
     */
    led_state = 0;
}


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  ACCESSING THE HARDWARE                         \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

void sysSetGreenLED (BOOL enable)
{
    cpuResetWatchDog();
    if (enable) led_state |= 0x01;
    else led_state &= (~0x01);
    out_GREEN_LED = (enable)?0:1;
}

void sysSetRedLED (BOOL enable)
{
    cpuResetWatchDog();
    if (enable) led_state |= 0x02;
    else led_state &= (~0x02);
    out_RED_LED = (enable)?0:1;
}

void sysSetYellowLED (BOOL enable)
{
    cpuResetWatchDog();
    if (enable) led_state |= 0x04;
    else led_state &= (~0x04);
    out_YELLOW_LED = (enable)?0:1;
}


/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  Accessing the Keypad                           \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

/** Returns the state of the keypad-port. The signals are already in positive
 * logic. So a set bit represents a pressed key.
 *
 * In this design, the LED outputs are shared with the keypad inputs. To get
 * the keys, we have to set the led outputs to 1, switch to inputs, read the
 * pin and restore the outputs.
 *
 * @return (T_Key) the inputs of the keypad port.
 * @memo internal function.
 */
T_Key sysReadRawKeypad (void)
{
    T_Key key_io;

    cpuResetWatchDog();
    out_GREEN_LED = 1;
    out_RED_LED = 1;
    out_YELLOW_LED = 1;
    SET_SW1_IN();
    SET_SW2_IN();
    SET_SW3_IN();

    key_io = mergedKeypadPins();
    if ( key_io )
    {
	cpuDelay_us(100);
	key_io = mergedKeypadPins();
    }

    SET_SW1_OUT();
    SET_SW2_OUT();
    SET_SW3_OUT();
    out_GREEN_LED = (led_state&0x01)? 0:1;
    out_RED_LED = (led_state&0x02)? 0:1;;
    out_YELLOW_LED = (led_state&0x04)? 0:1;;

    return key_io;
}



/* }}} */

/*+=========================================================================+*/
/*|                    IMPLEMENTATION OF LOCAL FUNCTIONS                    |*/
/*`========================================================================='*/
/* {{{ */


static void setupCPU_Ports (void)
{
    PORTA = VAL_PORT_A;  DDRA = DIR_PORT_A;
    PORTB = VAL_PORT_B;  DDRB = DIR_PORT_B;
    PORTC = VAL_PORT_C;  DDRC = DIR_PORT_C;
    PORTD = VAL_PORT_D;  DDRD = DIR_PORT_D;
}


/* Simply return the merged pins of the keypad. This means, all bits are
 * collected into one BYTE/WORD. Make sure that the bits are in positive logic.
 * So a pressed key must be 1!
 */
static T_Key mergedKeypadPins (void)
{
    T_Key w=0;

#ifdef DEBUG_KEY_IN
    out_MP1 = 0;
#endif
    cpuDelay_us(50);
//    if ( (in_SW1)==0 ) w |= SYS_SW1_KEY;
    if ( (in_SW2)==0 ) w |= SYS_SW2_KEY;
//    if ( (in_SW3)==0 ) w |= SYS_SW3_KEY;
#ifdef DEBUG_KEY_IN
    out_MP1 = 1;
#endif
    return w;
}

/* }}} */

/*             .-----------------------------------------------.             */
/* ___________/  INTERRUPT SERVICE ROUTINES                     \___________ */
/*            `-------------------------------------------------'            */
/* {{{ */

/* }}} */

/* ==[End of file]========================================================== */

